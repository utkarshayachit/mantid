/*WIKI* 


This algorithm sums, bin-by-bin, multiple spectra into a single spectra. The errors are summed in quadrature and the algorithm checks that the bin boundaries in X are the same. The new summed spectra are created at the start of the output workspace and have spectra index numbers that start at zero and increase in the order the groups are specified. Each new group takes the spectra numbers from the first input spectrum specified for that group. All detectors from the grouped spectra will be moved to belong to the new spectrum.

Not all spectra in the input workspace have to be copied to a group. If KeepUngroupedSpectra is set to true any spectra not listed will be copied to the output workspace after the groups in order. If KeepUngroupedSpectra is set to false only the spectra selected to be in a group will be used.

To create a single group the list of spectra can be identified using a list of either spectrum numbers, detector IDs or workspace indices. The list should be set against the appropriate property.

See [[LoadDetectorsGroupingFile]] for a description of a MapFile format.

== Previous Versions ==

=== Version 1 ===
The set of spectra to be grouped can be given as a list of either spectrum numbers, detector IDs or workspace indices. The new, summed spectrum will appear in the workspace at the first workspace index of the pre-grouped spectra (which will be given by the ResultIndex property after execution). The detectors for all the grouped spectra will be moved to belong to the first spectrum. ''A technical note: the workspace indices previously occupied by summed spectra will have their data zeroed and their spectrum number set to a value of -1.''


*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidDataHandling/GroupDetectors2.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/SpectraAxis.h"
#include "MantidAPI/WorkspaceOpOverloads.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidAPI/FileProperty.h"
#include "MantidKernel/Exception.h"
#include "MantidKernel/ListValidator.h"
#include "MantidDataHandling/LoadDetectorsGroupingFile.h"


#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <ios>
#include <set>
#include <vector>
#include <numeric>

#include <Poco/StringTokenizer.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/SAX/ContentHandler.h>
#include <Poco/SAX/Attributes.h>
#include <Poco/SAX/SAXParser.h>

namespace Mantid
{
namespace DataHandling
{
// Register the algorithm into the algorithm factory
DECLARE_ALGORITHM(GroupDetectors2)

/// Sets documentation strings for this algorithm
void GroupDetectors2::initDocs()
{
  this->setWikiSummary("Sums spectra bin-by-bin, equivalent to grouping the data from a set of detectors.  Individual groups can be specified by passing the algorithm a list of spectrum numbers, detector IDs or workspace indices. Many spectra groups can be created in one execution via an input file. ");
  this->setOptionalMessage("Sums spectra bin-by-bin, equivalent to grouping the data from a set of detectors.  Individual groups can be specified by passing the algorithm a list of spectrum numbers, detector IDs or workspace indices. Many spectra groups can be created in one execution via an input file.");
}

using namespace Kernel;
using namespace API;
using namespace DataObjects;
using std::size_t;

/// (Empty) Constructor
GroupDetectors2::GroupDetectors2() : m_FracCompl(0.0) {}

/// Destructor
GroupDetectors2::~GroupDetectors2() {}

// progress estimates
const double GroupDetectors2::CHECKBINS = 0.10;
const double GroupDetectors2::READFILE = 0.10;

void GroupDetectors2::init()
{
  declareProperty(new WorkspaceProperty<MatrixWorkspace>("InputWorkspace","",Direction::Input,
                                          boost::make_shared<CommonBinsValidator>()),"The name of the input 2D workspace");
  declareProperty(new WorkspaceProperty<MatrixWorkspace>("OutputWorkspace","",Direction::Output),
    "The name of the output workspace");
  std::vector<std::string> fileExts(2);
  fileExts[0] = ".xml";
  fileExts[1] = ".map";
  declareProperty(new FileProperty("MapFile", "", FileProperty::OptionalLoad, fileExts),
    "A file that consists of lists of spectra numbers to group. See the help\n"
    "for the file format");
  declareProperty(new ArrayProperty<specid_t>("SpectraList"),
    "An array containing a list of the spectrum numbers to combine\n"
    "(DetectorList and WorkspaceIndexList are ignored if this is set)" );
  declareProperty(new ArrayProperty<detid_t>("DetectorList"),
    "An array of detector IDs to combine (WorkspaceIndexList is ignored if this is\n"
    "set)" );
  declareProperty(new ArrayProperty<size_t>("WorkspaceIndexList"),
    "An array of workspace indices to combine" );
  declareProperty("KeepUngroupedSpectra",false,
    "If true ungrouped spectra will be copied to the output workspace\n"
    "and placed after the groups");

  std::vector<std::string> groupTypes(2);
  groupTypes[0] = "Sum";
  groupTypes[1] = "Average";
  using Mantid::Kernel::StringListValidator;
  declareProperty("Behaviour", "Sum", boost::make_shared<StringListValidator>(groupTypes),
                  "Whether to sum or average the values when grouping detectors.");
  // Are we preserving event workspaces?
  declareProperty("PreserveEvents", true, "Keep the output workspace as an EventWorkspace, if the input has events.");
}

void GroupDetectors2::exec()
{
  // Get the input workspace
  const MatrixWorkspace_const_sptr inputWS = getProperty("InputWorkspace");

  //Check if it is an event workspace
  const bool preserveEvents = getProperty("PreserveEvents");
  EventWorkspace_const_sptr eventW = boost::dynamic_pointer_cast<const EventWorkspace>(inputWS);
  if (eventW != NULL && preserveEvents)
  {
    this->execEvent();
    return;
  }

  const size_t numInHists = inputWS->getNumberHistograms();
  // Bin boundaries need to be the same, so do the full check on whether they actually are
  if (!API::WorkspaceHelpers::commonBoundaries(inputWS))
  {
    g_log.error() << "Can only group if the histograms have common bin boundaries\n";
    throw std::invalid_argument("Can only group if the histograms have common bin boundaries");
  }
  progress( m_FracCompl = CHECKBINS );
  interruption_point();

  // some values loaded into this vector can be negative so this needs to be a signed type
  std::vector<int64_t> unGroupedInds;
  //the ungrouped list could be very big but might be none at all
  unGroupedInds.reserve(numInHists);
  for( size_t i = 0; i < numInHists ; i++ )
  {
    unGroupedInds.push_back(i);
  }

  // read in the input parameters to make that map, if KeepUngroupedSpectra was set we'll need a list of the ungrouped spectrra too
  getGroups(inputWS, unGroupedInds);

  // converting the list into a set gets rid of repeated values, here the multiple GroupDetectors2::USED become one USED at the start
  const std::set<int64_t> unGroupedSet(unGroupedInds.begin(), unGroupedInds.end());

  // Check what the user asked to be done with ungrouped spectra
  const bool keepAll = getProperty("KeepUngroupedSpectra");
  // ignore the one USED value in set or ignore all the ungrouped if the user doesn't want them
  const size_t numUnGrouped = keepAll ? unGroupedSet.size()-1 : 0;

  MatrixWorkspace_sptr outputWS =
    WorkspaceFactory::Instance().create(inputWS, m_GroupSpecInds.size()+ numUnGrouped,
                                        inputWS->readX(0).size(), inputWS->blocksize());

  // prepare to move the requested histograms into groups, first estimate how long for progress reporting. +1 in the demonator gets rid of any divide by zero risk
  double prog4Copy=( (1.0 - m_FracCompl)/(static_cast<double>(numInHists-unGroupedSet.size())+1.) )*
    (keepAll ? static_cast<double>(numInHists-unGroupedSet.size())/static_cast<double>(numInHists): 1.);

  // Build a new map
  const size_t outIndex = formGroups(inputWS, outputWS, prog4Copy);

  // If we're keeping ungrouped spectra
  if (keepAll)
  {
    // copy them into the output workspace
    moveOthers(unGroupedSet, inputWS, outputWS, outIndex);
  } 
  
  g_log.information() << name() << " algorithm has finished\n";

  setProperty("OutputWorkspace",outputWS);
}

void GroupDetectors2::execEvent()
{
    // Get the input workspace
    const MatrixWorkspace_const_sptr matrixInputWS = getProperty("InputWorkspace");
    EventWorkspace_const_sptr inputWS= boost::dynamic_pointer_cast<const EventWorkspace>(matrixInputWS);


    const size_t numInHists = inputWS->getNumberHistograms();
    progress(m_FracCompl = CHECKBINS);
    interruption_point();

    // some values loaded into this vector can be negative so this needs to be a signed type
    std::vector<int64_t> unGroupedInds;
    //the ungrouped list could be very big but might be none at all
    unGroupedInds.reserve(numInHists);
    for( size_t i = 0; i < numInHists ; i++ )
    {
      unGroupedInds.push_back(i);
    }

    // read in the input parameters to make that map, if KeepUngroupedSpectra was set we'll need a list of the ungrouped spectrra too
    getGroups(inputWS, unGroupedInds);

    // converting the list into a set gets rid of repeated values, here the multiple GroupDetectors2::USED become one USED at the start
    const std::set<int64_t> unGroupedSet(unGroupedInds.begin(), unGroupedInds.end());

    // Check what the user asked to be done with ungrouped spectra
    const bool keepAll = getProperty("KeepUngroupedSpectra");
    // ignore the one USED value in set or ignore all the ungrouped if the user doesn't want them
    const size_t numUnGrouped = keepAll ? unGroupedSet.size()-1 : 0;

    //Make a brand new EventWorkspace
    EventWorkspace_sptr outputWS = boost::dynamic_pointer_cast<EventWorkspace>(
        WorkspaceFactory::Instance().create("EventWorkspace",  m_GroupSpecInds.size()+ numUnGrouped,
                                                  inputWS->readX(0).size(), inputWS->blocksize()));
    //Copy geometry over.
    WorkspaceFactory::Instance().initializeFromParent(inputWS, outputWS, true);

    // prepare to move the requested histograms into groups, first estimate how long for progress reporting. +1 in the demonator gets rid of any divide by zero risk
    double prog4Copy=( (1.0 - m_FracCompl)/(static_cast<double>(numInHists-unGroupedSet.size())+1.) )*
      (keepAll ? static_cast<double>(numInHists-unGroupedSet.size())/static_cast<double>(numInHists): 1.);

    // Build a new map
    const size_t outIndex = formGroupsEvent(inputWS, outputWS, prog4Copy);

    // If we're keeping ungrouped spectra
    if (keepAll)
    {
      // copy them into the output workspace
      moveOthersEvent(unGroupedSet, inputWS, outputWS, outIndex);
    }

    //Set all X bins on the output
    cow_ptr<MantidVec> XValues;
    XValues.access() = inputWS->readX(0);
    outputWS->setAllX(XValues);

    g_log.information() << name() << " algorithm has finished\n";

    setProperty("OutputWorkspace",outputWS);
}

/** Make a map containing spectra indexes to group, the indexes could have come from
*  file, or an array, spectra numbers ...
*  @param workspace :: the user selected input workspace
*  @param unUsedSpec :: spectra indexes that are not members of any group
*/
void GroupDetectors2::getGroups(API::MatrixWorkspace_const_sptr workspace,
                       std::vector<int64_t> &unUsedSpec)
{
  // this is the map that we are going to fill
  m_GroupSpecInds.clear();

  // There are several properties that may contain the user data go through them in order of precedence
  const std::string filename = getProperty("MapFile");
  if ( ! filename.empty() )
  {// The file property has been set so try to load the file


    Mantid::API::IAlgorithm_sptr loader = createChildAlgorithm("LoadDetectorsGroupingFile");
    loader->initialize();
    loader->setProperty("InputFile", filename);

    // Will not work till progress reporting implemented in LoadDetectorsGroupingFile
    setChildStartProgress(m_FracCompl);
    setChildEndProgress(m_FracCompl + READFILE);
    
    loader->execute();

    GroupingWorkspace_sptr groupingWS = loader->getProperty("OutputWorkspace");

    if(!groupingWS)
      throw std::invalid_argument("Unable to parse grouping from file");

    getGroupsFromWS(groupingWS, workspace, unUsedSpec);

    if(m_GroupSpecInds.empty())
      throw std::invalid_argument("No groups specified in a file, nothing to do");

    progress(m_FracCompl += READFILE);
    interruption_point();

    return;
  }

  const std::vector<specid_t> spectraList = getProperty("SpectraList");
  const std::vector<detid_t> detectorList = getProperty("DetectorList");
  const std::vector<size_t> indexList = getProperty("WorkspaceIndexList");

  // only look at these other parameters if the file wasn't set
  if ( ! spectraList.empty() )
  {
    workspace->getIndicesFromSpectra( spectraList, m_GroupSpecInds[0]);
    g_log.debug() << "Converted " << spectraList.size() << " spectra numbers into spectra indices to be combined\n";
  }
  else
  {// go through the rest of the properties in order of decreasing presidence, abort when we get the data we need ignore the rest
    if ( ! detectorList.empty() )
    {
      // we are going to group on the basis of detector IDs, convert from detectors to workspace indices
      workspace->getIndicesFromDetectorIDs( detectorList, m_GroupSpecInds[0]);
      g_log.debug() << "Found " << m_GroupSpecInds[0].size() << " spectra indices from the list of " << detectorList.size() << " detectors\n";
    }
    else if ( ! indexList.empty() )
    {
      m_GroupSpecInds[0] = indexList;
      g_log.debug() << "Read in " << m_GroupSpecInds[0].size() << " spectra indices to be combined\n";
    }
    //check we don't have an index that is too high for the workspace
    size_t maxIn = static_cast<size_t>(workspace->getNumberHistograms() - 1);
    std::vector<size_t>::const_iterator it = m_GroupSpecInds[0].begin();
    for( ; it != m_GroupSpecInds[0].end() ; ++it )
    {
      if ( *it > maxIn )
      {
        g_log.error() << "Spectra index " << *it << " doesn't exist in the input workspace, the highest possible index is " << maxIn << std::endl;
        throw std::out_of_range("One of the spectra requested to group does not exist in the input workspace");
      }
    }
  }

  if ( m_GroupSpecInds[0].empty() )
  {
    g_log.information() << name() << ": File, WorkspaceIndexList, SpectraList, and DetectorList properties are all empty\n";
    throw std::invalid_argument("All list properties are empty, nothing to group");
  }

  // up date unUsedSpec, this is used to find duplicates and when the user has set KeepUngroupedSpectra
  std::vector<size_t>::const_iterator index = m_GroupSpecInds[0].begin();
  for (  ; index != m_GroupSpecInds[0].end(); ++index )
  {// the vector<int> m_GroupSpecInds[0] must not index contain numbers that don't exist in the workspaace
    if ( unUsedSpec[*index] != USED )
    {
      unUsedSpec[*index] = USED;
    }
    else g_log.warning() << "Duplicate index, " << *index << ", found\n";
  }
}

