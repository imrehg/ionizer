#pragma once

#include "TransitionPage.h"
#include "FrequencySource.h"

class MgPage : public TransitionPage
{
public:
MgPage(const string& sPageName,
       ExperimentsSheet* pSheet,
       unsigned page_id);

virtual ~MgPage()
{
}

static bool matchTitle(const std::string& s)
{
	return (s == "Mg") || (s.find("Mg Al") != std::string::npos);
}

virtual string TransitionName() const
{
	return "Mg";
}

//	virtual void CalibrateTransitions();
//	virtual void MeasureTransitionFrequency(int sideband);

double AOMFrequency(int delta_n, bool AlwaysCalculate = false);
double PiTime(int delta_n, bool AlwaysCalculate = false);

virtual double ReferenceStateG(const physics::line& l);
virtual double ReferenceStateE(const physics::line& l);

virtual double GetCurrentGroundState()
{
	return -3;
}

virtual void ShiftLine(const physics::line&, double, int);

static auto_ptr<FrequencySource> pHFS_synth;

//return the motional mode amplitude z0 for the given mode number
virtual double ModeAmplitude(int);

//for 90 deg. Raman beams
double delta_k() const
{
	return 2*M_PI*sqrt(2.) / lambda();
}
double lambda() const
{
	return 280.e-9;
}
double mass() const
{
	return 4.15e-26;
}

virtual double CalibrateMagneticField();
virtual bool CanLink(const string& pname);

virtual ExpSCAN* getCalibrationExp(const calibration_item&);

protected:
virtual bool RecalculateParameters();
virtual void InitExperimentStart();

GUI_dds Detect, DopplerCool, Precool;
GUI_ttl Repump;
};
