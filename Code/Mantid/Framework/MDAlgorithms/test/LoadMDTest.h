#ifndef MANTID_MDEVENTS_LOADMDEWTEST_H_
#define MANTID_MDEVENTS_LOADMDEWTEST_H_

#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidKernel/CPUTimer.h"
#include "MantidKernel/System.h"
#include "MantidKernel/Timer.h"
#include "MantidMDEvents/MDBox.h"
#include "MantidMDEvents/MDGridBox.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidMDEvents/MDEventWorkspace.h"
#include "SaveMDTest.h"
#include <cxxtest/TestSuite.h>
#include <iomanip>
#include <iostream>
#include "MantidAPI/ExperimentInfo.h"
#include "MantidMDAlgorithms/LoadMD.h"

using namespace Mantid;
using namespace Mantid::MDEvents;
using namespace Mantid::MDAlgorithms;
using namespace Mantid::API;
using namespace Mantid::Kernel;

class LoadMDTest : public CxxTest::TestSuite
{
public:
    
  void test_Init()
  {
    LoadMD alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
  }
  

  /// Compare two box controllers and assert each part of them.
  static void compareBoxControllers(BoxController & a, BoxController & b)
  {
    TS_ASSERT_EQUALS( a.getNDims(), b.getNDims());
    TS_ASSERT_EQUALS( a.getMaxDepth(), b.getMaxDepth());
    TS_ASSERT_EQUALS( a.getMaxId(), b.getMaxId());
    TS_ASSERT_EQUALS( a.getSplitThreshold(), b.getSplitThreshold());
    TS_ASSERT_EQUALS( a.getNumMDBoxes(), b.getNumMDBoxes());
    TS_ASSERT_EQUALS( a.getNumSplit(), b.getNumSplit());
    TS_ASSERT_EQUALS( a.getMaxNumMDBoxes(), b.getMaxNumMDBoxes());
    for (size_t d=0; d< a.getNDims(); d++)
    {
      TS_ASSERT_EQUALS( a.getSplitInto(d), b.getSplitInto(d));
    }
  }



