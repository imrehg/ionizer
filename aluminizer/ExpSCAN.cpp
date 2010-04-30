#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "AluminizerApp.h"
#include "GlobalsPage.h"
#include "Voltages2.h"
#include "ExpSCAN.h"
#include "TransitionPage.h"
#include "Numerics.h"
#include "scan_variable.h"

#include "data_plot.h"
#include "histogram_plot.h"

using namespace std;
using namespace numerics;
using namespace physics;



ExpSCAN::ExpSCAN(const string& sPageName,
                 ExperimentsSheet* pSheet,
                 const char*, unsigned page_id) :
	ExperimentPage(sPageName, pSheet, page_id),
	pSignalChannel(0),
	unused_fit(false),
	pScan(0),
	pFit(0),

	ScanType("Scan type", "Frequency\nTime\nContinuous\nVoltage\nB-field\nLockIn\nGeneral\nMotor\n",  &m_TxtParams, "Frequency", &m_vParameters),
	ScanVariables("Scan variable", "",   &m_TxtParams, "", &m_vParameters),
	Priority("Priority",               &m_TxtParams, "1", &m_vParameters),
	LinkedParams("Linked params",          &m_TxtParams, "true", &m_vParameters),
	AutomaticParams("Auto-span",           &m_TxtParams, "true", &m_vParameters),
	NumScans("Scans",                  &m_TxtParams, "1", &m_vParameters),
	NumDataPoints("Data points",               &m_TxtParams, "50", &m_vParameters),
	RotationAngle("Rotation angle [pi]",       &m_TxtParams, "1", &m_vParameters),
	Span("Span",              &m_TxtParams, "1", &m_vParameters),
	FCenterFit("Fitted center",       &m_TxtParams, "", &m_vParameters, true),
	Start("Start",                  &m_TxtParams, "0.0", &m_vParameters),
	Stop("Stop",                &m_TxtParams, "1.0", &m_vParameters),
	Step("Step",                &m_TxtParams, "", &m_vParameters, true),
	Gain(Span),
	Modulation(Step),
	LockCenter(FCenterFit),
	GoodFit("Good fit",               &m_TxtParams, "false", &m_vParameters, true),
	IgnoreFit("Ignore fit",             &m_TxtParams, "false", &m_vParameters, true),
	FitMinContrast("Min. fit contrast",         &m_TxtParams, "0.7", &m_vParameters),
	TFittedPiTime("Fitted pi time [us]",       &m_TxtParams, "", &m_vParameters, true),
	FitMinAmplitude("Min. fit amplitude",        &m_TxtParams, "5", &m_vParameters),
	EnablePlot("Enable Plot",               &m_TxtParams, "1", &m_vParameters, false),
	Histogram("Histogram",              &m_TxtParams, "false", &m_vParameters, false),
	ScanTime("Scan time [s]",          &m_TxtParams, "", &m_vParameters, true),
	progress("Scan progress",          &m_TxtParams, "0", &m_vParameters),
	LogShots("Log shots",              &m_TxtParams, "false", &m_vParameters),
	NumMeasurements("Measurements [#]",       &m_TxtParams, "3", &m_vParameters),
	initial_delay(0),
	scan_name("scan"),
	exp_run_time(0),
	data_plotW(0),
	hist_plotW(0),
	cci(0),
	scanGUImode(0),
	oldScanSource(0)
{
	ScanTime.SetReadOnly(true);
	NumDataPoints.setToolTip("Number of points per scan");
	AutomaticParams.setToolTip("Automatically calculate scan range");
	NumMeasurements.setToolTip("Number of servo measurement points to acquire before signaling IDLE");

	//allows plots to be setup from non-GUI threads
	QObject::connect(this, SIGNAL(signal_setup_plots()),
	                 this, SLOT(slot_setup_plots()), Qt::BlockingQueuedConnection);

	Step.setPrecision(9);
	Span.setPrecision(9);
	Start.setPrecision(9);
	Stop.setPrecision(9);
	FCenterFit.setPrecision(9);

	/*
	   updateScanSourcesGUI();
	   updateScanGUI();
	   updateScanSpan();
	   updateScanStartStop();
	   updateScanStep();
	 */
}

