#pragma once

#include "ExperimentPage.h"
#include "Fitting.h"

using namespace std;

class CalibrationPage;
class data_plot;

class calibration_item
{
public:
calibration_item(const physics::line& l,
                 const std::string& scan_type,
                 const std::string& scan_variable,
                 const std::string& options,
                 CalibrationPage* pX, GUI_dds* pGUI,
                 unsigned num_scans = 0,
                 unsigned num_points = 0,
                 double num_flops = 0,
                 double wait = 0) :
	l(l), scan_type(scan_type), scan_variable(scan_variable), options(options), pX(pX), pGUI(pGUI),
	num_scans(num_scans), num_points(num_points), num_exp(0), num_flops(num_flops), wait(wait), span(0), start(0), stop(0)
{
}


bool IsTimeScan() const
{
	return scan_type.find("Time") != std::string::npos;
}

bool IsFreqScan() const
{
	return scan_type.find("Frequency") != std::string::npos;
}

GUI_dds* getParamGUI() const
{
	return pGUI;
}

physics::line l;
std::string scan_type;
std::string scan_variable;
std::string options;

CalibrationPage* pX;

protected:
GUI_dds* pGUI;

public:
unsigned num_scans, num_points, num_exp;
double num_flops, wait, span, start, stop;
};

class CalibrationPage : public ExperimentPage
{
public:
CalibrationPage(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);

calibration_item cal_item(const physics::line& l,
                          const std::string& scan_type,
                          const std::string& options,
                          GUI_dds* pGUI = 0);


virtual bool acceptCalibrationPlot(calibration_item*, data_plot*)
{
	return false;
}
virtual data_plot* get_data_plot(calibration_item*, const std::string&)
{
	return 0;
}

virtual void DidCalibration(const calibration_item*, numerics::FitObject*)
{
}

//called once when the experiment is first started
virtual void InitExperimentStart()
{
}

//called every time before the experiment continues, after another experiment was running
virtual void InitExperimentSwitch()
{
}

virtual run_status Run()
{
	return FINISHED;
}
virtual void DefaultExperimentState()
{
}
virtual void FinishRun()
{
}

virtual bool wantsPlotLegend()
{
	return true;
}

protected:
GUI_int NumScans, NumPoints;
GUI_double NumFlops;
};

