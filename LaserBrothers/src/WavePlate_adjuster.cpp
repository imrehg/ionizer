#include "WavePlate_adjuster.h"


#include <iostream>
#include <sstream>
#include <cstdio>
#include <stdexcept>

#include <analog_io.h>

using namespace std;

#ifdef WIN32
#define  snprintf  _snprintf
#endif

WavePlate_adjuster::WavePlate_adjuster(QWidget* parent, const std::string& port, unsigned baud) :
	QObject(parent),
	RS232device(port, baud),
	opt_actuator(2),
	wp0(parent), wp1(parent),
	nm_opt(2, 1000, 2000, this)	
{
	for(unsigned i=0; i<2; i++)
	{
		lastMovementS[i] = 0;
		lastMovementT[i].start();
	}

	wp[0] = &wp0;
	wp[1] = &wp1;

	for(unsigned i=0; i<2; i++)
	{
		current_angles[i] = wp[i]->value();
		wp[i]->setSingleStep(1000);
		wp[i]->setRange(-1000000, 1000000);

	}

	QObject::connect(wp[0], SIGNAL(valueChanged(int)),
						 this, SLOT(slot_updateAngle0(int)));

	QObject::connect(wp[1], SIGNAL(valueChanged(int)),
						 this, SLOT(slot_updateAngle1(int)));

	sendCmd("RS\r"); //switch to remote (computer) control
	SleepHelper::msleep(50);
	sendCmd("MR\r"); //switch to remote (computer) control
	SleepHelper::msleep(50);
	sendCmd("1SU+50\r");
	SleepHelper::msleep(50);
	sendCmd("1SU-50\r");
	SleepHelper::msleep(50);
	sendCmd("2SU+50\r");
	SleepHelper::msleep(50);
	sendCmd("2SU-50\r");
}

WavePlate_adjuster::~WavePlate_adjuster()
{
	sendCmd("ML\r"); //switch to local control
}

//figure out how to adjust waveplates based on lock-in type data
void WavePlate_adjuster::newData(double signal, bool bFault)
{
	if(! nm_opt.isRunning())
		nm_opt.start();

	if(! isMoving() )
		nm_opt.measured_f(signal, bFault);
}

bool WavePlate_adjuster::isMoving()
{
	return ( (lastMovementT[0].elapsed() < 2*lastMovementS[0]) || 
		     (lastMovementT[1].elapsed() < 2*lastMovementS[1]) );
}

void WavePlate_adjuster::shift(const std::vector<double>& dx)
{
	shiftAngle(0, dx[0], true);
	shiftAngle(1, dx[1], true);
}


int WavePlate_adjuster::getAngle(unsigned iChannel)
{
	return current_angles[iChannel];
}

void WavePlate_adjuster::slot_updateAngle0(int a)
{
	shiftAngle(0, a - getAngle(0), false);
}

void WavePlate_adjuster::slot_updateAngle1(int a)
{
	shiftAngle(1, a - getAngle(1), false);
}

void WavePlate_adjuster::setAngle(unsigned iChannel, int a)
{
	shiftAngle(iChannel, a - getAngle(iChannel), true);
}

int WavePlate_adjuster::shiftAngle(unsigned iChannel, int delta, bool bUpdateGUI)
{
	if(delta == 0)
		return getAngle(iChannel);

	char buff[64];

	lastMovementT[iChannel].restart();
	lastMovementS[iChannel] = abs(delta);

	if(delta > 0)
		snprintf(buff, 64, "%dPR+%d\r", iChannel+1, (int)(delta));
	else
		snprintf(buff, 64, "%dPR%d\r", iChannel+1, (int)(delta));

	bool bDebugAgilis = true;

	try
	{
		bool bRetry = true;

		while(bRetry)
		{
			sendCmd(buff);
			SleepHelper::msleep(5);

			if(bDebugAgilis)
				cout << "[Agilis] <<< " << buff << endl;;

			string r = sendCmd_getResponse("TE\r"); //check error code

			if(bDebugAgilis)
				cout << "[Agilis] >>> " << r;

			if(r.find("TE0") != string::npos)
				bRetry = false;
			else
				SleepHelper::msleep(25);
		}
	}
	catch(runtime_error e)
	{
		cerr << "[AgilisMotorsPage::sendCmd] RUNTIME ERROR: " << e.what() << endl;
	}

	current_angles[iChannel] += delta;

	if(bUpdateGUI)
	{
		wp[iChannel]->setValue(current_angles[iChannel]);
	}

	return current_angles[iChannel];
}
