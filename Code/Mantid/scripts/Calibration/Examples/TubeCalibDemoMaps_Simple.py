#
# TUBE CALIBRATION DEMONSTRATION PROGRAM FOR MAPS 
#
# This is a simple example for running calibration for and calibration run of MAPS.
# It uses the CalibrateMaps function  

#
from mantid.api import WorkspaceFactory  # For table worskspace of calibrations
from mantid.kernel import config  # To set default instrument to MAPS
from tube_calib_fit_params import * # To handle fit parameters
from ideal_tube import * # For ideal tube
from tube_calib import *  # For tube calibration functions
from tube_spec import * # For tube specification class

def CalibrateMaps( RunNumber ):
   '''
   RunNumber is the run number for the calibration.
   '''

   # == Set parameters for calibration ==
   previousDefaultInstrument = config['default.instrument']
   config['default.instrument']="MAPS"
   filename = str(RunNumber) # Name of calibration run
   print "Filename",filename
   rangeLower = 2000 # Integrate counts in each spectra from rangeLower to rangeUpper 
   rangeUpper = 10000 #


   # Set initial parameters for peak finding
   ExpectedHeight = -1000.0 # Expected Height of Gaussian Peaks  (initial value of fit parameter)
   ExpectedWidth = 8.0 # Expected width of Gaussian peaks in pixels (initial value of fit parameter)
   ExpectedPositions = [4.0, 85.0, 128.0, 165.0, 252.0] #Expected positions of the edges and Gaussian peaks in pixels (initial values of fit parameters)

   # Set what we want to calibrate (e.g whole intrument or one door )
   # First Calibrate A, B, C windows 
   CalibratedComponentsABC =  ['A1_window','A2_window', 'B1_window','B2_window', 'C1_window','C2_window','C3_window','C4_window'] 
   # Secondly Calibrate D windows with a different ideal tube
   CalibratedComponentsD = ['D1_window','D2_window','D3_window','D4_window']
 
    
   # Get calibration raw file and integrate it    
   rawCalibInstWS = Load(filename)  #'raw' in 'rawCalibInstWS' means unintegrated.
   print "Integrating Workspace"
   CalibInstWS = Integration( rawCalibInstWS, RangeLower=rangeLower, RangeUpper=rangeUpper )
   DeleteWorkspace(rawCalibInstWS)
   print "Created workspace (CalibInstWS) with integrated data from run and instrument to calibrate" 

   # == Create Objects needed for calibration ==

   #Create Calibration Table
   calibrationTable = CreateEmptyTableWorkspace(OutputWorkspace="CalibTable")
   calibrationTable.addColumn(type="int",name="Detector ID")  # "Detector ID" column required by ApplyCalbration
   calibrationTable.addColumn(type="V3D",name="Detector Position")  # "Detector Position" column required by ApplyCalbration

   # Specify components to calibrate
   tubeSetABC = TubeSpec(CalibInstWS)
   tubeSetABC.setTubeSpecByStringArray(CalibratedComponentsABC)
   tubeSetD = TubeSpec(CalibInstWS)
   tubeSetD.setTubeSpecByStringArray(CalibratedComponentsD)

   # Get ideal tubes, which depend on the tube set
   iTubeABC = IdealTube()
   iTubeD = IdealTube()
   # Set positions of where the shadows and ends should be.
   # First array gives positions in Metres and second array gives type 1=Gaussian peak 2=edge.
   # See http://www.mantidproject.org/IdealTube for details 
   # For A, B and C windows based on information supplied by Toby Perring.
   iTubeABC.setPositionsAndForm([-0.50375,-0.16667,-0.00, 0.16667, 0.50375 ],[2,1,1,1,2]) 
   # For D windows based on information supplied by Toby Perring.
   iTubeD.setPositionsAndForm([-0.715,-0.2205,-0.00, 0.2205, 0.715 ],[2,1,1,1,2]) 

   # Get fitting parameters
   fitPar = TubeCalibFitParams( ExpectedPositions, ExpectedHeight, ExpectedWidth )

   print "Created objects needed for calibration."

   # == Get the calibration and put results into calibration table ==
   print "Getting calibration (new positions of detectors) for A, B and C windows"
   getCalibration( CalibInstWS, tubeSetABC, calibrationTable,  fitPar, iTubeABC )
   print "Got calibration (new positions of detectors) for A, B and C windows "
   print "Getting calibration (new positions of detectors) for D windows"
   getCalibration( CalibInstWS, tubeSetD, calibrationTable,  fitPar, iTubeD )
   print "Got calibration (new positions of detectors) for D windows "

   # == Apply the Calibation ==
   ApplyCalibration( Workspace=CalibInstWS, PositionTable=calibrationTable)
   print "Applied calibration"


   # == Save workspace ==
   SaveNexusProcessed( CalibInstWS, 'TubeCalibDemoMapsResult.nxs',"Result of Running MAPS Calibration")
   print "saved calibrated workspace (CalibInstWS) into Nexus file TubeCalibDemoMapsResult.nxs"
   
   # == Reset dafault instrument ==
   config['default.instrument'] = previousDefaultInstrument

   # ==== End of CalibrateMaps() ====

CalibrateMaps( 14919 )