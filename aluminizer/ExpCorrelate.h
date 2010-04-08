#pragma once

#include "ExpSCAN_NewFPGA.h"

class ExpCorrelate : public ExpSCAN_NewFPGA
{
public:
ExpCorrelate(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id);

virtual ~ExpCorrelate()
{
}

static bool matchTitle(const std::string& s)
{
	return s.find("Correlate") != string::npos;
}

protected:
void AddPagePlots();

virtual void on_action(const std::string& s);

//get filter coefficient data from FPGA and plot
void update_plots(double min_delay = 0);

virtual void PostAcquireDataPoint(Scan_Base*, DataFeed&);

unsigned plot_columns(unsigned nPlots);
unsigned plot_rows(unsigned nPlots);

std::vector<histogram_plot*> page_plots;
double last_plot_update;   // time when plot was last updated

};

