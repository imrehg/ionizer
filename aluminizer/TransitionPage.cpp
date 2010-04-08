#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ExpAl.h"
#include "TransitionPage.h"
#include "ExpSCAN.h"


using namespace std;
using namespace physics;
using namespace numerics;

TransitionPage::TransitionPage(const std::string& sPageName,
                               ExperimentsSheet* pSheet,
                               unsigned page_id,
                               physics::HFS* g,
                               physics::HFS* e,
                               const std::string& xname,
                               int NumModes,
                               unsigned NumPorts) :
	CalibrationPage(sPageName, pSheet, page_id),
	NumNormalModes(NumModes),
	tx(g, e, xname, NumModes),
	AOMdelay("AOM delay", &m_TxtParams, "0", &m_vParameters),
	CalculateFreqs("Calculate frequencies", &m_TxtParams, "false", &m_vParameters),
	MagneticField("B field", &m_TxtParams, "", &m_vParameters),
	F0("f0", &m_TxtParams, "", &m_vParameters),
	GroundState("Ground state", &m_TxtParams, tx.g->mFmin(), tx.g->mFmax(), &m_vParameters),
	Polarization("Polarization", &m_TxtParams, "0", &m_vParameters),
	NumCalibrationModes("Calibrate modes [#]", &m_TxtParams, "2", &m_vParameters),
	current_mFg(0),
	NumPorts(NumPorts),
	motion(0)
{
	QObject::connect(this, SIGNAL(sigSelectScanTarget(const calibration_item *, bool)),
	                 this, SLOT(selectScanTarget(const calibration_item *, bool)),
	                 Qt::BlockingQueuedConnection);


	Polarization.SetValue(1);
	Polarization.SetReadOnly(true);

	GroundState.SetValue(-3);
	GroundState.SetReadOnly(true);

	MagneticField.setRange(-100, 100);
	F0.setFlag(RP_FLAG_NOPACK, true);
	F0.setRange(-2000, 2000);

	AOMdelay.setSuffix(" us");

	F0.setPrecision(9);
	F0.setSuffix(" MHz");

	MagneticField.setSuffix(" Gauss");
	MagneticField.setPrecision(6);

	CalculateFreqs.SetValue(false);

	GroundState.setRange(tx.g->mFmin(), tx.g->mFmax());
}

TransitionPage::~TransitionPage()
{
}

void TransitionPage::AddParams()
{
	FPGA_GUI::AddParams();

	motion = dynamic_cast<GUI_matrix*>(FindParameter("motion"));

	if (motion == 0 && NumNormalModes > 0)
	{
		cerr << GetName() << "::AddParams FPGA has no motion table." << endl;
		throw runtime_error("[TransitionPage::AddParams] FPGA has no motion table.");
	}

	if (motion)
	{
		if (NumNormalModes != motion->Value().nc)
		{
			cerr << GetName() << "::AddParams FPGA and PC have unequal number of motional modes." << endl;
			throw runtime_error("[TransitionPage::AddParams] FPGA and PC have unequal number of motional modes");
		}
	}

	for (int sideband = NumNormalModes; sideband >= -NumNormalModes; sideband--)
	{
		string sName = GetPulseName(tx.stretchedMinus(sideband));

		GUI_dds* p = dynamic_cast<GUI_dds*>(FindParameter(sName));

		if (p)
		{
			p->setPrecision(9);
			m_vSBParams.push_back(p);
		}
	}
}

void TransitionPage::UpdatePolarizationGUI(GUI_pol* p, physics::ElectronicTransition* t, double mFg)
{
	if (p && t)
		p->setRange((int)std::max(-1., t->e->mFmin() - mFg), (int)std::min(1., t->e->mFmax() - mFg));
}

void TransitionPage::UpdateGUIPulseNames(double mFg, double mFe)
{
	unsigned i = 0;

	for (int sideband = NumNormalModes; sideband >= -NumNormalModes; sideband--)
	{
		m_vSBParams.at(i)->SetName(GetPulseName(line(mFg, mFe, sideband)));

		i++;
	}
}

