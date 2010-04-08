#pragma once

#include "AluminizerApp.h"

template<class T> class ExpSCAN_FastLock : public T, public scans::LockParams
{
public:
ExpSCAN_FastLock(const std::string& sPageName, ExperimentsSheet* pSheet);
virtual ~ExpSCAN_FastLock()
{
}

protected:

virtual void FinishRun()
{
	probe_log.close();
	T::FinishRun();
}

void RecalculateModulation()
{
	if (T::RamseyTime > 0)
		Modulation.SetValue( 0.25e6 / (T::RamseyTime * T::GetCurrentTransition()->AOMorder) );
	else
	if (T::pX)
		Modulation.SetValue( 0.4e6 * T::pX->AOMFrequencyFourierLimit( T::InterrogationTime ) );
}

virtual void PreAcquireDataPoint(scans::Scan_Base* pScan, DataFeed& df)
{
	T::PreAcquireDataPoint(pScan, df);

	//figure out which channel takes which shots

	vector<bool> shotsL(T::NumExperiments, false);
	vector<bool> shotsM(T::NumExperiments, false);
	vector<bool> shotsR(T::NumExperiments, false);

	FillShotOrder(shotsL, shotsM, shotsR);

	/*	for(size_t i=0; i<shotsL.size(); i++)
	   {
	      cerr << " L: " << shotsL[i] << "   ";
	      cerr << " M: " << shotsM[i] << "   ";
	      cerr << " R: " << shotsR[i] << endl;
	   }
	 */
	pSignalL->SetShots(shotsL);
	pSignalR->SetShots(shotsR);
	pSignalM->SetShots(shotsM);
}

virtual void FillShotOrder(vector<bool>& vL, vector<bool>& vM, vector<bool>& vR)
{
	size_t keyLR = rand() % 2;

	for (size_t i = 0; i < static_cast<unsigned>(T::NumExperiments.Value()); i++)
	{
		// probe on resonance every 5th point
		if (i % 5 == 4)
			vM[i] = true;
		else
		{
			vL[i] = (i % 2 == keyLR);
			vR[i] = (i % 2 != keyLR);
		}
	}
}

virtual DataChannel* AddDataChannels(DataFeed& df)
{
	pSignalM = dynamic_cast<PMTHistogram*>(T::AddDataChannels(df));
	df.AddChannel(pSignalL = new PMTHistogram(*pSignalM));
	df.AddChannel(pSignalR = new PMTHistogram(*pSignalM));

	pSignalL->SetName("left");
	pSignalR->SetName("right");
	pSignalM->SetName("middle");

	df.AddChannel(pFrequencyChannel = new ConstantChannel("frequency", false));
	return df.AddChannel(pErrorChannel = new ConstantChannel("error", true));
}

virtual void PostAcquireDataPoint(scans::Scan_Base* pScan, DataFeed& df)
{
	double e = (pSignalR->GetAverage() - pSignalL->GetAverage()) / 8.0;

	current_error = e;

	pErrorChannel->SetCurrentData(current_error);
	pFrequencyChannel->SetCurrentData( GetCenter() );

	return T::PostAcquireDataPoint(pScan, df);
}

virtual void SetProbeFrequency(double f)
{
	probe_log << setprecision(3) << fixed << CurrentTime_s() << ", " << f << endl;

//		if(ScanDevice.Value() == "AC_B_field")
//			scan_pulse::pAC_Synth->SetDeviceFrequency(f);
//		else
	theApp->fpga_old->SetFreq(T::scan_name.c_str(), f * 1e-6, 0, 0);
}


//LockParams implementation
virtual double GetNormalizedErrorSignal()
{
	return current_error;
}

virtual void   ShiftCenterNormalized(double s)
{
	if (T::pX)
		T::pX->ShiftLine(T::CurrentLine(), Modulation * s, num_updates++);
	else
		T::FCenter.SetValue(T::FCenter + (Modulation * s) * 1e-6);
}

virtual double GetCenter()
{
	if (T::pX)
		return T::pX->AOMFrequency(T::CurrentLine());
	else
		return T::FCenter * 1e6;
}

virtual double GetGain()
{
	return Gain;
}
virtual double GetGainI()
{
	return IntegratorGain;
}

virtual int GetRunModulo()
{
	return RunModulo.Value();
}
virtual void SetRunModulo(int rm)
{
	RunModulo.SetValue(rm);
}

virtual std::string GetLockVariableName()
{
	return "f";
}
protected:

virtual bool calc_modulation()
{
	return true;
}

GUI_double Modulation;
GUI_int PointsToTake;


double current_error;
int num_updates;
double tLastPlotUpdate;
PMTHistogram* pSignalL;
PMTHistogram* pSignalM;
PMTHistogram* pSignalR;
ConstantChannel* pErrorChannel;
ConstantChannel* pFrequencyChannel;

ofstream probe_log;

private:
GUI_int RunModulo;
GUI_double Gain;
GUI_double IntegratorGain;
};

