#ifndef MANTID_ICAT_CATALOGHELPERTEST_H_
#define MANTID_ICAT_CATALOGHELPERTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidICat/CatalogHelper.h"

// The catalogs that will be used for testing.
#include "MantidICat/ICat3/GSoapGenerated/ICat3ICATPortBindingProxy.h"
#include "MantidICat/ICat4/GSoapGenerated/ICat4ICATPortBindingProxy.h"

/**
 * Creates multiple different catalogs to validate the helper across different catalog instances.
 * Note: This test should include new catalogs when they are added to Mantid.
 */
class CatalogHelperTest : public CxxTest::TestSuite
{
  void testSessionValidity() {}

  void testFormatDateTime() {}

  void testBytesToString() {}

  void testCastContainerType() {}

  void testSaveDataFiles() {}

  void testSaveDataSets() {}

  void testSaveInvestigations() {}

  void testSaveValueToTableWorkspace() {}

  void testThrowErrorMessage() {}
};

#endif /* MANTID_ICAT_CATALOGHELPERTEST_H_ */
