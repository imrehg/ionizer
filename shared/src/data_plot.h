#ifndef DATA_PLOT_H
#define DATA_PLOT_H

#include <QtGui/QMainWindow>
#include <QTimer>
#include <QReadWriteLock>
#include <QCloseEvent>
#include <QHBoxLayout>

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_plot_marker.h>

#include <vector>
#include <string>

#include "string_func.h"

class QwtPlotCurve;
class QwtPlotZoomer;
class QwtPlotGrid;

class PrecisePlotPicker : public QwtPlotPicker
{
public:
PrecisePlotPicker(int xAxis, int yAxis, int selectionFlags,
                  RubberBand rubberBand, DisplayMode trackerMode,
                  QwtPlotCanvas *canvas, double xOffset = 0, unsigned xDigits = 6, unsigned yDigits = 2) :
	QwtPlotPicker(xAxis, yAxis, selectionFlags, rubberBand, trackerMode, canvas),
	xOffset(xOffset),
	xDigits(xDigits),
	yDigits(yDigits)
{
}

virtual ~PrecisePlotPicker()
{
}

virtual QwtText trackerText(const QwtDoublePoint &pos) const
{
	QString text;

	char fmtX[64];
	char fmtY[64];
	char fmtXY[64];

	snprintf(fmtX, 64, "%%.%df", xDigits);
	snprintf(fmtY, 64, "%%.%df", yDigits);
	snprintf(fmtXY, 64, "%s, %s", fmtX, fmtY);

	switch (rubberBand())
	{
	case HLineRubberBand:
		text.sprintf(fmtY, pos.y());
		break;
	case VLineRubberBand:
		text.sprintf(fmtX, pos.x());
		break;
	default:
		text.sprintf(fmtXY, pos.x(), pos.y());
	}
	return QwtText(text);
}


protected:
double xOffset;
unsigned xDigits, yDigits;
};



class data_plot : public QWidget
{
Q_OBJECT

public:
data_plot(QWidget *parent, bool plot_average, int line_width,
          const std::string& title,
          const std::string& xlabel,
          const std::string& ylabel,
          bool bHasLegend);

~data_plot();


virtual void showEvent(QShowEvent * event );
virtual void closeEvent(QCloseEvent * event );

typedef std::vector<double> xy_t;

void addCurve(const char* label, bool bPlotAverage = true,
              int iLineWidth = -1, Qt::PenStyle ps = Qt::SolidLine, bool bNeedsUpdate = true);
void addCurveXY(const char* label, const std::vector<double>& vx, const std::vector<double>& vy);

void clearData();
void addX(double x);
void addY(double y, const char* curveName);
void addXY(double x, double* y, unsigned nY);

void replot(bool bForce = false, const std::string& type = "");

void setMarkerText(const std::string& s, int font_size = 14);
void set_xrange(double x0, double x1);
void set_xoffset(double x0);
void set_xlabel(const std::string&);
void set_ylabel(const std::string&);

void set_save_file_name(const std::string&);
const std::string&  get_save_file_name()
{
	return save_file_name;
}

void savePDF();
void saveSVG();
void set_start_time(double t);

bool canPrint();
void print();
void clear();
const std::string& getTitle()
{
	return title;
}

public slots:
void slot_replot();

signals:
void signal_replot();
void sig_addX(double);
void sig_addY(double, unsigned int);

public slots:
void slot_addX(double);
void slot_addY(double, unsigned int);

protected:
void updateCurves();
void setXYdata(const xy_t& x, const xy_t& y, size_t iCurve);
QColor chooseCurveColor(const char* label);

private:
std::string title;

unsigned iUpdate;
QwtPlot myPlot;
QwtLegend* pLegend;
std::vector<QwtPlotCurve*> curves;
std::vector<QwtPlotCurve*> averaged_curves;
std::vector<std::string> curveNames;


xy_t X;
std::vector< xy_t > Ys;


PrecisePlotPicker* d_picker;
QwtPlotZoomer* zoomer;
QwtPlotGrid* grid;
QwtPlotMarker* marker;

int nPendingReplots;
std::string next_replot_type;

class map_y
{
public:
map_y() : total(0), n(0)
{
}

double average() const
{
	return total / n;
}
void add(double d)
{
	total += d; n++;
}

double total;
unsigned n;
};

typedef std::map<double, map_y> map_xy_t;
std::vector<map_xy_t> y_averaged;

bool plot_average;
int line_width;

double xMin, xMax, yMin, yMax, x_offset;

std::string save_file_name;
std::string new_xlabel, new_ylabel;

unsigned nReplots;
time_t tStart;
bool bNewCurves;

std::vector<bool> curvesThatNeedUpdate;      // which curves get updated in updateCurves();

QReadWriteLock plot_lock;                    //protect curves,
QReadWriteLock data_lock;                    //protext X, Ys, and y_averaged

QHBoxLayout layout;
QFont fnt;

std::string markerText;
int markerFS;

bool bNewYscale;
std::vector<QColor> defaultColors, availableColors;
};

class plot_window : public QMainWindow
{
public:
plot_window(QWidget *parent, QWidget* pPlot,
            const std::string& window_title,
            const std::string& settings_name = "");

virtual ~plot_window();

virtual void showEvent(QShowEvent * event );
virtual void closeEvent(QCloseEvent * event );

QWidget* pPlot;
std::string window_title, settings_name;
};

#ifndef NO_COLOR_PREF
void global_getTraceColor(const char* label, QColor* clr);
#endif

#endif // DATA_PLOT_H