/**
 * Updates m_GroupSpecInds and unUsedSpec in the way that it matches the information provided in
 * groupingWS.
 * @param groupingWS WS with grouping information
 * @param unUsedSpec Vector with elements for every spectra which are set to USED if they are in some group.
 */
void GroupDetectors2::getGroupsFromWS(GroupingWorkspace_const_sptr groupingWS, MatrixWorkspace_const_sptr inputWS,
                                      std::vector<int64_t> &unUsedSpec)
{

  const spec2index_map* inputSpec2index = inputWS->getSpectrumToWorkspaceIndexMap();

  const size_t noOfSpectra = groupingWS->getNumberHistograms();

  for(size_t i = 0; i < noOfSpectra; i++)
  {
    const specid_t groupNo = static_cast<specid_t>(groupingWS->readY(i)[0]);

    if(groupNo != 0)
    {
      const specid_t spectrumNo = groupingWS->getSpectrum(i)->getSpectrumNo();

      spec2index_map::const_iterator it = inputSpec2index->find(spectrumNo);

      if(it == inputSpec2index->end())
        throw std::out_of_range("One of the spectra requested to group does not exist in the input workspace");

      const size_t& inputIndex = it->second;

      // it->second is input workspace index for the spectra
      m_GroupSpecInds[groupNo].push_back(inputIndex);

      unUsedSpec[inputIndex] = USED;
    }
  }
}

