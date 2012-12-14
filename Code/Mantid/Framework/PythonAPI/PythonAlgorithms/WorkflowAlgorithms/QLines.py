# Algorithm to start Bayes programs
from MantidFramework import *
from mantid.simpleapi import *
from mantid import config, logger, mtd
from IndirectImport import run_f2py_compatibility_test, is_supported_f2py_platform

if is_supported_f2py_platform():
	import IndirectBayes as Main

class QLines(PythonAlgorithm):
 
	def category(self):
		return "Workflow\\MIDAS;PythonAlgorithms"

	def PyInit(self):
		self.declareProperty(Name='InputType',DefaultValue='File',Validator=ListValidator(['File','Workspace']),Description = 'Origin of data input - File (*.nxs) or Workspace')
		self.declareProperty(Name='Instrument',DefaultValue='iris',Validator=ListValidator(['irs','iris','osi','osiris']),Description = 'Instrument')
		self.declareProperty(Name='Analyser',DefaultValue='graphite002',Validator=ListValidator(['graphite002','graphite004']),Description = 'Analyser & reflection')
		self.declareProperty(Name='Program',DefaultValue='QL',Validator=ListValidator(['QL','QSe']),Description = 'Name of program to run')
		self.declareProperty(Name='SamNumber',DefaultValue='',Validator=MandatoryValidator(),Description = 'Sample run number')
		self.declareProperty(Name='ResInputType',DefaultValue='File',Validator=ListValidator(['File','Workspace']),Description = 'Origin of res input - File (*_res.nxs) or Workspace')
		self.declareProperty(Name='ResType',DefaultValue='Res',Validator=ListValidator(['Res','Data']),Description = 'Format of Resolution file')
		self.declareProperty(Name='ResNumber',DefaultValue='',Validator=MandatoryValidator(),Description = 'Resolution run number')
		self.declareProperty(Name='BackgroundOption',DefaultValue='Sloping',Validator=ListValidator(['Sloping','Flat','Zero']),Description = 'Form of background to fit')
		self.declareProperty(Name='ElasticOption',DefaultValue=True,Description = 'Include elastic peak in fit')
		self.declareProperty(Name='FixWidth',DefaultValue=False,Description = 'Fix one of the widths')
		self.declareProperty(Name='WidthFile', DefaultValue='',Description = 'Name of file containing fixed width values')
		self.declareProperty(Name='ResNorm',DefaultValue=False,Description = 'Use ResNorm output file')
		self.declareProperty(Name='ResNormNumber', DefaultValue='',Description = 'Name of file containing fixed width values')
		self.declareProperty(Name='EnergyMin', DefaultValue=-0.5,Description = 'Minimum energy for fit. Default=-0.5')
		self.declareProperty(Name='EnergyMax', DefaultValue=0.5,Description = 'Maximum energy for fit. Default=0.5')
		self.declareProperty(Name='SamBinning', DefaultValue=1,Description = 'Binning value (integer) for sample. Default=1')
		self.declareProperty(Name='ResBinning', DefaultValue=1,Description = 'Binning value (integer) for resolution - QLd only. Default=1')
		self.declareProperty(Name='Sequence',DefaultValue=True,Description = 'Switch Sequence Off/On')
		self.declareProperty(Name='Plot',DefaultValue='None',Validator=ListValidator(['None','ProbBeta','Intensity','FwHm','Fit','All']),Description = 'Plot options')
		self.declareProperty(Name='Verbose',DefaultValue=True,Description = 'Switch Verbose Off/On')
		self.declareProperty(Name='Save',DefaultValue=False,Description = 'Switch Save result to nxs file Off/On')
 
	def PyExec(self):
		run_f2py_compatibility_test()
		
		self.log().information('QLines input')
		inType = self.getPropertyValue('InputType')
		prefix = self.getPropertyValue('Instrument')
		ana = self.getPropertyValue('Analyser')
		prog = self.getPropertyValue('Program')
		sam = self.getPropertyValue('SamNumber')
		rinType = self.getPropertyValue('ResInputType')
		rtype = self.getPropertyValue('ResType')
		res = self.getPropertyValue('ResNumber')
		elastic = self.getProperty('ElasticOption')
		bgd = self.getPropertyValue('BackgroundOption')
		width = self.getProperty('FixWidth')
		wfile = self.getPropertyValue('WidthFile')
		resnorm = self.getProperty('ResNorm')
		resn = self.getPropertyValue('ResNormNumber')
		emin = self.getPropertyValue('EnergyMin')
		emax = self.getPropertyValue('EnergyMax')
		nbin = self.getPropertyValue('SamBinning')
		nrbin = self.getPropertyValue('ResBinning')
		nbins = [nbin, nrbin]

		if rtype == 'Res':
			rext = 'res'
		if rtype == 'Data':
			rext = 'red'
		sname = prefix+sam+'_'+ana + '_red'
		rname = prefix+res+'_'+ana + '_' + rext
		rsname = prefix+resn+'_'+ana + '_ResNorm_Paras'
		erange = [float(emin), float(emax)]
		if elastic:
			o_el = 1
		else:
			o_el = 0
		if bgd == 'Sloping':
			o_bgd = 2
		if bgd == 'Flat':
			o_bgd = 1
		if bgd == 'Zero':
			o_bgd = 0
		if width:
			o_w1 = 1
		else:
			o_w1 = 0
		if resnorm:
			o_res = 1
		else:
			o_res = 0
		fitOp = [o_el, o_bgd, o_w1, o_res]
		loopOp = self.getProperty('Sequence')
		verbOp = self.getProperty('Verbose')
		plotOp = self.getPropertyValue('Plot')
		saveOp = self.getProperty('Save')

		workdir = config['defaultsave.directory']
		if inType == 'File':
			spath = os.path.join(workdir, sname+'.nxs')		# path name for sample nxs file
			LoadNexusProcessed(Filename=spath, OutputWorkspace=sname)
			Smessage = 'Sample from File : '+spath
		else:
			Smessage = 'Sample from Workspace : '+sname
		if rinType == 'File':
			rpath = os.path.join(workdir, rname+'.nxs')		# path name for res nxs file
			LoadNexusProcessed(Filename=rpath, OutputWorkspace=rname)
			Rmessage = 'Resolution from File : '+rpath
		else:
			Rmessage = 'Resolution from Workspace : '+rname
		if verbOp:
			logger.notice(Smessage)
			logger.notice(Rmessage)
		if fitOp[3] == 1:
			path = os.path.join(workdir, rsname+'.nxs')	# path name for resnnrm nxs file
			LoadNexusProcessed(Filename=path, OutputWorkspace='ResNorm')
		Main.QLRun(prog,sname,rname,rsname,erange,nbins,fitOp,wfile,loopOp,verbOp,plotOp,saveOp)

mantid.registerPyAlgorithm(QLines())         # Register algorithm with Mantid