#ifndef MANTIDQTCUSTOMINTERFACES_ROCKINGCURVE_H_
#define MANTIDQTCUSTOMINTERFACES_ROCKINGCURVE_H_

//----------------------
// Includes
//----------------------
#include "ui_StepScan.h"
#include "MantidQtAPI/UserSubWindow.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/IAlgorithm.h"

namespace MantidQt
{
namespace CustomInterfaces
{

class StepScan : public API::UserSubWindow
{
  Q_OBJECT

public:
  /// The name of the interface as registered into the factory
  static std::string name() { return "Step Scan Analysis"; }

  StepScan(QWidget *parent = 0);
  ~StepScan();

signals:
  void logsAvailable( const Mantid::API::MatrixWorkspace_const_sptr& );
  void logsUpdated( const Mantid::API::MatrixWorkspace_const_sptr& );
  void updatePlot( const QString& );

private slots:
  void triggerLiveListener(bool checked);
  void loadFile();
  void launchInstrumentWindow();
  void fillPlotVarCombobox(const Mantid::API::MatrixWorkspace_const_sptr& ws);
  void expandPlotVarCombobox(const Mantid::API::MatrixWorkspace_const_sptr& ws);
  void fillNormalizationCombobox();
  void runStepScanAlg();
  void runStepScanAlgLive(std::string stepScanProperties);

  void updateForNormalizationChange();
  void generateCurve(const QString& var);

  void helpClicked();

private:
  void initLayout();
  void startLiveListener();
  Mantid::API::IAlgorithm_sptr stopLiveListener();
  void setupOptionControls();
  void clearNormalizationCombobox();
  Mantid::API::IAlgorithm_sptr setupStepScanAlg();
  void cleanupWorkspaces();
  void plotCurve();

  void handleAddEvent(Mantid::API::WorkspaceAddNotification_ptr pNf);
  void handleReplEvent(Mantid::API::WorkspaceAfterReplaceNotification_ptr pNf);
  void checkForMaskWorkspace(const std::string& wsName);
  void checkForResultTableUpdate(const std::string& wsName);
  void checkForVaryingLogs(const std::string& wsName);

  Ui::StepScan m_uiForm;  ///< The form generated by Qt Designer
  std::string m_inputWSName, m_tableWSName, m_plotWSName;
  QString m_inputFilename;
  bool m_dataReloadNeeded;
  const std::string m_instrument; ///< The default instrument (for live data)
  Mantid::API::IAlgorithm_sptr m_monitorLiveData; ///< A handle to the running MonitorLiveData (null if none running)

  Poco::NObserver<StepScan, Mantid::API::WorkspaceAddNotification> m_addObserver;
  Poco::NObserver<StepScan, Mantid::API::WorkspaceAfterReplaceNotification> m_replObserver;
};

} // namespace CustomInterfaces
} // namespace MantidQt

#endif //MANTIDQTCUSTOMINTERFACES_ROCKINGCURVE_H_
