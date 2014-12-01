#include "MantidQtCustomInterfaces/ConvFit.h"

#include "MantidQtCustomInterfaces/UserInputValidator.h"
#include "MantidQtMantidWidgets/RangeSelector.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/FunctionDomain1D.h"

#include <QDoubleValidator>
#include <QFileInfo>
#include <QMenu>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace
{
  Mantid::Kernel::Logger g_log("ConvFit");
}

namespace MantidQt
{
namespace CustomInterfaces
{
namespace IDA
{
  ConvFit::ConvFit(QWidget * parent) : 
    IDATab(parent),
    m_stringManager(NULL), m_cfTree(NULL), 
    m_fixedProps(),
    m_cfInputWS(), m_cfInputWSName(),
    m_confitResFileType()
  {
  }
  
  void ConvFit::setup()
  {
    // Create Property Managers
    m_stringManager = new QtStringPropertyManager();

    // Create TreeProperty Widget
    m_cfTree = new QtTreePropertyBrowser();
    uiForm().confit_properties->addWidget(m_cfTree);

    // add factories to managers
    m_cfTree->setFactoryForManager(m_blnManager, qtCheckBoxFactory());
    m_cfTree->setFactoryForManager(m_dblManager, doubleEditorFactory());

    // Create Plot Widget
    m_plots["ConvFitPlot"] = new QwtPlot(m_parentWidget);
    m_plots["ConvFitPlot"]->setAxisFont(QwtPlot::xBottom, m_parentWidget->font());
    m_plots["ConvFitPlot"]->setAxisFont(QwtPlot::yLeft, m_parentWidget->font());
    m_plots["ConvFitPlot"]->setCanvasBackground(Qt::white);
    uiForm().confit_plot->addWidget(m_plots["ConvFitPlot"]);

    // Create Range Selectors
    m_rangeSelectors["ConvFitRange"] = new MantidQt::MantidWidgets::RangeSelector(m_plots["ConvFitPlot"]);
    m_rangeSelectors["ConvFitBackRange"] = new MantidQt::MantidWidgets::RangeSelector(m_plots["ConvFitPlot"], 
      MantidQt::MantidWidgets::RangeSelector::YSINGLE);
    m_rangeSelectors["ConvFitBackRange"]->setColour(Qt::darkGreen);
    m_rangeSelectors["ConvFitBackRange"]->setRange(0.0, 1.0);
    m_rangeSelectors["ConvFitHWHM"] = new MantidQt::MantidWidgets::RangeSelector(m_plots["ConvFitPlot"]);
    m_rangeSelectors["ConvFitHWHM"]->setColour(Qt::red);

    // Populate Property Widget

    // Option to convolve members
    m_properties["Convolve"] = m_blnManager->addProperty("Convolve");
    m_cfTree->addProperty(m_properties["Convolve"]);
    m_blnManager->setValue(m_properties["Convolve"], true);

    m_properties["FitRange"] = m_grpManager->addProperty("Fitting Range");
    m_properties["StartX"] = m_dblManager->addProperty("StartX");
    m_dblManager->setDecimals(m_properties["StartX"], NUM_DECIMALS);
    m_properties["EndX"] = m_dblManager->addProperty("EndX");
    m_dblManager->setDecimals(m_properties["EndX"], NUM_DECIMALS);
    m_properties["FitRange"]->addSubProperty(m_properties["StartX"]);
    m_properties["FitRange"]->addSubProperty(m_properties["EndX"]);
    m_cfTree->addProperty(m_properties["FitRange"]);

    m_properties["LinearBackground"] = m_grpManager->addProperty("Background");
    m_properties["BGA0"] = m_dblManager->addProperty("A0");
    m_dblManager->setDecimals(m_properties["BGA0"], NUM_DECIMALS);
    m_properties["BGA1"] = m_dblManager->addProperty("A1");
    m_dblManager->setDecimals(m_properties["BGA1"], NUM_DECIMALS);
    m_properties["LinearBackground"]->addSubProperty(m_properties["BGA0"]);
    m_properties["LinearBackground"]->addSubProperty(m_properties["BGA1"]);
    m_cfTree->addProperty(m_properties["LinearBackground"]);

    // Delta Function
    m_properties["DeltaFunction"] = m_grpManager->addProperty("Delta Function");
    m_properties["UseDeltaFunc"] = m_blnManager->addProperty("Use");
    m_properties["DeltaHeight"] = m_dblManager->addProperty("Height");
    m_dblManager->setDecimals(m_properties["DeltaHeight"], NUM_DECIMALS);
    m_properties["DeltaFunction"]->addSubProperty(m_properties["UseDeltaFunc"]);
    m_cfTree->addProperty(m_properties["DeltaFunction"]);

    m_properties["Lorentzian1"] = createLorentzian("Lorentzian 1");
    m_properties["Lorentzian2"] = createLorentzian("Lorentzian 2");

    uiForm().confit_leTempCorrection->setValidator(new QDoubleValidator(m_parentWidget));

    // Connections
    connect(m_rangeSelectors["ConvFitRange"], SIGNAL(minValueChanged(double)), this, SLOT(minChanged(double)));
    connect(m_rangeSelectors["ConvFitRange"], SIGNAL(maxValueChanged(double)), this, SLOT(maxChanged(double)));
    connect(m_rangeSelectors["ConvFitBackRange"], SIGNAL(minValueChanged(double)), this, SLOT(backgLevel(double)));
    connect(m_rangeSelectors["ConvFitHWHM"], SIGNAL(minValueChanged(double)), this, SLOT(hwhmChanged(double)));
    connect(m_rangeSelectors["ConvFitHWHM"], SIGNAL(maxValueChanged(double)), this, SLOT(hwhmChanged(double)));
    connect(m_dblManager, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(updateRS(QtProperty*, double)));
    connect(m_blnManager, SIGNAL(valueChanged(QtProperty*, bool)), this, SLOT(checkBoxUpdate(QtProperty*, bool)));
    connect(m_dblManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(plotGuess(QtProperty*)));
    connect(uiForm().confit_ckTempCorrection, SIGNAL(toggled(bool)), uiForm().confit_leTempCorrection, SLOT(setEnabled(bool)));

    // Have FWHM Range linked to Fit Start/End Range
    connect(m_rangeSelectors["ConvFitRange"], SIGNAL(rangeChanged(double, double)), m_rangeSelectors["ConvFitHWHM"], SLOT(setRange(double, double)));
    m_rangeSelectors["ConvFitHWHM"]->setRange(-1.0,1.0);
    hwhmUpdateRS(0.02);

    typeSelection(uiForm().confit_cbFitType->currentIndex());
    bgTypeSelection(uiForm().confit_cbBackground->currentIndex());

    // Replot input automatically when file / spec no changes
    connect(uiForm().confit_lePlotSpectrum, SIGNAL(editingFinished()), this, SLOT(plotInput()));
    connect(uiForm().confit_dsSampleInput, SIGNAL(dataReady(const QString&)), this, SLOT(plotInput()));
    
    connect(uiForm().confit_cbFitType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeSelection(int)));
    connect(uiForm().confit_cbBackground, SIGNAL(currentIndexChanged(int)), this, SLOT(bgTypeSelection(int)));
    connect(uiForm().confit_pbSingle, SIGNAL(clicked()), this, SLOT(singleFit()));

