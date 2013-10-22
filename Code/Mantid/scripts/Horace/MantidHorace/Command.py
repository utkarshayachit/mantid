from mantid.simpleapi import *

logger.warning("Mantid-Horace functionality is a Prototype. The implementation for this Mantid release is not complete.")

def __possible_emodes():
    return ['Elastic', 'Direct', 'Indirect']

def __single_gen_sqw(input_workspace, emode, alatt=[], angdeg=[], u=[], v=[]):
    
    ub_params = [alatt, angdeg, u, v]
    if any(ub_params) and not all(ub_params):
        raise ValueError("Either specify all of alatt, angledeg, u, v or none of them")
    elif all(ub_params):
        SetUB(Workspace=input_workspace, a=alatt[0], b=alatt[1], c=alatt[2], alpha=angdeg[0], beta=angdeg[1], gamma=angdeg[2], u=u, v=v)
    
    input_unit = input_workspace.getAxis(0).getUnit()
    comparitor_unit = UnitFactory.create('DeltaE')
    if comparitor_unit.unitID() != input_unit.unitID():
        raise ValueError("Units must be in Energy")
    
    min, max= ConvertToMDHelper(InputWorkspace=input_workspace, QDimensions='Q3D', dEAnalysisMode=emode)
    out_ws = ConvertToMD(InputWorkspace=input_workspace, QDimensions='Q3D', QConversionScales='HKL',dEAnalysisMode=emode, MinValues=min, MaxValues=max)
    return out_ws

def gen_sqw(input_workspaces, emode='Elastic', alatt=[], angdeg=[], u=[], v=[], psi=None, omega=None, dpsi=None, gl=None, gs=None):
    
    if not emode in __possible_emodes():
        raise ValueError("Unknown emode %s Allowed values are %s" % (emode, __possible_emodes()))
    
    converted = list() 
    if isinstance(input_workspaces, list):
        for ws in input_workspaces:
            outws = __single_gen_sqw(input_workspace=ws, emode=emode, alatt=alatt, angdeg=angdeg, u=u, v=v)
            list.append(outws)
    else:
        outws = __single_gen_sqw(input_workspace=input_workspaces, emode=emode, alatt=alatt, angdeg=angdeg, u=u, v=v) 
        return outws


    