void TransitionPage::AddAvailableActions(std::vector<std::string>* v)
{
	ExperimentPage::AddAvailableActions(v);
	v->push_back("CALC PI-TIMES");
}

void TransitionPage::on_action(const std::string& s)
{
	if (s == "CALC PI-TIMES")
	{
		CalculateParameters(false, true);
		PostUpdateData();
	}

	ExperimentPage::on_action(s);

//	else
//		ExperimentPage::on_action(s);
}

void TransitionPage::SetB(double B)
{
	MagneticField.SetValue(B);
}


bool TransitionPage::RecalculateParameters()
{
	debugQ("[TransitionPage::RecalculateParameters]", Title());

	bool Changed = false;

	double mFg = GetGroundState();
	UpdatePolarizationGUI(&Polarization, &tx, mFg);

	if (motion)
		for (int sb = 1; sb <= NumNormalModes; sb++)
			tx.SetModeLambDicke(motion->element(1, sb - 1), sb); // ModeLambDickes.at(sb-1)->Value(), sb);

	if (CalculateFreqs)
	{
		tx.SetF0(F0 * 1e6);

		for (int sideband = 1; sideband <= NumNormalModes; sideband++)
		{
			try
			{
				if ( GetModeFrequency(sideband) )
					tx.SetModeFrequency( GetModeFrequency(sideband), sideband);
			}
			catch (Uninitialized u )
			{
				cout << u << endl;
			}

			//      motrion.element(sideband-1)->SetReadOnly( false );
		}

		Changed |= CalculateParameters(true, false);
	}
	else
	{
		double B = CalibrateMagneticField();
		double f0 = tx.GetF0() * 1e-6;

		if (F0.IsInitialized())
		{
			double delta1 = fabs(F0 - f0);
			double delta2 = fabs(1 - F0 / std::max<double>(f0, 1.0));

			if (delta1 > 1e-14 && delta2 > 1e-14)
				Changed |= F0.SetValue(f0);
		}
		else
			Changed |= F0.SetValue(f0);

		if (MagneticField.IsInitialized())
		{
			double delta = fabs((B - MagneticField) / B);

			if (delta > 1e-9)
				Changed |= MagneticField.SetValue(B);
		}
		else
			Changed |= MagneticField.SetValue(B);

		//calibrate normal mode frequencies
		for (int sideband = 1; sideband <= NumNormalModes; sideband++)
		{
			if ( double fMode = GetModeFrequency(sideband) )
				tx.SetModeFrequency(fMode, sideband);
				//	Changed |= SetModeFrequency(fMode, sideband);

			//ModeFreqs.at(sideband-1)->SetReadOnly( true );
		}
	}

	MagneticField.SetReadOnly( !CalculateFreqs );
	F0.SetReadOnly( !CalculateFreqs );

//	if(CalculateAll)
//		Changed |= CalculateAll.SetValue(false);


	return Changed;
}

double TransitionPage::CalculateModeFrequency(int sb)
{
	double fBSB = AOMFrequency(line(tx.g->mFmin(), tx.e->mFmin(), ::abs(sb)), false);
	double fRSB = AOMFrequency(line(tx.g->mFmin(), tx.e->mFmin(), -::abs(sb)), false);

	return (fBSB - fRSB) / 2;
}


bool TransitionPage::CalculateParameters(bool bCalcFreqs, bool bCalcTimes)
{
	bool Changed = false;

	for (double mFg = tx.g->mFmin(); mFg <= tx.g->mFmax(); ++mFg)
	{
		for (double mFe = mFg - 1; mFe <= mFg + 1; ++mFe)
		{
			if (!tx.IsTransitionLegal(mFg, mFe))
				continue;

			for (int sideband = -NumNormalModes; sideband <= NumNormalModes; sideband++)
			{
				bool calculate = bCalcFreqs;

				line l(mFg, mFe, sideband);

				if ( calculate )
				{
					try
					{
						double f = AOMFrequency(l, true);
						Changed |= SetAOMFrequency(f, l);

						GUI_dds* pDDS = GetPulseParam(l);

						if (Changed && pDDS)
							pDDS->UpdateGUI_Value();
					}
					catch (Uninitialized u)
					{
						cout << u << endl;
					}
					catch (physics::ElectronicTransition::IllegalTransition it)
					{
						cout << it << endl;
					}
				}


				//try to calculate all pi times that are 0
				calculate = bCalcTimes && ShouldCalcPiTime(l);


				if ( calculate )
				{
					try
					{
						double t = PiTime(l, true);
						Changed |= SetPiTime(t, l, false);
					}
					catch (Uninitialized u)
					{
						cout << u << endl;
						Changed |= SetPiTime(1, l, false);
					}
					catch (physics::ElectronicTransition::IllegalPiTimeRatio ip)
					{
						cout << ip << endl;
					}
					catch (physics::ElectronicTransition::IllegalTransition it)
					{
						cout << it << endl;
					}
				}
			}
		}
	}

	return Changed;
}

