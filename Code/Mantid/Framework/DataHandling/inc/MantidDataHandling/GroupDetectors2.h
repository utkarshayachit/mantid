#ifndef MANTID_DATAHANDLING_GROUPDETECTORS2_H_
#define MANTID_DATAHANDLING_GROUPDETECTORS2_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidKernel/System.h"
#include <climits>
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidDataObjects/GroupingWorkspace.h"
#include <map>
#ifdef HAS_UNORDERED_MAP_H
#include <tr1/unordered_map>
#endif

#include <Poco/SAX/ContentHandler.h>

namespace Mantid
{
namespace DataHandling
{
/** An algorithm for grouping detectors and their associated spectra into
    single spectra and DetectorGroups.

    See LoadDetectorsGroupingFile for a MapFile format.

    @author Steve Williams and Russell Taylor (Tessella Support Services plc)
    @date 27/07/2009

    Copyright &copy; 2008-11 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://github.com/mantidproject/mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport GroupDetectors2 : public API::Algorithm
{
public:
  GroupDetectors2();
  virtual ~GroupDetectors2();

  /// Algorithm's name for identification overriding a virtual method
  virtual const std::string name() const { return "GroupDetectors"; };
  /// Algorithm's version for identification overriding a virtual method
  virtual int version() const { return 2; };
  /// Algorithm's category for identification overriding a virtual method
  virtual const std::string category() const{return "Transforms\\Grouping";}

private:
  /// Sets documentation strings for this algorithm
  virtual void initDocs();


#ifndef HAS_UNORDERED_MAP_H
/// used to store the lists of WORKSPACE INDICES that will be grouped, the keys are not used
typedef std::map<specid_t, std::vector<size_t> > storage_map;
#else
typedef std::tr1::unordered_map<specid_t, std::vector<size_t> > storage_map;
#endif

  /// An estimate of the percentage of the algorithm runtimes that has been completed 
  double m_FracCompl;
  /// stores lists of spectra indexes to group, although we never do an index search on it
  storage_map m_GroupSpecInds;


  // Implement abstract Algorithm methods
  void init();
  void exec();
  void execEvent();
  
  /// read in the input parameters and see what findout what will be to grouped
  void getGroups(API::MatrixWorkspace_const_sptr workspace, std::vector<int64_t> &unUsedSpec);

  /// Converts GroupingWorkspace to data structures used to form groups
  void getGroupsFromWS(DataObjects::GroupingWorkspace_const_sptr groupingWS, std::vector<int64_t> &unUsedSpec);

  /// Copy the and combine the histograms that the user requested from the input into the output workspace
  size_t formGroups(API::MatrixWorkspace_const_sptr inputWS, API::MatrixWorkspace_sptr outputWS, 
        const double prog4Copy);
  /// Copy the and combine the event lists that the user requested from the input into the output workspace
  size_t formGroupsEvent(DataObjects::EventWorkspace_const_sptr inputWS, DataObjects::EventWorkspace_sptr outputWS,
        const double prog4Copy);

  /// Copy the data data in ungrouped histograms from the input workspace to the output
  void moveOthers(const std::set<int64_t> &unGroupedSet, API::MatrixWorkspace_const_sptr inputWS, API::MatrixWorkspace_sptr outputWS, size_t outIndex);
  /// Copy the data data in ungrouped event lists from the input workspace to the output
  void moveOthersEvent(const std::set<int64_t> &unGroupedSet, DataObjects::EventWorkspace_const_sptr inputWS, DataObjects::EventWorkspace_sptr outputWS, size_t outIndex);

  /// flag values
  enum {
    USED = 1000-INT_MAX                                 ///< goes in the unGrouped spectra list to say that a spectrum will be included in a group, any other value and it isn't. Spectra numbers should always be positive so we shouldn't accidientally set a spectrum number to the this
  };
  
  static const double CHECKBINS;                         ///< a (worse case) estimate of the time required to check that the X bin boundaries are the same as a percentage of total algorithm run time
  static const double READFILE;                          ///< if a file must be read in estimate that reading it will take this percentage of the algorithm execution time
  static const int INTERVAL = 128;                       ///< copy this many histograms and then check for an algorithm notification and update the progress bar

};

} // namespace DataHandling
} // namespace Mantid

#endif /*MANTID_DATAHANDLING_GROUPDETECTORS2_H_*/
