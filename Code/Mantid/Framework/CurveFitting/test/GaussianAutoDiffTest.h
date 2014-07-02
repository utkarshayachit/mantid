#ifndef MANTID_CURVEFITTING_GAUSSIANAUTODIFFTEST_H_
#define MANTID_CURVEFITTING_GAUSSIANAUTODIFFTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidCurveFitting/GaussianAutoDiff.h"

using Mantid::CurveFitting::GaussianAutoDiff;
using namespace Mantid::API;

class GaussianAutoDiffTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static GaussianAutoDiffTest *createSuite() { return new GaussianAutoDiffTest(); }
  static void destroySuite( GaussianAutoDiffTest *suite ) { delete suite; }


  void test_Something()
  {
    TSM_ASSERT( "You forgot to write a test!", 0);
  }


};


#endif /* MANTID_CURVEFITTING_GAUSSIANAUTODIFFTEST_H_ */