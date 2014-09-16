# Reducers for use by ISIS Molecular Spectroscopy Group

import os.path
import os
import sys
import time
import types
import random
import string

import mantid
from mantid import simpleapi
from mantid.simpleapi import *
from mantid.kernel import config
import inelastic_indirect_reduction_steps as steps
from inelastic_indirect_reduction_steps import MSGReductionStep


def extract_workspace_name(filepath, suffix=''):
    """
    Returns a default workspace name for a given data file path.

    @param filepath: path of the file to generate a workspace name for
    @param suffix: string to append to name
    """
    filepath_tmp = filepath
    if type(filepath) is list:
        filepath_tmp = filepath[0]

    _, tail = os.path.split(filepath_tmp)
    basename, _ = os.path.splitext(tail)

    if type(filepath) is list:
        basename += "_combined"

    # TODO: check whether the workspace name is already in use
    #       and modify it if it is.

    return basename + suffix


def validate_loader(f):

    def validated_f(reducer, algorithm, *args, **kwargs):
        if issubclass(algorithm.__class__, MSGReductionStep) or algorithm is None:
            # If we have a MSGReductionStep object, just use it.
            # "None" is allowed as an algorithm (usually tells the reducer to skip a step)
            return f(reducer, algorithm)

        if isinstance(algorithm, types.FunctionType):
            # If we get a function, assume its name is an algorithm name
            algorithm = algorithm.func_name

        if isinstance(algorithm, types.StringType):
            # If we have a string, assume it's an algorithm name
            class _AlgorithmStep(MSGReductionStep):
                def __init__(self):
                    self.algorithm = None
                    self._data_file = None

                def get_algorithm(self):
                    return self.algorithm

                def setProperty(self, key, value):
                    kwargs[key] = value

                def execute(self, reducer, inputworkspace=None, outputworkspace=None):
                    """
                    Create a new instance of the requested algorithm object,
                    set the algorithm properties replacing the input and output
                    workspaces.
                    The execution will work for any combination of mandatory/optional
                    properties.

                    @param reducer: Reducer object managing the reduction
                    @param inputworkspace: input workspace name [optional]
                    @param outputworkspace: output workspace name [optional]
                    """
                    # If we don't have a data file, look up the workspace handle
                    if self._data_file is None:
                        if inputworkspace in reducer._data_files:
                            data_file = reducer._data_files[inputworkspace]
                            if data_file is None:
                                return
                        else:
                            raise RuntimeError("MSGReductionSteps.LoadRun doesn't recognize workspace handle %s" % inputworkspace)
                    else:
                        data_file = self._data_file

                    alg = mantid.api.AlgorithmManager.create(algorithm)
                    if not isinstance(alg, mantid.api.AlgorithmProxy):
                        raise RuntimeError("Reducer expects an Algorithm object from FrameworkManager, found '%s'" % str(type(alg)))

                    property_order = alg.orderedProperties()

                    # add the args to the kw list so everything can be set in a single way
                    for (key, arg) in zip(property_order[:len(args)], args):
                        kwargs[key] = arg

                    # Override input and output workspaces
                    if "Workspace" in kwargs:
                        kwargs["Workspace"] = inputworkspace
                    if "OutputWorkspace" in kwargs:
                        kwargs["OutputWorkspace"] = inputworkspace
                    if "Filename" in kwargs:
                        kwargs["Filename"] = data_file

                    if "AlternateName" in kwargs and \
                        kwargs["AlternateName"] in property_order:
                        kwargs[kwargs["AlternateName"]] = data_file

                    self.algorithm = alg
                    simpleapi._set_properties(alg, *(), **kwargs)
                    alg.execute()
                    if "OutputMessage" in property_order:
                        return alg.getPropertyValue("OutputMessage")
                    return "%s applied" % alg.name()
            return f(reducer, _AlgorithmStep())

        elif isinstance(algorithm, mantid.api.IAlgorithm) \
            or type(algorithm).__name__ == "IAlgorithm":
            class _AlgorithmStep(MSGReductionStep):
                def __init__(self):
                    self.algorithm = algorithm
                    self._data_file = None

                def get_algorithm(self):
                    return self.algorithm

                def setProperty(self, key, value):
                    kwargs[key] = value

                def execute(self, reducer, inputworkspace=None, outputworkspace=None):
                    """
                    Create a new instance of the requested algorithm object,
                    set the algorithm properties replacing the input and output
                    workspaces.
                    The execution will work for any combination of mandatory/optional
                    properties.

                    @param reducer: Reducer object managing the reduction
                    @param inputworkspace: input workspace name [optional]
                    @param outputworkspace: output workspace name [optional]
                    """
                    # If we don't have a data file, look up the workspace handle
                    if self._data_file is None:
                        if inputworkspace in reducer._data_files:
                            data_file = reducer._data_files[inputworkspace]
                            if data_file is None:
                                return
                        else:
                            raise RuntimeError("MSGReductionSteps.LoadRun doesn't recognize workspace handle %s" % inputworkspace)
                    else:
                        data_file = self._data_file

                    property_order = algorithm.orderedProperties()

                    # Override input and output workspaces
                    if "Workspace" in property_order:
                        algorithm.setPropertyValue("Workspace", inputworkspace)
                    if "OutputWorkspace" in property_order:
                        algorithm.setPropertyValue("OutputWorkspace", inputworkspace)
                    if "Filename" in property_order:
                        algorithm.setPropertyValue("Filename", data_file)

                    if "AlternateName" in kwargs and \
                        kwargs["AlternateName"] in property_order:
                        algorithm.setPropertyValue(kwargs["AlternateName"], data_file)

                    algorithm.execute()
                    return "%s applied" % algorithm.name()
            return f(reducer, _AlgorithmStep())

        else:
            raise RuntimeError("%s expects a MSGReductionStep object, found %s" % (f.__name__, algorithm.__class__))
    return validated_f


