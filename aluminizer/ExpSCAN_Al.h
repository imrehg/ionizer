#ifndef EXPSCAN_AL_H
#define EXPSCAN_AL_H

#include "ExpSCAN_NewFPGA.h"

class ExpSCAN_Al : public ExpSCAN_Detect
{
public:
ExpSCAN_Al(const string& sPageName,
           ExperimentsSheet* pSheet,
           unsigned exp_id);

virtual ~ExpSCAN_Al()
{
}

static bool matchTitle(const std::string& s)
{
	return (s.find("3P1 scan") != string::npos) || (s.find("3P1 cal") != string::npos) || (s.find("3P1 MM") != string::npos) ||
	       ( (s.find("3P1") != string::npos) && (s.find("lock") != string::npos) );
}

virtual void InitializeScan();
virtual double calcGroundState();
virtual void setupRemoteParam(GUI_double*);

virtual TransitionPage* GetCurrentTransition();
virtual std::string getTransitionBaseName()
{
	return "Al3P1";
}

//! setup apparatus for this experiment
virtual void InitExperimentSwitch();

//! put voltages etc. back to nominal values so other experiments work
virtual void DefaultExperimentState();

GUI_ttl* getRamseyPulse();

protected:
GUI_double shiftEh, shiftEv;
double oldEh, oldEv;
};

class ExpAl3P0 : public ExpSCAN_Al
{
public:
ExpAl3P0(const string& sPageName,
         ExperimentsSheet* pSheet,
         unsigned exp_id);

virtual ~ExpAl3P0()
{
}
virtual unsigned FitYColumn();

static bool matchTitle(const std::string& s)
{
	return (s.find("3P0 scan") != string::npos) || (s.find("3P0 cal") != string::npos);
}
virtual TransitionPage* GetCurrentTransition();
virtual std::string getTransitionBaseName()
{
	return "Al3P0";
}

virtual void AddAvailableActions(std::vector<std::string>*);
virtual void on_action(const std::string& s);

virtual void PreAcquireDataPoint(Scan_Base* sb, DataFeed& df);
virtual void PostAcquireDataPoint(Scan_Base* sb, DataFeed& df);
virtual void InitializeScan();

virtual void AddDataChannels(DataFeed&);

protected:
ConstantChannel* pRefCavityFreqChannel;
ConstantChannel* pRefCavityDriftChannel;
DataChannel* p3P1corrChannel;

RefCavityPage* p3P0CavityPage;
RefCavityPage* p3P1CavityPage;

bool bStop1S0;
};


class ExpAl3P0_lock : public ExpAl3P0
{
public:
ExpAl3P0_lock(const string& sPageName,
              ExperimentsSheet* pSheet,
              unsigned exp_id);

virtual ~ExpAl3P0_lock()
{
}

virtual void InitializeScan();
virtual void PostAcquireDataPoint(Scan_Base* sb, DataFeed& df);

static bool matchTitle(const std::string& s)
{
	return s.find("3P0 lock") != string::npos;
}

virtual bool UpdateMeasurements(measurements_t& m);

//setup apparatus for this experiment
virtual void InitExperimentSwitch();

//LockInParams overrides
//modulation occurs automatically on FPGA, so don't do anything here
virtual void   modulateOutput(double)
{
}

//	virtual double GetOutput();

virtual void   ShiftCenter(double d);
virtual double GetCenter();

virtual bool RecalculateModulation();

virtual GUI_dds* getLockInPulse();

virtual bool getErrorSignal(double* err, double* dX);

virtual double GetSignal();

//	virtual void UpdateScanPulses(double x);

virtual void PostCreateGUI();
void AddLinePlots();

//  double getOffsetFreq(unsigned );

virtual bool needInitFPGA()
{
	return true;
}                                               //initialize FPGA params at startup

protected:
GUI_dds* pClockPulse;
GUI_double TargetFreq;
GUI_double DutyCycle;
GUI_bool CorrectCavity;
GUI_double integralGain;

int iUpdate;

std::vector<DataChannel*> pNumProbes, pXitionProb, pProbeFreq0, pProbeFreq1;

DataChannel* pFPGAerr;
DataChannel* pDirection;
DataChannel* pClockXition;
DataChannel* pNumProbeLeft;
DataChannel* pNumProbeRight;
DataChannel* pNumXitionLeft;
DataChannel* pNumXitionRight;

double tStart;

int iLeftChannel, iRightChannel;
double nLeftXition, nRightXition;    //total number of transitions made on left and right gain points
double nProbesTotal, integralError;

std::vector<data_plot*> line_shape_plots;

int warningMode;    //0 if normal, 1 if no transitions seen recently
QSound xition_ok_sound;
};

#endif // EXPSCAN_AL_H
