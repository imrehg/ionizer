#pragma once

#include "ExperimentPage.h"
#include "Experiment.h"
#include "Histogram.h"
#include "Fitting.h"
#include "ScanObject.h"
#include "fractions.h"
#include "physics.h"

class GlobalsPage;
class Voltages;
class ScanSource;
class CoolingTransitionPage;
class plot_window;
class data_plot;
class calibration_item;

class ExpSCAN : public ExperimentPage
{
Q_OBJECT

public:
ExpSCAN(const string& sPageName, ExperimentsSheet* pSheet, const char* TargetStates, unsigned page_id);
virtual ~ExpSCAN();

virtual void AddAvailableActions(std::vector<std::string>*);
virtual void on_action(const std::string& s);

virtual void AddParams();
virtual physics::line CurrentLine();

virtual unsigned getPriority()
{
	return Priority.Value();
}
virtual void DoCalibration(calibration_item* ci, RunObject* pOwner = 0);
virtual void SetCurrentScan(const calibration_item* ci);
virtual DDS_Pulse_Widget* DriveLine(const physics::line&);

friend class ScanScheduler;
friend class Scan_Base;
friend class Scan_Fittable;
//	friend class scans::FrequencyScan;

bool IsLinkedScan();
bool IsScatterScan();
bool IsRamseyFScan();
bool IsRamseyPScan();

virtual void useFit(double)
{
}
virtual unsigned getNumExp()
{
	return 0;
}

virtual bool UpdateFittedCenter(double fFit, bool bGoodFit);
virtual bool UpdateFittedPiTime(double tFit, double t0, bool bGoodFit);

bool ShowHistogram()
{
	return Histogram;
}

virtual bool WantsFit()
{
	return true;
}

void SetPause(bool);

virtual bool WantsPrepare()
{
	return false;
}

virtual void UpdateFrequency(double f);
virtual void UpdateTime(double t);
virtual void UpdatePhase(double p);

//return time for current probe pulse in us
virtual double GetInterrogationTime()
{
	return 1;
}

virtual bool log_shots()
{
	return false;
}

double GetRabiRate();
double GetXoffset();

virtual double GetCenterFrequency()
{
	return 0;
}

double GetExpRunTime() const
{
	return exp_run_time;
}

//! called once when the experiment is fist started
virtual void InitializeScan()
{
}

//! data column in results array containing the y-data for the fit
virtual unsigned FitYColumn()
{
	return 1;
}

signals:
void signal_setup_plots();

protected slots:
void slot_setup_plots();

protected:
virtual void setNumExp(unsigned)
{
}

void addScanSource(GUI_dds* pPulseDDS);

//GUI functions
void showScanGUI(bool bShow);
bool updateScanGUI(bool bForceUpdate = false);
bool updateScanSourcesGUI(bool bForceUpdate = false);

void setScanUnit(const std::string& s);
void setScanMode(unsigned u);    //mode 1: span, center   mode 2: start stop

bool updateScanSpan();
bool updateScanStartStop();
bool updateScanStep();

ScanSource* currentScanSource();
scan_variable* currentScanVariable();

virtual void PreAcquireDataPoint(Scan_Base*, DataFeed&)
{
}
virtual void PostAcquireDataPoint(Scan_Base*, DataFeed&)
{
}

virtual void InitExperimentStart();

//! setup apparatus for this experiment
virtual void InitExperimentSwitch();

//! put voltages etc. back to nominal values so other experiments work
virtual void DefaultExperimentState();

virtual run_status Run()
{
	return pScan->DataPoint();
}


virtual void FinishRun();

virtual Scan_Base* CreateScan();

virtual void AddDataChannels(DataFeed&) = 0;

virtual bool RecalculateParameters();

virtual double AOMdelay(const string&);

virtual double GetNewFrequencyAfterFit();
virtual double GetNewPiTimeAfterFit();

virtual bool CheckUnusedFits()
{
	return true;
}

//are we scanning over the transition, or something else, e.g. a DAC voltage
virtual bool ScanTransition()
{
	return true;
}

virtual double NumFlops()
{
	return 4;
}

virtual TransitionPage* GetCurrentTransition();
virtual TransitionPage* GetCoolingTransition() = 0;

//ground and excited state for current scan
virtual double getGroundState();
virtual double getExcitedState();
virtual int getSideband();
virtual int getPolarization();

virtual string GetDataFileName();

string SetupGnuplotCmd();

virtual std::string GetFCenterName()
{
	return "Center [MHz]";
}
virtual std::string GetITimeName()
{
	return "Interrogation Time [us]";
}

//find the page with the named transition and return a pointer to it
//returns 0 upon failure
TransitionPage* FindTransitionPage(const std::string& name);

data_plot* get_data_plot();
histogram_plot* get_hist_plot();

protected:
DataChannel* pSignalChannel;

TransitionPage* pX;

string ScanUnit;

bool unused_fit;

//TODO private:
Scan_Base* pScan;
numerics::FitObject* pFit;

public:
GUI_combo ScanType, ScanVariables;

protected:
GUI_unsigned Priority;
GUI_bool LinkedParams;


public:
GUI_bool AutomaticParams;
GUI_int NumScans;

protected:
GUI_int NumDataPoints;


public:
GUI_double RotationAngle;
GUI_double Span;
GUI_double FCenterFit;
GUI_double Start;
GUI_double Stop;
GUI_double Step;

GUI_double& Gain;
GUI_double& Modulation;
GUI_double& LockCenter;

GUI_bool GoodFit, IgnoreFit;


protected:
GUI_double FitMinContrast;

public:
GUI_double TFittedPiTime;

protected:
GUI_double FitMinAmplitude;
//	GUI_double		FitMaxSumSquares;



GUI_bool EnablePlot;
GUI_bool Histogram;
GUI_double ScanTime;

GUI_progress progress;
GUI_bool LogShots;

// ! Number of sequential measurements to perform before signaling IDLE
// If the scheduler time slice is too short, the sequence may be interrupted.
GUI_unsigned NumMeasurements;

public:
double initial_delay;

protected:
double scan_start;

string scan_name;
string oldScanType;

double exp_run_time;

plot_window* data_plotW;
plot_window* hist_plotW;

calibration_item* cci;                          //current calibration item

std::vector< ScanSource* > p_scan_sources;      //available scan sources for this page

unsigned scanGUImode;                           //0-none   1-center/span   2-start/stop
ScanSource* oldScanSource;
};

class ExpSCAN_L : public ExpSCAN, public LockInParams
{
public:
ExpSCAN_L(const string& sPageName, ExperimentsSheet* pSheet, const char* TargetStates, unsigned page_id);
virtual ~ExpSCAN_L();

virtual bool RecalculateModulation()
{
	return false;
}

/************** LockInParams overrides ***********/
virtual scan_variable* getLockVariable();
virtual void   modulateOutput(double d);
virtual void   SetOutput(double d);
virtual double GetOutput();
virtual double GetModulation();
virtual void   ShiftCenter(double d);
virtual void   SetCenter(double d);
virtual double GetCenter();

virtual double GetGain();
virtual double GetSignal();
virtual std::string GetLockVariableName();

virtual double GetInitialCenter();
virtual int GetNumMeasurements();
/**************/
};