    uiForm().confit_lePlotSpectrum->setValidator(m_valInt);
    uiForm().confit_leSpectraMin->setValidator(m_valInt);
    uiForm().confit_leSpectraMax->setValidator(m_valInt);

    // Context menu
    m_cfTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_cfTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(fitContextMenu(const QPoint &)));

    // Tie
    connect(uiForm().confit_cbFitType,SIGNAL(currentIndexChanged(QString)),SLOT(showTieCheckbox(QString)));
    showTieCheckbox( uiForm().confit_cbFitType->currentText() );
  }

  void ConvFit::run()
  {
    if ( m_cfInputWS == NULL )
    {
      return;
    }

    QString fitType = fitTypeString();
    QString bgType = backgroundString();

    if(fitType == "")
    {
      g_log.error("No fit type defined");
    }

    bool useTies = uiForm().confit_ckTieCentres->isChecked();
    QString ties = (useTies ? "True" : "False");

    Mantid::API::CompositeFunction_sptr func = createFunction(useTies);
    std::string function = std::string(func->asString());
    QString stX = m_properties["StartX"]->valueText();
    QString enX = m_properties["EndX"]->valueText();
    QString specMin = uiForm().confit_leSpectraMin->text();
    QString specMax = uiForm().confit_leSpectraMax->text();

    QString pyInput =
      "from IndirectDataAnalysis import confitSeq\n"
      "input = '" + m_cfInputWSName + "'\n"
      "func = r'" + QString::fromStdString(function) + "'\n"
      "startx = " + stX + "\n"
      "endx = " + enX + "\n"
      "plot = '" + uiForm().confit_cbPlotOutput->currentText() + "'\n"
      "ties = " + ties + "\n"
      "save = ";
  
    if(specMin != "")
      pyInput += "specMin = " + specMin + "\n";

    if(specMax != "")
      pyInput += "specMax = " + specMax + "\n";

    pyInput += uiForm().confit_ckSaveSeq->isChecked() ? "True\n" : "False\n";

    if ( m_blnManager->value(m_properties["Convolve"]) ) pyInput += "convolve = True\n";
    else pyInput += "convolve = False\n";

    if ( uiForm().confit_ckVerbose->isChecked() ) pyInput += "verbose = True\n";
    else pyInput += "verbose = False\n";

    QString temperature = uiForm().confit_leTempCorrection->text();
    bool useTempCorrection = (!temperature.isEmpty() && uiForm().confit_ckTempCorrection->isChecked());
    if ( useTempCorrection ) 
    {
      pyInput += "temp=" + temperature + "\n";
    }
    else
    {
      pyInput += "temp=None\n";
    }
  
    pyInput +=    
      "bg = '" + bgType + "'\n"
      "ftype = '" + fitType + "'\n"
      "confitSeq(input, func, startx, endx, ftype, bg, temp, specMin, specMax, convolve, Verbose=verbose, Plot=plot, Save=save)\n";

    QString pyOutput = runPythonCode(pyInput);

    // Set the result workspace for Python script export
    QString inputWsName = QString::fromStdString(m_cfInputWS->getName());
    QString resultWsName = inputWsName.left(inputWsName.lastIndexOf("_")) + "_conv_" + fitType + bgType + specMin + "_to_" + specMax + "_Workspaces";
    m_pythonExportWsName = resultWsName.toStdString();
  }