ExpSCAN::~ExpSCAN()
{
	if (pScan)
		delete pScan;

	if (pFit)
		delete pFit;

	if (data_plotW)
	{
		data_plotW->close();
		delete data_plotW;
	}

	if (hist_plotW)
	{
		hist_plotW->close();
		delete hist_plotW;
	}
}

TransitionPage* ExpSCAN::GetCurrentTransition()
{
	return 0;
}

void ExpSCAN::AddAvailableActions(std::vector<std::string>* v)
{
	ExperimentPage::AddAvailableActions(v);

	if (pScan)
	{
		string fname = pScan->debugFileName();
		if (fname.length())
			v->push_back("DEBUG");
	}

	if (!m_pSheet->scan_scheduler.IsRunning(this, 0))
	{
		if (unused_fit)
			v->push_back("USE FIT");

		if (get_data_plot())
			if (get_data_plot()->canPrint())
				v->push_back("PRINT");
	}
}

void ExpSCAN::slot_setup_plots()
{
	//clear old plot windows

	if (data_plotW)
	{
		data_plotW->close();
		delete data_plotW;
		data_plotW = 0;
	}

	if (hist_plotW)
	{
		hist_plotW->close();
		delete hist_plotW;
		hist_plotW = 0;
	}

	if (EnablePlot)
	{
		string extended_title = Title() + " "  + pRunObject->GetStartTimeS();

		bool bPlotAverages = !IsContinuousScan(ScanType);
		bool bHasLegend = true;

		if (cci)
			if (cci->pX)
				if (cci->pX->acceptCalibrationPlot(0, 0))
					bHasLegend = cci->pX->wantsPlotLegend();

		data_plot* plot = new data_plot(this, bPlotAverages, pGlobals->PlotLineWidth, extended_title, "", "", bHasLegend);

		bool bNeedsPlotW = true;

		//allow page that the launched calibration to handle the plot
		if (cci)
			if (cci->pX)
				bNeedsPlotW = !(cci->pX->acceptCalibrationPlot(cci, plot));

		//otherwise create a separate plot window
		if (bNeedsPlotW)
		{
			data_plotW = new plot_window(this, plot, extended_title, Title() + "data");
			data_plotW->show();
		}
	}

	if (Histogram)
	{
		histogram_plot* hist_plot = new histogram_plot(this, "", "");

		string title = "Histogram --" + Title();
		hist_plotW = new plot_window(this, hist_plot, title);
		hist_plotW->show();
	}
}

void ExpSCAN::on_action(const std::string& s)
{
	if (s == "PRINT" && data_plotW)
		dynamic_cast<data_plot*>(data_plotW->pPlot)->print();

	if (s == "USE FIT" && unused_fit)
	{
		unused_fit = false;

		bool bOld = IgnoreFit;
		IgnoreFit.SetValue(false);

		if (IsTimeScan(ScanType))
			useFit(TFittedPiTime);
		else
			useFit(FCenterFit);

		IgnoreFit.SetValue(bOld);

		PostUpdateData();
		PostUpdateActions();
	}
	else
	if (s == "SHIFT F0")
	{
		/* TODO: re-implement
		   TransitionPage* pTX = GetCurrentTransition();

		   if( IsLinkedScan() && IsFreqScan(ScanType) && pTX && unused_frequency_fit)
		   {
		      double delta_f = 0; //(FCenterFit - FCenter) * 1e6;

		      if(delta_f != 0)
		      {
		         GetCurrentTransition()->ShiftAllFrequencies(delta_f);
		      }

		      unused_frequency_fit = false;

		      PostUpdateData();
		      PostUpdateActions();
		   } */
	}
	else
	{
		if (s == "DEBUG")
		{
			if (pScan)
			{
				string fname = pScan->debugFileName();
				if (fname.length())
					LaunchEditor(fname);
			}
		}

		ExperimentPage::on_action(s);
	}
}

