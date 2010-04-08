#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "MotionPage.h"

MotionPage::MotionPage(const std::string& title, ExperimentsSheet* pSheet) :
	ParamsPage(pSheet, title)
{
	int NumNormalModes = 3;

	for (int sideband = 1; sideband <= NumNormalModes; sideband++)
	{
		string name = ModeName(sideband) + " freq.";
		GUI_double* g = new GUI_double(name, &m_TxtParams, "0", &m_vParameters, false);
		g->setSuffix(" MHz");
		m_vAllocatedParams.push_back(g);

		name = ModeName(sideband) + " tHeat";
		g = new GUI_double(name, &m_TxtParams, "0", &m_vParameters, false);
		g->setSuffix(" us");

		m_vAllocatedParams.push_back(g);
	}

//	ExperimentPage::pMotion = this;
}

MotionPage::~MotionPage()
{
}

//Mode frequency parameter
GUI_double*  MotionPage::GetModeFrequencyParam(int delta_n)
{
	GUI_double* pIP = dynamic_cast<GUI_double*>(FindParameter(ModeName(delta_n) + " freq."));

	if (!pIP)
		throw runtime_error("[MotionPage::GetModeFrequencyParameter] non-existing parameter");

	return pIP;
}

//Mode frequency in Hz
bool MotionPage::SetModeFrequency(int delta_n, double f)
{
	return GetModeFrequencyParam(delta_n)->SetValue(f * 1e-6);
}

//Mode frequency in Hz
double MotionPage::GetModeFrequency(int delta_n)
{
	return (1e6) * (*GetModeFrequencyParam(delta_n));
}

//Mode heating time parameter
GUI_double*  MotionPage::GetModeHeatTimeParam(int delta_n)
{
	GUI_double* pIP = dynamic_cast<GUI_double*>(FindParameter(ModeName(delta_n) + " tHeat"));

	if (!pIP)
		throw runtime_error("[MotionPage::GetModeHeatTimeParam] non-existing parameter");

	return pIP;
}

//Mode heating time in us
bool MotionPage::SetModeHeatTime(int delta_n, double t)
{
	return GetModeHeatTimeParam(delta_n)->SetValue(t);
}

//Mode heating time in us
double MotionPage::GetModeHeatTime(int delta_n)
{
	return *GetModeHeatTimeParam(delta_n);
}

std::string MotionPage::ModeName(int sb)
{
	switch (sb)
	{
	case 0: return "";
	case 1: return "z";
	case 2: return "x";
	case 3: return "y";
	default: return "Mode" + to_string<int>(abs(sb));
	}
}

unsigned MotionPage::num_columns()
{
	return 4;
}
