import unittest
from mantid.simpleapi import *
from mantid.api import IMDEventWorkspace
from mantid.kernel import V3D

class MantidHoraceCommandGenSQWTest(unittest.TestCase):
    
    def test_import(self):
        try:
            import MantidHorace.Command
        except ImportError:
            self.assertTrue(False, "Imports should not have failed")
        
    
    def test_units_of_deltae_only(self):
        from MantidHorace.Command import gen_sqw, MantidHoraceInputException 
      
        tofWS = CreateWorkspace(DataX=[1,2], DataY=[1],DataE=[1],UnitX='TOF')
        try:
            gen_sqw(input_workspaces=tofWS)
        except MantidHoraceInputException:
            pass
        else:
            self.assertTrue(False, "Should have thrown with an input workspace in TOF")
        finally:
            DeleteWorkspace(tofWS)
        
    def test_known_emode_only(self):
        from MantidHorace.Command import gen_sqw, MantidHoraceInputException 
        in_ws = CreateWorkspace(DataX=[1,2], DataY=[1],DataE=[1],UnitX='DeltaE')
        try:
            gen_sqw(input_workspaces=in_ws, emode='Unknown')
        except MantidHoraceInputException:
            pass
        else:
            self.assertTrue(False, "Should have thrown with an unknown emode")
        finally:
            DeleteWorkspace(in_ws)
            
    def test_single_workspace(self):
        from MantidHorace.Command import gen_sqw, MantidHoraceInputException 
        
        source_ws = Load(Filename='CNCS_7860_event')
        source_ws = ConvertUnits(InputWorkspace=source_ws, Target='DeltaE', EMode='Direct', EFixed=3)
        SetUB(Workspace=source_ws,a=1.4165,b=1.4165,c=1.4165,u=[1,0,0],v=[0,1,0])    
        AddSampleLog(Workspace=source_ws,LogName='Psi',LogText='0',LogType='Number Series')
        SetGoniometer(Workspace=source_ws, Axis0='Psi,0,1,0,1')
        
        out_ws = gen_sqw(input_workspaces=source_ws, emode='Direct')
        
        self.assertTrue(isinstance(out_ws, IMDEventWorkspace), "OutputWorkspace should be an MDEventWorkspace")
        self.assertEqual(4, out_ws.getNumDims())
        lattice = out_ws.getExperimentInfo(0).sample().getOrientedLattice()
        self.assertAlmostEqual(1.4165, lattice.a())
        self.assertAlmostEqual(90, lattice.alpha())

        DeleteWorkspace(out_ws)
        DeleteWorkspace(source_ws)
    
    def test_overwrite_ub(self):
        from MantidHorace.Command import gen_sqw, MantidHoraceInputException 
        
        source_ws = Load(Filename='CNCS_7860_event')
        source_ws = ConvertUnits(InputWorkspace=source_ws, Target='DeltaE', EMode='Direct', EFixed=3)
        SetUB(Workspace=source_ws,a=1.4165,b=1.4165,c=1.4165,u=[1,0,0],v=[0,1,0])    
        AddSampleLog(Workspace=source_ws,LogName='Psi',LogText='0',LogType='Number Series')
        SetGoniometer(Workspace=source_ws, Axis0='Psi,0,1,0,1')
        
        out_ws = gen_sqw(input_workspaces=source_ws, emode='Direct', alatt=[9.218, 9.218, 9.197], angdeg=[90, 90, 120], u=[1,0,0], v=[0,1,0])
        
        self.assertTrue(isinstance(out_ws, IMDEventWorkspace), "OutputWorkspace should be an MDEventWorkspace")
        self.assertEqual(4, out_ws.getNumDims())
        lattice = out_ws.getExperimentInfo(0).sample().getOrientedLattice()
        self.assertAlmostEqual(9.218, lattice.a())
        self.assertAlmostEqual(120, lattice.gamma())

        DeleteWorkspace(out_ws)
        DeleteWorkspace(source_ws)
  
    def test_partial_ub_throws(self):
        from MantidHorace.Command import gen_sqw, MantidHoraceInputException 
        in_ws = CreateWorkspace(DataX=[1,2], DataY=[1],DataE=[1],UnitX='DeltaE')
        try:
            gen_sqw(input_workspaces=in_ws, emode='Direct', alatt=[1,1,1], angdeg=[90, 90, 90], u=[1,0,0]) # Note that no v parameter is given, so ub info is incomplete.
        except MantidHoraceInputException:
            pass
        else:
            self.assertTrue(False, "Should have thrown. No v parameter is given, so ub info is incomplete")
        finally:
            DeleteWorkspace(in_ws)
            
    def test_partial_goniometer_throws(self):
        from MantidHorace.Command import gen_sqw, MantidHoraceInputException 
        in_ws = CreateWorkspace(DataX=[1,2], DataY=[1],DataE=[1],UnitX='DeltaE')
        try:
            gen_sqw(input_workspaces=in_ws, emode='Direct', psi=[0,1,0,1])  # partial specification of goniometer settings
        except MantidHoraceInputException:
            pass
        else:
            self.assertTrue(False, "Should have thrown. Not enough information provided for the goniometer settings")
        finally:
            DeleteWorkspace(in_ws)
     

        
            
    
        
        
        
    
if __name__ == "__main__":
    unittest.main()
    