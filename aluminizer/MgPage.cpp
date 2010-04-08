#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "MgPage.h"
#include "HFS_Mg.h"
#include "MgAlExperimentsSheet.h"
#include "ExpSCAN_NewFPGA.h"
#include "ExpAl.h"

MgPage* ExpAl::pMg;

using namespace std;
using namespace physics;
using namespace numerics;

unsigned numModes(const string& sPageName)
{
	if(sPageName == "Mg")
		return 3;
	else
		return 6;
}

MgPage::MgPage(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	TransitionPage(sPageName, pSheet, page_id,
	               (new HFS_Mg25_II_S_One_Half(3)),
	               (new HFS_Mg25_II_S_One_Half(2)), "Mg", numModes(sPageName)),
	Detect("dds=0 t=100 fOn=221 fOff=190 sb=-999 name=Mg Detect", &m_TxtParams, &m_vParameters),
	DopplerCool("dds=0 t=100 fOn=218 fOff=190 sb=-999 name=Mg Doppler cool", &m_TxtParams, &m_vParameters),
	Precool("dds=0 t=100 fOn=221 fOff=0 sb=-999 name=Mg Precool", &m_TxtParams, &m_vParameters),
	Repump("t=100 name=Mg Repump", &m_TxtParams, &m_vParameters)
{
	if (pGlobals->GetIonXtal() == GetName() )
		ExpAl::pMg = this;

	pulse_names.push_back("raman");
	F0.SetValue( (-3 * HFS_Mg25_II::Mg25_2S12Ahfs) * 1e-6 );
	tx.SetF0(F0 * 1e6);
}

double MgPage::AOMFrequency(int delta_n, bool AlwaysCalculate)
{
	return TransitionPage::AOMFrequency(tx.stretchedMinus(delta_n), AlwaysCalculate);
}

double MgPage::PiTime(int delta_n, bool)
{
	line l = tx.stretchedMinus(delta_n);

	return TransitionPage::PiTime(l);
}

double MgPage::CalibrateMagneticField()
{
	if (AOMFrequency(0) > 0 && AOMFrequency(0) < 2e9)
		return tx.CalibrateFrequency(tx.stretchedMinus(), AOMFrequency(0), F0 * 1e6);
	else
		return 0;
}

void MgPage::InitExperimentStart()
{
	if (calibrations.empty())
	{
		//fill calibrations vector
		string calibration_type = "FULL";
		line l(mFg_min(), mFe_min(), 0);

		if (calibration_type == "FULL")
		{
			//first calibrate qubit carrier
			calibrations.push_back( cal_item(l, "Frequency", "", GetPulseParam(l)) );
			calibrations.push_back( cal_item(l, "Time", "", GetPulseParam(l)) );

			//calibrate repump
			//		calibrations.push_back( cal_item(l, "NoGSC CalRepump") );

			//calibrate CoCarrier
			l.mFg = l.mFe;
			calibrations.push_back( cal_item(l, "Frequency", "", GetPulseParam(l)) );
			calibrations.push_back( cal_item(l, "Time", "", GetPulseParam(l)) );


			l.mFg = mFg_min();
		}

		//calibrate qubit motional sidebands
		for (l.sb = -1; l.sb >= -NumCalibrationModes; l.sb = l.sb - 1)
			if (l.sb != 0)
				calibrations.push_back( cal_item(l, "Frequency", "", GetPulseParam(l)) );

		for (l.sb = -1 * NumCalibrationModes; l.sb < 0; l.sb = l.sb + 1)
			calibrations.push_back( cal_item(l, "Time", "", GetPulseParam(l)) );
	}

}


bool MgPage::RecalculateParameters()
{
	bool Changed = false;

	try {
		tx.SetPiTime(PiTime(0));

		//	for(int sideband = NumNormalModes; sideband > 0; sideband--)
		//		tx.CalibratePiTimeRatio(tx.ModeName(sideband), PiTime(sideband));
	}

	catch (Uninitialized) {}

	Changed |= TransitionPage::RecalculateParameters();

	return Changed;
}

double MgPage::ReferenceStateG(const physics::line& l)
{
	if (l.mFg == l.mFe)
		return -2;
	else
		return mFg_min();
}

double MgPage::ReferenceStateE(const physics::line& l)
{
	if (l.mFg == l.mFe)
		return ReferenceStateG(l);
	else
		return mFe_min();
}

void MgPage::ShiftLine(const physics::line& l, double delta_f, int)
{
	//for now just shift the transition frequency by f
	SetAOMFrequency(TransitionPage::AOMFrequency(l) + delta_f, l);
}

double MgPage::ModeAmplitude(int sb)
{
	//TODO: check what this does.  seems incorrect for Mg/Al
	if (sb == 1)
	{
		double omegaZ = GetModeFrequency(sb) * 2 * M_PI;
		return sqrt(hbar / (2 * mass() * omegaZ) );
	}
	else
		throw runtime_error("[MgPage::ModeAmplitude] bad mode number");
}


bool MgPage::CanLink(const string& pname)
{
	if (pGlobals->GetIonXtal() != GetName())
		return false;
	else
		return pname.find("Mg") != string::npos;
}

ExpSCAN* MgPage::getCalibrationExp(const calibration_item&)
{
	return dynamic_cast<ExpSCAN*>( m_pSheet->FindExperiment("Qubit cal 1") );
}