  /**
   * Validates the user's inputs in the ConvFit tab.
   */
  bool ConvFit::validate()
  {
    using Mantid::API::AnalysisDataService;
    
    UserInputValidator uiv;

    uiv.checkDataSelectorIsValid("Sample", uiForm().confit_dsSampleInput);
    uiv.checkDataSelectorIsValid("Resolution", uiForm().confit_dsResInput);

    auto range = std::make_pair(m_dblManager->value(m_properties["StartX"]), m_dblManager->value(m_properties["EndX"]));
    uiv.checkValidRange("Fitting Range", range);

    // Enforce the rule that at least one fit is needed; either a delta function, one or two lorentzian functions,
    // or both.  (The resolution function must be convolved with a model.)
    if ( uiForm().confit_cbFitType->currentIndex() == 0 && ! m_blnManager->value(m_properties["UseDeltaFunc"]) )
      uiv.addErrorMessage("No fit function has been selected.");

    QString error = uiv.generateErrorMessage();
    showMessageBox(error);

    return error.isEmpty();
  }

  void ConvFit::loadSettings(const QSettings & settings)
  {
    uiForm().confit_dsSampleInput->readSettings(settings.group());
    uiForm().confit_dsResInput->readSettings(settings.group());
  }

  namespace
  {
    ////////////////////////////
    // Anon Helper functions. //
    ////////////////////////////

    /**
     * Takes an index and a name, and constructs a single level parameter name
     * for use with function ties, etc.
     *
     * @param index :: the index of the function in the first level.
     * @param name  :: the name of the parameter inside the function.
     *
     * @returns the constructed function parameter name.
     */
    std::string createParName(size_t index, const std::string & name = "")
    {
      std::stringstream prefix;
      prefix << "f" << index << "." << name;
      return prefix.str();
    }

    /**
     * Takes an index, a sub index and a name, and constructs a double level 
     * (nested) parameter name for use with function ties, etc.
     *
     * @param index    :: the index of the function in the first level.
     * @param subIndex :: the index of the function in the second level.
     * @param name     :: the name of the parameter inside the function.
     *
     * @returns the constructed function parameter name.
     */
    std::string createParName(size_t index, size_t subIndex, const std::string & name = "")
    {
      std::stringstream prefix;
      prefix << "f" << index << ".f" << subIndex << "." << name;
      return prefix.str();
    }
  }

