#pragma once

#include "ExpSCAN_Be.h"
#include "ExpBuzz.h"

/*  Specialization of FastLock to servo the trap frequency to a nominal value.
   The lock always probes at the nominal frequency and works by shifting the endcaps mean. */

class ExpLockTrapFreq : public ExpLockIn< ExpBuzz<ExpSCAN_Be> >
{
public:
ExpLockTrapFreq(const string& sPageName, ExperimentsSheet* pSheet) :
	ExpLockIn< ExpBuzz<ExpSCAN_Be> >(sPageName, pSheet, "[V]", "EC"),
	AdjustEndcaps("Adjust endcaps",  &m_TxtParams, "false", &m_vParameters),
	VecVdacRatio("Vec/Vdac",  &m_TxtParams, "0.095", &m_vParameters, false),
	Vec("Vec [V]",  &m_TxtParams, "11", &m_vParameters, false),
	dVdFratio("dV/dF [V/MHz]",  &m_TxtParams, "0", &m_vParameters, true)
{
}

virtual ~ExpLockTrapFreq()
{
}

virtual void InitExperimentSwitch()
{
	SetCenter(pVoltages->GetVoltage(1));
	ExpLockIn< ExpBuzz<ExpSCAN_Be> >::InitExperimentSwitch();
}

virtual bool RecalculateParameters()
{
	bool bChanged = false;

	if (AdjustEndcaps.Value())
	{
		bChanged = true;

		AdjustEndcaps.SetValue(false);

		SetOutput(GetOutput() + (FCenter.Value() - FCenterFit.Value()) * dVdFratio );

		FCenterFit.SetValue(FCenter.Value());
	}

	SetCenter(GetOutput());

	bChanged |= dVdFratio.SetValue(2 * Vec / (VecVdacRatio * FCenter) );

	bChanged |= ExpLockIn< ExpBuzz<ExpSCAN_Be> >::RecalculateParameters();

	return bChanged;
}

protected:
virtual std::string GetFCenterName()
{
	switch (abs(Sideband.Value()))
	{
	case 1: return pGlobals->GetIonXtal() + " Nominal COM [MHz]";
	case 2: return pGlobals->GetIonXtal() + " Nominal Stretch [MHz]";
	default: return ExpLockIn< ExpBuzz<ExpSCAN_Be> >::GetFCenterName();
	}
}

//LockInParams overrides
virtual void   SetOutput(double d)
{
	pVoltages->SetVoltage(1, d);
}
virtual double GetOutput()
{
	return pVoltages->GetVoltage(1);
}

virtual bool calc_modulation()
{
	return false;
}

protected:
GUI_double VecVdacRatio;
GUI_double Vec;
GUI_double dVdFratio;

GUI_bool AdjustEndcaps;

};



