#pragma once

using namespace std;

#include "FPGA_GUI.h"

#include "NI_AnalogOut.h"
#include "AnalogOutput.h"
#include "AnalogOutParameter.h"
#include "ScanSource.h"

#include <tnt.h>


class VoltagesBase : public FPGA_GUI
{
public:
VoltagesBase(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);
virtual ~VoltagesBase()
{
}

void SetNamePrefix(const std::string& name);

virtual void RampSettings(const std::string& name0, const std::string& name1, unsigned num_steps, unsigned rampBack = 0) = 0;

virtual void SetE(int i, double) = 0;
virtual double GetE(int i) = 0;

vector<ParameterGUI_Base*> prefixed_params;
};

class Voltages : public VoltagesBase
{
Q_OBJECT

public:
Voltages(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);
virtual ~Voltages();

//  static bool matchTitle(const std::string& s) { return s.find("Voltages") != string::npos; }

//x: i=0
//y: i=1
//z: i=2
//TwistX: i=3
//TwistY: i=4
virtual void SetE(int i, double);
virtual double GetE(int i);

//! ramp voltages down (-1) or up (+1)
void Ramp(int direction);

//! re-order
void Reorder(const std::string& voltage_setting = "");

//! re-order if ReorderPeriod time has elapsed since last re-order
int ReorderPeriodic(bool bForce = false);

protected:
virtual void PostCreateGUI();

void UpdateAll(unsigned nSteps = 0, unsigned rampBack = 0);

//	void SetNamePrefix(const std::string& name);

void RampSettings(const std::string& name0, const std::string& name1, unsigned num_steps, unsigned rampBack = 0);
void RampTo(const std::vector<double>& V, unsigned nSteps, unsigned rampBack = 0);

virtual void AddAvailableActions(std::vector<std::string>* p);

virtual void on_action(const std::string& s);

void Crystallize(int dir1, int dir2);
void Dump();

void SetECratio(double);
double GetECratio();

void SetECmean(double);
double GetECmean();

void SetEndcaps(double ECratio_new, double ECmean_new, double ECtwist_new, double* ECratio_actual, double* ECmean_actual);
void SetExy(unsigned mode, double Ex, double Ey, double* Ex_actual, double* Ey_actual, double cmCE_new, double cmRF_new);

void SetEx(double);
void SetEy(double);
void SetEh(double);
void SetEv(double);

double GetEx();
double GetEy();
double GetEh();
double GetEv();

virtual bool RecalculateParameters();

void UpdateInverseMatrices();

void UpdateEhv();
void UpdateExy();

protected:
matrix_t VEmatrix();

double VExy(unsigned i, unsigned j);
double VEhv(unsigned i, unsigned j);

double ExyV(unsigned i, unsigned j);
double EhvV(unsigned i, unsigned j);

double twist_multiplier;
//	TriggererdNIAnalogOut tNI;

GUI_double ECratio;
GUI_double ECmean;
GUI_double ECtwist;

//GUI_bool		ForceIdentity;
GUI_bool NoUpdates;
GUI_combo SettingsName;
GUI_combo ControlMode;

GUI_unsigned RampSteps;
GUI_unsigned DwellTime;
GUI_bool CrystallizeReorder, FlipOrder;
GUI_unsigned ReorderSteps;
GUI_unsigned ReorderWait;
GUI_double ReorderPeriod;
GUI_combo ReorderDir;


GUI_matrix VcerfExy;
GUI_matrix VcerfEhv;

GUI_matrix Exy, Ehv;

GUI_double cmCE, cmRF;
GUI_double guiV6, guiV7;

//	GUI_double		V6low;

vector<AnalogOutParameter*> voltage_parameters;

AnalogOutParameter *V6, *endcaps1, *endcaps2a, *endcaps2b;

AnalogOutParameter* ce[2];
AnalogOutParameter* rf[2];

public:
//multiply EV by V=(Vce, Vdb) to get E=(Ex, Ey)
TNT::Array2D<double> M_ExyV, M_EhvV;

std::ofstream voltage_log;
std::string oldSettingsName;

signals:
void trigger_action(QString s);

public slots:
void slot_on_action(QString s);

protected:
double tLastReorder;

QSound sndCrystallize, sndDump;
int flip_sign;
};

//! Scan a voltage through Voltage page
class VScanSource : public ScanSource
{
public:
VScanSource(int iV, VoltagesBase* v_page, const std::string& name = "") : ScanSource(0), iV(iV), v_page(v_page), name(name)
{
	if (this->name.length() == 0)
		this->name = GetName(iV);
}

void SetScanOutput(double d)
{
	ScanSource::SetScanOutput(d); v_page->SetE(iV, d);
}
double GetOutput()
{
	return v_page->GetE(iV);
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
		v_page->SetE(iV, x);
}

protected:

int iV;
VoltagesBase* v_page;
std::string name;
};

extern Voltages* gVoltages;
