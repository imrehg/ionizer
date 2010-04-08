#pragma once

#include "ExperimentPage.h"
#include "CalibrationPage.h"

using namespace std;

class ExpQubit;

class DopplerTempCalibration : public CalibrationPage
{
public:
DopplerTempCalibration(const string& sPageName, ExperimentsSheet* pSheet);

//called once when the experiment is first started
virtual void InitExperimentStart();

virtual void DidCalibration(const calibration_item*, numerics::FitObject*);

virtual unsigned num_columns()
{
	return 6;
}

virtual bool acceptCalibrationPlot(calibration_item* ci, data_plot*);
virtual data_plot* get_data_plot(calibration_item* ci, const std::string& sTitle);

virtual void PostCreateGUI();

protected:
virtual bool needsBottomSpacer()
{
	return false;
}

protected:
std::vector<calibration_item> cal;
ExpQubit* pCal[2];

GUI_double Wait;
GUI_combo CalType;
GUI_int Sideband;


GUI_double rsb_min, rsb_max, rsb_height;
GUI_double bsb_min, bsb_max, bsb_height;
GUI_double n_bar;

std::vector<data_plot*> data_plots;

QHBoxLayout* plot_grid;
};
