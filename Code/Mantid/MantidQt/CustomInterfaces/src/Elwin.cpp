#include "MantidQtCustomInterfaces/Elwin.h"
#include "MantidQtCustomInterfaces/UserInputValidator.h"

#include "MantidQtMantidWidgets/RangeSelector.h"

#include <QFileInfo>

#include <qwt_plot.h>

namespace MantidQt
{
namespace CustomInterfaces
{
namespace IDA
{
  Elwin::Elwin(QWidget * parent) : IDATab(parent),
    m_elwTree(NULL)
  {
  }

  void Elwin::setup()
  {
    // Create QtTreePropertyBrowser object
    m_elwTree = new QtTreePropertyBrowser();
    uiForm().elwin_properties->addWidget(m_elwTree);

    // Editor Factories
    m_elwTree->setFactoryForManager(m_dblManager, doubleEditorFactory());
    m_elwTree->setFactoryForManager(m_blnManager, qtCheckBoxFactory());

    // Create Properties
    m_properties["R1S"] = m_dblManager->addProperty("Start");
    m_dblManager->setDecimals(m_properties["R1S"], NUM_DECIMALS);
    m_properties["R1E"] = m_dblManager->addProperty("End");
    m_dblManager->setDecimals(m_properties["R1E"], NUM_DECIMALS);  
    m_properties["R2S"] = m_dblManager->addProperty("Start");
    m_dblManager->setDecimals(m_properties["R2S"], NUM_DECIMALS);
    m_properties["R2E"] = m_dblManager->addProperty("End");
    m_dblManager->setDecimals(m_properties["R2E"], NUM_DECIMALS);

    m_properties["UseTwoRanges"] = m_blnManager->addProperty("Use Two Ranges");

    m_properties["Range1"] = m_grpManager->addProperty("Range One");
    m_properties["Range1"]->addSubProperty(m_properties["R1S"]);
    m_properties["Range1"]->addSubProperty(m_properties["R1E"]);
    m_properties["Range2"] = m_grpManager->addProperty("Range Two");
    m_properties["Range2"]->addSubProperty(m_properties["R2S"]);
    m_properties["Range2"]->addSubProperty(m_properties["R2E"]);

    m_elwTree->addProperty(m_properties["Range1"]);
    m_elwTree->addProperty(m_properties["UseTwoRanges"]);
    m_elwTree->addProperty(m_properties["Range2"]);

    // Create Slice Plot Widget for Range Selection
    m_plots["ElwinPlot"] = new QwtPlot(m_parentWidget);
    m_plots["ElwinPlot"]->setAxisFont(QwtPlot::xBottom, m_parentWidget->font());
    m_plots["ElwinPlot"]->setAxisFont(QwtPlot::yLeft, m_parentWidget->font());
    uiForm().elwin_plot->addWidget(m_plots["ElwinPlot"]);
    m_plots["ElwinPlot"]->setCanvasBackground(Qt::white);
    // We always want one range selector... the second one can be controlled from
    // within the elwinTwoRanges(bool state) function
    m_rangeSelectors["ElwinRange1"] = new MantidWidgets::RangeSelector(m_plots["ElwinPlot"]);
    connect(m_rangeSelectors["ElwinRange1"], SIGNAL(minValueChanged(double)), this, SLOT(minChanged(double)));
    connect(m_rangeSelectors["ElwinRange1"], SIGNAL(maxValueChanged(double)), this, SLOT(maxChanged(double)));
    // create the second range
    m_rangeSelectors["ElwinRange2"] = new MantidWidgets::RangeSelector(m_plots["ElwinPlot"]);
    m_rangeSelectors["ElwinRange2"]->setColour(Qt::darkGreen); // dark green for background
    connect(m_rangeSelectors["ElwinRange1"], SIGNAL(rangeChanged(double, double)), m_rangeSelectors["ElwinRange2"], SLOT(setRange(double, double)));
    connect(m_rangeSelectors["ElwinRange2"], SIGNAL(minValueChanged(double)), this, SLOT(minChanged(double)));
    connect(m_rangeSelectors["ElwinRange2"], SIGNAL(maxValueChanged(double)), this, SLOT(maxChanged(double)));
    m_rangeSelectors["ElwinRange2"]->setRange(m_rangeSelectors["ElwinRange1"]->getRange());
    // Refresh the plot window
    replot("ElwinPlot");
  
    connect(m_dblManager, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(updateRS(QtProperty*, double)));
    connect(m_blnManager, SIGNAL(valueChanged(QtProperty*, bool)), this, SLOT(twoRanges(QtProperty*, bool)));
    twoRanges(0, false);

    connect(uiForm().elwin_pbPlotInput, SIGNAL(clicked()), this, SLOT(plotInput()));
    connect(uiForm().elwin_inputFile, SIGNAL(filesFound()), this, SLOT(plotInput()));

    // Set any default values
    m_dblManager->setValue(m_properties["R1S"], -0.02);
    m_dblManager->setValue(m_properties["R1E"], 0.02);

    m_dblManager->setValue(m_properties["R2S"], -0.24);
    m_dblManager->setValue(m_properties["R2E"], -0.22);
  }

