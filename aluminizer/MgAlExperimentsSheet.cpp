#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "physics.h"
#include "HFS.h"
#include "HFS_Al.h"

#include "MgAlExperimentsSheet.h"

#include "Ovens.h"

#include "FPGA_page.h"

#include "Voltages2.h"
#include "MotionPage.h"
#include "MgPage.h"
#include "Al3P1Page.h"
#include "RefCavityPage.h"
#include "DopplerTempCalibration.h"
#include "ExpSCAN_NewFPGA.h"
#include "ExpMM.h"
#include "ExpZeroB.h"
#include "ExpLMS.h"
#include "ExpCorrelate.h"

#include "AluminizerApp.h"
#include "ExpSCAN_Al.h"

using namespace physics;

template<class T> void createPageIfTitleMatches(FPGA_GUI*& pGUI, const string& sTitle, ExperimentsSheet* pSheet, unsigned exp_id)
{
	if (pGUI ==  0)
		if (T::matchTitle(sTitle))
			pGUI = new T(sTitle, pSheet, exp_id);
}

//create the appropriate page from its title
FPGA_GUI* PageFactory(const string& sTitle, ExperimentsSheet* pSheet, unsigned exp_id)
{
	FPGA_GUI* pGUI = 0;

	createPageIfTitleMatches<MgPage>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<DACPage>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<Voltages2>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<Al3P1Page>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<Al3P0Page>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<FPGA_page>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpQubit>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpLoad>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpSCAN_Al>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpAl3P0>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpAl3P0_lock>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpZeroB>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpRF_lock>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpLMS>(pGUI, sTitle, pSheet, exp_id);
	createPageIfTitleMatches<ExpCorrelate>(pGUI, sTitle, pSheet, exp_id);

	if (pGUI == 0)
		return new ExpSCAN_Detect(sTitle, pSheet, exp_id);
	else
		return pGUI;
}

MgAlExperimentsSheet::MgAlExperimentsSheet()
//	m_ElsnerPage					(this)
//	m_ULE88Page						(this)

{
	//dynamically allocated pages
//	vPages.push_back(new ExpList							("List 1", this));

#ifdef CONFIG_AL
	vPages.push_back(new RunOvens("Ovens", this));
	vPages.push_back(new DopplerTempCalibration("RSB BSB", this));
#endif

	//add remotely defined pages
	unsigned nPages = theApp->fpga->NumExp();

	for (unsigned i = 0; i < nPages; i++)
	{
		string name = theApp->fpga->ExpName(i);
		vPages.push_back(PageFactory(name, this, i));
	}

//		ofstream pages_log(("pages_" + g_t0s + ".csv").c_str());

	for (size_t i = 0; i < pages.size(); i++)
//			pages_log << i << ", " << GetPage(i)->PageTitle() << endl;
		GetPage(i)->SetID(i);
}



MgAlExperimentsSheet::~MgAlExperimentsSheet()
{
	//release dynamically allocated pages
	while (!vPages.empty())
	{
		delete vPages.back();
		vPages.pop_back();
	}
}