/**
*  Move the user selected spectra in the input workspace into groups in the output workspace
*  @param inputWS :: user selected input workspace for the algorithm
*  @param outputWS :: user selected output workspace for the algorithm
*  @param prog4Copy :: the amount of algorithm progress to attribute to moving a single spectra
*  @return number of new grouped spectra
*/
size_t GroupDetectors2::formGroups( API::MatrixWorkspace_const_sptr inputWS, API::MatrixWorkspace_sptr outputWS, 
            const double prog4Copy)
{
  // get "Behaviour" string
  const std::string behaviour = getProperty("Behaviour");
  int bhv = 0;
  if ( behaviour == "Average" ) bhv = 1;

  API::MatrixWorkspace_sptr beh = API::WorkspaceFactory::Instance().create(
    "Workspace2D", static_cast<int>(m_GroupSpecInds.size()), 1, 1);

  g_log.debug() << name() << ": Preparing to group spectra into " << m_GroupSpecInds.size() << " groups\n";

  // where we are copying spectra to, we start copying to the start of the output workspace
  size_t outIndex = 0;
  // Only used for averaging behaviour. We may have a 1:1 map where a Divide would be waste as it would be just dividing by 1
  bool requireDivide(false);
  for ( storage_map::const_iterator it = m_GroupSpecInds.begin(); it != m_GroupSpecInds.end() ; ++it )
  {
    // This is the grouped spectrum
    ISpectrum * outSpec = outputWS->getSpectrum(outIndex);

    // The spectrum number of the group is the key
    outSpec->setSpectrumNo(it->first);
    // Start fresh with no detector IDs
    outSpec->clearDetectorIDs();

    // Copy over X data from first spectrum, the bin boundaries for all spectra are assumed to be the same here
    outSpec->dataX() = inputWS->readX(0);

    // the Y values and errors from spectra being grouped are combined in the output spectrum
    // Keep track of number of detectors required for masking
    size_t nonMaskedSpectra(0);
    beh->dataX(outIndex)[0] = 0.0;
    beh->dataE(outIndex)[0] = 0.0;
    for( std::vector<size_t>::const_iterator wsIter = it->second.begin(); wsIter != it->second.end(); ++wsIter)
    {
      const size_t originalWI = *wsIter;

      // detectors to add to firstSpecNum
      const ISpectrum * fromSpectrum = inputWS->getSpectrum(originalWI);

      // Add up all the Y spectra and store the result in the first one
      // Need to keep the next 3 lines inside loop for now until ManagedWorkspace mru-list works properly
      MantidVec &firstY = outSpec->dataY();
      MantidVec::iterator fYit;
      MantidVec::iterator fEit = outSpec->dataE().begin();
      MantidVec::const_iterator Yit = fromSpectrum->dataY().begin();
      MantidVec::const_iterator Eit = fromSpectrum->dataE().begin();
      for (fYit = firstY.begin(); fYit != firstY.end(); ++fYit, ++fEit, ++Yit, ++Eit)
      {
        *fYit += *Yit;
        // Assume 'normal' (i.e. Gaussian) combination of errors
        *fEit = std::sqrt( (*fEit)*(*fEit) + (*Eit)*(*Eit) );
      }

      // detectors to add to the output spectrum
      outSpec->addDetectorIDs(fromSpectrum->getDetectorIDs() );
      try
      {
        Geometry::IDetector_const_sptr det = inputWS->getDetector(originalWI);
        if( !det->isMasked() ) ++nonMaskedSpectra;
      }
      catch(Exception::NotFoundError&)
      {
        // If a detector cannot be found, it cannot be masked
        ++nonMaskedSpectra;
      }
    }
    if( nonMaskedSpectra == 0 ) ++nonMaskedSpectra; // Avoid possible divide by zero
    if(!requireDivide) requireDivide = (nonMaskedSpectra > 1);
    beh->dataY(outIndex)[0] = static_cast<double>(nonMaskedSpectra);

    // make regular progress reports and check for cancelling the algorithm
    if ( outIndex % INTERVAL == 0 )
    {
      m_FracCompl += INTERVAL*prog4Copy;
      if ( m_FracCompl > 1.0 )
        m_FracCompl = 1.0;
      progress(m_FracCompl);
      interruption_point();
    }
    outIndex ++;
  }
  
  if ( bhv == 1 && requireDivide )
  {
    g_log.debug() << "Running Divide algorithm to perform averaging.\n";
    Mantid::API::IAlgorithm_sptr divide = createChildAlgorithm("Divide");
    divide->initialize();
    divide->setProperty<API::MatrixWorkspace_sptr>("LHSWorkspace", outputWS);
    divide->setProperty<API::MatrixWorkspace_sptr>("RHSWorkspace", beh);
    divide->setProperty<API::MatrixWorkspace_sptr>("OutputWorkspace", outputWS);
    divide->execute();
  }

  g_log.debug() << name() << " created " << outIndex << " new grouped spectra\n";
  return outIndex;
}


