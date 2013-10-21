from mantid.simpleapi import *

logger.warning("Mantid-Horace functionality is a Prototype. The implementation for this Mantid release is not complete.")

def __possible_emodes():
    return ['Elastic', 'Direct', 'Indirect']

def __single_gen_sqw(input_workspace, emode):
    input_unit = input_workspace.getAxis(0).getUnit()
    comparitor_unit = UnitFactory.create('DeltaE')
    if comparitor_unit.unitID() != input_unit.unitID():
        raise ValueError("Units must be in Energy")
    
    min, max= ConvertToMDHelper(InputWorkspace=input_workspace, QDimensions='Q3D', dEAnalysisMode=emode)
    out_ws = ConvertToMD(InputWorkspace=input_workspace, QDimensions='Q3D', QConversionScales='HKL',dEAnalysisMode=emode, MinValues=min, MaxValues=max)
    return out_ws

def gen_sqw(input_workspaces, emode='Elastic', alatt=None, angledeg=None, u=None, v=None, psi=None, omega=None, dpsi=None, gl=None, gs=None):
    
    if not emode in __possible_emodes():
        raise ValueError("Unknown emode %s Allowed values are %s" % (emode, __possible_emodes()))
    
    converted = list() 
    if isinstance(input_workspaces, list):
        for ws in input_workspaces:
            outws = __single_gen_sqw(input_workspace=ws, emode=emode)
            list.append(outws)
    else:
        outws = __single_gen_sqw(input_workspace=input_workspaces, emode=emode) 
        return outws


    