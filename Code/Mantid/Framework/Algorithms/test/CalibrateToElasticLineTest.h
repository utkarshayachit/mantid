#ifndef MANTID_ALGORITHMS_CALIBRATETOELASTICLINETEST_H_
#define MANTID_ALGORITHMS_CALIBRATETOELASTICLINETEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/Workspace.h"
#include "MantidAlgorithms/CalibrateToElasticLine.h"

using Mantid::Algorithms::CalibrateToElasticLine;

class CalibrateToElasticLineTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static CalibrateToElasticLineTest *createSuite() { return new CalibrateToElasticLineTest(); }
  static void destroySuite( CalibrateToElasticLineTest *suite ) { delete suite; }


  void test_Init()
  {
    CalibrateToElasticLine alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
  }

  void test_exec()
  {
    // Name of the output workspace.
    std::string outWSName("CalibrateToElasticLineTest_OutputWS");
  
    CalibrateToElasticLine alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
//    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("InputWorkspace", ) );
//    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("OutputWorkspace", outWSName) );
//    TS_ASSERT_THROWS_NOTHING( alg.execute(); );
//    TS_ASSERT( alg.isExecuted() );
    
    // Retrieve the workspace from data service. TODO: Change to your desired type
//    Mantid::API::Workspace_sptr ws;
//    TS_ASSERT_THROWS_NOTHING( ws = Mantid::API::AnalysisDataService::Instance().retrieveWS<Mantid::API::Workspace>(outWSName) );
//    TS_ASSERT(ws);
//    if (!ws) return;
    
    // TODO: Check the results
    
    // Remove workspace from the data service.
//    Mantid::API::AnalysisDataService::Instance().remove(outWSName);
  }

  void xtest_Something()
  {
    TSM_ASSERT( "You forgot to write a test!", 0);
  }


};


#endif /* MANTID_ALGORITHMS_CALIBRATETOELASTICLINETEST_H_ */
