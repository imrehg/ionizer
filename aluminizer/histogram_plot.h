#pragma once

#include <valarray>

#include <QReadWriteLock>
#include <QAbstractTableModel>
#include <QTime>

#include <qwt_plot.h>
#include <qwt_interval_data.h>

class QwtPlotGrid;
class HistogramItem;


class histogram_plot : public QWidget
{
public:
Q_OBJECT

public:
histogram_plot(QWidget *parent,
               const std::string& xlabel,
               const std::string& ylabel,
			   const std::string& title="");

virtual ~histogram_plot();

void disableXaxis();

void Plot(const std::valarray<unsigned>& data);
void Plot(const std::valarray<double>& data);

//! plot data w/o binning
void barPlot(const std::valarray<double>& data);

void SetYRange(double y0, double y1);
void IncludeY(double y);

void replot();

public slots:
void slot_replot();

signals:
void signal_replot();

protected:

int nPendingReplots;

double xMin;
double xMax;
double yMin;
double yMax;

int nHist;
unsigned numValues;

QwtPlot plot;
QReadWriteLock data_lock;
QReadWriteLock yRange_lock;

QwtPlotGrid *grid;
HistogramItem* histogram;

QwtArray<QwtDoubleInterval> intervals;
QwtArray<double> values;

QHBoxLayout layout;
};



class simple_histogram : public QWidget
{
public:
	simple_histogram(QWidget *parent , const std::string& title);

	//! set data (already binned)
	void barPlot(const std::valarray<double>& data);
	
protected:

	virtual void mouseMoveEvent ( QMouseEvent * );
	virtual void paintEvent ( QPaintEvent * event );

	void drawBar(QPainter *painter, Qt::Orientation, const QRect& rect) const;

	std::valarray<double> hist_data;
	
	std::string title;
	double scale;
	int x, y;
	QTime tLastMouse;
	bool bNewData;
};

