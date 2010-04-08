#ifdef PCH
#include "common.h"
#endif

#include "RefCavityPage.h"
#include "ExpSCAN_Be.h"
#include "ExpSCAN_Al.h"
#include "ExpSCAN_Al_3P0.h"
#include "ExpSCAN_Be_HFS.h"
#include "ExpSCAN_FastLock.h"

using namespace physics;
using namespace numerics;

template<class T> ExpSCAN_FastLock<T>::ExpSCAN_FastLock<T>(const std::string & sPageName, ExperimentsSheet * pSheet) :
T(sPageName, pSheet),
Modulation("Modulation [Hz]",        &m_TxtParams, "0", &m_vParameters),
Gain("Gain",             &m_TxtParams, "0", &m_vParameters),
IntegratorGain("I Gain",           &m_TxtParams, "0", &m_vParameters),
PointsToTake("Points to take",         &m_TxtParams, "2", &m_vParameters),
RunModulo("Run modulo",       &m_TxtParams, "1", &m_vParameters),

tLastPlotUpdate(0),
pSignalL(0),
pSignalM(0),
pSignalR(0),
pErrorChannel(0),
pFrequencyChannel(0){
	ScanType.AddChoices("Lock\n");
	ScanType.SetValue("Lock");
	RemoveParameter(&ScanType);

	RemoveParameter(&RamseyTime);
	RemoveParameter(&RamseyPulse);

	RemoveParameter(&ShelvingPulses);
	RemoveParameter(&ScanTime);
	RemoveParameter(&progress);
}

template ExpSCAN_FastLock<ExpSCAN_Be>::ExpSCAN_FastLock<ExpSCAN_Be>(const std::string &, ExperimentsSheet*);
template ExpSCAN_FastLock<ExpSCAN_Al>::ExpSCAN_FastLock<ExpSCAN_Al>(const std::string &, ExperimentsSheet*);
template ExpSCAN_FastLock<ExpSCAN_Al_3P0>::ExpSCAN_FastLock<ExpSCAN_Al_3P0>(const std::string &, ExperimentsSheet*);
template ExpSCAN_FastLock<ExpSCAN_Be_HFS>::ExpSCAN_FastLock<ExpSCAN_Be_HFS>(const std::string &, ExperimentsSheet*);