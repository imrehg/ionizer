#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "data_plot.h"

#include <Utils.h>
#include <math.h>
#include <assert.h>

#include <qwt_plot_curve.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_grid.h>
#include <qwt_legend_item.h>
#include <qwt_symbol.h>

#include <QSettings>
#include <QPrinter>
#include <QPrintDialog>
#include <QtSvg/QSvgGenerator>

using namespace std;

data_plot::data_plot(QWidget *parent, bool plot_average, int line_width,
                     const std::string& title,
                     const std::string& xlabel,
                     const std::string& ylabel,
                     bool bHasLegend) :
	QWidget(parent),
	title(title),
	iUpdate(0),
	myPlot(this),
	pLegend(0),
	marker(0),
	nPendingReplots(0),
	plot_average(plot_average),
	line_width(line_width),
	xMin(1),
	xMax(-1),
	yMin(1),
	yMax(-1),
	x_offset(0),
	new_xlabel(xlabel),
	new_ylabel(ylabel),
	nReplots(0),
	tStart(0),
	bNewCurves(false),
	plot_lock(QReadWriteLock::Recursive),
	data_lock(QReadWriteLock::Recursive),
	layout(this),
	fnt("Helvetica", 14, QFont::Bold),
	bNewYscale(true),
	defaultColors(10)
{
	 defaultColors[0] = Qt::magenta;
	 defaultColors[1] = Qt::blue;
	 defaultColors[2] = Qt::red;
	 defaultColors[3] = Qt::darkGreen;
	 defaultColors[4] = Qt::darkCyan;
	 defaultColors[5] = Qt::green;
	 defaultColors[6] = Qt::gray;
	 defaultColors[7] = Qt::darkYellow;
	 defaultColors[8] = Qt::darkRed;
	 defaultColors[9] = Qt::darkMagenta;

	 availableColors = defaultColors;


	QObject::connect(this, SIGNAL(signal_replot()), this, SLOT(slot_replot()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(sig_addX(double)), this, SLOT(slot_addX(double)), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(sig_addY(double, unsigned int)), this, SLOT(slot_addY(double, unsigned int)), Qt::QueuedConnection);

	QPalette pal;
	pal.setColor(QPalette::Window, Qt::white);
	setPalette(pal);

	if (bHasLegend)
	{
		pLegend = new QwtLegend(this);
		pLegend->setDisplayPolicy(QwtLegend::FixedIdentifier, QwtLegendItem::ShowText | QwtLegendItem::ShowLine);
		myPlot.setMargin(10);
		myPlot.insertLegend(pLegend, QwtPlot::BottomLegend);
	}

//	setCentralWidget(&myPlot);

	setWindowTitle( title.c_str() );

	myPlot.canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
	myPlot.canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);
	myPlot.setCanvasBackground(Qt::white);
	myPlot.canvas()->setFrameStyle(QFrame::NoFrame | QFrame::Plain );

	d_picker = new PrecisePlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
	                                 QwtPicker::PointSelection | QwtPicker::DragSelection,
	                                 QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, myPlot.canvas());

	d_picker->setRubberBandPen(QColor(Qt::green));
	d_picker->setRubberBand(QwtPicker::CrossRubberBand);
	d_picker->setTrackerPen(QColor(Qt::black));
	d_picker->setEnabled(true);

	zoomer = new QwtPlotZoomer(myPlot.canvas());
	grid = new QwtPlotGrid();

	grid->attach(&myPlot);
	grid->setPen(QPen(Qt::gray));

	myPlot.setAutoReplot(false);
	layout.addWidget(&myPlot);
	myPlot.replot();
	myPlot.show();
}

void data_plot::clear()
{
	// clear data from all curves
	// curves themselves remain

	X.clear();

	for (unsigned i = 0; i < Ys.size(); i++)
		Ys[i].clear();

	for (unsigned i = 0; i < y_averaged.size(); i++)
		y_averaged[i].clear();

}

data_plot::~data_plot()
{
	while (!curves.empty())
	{
		delete curves.back();
		curves.pop_back();
	}

	while (!averaged_curves.empty())
	{
		delete averaged_curves.back();
		averaged_curves.pop_back();
	}


	delete d_picker;
	delete zoomer;
	delete grid;

	if (pLegend)
		delete pLegend;

	if (marker)
		delete marker;
}

void data_plot::set_start_time(double t)
{
	tStart = t;
}

