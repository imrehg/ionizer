#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ScanObject.h"
#include "AluminizerApp.h"
#include "CommonExperimentsSheet.h"
#include "ExpSCAN.h"
#include "ExpSCAN_NewFPGA.h"
#include "TransitionPage.h"
#include "scan_variable.h"
#include "TransitionPage.h"
#include "FPGA_connection.h"
#include "data_plot.h"
#include "histogram_plot.h"

using namespace std;
using namespace numerics;



Scan_Base* Scan_Base::pCurrentScan(0);

Scan_Base::Scan_Base(ExpSCAN* scanPage, vector<scan_variable*> scan_variables) :
	ScanPage(scanPage),
	pRunObject(ScanPage->pRunObject),
	dataFileName(pRunObject->GetOutputDirectory() + ScanPage->GetDataFileName()),
	dataFile(GetDataFileName().c_str()),
	debug_log_fname(dataFileName + "_dbg.txt"),
	debug_log_file(debug_log_fname.c_str()),
	scan_variables(scan_variables),
	total_points(0),
	shots_log((dataFileName + "_shots_log.txt").c_str()),
	ReplotModulo(theApp->m_pExperimentsSheet->m_GlobalsPage.ReplotModulo),
	minCounts(0),
	maxCounts(numeric_limits<double>::min()),
	PlotPointsSaved(0),
	num_replots(0),
	validData(0),
	bUnprepared(true),
	runStatus(ExperimentBase::OK),
	recorded_names(false),
	plot(scanPage->get_data_plot()),
	hist_plot(scanPage->get_hist_plot()),
	bNeedSetupPlot(true)
{
	if (plot)
	{
		plot->set_start_time(pRunObject->GetStartTime());
		plot->set_save_file_name(dataFileName);
	}

	//setup data feed
	ScanPage->AddDataChannels(data_feed);

	data_feed.Clear();
}

Scan_Base::~Scan_Base()
{
	while (!scan_variables.empty())
	{
		delete scan_variables.back();
		scan_variables.pop_back();
	}
}

scan_variable* Scan_Base::currentScanVariable()
{
	if (scan_variables.size())
		return scan_variables[0];
	else
		return 0;
}


void Scan_Base::SetupPlot()
{
	if (plot)
	{
		plot->set_xrange(GetXmin(), GetXmax()); //crashes here
		plot->set_xlabel(GetXlabel());
		plot->set_xoffset(GetXoffset());

		addPlotCurves();
	}
}

void Scan_Base::addPlotCurves()
{
	for (unsigned i = 0; i < data_feed.NumChannels(); i++)
	{
		if (data_feed.ShouldPlot(i))
		{
			Qt::PenStyle ps = Qt::SolidLine;

			if (data_feed.GetName(i).find("(.)") != string::npos)
				ps = Qt::DotLine;

			if (data_feed.GetName(i).find("(s)") != string::npos)
				plot->addCurve(data_feed.GetName(i).c_str());                  //plot thick
			else
				plot->addCurve(data_feed.GetName(i).c_str(), true, 0, ps);     //plot thin
		}
	}
}

void Scan_Base::SaveValidData()
{
	vector<ostream*> dataFiles;
	dataFiles.push_back(&dataFile);

	if (!recorded_names)
	{
		data_feed.RecordChannelNames(dataFile);
		dataFile << endl;

		recorded_names = true;
	}

	int NewPoints = data_feed.SaveValidData(dataFiles, &validData);

	PlotPointsSaved += NewPoints;

	if (validData.size() && NewPoints)
	{
		for (size_t i = validData.size() - NewPoints; i < validData.size(); i++)
			for (size_t j = 1; j < validData[i].size(); j++)
				if (data_feed.ShouldPlot(j))
				{
					maxCounts = std::max(maxCounts, validData[i][j]);
					minCounts = std::min(minCounts, validData[i][j]);
				}

		for_each(dataFiles.begin(), dataFiles.end(), mem_fun(&ostream::flush));
	}
}

bool Scan_Base::CanFit()
{
	return ScanPage->WantsFit();
}

