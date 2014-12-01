#include "MantidQtCustomInterfaces/IndirectMoments.h"

#include "MantidQtCustomInterfaces/UserInputValidator.h"

#include <QDoubleValidator>
#include <QFileInfo>

using namespace Mantid::API;

namespace MantidQt
{
namespace CustomInterfaces
{

  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  IndirectMoments::IndirectMoments(Ui::IndirectDataReduction& uiForm, QWidget * parent) : IndirectDataReductionTab(uiForm, parent)
  {
    const unsigned int NUM_DECIMALS = 6;

    // RAW PLOT
    m_plots["MomentsPlot"] = new QwtPlot(m_parentWidget);
    /* m_curves["MomentsPlotCurve"] = new QwtPlotCurve(); */
    m_rangeSelectors["MomentsRangeSelector"] = new MantidWidgets::RangeSelector(m_plots["MomentsPlot"]);
    m_rangeSelectors["MomentsRangeSelector"]->setInfoOnly(false);

    // Initilise plot
    m_plots["MomentsPlot"]->setCanvasBackground(Qt::white);
    m_plots["MomentsPlot"]->setAxisFont(QwtPlot::xBottom, parent->font());
    m_plots["MomentsPlot"]->setAxisFont(QwtPlot::yLeft, parent->font());

    // Add plot to UI
    m_uiForm.moment_plotSpace->addWidget(m_plots["MomentsPlot"]);

    // PREVIEW PLOT
    m_plots["MomentsPreviewPlot"] = new QwtPlot(m_parentWidget);

    // Initilise plot
    m_plots["MomentsPreviewPlot"]->setCanvasBackground(Qt::white);
    m_plots["MomentsPreviewPlot"]->setAxisFont(QwtPlot::xBottom, parent->font());
    m_plots["MomentsPreviewPlot"]->setAxisFont(QwtPlot::yLeft, parent->font());

    // Add plot to UI
    m_uiForm.moment_plotPreview->addWidget(m_plots["MomentsPreviewPlot"]);

    // PROPERTY TREE
    m_propTrees["MomentsPropTree"] = new QtTreePropertyBrowser();
    m_propTrees["MomentsPropTree"]->setFactoryForManager(m_dblManager, m_dblEdFac);
    m_uiForm.moment_treeSpace->addWidget(m_propTrees["MomentsPropTree"]);
    m_properties["EMin"] = m_dblManager->addProperty("EMin");
    m_properties["EMax"] = m_dblManager->addProperty("EMax");

    m_propTrees["MomentsPropTree"]->addProperty(m_properties["EMin"]);
    m_propTrees["MomentsPropTree"]->addProperty(m_properties["EMax"]);

    m_dblManager->setDecimals(m_properties["EMin"], NUM_DECIMALS);
    m_dblManager->setDecimals(m_properties["EMax"], NUM_DECIMALS);

    m_uiForm.moment_leScale->setValidator(new QDoubleValidator());

    connect(m_uiForm.moment_dsInput, SIGNAL(dataReady(const QString&)), this, SLOT(handleSampleInputReady(const QString&)));
    connect(m_uiForm.moment_ckScale, SIGNAL(toggled(bool)), m_uiForm.moment_leScale, SLOT(setEnabled(bool)));
    connect(m_uiForm.moment_ckScale, SIGNAL(toggled(bool)), m_uiForm.moment_validScale, SLOT(setVisible(bool)));

    connect(m_rangeSelectors["MomentsRangeSelector"], SIGNAL(selectionChangedLazy(double, double)), this, SLOT(rangeChanged(double, double)));
    connect(m_dblManager, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(updateProperties(QtProperty*, double)));

    // Update the preview plot when the algorithm completes
    connect(m_batchAlgoRunner, SIGNAL(batchComplete(bool)), this, SLOT(momentsAlgComplete(bool)));

    m_uiForm.moment_validScale->setStyleSheet("QLabel { color : #aa0000; }");
  }

  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  IndirectMoments::~IndirectMoments()
  {
  }

