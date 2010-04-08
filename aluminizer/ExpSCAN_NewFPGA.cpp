#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ExpSCAN_NewFPGA.h"
#include "FPGA_connection.h"
#include "GlobalsPage.h"
#include "AluminizerApp.h"
#include "MgPage.h"
#include "ExpAl.h"
#include "data_plot.h"
#include "histogram_plot.h"
#include "Voltages2.h"
#include "Ovens.h"
#include "MotorsPage.h"

ExpSCAN_NewFPGA::ExpSCAN_NewFPGA(const string& sPageName,
                                 ExperimentsSheet* pSheet,
                                 unsigned page_id) :
	ExpSCAN_L(sPageName, pSheet, 0, page_id),
	linked_pulse("Linked pulse", &m_TxtParams, "1", &m_vParameters),
	exp_pulse_name("experiment pulse"),
	DebugFirstExp("Debug 1st exp.", &m_TxtParams, "1", &m_vParameters),
	DisableReorder("Disable reorder", &m_TxtParams, "0", &m_vParameters),
	pReorderChannel(0)
{
	LinkedParams.SetValue(false);
	RemoveParameter(&LinkedParams);
	RemoveParameter(&RotationAngle);

	DisableReorder.setToolTip("Disable priodic reordering");

	rpDebugFirstExp = dynamic_cast<GUI_bool*>(FindParameter("Debug first exp."));
	rpNumExp    = dynamic_cast<GUI_unsigned*>(FindParameter("Experiments"));

//   SafeRecalculateParameters();
}


void ExpSCAN_NewFPGA::PostCreateGUI()
{
	for (size_t i = 0; i < m_vParameters.size(); i++)
	{
		if (GUI_dds * p = dynamic_cast<GUI_dds*>(m_vParameters[i]))
			if (p->input)
				QObject::connect(p->input, SIGNAL(setup_scan(scan_target)), this, SLOT(slot_setup_scan(scan_target)), Qt::AutoConnection);

		if (GUI_ttl * p = dynamic_cast<GUI_ttl*>(m_vParameters[i]))
			if (p->input)
				QObject::connect(p->input, SIGNAL(setup_scan(scan_target)), this, SLOT(slot_setup_scan(scan_target)), Qt::AutoConnection);
	}
}

DataChannel* ExpSCAN_NewFPGA::findFPGAchannel(const std::string& key)
{
	for (unsigned i = 0; i < pFPGAchannels.size(); i++)
		if (pFPGAchannels[i]->GetName().find(key) != string::npos)
			return pFPGAchannels[i];

	return 0;
}

unsigned ExpSCAN_NewFPGA::getNumExp()
{
	return rpNumExp->Value();
}

void ExpSCAN_NewFPGA::OnStop()
{
//	pFPGA->SendIRQ(0);
}

void ExpSCAN_NewFPGA::InitializeScan()
{
	ExpSCAN_NewFPGA* pRecover = dynamic_cast<ExpSCAN_NewFPGA*>(m_pSheet->FindExperiment("Recover"));

	if (pRecover && pRecover != this)
		pRecover->InitializeScan();

	if (ExpAl::pMg)
		ExpAl::pMg->on_action("UPDATE FPGA");

	pFPGA->SendParams(page_id, FPGA_params);
}

void ExpSCAN_NewFPGA::AddDataChannels(DataFeed& df)
{
	unsigned nChannels = pFPGA->getNumDataChannels(page_id);

	channelData.resize(nChannels);
	pFPGAchannels.resize(nChannels);

	for (unsigned i = 0; i < nChannels; i++)
	{
		string name = pFPGA->getDataChannelName(page_id, i);

		if (name.find("signal") != string::npos)
		{
			pSignalChannel = new HistogramChannel(pGlobals->HistogramSize, name, 3, true);
			pFPGAchannels[i] = pSignalChannel;
		}
		else
			pFPGAchannels[i] = new ConstantChannel(name, true);

		if (pFPGAchannels[i]->GetName().find("(h)") != string::npos)
			pFPGAchannels[i]->SetPlot(false);

		df.AddChannel(pFPGAchannels[i]);
	}

	df.AddChannel(pReorderChannel = new ConstantChannel("reorder", true, 1));
}