void data_plot::showEvent(QShowEvent * event )
{
	if (event->spontaneous() == false)
	{
		QSettings settings("NIST", "Aluminizer2");
		restoreGeometry(settings.value((title + "/geometry").c_str()).toByteArray());
	}

	QWidget::showEvent(event);
}

void data_plot::closeEvent(QCloseEvent *event)
{
	QSettings settings("NIST", "Aluminizer2");

	settings.setValue((title + "/geometry").c_str(), saveGeometry());

	QWidget::closeEvent(event);
}

void data_plot::addX(double x)
{
	emit sig_addX(x);
}

void data_plot::addY(double y, const char* name)
{
	if (name)
	{
		for (size_t i = 0; i < curveNames.size(); i++)
			if (strcmp(name, curveNames[i].c_str()) == 0)
				emit sig_addY(y, i);
	}
}

void data_plot::slot_addX(double x)
{
	QWriteLocker locker(&data_lock);

	X.push_back(x - x_offset);

	for (size_t i = 0; i < Ys.size(); i++)
		Ys[i].resize(X.size());
}

void data_plot::slot_addY(double y, unsigned int iCurve)
{
	QWriteLocker locker(&data_lock);

	//todo: there was a crash bug here, because sometimes
	// Ys.at(iCurve).size() == 0.  But slot_addX should have been
	// called prior.  why didn't this work?

	if (Ys.at(iCurve).size())
		Ys.at(iCurve).back() = y;

	if (plot_average && X.size())
		y_averaged[iCurve][X.back()].add(y);
}

void data_plot::setMarkerText(const std::string& s, int font_size)
{
	QWriteLocker locker(&plot_lock);

	markerFS = font_size;
	markerText = s;
}

void data_plot::clearData()
{
	QWriteLocker locker(&data_lock);

	X.clear();

	for (unsigned i = 0; i < Ys.size(); i++)
		Ys[i].clear();

	xMin = 1;
	xMax = -1;
	yMin = 1;
	yMax = -1;

}

void data_plot::addXY(double x, double* y, unsigned nY)
{
	QWriteLocker locker(&data_lock);

	X.push_back(x - x_offset);

	static double lastX = 0;

	if (lastX < x)
		lastX = x;
	else
		cout << lastX << " > " << x << endl;

	for (unsigned i = 0; i < nY; i++)
	{
		if (y[i] > yMax)
		{
			yMax = y[i] * 1.1;
			bNewYscale = true;
		}

		if (y[i] < yMin)
		{
			if (y[i] < 0)
				yMin = y[i] * 1.1;
			else
				yMin = y[i] * 0.9;

			bNewYscale = true;
		}


		Ys.at(i).push_back(y[i]);

		if (plot_average)
			y_averaged[i][x - x_offset].add(y[i]);
	}
}

QColor data_plot::chooseCurveColor(const char* label, int* iLineWidth)
{
	if(availableColors.size() == 0)
		availableColors = defaultColors;

	QColor clr = availableColors[0];

#ifndef NO_COLOR_PREF
	global_getTracePreference(label, &clr, iLineWidth);
#endif

	//delete selected color from available colors list if it's there
	vector<QColor>::iterator it = std::find(availableColors.begin(), availableColors.end(), clr);

	if(it != availableColors.end())
		availableColors.erase(it);

	return clr;
}

void data_plot::addCurve(const char* label, bool bPlotAverage, int iLineWidth,
                         Qt::PenStyle ps, bool bNeedsUpdate)
{
	if (this->plot_average && bPlotAverage)
		curves.push_back(new QwtPlotCurve());
	else
		curves.push_back(new QwtPlotCurve(label));

	curveNames.push_back(label);

	curvesThatNeedUpdate.push_back(bNeedsUpdate);


	QColor clr = chooseCurveColor(label, &iLineWidth);

	QPen line_pen( clr );
	line_pen.setStyle(ps);

	if (iLineWidth >= 0)
		line_pen.setWidth(iLineWidth);
	else
		line_pen.setWidth(line_width);

	if (this->plot_average && bPlotAverage)
	{
		QwtSymbol symb(QwtSymbol::Rect, QBrush(clr), QPen(clr), QSize(2, 2));
		curves.back()->setPen(QPen(clr));
		curves.back()->setStyle(QwtPlotCurve::Dots );
		//	curves.back()->setSymbol(symb);

		averaged_curves.push_back(new QwtPlotCurve(QString(label)));
		averaged_curves.back()->setPen(line_pen);
		averaged_curves.back()->setStyle(QwtPlotCurve::Lines);

		y_averaged.push_back(map_xy_t());
	}
	else
	{
		curves.back()->setPen(line_pen);
		curves.back()->setStyle(QwtPlotCurve::Lines);
	}

	Ys.push_back( xy_t(0) );

	bNewCurves = true;
}