//Update the GUI
bool ExpSCAN::RecalculateParameters()
{

	bool Changed = ExperimentPage::RecalculateParameters();

//	if(GetCurrentTransition())
//		GetCurrentTransition()->UpdatePolarizationGUI(&Polarization, &(GetCurrentTransition()->tx), GroundState());

	if ( IsLinkedScan() && GetCurrentTransition() )
	{
//		Changed |= GetCurrentTransition()->LinkSidebandPiTimeTo(&InterrogationTime, CurrentLine());
	}
	else
	{
//		Changed |= InterrogationTime.SetName(GetITimeName());
	}

	Changed |= updateScanSourcesGUI();
	Changed |= updateScanGUI();
	Changed |= updateScanSpan();
	Changed |= updateScanStartStop();
	Changed |= updateScanStep();


	return Changed;
}

void ExpSCAN::InitExperimentStart()
{
	emit signal_setup_plots();

	GoodFit.SetValue(false);

	unused_fit = false;

	if (LinkedParams)
		pX = GetCurrentTransition();
	else
		pX = 0;

	pRunObject->SetDataFileName(GetDataFileName());

	//save all parameters of all pages in the current directory
	m_pSheet->SaveAllParameters("./");

	//save all parameters of all pages into the output directory
	m_pSheet->SaveAllParameters(pRunObject->GetOutputDirectory());

	if (pScan)
	{
		delete pScan;
		pScan = 0;
	}

	pScan = CreateScan();
	pScan->InitializeScan();

	ofstream DataFilesList("DataFilesList.csv", ios_base::app);
	DataFilesList << pRunObject->GetDataFileName() << "," << 7 << endl;
	DataFilesList.close();
}

void ExpSCAN::FinishRun()
{
	cout << "[ExpSCAN::FinishRun] entry" << endl;

	QWriteLocker lock(&page_lock);

	if (pScan)
	{
		Scan_Fittable* pScanFittable = dynamic_cast<Scan_Fittable*>(pScan);

		if ( pScan->CanFit() && pScanFittable)
		{
			if (pFit)
			{
				delete pFit;
				pFit = 0;
			}

			pFit = pScanFittable->RunFit(this);

			if (get_data_plot())
				if (pGlobals->SavePlotType.Value() != "None")
					get_data_plot()->replot(true, pGlobals->SavePlotType.Value());

			if (cci)
				if (cci->pX)
				{
					calibration_item ci = *cci;
					ci.pX->DidCalibration(cci, pFit);
					cci = 0;
				}
		}
	}

	cci = 0;

	cout << "[ExpSCAN::FinishRun] updating data" << endl;

	PostUpdateData();

	cout << "[ExpSCAN::FinishRun] exit" << endl;
}

data_plot* ExpSCAN::get_data_plot()
{
	if (cci)
		if (cci->pX)
		{
			data_plot* pDP = cci->pX->get_data_plot(cci, Title());

			if (pDP)
				return pDP;
		}

	if (data_plotW)
		return dynamic_cast<data_plot*>(data_plotW->pPlot);
	else
		return 0;
}

histogram_plot* ExpSCAN::get_hist_plot()
{
	if (hist_plotW)
		return dynamic_cast<histogram_plot*>(hist_plotW->pPlot);
	else
		return 0;
}

bool ExpSCAN::UpdateFittedCenter(double fFit, bool bGoodFit)
{
	GoodFit.SetValue( bGoodFit );
	FCenterFit.SetValue( fFit );

	unused_fit = true;

	return false;
}

bool ExpSCAN::UpdateFittedPiTime(double tFit, double t0, bool bGoodFit)
{
	GoodFit.SetValue( bGoodFit );
	TFittedPiTime.SetValue( tFit );

	initial_delay = t0;

	if (fabs(initial_delay + tFit) < fabs(initial_delay))
		initial_delay += tFit;

	if (fabs(initial_delay - tFit) < fabs(initial_delay))
		initial_delay -= tFit;


	cout << "initial_delay = " << initial_delay << endl;

	unused_fit = true;

	return false;
}

