#ifndef AGILIS_MOTORS_H
#define AGILIS_MOTORS_H

#include "MotorsPage.h"

class AgilisMotorsPage : public ParamsPage
{
public:
AgilisMotorsPage(const string& sPageName, ExperimentsSheet* pSheet);
virtual ~AgilisMotorsPage();

virtual bool RecalculateParameters();
void AddAvailableActions(std::vector<std::string>* p);
void on_action(const std::string& s);

unsigned getNumMotors()
{
	return nMotors;
}
std::string getName(unsigned i)
{
	return Names.at(i)->Value();
}
void setScanOutput(unsigned i, double d)
{
	Positions.at(i)->SetValue( floor(d + 0.5) ); updatePositions();
}
double getScanOutput(unsigned i)
{
	return Positions.at(i)->Value();
}

protected:
void driveTo(unsigned iMotor, double pos);
virtual unsigned num_columns();
void updatePositions();

std::string sendCmd(const std::string& s);
void initController();


protected:
unsigned nMotors;

vector<GUI_string*>              Names;
vector<GUI_int_no_label*>         Positions, Increments, Decrements, Corrections;
vector<double>                InternalPos, corrections_needed;
GUI_string Response;
GUI_bool ImmediateUpdates, DebugAgilis, SingleStep, AutoCorrect;

RS232device rs232;
unsigned iCurrentMotor;
};


//! Scan Newport Agilis motors
class MScanSource : public ScanSource
{
public:
MScanSource(int iSS, AgilisMotorsPage* page) : ScanSource(0), iScanSource(iSS), page(page)
{
}

void SetScanOutput(double d)
{
	ScanSource::SetScanOutput(d); page->setScanOutput(iScanSource, d);
}
double GetOutput()
{
	return page->getScanOutput(iScanSource);
}
virtual std::string getName()
{
	return page->getName(iScanSource);
}
virtual std::string getType()
{
	return "Motor";
}
virtual unsigned GetMode()
{
	return 1;
}


std::string GetUnit()
{
	return "steps";
}

double GetMin()
{
	return -10000;
};
double GetMax()
{
	return 10000;
};

virtual bool isCompatibleScanType(const std::string& s)
{
	return (s == "LockIn") || (s == getType());
}

virtual void useFit(const std::string& scan_type, double x)
{
	if (isCompatibleScanType(scan_type))
		page->setScanOutput(iScanSource, x);
}

protected:

int iScanSource;
AgilisMotorsPage* page;
};

#endif //AGILIS_MOTORS_H
