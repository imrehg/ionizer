#pragma once

#include "TransitionPage.h"

class RefCavityPage;

class histogram_plot;

class Al3P1Page : public TransitionPage
{
public:
Al3P1Page(const string& sPageName,
          ExperimentsSheet* pSheet,
          unsigned page_id,
          int NumModes = 6);

virtual ~Al3P1Page()
{
}

virtual string TransitionName() const
{
	return "Al 3P1";
}

static bool matchTitle(const std::string& s)
{
	return s.find("Al 3P1") != string::npos;
}

//	virtual void CalibrateTransitions();
//	virtual void MeasureTransitionFrequency(int sideband);

//	double AOMFrequency(int delta_n, bool AlwaysCalculate=false);
double PiTime(int delta_n, bool AlwaysCalculate = false);

virtual double ReferenceStateG(const physics::line& l);
virtual double ReferenceStateE(const physics::line& l);

virtual double GetCurrentGroundState()
{
	return -3;
}

virtual void ShiftLine(const physics::line&, double, int);

//return the motional mode amplitude z0 for the given mode number
virtual double ModeAmplitude(int);

//for 90 deg. Raman beams
double delta_k() const
{
	return 2*M_PI*sqrt(2.) / lambda();
}
double lambda() const
{
	return 267.e-9;
}
double mass() const
{
	return 4.48e-26;
}

virtual double CalibrateMagneticField();

void ReCenterUVAOM(const physics::line& l1,
                   const physics::line& l2,
                   bool bAdjustDrift);

virtual bool CanLink(const string& pname);

virtual ExpSCAN* getCalibrationExp(const calibration_item&);
virtual void InitExperimentStart();

protected:
GUI_double CommonModeCorrection, NominalAOMCenter;
};


class Al3P0Page : public TransitionPage
{
public:
Al3P0Page(const string& sPageName,
          ExperimentsSheet* pSheet,
          unsigned page_id,
          int NumModes = 0);

virtual ~Al3P0Page()
{
}

virtual string TransitionName() const
{
	return "Al 3P0";
}
static bool matchTitle(const std::string& s)
{
	return s.find("Al 3P0") != string::npos;
}

//	virtual void CalibrateTransitions();
//	virtual void MeasureTransitionFrequency(int sideband);

//	double AOMFrequency(int delta_n, bool AlwaysCalculate=false);
double PiTime(int delta_n, bool AlwaysCalculate = false);

virtual double ReferenceStateG(const physics::line& l);
virtual double ReferenceStateE(const physics::line& l);

virtual double GetCurrentGroundState()
{
	return -3;
}

virtual void ShiftLine(const physics::line&, double, int);

//return the motional mode amplitude z0 for the given mode number
virtual double ModeAmplitude(int);

double lambda() const
{
	return 267.e-9;
}
double mass() const
{
	return 4.48e-26;
}

virtual double CalibrateMagneticField();

void ReCenterUVAOM(const physics::line& l1,
                   const physics::line& l2,
                   bool bAdjustDrift);

virtual bool CanLink(const string& pname);

virtual ExpSCAN* getCalibrationExp(const calibration_item&);

protected:

virtual void AddAvailableActions(std::vector<std::string>*);
virtual void on_action(const std::string& s);

public:
double currentClockState;

protected:
RefCavityPage* pRefCavity;
GUI_double CommonModeCorrection;
GUI_double NominalAOMCenter;
};
