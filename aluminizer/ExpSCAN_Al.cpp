#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif


#include "ExpSCAN_Al.h"
#include "FPGA_connection.h"
#include "RefCavityPage.h"
#include "Al3P1Page.h"
#include "data_plot.h"
#include "scan_variable.h"
#include "Voltages2.h"

ExpSCAN_Al::ExpSCAN_Al(const string& sPageName,
                       ExperimentsSheet* pSheet,
                       unsigned page_id) :
	ExpSCAN_Detect(sPageName, pSheet, page_id),
	shiftEh("Eh shift",  &m_TxtParams, "0", &m_vParameters),
	shiftEv("Ev shift",  &m_TxtParams, "0", &m_vParameters),
	oldEh(1e99),
	oldEv(1e99)
{

}

TransitionPage* ExpSCAN_Al::GetCurrentTransition()
{
	return ExpAl::pAl3P1;
}

void ExpSCAN_Al::setupRemoteParam(GUI_double* pGUI)
{
	if (pGUI->GetName() == "Ground state")
	{
		pGUI->setRange(-2.5, 2.5);
		pGUI->setPrecision(1);
		pGUI->setIncrement(1);
	}
}


double ExpSCAN_Al::calcGroundState()
{
	if (getPolarization() == 1)
		return 2.5;
	else
		return -2.5;
}

GUI_ttl* ExpSCAN_Al::getRamseyPulse()
{
	return dynamic_cast<GUI_ttl*>(FindParameter("Ramsey"));
}


void ExpSCAN_Al::InitializeScan()
{
	if (ExpAl::pAl3P1)
		ExpAl::pAl3P1->on_action("UPDATE FPGA");

	ExpSCAN_Detect::InitializeScan();
}

void ExpSCAN_Al::InitExperimentSwitch()
{
	ExpSCAN_Detect::InitExperimentSwitch();

	if (shiftEh.Value())
	{
		oldEh = pVoltages->getE(4);
		pVoltages->setE(4, oldEh + shiftEh);
	}

	if (shiftEv.Value())
	{
		oldEv = pVoltages->getE(5);
		pVoltages->setE(5, oldEv + shiftEv);
	}
}

void ExpSCAN_Al::DefaultExperimentState()
{
	if (shiftEh.Value() && (oldEh != 1e99))
		pVoltages->setE(4, oldEh);

	if (shiftEv.Value() && (oldEv != 1e99))
		pVoltages->setE(5, oldEv);

	ExpSCAN_Detect::DefaultExperimentState();
}

ExpAl3P0::ExpAl3P0(const string& sPageName,
                   ExperimentsSheet* pSheet,
                   unsigned page_id) :

	ExpSCAN_Al(sPageName, pSheet, page_id),
	pRefCavityFreqChannel(0),
	pRefCavityDriftChannel(0),
	p3P1corrChannel(0),
	p3P0CavityPage(0),
	p3P1CavityPage(0),
	bStop1S0(false)
{
}

TransitionPage* ExpAl3P0::GetCurrentTransition()
{
	return ExpAl::pAl3P0;
}

void ExpAl3P0::AddDataChannels(DataFeed& df)
{
	ExpSCAN_Al::AddDataChannels(df);
	df.AddChannel(pRefCavityFreqChannel = new ConstantChannel("Ref. cavity freq [Hz]", false, 3));
	df.AddChannel(pRefCavityDriftChannel = new ConstantChannel("Ref. cavity drift [Hz/s]", false, 3));
}

unsigned ExpAl3P0::FitYColumn()
{
	unsigned nChannels = pFPGA->getNumDataChannels(page_id);

	for (unsigned i = 0; i < nChannels; i++)
	{
		string name = pFPGA->getDataChannelName(page_id, i);
		if (name == "Clock xition (s)")
			return 1 + i;
	}

	return ExpSCAN_Al::FitYColumn();
}


void ExpAl3P0::AddAvailableActions(std::vector<std::string>* v)
{
	ExpSCAN_Detect::AddAvailableActions(v);

	if (m_pSheet->scan_scheduler.IsRunning(this, 0))
		v->push_back("STOP 1S0");
}