//in MHz
double ExpSCAN::GetRabiRate()
{
//	scan_pulse p(CurrentLine());
//	GetCurrentTransition()->CalculatePulse(p);
	throw runtime_error("[ExpSCAN::GetRabiRate] not implemented");
	return 1;
}

bool ExpSCAN::IsRamseyFScan()
{
	return ::IsRamseyFScan(ScanType);
}

bool ExpSCAN::IsRamseyPScan()
{
	return ::IsRamseyPScan(ScanType);
}

Scan_Base* ExpSCAN::CreateScan()
{
	vector<scan_variable*> scan_variables;

	if (IsLockInScan(ScanType))
		if (LockInParams * pLockInParams = dynamic_cast<LockInParams*>(this))
			if (ScanSource * ss = currentScanSource())
			{
				scan_variables.push_back(new source_scan_variable(ss));
				return new LockInScan(this, scan_variables, pLockInParams, ss);
			}

	if (ScanSource * ss = currentScanSource())
	{
		scan_variables.push_back(new source_scan_variable(ss));
		return new SourceScan(this, scan_variables, ss);
	}

	if (IsContinuousScan(ScanType))
		return new ContinuousScan(this, vector<scan_variable*>(0));

	throw runtime_error("[ExpSCAN::CreateScan] failure: unknown scan type");
}


//called after every interruption of the scan, e.g. lost ions, or scan repitition
void ExpSCAN::InitExperimentSwitch()
{

	if (pScan)
		pScan->InitExperimentSwitch();
}

void ExpSCAN::DefaultExperimentState()
{
	if (pScan)
		pScan->DefaultExperimentState();
}

void ExpSCAN::UpdateFrequency(double)
{
}
void ExpSCAN::UpdateTime(double)
{
}
void ExpSCAN::UpdatePhase(double)
{
}

//Must be called from a non-GUI thread to avoid deadlock.
void ExpSCAN::DoCalibration(calibration_item* ci, RunObject* pOwner)
{
	{
		QWriteLocker lock(&page_lock);

		cci = ci;

		SetCurrentScan(ci);
		LinkedParams.SetValue(true);
	}

	mutexPU.lock();
	PostUpdateData();
	page_updated.wait(&mutexPU);
	mutexPU.unlock();

//	cout << "[ExpSCAN::DoCalibration] start = " << Start.Value() << " stop = " << Stop.Value() << endl;

	if (pOwner)
		RunOwnedThread(pOwner);
	else
		m_pSheet->RunExperiment(Title());
}

void ExpSCAN::SetCurrentScan(const calibration_item* ci)
{
	QWriteLocker lock(&page_lock);

	DriveLine(ci->l);
	ScanType.SetValue(ci->scan_type);
	ScanVariables.SetValue(ci->scan_variable);

	if (ci->span != 0)
	{
		AutomaticParams.SetValue(false);
		Span.SetValue(ci->span);
	}
	else
		AutomaticParams.SetValue(true);

	if (ci->start != ci->stop)
	{
		Start.SetValue(ci->start);
		Stop.SetValue(ci->stop);
	}

	if (ci->num_exp)
		setNumExp(ci->num_exp);

	updateScanSourcesGUI(true);
	updateScanGUI(true);
	updateScanSpan();
	updateScanStartStop();
	updateScanStep();

	if (ci->num_scans)
		NumScans.SetValue(ci->num_scans);

	if (ci->num_points)
		NumDataPoints.SetValue(ci->num_points);

	if (ci->num_flops && ci->IsTimeScan())
	{
		AutomaticParams.SetValue(false);
		Start.SetValue(0.0);
		Stop.SetValue(ci->num_flops * GetInterrogationTime());
//	  Span.SetValue(Stop - Start);

//	  cout << "[ExpSCAN::SetCurrentScan] start = " << Start.Value() << " stop = " << Stop.Value() << endl;
	}
	else
		AutomaticParams.SetValue(true);

	GUI_ttl* pWaitPulse = dynamic_cast<GUI_ttl*>(FindParameter("Wait"));
	if (pWaitPulse)
		pWaitPulse->setTime(ci->wait);

	cout << "Scan type = " <<  ci->scan_type << endl;
	cout << "Scan variable = " <<  ScanVariables.Value() << endl;
}