void Scan_Base::PlotLatestData()
{
	if (plot != 0 && validData.size())
	{
		if (bNeedSetupPlot)
		{
			SetupPlot();
			bNeedSetupPlot = false;
		}

		double x = validData.back()[0];

		//	double y[1];
		//	y[0] = validData.back()[2];
		//	y[1] = validData.back()[1];

		plot->addX(x);

		for (unsigned i = 0; i < data_feed.NumChannels(); i++)
			if (data_feed.ShouldPlot(i))
				plot->addY(validData.back()[i], data_feed.GetName(i).c_str());

		if ((total_points % ReplotModulo) == 0)
			plot->replot();
	}

	if (hist_plot)
		if (HistogramChannel * pmtHist = dynamic_cast<HistogramChannel*>(ScanPage->pSignalChannel))
			hist_plot->Plot(pmtHist->GetHistogram());
}

void Scan_Base::InitializeScan()
{
	ScanPage->InitializeScan();

	StartScan();

	tStart = CurrentTime_s();

	bUnprepared = true;  //whether or not the ions need to be re-prepared
}

ExperimentBase::run_status Scan_Base::DataPoint()
{
	if ( !IsFinished() )
	{
		//return idle if we don't need more data
		if (runStatus == ExperimentBase::IDLE)
		{
			runStatus = ExperimentBase::OK;
			return ExperimentBase::IDLE;
		}

		total_points++;

		//update all scan variables for this scan point, e.g. set new FPGA times and freq's
		//or new electrode voltages
		double x = GetScanVariable();

		if (isScanning()) //continous and lock-in type scans do not update here
		{
			//update all scan_variables
			//the FPGA based f/t/p-scans are updated via scan_variables as well
			for (size_t i = 0; i < scan_variables.size(); i++)
				scan_variables[i]->set_scan_position(x);
		}

		//let the ScanPage do whatever it wants before the data is acquired
		ScanPage->PreAcquireDataPoint(this, data_feed);

		//run the experiment(s) on the FPGA
		AcquireDataPoint();

		//let the ScanPage do whatever it wants after the data is acquired
		ScanPage->PostAcquireDataPoint(this, data_feed);

		//do something useful with the data
		UseValidData();

		//move to the next data point
		NextDataPoint();

		num_current_points++;
	}

	if (IsFinished())
	{
		pRunObject->SetFinished();

		DefaultExperimentState();

		return ExperimentBase::FINISHED;
	}

	return ExperimentBase::OK;
}

//default implementation plots and saves all valid data.
//override this virtual function for more complicated behavior
void Scan_Base::UseValidData()
{
	SaveValidData();
	PlotLatestData();
}

//todo: rename this to PrepareIons
//tells the ScanPage to do what is necessary to prepare the ions.
//then sets up the FPGA to continue the current scan
void Scan_Base::InitExperimentSwitch()
{
	num_current_points = 0;
}


void Scan_Base::debugMsg(const std::string& s)
{
	if (debug_log_file.is_open())
	{
		debug_log_file << s.c_str();
		debug_log_file.flush();
	}
}

void Scan_Base::AcquireDataPoint()
{
	//start a new data point for averaging
	data_feed.StartNewAverage( GetScanVariable() );

	//pCurrentScan is a static class member which allows the static member function
	//FPGASingleShotInterruptCallBack to refer back to this object
	pCurrentScan = this;


//	unsigned n = ScanPage->getNumExp();

	//run NumExperiments shots of the experiment on the FPGA
	//after every shot FPGASingleShotInterruptCallBack is called
	ExpSCAN_NewFPGA* pNewScan = dynamic_cast<ExpSCAN_NewFPGA*>(ScanPage);


	if (pNewScan) //all scans are "NewFPGA" scans now...don'tneed this legacy distinction
	{
		pNewScan->UpdateScanPulses(GetScanVariable());

		//on the first data point extract the pulse sequence (debug info)
		//and ignore the results
		if (total_points == 1 && pNewScan->DebugFirstExp.Value())
			theApp->fpga->GenericExperiment(this, pNewScan->get_page_id(), EXP_FLAG_DEBUG, pNewScan->channelData);

		//run the experiments on the FPGA
		theApp->fpga->GenericExperiment(this, pNewScan->get_page_id(), 0, pNewScan->channelData);


		for (unsigned i = 0; i < pNewScan->channelData.size(); i++)
		{
			pNewScan->pFPGAchannels[i]->SetCurrentData(pNewScan->channelData[i]);
			HistogramChannel* pHist = dynamic_cast<HistogramChannel*>(pNewScan->pFPGAchannels[i]);

			if (pHist)
				pHist->SetHistogram(theApp->fpga->getResultsVector());
		}

		data_feed.AcquireSingleShot();

		if (pNewScan->LogShots.Value())
		{
			vector<unsigned> results = theApp->fpga->getResultsVector();

			shots_log << CurrentIndex() << ", " << GetScanVariable() << ", " << results.size();

			for (size_t i = 0; i < results.size(); i++)
				shots_log << ", " << results[i];

			shots_log << endl;
		}
	}
}

