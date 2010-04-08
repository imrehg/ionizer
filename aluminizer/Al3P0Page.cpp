#include "common.h"

#include "Al3P0Page.h"
#include "HFS_Al.h"
#include "RefCavityPage.h"

using namespace std;
using namespace physics;
using namespace numerics;

Al3P0Page::Al3P0Page(const std::string& sPageName,
                     ExperimentsSheet* pSheet,
                     auto_ptr<physics::HFS> g,
                     auto_ptr<physics::HFS> e) :
	Al3P1Page(sPageName, pSheet, g, e, 0),
{
	pRefCavity = dynamic_cast<RefCavityPage*>(m_pSheet->FindPage("Elsner"));
}

Al3P0Page::~Al3P0Page()
{
}


double Al3P0Page::ReferenceStateG(const physics::line& l)
{
	if (l.mFg == l.mFe)
		return -5 / 2.;
	else
		return mFg_min();
}

double Al3P0Page::CalculateModeFrequency(int sb)
{
	double fCarrier = AOMFrequency(line(mFg_min(), mFe_min(), 0), false) * AOMorder;
	double fRSB    = AOMFrequency(line(mFg_min(), mFe_min(), -::abs(sb)), false) * AOMorder;

	return fCarrier - fRSB;
}



