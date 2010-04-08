#pragma once

#include "ExpSCAN_NewFPGA.h"

class ExpLMS : public ExpSCAN_NewFPGA
{
public:
ExpLMS(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id);

virtual ~ExpLMS()
{
}

static bool matchTitle(const std::string& s)
{
	return s.find("ADC") != string::npos;
}

protected:
void AddPagePlots();
void save_taps();
void load_taps();

virtual void AddAvailableActions(std::vector<std::string>*);
virtual void on_action(const std::string& s);

//get filter coefficient data from FPGA and plot
void update_plots(double min_delay = 0);

virtual void PostAcquireDataPoint(Scan_Base*, DataFeed&);

unsigned plot_columns(unsigned nPlots);
unsigned plot_rows(unsigned nPlots);

std::vector<histogram_plot*> page_plots;
double last_plot_update;   // time when plot was last updated

GUI_bool replot_filters;
GUI_string taps_path;

};