  void Elwin::run()
  {
    QString pyInput =
      "from IndirectDataAnalysis import elwin\n"
      "input = [r'" + uiForm().elwin_inputFile->getFilenames().join("', r'") + "']\n"
      "eRange = [ " + QString::number(m_dblManager->value(m_properties["R1S"])) +","+ QString::number(m_dblManager->value(m_properties["R1E"]));

    if ( m_blnManager->value(m_properties["UseTwoRanges"]) )
    {
      pyInput += ", " + QString::number(m_dblManager->value(m_properties["R2S"])) + ", " + QString::number(m_dblManager->value(m_properties["R2E"]));
    }

    pyInput+= "]\n";

    pyInput+= "logType = '"+ uiForm().leLogName->text() +"'\n";
    
    if ( uiForm().elwin_ckNormalise->isChecked() ) pyInput += "normalise = True\n";
    else pyInput += "normalise = False\n";

    if ( uiForm().elwin_ckSave->isChecked() ) pyInput += "save = True\n";
    else pyInput += "save = False\n";

    if ( uiForm().elwin_ckVerbose->isChecked() ) pyInput += "verbose = True\n";
    else pyInput += "verbose = False\n";

    if ( uiForm().elwin_ckPlot->isChecked() ) pyInput += "plot = True\n";
    else pyInput += "plot = False\n";


    pyInput +=
      "eq1_ws, eq2_ws = elwin(input, eRange, log_type=logType, Normalise=normalise, Save=save, Verbose=verbose, Plot=plot)\n";

    QString pyOutput = runPythonCode(pyInput);
    UNUSED_ARG(pyOutput);
  }

  bool Elwin::validate()
  {
    UserInputValidator uiv;
    
    uiv.checkMWRunFilesIsValid("Input", uiForm().elwin_inputFile);

    auto rangeOne = std::make_pair(m_dblManager->value(m_properties["R1S"]), m_dblManager->value(m_properties["R1E"]));
    uiv.checkValidRange("Range One", rangeOne);

    bool useTwoRanges = m_blnManager->value(m_properties["UseTwoRanges"]);
    if( useTwoRanges )
    {
      auto rangeTwo = std::make_pair(m_dblManager->value(m_properties["R2S"]), m_dblManager->value(m_properties["R2E"]));
      uiv.checkValidRange("Range Two", rangeTwo);
      uiv.checkRangesDontOverlap(rangeOne, rangeTwo);
    }

    QString error = uiv.generateErrorMessage();
    showMessageBox(error);

    return error.isEmpty();
  }

  void Elwin::loadSettings(const QSettings & settings)
  {
    uiForm().elwin_inputFile->readSettings(settings.group());
  }

