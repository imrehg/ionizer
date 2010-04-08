#pragma once

#include "ScanSource.h"

/*! We have scan_variable and ScanSource.  Every scan_variable that's created is a
    "source_scan_variable". What's the disitinction?  The difference is in the life-cycle.
    ScanSources are created when the program starts and deleted when it ends.
    The GUI code keeps a list of all ScanSources and allows the user to choose among them.
    scan_variables are created only when a scan begins.  They point to a ScanSource.
    When the the scan is over, the scan_variable is destroyed.
    (Does this make any sense???
     We could get rid of scan_variables and have only ScanSources.)
 */


class scan_variable
{
public:
scan_variable()
{
}

void set_scan_position(double d)
{
	scan_pos = d; set_scan_positionI(d);
}
virtual void set_default_state(double d)
{
	default_state = d;
}

double get_scan_position()
{
	return scan_pos;
}
double get_default_state()
{
	return default_state;
}

void goto_default_state()
{
	set_scan_position(default_state);
}

protected:
virtual void set_scan_positionI(double) = 0;

double scan_pos;
double default_state;
};

class ScanSource;

class source_scan_variable : public scan_variable
{
public:
source_scan_variable(ScanSource* pSource) :
	pSource(pSource)
{
	default_state = pSource->GetOutput();
}

//! when servos are running, this function gets called to make a correction
virtual void set_default_state(double d);

protected:

virtual void set_scan_positionI(double d);

ScanSource* pSource;
};

