#ifndef MANTID_CURVEFITTING_GAUSSIANNUMDIFFTEST_H_
#define MANTID_CURVEFITTING_GAUSSIANNUMDIFFTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidCurveFitting/GaussianNumDiff.h"

using Mantid::CurveFitting::GaussianNumDiff;
using namespace Mantid::API;

class GaussianNumDiffTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static GaussianNumDiffTest *createSuite() { return new GaussianNumDiffTest(); }
  static void destroySuite( GaussianNumDiffTest *suite ) { delete suite; }


  void test_Something()
  {
    TSM_ASSERT( "You forgot to write a test!", 0);
  }


};


#endif /* MANTID_CURVEFITTING_GAUSSIANNUMDIFFTEST_H_ */