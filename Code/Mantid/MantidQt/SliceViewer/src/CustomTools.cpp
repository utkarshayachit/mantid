#include "MantidQtSliceViewer/CustomTools.h"
#include <iomanip>
#include <iosfwd>
#include <iostream>

namespace MantidQt
{
namespace SliceViewer
{

  CustomPicker::CustomPicker(int xAxis, int yAxis, QWidget* canvas)
  : QwtPlotPicker(xAxis, yAxis,CrossRubberBand, AlwaysOn, canvas)
  {
    setRubberBand(QwtPicker::CrossRubberBand);
    canvas->setMouseTracking(true);
  }


/** Called each time the mouse moves over the canvas */
  QwtText CustomPicker::trackerText(const QPoint & pos) const
  {
    emit mouseMoved(pos.x(), pos.y());
    return QwtText();
  }

  void CustomPicker::widgetMouseMoveEvent(QMouseEvent *e)
  {
    if ( !isActive() )
    {
      begin();
      append(e->pos());
    }

    QwtPlotPicker::widgetMouseMoveEvent(e);
  }

  void CustomPicker::widgetLeaveEvent(QEvent *)
  {
    end();
  }

  void CustomMagnifier::rescale(double factor)
  {
    if ( factor != 0.0 )
    {
      QwtPlotMagnifier::rescale(1 / factor);
      emit rescaled(factor);
    }
  }

}
}