double ExpSCAN::GetNewFrequencyAfterFit()
{
	if (GoodFit)
		return FCenterFit;
	else
		return GetCenterFrequency();
}

double ExpSCAN::GetNewPiTimeAfterFit()
{
	if (GoodFit)
		return TFittedPiTime;
	else
//		return InterrogationTime;
		return 1;
}

bool ExpSCAN::IsLinkedScan()
{
	return LinkedParams;
}

bool ExpSCAN::IsScatterScan()
{
	return pGlobals->ScatterScan;
}


string ExpSCAN::GetDataFileName()
{
	ostringstream name;

	name << GetName() << " " << ScanType;

	ScanSource* cSS = currentScanSource();

	if (cSS)
		if (cSS->pParam)
			name << "(" << cSS->pParam->get_display_label() << ")";

	string s = name.str();

	//replace ' ' and '/' with '_'
	for (size_t i = 0; i < s.length(); i++)
		if (s[i] == ' ' || s[i] == '/')
			s[i] = '_';

	return s;
}

physics::line ExpSCAN::CurrentLine()
{
	return line(getGroundState(),
	            getExcitedState(),
	            getSideband(),
	            RotationAngle * M_PI);
}


double ExpSCAN::getGroundState()
{
	GUI_double* pGS = dynamic_cast<GUI_double*>(FindParameter("Ground state"));

	if (pGS)
		return pGS->Value();
	else
		return 0;
}

double ExpSCAN::getExcitedState()
{
	return getGroundState() + getPolarization();
}

int ExpSCAN::getPolarization()
{
	GUI_int* pPol = dynamic_cast<GUI_int*>(FindParameter("Polarization"));

	if (pPol)
		return pPol->Value();
	else
		return 0;
}

int ExpSCAN::getSideband()
{
	GUI_int* pSB = dynamic_cast<GUI_int*>(FindParameter("Sideband"));

	if (pSB)
		return pSB->Value();
	else
		return 0;
}



TransitionPage* ExpSCAN::FindTransitionPage(const std::string& name)
{
	for (size_t i = 0; i < m_pSheet->NumPages(); i++)
		if (TransitionPage * p = dynamic_cast<TransitionPage*>(m_pSheet->GetPage(i)))
			if (p->TransitionName() == name)
				return p;

	return 0;
}

double ExpSCAN::AOMdelay(const string&)
{
	return 0; //GetCurrentTransition()->GetAOMdelay();
}

double ExpSCAN::GetXoffset()
{
	pX = GetCurrentTransition();

	if (pX)
	{
		line l = CurrentLine();
		l.sb = 0;
		double f0 = pX->AOMFrequency(l) * 1e-6;

		if (fabs(GetCenterFrequency() - f0) < 100)
			return f0;
	}

	return 0;
}

//add a DDS pulse scan source to the list if it's not already there
void ExpSCAN::addScanSource(GUI_dds* pPulseDDS)
{
	bool bExist = false;

	for (unsigned i = 0; i < p_scan_sources.size(); i++)
	{
		if (p_scan_sources[i]->pParam->get_fpga_name() == pPulseDDS->get_fpga_name())
		{
			bExist = true;
			break;
		}
	}

	if (!bExist)
	{
		p_scan_sources.push_back(new FScanSource(pFPGA, get_page_id(), pPulseDDS));
		p_scan_sources.push_back(new TScanSource(pFPGA, get_page_id(), pPulseDDS));

		updateScanSourcesGUI();
	}
}


