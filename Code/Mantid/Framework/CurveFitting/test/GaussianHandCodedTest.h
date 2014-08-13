#ifndef MANTID_CURVEFITTING_GAUSSIANHANDCODEDTEST_H_
#define MANTID_CURVEFITTING_GAUSSIANHANDCODEDTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidCurveFitting/GaussianHandCoded.h"

using Mantid::CurveFitting::GaussianHandCoded;
using namespace Mantid::API;

class GaussianHandCodedTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static GaussianHandCodedTest *createSuite() { return new GaussianHandCodedTest(); }
  static void destroySuite( GaussianHandCodedTest *suite ) { delete suite; }


  void test_Something()
  {
    TSM_ASSERT( "You forgot to write a test!", 0);
  }


};


#endif /* MANTID_CURVEFITTING_GAUSSIANHANDCODEDTEST_H_ */