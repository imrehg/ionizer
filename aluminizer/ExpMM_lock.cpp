#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ExpMM.h"
#include "Voltages.h"
#include "MotionPage.h"
#include "FPGA_connection.h"
#include "AluminizerApp.h"


ExpMM_Lock::ExpMM_Lock(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id, const std::string& units) :
	ExpLockIn<ExpSCAN_Heat>(sPageName, pSheet, exp_id, units, GetAbbrev(sPageName)),
	Ei(GetEi(sPageName)),
	Mode("Mode (2=X, 3=Y)", &m_TxtParams, "2", &m_vParameters),
	pProbeTimeChannel(0),
	pEFieldChannel(0)
{
}

ExpMM_Lock::~ExpMM_Lock()
{
}

int ExpMM_Lock::ModeNum()
{
	return Mode.Value();
}


string ExpMM_Lock::GetAbbrev(const string& sPageName)
{
	if (sPageName.find("Ex") != string::npos) return "Ex";
	else if (sPageName.find("Ey") != string::npos) return "Ey";
	else if (sPageName.find("Eh") != string::npos) return "Eh";
	else if (sPageName.find("Ev") != string::npos) return "Ev";

	return "E";
}

unsigned ExpMM_Lock::GetEi(const string& sPageName)
{
	if (sPageName.find("Ex") != string::npos) return 0;
	else if (sPageName.find("Ey") != string::npos) return 1;
	else if (sPageName.find("Eh") != string::npos) return 4;
	else if (sPageName.find("Ev") != string::npos) return 5;

	return 99999;
}


DataChannel* ExpMM_Lock::AddDataChannels(DataFeed& data_feed)
{
	DataChannel* pSignal = ExpLockIn<ExpSCAN_Heat>::AddDataChannels(data_feed);

	data_feed.AddChannel(pProbeTimeChannel = new ConstantChannel("probe time", false));
	data_feed.AddChannel(pEFieldChannel = new ConstantChannel("E field", false));

	return pSignal;
}


void ExpMM_Lock::PostAcquireDataPoint(scans::Scan_Base* pScan, DataFeed& df)
{
	if (pDrive)
		pProbeTimeChannel->SetCurrentData( pDrive->Value().t );

	pEFieldChannel->SetCurrentData( GetOutput() );

	return ExpLockIn<ExpSCAN_Heat>::PostAcquireDataPoint(pScan, df);
}

ExpMM_Lock_Exy::ExpMM_Lock_Exy(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id) :
	ExpMM_Lock(sPageName, pSheet, exp_id, "[V/cm]"),
	Sensitivity("Sensitivity [V/cm * us]^-1", &m_TxtParams, "1", &m_vParameters),
	sans_modulation(0),
	bHasModulation(false)
{
}

ExpMM_Lock_Exy::~ExpMM_Lock_Exy()
{
}


bool ExpMM_Lock_Exy::RecalculateModulation()
{
	if (pDrive)
		if (pDrive->Value().t > 0)
			return Modulation.SetValue(0.7 / (pDrive->Value().t * Sensitivity) );

	return false;
}

void ExpMM_Lock_Exy::PreAcquireDataPoint(scans::Scan_Base*, DataFeed& )
{
	if (pDrive)
		if (pDrive->Value().t)
		{
			pDrive->setFreq(pMotion->GetModeFrequency(ModeNum()) * 1e-6);
			pDrive->PostUpdateGUI();
		}
}

//apply the appropriate modulation to achieve the given pi-time
void ExpMM_Lock_Exy::ApplyModulation(double sign, double t_pi)
{
	if (!bHasModulation)
	{
		double m = sign / (t_pi * Sensitivity);
		modulateOutput(m);
		bHasModulation = true;
	}
}

//remove the applied modulation
void ExpMM_Lock_Exy::RemoveModulation()
{
	if (bHasModulation)
	{
		modulateOutput( 0 );
		bHasModulation = false;
	}
}