void ExpSCAN::AddParams()
{
	ExperimentPage::AddParams();

	//add scan sources to list for this page
	for (size_t i = 0; i < m_vParameters.size(); i++)
	{
		GUI_dds* pPulseDDS = dynamic_cast<GUI_dds*>(m_vParameters[i]);

		if (pPulseDDS)
		{
			p_scan_sources.push_back(new FScanSource(pFPGA, get_page_id(), pPulseDDS));
			p_scan_sources.push_back(new TScanSource(pFPGA, get_page_id(), pPulseDDS));
		}

		GUI_ttl* pPulseTTL = dynamic_cast<GUI_ttl*>(m_vParameters[i]);

		if (pPulseTTL)
			p_scan_sources.push_back(new TScanSource(pFPGA, get_page_id(), pPulseTTL));

		GUI_double* pDouble = dynamic_cast<GUI_double*>(m_vParameters[i]);

		if (pDouble)
		{
			DoubleScanSource* dss = new DoubleScanSource(pFPGA, get_page_id(), pDouble);
			p_scan_sources.push_back(dss);
		}
	}

//   SafeRecalculateParameters();
}

void ExpSCAN::setScanUnit(const std::string& unit)
{
	//update units for scan parameters
	string unit_str = unit.size() == 0 ? "" : "[" + unit + "]";

	if (IsLockInScan(ScanType))
	{
		Gain.SetName("Gain");
		Gain.setToolTip("Positive gain locks to peaks, negative to dips.");

		Modulation.SetName("Modulation");
		LockCenter.SetName("Lock center");


	}
	else
	{
		Start.SetName(string("Start ") + unit_str);
		Stop.SetName(string("Stop ")  + unit_str);
		Step.SetName(string("Step ")  + unit_str);
		Span.SetName(string("Span ")  + unit_str);
		Span.setToolTip("Start - Stop");
	}
}

void ExpSCAN::setScanMode(unsigned u)
{
	//update scan GUI mode
	// 0 - continuous, no scan parameters
	// 1 - center/span mode.  start/stop are automatically calculated
	// 2 - start/stop mode. center/span are automatically calculated
	scanGUImode = u;

	if (u == 0)
		showScanGUI(false);
	else
		showScanGUI(true);

	if (IsLockInScan(ScanType))
	{
		Span.SetReadOnly(false);
		Step.SetReadOnly(false);
		Gain.SetReadOnly(false);
	}
	else
	{
		Span.SetReadOnly(u == 2);
		Start.SetReadOnly(u == 1);
		Stop.SetReadOnly(u == 1);
		Step.SetReadOnly(true);
	}


	/*
	   ScanSource* cSS = currentScanSource();

	   if(cSS)
	    cSS->initScanPageGUI(this);
	 */
}

void ExpSCAN::showScanGUI(bool bShow)
{

	Span.Show(bShow);
	Step.Show(bShow);
	FCenterFit.Show(bShow);

	if (IsLockInScan(ScanType))
		bShow = false;

	NumScans.Show(bShow);
	NumDataPoints.Show(bShow);
	Start.Show(bShow);
	Stop.Show(bShow);

	FitMinAmplitude.Show(bShow);
	FitMinContrast.Show(bShow);
	TFittedPiTime.Show(bShow);
	AutomaticParams.Show(bShow);
	GoodFit.Show(bShow);
}

bool ExpSCAN::updateScanStartStop()
{
	bool bChanged = false;

	if (scanGUImode == 1)
	{
		ScanSource* fSS = dynamic_cast<FScanSource*>(currentScanSource());

		if (fSS && (GetInterrogationTime() != 0))
		{
			if (AutomaticParams && !IsLockInScan(ScanType))
			{
				TransitionPage* pX = GetCurrentTransition();

				double span = pX ?
				              4 * pX->AOMFrequencyFourierLimit(GetInterrogationTime()) :
				              4 / fabs( GetInterrogationTime() );

				bChanged |= Span.SetValue( fabs(span) );
				Span.SetReadOnly( true );
			}
			else
				Span.SetReadOnly( false );


			//         cout << "[ExpSCAN::updateScanStartStop] start = " << Start.Value() << " stop = " << Stop.Value() << endl;

		}


		ScanSource* cSS = currentScanSource();

		if (cSS)
		{
			bChanged |= Start.SetValue(cSS->getNonScanningValue() - Span  / 2);
			bChanged |= Stop.SetValue(cSS->getNonScanningValue() + Span  / 2);

//		 cout << "[ExpSCAN::updateScanStartStop] start = " << Start.Value() << " stop = " << Stop.Value() << endl;
		}
	}


	return bChanged;
}

