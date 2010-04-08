#pragma once

#include "FPGA_GUI.h"
#include "ScanSource.h"

#include <QTableWidgetItem>

class DACPage : public FPGA_GUI
{
public:
DACPage(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);
virtual ~DACPage();

static bool matchTitle(const std::string& s)
{
	return s.find("AD5") != string::npos;
}

//! Don't initialize FPGA params at startup.  Otherwise voltages get set to 0.
virtual bool needInitFPGA()
{
	return false;
}
};

class Voltages2 : public FPGA_GUI
{
Q_OBJECT

public:
Voltages2(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);
virtual ~Voltages2();

virtual void PostCreateGUI();

int ReorderPeriodic(bool bForce = false);

double getE(unsigned i);
void setE(unsigned i, double E);

//void AddAvailableActions(std::vector<std::string>*);
//void on_action(const std::string&);

static bool matchTitle(const std::string& s)
{
	return s.find("Voltages") != string::npos;
}

virtual bool RecalculateParameters();
public slots:
void colSelection( int );

protected:

bool calcXtallizeSettings();
unsigned Erow(unsigned i);

GUI_matrix* pEMatrix;
GUI_unsigned* pXtallizeSettings;

GUI_double ReorderPeriod;
GUI_combo ReorderDir;
GUI_bool FlipOrder;

double tLastReorder;
int flip_sign;
unsigned cMgAl, cAlMg;
};


//! Scan a voltage through Voltage page
class VScanSource : public ScanSource
{
public:
VScanSource(int iV, Voltages2* v_page, const std::string& name = "") : ScanSource(0), iV(iV), v_page(v_page), name(name)
{
	if (this->name.length() == 0)
		this->name = GetName(iV);
}

void SetScanOutput(double d)
{
	ScanSource::SetScanOutput(d); v_page->setE(iV, d);
}
double GetOutput()
{
	return v_page->getE(iV);
}
virtual std::string getName()
{
	return name;
}
virtual std::string getType()
{
	return "Voltage";
}
virtual unsigned GetMode()
{
	return 1;
}

std::string GetName(unsigned i)
{
	switch ( i)
	{
	case 0: return "Ex";
	case 1: return "Ey";
	case 2: return "<EC>";
	case 3: return "EC1/EC2";
	case 4: return "Eh";
	case 5: return "Ev";
	default: return "";
	}
}

std::string GetUnit()
{
	switch (iV)
	{
	case 0: return "V/cm";
	case 1: return "V/cm";
	case 2: return "V";
	case 3: return "";
	case 4: return "V/cm";
	case 5: return "V/cm";
	default: return "V";
	}
}

double GetMin()
{
	return -10;
};
double GetMax()
{
	return 10;
};

virtual bool isCompatibleScanType(const std::string& s)
{
	return (s == "LockIn") || (s == getType());
}

virtual void useFit(const std::string& scan_type, double x)
{
	if (isCompatibleScanType(scan_type))
		v_page->setE(iV, x);
}

protected:

int iV;
Voltages2* v_page;
std::string name;
};

extern Voltages2* gVoltages;