double TransitionPage::PiTimePowerFactor(const std::string&)
{
	return 1;
}

double TransitionPage::PiTime(const physics::line& l, bool bAlwaysCalculate)
{
	string name = GetPulseName(l);

	if (!tx.IsTransitionLegal(l))
		throw ElectronicTransition::IllegalTransition(name);

	double t = 0;

	if (!bAlwaysCalculate)
	{
		//first check for the pi time in the map
		if (GUI_dds * pIP = dynamic_cast< GUI_dds* >(FindParameter(name)))
			if (pIP->IsInitialized())
				return pIP->Value().t;

		if (m_TxtParams.GetValue(name).length())
			if ((t = from_string<dds_pulse_info>(m_TxtParams.GetValue(name)).t))
				return t;
	}

	//if that doesn't exist calculate it from the reference state and the C-G coefficients
	double mFg_ref = ReferenceStateG(l);
	double mFe_ref = ReferenceStateE(l);

	line l_ref(mFg_ref, mFe_ref, 0);
	name = GetPulseName(l_ref);

	if (InputParameter<dds_pulse_info>* pIP = dynamic_cast< InputParameter<dds_pulse_info>* >(FindParameter(name)))
		t = pIP->Value().t;
	else
		t = from_string<dds_pulse_info>(m_TxtParams.GetValue(name)).t;

	if (!t)
		throw Uninitialized(name);

	return t * ::fabs(tx.PiTimeRatio(l, l_ref));
}

double TransitionPage::AOMFrequencyFourierLimit(double PiTime)
{
	return 1 / fabs(PiTime);
}

double TransitionPage::FrequencySpan()
{
	return tx.Frequency(tx.stretchedPlus(), MagneticField) -
	       tx.Frequency(tx.stretchedMinus(), MagneticField);
}

double TransitionPage::FrequencyCenter()
{
	return (AOMFrequency(tx.stretchedPlus()) +
	        AOMFrequency(tx.stretchedMinus())) / 2;
}


template<class T> bool TransitionPage::LinkPulseTo(T* p, const line& l)
{
	if (!p) throw(runtime_error("[TransitionPage::LinkPulseTo] p = 0."));

	if (pGlobals->GetIonXtal() != GetName())
		return false;

	if (p->getFlags() & RP_FLAG_NOLINK)
		return false;

	string name = GetPulseName(l);
	T* pSB = dynamic_cast<T*>(FindParameter(name));

	if (pSB)
		return p->LinkTo(pSB);
	else
		return p->LinkTo(&m_TxtParams, name);
}

template bool TransitionPage::LinkPulseTo(GUI_ttl*, const line&);
template bool TransitionPage::LinkPulseTo(GUI_dds*, const line&);

template<class T> bool TransitionPage::LinkPulseTo(T* p, const string& pname)
{
	if (!p) throw(runtime_error("[TransitionPage::LinkPulseTo] p = 0."));

	if (!CanLink(pname) || (p->getFlags() & RP_FLAG_NOLINK) )
		return false;

	T* pSB = dynamic_cast<T*>(FindParameter(pname));

	if (pSB)
		return p->LinkTo(pSB);
	else
		return p->LinkTo(&m_TxtParams, pname);
}

template bool TransitionPage::LinkPulseTo(GUI_ttl*, const string&);
template bool TransitionPage::LinkPulseTo(GUI_dds*, const string&);
template bool TransitionPage::LinkPulseTo(GUI_double*, const string&);