  /**
   * Creates a function to carry out the fitting in the "ConvFit" tab.  The function consists
   * of various sub functions, with the following structure:
   *
   * Composite
   *  |
   *  +-- LinearBackground
   *  +-- Convolution
   *      |
   *      +-- Resolution
   *      +-- Model (AT LEAST one delta function or one/two lorentzians.)
   *          |
   *          +-- DeltaFunction (yes/no)
	 *					+-- ProductFunction
	 *							|
   *							+-- Lorentzian 1 (yes/no)
	 *							+-- Temperature Correction (yes/no)
	 *					+-- ProductFunction
	 *							|
   *							+-- Lorentzian 2 (yes/no)
	 *							+-- Temperature Correction (yes/no)
   *
   * @param tieCentres :: whether to tie centres of the two lorentzians.
   *
   * @returns the composite fitting function.
   */
  Mantid::API::CompositeFunction_sptr ConvFit::createFunction(bool tieCentres)
  {
    auto conv = boost::dynamic_pointer_cast<Mantid::API::CompositeFunction>(Mantid::API::FunctionFactory::Instance().createFunction("Convolution"));
    Mantid::API::CompositeFunction_sptr comp( new Mantid::API::CompositeFunction );

    Mantid::API::IFunction_sptr func;
    size_t index = 0;

    // -------------------------------------
    // --- Composite / Linear Background ---
    // -------------------------------------
    func = Mantid::API::FunctionFactory::Instance().createFunction("LinearBackground");
    comp->addFunction(func); 

    const int bgType = uiForm().confit_cbBackground->currentIndex(); // 0 = Fixed Flat, 1 = Fit Flat, 2 = Fit all
  
    if ( bgType == 0 || ! m_properties["BGA0"]->subProperties().isEmpty() )
    {
      comp->tie("f0.A0", m_properties["BGA0"]->valueText().toStdString() );
    }
    else
    {
      func->setParameter("A0", m_properties["BGA0"]->valueText().toDouble());
    }

    if ( bgType != 2 )
    {
      comp->tie("f0.A1", "0.0");
    }
    else
    {
      if ( ! m_properties["BGA1"]->subProperties().isEmpty() )
      {
        comp->tie("f0.A1", m_properties["BGA1"]->valueText().toStdString() );
      }
      else { func->setParameter("A1", m_properties["BGA1"]->valueText().toDouble()); }
    }

    // --------------------------------------------
    // --- Composite / Convolution / Resolution ---
    // --------------------------------------------
    func = Mantid::API::FunctionFactory::Instance().createFunction("Resolution");
    conv->addFunction(func);
    
    //add resolution file
    if (uiForm().confit_dsResInput->isFileSelectorVisible())
    {    
      std::string resfilename = uiForm().confit_dsResInput->getFullFilePath().toStdString();
      Mantid::API::IFunction::Attribute attr(resfilename);
      func->setAttribute("FileName", attr);
    }
    else
    {
      std::string resWorkspace = uiForm().confit_dsResInput->getCurrentDataName().toStdString();
      Mantid::API::IFunction::Attribute attr(resWorkspace);
      func->setAttribute("Workspace", attr);
    }

    // --------------------------------------------------------
    // --- Composite / Convolution / Model / Delta Function ---
    // --------------------------------------------------------
    Mantid::API::CompositeFunction_sptr model( new Mantid::API::CompositeFunction );

    bool useDeltaFunc = m_blnManager->value(m_properties["UseDeltaFunc"]);

    size_t subIndex = 0;

    if ( useDeltaFunc )
    {
      func = Mantid::API::FunctionFactory::Instance().createFunction("DeltaFunction");
			index = model->addFunction(func);
			std::string parName = createParName(index);
			populateFunction(func, model, m_properties["DeltaFunction"], parName, false);
    }

    // ------------------------------------------------------------
    // --- Composite / Convolution / Model / Temperature Factor ---
    // ------------------------------------------------------------

    //create temperature correction function to multiply with the lorentzians
    Mantid::API::IFunction_sptr tempFunc;
    QString temperature = uiForm().confit_leTempCorrection->text();
    bool useTempCorrection = (!temperature.isEmpty() && uiForm().confit_ckTempCorrection->isChecked());

    // -----------------------------------------------------
    // --- Composite / Convolution / Model / Lorentzians ---
    // -----------------------------------------------------
    std::string prefix1;
    std::string prefix2;

    int fitTypeIndex = uiForm().confit_cbFitType->currentIndex();  

    // Add 1st Lorentzian
    if(fitTypeIndex > 0)
    {
      //if temperature not included then product is lorentzian * 1
      //create product function for temp * lorentzian
      auto product = boost::dynamic_pointer_cast<Mantid::API::CompositeFunction>(Mantid::API::FunctionFactory::Instance().createFunction("ProductFunction"));
      
      if(useTempCorrection)
      {
        createTemperatureCorrection(product);
      }

      func = Mantid::API::FunctionFactory::Instance().createFunction("Lorentzian");
      subIndex = product->addFunction(func);
      index = model->addFunction(product);
      prefix1 = createParName(index, subIndex);

      populateFunction(func, model, m_properties["Lorentzian1"], prefix1, false);
    }

    // Add 2nd Lorentzian
    if(fitTypeIndex == 2)
    {
      //if temperature not included then product is lorentzian * 1
      //create product function for temp * lorentzian
      auto product = boost::dynamic_pointer_cast<Mantid::API::CompositeFunction>(Mantid::API::FunctionFactory::Instance().createFunction("ProductFunction"));
    
      if(useTempCorrection)
      {
        createTemperatureCorrection(product);
      }

      func = Mantid::API::FunctionFactory::Instance().createFunction("Lorentzian");
      subIndex = product->addFunction(func);
      index = model->addFunction(product);
      prefix2 = createParName(index, subIndex);
      
      populateFunction(func, model, m_properties["Lorentzian2"], prefix2, false);
    }

    conv->addFunction(model);
    comp->addFunction(conv);

    // Tie PeakCentres together
    if ( tieCentres )
    {
      std::string tieL = prefix1 + "PeakCentre";
      std::string tieR = prefix2 + "PeakCentre";
      model->tie(tieL, tieR);
    }

    comp->applyTies();
    return comp;
  }

  void ConvFit::createTemperatureCorrection(Mantid::API::CompositeFunction_sptr product)
  {
    //create temperature correction function to multiply with the lorentzians
    Mantid::API::IFunction_sptr tempFunc;
    QString temperature = uiForm().confit_leTempCorrection->text();
    
    //create user function for the exponential correction
    // (x*temp) / 1-exp(-(x*temp))
    tempFunc = Mantid::API::FunctionFactory::Instance().createFunction("UserFunction");
    //11.606 is the conversion factor from meV to K
    std::string formula = "((x*11.606)/Temp) / (1 - exp(-((x*11.606)/Temp)))";
    Mantid::API::IFunction::Attribute att(formula);
    tempFunc->setAttribute("Formula", att);
    tempFunc->setParameter("Temp", temperature.toDouble());

    product->addFunction(tempFunc);
    product->tie("f0.Temp", temperature.toStdString());
    product->applyTies();
  }

