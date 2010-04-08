#include "ExpMM.h"
#include "MotionPage.h"
#include "FPGA_connection.h"
#include "AluminizerApp.h"
#include "Bfield.h"

class ExpZeroB : public ExpSCAN_Detect
{
public:

ExpZeroB(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id) :
	ExpSCAN_Detect(sPageName, pSheet, exp_id),
	pB(0),
	iB(name2index(sPageName))
{
}

virtual ~ExpZeroB()
{
}

//LockInParams overrides
virtual void modulateOutput(double d)
{
	SetB(GetCenter() + d);
}
virtual double GetOutput()
{
	return GetB();
}

static bool matchTitle(const std::string& s)
{
	return s.find("Zero B") != string::npos;
}

protected:
void SetB(double d)
{
	GetBfieldPage(); pB->SetB0(iB, d);
}
double GetB()
{
	GetBfieldPage(); return pB->GetB0(iB);
}

void GetBfieldPage()
{
	if (!pB)
		pB = dynamic_cast<Bfield*>(m_pSheet->FindPage("B-field"));

	if (!pB)
		throw std::runtime_error("No B-field page.");
}

//setup apparatus for this experiment (set B=0, remember old B)
virtual void InitExperimentSwitch()
{
	GetBfieldPage();
	Bold = pB->GetB(iB);
	pB->SetB(iB, 0);
	ExpSCAN_Detect::InitExperimentSwitch();
}

//put voltages etc. back to nominal values so other experiments work (set B back to old B)
virtual void DefaultExperimentState()
{
	GetBfieldPage();
	pB->SetB(iB, Bold);

	ExpSCAN_Detect::DefaultExperimentState();
}

unsigned name2index(const std::string& s)
{
	if (s.find("Bx") != string::npos) return 0;
	else if (s.find("By") != string::npos) return 1;
	else return 2;
}

double Bold;

Bfield* pB;
unsigned iB;
};