void ExpAl3P0::on_action(const std::string& s)
{
	if (s == "STOP 1S0")
		bStop1S0 = true;

	ExpSCAN_Al::on_action(s);
}

void ExpAl3P0::InitializeScan()
{
	bStop1S0 = false;

	p3P0CavityPage = dynamic_cast<RefCavityPage*>(m_pSheet->FindPage("Elsner"));
	p3P1CavityPage = dynamic_cast<RefCavityPage*>(m_pSheet->FindPage("ULE88"));

	if (!p3P0CavityPage)
		throw runtime_error("[ExpAl3P0::InitializeScan] no reference cavity page");

	//find result channels w/ 3P1 correction
	p3P1corrChannel = findFPGAchannel("3P1 corr. [kHz]");

	ExpSCAN_Al::InitializeScan();
}

void ExpAl3P0::PreAcquireDataPoint(Scan_Base* sb, DataFeed& df)
{
	ExpSCAN_Detect::PreAcquireDataPoint(sb, df);
	pRefCavityFreqChannel->SetCurrentData(p3P0CavityPage->GetCurrentFrequency());
	pRefCavityDriftChannel->SetCurrentData(p3P0CavityPage->GetDriftRate());
}

void ExpAl3P0::PostAcquireDataPoint(Scan_Base* sb, DataFeed& df)
{
	ExpSCAN_Detect::PostAcquireDataPoint(sb, df);

	if (bStop1S0)
	{
		double n3P0, mF;
		pFPGA->getCurrentClockState(n3P0, mF);

		if (n3P0 <= 0.5)
			pRunObject->SetFinished();
	}

	//make 3P1 correction
	if (p3P1CavityPage)
		p3P1CavityPage->shiftVisFrequency(p3P1corrChannel->GetCurrentData() * 1e3);
}

ExpAl3P0_lock::ExpAl3P0_lock(const string& sPageName,
                             ExperimentsSheet* pSheet,
                             unsigned page_id) :
	ExpAl3P0(sPageName, pSheet, page_id),
	TargetFreq("Traget freq. [Hz]", &m_TxtParams, "0.0", &m_vParameters),
	DutyCycle("Duty cycle [%]",  &m_TxtParams, "0.0", &m_vParameters),
	CorrectCavity("Correct cavity",  &m_TxtParams, "1", &m_vParameters),
	integralGain("Integral gain",  &m_TxtParams, "0", &m_vParameters),
	iUpdate(0),
	pFPGAerr(0),
	pDirection(0),
	pClockXition(0),
	no_xition_sound(soundDir.absoluteFilePath("attention.wav")),
	xition_ok_sound(soundDir.absoluteFilePath("sounds/xition_ok.wav"))
{
	DutyCycle.SetReadOnly(true);
	CorrectCavity.setToolTip("Apply servo corrections to ref. cavity?");
}

