from mantid.simpleapi import *

logger.warning("Mantid-Horace functionality is a Prototype. The implementation for this Mantid release is not complete.")

class MantidHoraceInputException(ValueError):
    pass

def __possible_emodes():
    return ['Elastic', 'Direct', 'Indirect']

def __single_gen_sqw(input_workspace, emode, alatt=[], angdeg=[], u=[], v=[], psi=None, omega=None, dpsi=None, gl=None, gs=None):
    import numpy as np
    ub_params = [alatt, angdeg, u, v]
    goniometer_params = [psi, omega, dpsi, gl, gs]
    if any(ub_params) and not all(ub_params):
        raise MantidHoraceInputException("Either specify all of alatt, angledeg, u, v or none of them")
    elif all(ub_params):
        SetUB(Workspace=input_workspace, a=alatt[0], b=alatt[1], c=alatt[2], alpha=angdeg[0], beta=angdeg[1], gamma=angdeg[2], u=u, v=v)
    if any(goniometer_params) and not all(goniometer_params):
        raise MantidHoraceInputException("Either specify all of psi, omega, dpsi, gl, gs or none of them")
    elif all(goniometer_params):
        AddSampleLog(Workspace=input_workspace, LogName='gl', LogText=gl, LogType='Number')
        AddSampleLog(Workspace=input_workspace, LogName='gs', LogText=gs, LogType='Number')
        AddSampleLog(Workspace=input_workspace, LogName='dpsi', LogText=dpsi, LogType='Number')
        AddSampleLog(Workspace=input_workspace, LogName='omega', LogText=omega, LogType='Number')
        AddSampleLog(Workspace=input_workspace, LogName='psi', LogText=psi, LogType='Number')
        
        up = np.cross(u, v)
        axis0 = 'gl' + ','.join(map(str, u))
        axis1 = 'gs' + ','.join(map(str, v))
        
        '''TODO'''
        raise RuntimeError("The implementation here is incomplete.")
        SetGoniometer(Workspace=input_workspace, Axis0=axis0, Axis1=axis1, Axis2=axis2, Axis3=axis3, Axis4=axis4)
    
    input_unit = input_workspace.getAxis(0).getUnit()
    comparitor_unit = UnitFactory.create('DeltaE')
    if comparitor_unit.unitID() != input_unit.unitID():
        raise MantidHoraceInputException("Units must be in Energy")
    
    min, max= ConvertToMDHelper(InputWorkspace=input_workspace, QDimensions='Q3D', dEAnalysisMode=emode)
    out_ws = ConvertToMD(InputWorkspace=input_workspace, QDimensions='Q3D', QConversionScales='HKL',dEAnalysisMode=emode, MinValues=min, MaxValues=max)
    return out_ws

def gen_sqw(input_workspaces, emode='Elastic', alatt=[], angdeg=[], u=[], v=[], psi=None, omega=None, dpsi=None, gl=None, gs=None):
    
    if not emode in __possible_emodes():
        raise MantidHoraceInputException("Unknown emode %s Allowed values are %s" % (emode, __possible_emodes()))
    
    converted = list() 
    if isinstance(input_workspaces, list):
        for ws in input_workspaces:
            outws = __single_gen_sqw(input_workspace=ws, emode=emode, alatt=alatt, angdeg=angdeg, u=u, v=v, psi=psi, omega=omega, dpsi=dpsi, gl=gl, gs=gs)
            list.append(outws)
    else:
        outws = __single_gen_sqw(input_workspace=input_workspaces, emode=emode, alatt=alatt, angdeg=angdeg, u=u, v=v, psi=psi, omega=omega, dpsi=dpsi, gl=gl, gs=gs) 
        return outws


    