void ExpMM_Lock_Exy::RespondToMeasurements(scans::LockInScan* pScan, const measurements_t& m, size_t i)
{
	ExpLockIn<ExpSCAN_Heat>::RespondToMeasurements(pScan, m, i);

	if (i + 1 == m.size())
	{
		const measurement& l = m.at(pScan->LeftIndex());
		const measurement& c = m.at(pScan->CenterIndex());
		const measurement& r = m.at(pScan->RightIndex());

		double amplitude = 2. * c.counts - (r.counts + l.counts);
		//double contrast = 3 * amplitude / ( c.control + l.control + r.control );
		double contrast = amplitude / std::max<double>(c.counts, 2);

		cout << "contrast = " << setprecision(4) << contrast << endl;

		int rm = RunModulo.Value();

		if (pDrive)
		{
			if (pDrive->Value().t > 0)
			{
				if (pDrive->Value().t < 60)
					rm = 1;

				if (contrast > 0.4)
				{
					pDrive->setTime( std::min(pDrive->Value().t * 1.05, 100.) );
					if (pDrive->Value().t > 50)
						rm = std::min(15, rm + 1);

				}
				else
				{
					pDrive->setTime( std::max(pDrive->Value().t * 0.93, 10.) );
					rm = std::max(1, rm - 1);
				}

				pMotion->SetModeHeatTime(ModeNum(), pDrive->getTime());

				pFPGA->SendParam(page_id, pDrive);

				SetRunModulo(rm);
				RecalculateModulation();
			}
		}

		PostUpdateData();
	}
}


ExpMM_Lock_Freq::ExpMM_Lock_Freq(const string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id) :
	ExpMM_Lock(sPageName, pSheet, exp_id, "[MHz]"),
	pMMlock(0)
{
}

ExpMM_Lock_Freq::~ExpMM_Lock_Freq()
{
}

/*
   bool ExpMM_Lock_Freq::RecalculateModulation()
   {
   if(pDrive)
      return Modulation.SetValue( 0.4 / pDrive->Value().t );
   else
      return false;
   }
 */

void ExpMM_Lock_Freq::InitExperimentSwitch()
{
	if (pDrive)
		pDrive->setTime(pMotion->GetModeHeatTime(ModeNum()));

	RecalculateModulation();

	if (pMMlock == 0)
	{
		//figure out the name of the field compensating lock page
		std::string sName = (Title().find("Ex") != std::string::npos ? "Ex" : "Ey") + string(" lock");
		pMMlock = dynamic_cast<ExpMM_Lock_Exy*> (m_pSheet->FindPage(sName));
	}

	//apply voltage modulation when locking the frequency so that the resonance is visible
	if (pMMlock && pDrive && scans::IsLockInScan(ScanType))
		pMMlock->ApplyModulation(1, pDrive->Value().t);

	return ExpMM_Lock::InitExperimentSwitch();
}

void ExpMM_Lock_Freq::DefaultExperimentState()
{
	if (pMMlock)
		pMMlock->RemoveModulation();

	return ExpMM_Lock::DefaultExperimentState();
}

// in MHz
void ExpMM_Lock_Freq::SetCenter(double d)
{
	Center.SetValue(d);
	Center.PostUpdateGUI();

	pMotion->SetModeFrequency(ModeNum(), d * 1e6);
}

// in MHz
double ExpMM_Lock_Freq::GetCenter()
{
	double f = pMotion->GetModeFrequency(ModeNum());

	return f * 1e-6;
}

double ExpMM_Lock_Freq::getFreq()
{
	return pDrive->getFreq();
}

// in MHz
void ExpMM_Lock_Freq::setFreq(double d)
{
	if (pDrive)
	{
		cout << "[ExpMM_Lock_Freq::setFreq] " << d << endl;
		pDrive->setFreq(d);
		pDrive->PostUpdateGUI();
		pFPGA->SendParam(page_id, pDrive);
	}
}

// in MHz
void ExpMM_Lock_Freq::modulateOutput(double d)
{
	setFreq(GetCenter() + d);
}


double ExpMM_Lock_Freq::GetOutput()
{
	return getFreq();
}