  double ConvFit::getInstrumentResolution(std::string workspaceName)
  {
    using namespace Mantid::API;

    double resolution = 0.0;
    try
    {
      Mantid::Geometry::Instrument_const_sptr inst =
        AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(workspaceName)->getInstrument();
      std::string analyser = inst->getStringParameter("analyser")[0];

      // If the analyser component is not already in the data file the laod it from the parameter file
      if(inst->getComponentByName(analyser)->getNumberParameter("resolution").size() == 0)
      {
        std::string reflection = inst->getStringParameter("reflection")[0];

        IAlgorithm_sptr loadParamFile = AlgorithmManager::Instance().create("LoadParameterFile");
        loadParamFile->initialize();
        loadParamFile->setProperty("Workspace", workspaceName);
        loadParamFile->setProperty("Filename", inst->getName() + "_" + analyser + "_" + reflection + "_Parameters.xml");
        loadParamFile->execute();

        if(!loadParamFile->isExecuted())
        {
          g_log.error("Could not load parameter file, ensure instrument directory is in data search paths.");
          return 0.0;
        }

        inst = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(workspaceName)->getInstrument();
      }

      resolution = inst->getComponentByName(analyser)->getNumberParameter("resolution")[0];
    }
    catch(Mantid::Kernel::Exception::NotFoundError &e)
    {
      UNUSED_ARG(e);

      g_log.error("Could not load instrument resolution from parameter file");
      resolution = 0.0;
    }
      
    return resolution;
  }

  QtProperty* ConvFit::createLorentzian(const QString & name)
  {
    QtProperty* lorentzGroup = m_grpManager->addProperty(name);
    m_properties[name+".Amplitude"] = m_dblManager->addProperty("Amplitude");
    // m_dblManager->setRange(m_properties[name+".Amplitude"], 0.0, 1.0); // 0 < Amplitude < 1
    m_properties[name+".PeakCentre"] = m_dblManager->addProperty("PeakCentre");
    m_properties[name+".FWHM"] = m_dblManager->addProperty("FWHM");
    m_dblManager->setDecimals(m_properties[name+".Amplitude"], NUM_DECIMALS);
    m_dblManager->setDecimals(m_properties[name+".PeakCentre"], NUM_DECIMALS);
    m_dblManager->setDecimals(m_properties[name+".FWHM"], NUM_DECIMALS);
    m_dblManager->setValue(m_properties[name+".FWHM"], 0.02);
    lorentzGroup->addSubProperty(m_properties[name+".Amplitude"]);
    lorentzGroup->addSubProperty(m_properties[name+".PeakCentre"]);
    lorentzGroup->addSubProperty(m_properties[name+".FWHM"]);
    return lorentzGroup;
  }

  void ConvFit::populateFunction(Mantid::API::IFunction_sptr func, Mantid::API::IFunction_sptr comp, QtProperty* group, const std::string & pref, bool tie)
  {
    // Get subproperties of group and apply them as parameters on the function object
    QList<QtProperty*> props = group->subProperties();

    for ( int i = 0; i < props.size(); i++ )
    {
      if ( tie || ! props[i]->subProperties().isEmpty() )
      {
        std::string name = pref + props[i]->propertyName().toStdString();
        std::string value = props[i]->valueText().toStdString();
        comp->tie(name, value);
      }
      else
      {
        std::string propName = props[i]->propertyName().toStdString();
        double propValue = props[i]->valueText().toDouble();
				if ( propValue )
				{
					func->setParameter(propName, propValue);
				}
      }
    }
  }

  /**
   * Generate a string to describe the fit type selected by the user.
   * Used when naming the resultant workspaces.
   *
   * Assertions used to guard against any future changes that dont take
   * workspace naming into account.
   *
   * @returns the generated QString.
   */
  QString ConvFit::fitTypeString() const
  {
    QString fitType("");

    if( m_blnManager->value(m_properties["UseDeltaFunc"]) )
      fitType += "Delta";

    switch ( uiForm().confit_cbFitType->currentIndex() )
    {
    case 0:
      break;
    case 1:
      fitType += "1L"; break;
    case 2:
      fitType += "2L"; break;
    }

    return fitType;
  }
  
  /**
   * Generate a string to describe the background selected by the user.
   * Used when naming the resultant workspaces.
   *
   * Assertions used to guard against any future changes that dont take
   * workspace naming into account.
   *
   * @returns the generated QString.
   */
  QString ConvFit::backgroundString() const
  {
    switch ( uiForm().confit_cbBackground->currentIndex() )
    {
    case 0:
      return "FixF_s";
    case 1:
      return "FitF_s";
    case 2:
      return "FitL_s";
    default: 
      return "";
    }
  }

  void ConvFit::typeSelection(int index)
  {
    m_cfTree->removeProperty(m_properties["Lorentzian1"]);
    m_cfTree->removeProperty(m_properties["Lorentzian2"]);
  
    switch ( index )
    {
    case 0:
      m_rangeSelectors["ConvFitHWHM"]->setVisible(false);
      break;
    case 1:
      m_cfTree->addProperty(m_properties["Lorentzian1"]);
      m_rangeSelectors["ConvFitHWHM"]->setVisible(true);
      break;
    case 2:
      m_cfTree->addProperty(m_properties["Lorentzian1"]);
      m_cfTree->addProperty(m_properties["Lorentzian2"]);
      m_rangeSelectors["ConvFitHWHM"]->setVisible(true);
      break;
    }    
  }