void ExpAl3P0_lock::InitializeScan()
{
	warningMode = 0;
	tStart = CurrentTime_s();

	iUpdate = 0;

	pClockPulse = dynamic_cast<GUI_dds*>(FindParameter("Al3P0 carrier pi mFg = +5/2"));

	if (!pClockPulse)
		pClockPulse = dynamic_cast<GUI_dds*>(FindParameter("Al3P0 carrier pi mFg = -5/2"));

	if (!pClockPulse)
		pClockPulse = dynamic_cast<GUI_dds*>(FindParameter("shifted Al3P0 carrier pi mFg = +5/2"));

	if (!pClockPulse)
		pClockPulse = dynamic_cast<GUI_dds*>(FindParameter("experiment pulse"));

	if (!pClockPulse)
		throw runtime_error("[ExpAl3P0_lock::InitializeScan] no clock pulse");


	pNumProbes.clear();
	pXitionProb.clear();
	pProbeFreq0.clear();
	pProbeFreq1.clear();

	//find result channels w/ number of clock probes to compute duty cycle
	for (unsigned i = 0; i < pFPGAchannels.size(); i++)
	{
		int j = -1;

		sscanf(pFPGAchannels[i]->GetName().c_str(), "clock probes (%d) [#](h)", &j);

		if ( j >= 0 )
			pNumProbes.push_back(pFPGAchannels[i]);

		j = -1;
		sscanf(pFPGAchannels[i]->GetName().c_str(), "xition prob. (%d)(h)", &j);

		if ( j >= 0 )
			pXitionProb.push_back(pFPGAchannels[i]);

		j = -1;
		sscanf(pFPGAchannels[i]->GetName().c_str(), "probe det0 (%d) [Hz](h)", &j);

		if ( j >= 0 )
			pProbeFreq0.push_back(pFPGAchannels[i]);

		j = -1;
		sscanf(pFPGAchannels[i]->GetName().c_str(), "probe det1 (%d) [Hz](h)", &j);

		if ( j >= 0 )
			pProbeFreq1.push_back(pFPGAchannels[i]);
	}

	pFPGAerr = findFPGAchannel("FPGA error signal");
	pDirection = findFPGAchannel("probe dir");
	pClockXition = findFPGAchannel("Clock xition");

	iLeftChannel = -1;
	iRightChannel = -1;

	nLeftXition = 0;
	nRightXition = 0;

	nProbesTotal = 0;

	integralError = 0;

	RecalculateModulation();

	line_shape_plots[0]->clear();

	ExpAl3P0::InitializeScan();
}

void ExpAl3P0_lock::PostAcquireDataPoint(Scan_Base* sb, DataFeed& df)
{
	ExpAl3P0::PostAcquireDataPoint(sb, df);

	if (iLeftChannel == -1)
	{
		//figure out left & right channel index
		vector<int> np(pNumProbes.size());
		for (unsigned i = 0; i < pNumProbes.size(); i++)
			np[i] = pNumProbes[i]->GetCurrentData();

		iLeftChannel = max_element(np.begin(), np.end()) - np.begin();
		np[iLeftChannel] = 0;
		iRightChannel = max_element(np.begin(), np.end()) - np.begin();

		if (iRightChannel < iLeftChannel)
		{
			int i = iRightChannel;
			iRightChannel = iLeftChannel;
			iLeftChannel = i;
		}
	}

	//compute duty cycle
	double wall_time = CurrentTime_s() - tStart;
	double nProbes = pNumProbes[iRightChannel]->GetCurrentData() + pNumProbes[iLeftChannel]->GetCurrentData();
	nProbesTotal += nProbes;

	//compute projection noise
	nLeftXition += pXitionProb[iLeftChannel]->GetCurrentData() * pNumProbes[iLeftChannel]->GetCurrentData();
	nRightXition += pXitionProb[iRightChannel]->GetCurrentData() * pNumProbes[iRightChannel]->GetCurrentData();
	double delta = nRightXition - nLeftXition;
	double sum = nRightXition + nLeftXition;
	double sigma = sqrt(sum);

	char buff[128];
	sprintf(buff, "error = (%4.0f+/-%4.2f)/%4.0f", delta, sigma, nProbesTotal);
	pScan->getPlot()->setMarkerText(buff, 10);

	double tProbe = nProbesTotal * (pClockPulse->getTime() * 1e-6);;

	DutyCycle.SetValue(100 * tProbe / wall_time);
	DutyCycle.PostUpdateGUI();

	for (unsigned i = 0; i < pNumProbes.size(); i++)
	{
		if (pNumProbes[i]->GetCurrentData() > 0)
		{
			//double f = pNumProbes[0]


			double f = pDirection->GetCurrentData() > 0 ? pProbeFreq0.at(i)->GetCurrentData() : pProbeFreq1.at(i)->GetCurrentData();
			double p = pXitionProb.at(i)->GetCurrentData();
			line_shape_plots[0]->addX(f);
			line_shape_plots[0]->addY(p, "");
			line_shape_plots[0]->replot();
		}
	}

	integralError += integralGain * pFPGAerr->GetCurrentData();


	if (pClockXition->GetCurrentData() == 0)
	{
		no_xition_sound.play();
		warningMode = 1;
	}
	else
	{
		if (warningMode == 1)
		{
			xition_ok_sound.play();
			warningMode = 0;
		}
	}
}