  void IndirectMoments::setup()
  {
  }

  void IndirectMoments::run()
  {
    QString workspaceName = m_uiForm.moment_dsInput->getCurrentDataName();
    QString outputName = workspaceName.left(workspaceName.length() - 4);
    QString scaleString = m_uiForm.moment_leScale->text();
    double scale = 1.0;
    double eMin = m_dblManager->value(m_properties["EMin"]);
    double eMax = m_dblManager->value(m_properties["EMax"]);

    bool plot = m_uiForm.moment_ckPlot->isChecked();
    bool verbose = m_uiForm.moment_ckVerbose->isChecked();
    bool save = m_uiForm.moment_ckSave->isChecked();

    if(!scaleString.isEmpty())
      scale = scaleString.toDouble();

    std::string outputWorkspaceName = outputName.toStdString() + "_Moments";

    IAlgorithm_sptr momentsAlg = AlgorithmManager::Instance().create("SofQWMoments", -1);
    momentsAlg->initialize();
    momentsAlg->setProperty("Sample", workspaceName.toStdString());
    momentsAlg->setProperty("Scale", scale);
    momentsAlg->setProperty("EnergyMin", eMin);
    momentsAlg->setProperty("EnergyMax", eMax);
    momentsAlg->setProperty("Plot", plot);
    momentsAlg->setProperty("Verbose", verbose);
    momentsAlg->setProperty("Save", save);
    momentsAlg->setProperty("OutputWorkspace", outputWorkspaceName);

    // Set the workspace name for Python script export
    m_pythonExportWsName = outputWorkspaceName + "_M0";

    // Execute algorithm on seperate thread
    runAlgorithm(momentsAlg);
  }

  bool IndirectMoments::validate()
  {
    UserInputValidator uiv;

    uiv.checkDataSelectorIsValid("Sample input", m_uiForm.moment_dsInput);

    if (m_uiForm.moment_ckScale->isChecked())
      uiv.checkFieldIsValid("A valid scale must be supplied.\n", m_uiForm.moment_leScale, m_uiForm.moment_validScale);

    QString msg = uiv.generateErrorMessage();
    if (!msg.isEmpty())
    {
      emit showMessageBox(msg);
      return false;
    }

    return true;
  }

  void IndirectMoments::handleSampleInputReady(const QString& filename)
  {
    disconnect(m_dblManager, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(updateProperties(QtProperty*, double)));

    plotMiniPlot(filename, 0, "MomentsPlot", "MomentsPlotCurve");
    std::pair<double,double> range = getCurveRange("MomentsPlotCurve");
    setMiniPlotGuides("MomentsRangeSelector", m_properties["EMin"], m_properties["EMax"], range);
    setPlotRange("MomentsRangeSelector", m_properties["EMin"], m_properties["EMax"], range);

    connect(m_dblManager, SIGNAL(valueChanged(QtProperty*, double)), this, SLOT(updateProperties(QtProperty*, double)));

    // Update the results preview plot
    updatePreviewPlot();
  }

  /**
   * Updates the property manager when the range selector is moved.
   *
   * @param min :: The new value of the lower guide
   * @param max :: The new value of the upper guide
   */
  void IndirectMoments::rangeChanged(double min, double max)
  {
    m_dblManager->setValue(m_properties["EMin"], min);
    m_dblManager->setValue(m_properties["EMax"], max);
  }

