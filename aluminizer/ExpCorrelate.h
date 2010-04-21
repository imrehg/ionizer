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

virtual void on_action(const std::string& s);
virtual void PostAcquireDataPoint(Scan_Base*, DataFeed&);

virtual unsigned plot_columns(unsigned nPlots);
};

