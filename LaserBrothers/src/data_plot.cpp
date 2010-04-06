#include <stdlib.h>
#include "data_plot.h"
#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <iomanip>



#include "WavePlate_adjuster.h"

using namespace std;

MainWindow::MainWindow(InputParameters* params) :
adio(params),
lockers(4),
d_interval(0),
d_timerId(-1),
t(0),
status_file("laser_status.txt", ios_base::trunc),
settings("NIST", "LaserBothers"),
tb(this),
numLockers("Number of lockers", params, "4")
{
	setupIO();

	lockers.resize(numLockers);

	for(unsigned i=0; i<lockers.size(); i++)
		lockers[i] = new LockerWidget(this, i, params, this, &settings);

	for(unsigned i=0; i<lockers.size(); i++)
	{
		int dep = lockers[i]->dependency();

		if(dep >= 0)
			lockers[i]->addDependency(lockers[dep]);
	}

	tb.addAction("RUN", this, SLOT(run()));
	tb.addAction("STOP", this, SLOT(stop()));
	tb.addAction("STOP [DISABLE INTEGRATORS]", this, SLOT(stopDisable()));
	
        QGridLayout *layout = new QGridLayout(this);

	layout->addWidget(&tb, 0, 0, 1, 2);

	for(size_t i=0; i<lockers.size(); i++)
		layout->addWidget(lockers[i], 1+i/2, i%2);

	updateOutputs(false);
}



MainWindow::~MainWindow()
{
	stop();

	for(unsigned i=0; i<lockers.size(); i++)
		delete lockers[i];
}

void MainWindow::run()
{
	for(unsigned i=0; i<aIn.size(); i++)
		aIn[i]->start();

	setTimerInterval(5.0);
}

void MainWindow::stop()
{
	if(d_timerId >= 0)
	{
		killTimer(d_timerId);
		d_timerId = -1;
	}

	for(unsigned i=0; i<aIn.size(); i++)
		aIn[i]->stop();

	for (unsigned j=0;j<lockers.size();j++)
		lockers[j]->stop();
}

void MainWindow::stopDisable()
{
	stop();
	updateOutputs(false);
}


void MainWindow::setTimerInterval(double ms)
{
    d_interval = qRound(ms);

    if ( d_timerId >= 0 )
    {
        killTimer(d_timerId);
        d_timerId = -1;
    }
    if (d_interval >= 0 )
        d_timerId = startTimer(d_interval);
}

	
void MainWindow::timerEvent(QTimerEvent *)
{
	if(d_timerId < 0)
		return;

	t++;

	try
	{
		for(unsigned i=0; i<aIn.size(); i++)
			aIn[i]->getData();

		for (unsigned j=0;j<lockers.size();j++)
		{
			//process data
			lockers[j]->processNewData();
		}

		updateOutputs(true);

		// update the display
		for(unsigned i=0; i<lockers.size(); i++)
		{
			lockers[i]->updatePlotData();

			if(t % 20 == 0)
				lockers[i]->replot();
		}

	}
	catch (runtime_error e) {}
}

void MainWindow::updateOutputs(bool bRecalculate)
{
	try
	{
		for(unsigned i=0; i<lockers.size(); i++)
		{
			if(bRecalculate)
				lockers[i]->calculateOutput();
		}

		//only update status file if status has changed or 1 s has elapsed
		bool bNewStatus = lastReWrite.elapsed() > 1000;

		for(unsigned i=0; i<dOut.size(); i++)
		{
			bool bChanged = dOut[i]->updateDigitalOutputs();
			bNewStatus |= bChanged;
		}

		for(unsigned i=0; i<aOut.size(); i++)
			aOut[i]->updateAnalogOutputs();
		
		if(bNewStatus)
		{
			lastReWrite.start();
			status_file.seekp(0);
			
			for(unsigned i=0; i<lockers.size(); i++)
			{
				char buff [1024];
            snprintf(buff, 1024, "%14s %8s (last unlock %8d seconds ago)",
					lockers[i]->getName().c_str(),
					lockers[i]->getStatusStr(),
					lockers[i]->tLastUnlock.elapsed()/1000);

				status_file << buff << endl;
			}

			string timestr(QTime::currentTime().toString().toAscii());
			status_file << "as of " << timestr << endl;
		}
	}
	catch (runtime_error e) {}
}