void data_plot::addCurveXY(const char* label, const std::vector<double>& vx, const std::vector<double>& vy)
{
	assert(vx.size() == vy.size());

	QWriteLocker locker1(&plot_lock);
	QWriteLocker locker2(&data_lock);

	addCurve(label, false, 0, Qt::SolidLine, false);
	curves.back()->setData(&(vx[0]), &(vy[0]), vx.size());
}

void data_plot::setXYdata(const xy_t& x, const xy_t& y, size_t iCurve)
{
	size_t minSize = std::min(x.size(), y.size());

	if (minSize)
	{
		QWriteLocker locker(&plot_lock);

//		if(iCurve < curves.size() && iCurve < y_averaged.size())
		{
			curves.at(iCurve)->setData(&(x[0]), &(y[0]), minSize);

			if (plot_average && iCurve < y_averaged.size())
			{
				vector<double> xa(y_averaged[iCurve].size());
				vector<double> ya(y_averaged[iCurve].size());

				size_t j = 0;

				for (map_xy_t::const_iterator ixy = y_averaged[iCurve].begin(); ixy != y_averaged[iCurve].end(); ixy++)
				{
					xa[j] = ixy->first;
					ya[j] = ixy->second.average();

					j++;
				}

				averaged_curves.at(iCurve)->setData(&(xa[0]), &(ya[0]), xa.size());
			}
		}
	}
}

void data_plot::replot(bool bForce, const std::string& type)
{
	if (nPendingReplots <= 0 || bForce)
	{
		QWriteLocker locker(&plot_lock);

		next_replot_type = type;
		nPendingReplots++;
		emit signal_replot();
	}
}

void data_plot::slot_replot()
{
	QWriteLocker locker(&plot_lock);

	if (markerText.length() > 0)
	{
		QwtText txt(QString(markerText.c_str()));
		txt.setFont(QFont("Helvetica", markerFS, QFont::Bold));

		if (!marker)
		{
			marker = new QwtPlotMarker();

			marker->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
			marker->setLineStyle(QwtPlotMarker::NoLine);
			marker->attach(&myPlot);
			marker->show();
		}

		marker->setLabel(txt);
		markerText = "";
	}

	if (marker)
	{
		if (xMin < xMax)
			marker->setXValue(0.1 * xMax + 0.9 * xMin);

		if (bNewYscale && xMin < xMax)
			marker->setYValue(0.7 * yMin + 0.3 * yMax);
	}

	if (bNewCurves)
	{
		for (unsigned i = 0; i < curves.size(); i++)
		{
			if (curves[i]->title().text() == "")
				curves[i]->setItemAttribute(QwtPlotItem::Legend, false);

			curves[i]->attach(&myPlot);
		}

		for (unsigned i = 0; i < averaged_curves.size(); i++)
			averaged_curves[i]->attach(&myPlot);


		bNewCurves = false;
	}

	if (xMin < xMax)
	{
		myPlot.setAxisScale(QwtPlot::xBottom, xMin, xMax);
		xMin = xMax;
	}

	if (bNewYscale && yMin < yMax)
	{
		myPlot.setAxisScale(QwtPlot::yLeft, yMin, yMax);
		bNewYscale = false;
	}

	//update axis labels
	if (new_xlabel.length() > 0)
	{
		QwtText l(new_xlabel.c_str());
		l.setFont(myPlot.axisFont(QwtPlot::xBottom));
		myPlot.setAxisTitle(QwtPlot::xBottom, l);
		new_xlabel = "";
	}

	if (new_ylabel.length() > 0)
	{
		QwtText l(new_ylabel.c_str());
		l.setFont(myPlot.axisFont(QwtPlot::yLeft));
		myPlot.setAxisTitle(QwtPlot::yLeft, l);
		new_ylabel = "";
	}

	//make sure there is a title
	if (myPlot.title().text().length() == 0)
	{
		QwtText t(GetDateTimeString(tStart).c_str());
		t.setFont(myPlot.axisFont(QwtPlot::xBottom));
		myPlot.setTitle(t);
	}

	updateCurves();

	if (next_replot_type == "")
		myPlot.replot();

	if (next_replot_type == "PDF")
	{
		savePDF();
		next_replot_type = "";
		myPlot.replot();
	}

	if (next_replot_type == "SVG")
	{
		saveSVG();
		next_replot_type = "";
		myPlot.replot();
	}

	nReplots++;

	if (nReplots == 5)
		zoomer->setZoomBase();

	nPendingReplots--;
}

