#ifndef MANTID_API_IFUNCTIONAUTODIFFTEST_H_
#define MANTID_API_IFUNCTIONAUTODIFFTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAPI/IFunctionAutoDiff.h"

using Mantid::API::IFunctionAutoDiff;
using namespace Mantid::API;

class IFunctionAutoDiffTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static IFunctionAutoDiffTest *createSuite() { return new IFunctionAutoDiffTest(); }
  static void destroySuite( IFunctionAutoDiffTest *suite ) { delete suite; }


  void test_Something()
  {
    TSM_ASSERT( "You forgot to write a test!", 0);
  }


};


#endif /* MANTID_API_IFUNCTIONAUTODIFFTEST_H_ */