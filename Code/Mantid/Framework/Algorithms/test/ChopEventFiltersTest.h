#ifndef MANTID_ALGORITHMS_CHOPEVENTFILTERS_H_
#define MANTID_ALGORITHMS_CHOPEVENTFILTERS_H_

#include <cxxtest/TestSuite.h>

#include "MantidAlgorithms/ChopEventFilters.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidDataObjects/Workspace2D.h"

using Mantid::Algorithms::ChopEventFilters;

using namespace Mantid;
using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::DataObjects;

using namespace std;

class ChopEventFiltersTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static ChopEventFiltersTest *createSuite()
  {
    API::FrameworkManager::Instance();
    return new ChopEventFiltersTest();
  }
  static void destroySuite( ChopEventFiltersTest *suite ) { delete suite; }

  //----------------------------------------------------------------------------------------------
  /** Test on init and setup
    */
  void test_Init()
  {
    // Generate input workspace
    MatrixWorkspace_sptr splitws = gen_Splitters();
    AnalysisDataService::Instance().addOrReplace("SplitterWS", splitws);

    // Initialize ChopEventFilters
    ChopEventFilters ChopEventFilters;

    ChopEventFilters.initialize();
    TS_ASSERT(ChopEventFilters.isInitialized());

    // Set properties
    TS_ASSERT_THROWS_NOTHING(ChopEventFilters.setPropertyValue("InputWorkspace", "SplitterWS"));
    TS_ASSERT_THROWS_NOTHING(ChopEventFilters.setPropertyValue("OutputWorkspace", "Splitter2WS"));
    TS_ASSERT_THROWS_NOTHING(ChopEventFilters.setPropertyValue("WorkspaceGroup", "2"));
    TS_ASSERT_THROWS_NOTHING(ChopEventFilters.setProperty("NumberOfSplots", 10));
    TS_ASSERT_THROWS_NOTHING(ChopEventFilters.setProperty("IndexOfSlot", "3"));

    // Clean
    AnalysisDataService::Instance().remove("SplitterWS");

    return;
  }

  //----------------------------------------------------------------------------------------------
  /** Generate a workspace contains PG3_4866 5-th peak
    */
  MatrixWorkspace_sptr gen_PG3DiamondData()
  {
    vector<double> vecx, vecy, vece;

    vecx.push_back(10000);    vecy.push_back(1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(20000);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(30000);    vecy.push_back(2.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(35000);    vecy.push_back(-1.000000 ); vece.push_back(1.41421E+00);
    vecx.push_back(50000);    vecy.push_back(0.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(65000);    vecy.push_back(-1.000000 ); vece.push_back(1.00000E+00);
    vecx.push_back(70000);    vecy.push_back(2.000000 ); vece.push_back(1.41421E+00);
    vecx.push_back(80000);    vecy.push_back(3.000000 ); vece.push_back(1.41421E+00);
    vecx.push_back(90000);    vecy.push_back(4.000000 ); vece.push_back(1.73205E+00);
    vecx.push_back(95000);    vecy.push_back(-1.000000 ); vece.push_back(2.00000E+00);
    vecx.push_back(100000);    vecy.push_back(2.000000 ); vece.push_back(2.23607E+00);
    vecx.push_back(120000);

    size_t NVectors = 1;
    size_t sizex = vecx.size();
    size_t sizey = vecy.size();
    MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(
            WorkspaceFactory::Instance().create("Workspace2D", NVectors, sizex, sizey));

    MantidVec& vecX = ws->dataX(0);
    MantidVec& vecY = ws->dataY(0);
    MantidVec& vecE = ws->dataE(0);

    for (size_t i = 0; i < sizex; ++i)
        vecX[i] = vecx[i];
    for (size_t i = 0; i < sizey; ++i)
    {
        vecY[i] = vecy[i];
        vecE[i] = vece[i];
    }

    return ws;
  }

};


#endif /* MANTID_ALGORITHMS_CHOPEVENTFILTERS_H_ */
