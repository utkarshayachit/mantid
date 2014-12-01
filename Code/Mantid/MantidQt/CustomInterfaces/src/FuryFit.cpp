#include "MantidQtCustomInterfaces/FuryFit.h"

#include "MantidQtCustomInterfaces/UserInputValidator.h"
#include "MantidQtMantidWidgets/RangeSelector.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/FunctionDomain1D.h"

#include <math.h>
#include <QFileInfo>
#include <QMenu>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace MantidQt
{
namespace CustomInterfaces
{
namespace IDA
{
  FuryFit::FuryFit(QWidget * parent) : 
    IDATab(parent),
    m_stringManager(NULL), m_ffTree(NULL),
    m_ffRangeManager(NULL),
    m_fixedProps(),
    m_ffInputWS(), m_ffOutputWS(),
    m_ffInputWSName(),
    m_ties()
  {
  }
      
  void FuryFit::setup()
  {
    m_stringManager = new QtStringPropertyManager(m_parentWidget);

    m_ffTree = new QtTreePropertyBrowser(m_parentWidget);
    uiForm().furyfit_properties->addWidget(m_ffTree);
  
    // Setup FuryFit Plot Window
    m_plots["FuryFitPlot"] = new QwtPlot(m_parentWidget);
    m_plots["FuryFitPlot"]->setAxisFont(QwtPlot::xBottom, m_parentWidget->font());
    m_plots["FuryFitPlot"]->setAxisFont(QwtPlot::yLeft, m_parentWidget->font());
    uiForm().furyfit_vlPlot->addWidget(m_plots["FuryFitPlot"]);
    m_plots["FuryFitPlot"]->setCanvasBackground(QColor(255,255,255));
  
    m_rangeSelectors["FuryFitRange"] = new MantidQt::MantidWidgets::RangeSelector(m_plots["FuryFitPlot"]);
    connect(m_rangeSelectors["FuryFitRange"], SIGNAL(minValueChanged(double)), this, SLOT(xMinSelected(double)));
    connect(m_rangeSelectors["FuryFitRange"], SIGNAL(maxValueChanged(double)), this, SLOT(xMaxSelected(double)));

    m_rangeSelectors["FuryFitBackground"] = new MantidQt::MantidWidgets::RangeSelector(m_plots["FuryFitPlot"],
      MantidQt::MantidWidgets::RangeSelector::YSINGLE);
    m_rangeSelectors["FuryFitBackground"]->setRange(0.0,1.0);
    m_rangeSelectors["FuryFitBackground"]->setColour(Qt::darkGreen);
    connect(m_rangeSelectors["FuryFitBackground"], SIGNAL(minValueChanged(double)), this, SLOT(backgroundSelected(double)));

    // setupTreePropertyBrowser
    m_ffRangeManager = new QtDoublePropertyManager(m_parentWidget);
  
    m_ffTree->setFactoryForManager(m_dblManager, doubleEditorFactory());
    m_ffTree->setFactoryForManager(m_ffRangeManager, doubleEditorFactory());

    m_properties["StartX"] = m_ffRangeManager->addProperty("StartX");
    m_ffRangeManager->setDecimals(m_properties["StartX"], NUM_DECIMALS);
    m_properties["EndX"] = m_ffRangeManager->addProperty("EndX");
    m_ffRangeManager->setDecimals(m_properties["EndX"], NUM_DECIMALS);

    connect(m_ffRangeManager, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(propertyChanged(QtProperty*, double)));
    connect(m_dblManager, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(propertyChanged(QtProperty*, double)));

    m_properties["LinearBackground"] = m_grpManager->addProperty("LinearBackground");
    m_properties["BackgroundA0"] = m_ffRangeManager->addProperty("A0");
    m_ffRangeManager->setDecimals(m_properties["BackgroundA0"], NUM_DECIMALS);
    m_properties["LinearBackground"]->addSubProperty(m_properties["BackgroundA0"]);

    m_properties["Exponential1"] = createExponential("Exponential1");
    m_properties["Exponential2"] = createExponential("Exponential2");
  
    m_properties["StretchedExp"] = createStretchedExp("StretchedExp");

    m_ffRangeManager->setMinimum(m_properties["BackgroundA0"], 0);
    m_ffRangeManager->setMaximum(m_properties["BackgroundA0"], 1);

    m_dblManager->setMinimum(m_properties["Exponential1.Intensity"], 0);
    m_dblManager->setMaximum(m_properties["Exponential1.Intensity"], 1);

    m_dblManager->setMinimum(m_properties["Exponential2.Intensity"], 0);
    m_dblManager->setMaximum(m_properties["Exponential2.Intensity"], 1);

    m_dblManager->setMinimum(m_properties["StretchedExp.Intensity"], 0);
    m_dblManager->setMaximum(m_properties["StretchedExp.Intensity"], 1);

    typeSelection(uiForm().furyfit_cbFitType->currentIndex());

    // Connect to PlotGuess checkbox
    connect(m_dblManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(plotGuess(QtProperty*)));

    // Signal/slot ui connections
    connect(uiForm().furyfit_inputFile, SIGNAL(fileEditingFinished()), this, SLOT(plotInput()));
    connect(uiForm().furyfit_cbFitType, SIGNAL(currentIndexChanged(int)), this, SLOT(typeSelection(int)));
    connect(uiForm().furyfit_lePlotSpectrum, SIGNAL(editingFinished()), this, SLOT(plotInput()));
    connect(uiForm().furyfit_cbInputType, SIGNAL(currentIndexChanged(int)), uiForm().furyfit_swInput, SLOT(setCurrentIndex(int)));  
    connect(uiForm().furyfit_pbSingle, SIGNAL(clicked()), this, SLOT(singleFit()));

    //plot input connections
    connect(uiForm().furyfit_inputFile, SIGNAL(filesFound()), this, SLOT(plotInput()));
    connect(uiForm().furyfit_wsIqt, SIGNAL(currentIndexChanged(int)), this, SLOT(plotInput()));
    connect(uiForm().furyfit_pbPlotInput, SIGNAL(clicked()), this, SLOT(plotInput()));
    connect(uiForm().furyfit_cbInputType, SIGNAL(currentIndexChanged(int)), this, SLOT(plotInput()));

    // apply validators - furyfit
    uiForm().furyfit_lePlotSpectrum->setValidator(m_valInt);
    uiForm().furyfit_leSpectraMin->setValidator(m_valInt);
    uiForm().furyfit_leSpectraMax->setValidator(m_valInt);

    // Set a custom handler for the QTreePropertyBrowser's ContextMenu event
    m_ffTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ffTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(fitContextMenu(const QPoint &)));
  }

  void FuryFit::run()
  {
    if ( m_ffInputWS == NULL )
    {
      return;
    }

    const bool constrainBeta = uiForm().furyfit_ckConstrainBeta->isChecked();
    const bool constrainIntens = uiForm().furyfit_ckConstrainIntensities->isChecked();
    Mantid::API::CompositeFunction_sptr func = createFunction();
    func->tie("f0.A1", "0");
    
    if ( constrainIntens )
    {
      constrainIntensities(func);
    }
    
    func->applyTies();
    
    std::string function = std::string(func->asString());
    QString fitType = fitTypeString();
    QString specMin = uiForm().furyfit_leSpectraMin->text();
    QString specMax = uiForm().furyfit_leSpectraMax->text();

    QString pyInput = "from IndirectDataAnalysis import furyfitSeq, furyfitMult\n"
      "input = '" + m_ffInputWSName + "'\n"
      "func = r'" + QString::fromStdString(function) + "'\n"
      "ftype = '"   + fitTypeString() + "'\n"
      "startx = " + m_properties["StartX"]->valueText() + "\n"
      "endx = " + m_properties["EndX"]->valueText() + "\n"
      "plot = '" + uiForm().furyfit_cbPlotOutput->currentText() + "'\n"
      "spec_min = " + specMin + "\n"
      "spec_max = None\n";
    
    if(specMax != "")
        pyInput += "spec_max = " + specMax + "\n";

    if (constrainIntens) pyInput += "constrain_intens = True \n";
    else pyInput += "constrain_intens = False \n";

    if ( uiForm().furyfit_ckVerbose->isChecked() ) pyInput += "verbose = True\n";
    else pyInput += "verbose = False\n";

    if ( uiForm().furyfit_ckSaveSeq->isChecked() ) pyInput += "save = True\n";
    else pyInput += "save = False\n";

    if( !constrainBeta )
    {
      pyInput += "furyfitSeq(input, func, ftype, startx, endx, spec_min=spec_min, spec_max=spec_max, intensities_constrained=constrain_intens, Save=save, Plot=plot, Verbose=verbose)\n";
    }
    else
    {
      pyInput += "furyfitMult(input, func, ftype, startx, endx, spec_min=spec_min, spec_max=spec_max, intensities_constrained=constrain_intens, Save=save, Plot=plot, Verbose=verbose)\n";
    }
  
    QString pyOutput = runPythonCode(pyInput);

    // Set the result workspace for Python script export
    QString inputWsName = QString::fromStdString(m_ffInputWS->getName());
    QString resultWsName = inputWsName.left(inputWsName.lastIndexOf("_")) + "_fury_" + fitType + specMin + "_to_" + specMax + "_Workspaces";
    m_pythonExportWsName = resultWsName.toStdString();
  }

  bool FuryFit::validate()
  {
    using namespace Mantid::API;

    UserInputValidator uiv;

    switch( uiForm().furyfit_cbInputType->currentIndex() )
    {
    case 0:
      uiv.checkMWRunFilesIsValid("Input", uiForm().furyfit_inputFile); 

      //file should already be loaded by this point, but attempt to recover if not.
      if(!AnalysisDataService::Instance().doesExist(m_ffInputWSName.toStdString()))
      {
        //attempt to reload the nexus file.
        QString filename = uiForm().furyfit_inputFile->getFirstFilename();
        QFileInfo fi(filename);
        QString wsname = fi.baseName();

        loadFile(filename, wsname);
        m_ffInputWSName = wsname;
        m_ffInputWS = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(wsname.toStdString());
      }

      break;
    case 1:
      uiv.checkWorkspaceSelectorIsNotEmpty("Input", uiForm().furyfit_wsIqt); break;
    }

    auto range = std::make_pair(m_ffRangeManager->value(m_properties["StartX"]), m_ffRangeManager->value(m_properties["EndX"]));
    uiv.checkValidRange("Ranges", range);

    QString error = uiv.generateErrorMessage();
    showMessageBox(error);

    return error.isEmpty();
  }

  void FuryFit::loadSettings(const QSettings & settings)
  {
    uiForm().furyfit_inputFile->readSettings(settings.group());
  }

  Mantid::API::CompositeFunction_sptr FuryFit::createFunction(bool tie)
  {
    Mantid::API::CompositeFunction_sptr result( new Mantid::API::CompositeFunction );
    QString fname;
    const int fitType = uiForm().furyfit_cbFitType->currentIndex();

    Mantid::API::IFunction_sptr func = Mantid::API::FunctionFactory::Instance().createFunction("LinearBackground");
    func->setParameter("A0", m_ffRangeManager->value(m_properties["BackgroundA0"]));
    result->addFunction(func);
    result->tie("f0.A1", "0");
    if ( tie ) { result->tie("f0.A0", m_properties["BackgroundA0"]->valueText().toStdString()); }
  
    if ( fitType == 2 ) { fname = "StretchedExp"; }
    else { fname = "Exponential1"; }

    result->addFunction(createUserFunction(fname, tie));

    if ( fitType == 1 || fitType == 3 )
    {
      if ( fitType == 1 ) { fname = "Exponential2"; }
      else { fname = "StretchedExp"; }
      result->addFunction(createUserFunction(fname, tie));
    }

    // Return CompositeFunction object to caller.
    result->applyTies();
    return result;
  }

  Mantid::API::IFunction_sptr FuryFit::createUserFunction(const QString & name, bool tie)
  {
    Mantid::API::IFunction_sptr result = Mantid::API::FunctionFactory::Instance().createFunction("UserFunction");  
    std::string formula;

    if ( name.startsWith("Exp") ) { formula = "Intensity*exp(-(x/Tau))"; }
    else { formula = "Intensity*exp(-(x/Tau)^Beta)"; }

    Mantid::API::IFunction::Attribute att(formula);  
    result->setAttribute("Formula", att);

    QList<QtProperty*> props = m_properties[name]->subProperties();
    for ( int i = 0; i < props.size(); i++ )
    {
      std::string name = props[i]->propertyName().toStdString();
      result->setParameter(name, m_dblManager->value(props[i]));
      
      //add tie if parameter is fixed
      if ( tie || ! props[i]->subProperties().isEmpty() )
      {
        std::string value = props[i]->valueText().toStdString();
        result->tie(name, value);
      }
    }
    
    result->applyTies();
    return result;
  }

  QtProperty* FuryFit::createExponential(const QString & name)
  {
    QtProperty* expGroup = m_grpManager->addProperty(name);
    m_properties[name+".Intensity"] = m_dblManager->addProperty("Intensity");
    m_dblManager->setDecimals(m_properties[name+".Intensity"], NUM_DECIMALS);
    m_properties[name+".Tau"] = m_dblManager->addProperty("Tau");
    m_dblManager->setDecimals(m_properties[name+".Tau"], NUM_DECIMALS);
    expGroup->addSubProperty(m_properties[name+".Intensity"]);
    expGroup->addSubProperty(m_properties[name+".Tau"]);
    return expGroup;
  }

  QtProperty* FuryFit::createStretchedExp(const QString & name)
  {
    QtProperty* prop = m_grpManager->addProperty(name);
    m_properties[name+".Intensity"] = m_dblManager->addProperty("Intensity");
    m_properties[name+".Tau"] = m_dblManager->addProperty("Tau");
    m_properties[name+".Beta"] = m_dblManager->addProperty("Beta");
    m_dblManager->setRange(m_properties[name+".Beta"], 0, 1);
    m_dblManager->setDecimals(m_properties[name+".Intensity"], NUM_DECIMALS);
    m_dblManager->setDecimals(m_properties[name+".Tau"], NUM_DECIMALS);
    m_dblManager->setDecimals(m_properties[name+".Beta"], NUM_DECIMALS);
    prop->addSubProperty(m_properties[name+".Intensity"]);
    prop->addSubProperty(m_properties[name+".Tau"]);
    prop->addSubProperty(m_properties[name+".Beta"]);
    return prop;
  }

  QString FuryFit::fitTypeString() const
  {
    switch ( uiForm().furyfit_cbFitType->currentIndex() )
    {
    case 0:
      return "1E_s";
    case 1:
      return "2E_s";
    case 2:
      return "1S_s";
    case 3:
      return "1E1S_s";
    default:
      return "s";
    };
  }

  void FuryFit::typeSelection(int index)
  {
    m_ffTree->clear();

    m_ffTree->addProperty(m_properties["StartX"]);
    m_ffTree->addProperty(m_properties["EndX"]);
    m_ffTree->addProperty(m_properties["LinearBackground"]);
    
    //option should only be available with a single stretched exponential
    uiForm().furyfit_ckConstrainBeta->setEnabled((index == 2));
    if (!uiForm().furyfit_ckConstrainBeta->isEnabled())
    {
      uiForm().furyfit_ckConstrainBeta->setChecked(false);
    }

    switch ( index )
    {
    case 0:
      m_ffTree->addProperty(m_properties["Exponential1"]);

      //remove option to plot beta
      uiForm().furyfit_cbPlotOutput->removeItem(4);
      break;
    case 1:
      m_ffTree->addProperty(m_properties["Exponential1"]);
      m_ffTree->addProperty(m_properties["Exponential2"]);

      //remove option to plot beta
      uiForm().furyfit_cbPlotOutput->removeItem(4);
      break;
    case 2:
      m_ffTree->addProperty(m_properties["StretchedExp"]);

      //add option to plot beta
      if(uiForm().furyfit_cbPlotOutput->count() == 4)
      {
        uiForm().furyfit_cbPlotOutput->addItem("Beta");
      }
      
      break;
    case 3:
      m_ffTree->addProperty(m_properties["Exponential1"]);
      m_ffTree->addProperty(m_properties["StretchedExp"]);

      //add option to plot beta
      if(uiForm().furyfit_cbPlotOutput->count() == 4)
      {
        uiForm().furyfit_cbPlotOutput->addItem("Beta");
      }

      break;
    }

    plotGuess(NULL);
  }

  void FuryFit::plotInput()
  {
    using namespace Mantid::API;
    switch ( uiForm().furyfit_cbInputType->currentIndex() )
    {
    case 0: // "File"
      {
        if ( ! uiForm().furyfit_inputFile->isValid() )
        {
          return;
        }
        else
        {
          QFileInfo fi(uiForm().furyfit_inputFile->getFirstFilename());
          QString wsname = fi.baseName();
          if ( (m_ffInputWS == NULL) || ( wsname != m_ffInputWSName ) )
          {
            m_ffInputWSName = wsname;
            QString filename = uiForm().furyfit_inputFile->getFirstFilename();
            // get the output workspace
            loadFile(filename, m_ffInputWSName);
            m_ffInputWS = AnalysisDataService::Instance().retrieveWS<const MatrixWorkspace>(m_ffInputWSName.toStdString());
            if(!m_ffInputWS)
            {
              return;
            }
          }
        }
      }
      break;
    case 1: // Workspace
      {
        m_ffInputWSName = uiForm().furyfit_wsIqt->currentText();
        if(m_ffInputWSName.isEmpty())
        {
          return;
        }
        try
        {
          m_ffInputWS = AnalysisDataService::Instance().retrieveWS<const MatrixWorkspace>(m_ffInputWSName.toStdString());
        }
        catch ( Mantid::Kernel::Exception::NotFoundError & )
        {
          QString msg = "Workspace: '" + m_ffInputWSName + "' could not be "
            "found in the Analysis Data Service.";
          showMessageBox(msg);
          return;
        }
      }
      break;
    }

    int specNo = uiForm().furyfit_lePlotSpectrum->text().toInt();
    int nHist = static_cast<int>(m_ffInputWS->getNumberHistograms());
    int specMin = 0;
    int specMax = nHist - 1;

    m_valInt->setRange(specMin, specMax);
    uiForm().furyfit_leSpectraMin->setText(QString::number(specMin));
    uiForm().furyfit_leSpectraMax->setText(QString::number(specMax));

    if( specNo < 0 || specNo >= nHist )
    {
      if (specNo < 0)
      {
        specNo = 0;
      }
      else
      {
        specNo = nHist-1;
      }
      uiForm().furyfit_lePlotSpectrum->setText(QString::number(specNo));
    }

    plotMiniPlot(m_ffInputWS, specNo, "FuryFitPlot", "FF_DataCurve");
    try
    {
      const std::pair<double, double> range = getCurveRange("FF_DataCurve");
      m_rangeSelectors["FuryFitRange"]->setRange(range.first, range.second);
      m_ffRangeManager->setRange(m_properties["StartX"], range.first, range.second);
      m_ffRangeManager->setRange(m_properties["EndX"], range.first, range.second);
      
      setDefaultParameters("Exponential1");
      setDefaultParameters("Exponential2");
      setDefaultParameters("StretchedExp");

      m_plots["FuryFitPlot"]->setAxisScale(QwtPlot::xBottom, range.first, range.second);
      m_plots["FuryFitPlot"]->setAxisScale(QwtPlot::yLeft, 0.0, 1.0);
      replot("FuryFitPlot");
    }
    catch(std::invalid_argument & exc)
    {
      showMessageBox(exc.what());
    }
  }

  void FuryFit::setDefaultParameters(const QString& name)
  {
    double background = m_dblManager->value(m_properties["BackgroundA0"]);
    //intensity is always 1-background
    m_dblManager->setValue(m_properties[name+".Intensity"], 1.0-background);
    auto x = m_ffInputWS->readX(0);
    auto y = m_ffInputWS->readY(0);
    double tau = 0;

    if (x.size() > 4)
    {
      tau = -x[4] / log(y[4]);
    }

    m_dblManager->setValue(m_properties[name+".Tau"], tau);
    m_dblManager->setValue(m_properties[name+".Beta"], 1.0);
  }

  void FuryFit::xMinSelected(double val)
  {
    m_ffRangeManager->setValue(m_properties["StartX"], val);
  }

  void FuryFit::xMaxSelected(double val)
  {
    m_ffRangeManager->setValue(m_properties["EndX"], val);
  }

  void FuryFit::backgroundSelected(double val)
  {
    m_ffRangeManager->setValue(m_properties["BackgroundA0"], val);
    m_dblManager->setValue(m_properties["Exponential1.Intensity"], 1.0-val);
    m_dblManager->setValue(m_properties["Exponential2.Intensity"], 1.0-val);
    m_dblManager->setValue(m_properties["StretchedExp.Intensity"], 1.0-val);
  }

  void FuryFit::propertyChanged(QtProperty* prop, double val)
  {
    if ( prop == m_properties["StartX"] )
    {
      m_rangeSelectors["FuryFitRange"]->setMinimum(val);
    }
    else if ( prop == m_properties["EndX"] )
    {
      m_rangeSelectors["FuryFitRange"]->setMaximum(val);
    }
    else if ( prop == m_properties["BackgroundA0"])
    {
      m_rangeSelectors["FuryFitBackground"]->setMinimum(val);
      m_dblManager->setValue(m_properties["Exponential1.Intensity"], 1.0-val);
      m_dblManager->setValue(m_properties["Exponential2.Intensity"], 1.0-val);
      m_dblManager->setValue(m_properties["StretchedExp.Intensity"], 1.0-val);
    }
    else if( prop == m_properties["Exponential1.Intensity"] 
      || prop == m_properties["Exponential2.Intensity"] 
      || prop == m_properties["StretchedExp.Intensity"])
    {
      m_rangeSelectors["FuryFitBackground"]->setMinimum(1.0-val);
      m_dblManager->setValue(m_properties["Exponential1.Intensity"], val);
      m_dblManager->setValue(m_properties["Exponential2.Intensity"], val);
      m_dblManager->setValue(m_properties["StretchedExp.Intensity"], val);
    }
  }

  void FuryFit::constrainIntensities(Mantid::API::CompositeFunction_sptr func)
  {
    std::string paramName = "f1.Intensity";
    size_t index = func->parameterIndex(paramName);

    switch ( uiForm().furyfit_cbFitType->currentIndex() )
    {
    case 0: // 1 Exp
    case 2: // 1 Str
      if(!func->isFixed(index))
      {
        func->tie(paramName, "1-f0.A0");
      }
      else
      {
        std::string paramValue = boost::lexical_cast<std::string>(func->getParameter(paramName));
        func->tie(paramName, paramValue); 
        func->tie("f0.A0", "1-"+paramName);
      }
      break;
    case 1: // 2 Exp
    case 3: // 1 Exp & 1 Str
      if(!func->isFixed(index))
      {
        func->tie(paramName,"1-f2.Intensity-f0.A0");
      }
      else
      {
        std::string paramValue = boost::lexical_cast<std::string>(func->getParameter(paramName));
        func->tie(paramName,"1-f2.Intensity-f0.A0");
        func->tie(paramName, paramValue);
      }
      break;
    }
  }

  void FuryFit::singleFit()
  {
    // First create the function
    auto function = createFunction();

    const int fitType = uiForm().furyfit_cbFitType->currentIndex();
    if ( uiForm().furyfit_ckConstrainIntensities->isChecked() )
    {
      switch ( fitType )
      {
      case 0: // 1 Exp
      case 2: // 1 Str
        m_ties = "f1.Intensity = 1-f0.A0";
        break;
      case 1: // 2 Exp
      case 3: // 1 Exp & 1 Str
        m_ties = "f1.Intensity=1-f2.Intensity-f0.A0";
        break;
      default:
        break;
      }
    }
    QString ftype = fitTypeString();

    plotInput();
    if ( m_ffInputWS == NULL )
    {
      return;
    }

    QString pyInput = "from IndirectCommon import getWSprefix\nprint getWSprefix('%1')\n";
    pyInput = pyInput.arg(m_ffInputWSName);
    QString outputNm = runPythonCode(pyInput).trimmed();
    outputNm += QString("fury_") + ftype + uiForm().furyfit_lePlotSpectrum->text();
    std::string output = outputNm.toStdString();

    // Create the Fit Algorithm
    Mantid::API::IAlgorithm_sptr alg = Mantid::API::AlgorithmManager::Instance().create("Fit");
    alg->initialize();
    alg->setPropertyValue("Function", function->asString());
    alg->setPropertyValue("InputWorkspace", m_ffInputWSName.toStdString());
    alg->setProperty("WorkspaceIndex", uiForm().furyfit_lePlotSpectrum->text().toInt());
    alg->setProperty("StartX", m_ffRangeManager->value(m_properties["StartX"]));
    alg->setProperty("EndX", m_ffRangeManager->value(m_properties["EndX"]));
    alg->setProperty("Ties", m_ties.toStdString());
    alg->setPropertyValue("Output", output);
    alg->execute();

    if ( ! alg->isExecuted() )
    {
      QString msg = "There was an error executing the fitting algorithm. Please see the "
        "Results Log pane for more details.";
      showMessageBox(msg);
      return;
    }

    // Now show the fitted curve of the mini plot
    plotMiniPlot(outputNm+"_Workspace", 1, "FuryFitPlot", "FF_FitCurve");
    QPen fitPen(Qt::red, Qt::SolidLine);
    m_curves["FF_FitCurve"]->setPen(fitPen);
    replot("FuryFitPlot");

    Mantid::API::IFunction_sptr outputFunc = alg->getProperty("Function");

    // Get params.
    QMap<QString,double> parameters;
    std::vector<std::string> parNames = outputFunc->getParameterNames();
    std::vector<double> parVals;

    for( size_t i = 0; i < parNames.size(); ++i )
      parVals.push_back(outputFunc->getParameter(parNames[i]));

    for ( size_t i = 0; i < parNames.size(); ++i )
      parameters[QString(parNames[i].c_str())] = parVals[i];

    m_ffRangeManager->setValue(m_properties["BackgroundA0"], parameters["f0.A0"]);
  
    if ( fitType != 2 )
    {
      // Exp 1
      m_dblManager->setValue(m_properties["Exponential1.Intensity"], parameters["f1.Intensity"]);
      m_dblManager->setValue(m_properties["Exponential1.Tau"], parameters["f1.Tau"]);
    
      if ( fitType == 1 )
      {
        // Exp 2
        m_dblManager->setValue(m_properties["Exponential2.Intensity"], parameters["f2.Intensity"]);
        m_dblManager->setValue(m_properties["Exponential2.Tau"], parameters["f2.Tau"]);
      }
    }
  
    if ( fitType > 1 )
    {
      // Str
      QString fval;
      if ( fitType == 2 ) { fval = "f1."; }
      else { fval = "f2."; }
    
      m_dblManager->setValue(m_properties["StretchedExp.Intensity"], parameters[fval+"Intensity"]);
      m_dblManager->setValue(m_properties["StretchedExp.Tau"], parameters[fval+"Tau"]);
      m_dblManager->setValue(m_properties["StretchedExp.Beta"], parameters[fval+"Beta"]);
    }
  }

  void FuryFit::plotGuess(QtProperty*)
  {
    if ( m_curves["FF_DataCurve"] == NULL )
    {
      return;
    }

    Mantid::API::CompositeFunction_sptr function = createFunction(true);

    // Create the double* array from the input workspace
    const size_t binIndxLow = m_ffInputWS->binIndexOf(m_ffRangeManager->value(m_properties["StartX"]));
    const size_t binIndxHigh = m_ffInputWS->binIndexOf(m_ffRangeManager->value(m_properties["EndX"]));
    const size_t nData = binIndxHigh - binIndxLow;

    std::vector<double> inputXData(nData);

    const Mantid::MantidVec& XValues = m_ffInputWS->readX(0);

    const bool isHistogram = m_ffInputWS->isHistogramData();

    for ( size_t i = 0; i < nData ; i++ )
    {
      if ( isHistogram )
        inputXData[i] = 0.5*(XValues[binIndxLow+i]+XValues[binIndxLow+i+1]);
      else
        inputXData[i] = XValues[binIndxLow+i];
    }

    Mantid::API::FunctionDomain1DVector domain(inputXData);
    Mantid::API::FunctionValues outputData(domain);
    function->function(domain, outputData);

    QVector<double> dataX;
    QVector<double> dataY;

    for ( size_t i = 0; i < nData; i++ )
    {
      dataX.append(inputXData[i]);
      dataY.append(outputData.getCalculated(i));
    }

    // Create the curve
    removeCurve("FF_FitCurve");
    m_curves["FF_FitCurve"] = new QwtPlotCurve();
    m_curves["FF_FitCurve"]->setData(dataX, dataY);
    m_curves["FF_FitCurve"]->attach(m_plots["FuryFitPlot"]);
    QPen fitPen(Qt::red, Qt::SolidLine);
    m_curves["FF_FitCurve"]->setPen(fitPen);
    replot("FuryFitPlot");
  }

  void FuryFit::fitContextMenu(const QPoint &)
  {
    QtBrowserItem* item(NULL);

    item = m_ffTree->currentItem();

    if ( ! item )
      return;

    // is it a fit property ?
    QtProperty* prop = item->property();

    // is it already fixed?
    bool fixed = prop->propertyManager() != m_dblManager;

    if ( fixed && prop->propertyManager() != m_stringManager ) 
      return;

    // Create the menu
    QMenu* menu = new QMenu("FuryFit", m_ffTree);
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

  void FuryFit::fixItem()
  {
    QtBrowserItem* item = m_ffTree->currentItem();

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

  void FuryFit::unFixItem()
  {
    QtBrowserItem* item = m_ffTree->currentItem();

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
} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