/**
*  Move the user selected spectra in the input workspace into groups in the output workspace
*  @param inputWS :: user selected input workspace for the algorithm
*  @param outputWS :: user selected output workspace for the algorithm
*  @param prog4Copy :: the amount of algorithm progress to attribute to moving a single spectra
*  @return number of new grouped spectra
*/
size_t GroupDetectors2::formGroupsEvent( DataObjects::EventWorkspace_const_sptr inputWS, DataObjects::EventWorkspace_sptr  outputWS,
            const double prog4Copy)
{
  // get "Behaviour" string
  const std::string behaviour = getProperty("Behaviour");
  int bhv = 0;
  if ( behaviour == "Average" ) bhv = 1;

  API::MatrixWorkspace_sptr beh = API::WorkspaceFactory::Instance().create(
    "Workspace2D", static_cast<int>(m_GroupSpecInds.size()), 1, 1);

  g_log.debug() << name() << ": Preparing to group spectra into " << m_GroupSpecInds.size() << " groups\n";


  // where we are copying spectra to, we start copying to the start of the output workspace
  size_t outIndex = 0;
  // Only used for averaging behaviour. We may have a 1:1 map where a Divide would be waste as it would be just dividing by 1
  bool requireDivide(false);
  for ( storage_map::const_iterator it = m_GroupSpecInds.begin(); it != m_GroupSpecInds.end() ; ++it )
  {
    // This is the grouped spectrum
    EventList & outEL = outputWS->getEventList(outIndex);

    // The spectrum number of the group is the key
    outEL.setSpectrumNo(it->first);
    // Start fresh with no detector IDs
    outEL.clearDetectorIDs();

    // the Y values and errors from spectra being grouped are combined in the output spectrum
    // Keep track of number of detectors required for masking
    size_t nonMaskedSpectra(0);
    beh->dataX(outIndex)[0] = 0.0;
    beh->dataE(outIndex)[0] = 0.0;
    for( std::vector<size_t>::const_iterator wsIter = it->second.begin(); wsIter != it->second.end(); ++wsIter)
    {
      const size_t originalWI = *wsIter;

      const EventList & fromEL=inputWS->getEventList(originalWI);
      //Add the event lists with the operator
      outEL += fromEL;


      // detectors to add to the output spectrum
      outEL.addDetectorIDs(fromEL.getDetectorIDs() );
      try
      {
        Geometry::IDetector_const_sptr det = inputWS->getDetector(originalWI);
        if( !det->isMasked() ) ++nonMaskedSpectra;
      }
      catch(Exception::NotFoundError&)
      {
        // If a detector cannot be found, it cannot be masked
        ++nonMaskedSpectra;
      }
    }
    if( nonMaskedSpectra == 0 ) ++nonMaskedSpectra; // Avoid possible divide by zero
    if(!requireDivide) requireDivide = (nonMaskedSpectra > 1);
    beh->dataY(outIndex)[0] = static_cast<double>(nonMaskedSpectra);

    // make regular progress reports and check for cancelling the algorithm
    if ( outIndex % INTERVAL == 0 )
    {
      m_FracCompl += INTERVAL*prog4Copy;
      if ( m_FracCompl > 1.0 )
        m_FracCompl = 1.0;
      progress(m_FracCompl);
      interruption_point();
    }
    outIndex ++;
  }


  if ( bhv == 1 && requireDivide )
  {
    g_log.debug() << "Running Divide algorithm to perform averaging.\n";
    Mantid::API::IAlgorithm_sptr divide = createChildAlgorithm("Divide");
    divide->initialize();
    divide->setProperty<API::MatrixWorkspace_sptr>("LHSWorkspace", outputWS);
    divide->setProperty<API::MatrixWorkspace_sptr>("RHSWorkspace", beh);
    divide->setProperty<API::MatrixWorkspace_sptr>("OutputWorkspace", outputWS);
    divide->execute();
  }


  g_log.debug() << name() << " created " << outIndex << " new grouped spectra\n";
  return outIndex;
}


