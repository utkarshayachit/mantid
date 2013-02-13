# Algorithm to start Bayes programs
from MantidFramework import *
from mantidsimple import *

class Moments(PythonAlgorithm):
 
	def category(self):
		return "Workflow\\MIDAS;PythonAlgorithms"

	def PyInit(self):
		self.declareProperty(Name='InputType',DefaultValue='File',Validator=ListValidator(['File','Workspace']),Description = 'Origin of data input - File or Workspace')
		self.declareProperty(Name='Instrument',DefaultValue='iris',Validator=ListValidator(['irs','iris','osi','osiris']),Description = 'Instrument')
		self.declareProperty(Name='Analyser',DefaultValue='graphite002',Validator=ListValidator(['graphite002','graphite004']),Description = 'Analyser & reflection')
		self.declareProperty(Name='SamNumber',DefaultValue='',Validator=MandatoryValidator(),Description = 'Sample run number')
		self.declareProperty(Name='EnergyMin', DefaultValue=-0.5,Description = 'Minimum energy for fit. Default=-0.5')
		self.declareProperty(Name='EnergyMax', DefaultValue=0.5,Description = 'Maximum energy for fit. Default=0.5')
		self.declareProperty(Name='MultiplyBy', DefaultValue=1.0,Description = 'Scale factor to multiply S(Q,w). Default=1.0')
		self.declareProperty('Verbose',DefaultValue=True,Description = 'Switch Verbose Off/On')
		self.declareProperty('Plot',DefaultValue=True,Description = 'Switch Plot Off/On')
		self.declareProperty('Save',DefaultValue=False,Description = 'Switch Save result to nxs file Off/On')
 
	def PyExec(self):

		self.log().information('Moments calculation')
		inType = self.getPropertyValue('InputType')
		prefix = self.getPropertyValue('Instrument')
		ana = self.getPropertyValue('Analyser')
		sn = self.getPropertyValue('SamNumber')
		sam = prefix+sn+'_'+ana+'_sqw'
		emin = self.getPropertyValue('EnergyMin')
		emax = self.getPropertyValue('EnergyMax')
		erange = [float(emin), float(emax)]
		factor = self.getPropertyValue('MultiplyBy')
		factor = float(factor)

		verbOp = self.getProperty('Verbose')
		plotOp = self.getProperty('Plot')
		saveOp = self.getProperty('Save')
		workdir = config['defaultsave.directory']
		if inType == 'File':
			spath = os.path.join(workdir, sam+'.nxs')		# path name for sample nxs file
			self.log().notice('Input from File : '+spath)
			LoadNexus(Filename=spath, OutputWorkspace=sam)
		else:
			self.log().notice('Input from Workspace : '+sam)
		from IndirectEnergyConversion import SqwMoments
		SqwMoments(sam,erange,factor,verbOp,plotOp,saveOp)

mantid.registerPyAlgorithm(Moments())         # Register algorithm with Mantid
