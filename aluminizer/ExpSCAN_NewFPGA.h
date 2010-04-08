#pragma once

#include "AluminizerApp.h"
#include "ExpSCAN.h"
#include "ScanObject.h"
#include "ExpLockIn.h"

#include <QSharedMemory>

class RefCavityPage;

class ExpSCAN_NewFPGA : public ExpSCAN_L
{
Q_OBJECT

public:
ExpSCAN_NewFPGA(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id);

virtual ~ExpSCAN_NewFPGA()
{
}

virtual void PostCreateGUI();

virtual double GetInterrogationTime();
virtual TransitionPage* GetCurrentTransition();
virtual TransitionPage* GetCoolingTransition();

virtual void InitializeScan();

virtual void UpdateScanPulses(double x);

virtual double GetCenterFrequency();
unsigned getNumExp();

virtual void useFit(double x);

virtual void OnStop();

virtual bool UpdateFittedPiTime(double tFit, double t0, bool bGoodFit);
virtual void AddDataChannels(DataFeed&);
virtual bool RecalculateParameters();

virtual std::string getTransitionBaseName()
{
	return "";
}

//! setup apparatus for this experiment
virtual void InitExperimentSwitch();

virtual void setNumExp(unsigned n);

public slots:
void slot_setup_scan(scan_target);

protected:
bool relinkPulses();
DataChannel* findFPGAchannel(const std::string& key);

scan_target current_scan_target;

//	GUI_string       scanName;

GUI_unsigned*    rpNumExp;
GUI_bool*      rpDebugFirstExp;

GUI_bool linked_pulse;
std::string exp_pulse_name;

public:

GUI_bool DebugFirstExp;
GUI_bool DisableReorder;

protected:
ConstantChannel* pReorderChannel;

public:
std::vector<DataChannel*> pFPGAchannels;
std::vector<double> channelData;
};


class ExpSCAN_Detect : public ExpSCAN_NewFPGA
{
public:
ExpSCAN_Detect(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id);

virtual ~ExpSCAN_Detect()
{
}
virtual double NumFlops()
{
	return 4;
}
virtual std::string getTransitionBaseName()
{
	return "Mg";
}

void PostAcquireDataPoint(Scan_Base* sb, DataFeed& df);
virtual void AddDataChannels(DataFeed&);

static bool matchTitle(const std::string& s)
{
	return s.find("Detect") != string::npos;
}

QSharedMemory xy_memory_shared;
ConstantChannel* dcX;
ConstantChannel* dcY;
};

class RunOvens;

class ExpLoad : public ExpSCAN_Detect
{
public:
ExpLoad(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id);

virtual ~ExpLoad()
{
}

virtual void InitializeScan();
virtual void DefaultExperimentState();

virtual void PostAcquireDataPoint(Scan_Base*, DataFeed&);

static bool matchTitle(const std::string& s)
{
	return s.find("Load") != string::npos;
}

RunOvens* pOvens;

GUI_double MaxTime, MaxCounts, MinCounts;
GUI_bool LaunchHeatingExp;
QSound loaded_sound, times_up_sound;
bool bMg, bAl;
};

class ExpQubit : public ExpSCAN_Detect
{
public:
ExpQubit(const string& sPageName,
         ExperimentsSheet* pSheet,
         unsigned exp_id);

virtual ~ExpQubit()
{
}

virtual void DoCalibration(calibration_item* ci, RunObject* pOwner = 0);
virtual void setupRemoteParam(GUI_double*);

static bool matchTitle(const std::string& s)
{
	return (s.find("Qubit") != string::npos) || (s.find("Order check") != string::npos);
}

};

class ExpRF_lock : public ExpSCAN_Detect
{
public:
ExpRF_lock(const string& sPageName,
           ExperimentsSheet* pSheet,
           unsigned page_id) :
	ExpSCAN_Detect(sPageName, pSheet, page_id),
	LockInPulseName("Lock-in pulse", "", &m_TxtParams, "", &m_vParameters),
	RamseyMaxT("Max. Ramsey time [us]", &m_TxtParams, "1000", &m_vParameters)
{
}

virtual ~ExpRF_lock()
{
}

virtual void DefaultExperimentState();

virtual void AddParams()
{
	ExpSCAN_Detect::AddParams();

	for (size_t i = 0; i < m_vParameters.size(); i++)
	{
		GUI_dds* p = dynamic_cast<GUI_dds*>(m_vParameters[i]);

		if (p != 0)
		{
			if (p->GetName().find("RF") != std::string::npos)
			{
				LockInPulseName.AddChoice(p->GetName());
				LockInPulseName.SetValue(p->GetName());
			}
		}
	}
}

virtual void InitializeScan()
{
	pLockInPulse = dynamic_cast<GUI_dds*>(FindParameter(LockInPulseName.Value()));
	pRamseyPulse = dynamic_cast<GUI_ttl*>(FindParameter("Ramsey"));

	if (!pLockInPulse)
		throw runtime_error("unknown lock-in pulse");

	ExpSCAN_Detect::InitializeScan();
}

static bool matchTitle(const std::string& s)
{
	return s.find("RF") != string::npos;
}

//LockInParams overrides
virtual void modulateOutput(double d)    //in Hz
{ double fNew = (GetCenter() + d);

  cout << "f = " << fNew << "        modulation = " << d << endl;

  getLockInPulse()->setFreq(fNew * 1e-6); }

//in Hz
virtual double GetOutput()
{
	return getLockInPulse()->getFreq() * 1e6;
}
virtual bool RecalculateModulation();
virtual GUI_dds* getLockInPulse()
{
	return pLockInPulse;
}
virtual void UpdateScanPulses(double x);

protected:
GUI_combo LockInPulseName;
GUI_double RamseyMaxT;
GUI_dds* pLockInPulse;
GUI_ttl* pRamseyPulse;
};