  //=================================================================================================================
  /** Compare two MDEventWorkspaces
   *
   * @param ws :: workspace to check
   * @param ws1 :: reference workspace
   * @param BoxStructureOnly :: true if you only compare the box structure and ignore differences in event lists
   */
  template<typename MDE, size_t nd>
  static void do_compare_MDEW(boost::shared_ptr<MDEventWorkspace<MDE,nd> > ws1,
      boost::shared_ptr<MDEventWorkspace<MDE,nd> > ws2,
      bool BoxStructureOnly = false)
  {
    TS_ASSERT(ws1->getBox());

    // Compare the initial to the final workspace
    TS_ASSERT_EQUALS(ws1->getBox()->getNumChildren(), ws2->getBox()->getNumChildren());
    if (!BoxStructureOnly)
    { TS_ASSERT_EQUALS(ws1->getNPoints(), ws2->getNPoints()); }

    TS_ASSERT_EQUALS(ws1->getBoxController()->getMaxId(), ws2->getBoxController()->getMaxId());
    // Compare all the details of the box1 controllers
    compareBoxControllers(*ws1->getBoxController(), *ws2->getBoxController());
    
    // Compare every box1
    std::vector<MDBoxBase<MDE,nd>*> boxes;
    std::vector<MDBoxBase<MDE,nd>*> boxes1;

    ws1->getBox()->getBoxes(boxes, 1000, false);
    ws2->getBox()->getBoxes(boxes1, 1000, false);

    TS_ASSERT_EQUALS( boxes.size(), boxes1.size());
    if (boxes.size() != boxes1.size()) return;

    for (size_t j=0; j<boxes.size(); j++)
    {
      MDBoxBase<MDE,nd>* box1 = boxes[j];
      MDBoxBase<MDE,nd>* box2 = boxes1[j];

      //std::cout << "ID: " << box1->getId() << std::endl;
      TS_ASSERT_EQUALS( box1->getId(), box2->getId() );
      TS_ASSERT_EQUALS( box1->getDepth(), box2->getDepth() );
      TS_ASSERT_EQUALS( box1->getNumChildren(), box2->getNumChildren() );
      for (size_t i=0; i<box1->getNumChildren(); i++)
      {
        TS_ASSERT_EQUALS( box1->getChild(i)->getId(), box2->getChild(i)->getId() );
      }
      for (size_t d=0; d<nd; d++)
      {
        TS_ASSERT_DELTA( box1->getExtents(d).min, box2->getExtents(d).min, 1e-5);
        TS_ASSERT_DELTA( box1->getExtents(d).max, box2->getExtents(d).max, 1e-5);
      }
      TS_ASSERT_DELTA( box1->getVolume(), box2->getVolume(), 1e-3);
      if (!BoxStructureOnly)
      {
        TS_ASSERT_DELTA( box1->getSignal(), box2->getSignal(), 1e-3);
        TS_ASSERT_DELTA( box1->getErrorSquared(), box2->getErrorSquared(), 1e-3);
        TS_ASSERT_EQUALS( box1->getNPoints(), box2->getNPoints() );
      }
      TS_ASSERT( box1->getBoxController() );
      TS_ASSERT( box1->getBoxController()==ws1->getBoxController() );

      // Are both MDGridBoxes ?
      MDGridBox<MDE,nd>* gridbox1 = dynamic_cast<MDGridBox<MDE,nd>*>(box1);
      MDGridBox<MDE,nd>* gridbox2 = dynamic_cast<MDGridBox<MDE,nd>*>(box2);
      if (gridbox1)
      {
        for (size_t d=0; d<nd; d++)
        {
          TS_ASSERT_DELTA( gridbox1->getBoxSize(d), gridbox2->getBoxSize(d), 1e-4);
        }
      }

      // Are both MDBoxes (with events)
      MDBox<MDE,nd>* mdbox1 = dynamic_cast<MDBox<MDE,nd>*>(box1);
      MDBox<MDE,nd>* mdbox2 = dynamic_cast<MDBox<MDE,nd>*>(box2);
      if (mdbox1)
      {
        TS_ASSERT( mdbox2 );
        if (!BoxStructureOnly)
        {
          const std::vector<MDE > & events1 = mdbox1->getConstEvents();
          const std::vector<MDE > & events2 = mdbox2->getConstEvents();
          TS_ASSERT_EQUALS( events1.size(), events2.size() );
          if (events1.size() == events2.size() && events1.size() > 2)
          {
            // Check first and last event
            for (size_t i=0; i<events1.size(); i+=events1.size()-1)
            {
              for (size_t d=0; d<nd; d++)
              {
                TS_ASSERT_DELTA( events1[i].getCenter(d), events2[i].getCenter(d), 1e-4);
              }
              TS_ASSERT_DELTA( events1[i].getSignal(), events2[i].getSignal(), 1e-4);
              TS_ASSERT_DELTA( events1[i].getErrorSquared(), events2[i].getErrorSquared(), 1e-4);
            }
          }
          mdbox1->releaseEvents();
          mdbox2->releaseEvents();
        }// Don't compare if BoxStructureOnly
      } // is mdbox1
    }

    TS_ASSERT_EQUALS(ws1->getNumExperimentInfo(), ws2->getNumExperimentInfo());
    if (ws1->getNumExperimentInfo() == ws2->getNumExperimentInfo())
    {
      for (uint16_t i=0; i < ws1->getNumExperimentInfo(); i++)
      {
        ExperimentInfo_sptr ei1 = ws1->getExperimentInfo(i);
        ExperimentInfo_sptr ei2 = ws2->getExperimentInfo(i);
        //TS_ASSERT_EQUALS(ei1->getInstrument()->getName(), ei2->getInstrument()->getName());
      }
    }

  }



