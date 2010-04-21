#include "common.h"
#include "ExpCorrelate.h"
#include "FPGA_connection.h"
#include "data_plot.h"
#include "histogram_plot.h"
#include "Utils.h"

ExpCorrelate::ExpCorrelate(const string& sPageName,
                           ExperimentsSheet* pSheet,
                           unsigned page_id) :
	ExpSCAN_NewFPGA(sPageName, pSheet, page_id)
{
}

unsigned ExpCorrelate::plot_columns(unsigned nPlots)
{
	return std::max<int>(1, nPlots / 2);
}



void ExpCorrelate::on_action(const std::string& s)
{
	if (s == "RUN")
		AddPagePlots();

	ExpSCAN_NewFPGA::on_action(s);
}

void ExpCorrelate::PostAcquireDataPoint(Scan_Base* sb, DataFeed& df)
{
	ExpSCAN_NewFPGA::PostAcquireDataPoint(sb, df);

	update_plots(0);
}
