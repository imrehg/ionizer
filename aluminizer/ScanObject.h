#pragma once

#include "Histogram.h"
#include "Fitting.h"
#include "ScanScheduler.h"
#include "ScanSource.h"

using namespace std;

class ExpSCAN_Al;
class ExpSCAN;
class RunObject;
class Voltages;
class scan_variable;
class TransitionPage;
class data_plot;
class histogram_plot;

//!Scan_Base maintains the state for a scan, saves all incoming data and plots it.

/*!
   Scan_Base represents a currently running experiment.
   This could be one or multiple "scans" of a time or frequency variable,
   a continuous chart-recorder type measurement, or a LockIn type servo.

   This links together the GUI page that launched the experiment, and the HW
   that actually runs the experiment.  Classes derived from Scan_Base take care of
   sequencing of experiment parameters, saving the data, and updating plots.

   Generally, the thing that gets scanned is a ScanSource object.  We have
   time, frequency, B-field, voltage, and maybe other scan sources.  ScanSource objects
   can take a single parameter, and adjust the experiment to reflect this new value.  They
   also remember the previous "non-scanning" output, so that when this scan releases the
   experiment, the state can be reset to normal.  ScanSource also contains unit information.

   LockInScan takes a ScanSource as input, and does a LockIn servo on this variable, using the
   experiment results and gain parameters to make feedback adjustments.
 */

class Scan_Base
{
public:
Scan_Base(ExpSCAN* scanPage, vector<scan_variable*> scan_variables);

virtual ~Scan_Base();

scan_variable* currentScanVariable();

virtual bool isScanning()
{
	return true;
}
virtual bool IsFinished() const = 0;

virtual double GetScanVariable() = 0;

virtual void InitializeScan();

virtual ExperimentBase::run_status  DataPoint();

virtual string GetScanUnit()
{
	return "";
}
virtual string GetXlabel()
{
	return "";
}
virtual double GetXoffset()
{
	return 0;
}
virtual double GetXscale()
{
	return 1;
}

virtual double GetXmin() = 0;
virtual double GetXmax() = 0;

virtual double GetYmin()
{
	return minCounts;
}
virtual double GetYmax()
{
	return maxCounts * 1.2;
}

virtual string GetPlotLabel()
{
	return "";
}

data_plot* getPlot()
{
	return plot;
}

virtual bool UseDots() const
{
	return false;
}

virtual bool CanFit();

bool HasValidData()
{
	return validData.size() != 0;
}

string GetDataFileName(const string& ext = ".csv") const
{
	return dataFileName + ext;
}
virtual void InitExperimentSwitch();

friend class ScanScheduler;

int GetTotalPoints() const
{
	return total_points;
}

void setRunStatus(ExperimentBase::run_status r)
{
	runStatus = r;
}

virtual size_t CurrentIndex() const
{
	return 0;
}

virtual void DefaultExperimentState();

void debugMsg(const std::string& s);

const std::string& debugFileName()
{
	return debug_log_fname;
}

protected:
virtual void StartScan() = 0;
virtual void NextDataPoint() = 0;

virtual void UseValidData();
virtual void SaveValidData();

//plotting functions
virtual void SetupPlot();
virtual void addPlotCurves();
virtual void PlotLatestData();
bool PlotPoints(int nPoints);

virtual void AcquireDataPoint();

virtual bool FastReplot()
{
	return false;
}

static Scan_Base* pCurrentScan;
protected:

ExpSCAN* ScanPage;
RunObject* pRunObject;
const string dataFileName;    //data file name w/o extension
ofstream dataFile;

std::string debug_log_fname;
std::ofstream debug_log_file;


vector<string> ChannelNames;

vector<scan_variable*> scan_variables;

public:

int total_points;          //total number of points taken for this scan
int num_current_points;    //points taken for this scan during current time slice

ofstream shots_log;


public:
double tStart;

protected:
string pulseFileName;

int ReplotModulo;

double minCounts;
double maxCounts;

int PlotPointsSaved;
int num_replots;

vector< vector<double> > validData;

DataFeed data_feed;

bool bUnprepared;
ExperimentBase::run_status runStatus;
bool recorded_names;


protected:
data_plot* plot;
histogram_plot* hist_plot;
bool bNeedSetupPlot;
};