  //=================================================================================================================
  template <size_t nd>
  void do_test_exec(bool FileBackEnd, bool deleteWorkspace=true, double memory=0, bool BoxStructureOnly = false)
  {
    typedef MDLeanEvent<nd> MDE;

    //------ Start by creating the file ----------------------------------------------
    // Make a 1D MDEventWorkspace
    boost::shared_ptr<MDEventWorkspace<MDE,nd> > ws1 = MDEventsTestHelper::makeMDEW<nd>(10, 0.0, 10.0, 0);
    ws1->getBoxController()->setSplitThreshold(100);
    // Put in ADS so we can use fake data
    AnalysisDataService::Instance().addOrReplace("LoadMDTest_ws", boost::dynamic_pointer_cast<IMDEventWorkspace>(ws1));
    FrameworkManager::Instance().exec("FakeMDEventData", 6,
        "InputWorkspace", "LoadMDTest_ws", "UniformParams", "10000", "RandomizeSignal", "1");
//    FrameworkManager::Instance().exec("FakeMDEventData", 6,
//        "InputWorkspace", "LoadMDTest_ws", "PeakParams", "30000, 5.0, 0.01", "RandomizeSignal", "1");

    // ------ Make a ExperimentInfo entry ------------
    ExperimentInfo_sptr ei(new ExperimentInfo());
    ei->mutableRun().setProtonCharge(1.234);
    // TODO: Add instrument
    ws1->addExperimentInfo(ei);

    // -------- Save it ---------------
    SaveMD saver;
    TS_ASSERT_THROWS_NOTHING( saver.initialize() )
    TS_ASSERT( saver.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( saver.setProperty("InputWorkspace", "LoadMDTest_ws" ) );
    TS_ASSERT_THROWS_NOTHING( saver.setPropertyValue("Filename",  "LoadMDTest" + Strings::toString(nd) + ".nxs") );

    // Retrieve the full path; delete any pre-existing file
    std::string filename = saver.getPropertyValue("Filename");
    if (Poco::File(filename).exists()) Poco::File(filename).remove();

    TS_ASSERT_THROWS_NOTHING( saver.execute(); );
    TS_ASSERT( saver.isExecuted() );


    //------ Now the loading -------------------------------------
    // Name of the output workspace.
    std::string outWSName("LoadMDTest_OutputWS");

    CPUTimer tim;

    LoadMD alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("Filename", filename) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("FileBackEnd", FileBackEnd) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("Memory", memory) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("OutputWorkspace", outWSName) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("MetadataOnly", false));
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("BoxStructureOnly", BoxStructureOnly));
    TS_ASSERT_THROWS_NOTHING( alg.execute(); );
    TS_ASSERT( alg.isExecuted() );

    std::cout << tim << " to do the entire MDEW loading." << std::endl;

    // Retrieve the workspace from data service.
    IMDEventWorkspace_sptr iws;
    TS_ASSERT_THROWS_NOTHING( iws = AnalysisDataService::Instance().retrieveWS<IMDEventWorkspace>(outWSName) );
    TS_ASSERT(iws);
    if (!iws) return;

    boost::shared_ptr<MDEventWorkspace<MDE,nd> > ws = boost::dynamic_pointer_cast<MDEventWorkspace<MDLeanEvent<nd>,nd> >(iws);

    // Perform the full comparison
    do_compare_MDEW(ws, ws1, BoxStructureOnly);

    // Look for the not-disk-cached-cause-they-are-too-small
    if (memory > 0)
    {
      // Force a flush of the read-write cache
      BoxController_sptr bc = ws->getBoxController();
      DiskBuffer & dbuf = bc->getDiskBuffer();
      dbuf.flushCache();

      typename std::vector<MDBoxBase<MDE,nd>*> boxes;
      ws->getBox()->getBoxes(boxes, 1000, false);
      for (size_t i=0; i<boxes.size(); i++)
      {
        MDBox<MDE,nd>*box = dynamic_cast<MDBox<MDE,nd>*>(boxes[i]);
        if (box)
        {
          TSM_ASSERT("Large box should not be in memory", !box->getInMemory());
          TSM_ASSERT("Large box should be cached to disk", box->getOnDisk());
        }
      }
    }

