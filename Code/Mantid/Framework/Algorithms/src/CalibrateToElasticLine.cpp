/*WIKI*
TODO: Enter a full wiki-markup description of your algorithm here. You can then use the Build/wiki_maker.py script to generate your full wiki page.
*WIKI*/

#include <fstream>
#include <iostream>

#include "MantidAlgorithms/CalibrateToElasticLine.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/FileProperty.h"
#include "MantidKernel/FileValidator.h"
#include "MantidGeometry/Instrument/ComponentHelper.h"
#include "MantidGeometry/Instrument/ParameterMap.h"


namespace Mantid
{
namespace Algorithms
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(CalibrateToElasticLine)
  


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  CalibrateToElasticLine::CalibrateToElasticLine()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  CalibrateToElasticLine::~CalibrateToElasticLine()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Algorithm's name for identification. @see Algorithm::name
  const std::string CalibrateToElasticLine::name() const { return "CalibrateToElasticLine";};
  
  /// Algorithm's version for identification. @see Algorithm::version
  int CalibrateToElasticLine::version() const { return 1;};
  
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string CalibrateToElasticLine::category() const { return "Calibration";}

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void CalibrateToElasticLine::initDocs()
  {
    this->setWikiSummary("TODO: Enter a quick description of your algorithm.");
    this->setOptionalMessage("TODO: Enter a quick description of your algorithm.");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void CalibrateToElasticLine::init()
  {

      this->declareProperty(new API::WorkspaceProperty<API::MatrixWorkspace>("InputWorkspace","",Kernel::Direction::Input,
                                                                             boost::make_shared<API::WorkspaceUnitValidator>("TOF")),
                            "Name of the input workspace.");
      this->declareProperty(new API::WorkspaceProperty<API::MatrixWorkspace>("OutputWorkspace","",Kernel::Direction::Output),
                            "Name of the output workspace, can be the same as the input.");

      this->declareProperty(new API::FileProperty("ElasticPositionsFilename", "", API::FileProperty::OptionalLoad, ".dat"),
                            "Path to a file containing the elastic line positions in tof");
  }

  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm
   */
  void CalibrateToElasticLine::exec()
  {
    // Get the workspaces
      API::MatrixWorkspace_const_sptr inputWS =  this->getProperty("InputWorkspace");
      API::MatrixWorkspace_sptr outputWS = this->getProperty("OutputWorkspace");

      // Get the filename of the elastic positions file
      std::string filename = this->getProperty("ElasticPositionsFilename");

      // Lets load the file
      g_log.debug() << "Reading elastic estimate data from " << filename << std::endl;
      std::ifstream infile(filename);
      std::string lineData;

      // Some vectors to store the contents in
      std::vector<int> detectorId;
      std::vector<double> peakCentre;
      //std::vector<double> peakHeight;
      //std::vector<int> numberOfEvents;
      //std::vector<int> bankNumber;

      const int headers_to_skip = 1;      // lines to skip on reading
      const int max_num_of_char_in_a_line = 512;// Maximum number of characters expected in a single line in the header
      for (int i = 0; i < headers_to_skip; ++i) {
          infile.ignore(max_num_of_char_in_a_line,'\n');
      }

      while(getline(infile, lineData))
      {
          // Some temporary variables
          int id;
          double centre;
          // maybe not need the next variables
          //int events, bank;
          //double height;

          std::stringstream lineStream(lineData);

          lineStream >> id;
          lineStream >> centre;

          //infile >> id >> centre; // >> height >> events >> bank;

          //std::cout << "ID=" << id << ", TOF=" << centre << std::endl;

          detectorId.push_back(id);
          peakCentre.push_back(centre);
          //peakHeight.push_back(height);
          //numberOfEvents.push_back(events);
          //bankNumber.push_back(bank);

      }

      for(std::vector<int>::size_type j = 0; j < 10; ++j)
      {
          g_log.warning() << detectorId[j] << "\n";
      }

//              for(vector<float>::size_type j = 0; j < volt2_array.size(); ++j)
//              cout << volt2_array[j] << '\n';


      // Check if the input and output workspaces are the same
      if (outputWS != inputWS)
      {
          // If not the same, create a new workspace for the output
          outputWS = API::WorkspaceFactory::Instance().create(inputWS);
      }


      Geometry::Instrument_const_sptr inst = outputWS->getInstrument();

      Geometry::ParameterMap& pmap = outputWS->instrumentParameters();

      // Get the number of spectra
      const std::size_t numberOfChannels = inputWS->blocksize();
      const int numberOfSpectra = static_cast<int>(inputWS->size() / numberOfChannels);


      // Get L1
      Geometry::IObjComponent_const_sptr source = inst->getSource();
      Geometry::IObjComponent_const_sptr sample = inst->getSample();
      if (source == NULL)
      {
          throw Kernel::Exception::InstrumentDefinitionError("Cannot determine source.");
      }
      if (sample == NULL)
      {
          throw Kernel::Exception::InstrumentDefinitionError("Cannot determine sample.");
      }
      double l1 = 0.0;
      try
      {
          l1 = source->getDistance(*sample);
          g_log.debug() << "Incident Flight Path (L1) = " << l1 << std::endl;
      }
      catch (Kernel::Exception::NotFoundError &)
      {
          g_log.error() << "Cannot calculate incident flight path (L1).";
          throw Kernel::Exception::InstrumentDefinitionError("Cannot calculate incident flight path (L1)", outputWS->getTitle());
      }

      // TODO: Initialise the progress bar
      // API::Progress prog(this, 0.0, 1.0, numberOfSpectra);

      // Loop over the spectra
//      for (int i = 0; i < numberOfSpectra; ++i)
      for (std::vector<int>::size_type i=0; i < detectorId.size(); ++i)
      //for (int i = 0; i < 10; ++i)
      {
          Geometry::IDetector_const_sptr det = outputWS->getDetectorByID(detectorId[i]);
          if (!det->isMonitor())
          {
              // Get secondary flight path (L2)
              double l2 = det->getDistance(*sample);
              // Get scattering angle (radians)
              double twoTheta = det->getTwoTheta(sample->getPos(),
                                          Kernel::V3D(0.0,0.0,1.0)*Geometry::rad2deg);

              // Now let's work out what energy is required for this TOF
              double ltotal = l1+l2;
              double factor = ((PhysicalConstants::NeutronMass / 2.0) * (ltotal*ltotal))
                      / (PhysicalConstants::meV * 1e-12);
              double energy = factor / (peakCentre[i]*peakCentre[i]);

              g_log.warning() << "det(" << i << ") = " << det->getFullName() << std::endl;
              g_log.warning() << "      Ef = " << det->getNumberParameter("Efixed").at(0) << std::endl;

              pmap.addDouble(det.get(), "Efixed", energy);

              g_log.warning() << "      Ef = " << det->getNumberParameter("Efixed").at(0) << std::endl;


          }
      }

  }

} // namespace Algorithms
} // namespace Mantid
