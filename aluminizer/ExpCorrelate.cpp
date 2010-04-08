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

void ExpCorrelate::update_plots(double min_delay)
{
	if (page_plots.size() == 0)
		AddPagePlots();

	if (min_delay + last_plot_update > CurrentTime_s())
		return;

	for (unsigned i = 0; i < page_plots.size(); i++)
	{
		if (page_plots[i])
		{
			last_plot_update = CurrentTime_s();

			valarray<double> pp = pFPGA->getPagePlotData(page_id, i);

			page_plots[i]->barPlot(pp);
		}
	}
}

unsigned ExpCorrelate::plot_columns(unsigned nPlots)
{
	return std::max<int>(1, nPlots / 2);
}

unsigned ExpCorrelate::plot_rows(unsigned nPlots)
{
	return nPlots / plot_columns(nPlots);
}

void ExpCorrelate::AddPagePlots()
{
	unsigned nPlots = pFPGA->numPagePlots(page_id);

	if (nPlots == page_plots.size())
		return;

	for (unsigned j = 0; j < plot_rows(nPlots); j++)
	{
		hgrids.push_back(new QHBoxLayout());
		grid.addLayout(hgrids.back(), grid.rowCount(), 0, 4, -1);

		for (unsigned i = 0; i < plot_columns(nPlots); i++)
		{
			page_plots.push_back(new histogram_plot(this, "", ""));
			hgrids.back()->addWidget(page_plots.back());
			page_plots.back()->show();
		}
	}
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
