#ifndef LOADNEXUSPROCESSEDTEST_H_
#define LOADNEXUSPROCESSEDTEST_H_

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidDataHandling/LoadInstrument.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidGeometry/Instrument.h"
#include "MantidDataHandling/LoadNexusProcessed.h"
#include "MantidDataHandling/SaveNexusProcessed.h"
#include "MantidDataHandling/Load.h"
#include "SaveNexusProcessedTest.h"
#include <cxxtest/TestSuite.h>
#include <iostream>
#include <Poco/File.h>
#include <boost/lexical_cast.hpp>
#include "MantidGeometry/IDTypes.h"

using namespace Mantid::Kernel;
using namespace Mantid::DataObjects;
using namespace Mantid::API;
using Mantid::detid_t;

// Note that this suite tests an old version of Nexus processed files that we continue to support.
// LoadRawSaveNxsLoadNxs tests the current version of Nexus processed by loading 
// a newly created Nexus processed file. 
//
// LoadRawSaveNxsLoadNxs should be run when making any changes to LoadNexusProcessed
// in addition to this test.

class LoadNexusProcessedTest: public CxxTest::TestSuite
{
public:
  static LoadNexusProcessedTest *createSuite()
  {
    return new LoadNexusProcessedTest();
  }
  static void destroySuite(LoadNexusProcessedTest *suite)
  {
    delete suite;
  }

  LoadNexusProcessedTest() :
      testFile("GEM38370_Focussed_Legacy.nxs"), output_ws("nxstest")
  {

  }

  ~LoadNexusProcessedTest()
  {
    AnalysisDataService::Instance().clear();
  }

  void testFastMultiPeriodDefault()
  {
    LoadNexusProcessed alg;
    alg.initialize();
    TS_ASSERT(alg.isInitialized());
    const bool bFastMultiPeriod = alg.getProperty("FastMultiPeriod");
    TSM_ASSERT("Should defalt to offering fast multiperiod loading", bFastMultiPeriod);
  }

  void testProcessedFile()
  {

    LoadNexusProcessed alg;

    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());

    alg.setPropertyValue("Filename", testFile);
    alg.setPropertyValue("OutputWorkspace", output_ws);

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    //Test some aspects of the file
    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    MatrixWorkspace_sptr matrix_ws = boost::dynamic_pointer_cast<MatrixWorkspace>(workspace);
    TS_ASSERT( matrix_ws.get());

    // Test proton charge from the sample block
    TS_ASSERT_DELTA(matrix_ws->run().getProtonCharge(), 30.14816, 1e-5);

    doHistoryTest(matrix_ws);

