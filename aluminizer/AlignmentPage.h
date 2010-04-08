#ifndef ALIGNMENT_PAGE_H
#define ALIGNMENT_PAGE_H

#include "CalibrationPage.h"

class ExpQubit;
class ExpSCAN_Detect;

class AlignMirrorsPage : public CalibrationPage
{
public:
AlignMirrorsPage(const string& sPageName, ExperimentsSheet* pSheet);

void AddAvailableActions(std::vector<std::string>* p);
void on_action(const std::string& s);

virtual void DidCalibration(const calibration_item*, numerics::FitObject*);

virtual unsigned num_columns()
{
	return 6;
}

virtual bool acceptCalibrationPlot(calibration_item* ci, data_plot*);
virtual data_plot* get_data_plot(calibration_item* ci, const std::string& sTitle);

virtual void PostCreateGUI();

virtual void InitExperimentStart();

virtual bool wantsPlotLegend()
{
	return false;
}

protected:

int getPlotIndex(const calibration_item* ci);

virtual bool needsBottomSpacer()
{
	return false;
}

protected:
GUI_int NumExp;
vector<GUI_double*> FitCenters;
vector<GUI_int*> Spans;

std::vector<calibration_item> cal;
ExpQubit* pCal;

std::vector<data_plot*> data_plots;

QGridLayout* plot_grid;
};

class AlignBfieldPage : public CalibrationPage
{
public:
AlignBfieldPage(const string& sPageName, ExperimentsSheet* pSheet);

void AddAvailableActions(std::vector<std::string>* p);
void on_action(const std::string& s);

virtual void DidCalibration(const calibration_item*, numerics::FitObject*);

virtual unsigned num_columns()
{
	return 6;
}

virtual bool acceptCalibrationPlot(calibration_item* ci, data_plot*);
virtual data_plot* get_data_plot(calibration_item* ci, const std::string& sTitle);

virtual void PostCreateGUI();

virtual void InitExperimentStart();

virtual bool wantsPlotLegend()
{
	return false;
}

protected:

int getPlotIndex(const calibration_item* ci);

virtual bool needsBottomSpacer()
{
	return false;
}

protected:
vector<GUI_double*> FitCenters;
GUI_double Bx_start, Bx_stop, By_start, By_stop;

std::vector<calibration_item> cal;
ExpSCAN_Detect* pCal;

std::vector<data_plot*> data_plots;

QGridLayout* plot_grid;

unsigned nFields;
};


#endif //ALIGNMENT_PAGE_H