double TransitionPage::AOMFrequency(const physics::line& l, bool AlwaysCalculate)
{
	if (!AlwaysCalculate)
	{
		string name = GetPulseName(l);
		GUI_dds* pIP = dynamic_cast<GUI_dds*>(FindParameter(name));

		double value = 0;

		if (pIP)
		{
			if (pIP->IsInitialized())
				value = pIP->Value().fOn;
		}
		else
			value = from_string<dds_pulse_info>(m_TxtParams.GetValue(name)).fOn;

		if (value)
			return value * 1e6;
	}
	else
	{
		double carrier_Stark_Shift = 0;
		double f0 = tx.Frequency(l, MagneticField) ;

		//estimate stark shifts due to carrier
		if (l.sb != 0)
		{
			physics::line l0 = l;
			l0.sb = 0;
			double tPi = PiTime(l0);

			if (tPi > 0)
			{
				double detuning = std::max<double>(1e5, GetModeFrequency(abs(l.sb)));
				double carrier_Rabi_Rate = 0.25e6 / tPi;
				carrier_Stark_Shift = 2 * pow(carrier_Rabi_Rate, 2) / detuning;

				if (l.sb > 0)
					carrier_Stark_Shift *= -1;
			}
		}

		return f0 + carrier_Stark_Shift;
	}

	return 0;
}


//Transition frequency for mFg --> mFe in AOM Hz (Hz scaled by Fmultiplier)
bool TransitionPage::SetAOMFrequency(double f, const physics::line& l)
{
	string name = GetPulseName(l);
	dds_pulse_info i = m_TxtParams.GetValue(name);

	//ignore sub-mHz changes
	double delta = fabs(i.fOn * 1e6 - f);

	if (delta > 1e-3)
	{
		i.fOn = f * 1e-6;
		m_TxtParams.UpdatePair( name, to_string<dds_pulse_info>(i), -1);
		return true;
	}
	else
		return false;
}

bool TransitionPage::SetPiTime(double t, const physics::line& l, bool)
{
	string name = GetPulseName(l);
	dds_pulse_info i = m_TxtParams.GetValue(name);

	i.t = t;
	return m_TxtParams.UpdatePair( name, to_string<dds_pulse_info>(i), -1);
}


//Mode frequency in Hz
bool TransitionPage::SetModeFrequency(double f, int delta_n)
{
	return motion->SetElement(0, delta_n - 1, f * 1e-6);
}

//Mode frequency in Hz
double TransitionPage::GetModeFrequency(int delta_n)
{
	return motion->element(0, delta_n - 1) * 1e6;
}

std::string TransitionPage::GetPulseName(const physics::line& l)
{
//	if(l.pulse == "default" || l.pulse == "")
//		l.pulse = GetDefaultPulse(l);

	return tx.TransitionName(l);
}

GUI_dds* TransitionPage::GetPulseParam(const physics::line& l)
{
	string name = GetPulseName(l);
	GUI_dds* pGUI = dynamic_cast< GUI_dds* >(FindParameter(GetPulseName(l)));

	return pGUI;
}

GUI_dds* TransitionPage::GetPulseParam(const string& name)
{
	return dynamic_cast< GUI_dds* >(FindParameter(name));
}

double TransitionPage::GetGroundState()
{
	return GroundState.Value();
}

double TransitionPage::GetExcitedState()
{
	return GetGroundState() + static_cast<int>(Polarization.Value());
}


//rescale all pi-times by f
void TransitionPage::ScalePiTimes(double f)
{
	cout << Title() << "::ScalePiTimes " << setprecision(6) << f << endl;

	//scale all UV pi-times for all pulses
	for (size_t iPulse = 0; iPulse < pulse_names.size(); iPulse++)
	{
		for (int sideband = 2; sideband >= 0; sideband--)
			for (double mFg = tx.g->mFmin(); mFg <= tx.g->mFmax(); ++mFg)
				for (double mFe = mFg - 1; mFe <= mFg + 1; ++mFe)
				{
					line l(mFg, mFe, sideband);
					if (tx.IsTransitionLegal(l))
					{
						try
						{
							double t = PiTime(l);
							SetPiTime(t * f,  l, false);
						}
						catch (Uninitialized)
						{}
					}
				}
	}

	//recalculate all parameters
	PostUpdateData();
}

