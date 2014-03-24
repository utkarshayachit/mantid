/***************************************************************************
    File                 : QwtBarCurve.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Bar curve

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "QwtBarCurve.h"
#include <QPainter>

QwtBarCurve::QwtBarCurve(BarStyle style, Table *t, const QString& xColName, const QString& name, int startRow, int endRow):
    DataCurve(t, xColName, name, startRow, endRow)
{
bar_offset=0;
bar_gap=0;
bar_style=style;

setPen(QPen(Qt::black, 1, Qt::SolidLine));
setBrush(QBrush(Qt::red));
setStyle(QwtPlotCurve::UserCurve);

if (bar_style == Vertical)
	setType(Graph::VerticalBars);
else
	setType(Graph::HorizontalBars);
}

void QwtBarCurve::copy(const QwtBarCurve *b)
{
bar_gap = b->bar_gap;
bar_offset = b->bar_offset;
bar_style = b->bar_style;
}

void QwtBarCurve::draw(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, int from, int to) const
{
   if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = static_cast<int>(dataSize()) - 1;

    painter->save();
    painter->setPen(QwtPlotCurve::pen());
    painter->setBrush(QwtPlotCurve::brush());

    double ref;
	double bar_width = 0;

    if (bar_style == Vertical)
       ref= yMap.transform(1e-100); //smalest positive value for log scales
    else
       ref= xMap.transform(1e-100);

        QPointF fromPt = sample(from);
        QPointF fromP1Pt = sample(from+1);
        if (bar_style == Vertical)
	{
		double dx = fabs(xMap.transform(fromP1Pt.x())-xMap.transform(fromPt.x()));
		for (int i=from+2; i<to; i++)
		{
		    QPointF pt = sample(i);
		    QPointF ptP1 = sample(i+1);
		    double min = fabs(xMap.transform(ptP1.x())-xMap.transform(pt.x()));
			if (min <= dx)
				dx=min;
		}
		bar_width = dx*(1-bar_gap*0.01);
	}
	else
	{
		double dy = fabs(yMap.transform(fromP1Pt.y())-yMap.transform(fromPt.y()));
		for (int i=from+2; i<to; i++)
		{
		    QPointF pt = sample(i);
		    QPointF ptP1 = sample(i+1);
			double min = fabs(yMap.transform(ptP1.y())-yMap.transform(pt.y()));
			if (min <= dy)
				dy=min;
		}
		bar_width = dy*(1-bar_gap*0.01);
	}

	const double half_width = (0.5-bar_offset*0.01)*bar_width;
	double bw1 = bar_width + 1;
	for (int i=from; i<=to; i++)
	{
		QPointF pt = sample(i);
		const double px = xMap.transform(pt.x());
		const double py = yMap.transform(pt.y());

		if (bar_style == Vertical)
		{
			if (pt.y() < 0.0)
				painter->drawRect(QRectF(px-half_width, ref, bw1, (py-ref)));
			else
				painter->drawRect(QRectF(px-half_width, py, bw1, (ref-py+1)));
		}
		else
		{
			if (pt.x() < 0.0)
				painter->drawRect(QRectF(px, py-half_width, (ref-px), bw1));
			else
				painter->drawRect(QRectF(ref, py-half_width, (px-ref), bw1));
		}
	}
	painter->restore();
}

QRectF QwtBarCurve::boundingRect() const
{
QRectF rect = QwtPlotCurve::boundingRect();
double n= (double)dataSize();

if (bar_style == Vertical)
	{
	double dx=(rect.right()-rect.left())/n;
	rect.setLeft(rect.left()-dx);
	rect.setRight(rect.right()+dx);
	}
else
	{
	double dy=(rect.bottom()-rect.top())/n;
	rect.setTop(rect.top()-dy);
	rect.setBottom(rect.bottom()+dy);
	}

return rect;
}

void QwtBarCurve::setGap (int gap)
{
if (bar_gap == gap)
	return;

bar_gap =gap;
}

void QwtBarCurve::setOffset(int offset)
{
if (bar_offset == offset)
	return;

bar_offset = offset;
}

double QwtBarCurve::dataOffset()
{
	if (bar_style == Vertical)
	{
		const QwtScaleMap &xMap = plot()->canvasMap(xAxis());
		double dx = fabs(xMap.transform(sample(0).x())-xMap.transform(sample(0).x()));
		double bar_width = dx*(1-bar_gap*0.01);
		if (plot()->isVisible())
		{
			for (int i = 2; i<static_cast<int>(dataSize()); i++)
			{
				double min = fabs(xMap.transform(sample(i+1).x())-xMap.transform(sample(i).x()));
				if (min <= dx)
					dx=min;
			}
			double x1 = xMap.transform(minXValue()) + bar_offset*0.01*bar_width;
			return xMap.invTransform(x1) - minXValue();
		}
		else
			return 0.5*bar_offset*0.01*bar_width;
	}
	else
	{
		const QwtScaleMap &yMap = plot()->canvasMap(yAxis());
		double dy = fabs(yMap.transform(sample(1).y())-yMap.transform(sample(0).y()));
		double bar_width = dy*(1-bar_gap*0.01);
		if (plot()->isVisible())
		{
			for (int i=2; i<static_cast<int>(dataSize()); i++)
			{
				double min = fabs(yMap.transform(sample(i+1).y())-yMap.transform(sample(i).y()));
				if (min <= dy)
					dy=min;
			}
			double y1 = yMap.transform(minYValue()) + bar_offset*0.01*bar_width;
			return yMap.invTransform(y1) - minYValue();
		}
		else
			return 0.5*bar_offset*0.01*bar_width;
	}
}
