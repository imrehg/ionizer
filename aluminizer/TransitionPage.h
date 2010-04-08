#pragma once

#include "Numerics.h"
#include "Fitting.h"
#include "fractions.h"
#include "physics.h"
#include "Transition.h"
#include "Experiment.h"
#include "ExperimentPage.h"
#include "GlobalsPage.h"
#include "HFS.h"
#include "CalibrationPage.h"

using namespace std;

class TransitionPage : public CalibrationPage
{
Q_OBJECT

public:
TransitionPage(const string& sPageName,
               ExperimentsSheet* pSheet,
               unsigned page_id,
               physics::HFS* g,
               physics::HFS* e,
               const std::string& xname,
               int NumModes,
               unsigned NumPorts = 1);

virtual ~TransitionPage();

virtual void AddParams();
virtual void AddAvailableActions(std::vector<std::string>*);
virtual void on_action(const std::string& s);

virtual string TransitionName() const = 0;

/*supply a vector of sidebands to be calibrated
   this interface needs some work.  ideally there should be a standard way to specify
   what is to be calibrated.  we could have a new class identifiying transition time and
   frequency variables or use strings */

double GetAOMdelay()
{
	return AOMdelay;
}

virtual double PiTimePowerFactor(const string& PulseName);
virtual double PiTime(const physics::line& l, bool bAlwaysCalculate = false);

virtual double ReferenceStateG(const physics::line& l) = 0;
virtual double ReferenceStateE(const physics::line& l) = 0;

bool SetPiTime(double t, const physics::line& l, bool bCanRescale = true);
double AOMFrequencyFourierLimit(double PiTime);

/**
 *Returns the transition frequency for mFg --> mFe, sideband delta_n, in AOM Hz (Hz scaled by Fmultiplier)
 *If AlwaysCalculate is true the value is always calculated from the transitions and the motional frequencies.
 *Otherwise the value is first looked up in the calibration map.
 */
double AOMFrequency(const physics::line& l, bool AlwaysCalculate = false);

bool SetAOMFrequency(double f, const physics::line& l);

bool SetModeFrequency(double f, int delta_n);
double GetModeFrequency(int delta_n);

double FrequencySpan();
double FrequencyCenter();

template<class T> bool LinkPulseTo(T*, const physics::line& l);
template<class T> bool LinkPulseTo(T*, const std::string& name);

virtual bool CanLink(const string&)
{
	return false;
}

void ScalePiTimes(double);

void SetB(double B);

//return the current ground state of the ion
//todo: allow specification of ground vs. excited state

double GetCurrentGroundState() const
{
	return current_mFg;
}
void SetCurrentGroundState(double mFg)
{
	current_mFg = mFg;
}

virtual void ShiftLine(const physics::line&, double, int) = 0;
void ShiftAllFrequencies(double delta_f);

int NormalModes() const
{
	return NumNormalModes;
}

double mFg_min() const
{
	return tx.g->mFmin();
}
double mFg_max() const
{
	return tx.g->mFmax();
}
double mFe_min() const
{
	return tx.e->mFmin();
}
double mFe_max() const
{
	return tx.e->mFmax();
}

virtual void InitExperimentStart()
{
}
virtual void InitExperimentSwitch()
{
}

deque<calibration_item> calibrations;


virtual run_status Run();
virtual void DefaultExperimentState()
{
}
virtual void FinishRun()
{
}

static void UpdatePolarizationGUI(GUI_pol* p, physics::ElectronicTransition* t, double mFg);

GUI_dds* GetPulseParam(const physics::line& l);
GUI_dds* GetPulseParam(const std::string& name);

virtual ExpSCAN* getCalibrationExp(const calibration_item&);
virtual void DidCalibration(const calibration_item*, numerics::FitObject*);

virtual void PostCreateGUI();

signals:
void sigSelectScanTarget(const calibration_item*, bool);

protected slots:
void selectScanTarget(const calibration_item*, bool);


public slots:
void slot_cal_t(Pulse_Widget*);
void slot_cal_f(Pulse_Widget*);

protected:
virtual bool needInitFPGA()
{
	return true;
}                                               //initialize FPGA params at startup

void UpdateGUIPulseNames(double mFg, double mFe);

virtual bool ShouldCalcPiTime(const physics::line&)
{
	return true;
}

virtual string GetPulseName(const physics::line& l);
physics::line getLine(const std::string& s);


int NumNormalModes;

virtual double GetGroundState();
virtual double GetExcitedState();

//CalibrateMagneticField must calibrate the transition's magnetic field
virtual double CalibrateMagneticField() = 0;

//calculate motional freq's from line centers
virtual double CalculateModeFrequency(int);

virtual bool RecalculateParameters();

bool CalculateParameters(bool bCalcFreqs, bool bCalcTimes);

public:
physics::MotionalTransition tx;

public:
GUI_double AOMdelay;

protected:
GUI_bool CalculateFreqs;

public:
GUI_double MagneticField;
GUI_double F0;

protected:
GUI_ZS GroundState;
GUI_pol Polarization;
GUI_int NumCalibrationModes;
// GUI_string      CalibrationList;
protected:
std::vector<physics::line> calibration_pulses;
std::vector<std::string> pulse_names;

std::string current_carrier_name;

//frequencies & Lamb-Dickes for the different ports (beams)
GUI_matrix* motion;

double current_mFg;     //current Zeeman state

std::vector<GUI_dds*> m_vSBParams;

int AOMorder;
unsigned NumPorts;
};

class CoolingTransitionPage : public TransitionPage
{
public:
CoolingTransitionPage(const string& sPageName,
                      ExperimentsSheet* pSheet,
                      unsigned page_id,
                      physics::HFS* g,
                      physics::HFS* e,
                      const std::string& xname,
                      int NumModes = 0) :

	TransitionPage(sPageName, pSheet, page_id, g, e, xname, NumModes)
{
}

virtual ~CoolingTransitionPage()
{
}

virtual double RamanCoolingTime() const = 0;
};