void TransitionPage::ShiftAllFrequencies(double delta_f)
{
	//shift the UV AOM frequencies, units for delta_f are Hz on the AOM
	for (int sideband = NormalModes(); sideband >= -NormalModes(); sideband--)
	{
		//if motional frequencies are fixed, only shift carriers
		if (sideband == 0)
		{
			for (double mFg = mFg_min(); mFg <= mFg_max(); ++mFg)
				for (double mFe = mFg - 1; mFe <= mFg + 1; ++mFe)
				{
					line l(mFg, mFe, sideband);
					if (tx.IsTransitionLegal(l))
					{
						double f = AOMFrequency(l, false);
						SetAOMFrequency(f + delta_f, l);
					}
				}
		}
	}
}

//convert name to transition info
physics::line TransitionPage::getLine(const std::string& s)
{
	//invert this:
	//snprintf(sbuff, 63, "%s %s %s mFg = %s%d%s", base_name.c_str(),sbName(sb).c_str(), polName(pol), sp, mFg2, sHalf);

	physics::line l(s);

	return l;
}
void TransitionPage::slot_cal_t(Pulse_Widget* p)
{
	calibrations.clear();
	physics::line l = getLine(p->getName());
	calibrations.push_back(cal_item(l, "Time", "", GetPulseParam(l)));

	on_action("RUN");
}

void TransitionPage::slot_cal_f(Pulse_Widget* p)
{
	calibrations.clear();
	physics::line l = getLine(p->getName());
	calibrations.push_back(cal_item(l, "Frequency", "", GetPulseParam(l)));

	on_action("RUN");
}

ExpSCAN* TransitionPage::getCalibrationExp(const calibration_item&)
{
	return 0;
}

ExperimentBase::run_status TransitionPage::Run()
{
	if (!calibrations.empty())
	{
		calibration_item& ci = calibrations.front();

		ExpSCAN* cal = getCalibrationExp(ci);

		if (!cal)
			throw runtime_error("[TransitionPage::Run] Unable to find calibration experiment");

		//    pGlobals->SetIonXtal(Title());

		cal->PostUpdateData();

		emit sigSelectScanTarget(&ci, true);
		cal->DoCalibration(&ci, pRunObject);

		PostUpdateData();

		return OK;
	}
	else
		return FINISHED;
}

void TransitionPage::PostCreateGUI()
{

	for (size_t i = 0; i < m_vParameters.size(); i++)
	{
		if (GUI_dds * p = dynamic_cast<GUI_dds*>(m_vParameters[i]))
			p->input->setTransitionPage(this);

		if (GUI_ttl * p = dynamic_cast<GUI_ttl*>(m_vParameters[i]))
			p->input->setTransitionPage(this);
	}
}


void TransitionPage::selectScanTarget(const calibration_item* ci, bool b)
{
	if (ci)
		if (ci->getParamGUI())
			ci->getParamGUI()->selectScanTarget(ci->scan_type, b);
}


void TransitionPage::DidCalibration(const calibration_item* ci, numerics::FitObject* pFit)
{
	emit sigSelectScanTarget(ci, false);  //crashes here when pressing stop during Mg+ calibration

	if (pFit->GoodFit())
	{
		numerics::TimeFitObject* tf = dynamic_cast<numerics::TimeFitObject*>(pFit);

		if (tf && ci->IsTimeScan())
		{
			SetPiTime(tf->GetPiTime(), ci->l);

			//calculate pi-times if this was the carrier
			//      if(ci->l.sb == 0)
			//         CalculateParameters(false, true);

			PostUpdateData();
		}

		numerics::PeakFitObject* ff = dynamic_cast<numerics::PeakFitObject*>(pFit);

		if (ff && ci->IsFreqScan())
		{
			SetAOMFrequency(ff->GetCenter() * 1e6, ci->l);
			PostUpdateData();
		}
	}

	calibrations.pop_front();
}