    boost::shared_ptr<const Mantid::Geometry::Instrument> inst = matrix_ws->getInstrument();
    TS_ASSERT_EQUALS(inst->getName(), "GEM");
    TS_ASSERT_EQUALS(inst->getSource()->getPos().Z(), -17);
  }

  void testNexusProcessed_Min_Max()
  {

    LoadNexusProcessed alg;

    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    testFile = "focussed.nxs";
    alg.setPropertyValue("Filename", testFile);
    alg.setPropertyValue("OutputWorkspace", output_ws);
    alg.setPropertyValue("SpectrumMin", "2");
    alg.setPropertyValue("SpectrumMax", "4");

    TS_ASSERT_THROWS_NOTHING(alg.execute());
    TS_ASSERT( alg.isExecuted());

    //Test some aspects of the file
    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    MatrixWorkspace_sptr matrix_ws = boost::dynamic_pointer_cast<MatrixWorkspace>(workspace);
    TS_ASSERT( matrix_ws.get());

    //Testing the number of histograms
    TS_ASSERT_EQUALS(matrix_ws->getNumberHistograms(), 3);
    doHistoryTest(matrix_ws);

    boost::shared_ptr<const Mantid::Geometry::Instrument> inst = matrix_ws->getInstrument();
    TS_ASSERT_EQUALS(inst->getName(), "GEM");
    TS_ASSERT_EQUALS(inst->getSource()->getPos().Z(), -17);

  }

  void testNexusProcessed_List()
  {
    LoadNexusProcessed alg;

    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    testFile = "focussed.nxs";
    alg.setPropertyValue("Filename", testFile);
    alg.setPropertyValue("OutputWorkspace", output_ws);
    alg.setPropertyValue("SpectrumList", "1,2,3,4");

    TS_ASSERT_THROWS_NOTHING(alg.execute());
    TS_ASSERT( alg.isExecuted());

    //Test some aspects of the file
    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    MatrixWorkspace_sptr matrix_ws = boost::dynamic_pointer_cast<MatrixWorkspace>(workspace);
    TS_ASSERT( matrix_ws.get());

    //Testing the number of histograms
    TS_ASSERT_EQUALS(matrix_ws->getNumberHistograms(), 4);

    //Test history
    doHistoryTest(matrix_ws);

    boost::shared_ptr<const Mantid::Geometry::Instrument> inst = matrix_ws->getInstrument();
    TS_ASSERT_EQUALS(inst->getName(), "GEM");
    TS_ASSERT_EQUALS(inst->getSource()->getPos().Z(), -17);

  }

  void testNexusProcessed_Min_Max_List()
  {
    LoadNexusProcessed alg;

    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    testFile = "focussed.nxs";
    alg.setPropertyValue("Filename", testFile);
    alg.setPropertyValue("OutputWorkspace", output_ws);
    alg.setPropertyValue("SpectrumMin", "1");
    alg.setPropertyValue("SpectrumMax", "3");
    alg.setPropertyValue("SpectrumList", "4,5");

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    //Test some aspects of the file
    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    MatrixWorkspace_sptr matrix_ws = boost::dynamic_pointer_cast<MatrixWorkspace>(workspace);
    TS_ASSERT( matrix_ws.get());

    //Testing the number of histograms
    TS_ASSERT_EQUALS(matrix_ws->getNumberHistograms(), 5);

    //Test history
    doHistoryTest(matrix_ws);

    boost::shared_ptr<const Mantid::Geometry::Instrument> inst = matrix_ws->getInstrument();
    TS_ASSERT_EQUALS(inst->getName(), "GEM");
    TS_ASSERT_EQUALS(inst->getSource()->getPos().Z(), -17);

  }

  void testNexusProcessed_Min()
  {
    LoadNexusProcessed alg;

    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    testFile = "focussed.nxs";
    alg.setPropertyValue("Filename", testFile);
    alg.setPropertyValue("OutputWorkspace", output_ws);
    alg.setPropertyValue("SpectrumMin", "4");

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    //Test some aspects of the file
    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    MatrixWorkspace_sptr matrix_ws = boost::dynamic_pointer_cast<MatrixWorkspace>(workspace);
    TS_ASSERT( matrix_ws.get());

    //Testing the number of histograms
    TS_ASSERT_EQUALS(matrix_ws->getNumberHistograms(), 3);

    //Test history
    doHistoryTest(matrix_ws);

    boost::shared_ptr<const Mantid::Geometry::Instrument> inst = matrix_ws->getInstrument();
    TS_ASSERT_EQUALS(inst->getName(), "GEM");
    TS_ASSERT_EQUALS(inst->getSource()->getPos().Z(), -17);

  }

  void testNexusProcessed_Max()
  {
    LoadNexusProcessed alg;

    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    testFile = "focussed.nxs";
    alg.setPropertyValue("Filename", testFile);
    alg.setPropertyValue("OutputWorkspace", output_ws);
    alg.setPropertyValue("SpectrumMax", "3");

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    //Test some aspects of the file
    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    MatrixWorkspace_sptr matrix_ws = boost::dynamic_pointer_cast<MatrixWorkspace>(workspace);
    TS_ASSERT( matrix_ws.get());

    //Testing the number of histograms
    TS_ASSERT_EQUALS(matrix_ws->getNumberHistograms(), 3);

    //Test history
    doHistoryTest(matrix_ws);

    boost::shared_ptr<const Mantid::Geometry::Instrument> inst = matrix_ws->getInstrument();
    TS_ASSERT_EQUALS(inst->getName(), "GEM");
    TS_ASSERT_EQUALS(inst->getSource()->getPos().Z(), -17);

  }

  // Saving and reading masking correctly
  void testMasked()
  {
    LoadNexusProcessed alg;

    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    testFile = "focussed.nxs";
    alg.setPropertyValue("Filename", testFile);
    testFile = alg.getPropertyValue("Filename");

    alg.setPropertyValue("OutputWorkspace", output_ws);

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    //Test some aspects of the file
    MatrixWorkspace_sptr workspace;
    workspace = boost::dynamic_pointer_cast<MatrixWorkspace>(
        AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    for (size_t si = 0; si < workspace->getNumberHistograms(); ++si)
    {
      workspace->maskBin(si, 0, 1.0);
      workspace->maskBin(si, 1, 1.0);
      workspace->maskBin(si, 2, 1.0);
    }

    SaveNexusProcessed save;
    save.initialize();
    save.setPropertyValue("InputWorkspace", output_ws);
    std::string filename = "LoadNexusProcessed_tmp.nxs";
    save.setPropertyValue("Filename", filename);
    filename = save.getPropertyValue("Filename");
    save.execute();
    LoadNexusProcessed load;
    load.initialize();
    load.setPropertyValue("Filename", filename);
    load.setPropertyValue("OutputWorkspace", output_ws);
    load.execute();

    workspace = boost::dynamic_pointer_cast<MatrixWorkspace>(
        AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    TS_ASSERT_EQUALS(workspace->getNumberHistograms(), 6);

    TS_ASSERT(workspace->hasMaskedBins(0));
    TS_ASSERT(workspace->hasMaskedBins(1));
    TS_ASSERT(workspace->hasMaskedBins(2));
    TS_ASSERT(workspace->hasMaskedBins(3));
    TS_ASSERT(workspace->hasMaskedBins(4));
    TS_ASSERT(workspace->hasMaskedBins(5));

    if (Poco::File(filename).exists())
      Poco::File(filename).remove();
  }

  void dotest_LoadAnEventFile(EventType type)
  {
    std::string filename_root = "LoadNexusProcessed_ExecEvent_";

    // Call a function that writes out the file
    std::string outputFile;
    EventWorkspace_sptr origWS = SaveNexusProcessedTest::do_testExec_EventWorkspaces(filename_root, type,
        outputFile, false, false);

    LoadNexusProcessed alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    alg.setPropertyValue("Filename", outputFile);
    alg.setPropertyValue("OutputWorkspace", output_ws);

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    //Test some aspects of the file
    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve(output_ws));
    TS_ASSERT( workspace.get());

    EventWorkspace_sptr ws = boost::dynamic_pointer_cast<EventWorkspace>(workspace);
    TS_ASSERT( ws);
    if (!ws)
      return;

    //Testing the number of histograms
    TS_ASSERT_EQUALS(ws->getNumberHistograms(), 5);

    for (size_t wi = 0; wi < 5; wi++)
    {
      const EventList & el = ws->getEventList(wi);
      TS_ASSERT_EQUALS( el.getEventType(), type);
      TS_ASSERT( el.hasDetectorID(detid_t(wi+1)*10));
    }
    TS_ASSERT_EQUALS( ws->getEventList(0).getNumberEvents(), 300);
    TS_ASSERT_EQUALS( ws->getEventList(1).getNumberEvents(), 100);
    TS_ASSERT_EQUALS( ws->getEventList(2).getNumberEvents(), 200);
    TS_ASSERT_EQUALS( ws->getEventList(3).getNumberEvents(), 0);
    TS_ASSERT_EQUALS( ws->getEventList(4).getNumberEvents(), 100);

    // Do the comparison algo to check that they really are the same
    origWS->sortAll(TOF_SORT, NULL);
    ws->sortAll(TOF_SORT, NULL);

    IAlgorithm_sptr alg2 = AlgorithmManager::Instance().createUnmanaged("CheckWorkspacesMatch");
    alg2->initialize();
    alg2->setProperty<MatrixWorkspace_sptr>("Workspace1", origWS);
    alg2->setProperty<MatrixWorkspace_sptr>("Workspace2", ws);
    alg2->setProperty<double>("Tolerance", 1e-5);
    alg2->setProperty<bool>("CheckAxes", false);
    alg2->execute();
    if (alg2->isExecuted())
    {
      TS_ASSERT(alg2->getPropertyValue("Result") == "Success!");
    }
    else
    {
      TS_ASSERT( false);
    }

    //Clear old file
    if (Poco::File(outputFile).exists())
      Poco::File(outputFile).remove();

  }

  void test_LoadEventNexus_TOF()
  {
    dotest_LoadAnEventFile(TOF);
  }

  void test_LoadEventNexus_WEIGHTED()
  {
    dotest_LoadAnEventFile(WEIGHTED);
  }

  void test_LoadEventNexus_WEIGHTED_NOTIME()
  {
    dotest_LoadAnEventFile(WEIGHTED_NOTIME);
  }

  void test_load_saved_workspace_group()
  {
    LoadNexusProcessed alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    alg.setPropertyValue("Filename", "WorkspaceGroup.nxs");
    alg.setPropertyValue("OutputWorkspace", "group");

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve("group"));
    WorkspaceGroup_sptr group = boost::dynamic_pointer_cast<WorkspaceGroup>(workspace);
    TS_ASSERT( group);
    int groupSize = group->getNumberOfEntries();
    TS_ASSERT_EQUALS( groupSize, 12);
    for (int i = 0; i < groupSize; ++i)
    {
      MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(group->getItem(i));
      TS_ASSERT( ws);
      TS_ASSERT_EQUALS( ws->getNumberHistograms(), 1);
      TS_ASSERT_EQUALS( ws->blocksize(), 10);
      TS_ASSERT_EQUALS( ws->name(), "group_" + boost::lexical_cast<std::string>( i + 1 ));
    }

  }

  void test_load_workspace_group_unique_names()
  {
    LoadNexusProcessed alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());

    //Group two uses unique names for each workspace
    alg.setPropertyValue("Filename", "WorkspaceGroup2.nxs");
    alg.setPropertyValue("OutputWorkspace", "group");

    const char* suffix[] =
    { "eq2", "eq1", "elf" };
    TS_ASSERT_THROWS_NOTHING(alg.execute());

    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve("group"));
    WorkspaceGroup_sptr group = boost::dynamic_pointer_cast<WorkspaceGroup>(workspace);
    TS_ASSERT( group);
    int groupSize = group->getNumberOfEntries();
    TS_ASSERT_EQUALS( groupSize, 3);
    for (int i = 0; i < groupSize; ++i)
    {
      MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(group->getItem(i));
      TS_ASSERT( ws);
      TS_ASSERT_EQUALS( ws->getNumberHistograms(), 1);
      TS_ASSERT_EQUALS( ws->blocksize(), 2);
      TS_ASSERT_EQUALS( ws->name(), "irs55125_graphite002_to_55131_" + std::string(suffix[i]));
    }
  }

  void test_load_workspace_group_unique_names_two_workspaces()
  {
    LoadNexusProcessed alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());

    //Group two uses unique names for each workspace
    alg.setPropertyValue("Filename", "WorkspaceGroup2.nxs");
    alg.setPropertyValue("OutputWorkspace", "group");

    const char* suffix[] =
    { "eq2", "eq1", "elf" };
    TS_ASSERT_THROWS_NOTHING(alg.execute());

    Workspace_sptr workspace;
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve("group"));
    WorkspaceGroup_sptr group = boost::dynamic_pointer_cast<WorkspaceGroup>(workspace);
    TS_ASSERT( group);
    int groupSize = group->getNumberOfEntries();
    TS_ASSERT_EQUALS( groupSize, 3);
    for (int i = 0; i < groupSize; ++i)
    {
      MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(group->getItem(i));
      TS_ASSERT( ws);
      TS_ASSERT_EQUALS( ws->getNumberHistograms(), 1);
      TS_ASSERT_EQUALS( ws->blocksize(), 2);
      TS_ASSERT_EQUALS( ws->name(), "irs55125_graphite002_to_55131_" + std::string(suffix[i]));
    }

    //load same file again, but to a different group
    //this checks that the names will be unique

    LoadNexusProcessed alg2;
    TS_ASSERT_THROWS_NOTHING(alg2.initialize());
    TS_ASSERT( alg2.isInitialized());

    //Group two uses unique names for each workspace
    alg2.setPropertyValue("Filename", "WorkspaceGroup2.nxs");
    alg2.setPropertyValue("OutputWorkspace", "group2");

    TS_ASSERT_THROWS_NOTHING(alg2.execute());
    TS_ASSERT_THROWS_NOTHING( workspace = AnalysisDataService::Instance().retrieve("group2"));
    group = boost::dynamic_pointer_cast<WorkspaceGroup>(workspace);
    TS_ASSERT( group);
    groupSize = group->getNumberOfEntries();
    TS_ASSERT_EQUALS( groupSize, 3);

    for (int i = 0; i < groupSize; ++i)
    {
      MatrixWorkspace_sptr ws = boost::dynamic_pointer_cast<MatrixWorkspace>(group->getItem(i));
      TS_ASSERT( ws);
      TS_ASSERT_EQUALS( ws->getNumberHistograms(), 1);
      TS_ASSERT_EQUALS( ws->blocksize(), 2);
      TS_ASSERT_EQUALS( ws->name(), "irs55125_graphite002_to_55131_" + std::string(suffix[i]) + "_1");
    }
  }

  void test_load_fit_parameters()
  {
    LoadNexusProcessed alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT( alg.isInitialized());
    alg.setPropertyValue("Filename", "HRP38692a.nxs");
    alg.setPropertyValue("OutputWorkspace", "HRPDparameters");

    TS_ASSERT_THROWS_NOTHING(alg.execute());

    MatrixWorkspace_sptr ws;
    ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>("HRPDparameters");

    // test to see if parameters are loaded
    std::vector<boost::shared_ptr<const Mantid::Geometry::IComponent> > bankComp =
        ws->getInstrument()->getAllComponentsWithName("bank_bsk");

    TS_ASSERT( bankComp[0]->getParameterNames().size() == 3);
  }

  void testTableWorkspace()
  {
    Load alg;
    alg.initialize();
    alg.setPropertyValue("Filename", "SavedTableWorkspace.nxs");
    const std::string wsName("SavedTableWorkspace");
    alg.setPropertyValue("OutputWorkspace", wsName);
    TS_ASSERT( alg.execute());

    TableWorkspace_const_sptr ws = AnalysisDataService::Instance().retrieveWS<TableWorkspace>(wsName);
    TS_ASSERT_EQUALS( ws->columnCount(), 3);
    TS_ASSERT_EQUALS( ws->rowCount(), 4);
    Column_const_sptr column;
    TS_ASSERT_THROWS_NOTHING( column = ws->getColumn("Name"));
    TS_ASSERT_EQUALS( column->cell<std::string>(0), "Height");
    TS_ASSERT_EQUALS( column->cell<std::string>(1), "PeakCentre");
    TS_ASSERT_EQUALS( column->cell<std::string>(2), "Sigma");
    TS_ASSERT_EQUALS( column->cell<std::string>(3), "Cost function value");
    TS_ASSERT_THROWS_NOTHING( column = ws->getColumn("Value"));
    TS_ASSERT_DELTA( column->cell<double>(0), 79.2315, 0.0001);
    TS_ASSERT_DELTA( column->cell<double>(1), 2.3979, 0.0001);
    TS_ASSERT_DELTA( column->cell<double>(2), 0.3495, 0.0001);
    TS_ASSERT_DELTA( column->cell<double>(3), 35.6841, 0.0001);
    TS_ASSERT_THROWS_NOTHING( column = ws->getColumn("Error"));
    TS_ASSERT_DELTA( column->cell<double>(0), 0.814478, 0.0001);
    TS_ASSERT_DELTA( column->cell<double>(1), 0.00348291, 0.000001);
    TS_ASSERT_DELTA( column->cell<double>(2), 0.00342847, 0.000001);
    TS_ASSERT_EQUALS( column->cell<double>(3), 0);

    AnalysisDataService::Instance().remove(wsName);
  }

  void testTableWorkspace_vectorColumn()
  {
    // Create a table we will save
    ITableWorkspace_sptr table = WorkspaceFactory::Instance().createTable();
    table->addColumn("vector_int", "IntVectorColumn");
    table->addColumn("vector_double", "DoubleVectorColumn");

    std::vector<double> d1, d2, d3;
    d1.push_back(0.5);
    d2.push_back(1.0);
    d2.push_back(2.5);
    d3.push_back(4.0);

    std::vector<int> i1(Strings::parseRange("1"));
    std::vector<int> i2(Strings::parseRange("2,3,"));
    std::vector<int> i3(Strings::parseRange("4,5,6,7"));

    // Add some rows of different sizes
    TableRow row1 = table->appendRow();
    row1 << i1 << d1;
    TableRow row2 = table->appendRow();
    row2 << i2 << d2;
    TableRow row3 = table->appendRow();
    row3 << i3 << d3;

    ScopedWorkspace inTableEntry(table);
    std::string savedFileName("LoadNexusProcessedTest_testTableWorkspace_vectorColumn.nxs");

    SaveNexusProcessed saveAlg;
    saveAlg.initialize();
    saveAlg.setPropertyValue("InputWorkspace", inTableEntry.name());
    saveAlg.setPropertyValue("Filename", savedFileName);
    TS_ASSERT_THROWS_NOTHING( saveAlg.execute());
    TS_ASSERT( saveAlg.isExecuted());

    if (!saveAlg.isExecuted())
      return; // Nothing to check

    // Get absolute path to the saved file
    savedFileName = saveAlg.getPropertyValue("Filename");

    ScopedWorkspace outTableEntry;

    LoadNexusProcessed loadAlg;
    loadAlg.initialize();
    loadAlg.setPropertyValue("Filename", savedFileName);
    loadAlg.setPropertyValue("OutputWorkspace", outTableEntry.name());

    TS_ASSERT_THROWS_NOTHING( loadAlg.execute());
    TS_ASSERT( loadAlg.isExecuted());

    // The file is not needed anymore
    Poco::File(savedFileName).remove();

    if (!loadAlg.isExecuted())
      return; // Nothing to check

    auto outTable = boost::dynamic_pointer_cast<const TableWorkspace>(outTableEntry.retrieve());
    TS_ASSERT( outTable);

    if (!outTable)
      return; // Nothing to check

    TS_ASSERT_EQUALS( outTable->columnCount(), 2);
    TS_ASSERT_EQUALS( outTable->rowCount(), 3);

    Column_const_sptr column; // Column we are currently checking

    TS_ASSERT_THROWS_NOTHING( column = outTable->getColumn("IntVectorColumn"));
    TS_ASSERT( column->isType< std::vector<int> >());

    if (column->isType<std::vector<int> >())
    {
      TS_ASSERT_EQUALS( column->cell< std::vector<int> >(0), i1);
      TS_ASSERT_EQUALS( column->cell< std::vector<int> >(1), i2);
      TS_ASSERT_EQUALS( column->cell< std::vector<int> >(2), i3);
    }

    TS_ASSERT_THROWS_NOTHING( column = outTable->getColumn("DoubleVectorColumn"));
    TS_ASSERT( column->isType< std::vector<double> >());

    if (column->isType<std::vector<double> >())
    {
      TS_ASSERT_EQUALS( column->cell< std::vector<double> >(0), d1);
      TS_ASSERT_EQUALS( column->cell< std::vector<double> >(1), d2);
      TS_ASSERT_EQUALS( column->cell< std::vector<double> >(2), d3);
    }
  }

  void do_load_multiperiod_workspace(bool fast)
  {
    LoadNexusProcessed loader;
    loader.setChild(true);
    loader.initialize();
    loader.setPropertyValue("Filename", "POLREF00004699_nexus.nxs");
    loader.setPropertyValue("OutputWorkspace", "ws");
    loader.setProperty("FastMultiPeriod", fast);

    TS_ASSERT( loader.execute());

    Workspace_sptr outWS = loader.getProperty("OutputWorkspace");
    WorkspaceGroup_sptr asGroupWS = boost::dynamic_pointer_cast<WorkspaceGroup>(outWS);
    TSM_ASSERT("We expect a group workspace back", asGroupWS);
    TSM_ASSERT_EQUALS("We expect the size to be 2", 2, asGroupWS->size());
    MatrixWorkspace_sptr period1 = boost::dynamic_pointer_cast<MatrixWorkspace>(asGroupWS->getItem(0));
    MatrixWorkspace_sptr period2 = boost::dynamic_pointer_cast<MatrixWorkspace>(asGroupWS->getItem(1));
    TSM_ASSERT("We expect the group workspace is multiperiod", asGroupWS->isMultiperiod());
    TSM_ASSERT_EQUALS("X-data should be identical", period1->readX(0), period2->readX(0));
    TSM_ASSERT_DIFFERS("Y-data should be different", period1->readY(0), period2->readY(0));
    TSM_ASSERT_DIFFERS("E-data should be different", period1->readE(0), period2->readE(0));

    TS_ASSERT(period1->getInstrument());
    TS_ASSERT(period2->getInstrument());

    auto period1Logs = period1->run().getLogData();
    auto period2Logs = period2->run().getLogData();

    TSM_ASSERT_EQUALS("We expect to have the same number of log entries", period1Logs.size(),
        period2Logs.size());

    TSM_ASSERT_THROWS("Should only have a period 1 entry", period1->run().getLogData("period 2"),
        Exception::NotFoundError&);
    TSM_ASSERT_THROWS("Should only have a period 2 entry", period2->run().getLogData("period 1"),
        Exception::NotFoundError&);
  }

  void test_load_multiperiod_workspace_fast()
  {
    do_load_multiperiod_workspace(true /*Use optimised route*/);
  }

  void test_load_multiperiod_workspace_old()
  {
    do_load_multiperiod_workspace(false /*Use old route*/);
  }