/**
*  Only to be used if the KeepUnGrouped property is true, moves the spectra that were not selected
*  to be in a group to the end of the output spectrum
*  @param unGroupedSet :: list of WORKSPACE indexes that were included in a group
*  @param inputWS :: user selected input workspace for the algorithm
*  @param outputWS :: user selected output workspace for the algorithm
*  @param outIndex :: the next spectra index available after the grouped spectra
*/
void GroupDetectors2::moveOthers(const std::set<int64_t> &unGroupedSet, API::MatrixWorkspace_const_sptr inputWS, API::MatrixWorkspace_sptr outputWS,
         size_t outIndex)
{
  g_log.debug() << "Starting to copy the ungrouped spectra" << std::endl;
  double prog4Copy = (1. - 1.*static_cast<double>(m_FracCompl))/static_cast<double>(unGroupedSet.size());

  std::set<int64_t>::const_iterator copyFrIt = unGroupedSet.begin();
  // go thorugh all the spectra in the input workspace
  for ( ; copyFrIt != unGroupedSet.end(); ++copyFrIt )
  {
    if( *copyFrIt == USED ) continue; //Marked as not to be used
    size_t sourceIndex = static_cast<size_t>(*copyFrIt);

    // The input spectrum we'll copy
    const ISpectrum * inputSpec = inputWS->getSpectrum(sourceIndex);

    // Destination of the copying
    ISpectrum * outputSpec = outputWS->getSpectrum(outIndex);

    // Copy the data
    outputSpec->dataX() = inputSpec->dataX();
    outputSpec->dataY() = inputSpec->dataY();
    outputSpec->dataE() = inputSpec->dataE();

    // Spectrum numbers etc.
    outputSpec->setSpectrumNo(inputSpec->getSpectrumNo());
    outputSpec->clearDetectorIDs();
    outputSpec->addDetectorIDs( inputSpec->getDetectorIDs() );

    // go to the next free index in the output workspace
    outIndex ++;
    // make regular progress reports and check for cancelling the algorithm
    if ( outIndex % INTERVAL == 0 )
    {
      m_FracCompl += INTERVAL*prog4Copy;
      if ( m_FracCompl > 1.0 )
      {
        m_FracCompl = 1.0;
      }
      progress(m_FracCompl);
      interruption_point();
    }
  }

  g_log.debug() << name() << " copied " << unGroupedSet.size()-1 << " ungrouped spectra\n";
}