  void ConvFit::bgTypeSelection(int index)
  {
    if ( index == 2 )
    {
      m_properties["LinearBackground"]->addSubProperty(m_properties["BGA1"]);
    }
    else
    {
      m_properties["LinearBackground"]->removeSubProperty(m_properties["BGA1"]);
    }
  }

  void ConvFit::plotInput()
  {
    using Mantid::API::MatrixWorkspace;
    using Mantid::API::AnalysisDataService;
    using Mantid::Kernel::Exception::NotFoundError;

    const bool plotGuess = uiForm().confit_ckPlotGuess->isChecked();
    uiForm().confit_ckPlotGuess->setChecked(false);

    if(uiForm().confit_dsSampleInput->getCurrentDataName() != m_cfInputWSName)
    {      
      m_cfInputWSName = uiForm().confit_dsSampleInput->getCurrentDataName();
      m_cfInputWS = AnalysisDataService::Instance().retrieveWS<const MatrixWorkspace>(m_cfInputWSName.toStdString());
      
      if(!m_cfInputWS)
      {
        showMessageBox("Could not find the workspace in ADS. See log for details.");
      }
    }

    int specNo = uiForm().confit_lePlotSpectrum->text().toInt();
    // Set spectra max value
    int specMin = 0;
    int specMax = static_cast<int>(m_cfInputWS->getNumberHistograms()) - 1;

    m_valInt->setRange(specMin, specMax);
    uiForm().confit_leSpectraMin->setText(QString::number(specMin));
    uiForm().confit_leSpectraMax->setText(QString::number(specMax));

    if ( specNo < 0 || specNo > specMax )
    {
      uiForm().confit_lePlotSpectrum->setText("0");
      specNo = 0;
    }

    int smCurrent = uiForm().confit_leSpectraMax->text().toInt();
    if ( smCurrent < 0 || smCurrent > specMax )
    {
      uiForm().confit_leSpectraMax->setText(QString::number(specMax));
    }

    plotMiniPlot(m_cfInputWS, specNo, "ConvFitPlot", "CFDataCurve");
    try
    {
      const std::pair<double, double> range = getCurveRange("CFDataCurve");
      m_rangeSelectors["ConvFitRange"]->setRange(range.first, range.second);
      uiForm().confit_ckPlotGuess->setChecked(plotGuess);
    }
    catch(std::invalid_argument & exc)
    {
      showMessageBox(exc.what());
    }

    // Default FWHM to resolution of instrument
    double resolution = getInstrumentResolution(m_cfInputWSName.toStdString());
    if(resolution > 0)
    {
      m_dblManager->setValue(m_properties["Lorentzian 1.FWHM"], resolution);
      m_dblManager->setValue(m_properties["Lorentzian 2.FWHM"], resolution);
    }
  }

  void ConvFit::plotGuess(QtProperty*)
  {
    if ( ! uiForm().confit_ckPlotGuess->isChecked() || m_curves["CFDataCurve"] == NULL )
    {
      return;
    }

    bool tieCentres = (uiForm().confit_cbFitType->currentIndex() > 1);
    Mantid::API::CompositeFunction_sptr function = createFunction(tieCentres);

    if ( m_cfInputWS == NULL )
    {
      plotInput();
    }

    const size_t binIndexLow = m_cfInputWS->binIndexOf(m_dblManager->value(m_properties["StartX"]));
    const size_t binIndexHigh = m_cfInputWS->binIndexOf(m_dblManager->value(m_properties["EndX"]));
    const size_t nData = binIndexHigh - binIndexLow;

    std::vector<double> inputXData(nData);
    const Mantid::MantidVec& XValues = m_cfInputWS->readX(0);
    const bool isHistogram = m_cfInputWS->isHistogramData();

    for ( size_t i = 0; i < nData; i++ )
    {
      if ( isHistogram )
      {
        inputXData[i] = 0.5 * ( XValues[binIndexLow+i] + XValues[binIndexLow+i+1] );
      }
      else
      {
        inputXData[i] = XValues[binIndexLow+i];
      }
    }

    Mantid::API::FunctionDomain1DVector domain(inputXData);
    Mantid::API::FunctionValues outputData(domain);
    function->function(domain, outputData);

    QVector<double> dataX, dataY;

    for ( size_t i = 0; i < nData; i++ )
    {
      dataX.append(inputXData[i]);
      dataY.append(outputData.getCalculated(i));
    }

    removeCurve("CFCalcCurve");
    m_curves["CFCalcCurve"] = new QwtPlotCurve();
    m_curves["CFCalcCurve"]->setData(dataX, dataY);
    QPen fitPen(Qt::red, Qt::SolidLine);
    m_curves["CFCalcCurve"]->setPen(fitPen);
    m_curves["CFCalcCurve"]->attach(m_plots["ConvFitPlot"]);
    m_plots["ConvFitPlot"]->replot();
  }