void ExpSCAN_NewFPGA::UpdateScanPulses(double)
{
	if (pVoltages && !DisableReorder.Value())
	{
		int dir = pVoltages->ReorderPeriodic();
		pReorderChannel->SetCurrentData(dir);
	}
}

void ExpSCAN_NewFPGA::slot_setup_scan(scan_target st)
{
	QWriteLocker lock(&page_lock);

	current_scan_target = st;

	ScanType.SetValue(st.scan_type);
	ScanVariables.SetValue(st.label);

	printf("[ExpSCAN_NewFPGA::slot_setup_scan] %s (%s)\n", st.label.c_str(), st.scan_type.c_str());

	updateScanGUI(true);
	updateScanSourcesGUI(true);

	SafeRecalculateParameters();

	emit sig_update_data();
}

void ExpSCAN_NewFPGA::useFit(double x)
{
	ScanSource* cSS = dynamic_cast<ScanSource*>(currentScanSource());

	if (cSS && !IgnoreFit.Value())
		cSS->useFit(ScanType, x);
}

double ExpSCAN_NewFPGA::GetInterrogationTime()
{
	QWriteLocker lock(&page_lock);

	PulseScanSource* pSS = dynamic_cast<PulseScanSource*>(currentScanSource());

	if (pSS)
		return pSS->getPulseTime();
	else
	{
		cerr << "[ExpSCAN_NewFPGA::GetInterrogationTime()] unknown interrogation time" << endl;
		return 1;
	}
}

double ExpSCAN_NewFPGA::GetCenterFrequency()
{
	FScanSource* fSS = dynamic_cast<FScanSource*>(currentScanSource());

	if (fSS)
		return fSS->getPulseFreq();
	else
		return 0;
}

bool ExpSCAN_NewFPGA::UpdateFittedPiTime(double tFit, double t0, bool bGoodFit)
{
	ExpSCAN::UpdateFittedPiTime(tFit, t0, bGoodFit);

	if (bGoodFit && !IgnoreFit.Value())
	{
//todo		pulse_t_scan* p = dynamic_cast<pulse_t_scan*>(current_pulse_scan);
//		if(p)
		{
//			p->setTime(tFit);

			GetCurrentTransition()->SetPiTime(tFit, CurrentLine());
		}

		return true;
	}
	else
		return false;
}

void ExpSCAN_NewFPGA::InitExperimentSwitch()
{
	GUI_dds* pPulseGUI = dynamic_cast<GUI_dds*>(FindParameter(exp_pulse_name));

	if (pPulseGUI)
	{
		pPulseGUI->PostUpdateGUI();
		pFPGA->SendParam(page_id, pPulseGUI);
	}

	ExpSCAN_L::InitExperimentSwitch();
}

bool ExpSCAN_NewFPGA::relinkPulses()
{
	bool bChanged = false;
	GUI_dds* pPulseGUI = dynamic_cast<GUI_dds*>(FindParameter(exp_pulse_name));
	TransitionPage* pX = GetCurrentTransition();

	if (pPulseGUI && pX && getTransitionBaseName().length())
	{
		string sNew = "experiment pulse";

		if (linked_pulse.Value())
			sNew = pulse_name(getTransitionBaseName(),  2 * getGroundState(), getPolarization(), getSideband());

		if (exp_pulse_name != sNew && pX)
		{
			exp_pulse_name = sNew;

			if (linked_pulse.Value())
				pX->LinkPulseTo(pPulseGUI, sNew);
			else
				pPulseGUI->SetName(sNew);

			bChanged = true;
		}
	}

	return bChanged;
}

bool ExpSCAN_NewFPGA::RecalculateParameters()
{
	bool bChanged = relinkPulses();


	if (bChanged)
	{
		ExpSCAN_L::RecalculateParameters();
		return true;
	}
	else
		return ExpSCAN_L::RecalculateParameters();
}