  /**
   * Handles when properties in the property manager are updated.
   *
   * Performs validation and uodated preview plot.
   *
   * @param prop :: The property being updated
   * @param val :: The new value for the property
   */
  void IndirectMoments::updateProperties(QtProperty* prop, double val)
  {
    if(prop == m_properties["EMin"])
    {
      double emax = m_dblManager->value(m_properties["EMax"]);
      if(val >  emax)
      {
        m_dblManager->setValue(prop, emax);
      }
      else
      {
        m_rangeSelectors["MomentsRangeSelector"]->setMinimum(val);
      }
    }
    else if (prop == m_properties["EMax"])
    {
      double emin = m_dblManager->value(m_properties["EMin"]);
      if(emin > val)
      {
        m_dblManager->setValue(prop, emin);
      }
      else
      {
        m_rangeSelectors["MomentsRangeSelector"]->setMaximum(val);
      }
    }

    updatePreviewPlot();
  }

  /**
   * Runs the moments algorithm with preview properties.
   */
  void IndirectMoments::updatePreviewPlot(QString workspaceName)
  {
    if(workspaceName.isEmpty())
      workspaceName = m_uiForm.moment_dsInput->getCurrentDataName();

    QString outputName = workspaceName.left(workspaceName.length() - 4);
    QString scaleString = m_uiForm.moment_leScale->text();
    double scale = 1.0;
    double eMin = m_dblManager->value(m_properties["EMin"]);
    double eMax = m_dblManager->value(m_properties["EMax"]);

    bool verbose = m_uiForm.moment_ckVerbose->isChecked();

    if(!scaleString.isEmpty())
      scale = scaleString.toDouble();

    std::string outputWorkspaceName = outputName.toStdString() + "_Moments";

    IAlgorithm_sptr momentsAlg = AlgorithmManager::Instance().create("SofQWMoments");
    momentsAlg->initialize();
    momentsAlg->setProperty("Sample", workspaceName.toStdString());
    momentsAlg->setProperty("Scale", scale);
    momentsAlg->setProperty("EnergyMin", eMin);
    momentsAlg->setProperty("EnergyMax", eMax);
    momentsAlg->setProperty("Plot", false);
    momentsAlg->setProperty("Verbose", verbose);
    momentsAlg->setProperty("Save", false);
    momentsAlg->setProperty("OutputWorkspace", outputWorkspaceName);

    // Make sure there are no other algorithms in the queue.
    // It seems to be possible to have the selctionChangedLazy signal fire multiple times
    // if the renage selector is moved in a certain way.
    if(m_batchAlgoRunner->queueLength() == 0)
      runAlgorithm(momentsAlg);
  }

  /**
   * Handles plotting the preview plot when the algorithm finishes.
   *
   * @param error True if the algorithm exited due to error, false otherwise
   */
  void IndirectMoments::momentsAlgComplete(bool error)
  {
    if(error)
      return;

    QString workspaceName = m_uiForm.moment_dsInput->getCurrentDataName();
    QString outputName = workspaceName.left(workspaceName.length() - 4);
    std::string outputWorkspaceName = outputName.toStdString() + "_Moments";

    WorkspaceGroup_sptr resultWsGroup = AnalysisDataService::Instance().retrieveWS<WorkspaceGroup>(outputWorkspaceName);
    std::vector<std::string> resultWsNames = resultWsGroup->getNames();

    if(resultWsNames.size() < 4)
      return;

    // Plot each spectrum
    plotMiniPlot(QString::fromStdString(resultWsNames[0]), 0, "MomentsPreviewPlot", "Moments_M0");
    plotMiniPlot(QString::fromStdString(resultWsNames[2]), 0, "MomentsPreviewPlot", "Moments_M2");
    plotMiniPlot(QString::fromStdString(resultWsNames[3]), 0, "MomentsPreviewPlot", "Moments_M4");

    // Colour plots as close to plot output as possible
    m_curves["Moments_M0"]->setPen(QColor(Qt::green));
    m_curves["Moments_M2"]->setPen(QColor(Qt::black));
    m_curves["Moments_M4"]->setPen(QColor(Qt::red));

    m_plots["MomentsPreviewPlot"]->replot();
  }

} // namespace CustomInterfaces
} // namespace Mantid
