#ifndef CUSTOMTOOLS_H_
#define CUSTOMTOOLS_H_
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_canvas.h>
#include <qwt_picker_machine.h>
#include <QMouseEvent>
/*
 * CustomTools.h
 *
 * Some customized versions of QwtTools for the slice viewer
 *
 *  Created on: Oct 12, 2011
 *      Author: Janik zikovsky
 */

namespace MantidQt
{
namespace SliceViewer
{


//========================================================================
class PickerMachine : public QwtPickerMachine
{
public:
  typedef QList<QwtPickerMachine::Command> CommandList;

  PickerMachine(QwtPickerMachine::SelectionType type)
    : QwtPickerMachine(type) {}

  virtual CommandList transition(
      const QwtEventPattern &, const QEvent *e)
  {
    CommandList cmdList;
    if ( e->type() == QEvent::MouseMove )
    cmdList += Move;

    return cmdList;
  }
};

//========================================================================
/** Customized QwtPlotMagnifier for zooming in on the view */
class CustomMagnifier : public QwtPlotMagnifier
{
  Q_OBJECT
public:
  CustomMagnifier(QWidget* canvas) : QwtPlotMagnifier(canvas)
  {
  }
signals:
  /// Signal to emitted upon scaling.
  void rescaled(double factor) const;  
protected:
  /** Method to flip the way the wheel operates */
  virtual void rescale(double factor);
};


/** Picker for looking at the data under the mouse */
class CustomPicker : public QwtPlotPicker
{
  Q_OBJECT

public:
  CustomPicker(int xAxis, int yAxis, QWidget *canvas);
  void widgetMouseMoveEvent(QMouseEvent *e);
  void widgetLeaveEvent(QEvent *);

signals:
  void mouseMoved(double /*x*/, double /*y*/) const;

protected:
  QwtText trackerText (const QPoint & pos) const;
};



//========================================================================
/** Custom zoomer for zooming onto the slice */
class CustomZoomer: public QwtPlotZoomer
{
public:
  CustomZoomer(QwtPlotCanvas* canvas): QwtPlotZoomer(canvas)
  {
    setTrackerMode(QwtPicker::AlwaysOn);
  }

protected:
  virtual QwtText trackerText( const QPoint& p ) const
  {
    QwtText t( QwtPlotPicker::trackerText( p ));
    QColor c(Qt::white);
    c.setAlpha(120);
    t.setBackgroundBrush( QBrush(c) );
    return t;
  }
};



} // namespace SliceViewer
} // namespace Mantid

#endif /* CUSTOMTOOLS_H_ */
