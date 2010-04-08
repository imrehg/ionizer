#include "CalibrationPage.h"

using namespace std;

CalibrationPage::CalibrationPage(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	ExperimentPage(sPageName, pSheet, page_id),
	NumScans("Scans", &m_TxtParams, "10", &m_vParameters),
	NumPoints("Points", &m_TxtParams, "20", &m_vParameters),
	NumFlops("Flops", &m_TxtParams, "6", &m_vParameters)
{
	NumFlops.setRange(0, 1e4);
	NumFlops.setIncrement(0.001);
}

calibration_item CalibrationPage::cal_item(const physics::line& l,
                                           const std::string& scan_type,
                                           const std::string& options,
                                           GUI_dds* pGUI)
{
	calibration_item ci(l, scan_type, "experiment pulse", options, this, pGUI, NumScans, NumPoints, NumFlops);

	if (ci.IsFreqScan())
		ci.scan_variable = ci.scan_variable + "(F)";

	if (ci.IsTimeScan())
		ci.scan_variable = ci.scan_variable + "(T)";

	return ci;
}
