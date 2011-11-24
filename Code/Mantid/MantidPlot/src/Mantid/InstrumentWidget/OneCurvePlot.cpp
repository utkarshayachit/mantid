#include "OneCurvePlot.h"
#include "PeakMarker2D.h"

#include <qwt_plot_curve.h>
#include <qwt_scale_div.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_canvas.h>
#include <qwt_compat.h>
#include <qwt_plot_zoomer.h>

#include <QFontMetrics>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QPainter>

#include <iostream>
#include <cmath>

OneCurvePlot::OneCurvePlot(QWidget* parent):
QwtPlot(parent),m_curve(NULL)
{
  setAxisFont(QwtPlot::xBottom, parent->font());
  setAxisFont(QwtPlot::yLeft, parent->font());
  canvas()->setCursor(Qt::ArrowCursor);
  //setMouseTracking(true);
  //canvas()->setMouseTracking(true);
  setContextMenuPolicy(Qt::DefaultContextMenu);
  m_zoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft,
      QwtPicker::DragSelection | QwtPicker::CornerToCorner, QwtPicker::AlwaysOff, canvas());
  m_zoomer->setRubberBandPen(QPen(Qt::black));
  QList<QColor> colors;
  m_colors << Qt::red<< Qt::green  << Qt::blue << Qt::cyan << Qt::magenta << Qt::yellow << Qt::gray;
  m_colors << Qt::darkRed<< Qt::darkGreen  << Qt::darkBlue << Qt::darkCyan << Qt::darkMagenta << Qt::darkYellow << Qt::darkGray;
  m_colorIndex = 0;
}

/**
  * Set the scale of the horizontal axis
  * @param from :: Minimum value
  * @param to :: Maximum value
  */
void OneCurvePlot::setXScale(double from, double to)
{
  QFontMetrics fm(this->font());
  int n = from != 0.0 ? abs(static_cast<int>(floor(log10(fabs(from))))) : 0;
  int n1 = to != 0.0 ? abs(static_cast<int>(floor(log10(fabs(to))))) : 0;
  if (n1 > n) n = n1;
  n += 4;
  // approxiamte width of a tick label in pixels
  int labelWidth = n * fm.width("0");
  // calculate number of major ticks
  int nMajorTicks = this->width() / labelWidth;
  // try creating a scale
  const QwtScaleDiv div = axisScaleEngine(QwtPlot::xBottom)->divideScale(from,to,nMajorTicks,nMajorTicks);
  // Major ticks are placed at round numbers so the first or last tick could be missing making
  // scale look ugly. Trying to fix it if possible
  bool rescaled = false;
  // get actual tick positions
  const QwtValueList& ticks = div.ticks(QwtScaleDiv::MajorTick);
  if (!ticks.empty() && ticks.size() < nMajorTicks)
  {
    // how much first tick is shifted from the lower bound
    double firstShift = ticks.front() - div.lBound();
    // how much last tick is shifted from the upper bound
    double lastShift = div.hBound() - ticks.back();
    // range of the scale
    double range = fabs(div.hBound() - div.lBound());
    // we say that 1st tick is missing if first tick is father away from its end of the scale
    // than the last tick is from its end
    bool isFirstMissing =  fabs(firstShift) > fabs(lastShift) ;
    // if first tick is missing
    if (isFirstMissing)
    {
      // distance between nearest major ticks
      double tickSize = 0;
      if (ticks.size() == 1)
      {
        // guess the tick size in case of only one visible
        double tickLog = log10(firstShift);
        tickLog = tickLog > 0 ? ceil(tickLog) : floor(tickLog);
        tickSize = pow(10.,tickLog);
      }
      else if (ticks.size() > 1)
      {
        // take the difference between the two first ticks
        tickSize = ticks[1] - ticks[0];
      }
      // claculate how much lower bound must be moved to make the missing tick visible
      double shift = (ticks.front() - tickSize) - from;
      // if the shift is not very big rescale the axis
      if (fabs(shift/range) < 0.1)
      {
        from += shift;
        const QwtScaleDiv updatedDiv = axisScaleEngine(QwtPlot::xBottom)->divideScale(from,to,nMajorTicks,nMajorTicks);
        setAxisScaleDiv(xBottom,updatedDiv);
        rescaled = true;
      }
    }
    else // last tick is missing
    {
      // distance between nearest major ticks
      double tickSize = 0;
      if (ticks.size() == 1)
      {
        // guess the tick size in case of only one visible
        double tickLog = log10(lastShift);
        tickLog = tickLog > 0 ? ceil(tickLog) : floor(tickLog);
        tickSize = pow(10.,tickLog);
      }
      else if (ticks.size() > 1)
      {
        // take the difference between the two first ticks
        tickSize = ticks[1] - ticks[0];
      }
      // claculate how much upper bound must be moved to make the missing tick visible
      double shift = (ticks.back() + tickSize) - to;
      // if the shift is not very big rescale the axis
      if (fabs(shift/range) < 0.1)
      {
        to += shift;
        const QwtScaleDiv updatedDiv = axisScaleEngine(QwtPlot::xBottom)->divideScale(from,to,nMajorTicks,nMajorTicks);
        setAxisScaleDiv(xBottom,updatedDiv);
        rescaled = true;
      }
    }
  }

  if (!rescaled)
  {
    setAxisScaleDiv(xBottom,div);
  }
  m_zoomer->setZoomBase();
}