def validate_step(f):
    """
    Decorator for Reducer methods that need a MSGReductionStep
    object as its first argument.

    Example:
        @validate_step
        def some_func(self, reduction_step):
            [...]

    Arguments to a Mantid algorithm function should be passed as arguments.
    Example:
        #Load("my_file.txt", "my_wksp") will become:
        reducer.some_func(Load, "my_file.txt", "my_wksp")

    InputWorkspace and OutputWorkspace arguments can be left as None
    if they are to be overwritten by the Reducer.
    """

    def validated_f(reducer, algorithm, *args, **kwargs):
        """
        Wrapper function around the function f.
        The function ensures that the algorithm parameter
        is a sub-class of MSGReductionStep

        @param algorithm: algorithm name, MSGReductionStep object, or Mantid algorithm function
        """
        if issubclass(algorithm.__class__, MSGReductionStep) or algorithm is None:
            # If we have a MSGReductionStep object, just use it.
            # "None" is allowed as an algorithm (usually tells the reducer to skip a step)
            return f(reducer, algorithm)

        if isinstance(algorithm, types.FunctionType):
            # If we get a function, assume its name is an algorithm name
            algorithm = algorithm.func_name

        if isinstance(algorithm, types.StringType):
            # If we have a string, assume it's an algorithm name
            class _AlgorithmStep(MSGReductionStep):
                def __init__(self):
                    self.algorithm = None

                def get_algorithm(self):
                    return self.algorithm

                def setProperty(self, key, value):
                    kwargs[key] = value

                def execute(self, reducer, inputworkspace=None, outputworkspace=None):
                    """
                    Create a new instance of the requested algorithm object,
                    set the algorithm properties replacing the input and output
                    workspaces.
                    The execution will work for any combination of mandatory/optional
                    properties.

                    @param reducer: Reducer object managing the reduction
                    @param inputworkspace: input workspace name [optional]
                    @param outputworkspace: output workspace name [optional]
                    """
                    if outputworkspace is None:
                        outputworkspace = inputworkspace
                    alg = mantid.AlgorithmManager.create(algorithm)
                    if not isinstance(alg, mantid.api.AlgorithmProxy):
                        raise RuntimeError("Reducer expects an Algorithm object from FrameworkManager, found '%s'" % str(type(alg)))

                    property_order = alg.orderedProperties()

                    # add the args to the kw list so everything can be set in a single way
                    for (key, arg) in zip(property_order[:len(args)], args):
                        kwargs[key] = arg

                    # Override input and output workspaces
                    if "Workspace" in kwargs:
                        kwargs["Workspace"] = inputworkspace
                    if "InputWorkspace" in kwargs:
                        kwargs["InputWorkspace"] = inputworkspace
                    if "OutputWorkspace" in kwargs:
                        kwargs["OutputWorkspace"] = outputworkspace

                    self.algorithm = alg
                    simpleapi._set_properties(alg, *(), **kwargs)
                    alg.execute()
                    if "OutputMessage" in property_order:
                        return alg.getPropertyValue("OutputMessage")
                    return "%s applied" % alg.name()
            return f(reducer, _AlgorithmStep())

        elif isinstance(algorithm, mantid.api.IAlgorithm) \
            or type(algorithm).__name__ == "IAlgorithm":
            class _AlgorithmStep(MSGReductionStep):
                def __init__(self):
                    self.algorithm = algorithm

                def get_algorithm(self):
                    return self.algorithm

                def setProperty(self, key, value):
                    kwargs[key] = value

                def execute(self, reducer, inputworkspace=None, outputworkspace=None):
                    """
                    Create a new instance of the requested algorithm object,
                    set the algorithm properties replacing the input and output
                    workspaces.
                    The execution will work for any combination of mandatory/optional
                    properties.

                    @param reducer: Reducer object managing the reduction
                    @param inputworkspace: input workspace name [optional]
                    @param outputworkspace: output workspace name [optional]
                    """
                    if outputworkspace is None:
                        outputworkspace = inputworkspace
                    property_order = algorithm.orderedProperties()

                    # Override input and output workspaces
                    if "Workspace" in property_order:
                        algorithm.setPropertyValue("Workspace", inputworkspace)
                    if "InputWorkspace" in property_order:
                        algorithm.setPropertyValue("InputWorkspace", inputworkspace)
                    if "OutputWorkspace" in property_order:
                        algorithm.setPropertyValue("OutputWorkspace", outputworkspace)

                    algorithm.execute()
                    if "OutputMessage" in property_order:
                        return algorithm.getPropertyValue("OutputMessage")
                    return "%s applied" % algorithm.name()
            return f(reducer, _AlgorithmStep())

        else:
            raise RuntimeError("%s expects a MSGReductionStep object, found %s" % (f.__name__, algorithm.__class__))
    return validated_f


