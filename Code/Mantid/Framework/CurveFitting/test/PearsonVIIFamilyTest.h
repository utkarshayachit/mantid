#ifndef MANTID_CURVEFITTING_PEARSONVIIFAMILYTEST_H_
#define MANTID_CURVEFITTING_PEARSONVIIFAMILYTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidCurveFitting/PearsonVIIFamily.h"

using Mantid::CurveFitting::PearsonVIIFamily;
using namespace Mantid::API;

class PearsonVIIFamilyTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static PearsonVIIFamilyTest *createSuite() { return new PearsonVIIFamilyTest(); }
  static void destroySuite( PearsonVIIFamilyTest *suite ) { delete suite; }


  void test_Something()
  {
    TSM_ASSERT( "You forgot to write a test!", 0);
  }


};


#endif /* MANTID_CURVEFITTING_PEARSONVIIFAMILYTEST_H_ */