void ExpSCAN_NewFPGA::setNumExp(unsigned n)
{
	if (rpNumExp)
		rpNumExp->SetValue(n);
	else
		cerr << "WARNING: can't set number of experiments to " << n << endl;
}

ExpSCAN_Detect::ExpSCAN_Detect(const string& sPageName,
                               ExperimentsSheet* pSheet,
                               unsigned page_id) :
	ExpSCAN_NewFPGA(sPageName, pSheet, page_id),
	xy_memory_shared("webcamQt_XY"),
	dcX(0), dcY(0)
{
	if (!xy_memory_shared.attach(QSharedMemory::ReadOnly))
		xy_memory_shared.create(2 * sizeof(double), QSharedMemory::ReadOnly);
}

void ExpSCAN_Detect::AddDataChannels(DataFeed& df)
{
	ExpSCAN_NewFPGA::AddDataChannels(df);

	if (pGlobals->recordCameraXY)
	{
		df.AddChannel(dcX = new ConstantChannel("Camera X", true, 3));
		df.AddChannel(dcY = new ConstantChannel("Camera Y", true, 3));
	}
	else
	{
		dcX = 0;
		dcY = 0;
	}
}

void ExpSCAN_Detect::PostAcquireDataPoint(Scan_Base* sb, DataFeed& df)
{
	if (dcX != 0 && dcY != 0)
	{
		double xy[2];
		xy_memory_shared.lock();
		memcpy(xy, xy_memory_shared.data(), 2 * sizeof(double));
		xy_memory_shared.unlock();

		dcX->SetCurrentAverage(xy[0] - 150);
		dcY->SetCurrentAverage(xy[1] + 28);
	}

	ExpSCAN_NewFPGA::PostAcquireDataPoint(sb, df);
}


ExpQubit::ExpQubit(const string& sPageName,
                   ExperimentsSheet* pSheet,
                   unsigned page_id) :
	ExpSCAN_Detect(sPageName, pSheet, page_id)
{
}

void ExpQubit::setupRemoteParam(GUI_double* pGUI)
{
	if (pGUI->GetName() == "Ground state")
	{
		pGUI->setRange(-3, 3);
		pGUI->setPrecision(0);
		pGUI->setIncrement(1);
	}
}


TransitionPage* ExpSCAN_NewFPGA::GetCurrentTransition()
{
	return ExpAl::pMg;
}

TransitionPage* ExpSCAN_NewFPGA::GetCoolingTransition()
{
	return ExpAl::pMg;
}

void ExpQubit::DoCalibration(calibration_item* ci, RunObject* pOwner)
{
	GUI_bool* pGSC = dynamic_cast<GUI_bool*>(FindParameter("Sideband cooling"));

	if (pGSC)
		pGSC->SetValue(ci->options.find("NoGSC") == string::npos);

	IgnoreFit.SetValue(ci->options.find("IgnoreFit") != string::npos);

	ExpSCAN_Detect::DoCalibration(ci, pOwner);
}


void ExpRF_lock::UpdateScanPulses(double x)
{
	ExpSCAN_Detect::UpdateScanPulses(x);

	//Adjust Ramsy time to reach maximum
	if (RamseyMaxT.Value() > 0)
	{
		if (pRamseyPulse && getLockInPulse())
		{
			double t = pRamseyPulse->getTime();
			if (t < RamseyMaxT.Value())
			{
				t *= 1.05;
				t = std::min(t, RamseyMaxT.Value());

				pRamseyPulse->setTime(t);
			}
		}
	}

	RecalculateModulation();

	if (pRamseyPulse)
		pFPGA->SendParam(page_id, pRamseyPulse);

	if (getLockInPulse())
		pFPGA->SendParam(page_id, getLockInPulse()); // send RF pulse info
}

void ExpRF_lock::DefaultExperimentState()
{
	ExpSCAN_Detect::DefaultExperimentState();

	if (getLockInPulse())
		pFPGA->SendParam(page_id, getLockInPulse());
}