class MSGReducer(object):
    """
    This is the base class for the reducer classes to be used by the ISIS
    Molecular Spectroscopy Group (MSG). It exists to serve the functions that
    are common to both spectroscopy and diffraction workflows in the hopes of
    providing a semi-consistent interface to both.
    """

    _data_files = {}            # List of data files to process
    _reduction_steps = []       # List of reduction steps
    output_workspaces = []      # Output workspaces

    _instrument_name = None     # Name of the instrument used in experiment.
    _sum = False                # Whether to sum input files or treat them sequentially.
    _load_logs = False          # Whether to load the log file(s) associated with the raw file.
    _multiple_frames = False
    _detector_range = [-1, -1]
    _masking_detectors = {}
    _parameter_file = None
    _rebin_string = None
    _fold_multiple_frames = True
    _save_formats = []
    _info_table_props = None
    _extra_load_opts = {}

    def __init__(self):
        self.UID = ''.join(random.choice(string.ascii_lowercase + string.ascii_uppercase + string.digits) for x in range(5))
        self._data_files = {}
        self._reduction_steps = []

    def _setup_steps(self):
        """
        This should be implemented in the derived class.
        """
        raise RuntimeError('No setup_steps implementation in class derived from MSGReducer')

    def pre_process(self):
        self._reduction_steps = []

        load_data = steps.LoadData()
        load_data.set_ws_list(self._data_files)
        load_data.set_sum(self._sum)
        load_data.set_load_logs(self._load_logs)
        load_data.set_detector_range(self._detector_range[0], self._detector_range[1])
        load_data.set_parameter_file(self._parameter_file)
        load_data.set_extra_load_opts(self._extra_load_opts)
        load_data.execute(self, None)

        self._multiple_frames = load_data.is_multiple_frames()

        if self._info_table_props is not None:
            ws_names = load_data.get_ws_list().keys()
            ws_name_list = ", ".join(ws_names)
            props_list = ", ".join(self._info_table_props)
            CreateLogPropertyTable(
                OutputWorkspace="RunInfo",
                InputWorkspaces=ws_name_list,
                LogPropertyNames=props_list,
                GroupPolicy="First")

        if self._sum:
            self._data_files = load_data.get_ws_list()

        self._setup_steps()

    def set_detector_range(self, start, end):
        """
        Sets the start and end detector points for the reduction process.
        These numbers are to be the *workspace index*, not the spectrum number.
        Example:
            reducer.set_detector_range(2,52)
        """
        if not isinstance(start, int) or not isinstance(end, int):
            raise TypeError("start and end must be integer values")
        self._detector_range = [start, end]

    def set_fold_multiple_frames(self, value):
        """
        When this is set to False, the reducer will not run the FoldData
        reduction step or any step which appears after it in the reduction
        chain.
        This will only affect data which would ordinarily have used this
        function (ie TOSCA on multiple frames).
        """
        if not isinstance(value, bool):
            raise TypeError("value must be of boolean type")
        self._fold_multiple_frames = value

    def set_instrument_name(self, instrument):
        """
        Unlike the SANS reducers, we do not create a class to describe the
        instruments. Instead, we load the instrument and parameter file and
        query it for information.
        Raises:
            * ValueError if an instrument name is not provided.
            * RuntimeError if IDF could not be found or is invalid.
            * RuntimeError if workspace index of the Monitor could not be
                determined.
        Example use:
            reducer.set_instrument_name("IRIS")
        """
        if not isinstance(instrument, str):
            raise ValueError("Instrument name must be given.")
        self._instrument_name = instrument

    def set_parameter_file(self, file_name):
        """
        Sets the parameter file to be used in the reduction. The parameter
        file will contain some settings that are used throughout the reduction
        process.
        Note: This is *not* the base parameter file, ie "IRIS_Parameters.xml"
        but, rather, the additional parameter file.
        """
        self._parameter_file = \
            os.path.join(config["parameterDefinition.directory"], file_name)

    def set_rebin_string(self, rebin):
        """
        Sets the rebin string to be used with the Rebin algorithm.
        """
        if not isinstance(rebin, str):
            raise TypeError("rebin variable must be of string type")
        self._rebin_string = rebin

    def set_sum_files(self, value):
        """
        Mark whether multiple runs should be summed together for the process
        or treated individually.
        The default value for this is False.
        """
        if not isinstance(value, bool):
            raise TypeError("value must be either True or False (boolean)")
        self._sum = value

    def set_load_logs(self, value):
        """
        Mark whether the log file(s) associated with a raw file should be
        loaded along with the raw file.
        The default value for this is False.
        """
        if not isinstance(value, bool):
            raise TypeError("value must be either True or False (boolean)")
        self._load_logs = value

    def set_save_formats(self, formats):
        """
        Selects the save formats in which to export the reduced data.
        formats should be a list object of strings containing the file
        extension that signifies the type.
        For example:
            reducer.set_save_formats(['nxs', 'spe'])
        Tells the reducer to save the final result as a NeXuS file, and as an
        SPE file.
        Please see the documentation for the SaveItem reduction step for more
        details.
        """
        if not isinstance(formats, list):
            raise TypeError("formats variable must be of list type")
        self._save_formats = formats

    def append_load_option(self, name, value):
        """
        Additional options for the Load call, require name & value
        of property
        """
        self._extra_load_opts[name] = value

    def get_result_workspaces(self):
        """
        Returns a Python list object containing the names of the workspaces
        processed at the last reduction step. Using this, you can incorporate
        the reducer into your own scripts.
        It will only be effective after the reduce() function has run.
        Example:
            wslist = reducer.get_result_workspaces()
            plotSpectrum(wslist, 0) # Plot the first spectrum of each of
                    # the result workspaces
        """
        nsteps = len(self._reduction_steps)
        for i in range(0, nsteps):
            try:
                step = self._reduction_steps[nsteps - (i + 1)]
                return step.get_result_workspaces()
            except AttributeError:
                pass
            except IndexError:
                raise RuntimeError("None of the reduction steps implement "
                                   "the get_result_workspaces() method.")

    def _get_monitor_index(self, workspace):
        """
        Determine the workspace index of the first monitor spectrum.
        """
        inst = workspace.getInstrument()
        try:
            monitor_index = inst.getNumberParameter('Workflow.Monitor1-SpectrumNumber')[0]
            return int(monitor_index)
        except IndexError:
            raise ValueError('Unable to retrieve spectrum number of monitor.')

    @validate_step
    def append_step(self, reduction_step):
        """
        Append a reduction step

        @param reduction_step: MSGReductionStep object
        """
        if reduction_step is None:
            return None

        self._reduction_steps.append(reduction_step)

        return reduction_step

    def clear_data_files(self):
        """
        Empty the list of files to reduce while keeping all the
        other options the same.
        """
        self._data_files = {}

    def append_data_file(self, data_file, workspace=None):
        """
        Append a file to be processed.

        @param data_file: name of the file to be processed
        @param workspace: optional name of the workspace for this data,
            default will be the name of the file
        TODO: this needs to be an ordered list
        """
        if data_file is None:
            if workspace in mtd:
                self._data_files[workspace] = None
                return
            else:
                raise RuntimeError("Trying to append a data set without a file name or an existing workspace.")
        if type(data_file) is list:
            if workspace is None:
                # Use the first file to determine the workspace name
                workspace = extract_workspace_name(data_file[0])
        else:
            if workspace is None:
                workspace = extract_workspace_name(data_file)

        self._data_files[workspace] = data_file

    def reduce(self):
        """
        Go through the list of reduction steps.
        """
        self.output_workspaces = []

        logger.notice('%s reduction started' % self._instrument_name)

        # Go through the list of steps that are common to all data files
        self.pre_process()

        # Go through the list of files to be reduced
        for file_ws in self._data_files:
            logger.information('Reducing: %s' % file_ws)
            for item in self._reduction_steps:
                try:
                    result = item.execute(self, file_ws)
                    if result is not None and len(str(result)) > 0:
                        logger.information(result)
                except:
                    logger.notice(sys.exc_value)
                    raise

        logger.notice('%s reduction completed' % self._instrument_name)
