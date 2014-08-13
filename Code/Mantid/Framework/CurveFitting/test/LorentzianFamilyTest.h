#ifndef MANTID_CURVEFITTING_LORENTZIANFAMILYTEST_H_
#define MANTID_CURVEFITTING_LORENTZIANFAMILYTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidCurveFitting/LorentzianFamily.h"

using Mantid::CurveFitting::LorentzianFamily;
using namespace Mantid::API;

class LorentzianFamilyTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static LorentzianFamilyTest *createSuite() { return new LorentzianFamilyTest(); }
  static void destroySuite( LorentzianFamilyTest *suite ) { delete suite; }


  void test_Something()
  {
    TSM_ASSERT( "You forgot to write a test!", 0);
  }


};


#endif /* MANTID_CURVEFITTING_LORENTZIANFAMILYTEST_H_ */