private:
  void doHistoryTest(MatrixWorkspace_sptr matrix_ws)
  {
    const WorkspaceHistory history = matrix_ws->getHistory();
    int nalgs = static_cast<int>(history.size());
    TS_ASSERT_EQUALS(nalgs, 4);

    if (nalgs == 4)
    {
      TS_ASSERT_EQUALS(history[0]->name(), "LoadRaw");
      TS_ASSERT_EQUALS(history[1]->name(), "AlignDetectors");
      TS_ASSERT_EQUALS(history[2]->name(), "DiffractionFocussing");
      TS_ASSERT_EQUALS(history[3]->name(), "LoadNexusProcessed");
    }
  }

  LoadNexusProcessed algToBeTested;
  std::string testFile, output_ws;
};

//------------------------------------------------------------------------------
// Performance test
//------------------------------------------------------------------------------

class LoadNexusProcessedTestPerformance: public CxxTest::TestSuite
{
public:
  void testHistogramWorkspace()
  {
    LoadNexusProcessed loader;
    loader.initialize();
    loader.setPropertyValue("Filename", "PG3_733_focussed.nxs");
    loader.setPropertyValue("OutputWorkspace", "ws");
    TS_ASSERT( loader.execute());
  }
};

#endif /*LOADNEXUSPROCESSEDTESTRAW_H_*/