//! specialization of a scans that can be fit to some function
class Scan_Fittable : public Scan_Base
{
public:
Scan_Fittable(ExpSCAN* scanPage, vector<scan_variable*> scan_variables);

virtual ~Scan_Fittable();

virtual ExperimentBase::run_status  DataPoint();

//	virtual bool CanFit() { return Scan_Base::CanFit(); }

//return a new FitObject
virtual numerics::FitObject* RunFit(ExpSCAN* pScanPage = 0);

virtual double GetXmin();
virtual double GetXmax();

protected:
virtual numerics::FitObject* CreateFitObject() = 0;

virtual bool IsFinished() const;
virtual double GetScanVariable();

void StartScan();
virtual void NextDataPoint();
virtual size_t CurrentIndex() const;

virtual bool UseDots() const;

double ScanCenter() const;
double ScanSpan() const;
double ScanMax() const;
double ScanMin() const;

int NumDataPoints() const;

vector<size_t> indeces;
vector<double> x;

std::vector<double> fit_results;

protected:
void AddPointsAt(double x, unsigned n);                     //add extra points at x
void AddPointsRange(double x1, double x2, unsigned n);      //add n points to span [x1, x2]

protected:
size_t current_index;
bool ScatterScan;

double FitMinContrast;
double FitMinAmplitude;

int scans_completed;
};

//! continuous scans are used for beam alignment and servo loops
class ContinuousScan : public Scan_Base
{
public:
ContinuousScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables);

virtual ~ContinuousScan();

virtual bool isScanning()
{
	return false;
}
virtual bool CanFit()
{
	return false;
}

virtual bool IsFinished() const
{
	return pRunObject->ShouldStop();
}
virtual double GetScanVariable();
virtual void StartScan();
virtual void NextDataPoint()
{
	num_points++;
}

virtual double GetXmin();
virtual double GetXmax();


protected:
virtual void PlotLatestData();

virtual bool FastReplot()
{
	return true;
}
virtual bool UsesTime()
{
	return false;
}

int num_points;
double tStart;
ofstream plotDataFile;
string plotDataFileName;
vector<string> temp_file_names;
};

class NPointScan : public ContinuousScan
{
public:
NPointScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables, int max_points);
virtual ~NPointScan()
{
}

virtual bool IsFinished() const;

int max_points;
};

class LockInScan;

//! scan pages should be derived from LockParams to interface properly with LockScans
class LockParams
{
public:
LockParams() : integrator(0)
{
}
virtual ~LockParams()
{
}

virtual void RespondToMeasurement()
{
	integrator += GetNormalizedErrorSignal();
	ShiftCenterNormalized( (GetGain() / 2) * GetNormalizedErrorSignal() +
	                       integrator * GetGainI() / 2);
}

virtual double GetNormalizedErrorSignal() = 0;

virtual void ShiftCenterNormalized(double) = 0;
virtual double GetCenter() = 0;
virtual double GetGain() = 0;
virtual double GetGainI()
{
	return 0;
}

virtual std::string GetLockVariableName() = 0;

virtual bool MakeCorrection()
{
	return true;
}

virtual int GetNumMeasurements() = 0;
protected:
double integrator;
};