    // Remove workspace from the data service.
    if (deleteWorkspace)
    {
      ws->getBoxController()->closeFile(true);
      AnalysisDataService::Instance().remove(outWSName);
      if (Poco::File(filename).exists()) Poco::File(filename).remove();
    }
  }


  /** Follow up test that:
   *  - Modifies the data in a couple of ways
   *  - Saves AGAIN to update a file back end
   *  - Re-loads to a brand new workspace and compares everything. */
  template <size_t nd>
  void do_test_UpdateFileBackEnd()
  {
    std::string outWSName("LoadMDTest_OutputWS");
    IMDEventWorkspace_sptr iws;
    TS_ASSERT_THROWS_NOTHING( iws = AnalysisDataService::Instance().retrieveWS<IMDEventWorkspace>(outWSName) );
    TS_ASSERT(iws); if (!iws) return;
    boost::shared_ptr<MDEventWorkspace<MDLeanEvent<nd>,nd> > ws2 = boost::dynamic_pointer_cast<MDEventWorkspace<MDLeanEvent<nd>,nd> >(iws);

    // Modify that by adding some boxes
    MDGridBox<MDLeanEvent<nd>,nd> * box = dynamic_cast<MDGridBox<MDLeanEvent<nd>,nd>*>(ws2->getBox());
    // Now there are 54 boxes
    box->splitContents(12);

    // And add an ExperimentInfo thingie
    ExperimentInfo_sptr ei(new ExperimentInfo());
    ei->mutableRun().setProtonCharge(2.345);
    iws->addExperimentInfo(ei);

    // Add one event using addEvent(). The event will need to be written out to disk too.
    MDLeanEvent<nd> ev(1.0, 1.0);
    for (size_t d=0; d<nd; d++) ev.setCenter(d, 0.5);
    box->addEvent(ev);

    // Modify a different box by accessing the events
    MDBox<MDLeanEvent<nd>,nd> * box8 = dynamic_cast<MDBox<MDLeanEvent<nd>,nd>*>(box->getChild(8));
    std::vector<MDLeanEvent<nd> > & events = box8->getEvents();
    // Add 10 to this signal
    signal_t newSignal = events[0].getSignal() + 10.0;
    events[0].setSignal( float(newSignal) );
    box8->releaseEvents();

//    // Modify a third box by adding an event
//    MDBox<MDLeanEvent<nd>,nd> * box17 = dynamic_cast<MDBox<MDLeanEvent<nd>,nd>*>(box->getChild(17));
//    std::vector<MDLeanEvent<nd> > & events17 = box17->getEvents();
//    MDLeanEvent<nd> ev_new(1.0, 1.0);
//    box->addEvent( ev_new );
//    box17->releaseEvents();

    ws2->refreshCache();

    // There are now 2 more events
    TS_ASSERT_EQUALS( ws2->getNPoints(), 10001 );

    // There are some new boxes that are not cached to disk at this point.
    // Save it again.
    SaveMD saver;
    TS_ASSERT_THROWS_NOTHING( saver.initialize() )
    TS_ASSERT( saver.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( saver.setPropertyValue("InputWorkspace", outWSName ) );
    TS_ASSERT_THROWS_NOTHING( saver.setPropertyValue("Filename", "") );
    TS_ASSERT_THROWS_NOTHING( saver.setPropertyValue("UpdateFileBackEnd", "1") );
    TS_ASSERT_THROWS_NOTHING( saver.execute(); );
    TS_ASSERT( saver.isExecuted() );

    // Now we look at the file that's currently open
    ::NeXus::File * file = ws2->getBoxController()->getFile();
    TSM_ASSERT_LESS_THAN( "The event_data field in the file must be at least 10002 long.", 10002, file->getInfo().dims[0] );


    // The file should have been modified but that's tricky to check directly.
    std::string filename = ws2->getBoxController()->getFilename();
    // Now we re-re-load it!
    LoadMD alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("Filename", filename) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("FileBackEnd", false) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("OutputWorkspace", "reloaded_again") );
    TS_ASSERT_THROWS_NOTHING( alg.execute(); );
    TS_ASSERT( alg.isExecuted() );

    TS_ASSERT_THROWS_NOTHING( iws = AnalysisDataService::Instance().retrieveWS<IMDEventWorkspace>("reloaded_again") );
    boost::shared_ptr<MDEventWorkspace<MDLeanEvent<nd>,nd> > ws3 = boost::dynamic_pointer_cast<MDEventWorkspace<MDLeanEvent<nd>,nd> >(iws);
    TS_ASSERT(ws3); if (!ws3) return;
    ws3->refreshCache();

    // Perform the full comparison of the second and 3rd loaded workspaces
    do_compare_MDEW(ws2, ws3);

    ws2->getBoxController()->closeFile(true);
    AnalysisDataService::Instance().remove(outWSName);

  }


  //=================================================================================================================

  /// Load directly to memory
  void test_exec_1D()
  {
    do_test_exec<1>(false);
  }

  /// Run the loading but keep the events on file and load on demand
  void test_exec_1D_with_file_backEnd()
  {
    do_test_exec<1>(true);
  }

  /// Load directly to memory
  void test_exec_3D()
  {
    do_test_exec<3>(false);
  }

  /// Run the loading but keep the events on file and load on demand
  void test_exec_3D_with_FileBackEnd()
  {
    do_test_exec<3>(true);
  }

  /// Run the loading but keep the events on file and load on demand
  void test_exec_3D_with_FileBackEnd_andSmallBuffer()
  {
    do_test_exec<3>(true, true, 1.0);
  }
  

  /** Use the file back end,
   * then change it and save to update the file at the back end.
   */
  void test_exec_3D_with_FileBackEnd_then_update_SaveMDEW()
  {
    std::cout << "Starting the first step\n";
    do_test_exec<3>(true, false);
    std::cout << "\nStarting the update step\n";
    do_test_UpdateFileBackEnd<3>();
  }


  /// Only load the box structure, no events
  void test_exec_3D_BoxStructureOnly()
  {
    do_test_exec<3>(false, true, 0.0, true);
  }




  //=================================================================================================================

  void testMetaDataOnly()
  {
    //------ Start by creating the file ----------------------------------------------
    // Make a 1D MDEventWorkspace
    boost::shared_ptr<MDEventWorkspace<MDLeanEvent<2>,2> > ws1 = MDEventsTestHelper::makeMDEW<2>(10, 0.0, 10.0, 0);
    ws1->getBoxController()->setSplitThreshold(100);
    AnalysisDataService::Instance().addOrReplace("LoadMDTest_ws", boost::dynamic_pointer_cast<IMDEventWorkspace>(ws1));

    // Save it
    SaveMD saver;
    TS_ASSERT_THROWS_NOTHING( saver.initialize() )
    TS_ASSERT( saver.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( saver.setProperty("InputWorkspace", "LoadMDTest_ws" ) );
    TS_ASSERT_THROWS_NOTHING( saver.setPropertyValue("Filename", "LoadMDTest2.nxs") );
    TS_ASSERT_THROWS_NOTHING( saver.execute(); );
    TS_ASSERT( saver.isExecuted() );

    // Retrieve the full path
    std::string filename = saver.getPropertyValue("Filename");

    //------ Now the loading -------------------------------------
    // Name of the output workspace.
    std::string outWSName("LoadMDTest_OutputWS");
    LoadMD alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("Filename", filename) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("FileBackEnd", false) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("Memory", 0.0) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("OutputWorkspace", outWSName) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("MetadataOnly", true));
    TS_ASSERT_THROWS_NOTHING( alg.execute(); );
    TS_ASSERT( alg.isExecuted() );
    boost::shared_ptr<MDEventWorkspace<MDLeanEvent<2>,2> > ws = AnalysisDataService::Instance().retrieveWS<MDEventWorkspace<MDLeanEvent<2>,2> >(outWSName);

    TSM_ASSERT_EQUALS("Should have no events!", 0, ws->getNPoints());
    TSM_ASSERT_EQUALS("Wrong number of dimensions", 2, ws->getNumDims());

    AnalysisDataService::Instance().remove(outWSName);
    try
    {  if (Poco::File(filename).exists()) Poco::File(filename).remove(); }
    catch (...)
    { /* ignore windows error */ }

  }



  /** Run SaveMD with the MDHistoWorkspace */
  void doTestHisto(MDHistoWorkspace_sptr ws)
  {
    std::string filename = "SaveMDTestHisto.nxs";

    SaveMD alg1;
    TS_ASSERT_THROWS_NOTHING( alg1.initialize() )
    TS_ASSERT( alg1.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg1.setProperty("InputWorkspace", ws) );
    TS_ASSERT_THROWS_NOTHING( alg1.setPropertyValue("Filename", filename) );
    alg1.execute();
    TS_ASSERT( alg1.isExecuted() );
    filename = alg1.getPropertyValue("Filename");

    LoadMD alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("Filename", filename) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("OutputWorkspace", "loaded") );
    TS_ASSERT_THROWS_NOTHING( alg.execute(); );
    TS_ASSERT( alg.isExecuted() );

    MDHistoWorkspace_sptr newWS;
    TS_ASSERT_THROWS_NOTHING( newWS = AnalysisDataService::Instance().retrieveWS<MDHistoWorkspace>("loaded") );
    TS_ASSERT(newWS); if (!newWS) return;

    TS_ASSERT_EQUALS( ws->getNPoints(), newWS->getNPoints());
    TS_ASSERT_EQUALS( ws->getNumDims(), newWS->getNumDims());
    for (size_t i=0; i<ws->getNPoints(); i++)
    {
      TS_ASSERT_DELTA(ws->getSignalAt(i), newWS->getSignalAt(i), 1e-6);
      TS_ASSERT_DELTA(ws->getErrorAt(i), newWS->getErrorAt(i), 1e-6);
      TS_ASSERT_DELTA(ws->getNumEventsAt(i), newWS->getNumEventsAt(i), 1e-6);
      TS_ASSERT_EQUALS(ws->getIsMaskedAt(i), newWS->getIsMaskedAt(i));
    }

    if (Poco::File(filename).exists())
      Poco::File(filename).remove();
  }

  void test_histo2() 
  {
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(2.5, 2, 10, 10.0, 3.5, "histo2", 4.5);
    doTestHisto(ws);
  }

  void test_histo3()
  {
    MDHistoWorkspace_sptr ws = MDEventsTestHelper::makeFakeMDHistoWorkspace(2.5, 3, 4, 10.0, 3.5, "histo3", 4.5);
    doTestHisto(ws);
  }

};

#endif /* MANTID_MDEVENTS_LOADMDEWTEST_H_ */

