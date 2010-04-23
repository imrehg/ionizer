#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <math.h>

#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>

#include "histogram_plot.h"
#include "histogram_item.h"

#include <QSettings>
#include <QCloseEvent>
#include <QShowEvent>

histogram_plot::histogram_plot(QWidget *parent,
                               const std::string& xlabel,
                               const std::string& ylabel,
							   const std::string& title) :
	QWidget(parent),
	nPendingReplots(0),
	xMin(1),
	xMax(-1),
	plot(this),
	data_lock(),
	yRange_lock(),
	layout(this)
{
	printf("histogram_plot::histogram_plot\n");

	QObject::connect(this, SIGNAL(signal_replot()), this, SLOT(slot_replot()));

//	plot.setMargin(-50);
	plot.setCanvasBackground(QColor(Qt::white));
	plot.canvas()->setFrameStyle(QFrame::NoFrame | QFrame::Plain );

	grid = new QwtPlotGrid;
	grid->enableXMin(true);
	grid->enableYMin(true);
	grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
	grid->setMinPen(QPen(Qt::gray, 0, Qt::DotLine));
	grid->attach(&plot);

	QwtText txt(title.c_str());
	QFont fnt = txt.font();
	fnt.setPointSize(8);

	if (xlabel.size())
		plot.setAxisTitle(QwtPlot::xBottom, xlabel.c_str());

	if (ylabel.size())
		plot.setAxisTitle(QwtPlot::xBottom, ylabel.c_str());

	if(title.size())
	{
		QwtText txt(title.c_str());
		QFont fnt = txt.font();
		fnt.setPointSize(8);
		txt.setFont(fnt);
		plot.setTitle(txt);
	}

	plot.setAxisFont(QwtPlot::xBottom, fnt);
	plot.setAxisFont(QwtPlot::yLeft, fnt);

	histogram = new HistogramItem();
	histogram->setColor(Qt::darkCyan);
	histogram->attach(&plot);

	layout.addWidget(&plot);
	layout.setSizeConstraint(QLayout::SetNoConstraint);
//	layout.setGeometry(QRect(0, 0, 400, 400));

	plot.replot();
	plot.show();
}

histogram_plot::~histogram_plot()
{
	delete grid;
	delete histogram;
}

void histogram_plot::replot()
{
	if (nPendingReplots <= 0)
	{
		nPendingReplots++;
		emit signal_replot();
	}
}

void histogram_plot::slot_replot()
{
	{
		QReadLocker locker(&yRange_lock);

		if (yMin < yMax)
		{
			plot.setAxisScale(QwtPlot::yLeft, yMin, yMax);
			yMin = yMax;
		}
	}

	{
		QReadLocker locker(&data_lock);
		histogram->setData(QwtIntervalData(intervals, values));
	}

	plot.replot();

	nPendingReplots--;
}

void histogram_plot::SetYRange(double y0, double y1)
{
	QWriteLocker locker(&yRange_lock);

	yMin = y0;
	yMax = y1;
}

void histogram_plot::IncludeY(double y)
{
	QWriteLocker locker(&yRange_lock);

	if (y > yMax)
	{
		yMin = 0;
		yMax = std::min(y * 1.5, 1.0);
	}

	if (yMax > y * 3)
	{
		yMin = 0;
		yMax *= 0.99;
	}
}

void histogram_plot::Plot(const std::valarray<unsigned>& data)
{
	{
		QWriteLocker locker(&data_lock);

		if (intervals.size() != (int)(data.size()))
		{
			intervals.resize(data.size());
			values.resize(data.size());

			for (int i = 0; i < intervals.size(); i++ )
				intervals[i] = QwtDoubleInterval(i, i + 1);
		}

		double nTotal = 0;

		for (size_t i = 0; i < data.size(); i++ )
			nTotal += data[i];

		for (size_t i = 0; i < data.size(); i++ )
			values[i] = data[i] / nTotal;

		double max_val = data.max() / nTotal;

		IncludeY(max_val);
	}

	replot();
}

void histogram_plot::Plot(const std::valarray<double>& data)
{
	{
		QWriteLocker locker(&data_lock);

		if (intervals.size() != (int)(data.size()))
		{
			intervals.resize(data.size());
			values.resize(data.size());

			for (int i = 0; i < intervals.size(); i++ )
				intervals[i] = QwtDoubleInterval(i, i + 1);
		}

		double nTotal = 0;

		for (size_t i = 0; i < data.size(); i++ )
			nTotal += data[i];

		for (size_t i = 0; i < data.size(); i++ )
			values[i] = data[i] / nTotal;

		double max_val = data.max() / nTotal;

		IncludeY(max_val);
	}

	replot();
}

void histogram_plot::barPlot(const std::valarray<double>& data)
{
	if (data.size() == 0)
		return;

	{
		QWriteLocker locker(&data_lock);

		if (intervals.size() != (int)(data.size()))
		{
			intervals.resize(data.size());
			values.resize(data.size());

			for (int i = 0; i < intervals.size(); i++ )
				intervals[i] = QwtDoubleInterval(i, i + 1);
		}

		for (size_t i = 0; i < data.size(); i++ )
			values[i] = data[i];

		double max_val = data.max();
		double min_val = data.min();

		if (max_val > 0)
			max_val *= 2;
		else
			max_val = 0;

		if (min_val < 0)
			min_val *= 2;
		else
			min_val = 0;

		SetYRange(min_val, max_val);
	}

	replot();
}

void histogram_plot::disableXaxis()
{
	plot.enableAxis(QwtPlot::xBottom, false);
}