//called after every interruption of the scan, e.g. lost ions, or scan repitition
void ExpAl3P0_lock::InitExperimentSwitch()
{
	bool bInternalShift = !CorrectCavity.Value();

	if (!bInternalShift)
	{
		physics::line l1 = CurrentLine();
		double fNew = ExpAl::pAl3P0->AOMFrequency(l1) * 1e-6;
		getLockInPulse()->setFreq(fNew);
		getLockVariable()->set_default_state(fNew);
	}

//   pFPGA->SendParam(page_id, getLockInPulse());
//   getLockInPulse()->PostUpdateGUI();

	ExpAl3P0::InitExperimentSwitch();

	printf("[%s::InitExperimentSwitch()] f = %9.3f\r\n", Title().c_str(), getLockInPulse()->getFreq() * 1e6);
}

void ExpAl3P0_lock::ShiftCenter(double d) // in MHz
{
	modulateOutput(0);                     // make sure frequencies are unmodulated before centering Zeeman components

	double fOld = getLockInPulse()->getFreq();

	bool bInternalShift = !CorrectCavity.Value();

	if (bInternalShift)
	{
		getLockInPulse()->setFreq(fOld + d);
		getLockVariable()->set_default_state(fOld + d);
	}
	else
	{
		physics::line l1 = CurrentLine();
		ExpAl::pAl3P0->ShiftLine(l1, d * 1e6, iUpdate++);

		double fNew = ExpAl::pAl3P0->AOMFrequency(l1) * 1e-6;
		getLockInPulse()->setFreq(fNew);
		getLockVariable()->set_default_state(fNew);
	}

	getLockInPulse()->PostUpdateGUI();

	double fNew = getLockInPulse()->getFreq();

	printf("[%s::ShiftCenter(%6.3f)] fOld = %9.3f  --> fNew = %9.3f\r\n", Title().c_str(), d * 1e6, fOld * 1e6, fNew * 1e6);
}


double ExpAl3P0_lock::GetCenter() // in Hz
{
	return getLockInPulse()->getFreq();
}


GUI_dds* ExpAl3P0_lock::getLockInPulse()
{
	return pClockPulse;
}

bool ExpAl3P0_lock::RecalculateModulation() // in Hz
{
	if (getLockInPulse())
		if (getLockInPulse()->Value().t > 0)
		{
			GUI_ttl* pRamsey = getRamseyPulse();
			if(pRamsey)
				if(pRamsey->isEnabled() && pRamsey->Value().t )
					return Modulation.SetValue(0.25e6 / (pRamsey->Value().t) );

			return Modulation.SetValue(0.4e6 / (getLockInPulse()->Value().t) );
		}

	return false;
}

double ExpAl3P0_lock::GetSignal()
{
	return 0;
}

bool ExpAl3P0_lock::getErrorSignal(double* err, double* dX)
{
	*dX = 2e-6 * GetModulation();
	*err = pFPGAerr->GetCurrentData();

	printf("error signal = %6.3f   integral error = %6.3f    dX = %6.3f Hz\r\n", *err, integralError, (*dX) * 1e6);

	*err += integralError;

	return true;
}

bool ExpAl3P0_lock::UpdateMeasurements(measurements_t&)
{
	return true;
}

void ExpAl3P0_lock::PostCreateGUI()
{
	ExpAl3P0::PostCreateGUI();
	AddLinePlots();
}

//must be called in GUI thread
void ExpAl3P0_lock::AddLinePlots()
{
	hgrids.push_back(new QHBoxLayout());
	grid.addLayout(hgrids.back(), grid.rowCount(), 0, 6, -1);


	line_shape_plots.push_back(new data_plot(this, true, 2, "", "", "", false));
	hgrids.back()->addWidget(line_shape_plots.back());
	//  hgrids.back()->setSizeConstraint(QLayout::SetFixedSize);
	line_shape_plots.back()->show();

	line_shape_plots.back()->addCurve("");
}
