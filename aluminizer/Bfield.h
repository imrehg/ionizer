#pragma once

using namespace std;

#include "ExperimentPage.h"
#include "AnalogOutput.h"
#include "AnalogOutParameter.h"
#include "ScanSource.h"

#include <tnt.h>

class Bfield : public ParamsPage
{
public:
Bfield(const string& sPageName, ExperimentsSheet* pSheet);
virtual ~Bfield();

void SetB(unsigned i, double B);
double GetB(unsigned i);

void SetB0(unsigned i, double B);
double GetB0(unsigned i);

protected:
void Update();
virtual bool RecalculateParameters();
virtual unsigned num_columns()
{
	return 6;
};

protected:
vector<GUI_double*> B0gui;
vector<GUI_double*> Bgui;
vector<AnalogOutParameter*> Igui;
vector<GUI_double*> B2Igui;

//multiply B2I by B=(Bx, By, Bz) to get I=(Ix, Iy, Iz)
TNT::Array2D<double> B2I;
TNT::Array2D<double> B;
TNT::Array2D<double> I;
};

class BScanSource : public ScanSource
{
public:
BScanSource(int iB, Bfield* B_page, const std::string& name = "") : ScanSource(0), iB(iB), B_page(B_page), name(name)
{
	if (this->name.length() == 0)
		this->name = GetName(iB);
}

void SetScanOutput(double d)
{
	ScanSource::SetScanOutput(d); B_page->SetB(iB, d);
}
double GetOutput()
{
	return B_page->GetB(iB);
}
virtual std::string getName()
{
	return name;
}
virtual std::string getType()
{
	return "B-field";
}

std::string GetName(unsigned i)
{
	switch ( i)
	{
	case 0: return "Bx";
	case 1: return "By";
	case 2: return "Bz";
	default: return "";
	}
}

std::string GetUnit()
{
	return "mG";
}

double GetMin()
{
	return -1000;
};
double GetMax()
{
	return 1000;
};

virtual bool isCompatibleScanType(const std::string& s)
{
	return (s == "LockIn") || (s == getType());
}

virtual void useFit(const std::string& scan_type, double x)
{
	if (isCompatibleScanType(scan_type))
		B_page->SetB(iB, x);
}

protected:

int iB;
Bfield* B_page;
std::string name;
};

extern Bfield* gBfield;