bool ExpSCAN::updateScanSpan()
{
	if (IsLockInScan(ScanType))
		return false;

	bool bChanged = false;

	if (scanGUImode == 2)
	{
		ScanSource* tSS = dynamic_cast<TScanSource*>(currentScanSource());

		if (tSS)   //crashes here?
		{
			double tPi = tSS->getNonScanningValue();

			if (tSS && (tPi != 0))
			{
				if (AutomaticParams)
				{
					bChanged |= Start.SetValue(0.1);
					bChanged |= Stop.SetValue( 2 * NumFlops() * tPi );

					Start.SetReadOnly(true);
					Stop.SetReadOnly(true);

					if (debugQ("[ExpSCAN::updateScanSpan]", ""))
						cout << "[ExpSCAN::updateScanSpan] start = " << Start.Value() << " stop = " << Stop.Value() << endl;
				}
				else
				{
					Start.SetReadOnly(false);
					Stop.SetReadOnly(false);
				}
			}
		}


		bChanged |= Span.SetValue(Stop - Start);
	}

	return bChanged;
}

bool ExpSCAN::updateScanStep()
{
	if (IsLockInScan(ScanType))
		return false;
	else
		return Step.SetValue((Stop - Start) / (NumDataPoints - 1));
}

bool ExpSCAN::updateScanGUI(bool bForceUpdate)
{
	QWriteLocker lock(&page_lock);

	//figure out what should be scanned
	ScanSource* cSS = currentScanSource();

	//is the current scan source different from before?
	if (cSS != oldScanSource || bForceUpdate)
	{
		//if so, make sure any fit information is no longer used
		unused_fit = false;

		//if there was an earlier scan source, un-highlight this
		if (oldScanSource)
		{
			ParameterGUI_Base* pOld = oldScanSource->pParam;

			//this function changes the parameter background to indicate that it will be scanned or not
			//it results in a signal that causes the actual GUI update
			if (pOld)
				pOld->selectScanTarget(oldScanSource->getType(), false);
		}

		if (cSS)
		{
			//highlight the new scan source
			ParameterGUI_Base* pNew = cSS->pParam;

			if (pNew)
				pNew->selectScanTarget(cSS->getType(), true);
		}

		oldScanSource = cSS;

		if (cSS != 0)
		{
			//update the units for the new scan
			setScanUnit(cSS->GetUnit());

			//select start/stop or center/span mode depending on the type of scan (frequency/time/whatever)
			setScanMode(cSS->GetMode());

			return true;
		}
		else
			setScanMode(0);
	}

	return false;
}

ScanSource* ExpSCAN::currentScanSource()
{
	QWriteLocker lock(&page_lock);

	string ssType = ScanType;
	string ssName = ScanVariables;

	for (size_t i = 0; i < g_scan_sources.size(); i++)
	{
		if (g_scan_sources[i]->isCompatibleScanType(ScanType.Value()))
			if (g_scan_sources[i]->getName() == ssName)
				return g_scan_sources[i];
	}

	for (size_t i = 0; i < p_scan_sources.size(); i++)
	{
		if (p_scan_sources[i]->isCompatibleScanType(ScanType.Value()))
			if (p_scan_sources[i]->getName() == ssName)
				return p_scan_sources[i];
	}

	return 0;
}

scan_variable* ExpSCAN::currentScanVariable()
{
	if (pScan)
		return pScan->currentScanVariable();
	else
		return 0;
}

