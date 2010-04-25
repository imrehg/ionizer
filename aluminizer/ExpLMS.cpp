#include "common.h"
#include "ExpLMS.h"
#include "FPGA_connection.h"
#include "data_plot.h"
#include "histogram_plot.h"
#include "ionizer_utils.h"

ExpLMS::ExpLMS(const string& sPageName,
               ExperimentsSheet* pSheet,
               unsigned page_id) :
	ExpSCAN_NewFPGA(sPageName, pSheet, page_id),
	replot_filters("Replot filters", &m_TxtParams, "true", &m_vParameters),
	taps_path("Taps load path", &m_TxtParams, "c:/data/taps/", &m_vParameters)
{
}

void ExpLMS::update_plots(double min_delay)
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

unsigned ExpLMS::plot_columns(unsigned nPlots)
{
	return nPlots / 2;
}

unsigned ExpLMS::plot_rows(unsigned nPlots)
{
	return nPlots / plot_columns(nPlots);
}

void ExpLMS::AddPagePlots()
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

void ExpLMS::load_taps()
{
	if (page_plots.size() == 0)
		return;

	GUI_unsigned* num_taps = dynamic_cast<GUI_unsigned*>(FindFPGAParameter("Num. taps"));
	if (num_taps == 0)
		throw runtime_error("unable to find Num. taps parameter");

	int M = num_taps->Value();
	valarray <int> taps(M * page_plots.size());
	string line;

	ifstream ifs;
	ifs.open((taps_path.Value()).c_str());

	if (ifs.is_open())
	{
		unsigned i = 0;
		while ((i < M * page_plots.size()) && (!ifs.eof()))
		{
			getline(ifs, line);
			taps[i] = atoi(line.c_str());
			i++;
		}
	}
	ifs.close();

	//valarray <int> f (M);
//	for(unsigned j=0; j<M; j++)
//		f[j] = j;

	for (unsigned i = 0; i < page_plots.size(); i++)
	{
		throw runtime_error("function call deleted -- missing code.  TR 2/25/2010");
		//	pFPGA->setCoefficients(page_id, i,taps[slice(i*M,M,1)]);
		//	pFPGA->setCoefficients(page_id, i,f);
		//taps = taps.cshift(M);
	}
	cout << "taps loaded!   tap[1] = " << taps[1] << endl;
}

void ExpLMS::save_taps()
{
	FILE* tFile;

	QDir wd(("C:\\data\\taps\\" + GetDateTimeString(0, 0)).c_str());

	wd.mkpath(wd.absolutePath());
	string fName = "C:\\data\\taps\\" + GetDateTimeString(0, 0) + "\\taps.txt";

	if (page_plots[1])
	{
		tFile = fopen(fName.c_str(), "w");

		for (unsigned i = 0; i < page_plots.size(); i++)
		{
			valarray<double> pp = pFPGA->getPagePlotData(page_id, i);

			for (unsigned j = 0; j < pp.size(); j++)
				fprintf(tFile, "%d\n", int( pp[j]));
		}
	}

	cout << fName << endl;
	fclose(tFile);
}

void ExpLMS::on_action(const std::string& s)
{
	if (s == "RUN")
		AddPagePlots();

	if (s == "UPDATE PLOTS")
		update_plots(0);

	if (s == "SAVE TAPS")
		save_taps();

	if (s == "LOAD TAPS")
		load_taps();

	ExpSCAN_NewFPGA::on_action(s);
}

void ExpLMS::PostAcquireDataPoint(Scan_Base* sb, DataFeed& df)
{
	ExpSCAN_NewFPGA::PostAcquireDataPoint(sb, df);

	if (replot_filters)
		update_plots(0);
}

void ExpLMS::AddAvailableActions(std::vector<std::string>* v)
{
	ExpSCAN_NewFPGA::AddAvailableActions(v);

	v->push_back("UPDATE PLOTS");
	v->push_back("SAVE TAPS");
	v->push_back("LOAD TAPS");
}