  void ConvFit::singleFit()
  {
    plotInput();

    if ( m_curves["CFDataCurve"] == NULL )
    {
      showMessageBox("There was an error reading the data file.");
      return;
    }

    uiForm().confit_ckPlotGuess->setChecked(false);

    Mantid::API::CompositeFunction_sptr function = createFunction(uiForm().confit_ckTieCentres->isChecked());

    // get output name
    QString fitType = fitTypeString();
    QString bgType = backgroundString();

    if(fitType == "")
    {
      g_log.error("No fit type defined!");
    }

    QString outputNm = runPythonCode(QString("from IndirectCommon import getWSprefix\nprint getWSprefix('") + m_cfInputWSName + QString("')\n")).trimmed();
    outputNm += QString("conv_") + fitType + bgType + uiForm().confit_lePlotSpectrum->text();  
    std::string output = outputNm.toStdString();

    Mantid::API::IAlgorithm_sptr alg = Mantid::API::AlgorithmManager::Instance().create("Fit");
    alg->initialize();
    alg->setPropertyValue("Function", function->asString());
    alg->setPropertyValue("InputWorkspace", m_cfInputWSName.toStdString());
    alg->setProperty<int>("WorkspaceIndex", uiForm().confit_lePlotSpectrum->text().toInt());
    alg->setProperty<double>("StartX", m_dblManager->value(m_properties["StartX"]));
    alg->setProperty<double>("EndX", m_dblManager->value(m_properties["EndX"]));
    alg->setProperty("Output", output);
    alg->setProperty("CreateOutput", true);
    alg->setProperty("OutputCompositeMembers", true);
    alg->setProperty("ConvolveMembers", true);
    alg->execute();
   
    if ( ! alg->isExecuted() )
    {
      showMessageBox("Fit algorithm failed.");
      return;
    }

    // Plot the line on the mini plot
    plotMiniPlot(outputNm+"_Workspace", 1, "ConvFitPlot", "CFCalcCurve");
    QPen fitPen(Qt::red, Qt::SolidLine);
    m_curves["CFCalcCurve"]->setPen(fitPen);
    replot("ConvFitPlot");

    Mantid::API::IFunction_sptr outputFunc = alg->getProperty("Function");

    // Get params.
    QMap<QString,double> parameters;
    std::vector<std::string> parNames = outputFunc->getParameterNames();
    std::vector<double> parVals;

    for( size_t i = 0; i < parNames.size(); ++i )
      parVals.push_back(outputFunc->getParameter(parNames[i]));

    for ( size_t i = 0; i < parNames.size(); ++i )
      parameters[QString(parNames[i].c_str())] = parVals[i];

    // Populate Tree widget with values
    // Background should always be f0
    m_dblManager->setValue(m_properties["BGA0"], parameters["f0.A0"]);
    m_dblManager->setValue(m_properties["BGA1"], parameters["f0.A1"]);

    int noLorentz = uiForm().confit_cbFitType->currentIndex();

    int funcIndex = 0;
		int subIndex = 0;

		//check if we're using a temperature correction
		if (uiForm().confit_ckTempCorrection->isChecked() && 
				!uiForm().confit_leTempCorrection->text().isEmpty())
		{
				subIndex++;
		}

		bool usingDeltaFunc = m_blnManager->value(m_properties["UseDeltaFunc"]);
		bool usingCompositeFunc = ((usingDeltaFunc && noLorentz > 0) || noLorentz > 1);
    QString prefBase = "f1.f1.";

		if ( usingDeltaFunc )
    {
      QString key = prefBase;
			if (usingCompositeFunc)
			{
				key += "f0.";
			}
			
			key += "Height";

      m_dblManager->setValue(m_properties["DeltaHeight"], parameters[key]);
      funcIndex++;
    }

    if ( noLorentz > 0 )
    {
      // One Lorentz
			QString pref = prefBase;

			if ( usingCompositeFunc )
			{
				pref += "f" + QString::number(funcIndex) + ".f" + QString::number(subIndex) + ".";
			}
			else
			{
				pref += "f" + QString::number(subIndex) + ".";
			}

      m_dblManager->setValue(m_properties["Lorentzian 1.Amplitude"], parameters[pref+"Amplitude"]);
      m_dblManager->setValue(m_properties["Lorentzian 1.PeakCentre"], parameters[pref+"PeakCentre"]);
      m_dblManager->setValue(m_properties["Lorentzian 1.FWHM"], parameters[pref+"FWHM"]);
      funcIndex++;
    }

    if ( noLorentz > 1 )
    {
      // Two Lorentz
			QString pref = prefBase;
	   	pref += "f" + QString::number(funcIndex) + ".f" + QString::number(subIndex) + ".";

      m_dblManager->setValue(m_properties["Lorentzian 2.Amplitude"], parameters[pref+"Amplitude"]);
      m_dblManager->setValue(m_properties["Lorentzian 2.PeakCentre"], parameters[pref+"PeakCentre"]);
      m_dblManager->setValue(m_properties["Lorentzian 2.FWHM"], parameters[pref+"FWHM"]);
    }
  }

  void ConvFit::minChanged(double val)
  {
    m_dblManager->setValue(m_properties["StartX"], val);
  }

  void ConvFit::maxChanged(double val)
  {
    m_dblManager->setValue(m_properties["EndX"], val);
  }