/**
  * Set the scale of the vertical axis
  * @param from :: Minimum value
  * @param to :: Maximum value
  */
void OneCurvePlot::setYScale(double from, double to)
{
  if (isYLogScale())
  {
    if (from == 0 && to == 0)
    {
      from = 1;
      to = 10;
    }
    else
    {
      double yPositiveMin = to;
      QMap<QString,QwtPlotCurve*>::const_iterator cv = m_stored.begin();
      QwtPlotCurve* curve = NULL;
      do
      {
        if (cv != m_stored.end())
        {
          curve = cv.value();
          ++cv;
        }
        else if (curve == m_curve)
        {
          curve = NULL;
          break;
        }
        else
        {
          curve = m_curve;
        }
        if (!curve) break;
        int n = curve->dataSize();
        for(int i = 0; i < n; ++i)
        {
          double y = curve->y(i);
          if (y > 0 && y < yPositiveMin)
          {
            yPositiveMin = y;
          }
        }
      }while(curve);
      from = yPositiveMin;
    }
  }
  setAxisScale(QwtPlot::yLeft,from,to);
  m_zoomer->setZoomBase();
}

/**
  * Set the data for the curve to display
  * @param x :: A pointer to x values
  * @param y :: A pointer to y values
  * @param dataSize :: The size of the data
  */
void OneCurvePlot::setData(const double* x,const double* y,int dataSize)
{
  if (!m_curve)
  {
    m_curve = new QwtPlotCurve();
    m_curve->attach(this);
  }

  m_curve->setData(x,y,dataSize);
}

/**
 * Set a label which will identify the curve when it is stored.
 */
void OneCurvePlot::setLabel(const QString& label)
{
  m_label = label;
}

/**
  * Remove the curve. Rescale the axes if there are stored curves.
  */
void OneCurvePlot::clearCurve()
{
  // remove the curve
  if (m_curve)
  {
    m_curve->attach(0);
    m_curve = NULL;
  }
  // if there are stored curves rescale axes to make them fully visible
  if (hasStored())
  {
    const QwtScaleDiv *divX = axisScaleDiv(QwtPlot::xBottom);
    double xmin = divX->lBound();
    double xmax = divX->hBound();
    const QwtScaleDiv *divY = axisScaleDiv(QwtPlot::yLeft);
    double ymin = divY->lBound();
    double ymax = divY->hBound();
    QMap<QString,QwtPlotCurve*>::const_iterator curve = m_stored.begin();
    for(;curve!=m_stored.end();++curve)
    {
      QwtDoubleRect br = (**curve).boundingRect();
      if (br.left() < xmin) xmin = br.left();
      if (br.right() > xmax) xmax = br.right();
      if (br.top() < ymin) ymin = br.top();
      if (br.bottom() > ymax) ymax = br.bottom();
    }
    setXScale(xmin,xmax);
    setYScale(ymin,ymax);
  }
}

void OneCurvePlot::resizeEvent(QResizeEvent *e)
{
  QwtPlot::resizeEvent(e);
  recalcAxisDivs();
}

/**
  * Recalculate axis divisions to make sure that tick labels don't overlap
  */
void OneCurvePlot::recalcAxisDivs()
{
  recalcXAxisDivs();
  recalcYAxisDivs();
}

/**
  * Recalculate x-axis divisions to make sure that tick labels don't overlap
  */
void OneCurvePlot::recalcXAxisDivs()
{
  const QwtScaleDiv *div0 = axisScaleDiv(QwtPlot::xBottom);
  double from = div0->lBound();
  double to = div0->hBound();
  setXScale(from,to);
}

/**
  * Recalculate y-axis divisions to make sure that tick labels don't overlap
  */
