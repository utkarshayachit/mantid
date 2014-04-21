/*WIKI*

 Develop time: P0 (45min) +


== Divide splitter by time ==
* Required:
 1. WorkspaceGroup (wg)
 2. Number of time slots (N)
 3. Index of time slot (i)
* Algorithm:
 For each splitter (t_i^(s), t_i^(e)) corresponding to workspace group, wg,
 a new splitter will be generated as
 T_i^(s) = t_i^(s) + deltaT \cdot i
 T_i^(e) = t_i^(s) + deltaT \cdot (i+1)
 where
 deltaT = (t_i^(e) - t_i^(s))/N


 *WIKI*/

#include "MantidAlgorithms/ChopEventFilters.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/WorkspaceFactory.h"

namespace Mantid
{
namespace Algorithms
{
  using namespace Mantid::API;
  using namespace Mantid::Kernel;

  using namespace std;

  const double TOL = 1.0E-5;

  DECLARE_ALGORITHM(ChopEventFilters)


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  ChopEventFilters::ChopEventFilters()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  ChopEventFilters::~ChopEventFilters()
  {
  }

  /** Wiki documentation
   */
  void ChopEventFilters::initDocs()
  {
    setWikiSummary("Chop the event filters for a specified workspace group to even time slots "
                   "and output one slot to a new workspace. ");
  }

  /** Declare properties
   */
  void ChopEventFilters::init()
  {
    declareProperty(new WorkspaceProperty<MatrixWorkspace>("InputWorkspace", "", Direction::Input),
                    "Name of input event filter workspace to be processed.");

    declareProperty(new WorkspaceProperty<MatrixWorkspace>("OutputWorkspace", "", Direction::Output),
                    "Name of the output event filter workspace as a result of processing.");

    declareProperty("WorkspaceGroup", EMPTY_INT(), "Workspace group of the evnet filers to be processed.");

    // FIXME - Need a validator for integer range
    declareProperty("NumberOfSlots", 2, "Number of time slots for an individual filter to split to. ");

    // FIXME - Need a validator for slot index
    declareProperty("IndexOfSlot", EMPTY_INT(), "Index of the time slot to be kept.");

  }

  /** Main execution
   */
  void ChopEventFilters::exec()
  {
    // Process inputs
    processAlgProperties();

    // Calculating splitted
    chopEventFilterByTime();

    // Finale
    if (m_outputWS) setProperty("OutputWorkspace", m_outputWS);
    else throw std::runtime_error("Output workspace is a null pointer.");

    return;
  }

  /** Process input property
   */
  void ChopEventFilters::processAlgProperties()
  {
    m_inputWS = getProperty("InputWorkspace");

    m_wsGroup = getProperty("WorkspaceGroup");
    if (isEmpty(m_wsGroup))
      throw std::runtime_error("Workspace must be given.");

    m_numSlots = getProperty("NumberOfSlots");

    m_slotIndex = getProperty("IndexOfSlot");
    if (m_slotIndex >= m_numSlots)
      throw std::runtime_error("Slot index is out of boundary as number of slots.");

    return;
  }

  /** Chop the selected splitters by time
   */
  void ChopEventFilters::chopEventFilterByTime()
  {
    double targetWSgroup = static_cast<double> (m_wsGroup);

    const MantidVec& vecX = m_inputWS->readX(0);
    const MantidVec& vecY = m_inputWS->readY(0);
    if (vecY.size() == vecX.size())
    {
      throw runtime_error("Input workspace is wrong at x and y's sizes.");
    }

    // Find out splitters with matched workspace group
    double invertN = 1./static_cast<double> (m_numSlots);
    double slotindex_d = static_cast<double> (m_slotIndex);
    size_t numfilters = vecY.size();
    for (size_t i = 0; i < numfilters; ++i)
    {
      double tmpgroup = vecY[i];
      if (fabs(tmpgroup - targetWSgroup) < TOL)
      {
        // Found a group
        double abs_start = vecX[i];
        double abs_end = vecX[i+1];
        double deltaT = (abs_end - abs_start)*invertN;

        // Set up new start time and end time
        double newstart = abs_start + slotindex_d * deltaT;
        double newend = newstart + deltaT;

        // Append to output vector
        if (m_outX.size() == 0)
        {
          // First entry
          m_outX.push_back(newstart);
          m_outX.push_back(newend);
          m_outY.push_back(targetWSgroup);
        }
        else
        {
          // Appending to previous
          if (newstart - m_outX.back() > TOL)
          {
            // This splitter is not adjacent
            m_outX.push_back(newstart);
            m_outX.push_back(newend);
            m_outY.push_back(-1);
            m_outY.push_back(targetWSgroup);
          }
          else
          {
            // New splitter is same and adjacent to previous one.
            m_outX.back() = newend;
            g_log.warning("It is not likely to happen that new splitter is continous to previous one.");
          }
        }

      }

    } /// END(for i)

    // Check
    if (m_outX.size() == 0)
    {
      std::stringstream errss;
      errss << "Input workspace does not have any splitters with target workspace group "
            << m_wsGroup << ", as user specified. ";
      throw runtime_error(errss.str());
    }

    // Build output workspace group
    size_t nspec = 1;
    size_t sizey = m_outY.size();
    m_outputWS = boost::dynamic_pointer_cast<MatrixWorkspace>(WorkspaceFactory::Instance().create("Workspace2D", nspec, sizey+1, sizey));
    MantidVec& dataX = m_outputWS->dataX(0);
    MantidVec& dataY = m_outputWS->dataY(0);
    for (size_t i = 0; i < sizey; ++i)
    {
      dataX[i] = m_outX[i];
      dataY[i] = m_outY[i];
    }
    dataX[sizey] = m_outX[sizey];

    return;
  }

  


} // namespace Algorithms
} // namespace Mantid