//! scan pages should be derived from LockInParams to interface properly with LockInScans
class LockInParams : public LockParams
{
public:
LockInParams()
{
}
virtual ~LockInParams()
{
}

class measurement
{
public:

measurement() : x(0), counts(-1)
{
}

double x;
double counts;
};

typedef vector<measurement> measurements_t;

/*

 *output* is analogous to the output of a lock-in detector.
 *signal* is analogous to the input signal of a lock-in detector
 *center* is the current best estimate for the correct value of the parameter being locked.
   The following function must be over-ridden:

 */

//modulate the output by a certain amount: output = center+d
virtual void modulateOutput(double d) = 0;

//return current output
virtual double GetOutput() = 0;

//return current modulation
virtual double GetModulation() = 0;

//shift center by delta: center = center + delta
virtual void ShiftCenter(double delta) = 0;

//return input signal
virtual double GetSignal() = 0;

//	virtual void SetCenter(double) {};
virtual double GetNormalizedErrorSignal()
{
	return error_signal;
}

virtual void ShiftCenterNormalized(double s)
{
	ShiftCenter( s * deltaX);
}

virtual bool getErrorSignal(double*, double*)
{
	return false;
}
virtual bool UpdateMeasurements(measurements_t&)
{
	return false;
}

//by default just drive towards the peak using the demodulated signal & gain
virtual void RespondToMeasurements(LockInScan* pScan, const measurements_t& m, size_t i);
virtual void RespondToMeasurements(LockInScan* pScan, double error_signal, double deltaX);

virtual int GetNumMeasurements() = 0;

protected:
double error_signal;
double deltaX;
};

//a scan that locks to a dispersive signal
class LockScan : public ContinuousScan
{
public:
LockScan(ExpSCAN*, vector<scan_variable*> scan_variables, LockParams*, ScanSource*);
virtual ~LockScan()
{
}

virtual ExperimentBase::run_status  DataPoint();

protected:
virtual size_t GetNumToTake()
{
	return pLockParams->GetNumMeasurements();
}

virtual void UseValidData();

virtual string GetPlotLabel();

protected:
virtual bool UsesTime()
{
	return false;
}

ConstantChannel* pCenterChannel;
LockParams* pLockParams;
size_t num_new_points;
int num_skipped;
ScanSource* ss;
};

//a scan that locks to an extreme. it uses lock-in to create a dispersive signal which
//is used by LockScan to achieve the lock
class LockInScan : public LockScan
{
public:
LockInScan(ExpSCAN*, vector<scan_variable*> scan_variables, LockInParams*, ScanSource*);
virtual ~LockInScan()
{
}

void ClearMeasurements();

int LeftIndex() const
{
	return 0;
}
int RightIndex() const
{
	return 1;
}
int CenterIndex() const
{
	return 2;
}

protected:

virtual void AcquireDataPoint();

virtual void NextDataPoint();
virtual void DefaultExperimentState();       //prepare to pause this scan and restore experiment to normal state

virtual void UseValidData();
double ModulationState(int);

void UpdateLockSignalChannels();

protected:
virtual bool UsesTime()
{
	return false;
}

LockInParams* pLockInParams;

int iCurrentPoint;
size_t flipLeftRight;
unsigned nLeft, nRight, nCenter;

LockInParams::measurements_t measurements;

ConstantChannel* pcAdjustments;
ConstantChannel* pcLeftSignal;
ConstantChannel* pcCenterSignal;
ConstantChannel* pcRightSignal;
};

class SourceScan : public Scan_Fittable
{
public:
SourceScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables, ScanSource* ss);
virtual ~SourceScan()
{
}

protected:

virtual string GetScanUnit()
{
	return "[" + ss->GetUnit() + "]";
}
string GetXlabel();

virtual void DefaultExperimentState();

virtual numerics::FitObject* CreateFitObject();
virtual bool CanFit();

ScanSource* ss;
};

/* should maybe replace these functions with members of ScanBase */
bool IsSmartFrequencyScan(const string&);

bool IsFreqScan(const string&);

bool IsRamseyFScan(const string&);

bool IsRamseyPScan(const string&);

bool IsTimeScan(const string&);

bool IsPhaseScan(const string&);

bool IsContinuousScan(const string&);

bool IsNPointScan(const string&);

bool IsLockScan(const string&);

bool IsLockInScan(const string&);

bool IsEndcapsVScan(const string& s);

bool IsTwistVScan(const string& s);

bool IsSigmaPZTScan(const string& s);