void OneCurvePlot::recalcYAxisDivs()
{
  const QwtScaleDiv *div0 = axisScaleDiv(QwtPlot::yLeft);
  double from = div0->lBound();
  double to = div0->hBound();
  setYScale(from,to);
}

void OneCurvePlot::contextMenuEvent (QContextMenuEvent *e)
{
  // context menu will be handled with mouse events
  e->accept();
}

void OneCurvePlot::mousePressEvent(QMouseEvent* e)
{
  if (e->buttons() & Qt::RightButton)
  {
    if (m_zoomer->zoomRectIndex() == 0)
    {
      e->accept();
      // plot owner will display and process context menu
      emit showContextMenu();
    }
    return;
  }
  if (e->buttons() & Qt::LeftButton)
  {
    e->accept();
    m_x0 = e->x();
    m_y0 = e->y();
  }
}

void OneCurvePlot::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton)
  {
    if (m_x0 == e->x() && m_y0 == e->y())
    {// there were no dragging
      emit clickedAt(invTransform(xBottom,e->x()-canvas()->x()),invTransform(yLeft,e->y()-canvas()->y()));
    }
  }
}

void OneCurvePlot::setYAxisLabelRotation(double degrees)
{
  axisScaleDraw(yLeft)->setLabelRotation(degrees);
}

/**
  * Set the log scale on the y axis
  */
void OneCurvePlot::setYLogScale()
{
  const QwtScaleDiv *div = axisScaleDiv(QwtPlot::yLeft);
  double from = div->lBound();
  double to = div->hBound();
  QwtLog10ScaleEngine* logEngine = new QwtLog10ScaleEngine();
  setAxisScaleEngine(yLeft,logEngine);
  setYScale(from,to);
  recalcYAxisDivs();
  replot();
}

/**
  * Set the linear scale on the y axis
  */
void OneCurvePlot::setYLinearScale()
{
  QwtLinearScaleEngine* engine = new QwtLinearScaleEngine();
  setAxisScaleEngine(yLeft,engine);
  replot();
}

/**
 * Add new peak label
 * @param label :: A pointer to a PeakLabel, becomes owned by OneCurvePlot
 */
void OneCurvePlot::addPeakLabel(PeakLabel* label)
{
  label->attach(this);
  m_peakLabels.append(label);
}

/**
 * Removes all peak labels.
 */
void OneCurvePlot::clearPeakLabels()
{
  foreach(PeakLabel* label, m_peakLabels)
  {
    label->detach();
    delete label;
  }
  m_peakLabels.clear();
}

/**
 * Returns true if the current curve isn't NULL
 */
bool OneCurvePlot::hasCurve()const
{
  return m_curve != NULL;
}

/**
 * Store current curve.
 */
void OneCurvePlot::store()
{
  if (m_curve)
  {
    removeCurve(m_label);
    m_stored.insert(m_label,m_curve);
    m_curve->setPen(QPen(m_colors[m_colorIndex]));
    ++m_colorIndex;
    m_colorIndex %= m_colors.size();
    m_curve = NULL;
    m_label = "";
  }
}

/**
 * Returns true if there are some stored curves.
 */
bool OneCurvePlot::hasStored()const
{
  return ! m_stored.isEmpty();
}

QStringList OneCurvePlot::getLabels()const
{
  QStringList out;
  QMap<QString,QwtPlotCurve*>::const_iterator it = m_stored.begin();
  for(;it!=m_stored.end();++it)
  {
    out << it.key();
  }
  return out;
}

/**
 * Remove a stored curve.
 * @param label :: The label of a curve to remove.
 */
void OneCurvePlot::removeCurve(const QString& label)
{
  QMap<QString,QwtPlotCurve*>::iterator it = m_stored.find(label);
  if (it != m_stored.end())
  {
    it.value()->detach();
    m_stored.erase(it);
  }
}

/**
 * Does the y axis have the log scale?
 */
bool OneCurvePlot::isYLogScale()const
{
  const QwtScaleEngine *engine = axisScaleEngine(yLeft);
  return dynamic_cast<const QwtLog10ScaleEngine*>(engine) != NULL;
}

/* ---------------------------- PeakLabel --------------------------- */

/**
 * Draw PeakLabel on a plot
 */
void PeakLabel::draw(QPainter *painter, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &canvasRect) const
{
  (void)yMap;
  int x = xMap.transform(m_marker->getTOF());
  int y = static_cast<int>(canvasRect.top() + m_marker->getLabelRect().height());
  painter->drawText(x,y,m_marker->getLabel());
  //std::cerr << x << ' ' << y << ' ' << m_marker->getLabel().toStdString() << std::endl;
}

