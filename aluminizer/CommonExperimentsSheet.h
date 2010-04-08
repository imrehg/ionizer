#pragma once

#include "ExperimentsSheet.h"

#include "GlobalsPage.h"
#include "Bfield.h"
#include "DDSPage.h"

#ifdef CONFIG_AL
#include "RefCavityPage.h"
#endif

#include "MotorsPage.h"
#include "AgilisMotors.h"
#include "MotionPage.h"
#include "about.h"
#include "AlignmentPage.h"

/* compiler generates a screwy warning so turn it off temporarily... */
//#pragma warning(push)
//#pragma warning(disable : 4355)

class CommonExperimentsSheet  : public ExperimentsSheet
{
public:

CommonExperimentsSheet() :
	m_Switches(this),
	m_AboutPage(this),
	m_GlobalsPage("Globals", this),

#ifdef CONFIG_AL
	m_ElsnerPage(this),
	m_ULE88Page(this),
#endif

	m_Bfield("B-field", this),
	m_DDSPage("DDS", this),
	m_MotorsPage("Motors", this),
	m_AgilisMotorsPage("Agilis", this),
	m_AlignMgPage("Align Mg+", this),
	m_AlignBfieldPage("Align B", this)
{
	//	m_GlobalsPage.UpdateData();
}

virtual ~CommonExperimentsSheet()
{
}
virtual void PrepareCloseWindow()
{
	m_GlobalsPage.RememberWindowPos();
}

SwitchPanel m_Switches;
AboutPage m_AboutPage;
GlobalsPage m_GlobalsPage;

#ifdef CONFIG_AL
ElsnerPage m_ElsnerPage;
ULE88Page m_ULE88Page;
#endif

Bfield m_Bfield;

DDSPage m_DDSPage;
MotorsPage m_MotorsPage;
AgilisMotorsPage m_AgilisMotorsPage;
AlignMirrorsPage m_AlignMgPage;
AlignBfieldPage m_AlignBfieldPage;
};


//#pragma warning(pop)

