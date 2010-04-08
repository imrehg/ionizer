#include "DopplerTempCalibration.h"
#include "ExpSCAN_NewFPGA.h"
#include "data_plot.h"

using namespace std;

DopplerTempCalibration::DopplerTempCalibration(const string& sPageName, ExperimentsSheet* pSheet) :
	CalibrationPage(sPageName, pSheet, 1000),
	CalType("Cal. type", "Time\nFrequency\n",    &m_TxtParams, "Time", &m_vParameters),
	Wait("Wait [us]", &m_TxtParams, "0", &m_vParameters),
	Sideband("Sideband", &m_TxtParams, "1", &m_vParameters),
	rsb_min("RSB min.", &m_TxtParams, "1", &m_vParameters),
	rsb_max("RSB max.", &m_TxtParams, "1", &m_vParameters),
	rsb_height("RSB height", &m_TxtParams, "1", &m_vParameters),
	bsb_min("BSB min.", &m_TxtParams, "1", &m_vParameters),
	bsb_max("BSB max.", &m_TxtParams, "1", &m_vParameters),
	bsb_height("BSB height", &m_TxtParams, "1", &m_vParameters),
	n_bar("n-bar", &m_TxtParams, "1", &m_vParameters),
	data_plots(2, (data_plot*) 0)
{
	rsb_min.SetReadOnly(true);
	rsb_max.SetReadOnly(true);
	rsb_height.SetReadOnly(true);

	bsb_min.SetReadOnly(true);
	bsb_max.SetReadOnly(true);
	bsb_height.SetReadOnly(true);

	n_bar.SetReadOnly(true);

	Sideband.addFlag(RP_FLAG_NOPACK);
}

void DopplerTempCalibration::DidCalibration(const calibration_item* ci, numerics::FitObject* pFit)
{
	if (ci->l.sb < 0)
	{
		rsb_min.SetValue(pFit->get_fitYmin());
		rsb_max.SetValue(pFit->get_fitYmax());
		rsb_height.SetValue(rsb_max - rsb_min);
	}
	else
	{
		bsb_min.SetValue(pFit->get_fitYmin());
		bsb_max.SetValue(pFit->get_fitYmax());
		bsb_height.SetValue(bsb_max - bsb_min);
	}

	double delta = std::max<double>( (bsb_height - rsb_height), 1e-9);
	double nb = rsb_height / delta;

	n_bar.SetValue(nb);

	emit sig_update_data();
}

//called once when the experiment is first started
void DopplerTempCalibration::InitExperimentStart()
{
	//find calibration pages
	pCal[0] = dynamic_cast<ExpQubit*>(m_pSheet->FindPage("Qubit cal 1"));
	pCal[1] = dynamic_cast<ExpQubit*>(m_pSheet->FindPage("Qubit cal 2"));

	if (!pCal[0] || !pCal[1])
		throw runtime_error("[DopplerTempCalibration::InitExperimentStart] couldn't find calibration pages");

	physics::line l1(-3.0, -2.0, Sideband);
	physics::line l2(-3.0, -2.0, -1 * Sideband);

	cal.clear();

	if (CalType.Value() == "Time")
	{
		calibration_item ci = cal_item(l1, "Time", "NoGSC IgnoreFit");
		ci.wait = Wait;
		cal.push_back(ci);

		ci = cal_item(l2, "Time", "NoGSC IgnoreFit");
		ci.wait = Wait;
		cal.push_back( ci );
	}
	else
	{
		calibration_item ci = cal_item(l1, "Frequency", "NoGSC");
		ci.num_scans = 2;
		ci.wait = Wait;
		cal.push_back(ci);

		ci = cal_item(l2, "Frequency", "NoGSC");
		ci.num_scans = 2;
		ci.wait = Wait;
		cal.push_back(ci);
	}

	for (unsigned i = 0; i < 2; i++)
		pCal[i]->DoCalibration( &(cal[i]), 0 );
}

void DopplerTempCalibration::PostCreateGUI()
{
	CalibrationPage::PostCreateGUI();

	plot_grid = new QHBoxLayout();
	grid.addLayout(plot_grid, grid.rowCount(), 0, 12, -1);
}


bool DopplerTempCalibration::acceptCalibrationPlot(calibration_item*, data_plot* pDP)
{
	if (!pDP)
		return true;

	if (pDP->getTitle().find("Qubit cal 1") != string::npos)
	{
		if (data_plots[0])
		{
			plot_grid->removeWidget(data_plots[0]);
			delete data_plots[0];
		}

		data_plots[0] = pDP;
	}

	if (pDP->getTitle().find("Qubit cal 2") != string::npos)
	{
		if (data_plots[1])
		{
			plot_grid->removeWidget(data_plots[1]);
			delete data_plots[1];
		}

		data_plots[1] = pDP;
	}

	plot_grid->addWidget(pDP);

	return true;
}


data_plot* DopplerTempCalibration::get_data_plot(calibration_item*, const std::string& sTitle)
{
	for (unsigned i = 0; i < data_plots.size(); i++)
		if (data_plots[i])
			if (data_plots[i]->getTitle().find(sTitle) != string::npos)
				return data_plots[i];

	return 0;
}