bool ExpRF_lock::RecalculateModulation()
{
	if (getLockInPulse())
	{
		bool bRamsey = false;

		if (pRamseyPulse)
			bRamsey = (pRamseyPulse->Value().t > 0) && (pRamseyPulse->isEnabled());

		if (getLockInPulse()->Value().t > 0)
		{
			if (bRamsey)
				return Modulation.SetValue(0.25e6 / (pRamseyPulse->Value().t + getLockInPulse()->Value().t) );
			else
				return Modulation.SetValue(0.4e6 / (getLockInPulse()->Value().t) );
		}
	}
	return false;
}

ExpLoad::ExpLoad(const string& sPageName,
                 ExperimentsSheet* pSheet,
                 unsigned exp_id) : ExpSCAN_Detect(sPageName, pSheet, exp_id),
	MaxTime("Max load time [s]", &m_TxtParams, "60", &m_vParameters),
	MaxCounts("Max counts", &m_TxtParams, "100", &m_vParameters),
	MinCounts("Min counts", &m_TxtParams, "-100", &m_vParameters),
	LaunchHeatingExp("Launch heat exp.", &m_TxtParams, "1", &m_vParameters),
	loaded_sound(soundDir.absoluteFilePath("smb3_powerup.wav")),
	times_up_sound(soundDir.absoluteFilePath("smb3_coin.wav")),
	bMg(sPageName.find("Mg") != string::npos),
	bAl(sPageName.find("Al") != string::npos)
{
	MaxCounts.setToolTip("If background-corrected counts exceed this, the load stops.");
	MinCounts.setToolTip("If background-corrected counts fall below this, the load stops.");
}

void ExpLoad::InitializeScan()
{
	RunOvens* pOvens = dynamic_cast<RunOvens*>(m_pSheet->FindExperiment("Ovens"));
	MotorsPage* pMotors = dynamic_cast<MotorsPage*>(m_pSheet->FindPage("Motors"));

	if (pOvens && bMg)
		pOvens->setMgOven(true);

	if (pOvens && bAl)
	{
		pOvens->setAlOven(true);
		pOvens->setAlPI(true);
	}

	if (pMotors && bMg)
		pMotors->setMgPI(true);

	ExpSCAN_Detect::InitializeScan();
}

void ExpLoad::PostAcquireDataPoint(Scan_Base* sb, DataFeed& df)
{
	ExpSCAN_Detect::PostAcquireDataPoint(sb, df);

	//if counts exceed threshold, stop the load
	for (size_t i = 0; i < df.NumChannels(); i++)
	{
		if (df.GetName(i).find("Mg+ sig.") != string::npos)
		{
			if ( (df.getAverage(i) > MaxCounts) || (df.getAverage(i) < MinCounts) )
			{
				loaded_sound.play();
				DefaultExperimentState();
				SleepHelper::msleep(5000); //wait for 5 s to allow ions to cool
				pRunObject->SetFinished();

				if (LaunchHeatingExp)
				{
					ExperimentPage* exp = dynamic_cast<ExperimentPage*>(m_pSheet->FindExperiment("Heat Z"));

					if (exp)
						exp->on_action("RUN");
				}
			}
		}
	}

	//if max time has expired, stop the load
	if ((CurrentTime_s() - sb->tStart) > MaxTime)
	{
		pRunObject->SetFinished();
		times_up_sound.play();
	}
}


void ExpLoad::DefaultExperimentState()
{
	RunOvens* pOvens = dynamic_cast<RunOvens*>(m_pSheet->FindExperiment("Ovens"));
	MotorsPage* pMotors = dynamic_cast<MotorsPage*>(m_pSheet->FindPage("Motors"));

	if (pOvens && bMg)
		pOvens->setMgOven(false);

	if (pOvens && bAl)
	{
		pOvens->setAlOven(false);
		pOvens->setAlPI(false);
	}

	if (pMotors && bMg)
		pMotors->setMgPI(false);

	ExpSCAN_Detect::DefaultExperimentState();
}



