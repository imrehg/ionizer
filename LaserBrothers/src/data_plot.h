#ifndef _DATA_PLOT_H
#define _DATA_PLOT_H


#include <qwt_counter.h>

#include <qmainwindow.h>
#include <QToolBar>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>

#ifdef WIN32
#include <NIDAQmx.h>
#endif

#include <vector>
#include <valarray>

#include "LockerWidget.h"
#include "NP_adjuster.h"

#include <analog_io.h>

#include "InputParameters.h"
#include "adio.h"

class DataPlot;
class LockerWidget;
class WavePlate_adjuster;


class MainWindow : public QWidget, public adio
{
	Q_OBJECT

public:
    MainWindow(InputParameters* params);
	virtual ~MainWindow();

	void DAQmxErrChk(int e) ; //throws runtime_error if DAQmx fails

protected:

	void setTimerInterval(double ms);
   virtual void timerEvent(QTimerEvent *e);
	void updateOutputs(bool bRecalculate);

protected slots:
	void run();
	void stop();
	void stopDisable();

protected:

	std::vector<LockerWidget*> lockers;
	
	char        errBuff[2048];

	int d_interval; // timer in ms
    int d_timerId;

	unsigned t;

	std::ofstream status_file;

	QTime lastReWrite;
	QSettings settings;
	QToolBar tb;

	param_int numLockers;
};


#endif