//return whether or not individual points should be plotted
//gnuplot gets bogged down after a while, so don't plot more than 1000
bool Scan_Base::PlotPoints(int nPoints)
{
	return nPoints < 1000;
}

void Scan_Base::DefaultExperimentState()
{
	for (size_t i = 0; i < scan_variables.size(); i++)
		scan_variables[i]->goto_default_state();
}

Scan_Fittable::Scan_Fittable(ExpSCAN* scanPage, vector<scan_variable*> scan_variables) :
	Scan_Base(scanPage, scan_variables),
	indeces(0),
	x(ScanPage->NumDataPoints),
	current_index(0),
	ScatterScan(ScanPage->IsScatterScan()),
	FitMinContrast(ScanPage->FitMinContrast),
	FitMinAmplitude(ScanPage->FitMinAmplitude),
	scans_completed(0)
{
	if (!NumDataPoints())
		throw "Can't scan with NumDataPoints = 0.";

	for (size_t i = 0; i < x.size(); i++)
		x[i] = ScanPage->Start + ScanPage->Step * i;
}

Scan_Fittable::~Scan_Fittable()
{
}

ExperimentBase::run_status Scan_Fittable::DataPoint()
{
	if (indeces.size())
	{
		ScanPage->progress.SetValue(current_index / (double)(indeces.size()) );
		ScanPage->ScanTime.SetValue((CurrentTime_s() - tStart) * (double)(indeces.size()) / current_index );
	}

	if ( current_index == indeces.size() )
	{
		scans_completed++;

		if (!IsFinished())
			InitializeScan();
	}

	if ( current_index == 0 )
		ScanPage->progress.set_display_label("Scan " + to_string<unsigned>(scans_completed + 1) + "/"  + to_string<unsigned>(ScanPage->NumScans.Value()));

	ScanPage->progress.PostUpdateGUI();
	ScanPage->ScanTime.PostUpdateGUI();

	return Scan_Base::DataPoint();
}



numerics::FitObject* Scan_Fittable::RunFit(ExpSCAN* pScanPage)
{
	numerics::FitObject* pFit = CreateFitObject();

	if (pFit->DoFit())
	{
		if (pScanPage)
			pFit->UpdateScanPage(pScanPage);

		if (plot)
		{
			pFit->PlotFit(plot, GetXoffset(), GetXscale());
			plot->replot(true);
		}
	}

	return pFit;
}

int Scan_Fittable::NumDataPoints() const
{
	return static_cast<int>(x.size());
}

void Scan_Fittable::AddPointsAt(double f, unsigned n)
{
	x.resize(x.size() + n, f);
}

void Scan_Fittable::AddPointsRange(double f1, double f2, unsigned n)
{
	if (n <= 1)
		return;

	size_t oldSize = x.size();
	x.resize(oldSize + n);

	for (size_t i = 0; i < n; i++)
		x[oldSize + i] = f1 + i * (f2 - f1) / (n - 1);
}

double Scan_Fittable::ScanMax() const
{
	if (x.size())
		return *max_element(x.begin(), x.end()) ;
	else
		return 1;
}

double Scan_Fittable::ScanMin() const
{
	if (x.size())
		return *min_element(x.begin(), x.end()) ;
	else
		return 0;
}

double Scan_Fittable::ScanCenter() const
{
	return 0.5 * ( ScanMax() +  ScanMin() );
}

double Scan_Fittable::ScanSpan() const
{
	return ScanMax() -  ScanMin();
}

double Scan_Fittable::GetXmin()
{
	return (ScanMin() - GetXoffset()) / GetXscale();
}

double Scan_Fittable::GetXmax()
{
	return (ScanMax() - GetXoffset()) / GetXscale();
}

void Scan_Fittable::StartScan()
{
	current_index = 0;

	validData.reserve(validData.size() + x.size());

	indeces.resize(x.size());

	for (size_t i = 0; i < indeces.size(); i++)
		indeces[int(i)] = int(i);

	if (ScatterScan)
		random_shuffle(indeces.begin(), indeces.end());
}