  void ConvFit::hwhmChanged(double val)
  {
    const double peakCentre = m_dblManager->value(m_properties["Lorentzian 1.PeakCentre"]);
    // Always want FWHM to display as positive.
    const double hwhm = std::fabs(val-peakCentre);
    // Update the property
    m_rangeSelectors["ConvFitHWHM"]->blockSignals(true);
    m_dblManager->setValue(m_properties["Lorentzian 1.FWHM"], hwhm*2);
    m_rangeSelectors["ConvFitHWHM"]->blockSignals(false);
  }

  void ConvFit::backgLevel(double val)
  {
    m_dblManager->setValue(m_properties["BGA0"], val);
  }

  void ConvFit::updateRS(QtProperty* prop, double val)
  {
    if ( prop == m_properties["StartX"] ) { m_rangeSelectors["ConvFitRange"]->setMinimum(val); }
    else if ( prop == m_properties["EndX"] ) { m_rangeSelectors["ConvFitRange"]->setMaximum(val); }
    else if ( prop == m_properties["BGA0"] ) { m_rangeSelectors["ConvFitBackRange"]->setMinimum(val); }
    else if ( prop == m_properties["Lorentzian 1.FWHM"] ) { hwhmUpdateRS(val); }
    else if ( prop == m_properties["Lorentzian 1.PeakCentre"] )
    {
        hwhmUpdateRS(m_dblManager->value(m_properties["Lorentzian 1.FWHM"]));
    }
  }

  void ConvFit::hwhmUpdateRS(double val)
  {
    const double peakCentre = m_dblManager->value(m_properties["Lorentzian 1.PeakCentre"]);
    m_rangeSelectors["ConvFitHWHM"]->setMinimum(peakCentre-val/2);
    m_rangeSelectors["ConvFitHWHM"]->setMaximum(peakCentre+val/2);
  }

  void ConvFit::checkBoxUpdate(QtProperty* prop, bool checked)
  {
    // Add/remove some properties to display only relevant options
    if ( prop == m_properties["UseDeltaFunc"] )
    {
      if ( checked ) 
      { 
        m_properties["DeltaFunction"]->addSubProperty(m_properties["DeltaHeight"]);
        uiForm().confit_cbPlotOutput->addItem("Height");
        uiForm().confit_cbPlotOutput->addItem("EISF");
      }
      else 
      { 
        m_properties["DeltaFunction"]->removeSubProperty(m_properties["DeltaHeight"]);
        uiForm().confit_cbPlotOutput->removeItem(uiForm().confit_cbPlotOutput->count()-1);
        uiForm().confit_cbPlotOutput->removeItem(uiForm().confit_cbPlotOutput->count()-1);
      }
    }
  }

  void ConvFit::fitContextMenu(const QPoint &)
  {
    QtBrowserItem* item(NULL);

    item = m_cfTree->currentItem();

    if ( ! item )
      return;

    // is it a fit property ?
    QtProperty* prop = item->property();

    if ( prop == m_properties["StartX"] || prop == m_properties["EndX"] )
      return;

    // is it already fixed?
    bool fixed = prop->propertyManager() != m_dblManager;

    if ( fixed && prop->propertyManager() != m_stringManager ) 
      return;

    // Create the menu
    QMenu* menu = new QMenu("ConvFit", m_cfTree);
    QAction* action;

    if ( ! fixed )
    {
      action = new QAction("Fix", m_parentWidget);
      connect(action, SIGNAL(triggered()), this, SLOT(fixItem()));
    }
    else
    {
      action = new QAction("Remove Fix", m_parentWidget);
      connect(action, SIGNAL(triggered()), this, SLOT(unFixItem()));
    }

    menu->addAction(action);

    // Show the menu
    menu->popup(QCursor::pos());
  }

  void ConvFit::fixItem()
  {
    QtBrowserItem* item = m_cfTree->currentItem();

    // Determine what the property is.
    QtProperty* prop = item->property();
		QtProperty* fixedProp = m_stringManager->addProperty( prop->propertyName() );
    QtProperty* fprlbl = m_stringManager->addProperty("Fixed");
    fixedProp->addSubProperty(fprlbl);
    m_stringManager->setValue(fixedProp, prop->valueText());

    item->parent()->property()->addSubProperty(fixedProp);

    m_fixedProps[fixedProp] = prop;

    item->parent()->property()->removeSubProperty(prop);
  }

  void ConvFit::unFixItem()
  {
    QtBrowserItem* item = m_cfTree->currentItem();

    QtProperty* prop = item->property();
    if ( prop->subProperties().empty() )
    { 
      item = item->parent();
      prop = item->property();
    }

    item->parent()->property()->addSubProperty(m_fixedProps[prop]);
    item->parent()->property()->removeSubProperty(prop);
    m_fixedProps.remove(prop);
    QtProperty* proplbl = prop->subProperties()[0];
    delete proplbl;
    delete prop;
  }

  void ConvFit::showTieCheckbox(QString fitType)
  {
      uiForm().confit_ckTieCentres->setVisible( fitType == "Two Lorentzians" );
  }

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