bool ExpSCAN::updateScanSourcesGUI(bool bForceUpdate)
{
	QWriteLocker lock(&page_lock);
	ScanSource* css = currentScanSource();

	if (css)
		if (css->getName() != ScanVariables.Value())
			bForceUpdate = true;

	if (bForceUpdate || (oldScanType != ScanType.Value()) )
	{
		bIgnoreGUISignals = true;

		oldScanType = ScanType.Value();
		string oSS = ScanVariables.Value();

		ScanVariables.clearChoices();

		//add globally available scan sources
		for (size_t i = 0; i < g_scan_sources.size(); i++)
		{
			if (g_scan_sources[i]->isCompatibleScanType(ScanType.Value()))
//            cout << "Add scan source " << g_scan_sources[i]->getName() << endl;
				ScanVariables.AddChoice(g_scan_sources[i]->getName());
		}

		//add scan sources for this page
		for (size_t i = 0; i < p_scan_sources.size(); i++)
		{
			if (p_scan_sources[i]->isCompatibleScanType(ScanType.Value()))
//			cout << "Add page scan source " << p_scan_sources[i]->getName() << endl;
				ScanVariables.AddChoice(p_scan_sources[i]->getName());
		}

		if (oSS.length())
			ScanVariables.SetValue(oSS);

		bIgnoreGUISignals = false;

		updateScanGUI(true);
		return true;
	}

	return false;
}

DDS_Pulse_Widget* ExpSCAN::DriveLine(const physics::line& l)
{
	GUI_int* pSB = dynamic_cast<GUI_int*>(FindParameter("Sideband"));
	GUI_double* pGS = dynamic_cast<GUI_double*>(FindParameter("Ground state"));
	GUI_int* pPol = dynamic_cast<GUI_int*>(FindParameter("Polarization"));

	if (pSB)
		pSB->SetValue(l.sb);

	if (pGS)
		pGS->SetValue(l.mFg);

	if (pPol)
		pPol->SetValue(l.Polarization());

	RecalculateParameters();

	return 0;
}



bool debug_lock = true;

ExpSCAN_L::ExpSCAN_L(const string& sPageName, ExperimentsSheet* pSheet,
                     const char* TargetStates, unsigned page_id) :
	ExpSCAN(sPageName, pSheet, TargetStates, page_id)
{
}

ExpSCAN_L::~ExpSCAN_L()
{
}

scan_variable* ExpSCAN_L::getLockVariable()
{
	return currentScanVariable();
}

void ExpSCAN_L::modulateOutput(double d)
{
	if (debug_lock)
		printf("[ExpSCAN_L::modOutput]  %12.9f\r\n", d);

	SetOutput(GetCenter() + d);
}

void ExpSCAN_L::SetOutput(double d)
{
	if (debug_lock)
		printf("[ExpSCAN_L::SetOutput]  %12.9f\r\n", d);

	return getLockVariable()->set_scan_position(d);
}

double ExpSCAN_L::GetOutput()
{
	return getLockVariable()->get_scan_position();
}

double ExpSCAN_L::GetModulation()
{
	RecalculateModulation();
	return Modulation;
}

void ExpSCAN_L::ShiftCenter(double d)
{
	SetCenter(GetCenter() + d);
}

void ExpSCAN_L::SetCenter(double d)
{
	if (d != GetCenter())
	{
		getLockVariable()->set_default_state(d);
		LockCenter.SetValue(d);
		LockCenter.PostUpdateGUI();

		char buff[128];
		sprintf(buff, "%s = %9.3f %s", GetLockVariableName().c_str(), d, pScan->GetScanUnit().c_str());
		pScan->getPlot()->setMarkerText(buff);
	}

	if (debug_lock)
		printf("[ExpSCAN_L::SetCenter (%f)]  %12.9f\r\n", fmod(CurrentTime_s(), 100), d);

}

double ExpSCAN_L::GetCenter()
{
	double d = getLockVariable()->get_default_state();

	if (debug_lock)
		printf("[ExpSCAN_L::GetCenter (%f)]  %12.9f\r\n", fmod(CurrentTime_s(), 100), d);

	return d;
}

double ExpSCAN_L::GetGain()
{
	return Gain;
}
int ExpSCAN_L::GetNumMeasurements()
{
	return NumMeasurements;
}

double ExpSCAN_L::GetSignal()
{
	return pSignalChannel->GetAverage();
}
std::string ExpSCAN_L::GetLockVariableName()
{
	return currentScanSource()->getName();
}

double ExpSCAN_L::GetInitialCenter()
{
	return GetCenter();
}