void Scan_Fittable::NextDataPoint()
{
	current_index++;
}

size_t Scan_Fittable::CurrentIndex() const
{
	return indeces.at(current_index);
}

bool Scan_Fittable::IsFinished() const
{
	return (scans_completed >= ScanPage->NumScans.Value()) || pRunObject->ShouldStop();
}

bool Scan_Fittable::UseDots() const
{
	return x.size() > 200;
}

double Scan_Fittable::GetScanVariable()
{
	return x.at(CurrentIndex());
}

ContinuousScan::ContinuousScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables) :
	Scan_Base(scanPage, scan_variables)
{
	ReplotModulo = 1;

	QDir d("temp");
	d.mkdir("temp");
}

ContinuousScan::~ContinuousScan()
{
	if (plotDataFile.is_open())
		plotDataFile.close();

	while (!temp_file_names.empty())
	{
		remove(temp_file_names.back().c_str());
		temp_file_names.pop_back();
	}
}

void ContinuousScan::PlotLatestData()
{
	if (plot)
		plot->set_xrange(GetXmin(), GetXmax());

	Scan_Base::PlotLatestData();
}
void ContinuousScan::StartScan()
{
	tStart = CurrentTime_s();
	num_points = 0;
}

double ContinuousScan::GetScanVariable()
{
	if (UsesTime())
		return CurrentTime_s() - tStart;
	else
		return num_points;
}

double ContinuousScan::GetXmin()
{
	if (UsesTime())
		return std::max(GetScanVariable() - 10.0, 0.0);
	else
		return std::max(GetScanVariable() - 50.0, 0.0);

}

double ContinuousScan::GetXmax()
{
	return GetScanVariable();
}

NPointScan::NPointScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables, int max_points)
	: ContinuousScan(scanPage, scan_variables),
	max_points(max_points)
{
}

bool NPointScan::IsFinished() const
{
	return total_points >= max_points;
}


SourceScan::SourceScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables, ScanSource* ss) :
	Scan_Fittable(scanPage, scan_variables),
	ss(ss)
{
}

string SourceScan::GetXlabel()
{
	return ss->getXlabel();
}


void SourceScan::DefaultExperimentState()
{
	cout << "Revert to default state: "  << ScanPage->GetName() << endl;

	Scan_Fittable::DefaultExperimentState();
	ss->setDefaultValue();
}


numerics::FitObject* SourceScan::CreateFitObject()
{
	if (ss->getName() == "Mg Detect(T)" && ScanPage->Title() == "Repump" )
		return new FitRepump(&validData, 0, ScanPage->FitYColumn(),
		                     pRunObject->GetOutputDirectory());

	if (ss->fitsLorentzian())
		return new FitLorentzian(&validData, 0, ScanPage->FitYColumn(),
		                         FitMinContrast, FitMinAmplitude, pRunObject->GetOutputDirectory());

	if (FScanSource * fSS = dynamic_cast<FScanSource*>(ss))
		return new FitRabiFreq(&validData, 0, ScanPage->FitYColumn(),
		                       FitMinContrast, FitMinAmplitude, fSS->getPulseTime(), pRunObject->GetOutputDirectory());

	if (dynamic_cast<TScanSource*>(ss) || ss->fitsSine())
		return new FitRabiTime(&validData, 0, ScanPage->FitYColumn(),
		                       FitMinContrast, FitMinAmplitude, pRunObject->GetOutputDirectory());

	return 0;
}

bool SourceScan::CanFit()
{
	return true;
}

bool IsFreqScan(const string& s)
{
	return s == "Frequency";
}

bool IsSmartFrequencyScan(const string& s)
{
	return s == "SmartF";
}

bool IsRamseyFScan(const string& s)
{
	return s == "RamseyF";
}

bool IsRamseyPScan(const string& s)
{
	return s == "RamseyP";
}

bool IsTimeScan(const string& s)
{
	return s == "Time";
}

bool IsPhaseScan(const string& s)
{
	return s == "Phase";
}

bool IsContinuousScan(const string& s)
{
	return s == "Continuous";
}

bool IsNPointScan(const string& s)
{
	return s == "NPoint";
}

bool IsLockScan(const string& s)
{
	return s == "Lock";
}

bool IsLockInScan(const string& s)
{
	return s == "LockIn";
}

