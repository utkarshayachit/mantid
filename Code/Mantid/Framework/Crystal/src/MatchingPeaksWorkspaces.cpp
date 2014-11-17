#include "MantidCrystal/MatchingPeaksWorkspaces.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidDataObjects/PeaksWorkspace.h"

namespace Mantid
{
namespace Crystal
{
  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(MatchingPeaksWorkspaces)

  using namespace Kernel;
  using namespace API;
  using DataObjects::PeaksWorkspace;
  using DataObjects::PeaksWorkspace_const_sptr;
  using DataObjects::PeaksWorkspace_sptr;
  using DataObjects::Peak;

  /** Constructor
   */
  MatchingPeaksWorkspaces::MatchingPeaksWorkspaces()
  {
  }

  /** Destructor
   */
  MatchingPeaksWorkspaces::~MatchingPeaksWorkspaces()
  {
  }

  /// Algorithm's name for identification. @see Algorithm::name
  const std::string MatchingPeaksWorkspaces::name() const { return "MatchingPeaksWorkspaces"; };
  /// Algorithm's version for identification. @see Algorithm::version
  int MatchingPeaksWorkspaces::version() const { return 1; };
  /// Algorithm's category for identification. @see Algorithm::category
  const std::string MatchingPeaksWorkspaces::category() const { return "Crystal"; }

  /** Initialises the algorithm's properties.
   */
  void MatchingPeaksWorkspaces::init()
  {
    declareProperty(new WorkspaceProperty<PeaksWorkspace>("LHSWorkspace","",Direction::Input),
        "The first of peaks.");
    declareProperty(new WorkspaceProperty<PeaksWorkspace>("RHSWorkspace","",Direction::Input),
        "The second set of peaks.");
    declareProperty(new WorkspaceProperty<PeaksWorkspace>("OutputWorkspace","",Direction::Output),
        "The set of peaks that are in the first, but not the second, workspace.");

    auto mustBePositive = boost::make_shared<BoundedValidator<double> >();
    mustBePositive->setLower(0.0);
    // N.B. Andrei reckons it should be delta_q/q
    declareProperty("Tolerance", 0.0, mustBePositive,
        "Maximum difference in each component of Q for which peaks are considered identical");
  }

  /** Executes the algorithm.
   */
  void MatchingPeaksWorkspaces::exec()
  {
    PeaksWorkspace_const_sptr LHSWorkspace = getProperty("LHSWorkspace");
    PeaksWorkspace_const_sptr RHSWorkspace = getProperty("RHSWorkspace");
    const double Tolerance = getProperty("Tolerance");

    // Warn if not the same instrument, sample
    if ( LHSWorkspace->getInstrument()->getName() != RHSWorkspace->getInstrument()->getName() )
    {
      g_log.warning("The two input workspaces do not appear to come from data take on the same instrument");
    }
    if ( LHSWorkspace->sample().getName() != RHSWorkspace->sample().getName() )
    {
      g_log.warning("The two input workspaces do not appear to relate to the same sample");
    }

    // Copy the first workspace to our output workspace
    PeaksWorkspace_sptr output(LHSWorkspace->clone());
    // Get hold of the peaks in the second workspace
    auto & rhsPeaks = RHSWorkspace->getPeaks();
    // Get hold of the peaks in the first workspace as we'll need to examine them
    auto & lhsPeaks = output->getPeaks();

    Progress progress(this, 0, 1, rhsPeaks.size());

    // Loop over the peaks in the first workspace, searching for a match in the first
    for ( size_t i = 0; i <lhsPeaks.size(); ++i )
    {
      const Peak& currentPeak = lhsPeaks[i];
      bool match = false;
      // Now have to go through the second workspace checking for matches
      // Not doing anything clever as peaks workspace are typically not large - just a linear search
      for ( int j = 0; j < RHSWorkspace->getNumberPeaks(); ++j )
      {
        const V3D deltaQ = currentPeak.getQSampleFrame() - rhsPeaks[j].getQSampleFrame();
        if ( deltaQ.nullVector(Tolerance) )  // Using a V3D method that does the job
        {
          // As soon as we find a match, remove it from the output and move onto the next rhs peak
          match = true;
          break;
        }
      }

      if(!match) output->removePeak(static_cast<int>(i));
      progress.report();
    }

    setProperty("OutputWorkspace", output);
  }

} // namespace Crystal
} // namespace Mantid
