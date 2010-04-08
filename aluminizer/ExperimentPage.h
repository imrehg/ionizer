#pragma once

#include "ExperimentsSheet.h"
#include "AluminizerPage.h"
#include "Experiment.h"
#include "FPGA_GUI.h"

class GlobalsPage;
class Voltages2;
class SwitchPanel;
class FPGA_connection;
class MotionPage;

//need this forward declaration
class MgAlExperimentsSheet;


class ExperimentPage : public FPGA_GUI, public ExperimentBase
{
public:
ExperimentPage(const std::string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);
virtual ~ExperimentPage()
{
};

virtual void AddAvailableActions(std::vector<std::string>*);
virtual void on_action(const std::string& s);

virtual bool RecalculateParameters();

virtual void PostInitialize()
{
	emit sig_started(iSheetPage);

	if (!ScanScheduler::IsShuttingDown())
		PostUpdateActions();
}

virtual void PostFinish()
{
	emit sig_finished(iSheetPage);

	if (!ScanScheduler::IsShuttingDown())
		PostUpdateActions();
}

virtual void OnStop()
{
}
public:
MgAlExperimentsSheet*   m_pBeAlSheet;
static GlobalsPage*     pGlobals;
static Voltages2*     pVoltages;
static SwitchPanel*     pSwitches;
//	static MotionPage*		pMotion;
};