  void Elwin::setDefaultResolution(Mantid::API::MatrixWorkspace_const_sptr ws)
  {
    auto inst = ws->getInstrument();
    auto analyser = inst->getStringParameter("analyser");

    if(analyser.size() > 0)
    {
      auto comp = inst->getComponentByName(analyser[0]);
      auto params = comp->getNumberParameter("resolution", true);

      //set the default instrument resolution
      if(params.size() > 0)
      {
        double res = params[0];
        m_dblManager->setValue(m_properties["R1S"], -res);
        m_dblManager->setValue(m_properties["R1E"], res);

        m_dblManager->setValue(m_properties["R2S"], -10*res);
        m_dblManager->setValue(m_properties["R2E"], -9*res);
      }

    }
  }

  void Elwin::setDefaultSampleLog(Mantid::API::MatrixWorkspace_const_sptr ws)
  {
    auto inst = ws->getInstrument();
    auto log = inst->getStringParameter("Workflow.SE-log");
    QString logName("sample");

    if(log.size() > 0)
    {
      logName = QString::fromStdString(log[0]);
    }
    
    uiForm().leLogName->setText(logName);
  }


  void Elwin::plotInput()
  {
    using namespace Mantid::API;

    if ( uiForm().elwin_inputFile->isValid() )
    {
      QString filename = uiForm().elwin_inputFile->getFirstFilename();
      QFileInfo fi(filename);
      QString wsname = fi.baseName();

      loadFile(filename, wsname);
      auto ws = AnalysisDataService::Instance().retrieveWS<MatrixWorkspace>(wsname.toStdString());

      if(!ws)
      {
        return;
      }

      setDefaultResolution(ws);
      setDefaultSampleLog(ws);

      plotMiniPlot(ws, 0, "ElwinPlot", "ElwinDataCurve");
      try
      {
        const std::pair<double, double> range = getCurveRange("ElwinDataCurve");
        m_rangeSelectors["ElwinRange1"]->setRange(range.first, range.second);
        // Replot
        replot("ElwinPlot");
      }
      catch(std::invalid_argument & exc)
      {
        showMessageBox(exc.what());
      }

    }
    else
    {
      showMessageBox("Selected input files are invalid.");
    }
  }

  void Elwin::twoRanges(QtProperty*, bool val)
  {
    m_rangeSelectors["ElwinRange2"]->setVisible(val);
  }

  void Elwin::minChanged(double val)
  {
    MantidWidgets::RangeSelector* from = qobject_cast<MantidWidgets::RangeSelector*>(sender());
    if ( from == m_rangeSelectors["ElwinRange1"] )
    {
      m_dblManager->setValue(m_properties["R1S"], val);
    }
    else if ( from == m_rangeSelectors["ElwinRange2"] )
    {
      m_dblManager->setValue(m_properties["R2S"], val);
    }
  }

  void Elwin::maxChanged(double val)
  {
    MantidWidgets::RangeSelector* from = qobject_cast<MantidWidgets::RangeSelector*>(sender());
    if ( from == m_rangeSelectors["ElwinRange1"] )
    {
      m_dblManager->setValue(m_properties["R1E"], val);
    }
    else if ( from == m_rangeSelectors["ElwinRange2"] )
    {
      m_dblManager->setValue(m_properties["R2E"], val);
    }
  }

  void Elwin::updateRS(QtProperty* prop, double val)
  {
    if ( prop == m_properties["R1S"] ) m_rangeSelectors["ElwinRange1"]->setMinimum(val);
    else if ( prop == m_properties["R1E"] ) m_rangeSelectors["ElwinRange1"]->setMaximum(val);
    else if ( prop == m_properties["R2S"] ) m_rangeSelectors["ElwinRange2"]->setMinimum(val);
    else if ( prop == m_properties["R2E"] ) m_rangeSelectors["ElwinRange2"]->setMaximum(val);
  }
} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt
