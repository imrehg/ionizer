#include "AlignmentPage.h"
#include "ExpSCAN_NewFPGA.h"
#include "data_plot.h"
#include "AgilisMotors.h"
#include "Bfield.h"
#include "ExpAl.h"

using namespace std;

AlignMirrorsPage::AlignMirrorsPage(const string& sPageName, ExperimentsSheet* pSheet) :
	CalibrationPage(sPageName, pSheet, 1000),
	NumExp("Num. exp.", &m_TxtParams, "100", &m_vParameters)
{
	RemoveParameter(&NumPoints);
	NumExp.setRange(0, 200);

	unsigned nMotors = ExpAl::pMirrorMotors->getNumMotors();

	data_plots.resize(nMotors, (data_plot*)0),
	Spans.resize(nMotors);
	FitCenters.resize(nMotors);

	for (unsigned i = 0; i < nMotors; i++)
	{
		char sbuff[64];

		snprintf(sbuff, 64, "Span (%s)", ExpAl::pMirrorMotors->getName(i).c_str() );
		Spans[i] = new GUI_int(sbuff,   &m_TxtParams, "20", &m_vParameters);
		m_vAllocatedParams.push_back(Spans[i]);
		Spans[i]->setFlag(RP_FLAG_4_COLUMN, true);

		snprintf(sbuff, 64, "Fit (%s)", ExpAl::pMirrorMotors->getName(i).c_str() );
		FitCenters[i] = new GUI_double(sbuff, &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(FitCenters[i]);

		FitCenters[i]->SetReadOnly(true);
		FitCenters[i]->setFlag(RP_FLAG_4_COLUMN, true);
		FitCenters[i]->setRange(-1e4, 1e4);
		FitCenters[i]->setIncrement(0.1);
	}
}

void AlignMirrorsPage::AddAvailableActions(std::vector<std::string>* p)
{
	CalibrationPage::AddAvailableActions(p);

	if (cal.size() == 0)
		for (unsigned i = 0; i < ExpAl::pMirrorMotors->getNumMotors(); i++)
			p->push_back(ExpAl::pMirrorMotors->getName(i));

	p->push_back("USE FITS");
}

void AlignMirrorsPage::on_action(const std::string& s)
{
	bool bTriggered = false;

	if (cal.size() == 0)
	{
		for (unsigned i = 0; i < ExpAl::pMirrorMotors->getNumMotors(); i++)
		{
			if (ExpAl::pMirrorMotors->getName(i) == s || (s == "RUN"))
			{
				physics::line l1(-3.0, -2.0, 0);

				cal.push_back( cal_item(l1, "Motor", "NoGSC IgnoreFit") );
				cal.back().scan_variable = ExpAl::pMirrorMotors->getName(i);
				cal.back().span = Spans[i]->Value();
				cal.back().num_points = cal.back().span + 1;
				cal.back().num_exp = NumExp;

				bTriggered = true;
			}
		}

		if (s == "USE FITS")
			for (unsigned i = 0; i < FitCenters.size(); i++)
				ExpAl::pMirrorMotors->setScanOutput(i, FitCenters[i]->Value());

		if (cal.size())
			m_pSheet->scan_scheduler.StartExperiment(this, 0);
	}



	if (!bTriggered)
		CalibrationPage::on_action(s);
}

//called once when the experiment is first started.  launches calibration experiment
void AlignMirrorsPage::InitExperimentStart()
{
	//find calibration page
	pCal = dynamic_cast<ExpQubit*>(m_pSheet->FindPage("Qubit cal 1"));

	if (!pCal)
		throw runtime_error("[AlignMirrorsPage::InitExperimentStart] couldn't find calibration page");

	if (cal.size())
		pCal->DoCalibration( &(cal[0]), 0 );
}

void AlignMirrorsPage::DidCalibration(const calibration_item* ci, numerics::FitObject* pFit)
{
	numerics::PeakFitObject* pFitPeak = dynamic_cast<numerics::PeakFitObject*>(pFit);

	if (pFitPeak)
	{
		FitCenters.at(getPlotIndex(ci))->SetValue(pFitPeak->GetCenter());
		emit sig_update_data();
	}

	for (unsigned i = 0; i < cal.size(); i++)
		if (ci == &(cal[i]) )
			cal.erase( cal.begin() + i );

	if (cal.size())
		m_pSheet->scan_scheduler.StartExperiment(this, 0);
}

void AlignMirrorsPage::PostCreateGUI()
{
	CalibrationPage::PostCreateGUI();

	plot_grid = new QGridLayout();
	grid.addLayout(plot_grid, grid.rowCount(), 0, 12, -1);
}

int AlignMirrorsPage::getPlotIndex(const calibration_item* ci)
{
	for (unsigned i = 0; i < ExpAl::pMirrorMotors->getNumMotors(); i++)
		if (ExpAl::pMirrorMotors->getName(i) == ci->scan_variable)
			return i;

	throw runtime_error("[AlignMirrorsPage::getPlotIndex] no such motor: " + ci->scan_variable);
}

bool AlignMirrorsPage::acceptCalibrationPlot(calibration_item* ci, data_plot* pDP)
{
	if (!pDP)
		return true;

	int iPlot = getPlotIndex(ci);

	if (data_plots[iPlot])
	{
		plot_grid->removeWidget(data_plots[iPlot]);
		delete data_plots[iPlot];
	}

	data_plots[iPlot] = pDP;

	plot_grid->addWidget(pDP, iPlot / 2, iPlot % 2);

	return true;
}

data_plot* AlignMirrorsPage::get_data_plot(calibration_item* ci, const std::string&)
{
	return data_plots.at(getPlotIndex(ci));

	return 0;
}


AlignBfieldPage::AlignBfieldPage(const string& sPageName, ExperimentsSheet* pSheet) :
	CalibrationPage(sPageName, pSheet, 1000),
	Bx_start("Bx start", &m_TxtParams, "2000", &m_vParameters),
	Bx_stop("Bx stop", &m_TxtParams, "4000", &m_vParameters),
	By_start("By start", &m_TxtParams, "0", &m_vParameters),
	By_stop("By stop", &m_TxtParams, "1000", &m_vParameters),
	nFields(2)
{
	Bx_start.setFlag(RP_FLAG_4_COLUMN, true);
	Bx_stop.setFlag(RP_FLAG_4_COLUMN, true);

	data_plots.resize(nFields, (data_plot*)0),
	FitCenters.resize(nFields);

	for (unsigned i = 0; i < nFields; i++)
	{
		char sbuff[64];

		snprintf(sbuff, 64, "Fit (%d)", i);
		FitCenters[i] = new GUI_double(sbuff,  &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(FitCenters[i]);

		FitCenters[i]->SetReadOnly(true);
	}
}

void AlignBfieldPage::AddAvailableActions(std::vector<std::string>* p)
{
	CalibrationPage::AddAvailableActions(p);

	if (cal.size() == 0)
	{
		p->push_back("Bx");
		p->push_back("By");
	}

	p->push_back("USE FITS");
}

void AlignBfieldPage::on_action(const std::string& s)
{
	bool bTriggered = false;

	if (cal.size() == 0)
	{
		physics::line l1(-3.0, -2.0, 0);

		if ("Bx" == s || (s == "RUN"))
		{
			cal.push_back( cal_item(l1, "B-field", "IgnoreFit") );
			cal.back().scan_variable = "Bx";
			cal.back().start = Bx_start;
			cal.back().stop = Bx_stop;

			bTriggered = true;
		}

		if ("By" == s || (s == "RUN"))
		{
			cal.push_back( cal_item(l1, "B-field", "IgnoreFit") );
			cal.back().scan_variable = "By";
			cal.back().start = By_start;
			cal.back().stop = By_stop;


			bTriggered = true;
		}


		if (s == "USE FITS")
			for (unsigned i = 0; i < FitCenters.size(); i++)
				gBfield->SetB(i, FitCenters[i]->Value());

		if (cal.size())
			m_pSheet->scan_scheduler.StartExperiment(this, 0);
	}

	if (!bTriggered)
		CalibrationPage::on_action(s);
}

//called once when the experiment is first started.  launches calibration experiment
void AlignBfieldPage::InitExperimentStart()
{
	//find calibration page
	pCal = dynamic_cast<ExpSCAN_Detect*>(m_pSheet->FindPage("Detect"));

	if (!pCal)
		throw runtime_error("[AlignBfieldPage::InitExperimentStart] couldn't find calibration page");

	if (cal.size())
		pCal->DoCalibration( &(cal[0]), 0 );
}

void AlignBfieldPage::DidCalibration(const calibration_item* ci, numerics::FitObject* pFit)
{
	numerics::PeakFitObject* pFitPeak = dynamic_cast<numerics::PeakFitObject*>(pFit);

	if (pFitPeak)
	{
		FitCenters.at(getPlotIndex(ci))->SetValue(pFitPeak->GetCenter());
		emit sig_update_data();
	}

	for (unsigned i = 0; i < cal.size(); i++)
		if (ci == &(cal[i]) )
			cal.erase( cal.begin() + i );

	if (cal.size())
		m_pSheet->scan_scheduler.StartExperiment(this, 0);
}

//move to base class?
void AlignBfieldPage::PostCreateGUI()
{
	CalibrationPage::PostCreateGUI();

	plot_grid = new QGridLayout();
	grid.addLayout(plot_grid, grid.rowCount(), 0, 12, -1);
}

int AlignBfieldPage::getPlotIndex(const calibration_item* ci)
{
	if ("Bx" == ci->scan_variable)
		return 0;

	if ("By" == ci->scan_variable)
		return 1;

	throw runtime_error("[AlignBfieldPage::getPlotIndex] no such plot: " + ci->scan_variable);
}

//move to base class?
bool AlignBfieldPage::acceptCalibrationPlot(calibration_item* ci, data_plot* pDP)
{
	if (!pDP)
		return true;

	int iPlot = getPlotIndex(ci);

	if (data_plots[iPlot])
	{
		plot_grid->removeWidget(data_plots[iPlot]);
		delete data_plots[iPlot];
	}

	data_plots[iPlot] = pDP;

	plot_grid->addWidget(pDP, iPlot / 2, iPlot % 2);

	return true;
}

//move to base class?
data_plot* AlignBfieldPage::get_data_plot(calibration_item* ci, const std::string&)
{
	return data_plots.at(getPlotIndex(ci));

	return 0;
}
