#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ExperimentPage.h"
#include "MgAlExperimentsSheet.h"

using namespace std;


GlobalsPage*      ExperimentPage::pGlobals(0);
Voltages2*        ExperimentPage::pVoltages(0);
SwitchPanel*      ExperimentPage::pSwitches(0);
//MotionPage*			ExperimentPage::pMotion(0);

ParamsPage::ParamsPage(ExperimentsSheet* pSheet, const std::string& sPageName) :
	TxtParametersGUI(pSheet, sPageName),
	m_TxtParams(ParamsFileName())
{
}

std::string ParamsPage::ParamsFileName()
{
	string s = Title();

	replace(s.begin(), s.end(), '/', '_');

	return "params" + s + ".txt";
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
