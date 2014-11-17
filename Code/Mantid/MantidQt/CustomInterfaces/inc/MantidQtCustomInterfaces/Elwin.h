#ifndef MANTIDQTCUSTOMINTERFACESIDA_ELWIN_H_
#define MANTIDQTCUSTOMINTERFACESIDA_ELWIN_H_

#include "MantidAPI/MatrixWorkspace.h"
#include "MantidQtCustomInterfaces/IDATab.h"

namespace MantidQt
{
namespace CustomInterfaces
{
namespace IDA
{
  class DLLExport Elwin : public IDATab
  {
    Q_OBJECT

  public:
    Elwin(QWidget * parent = 0);

  private:
    virtual void setup();
    virtual void run();
    virtual bool validate();
    virtual void loadSettings(const QSettings & settings);
    virtual QString helpURL() {return "Elwin";}
    void setDefaultResolution(Mantid::API::MatrixWorkspace_const_sptr ws);
    void setDefaultSampleLog(Mantid::API::MatrixWorkspace_const_sptr ws);

  private slots:
    void plotInput();
    void twoRanges(QtProperty *, bool);
    void minChanged(double val);
    void maxChanged(double val);
    void updateRS(QtProperty * prop, double val);

  private:
    QtTreePropertyBrowser* m_elwTree;

  };
} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQTCUSTOMINTERFACESIDA_ELWIN_H_ */