/**
*  Only to be used if the KeepUnGrouped property is true, moves the spectra that were not selected
*  to be in a group to the end of the output spectrum
*  @param unGroupedSet :: list of WORKSPACE indexes that were included in a group
*  @param inputWS :: user selected input workspace for the algorithm
*  @param outputWS :: user selected output workspace for the algorithm
*  @param outIndex :: the next spectra index available after the grouped spectra
*/
void GroupDetectors2::moveOthersEvent(const std::set<int64_t> &unGroupedSet, DataObjects::EventWorkspace_const_sptr inputWS,
                                      DataObjects::EventWorkspace_sptr outputWS,size_t outIndex)
{
  g_log.debug() << "Starting to copy the ungrouped spectra" << std::endl;
  double prog4Copy = (1. - 1.*static_cast<double>(m_FracCompl))/static_cast<double>(unGroupedSet.size());

  std::set<int64_t>::const_iterator copyFrIt = unGroupedSet.begin();
  // go thorugh all the spectra in the input workspace
  for ( ; copyFrIt != unGroupedSet.end(); ++copyFrIt )
  {
    if( *copyFrIt == USED ) continue; //Marked as not to be used
    size_t sourceIndex = static_cast<size_t>(*copyFrIt);

    // The input spectrum we'll copy
    const EventList & inputSpec = inputWS->getEventList(sourceIndex);

    // Destination of the copying
    EventList & outputSpec=outputWS->getEventList(outIndex);

    // Copy the data
    outputSpec+=inputSpec;

    // Spectrum numbers etc.
    outputSpec.setSpectrumNo(inputSpec.getSpectrumNo());
    outputSpec.clearDetectorIDs();
    outputSpec.addDetectorIDs( inputSpec.getDetectorIDs() );

    // go to the next free index in the output workspace
    outIndex ++;
    // make regular progress reports and check for cancelling the algorithm
    if ( outIndex % INTERVAL == 0 )
    {
      m_FracCompl += INTERVAL*prog4Copy;
      if ( m_FracCompl > 1.0 )
      {
        m_FracCompl = 1.0;
      }
      progress(m_FracCompl);
      interruption_point();
    }
  }

  g_log.debug() << name() << " copied " << unGroupedSet.size()-1 << " ungrouped spectra\n";
}

} // namespace DataHandling
} // namespace Mantid
