#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ExperimentPage.h"
#include "MgAlExperimentsSheet.h"
#include <QTableView>
#include <QHeaderView>

using namespace std;


GlobalsPage*      ExperimentPage::pGlobals(0);
Voltages2*        ExperimentPage::pVoltages(0);
SwitchPanel*      ExperimentPage::pSwitches(0);
//MotionPage*			ExperimentPage::pMotion(0);

ParamsPage::ParamsPage(ExperimentsSheet* pSheet, const std::string& sPageName) :
	TxtParametersGUI(pSheet, sPageName),
	m_TxtParams(ParamsFileName()),
	view(0)
{
}

ParamsPage::~ParamsPage()
{
	if(view)
		delete view;
}

void ParamsPage::SaveParams(const std::string& OutputDirectory)
{
	m_TxtParams.SaveState(OutputDirectory);
}


std::string ParamsPage::ParamsFileName()
{
	string s = Title();

	replace(s.begin(), s.end(), '/', '_');

	return "params" + s + ".txt";
}

void ParamsPage::createModelView()
{

	if(view)
	{
		if(! view->isVisible())
		{
			delete view;
			view = 0;
		}
	}

	if(view == 0)
	{
		view = new QTableView;
		view->setModel(&m_TxtParams);
		view->setWindowTitle(title.c_str());
		view->horizontalHeader()->setVisible(true);
		view->verticalHeader()->setVisible(true);
		view->setAlternatingRowColors(true);
		view->show();
	}

	view->reset();
	//view->refresh();
}

void ParamsPage::on_action(const std::string& s)
{
	if (s == "PARAMS")
	{
		createModelView();
	}
	else
		TxtParametersGUI::on_action(s);
}

void ParamsPage::AddAvailableActions(std::vector<std::string>* v)
{
	TxtParametersGUI::AddAvailableActions(v);
	v->push_back("PARAMS");
}

ExperimentPage::ExperimentPage(const std::string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	FPGA_GUI(sPageName, pSheet, page_id),
	ExperimentBase(sPageName),
	m_pBeAlSheet(dynamic_cast<MgAlExperimentsSheet*>(pSheet))
//run_button("Run", &m_TxtParams, "false", &m_vParameters, false, false),
//pause_button("Pause", &m_TxtParams, "false", &m_vParameters, false, false)
{
//	RemoveParameter(&pause_button);
}



void ExperimentPage::AddAvailableActions(std::vector<std::string>* v)
{
	FPGA_GUI::AddAvailableActions(v);

	if (m_pSheet->scan_scheduler.IsRunning(this, 0))
		v->push_back("STOP");
	else
		v->push_back("RUN");
}

void ExperimentPage::on_action(const std::string& s)
{
	if (s == "RUN")
		m_pSheet->scan_scheduler.StartExperiment(this, 0);

	if (s == "STOP")
	{
		OnStop();
		m_pSheet->scan_scheduler.FinishExperiment(this, 0);
	}

	FPGA_GUI::on_action(s);
}

bool ExperimentPage::RecalculateParameters()
{
//	m_pBeAlSheet->UpdateButtons();
	return ParamsPage::RecalculateParameters();
}