void data_plot::updateCurves()
{
	QWriteLocker locker(&data_lock);

	for (size_t i = 0; i < Ys.size(); i++)
		if (curvesThatNeedUpdate.at(i) == true)
			setXYdata(X, Ys[i], i);
}

void data_plot::set_xrange(double x0, double x1)
{
	QWriteLocker plot_locker(&plot_lock);

	bool bFilter = false;

	if (xMin != x0)
	{
		xMin = x0;
		bFilter = true;
	}

	if (xMax != x1)
	{
		xMax = x1;
		bFilter = true;
	}

	if (bFilter)
	{
		size_t i = 0;

		QWriteLocker data_locker(&data_lock);

		while (i < X.size())
		{
			if (X[i] < xMin || X[i] > xMax)
			{
				X.erase(X.begin() + i);

				for (size_t j = 0; j < Ys.size(); j++)
				{
					if (X.size() + 1 != Ys[j].size())
						throw runtime_error("[data_plot::set_xrange] inconsistent plot data");

					Ys[j].erase(Ys[j].begin() + i);
				}
			}
			else
				i++;
		}
	}
}

void data_plot::set_xoffset(double x0)
{
	QWriteLocker locker(&plot_lock);

	x_offset = x0;
}

void data_plot::set_xlabel(const std::string& s)
{
	QWriteLocker locker(&plot_lock);

	new_xlabel = s;
}

void data_plot::set_ylabel(const std::string& s)
{
	QWriteLocker locker(&plot_lock);

	new_ylabel = s;
}

void data_plot::savePDF()
{
	if (save_file_name.length() > 0)
	{
		QPrinter printer;
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOrientation(QPrinter::Landscape);
		printer.setOutputFileName((save_file_name + ".pdf").c_str());
		printer.setCreator("Aluminizer2");

		myPlot.print(printer);
	}
}

void data_plot::saveSVG()
{
	if (save_file_name.length() > 0)
	{
		QSvgGenerator svg;
		svg.setFileName((save_file_name + ".svg").c_str());
		svg.setSize(QSize(600, 450));

		myPlot.print(svg);

		svg.setFileName((save_file_name + "_2.svg").c_str());
	}
}

bool data_plot::canPrint()
{
	return true;
}

void data_plot::print()
{
	QPrinter printer;

	printer.setOrientation(QPrinter::Landscape);

	QPrintDialog dialog(&printer, &myPlot);
	dialog.setWindowTitle(tr("Print Document"));

	if (dialog.exec() == QDialog::Accepted)
		myPlot.print(printer);
}

void data_plot::set_save_file_name(const std::string& s)
{
	save_file_name = s;
}

plot_window::plot_window(QWidget *parent, QWidget* pPlot,
                         const std::string& window_title,
                         const std::string& settings_name) :
	QMainWindow(parent),
	pPlot(pPlot),
	window_title(window_title),
	settings_name(settings_name)
{
	if (settings_name.empty())
		this->settings_name = this->window_title;

	QPalette pal;
	pal.setColor(QPalette::Window, Qt::white);
	setPalette(pal);

	setCentralWidget(pPlot);
	setWindowTitle( window_title.c_str() );
	pPlot->show();
}

plot_window::~plot_window()
{
	if (pPlot)
		delete pPlot;
}

void plot_window::showEvent(QShowEvent * event )
{
	if (event->spontaneous() == false)
	{
		QSettings settings("NIST", "Aluminizer2");
		restoreGeometry(settings.value((settings_name + "/geometry").c_str()).toByteArray());
	}

	QMainWindow::showEvent(event);
}

void plot_window::closeEvent(QCloseEvent *event)
{
	QSettings settings("NIST", "Aluminizer2");

	settings.setValue((settings_name + "/geometry").c_str(), saveGeometry());
	QMainWindow::closeEvent(event);
}
