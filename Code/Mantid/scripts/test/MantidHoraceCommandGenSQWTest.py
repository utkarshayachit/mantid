import unittest
from mantid.simpleapi import *
from mantid.api import IMDEventWorkspace

class MantidHoraceCommandGenSQWTest(unittest.TestCase):
    
    def test_import(self):
        try:
            import MantidHorace.Command
        except ImportError:
            self.assertTrue(False, "Imports should not have failed")
        
    
    def test_units_of_deltae_only(self):
        from MantidHorace.Command import gen_sqw 
      
        tofWS = CreateWorkspace(DataX=[1,2], DataY=[1],DataE=[1],UnitX='TOF')
        try:
            gen_sqw(input_workspaces=tofWS)
            self.assertTrue(False, "Should have thrown with an input workspace in TOF")
        except ValueError:
            pass
        finally:
            DeleteWorkspace(tofWS)
        
    def test_known_emode_only(self):
        from MantidHorace.Command import gen_sqw 
        in_ws = CreateWorkspace(DataX=[1,2], DataY=[1],DataE=[1],UnitX='DeltaE')
        try:
            gen_sqw(input_workspaces=in_ws, emode='Unknown')
        except ValueError:
            pass
        finally:
            DeleteWorkspace(in_ws)
            
    def test_single_workspace(self):
        from MantidHorace.Command import gen_sqw 
        
        source_ws = Load(Filename='CNCS_7860_event')

        source_ws = ConvertUnits(InputWorkspace=source_ws, Target='DeltaE', EMode='Direct', EFixed=3)
        SetUB(Workspace=source_ws,a=1.4165,b=1.4165,c=1.4165,u=[1,0,0],v=[0,1,0])    
        AddSampleLog(Workspace=source_ws,LogName='Psi',LogText='0',LogType='Number Series')
        SetGoniometer(Workspace=source_ws, Axis0='Psi,0,1,0,1')
        
        out_ws = gen_sqw(input_workspaces=source_ws, emode='Direct')
        
        print out_ws.getNEvents(), source_ws.getNEvents()
        self.assertTrue(isinstance(out_ws, IMDEventWorkspace), "OutputWorkspace should be an MDEventWorkspace")
        self.assertEqual(4, out_ws.getNumDims())
        
        DeleteWorkspace(out_ws)
        DeleteWorkspace(source_ws)
        
        
     

        
            
    
        
        
        
    
if __name__ == "__main__":
    unittest.main()
    