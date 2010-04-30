#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

//#include "AluminizerApp.h"
#include "Experiment.h"

using namespace std;

RunObject::RunObject(ExperimentBase* exp, RunObject* pOwner) :
	exp(exp),
	owner(pOwner),
	owned(0),
	tStart(CurrentTime_s()),
	tLastRan(0),
	ttStart(time(0)),
	OutputDirectory(pOwner ? pOwner->GetOutputDirectory() : CreateOutputDirectory()),
	working_dir(OutputDirectory.c_str()),
	run_time(0),
	bStopNow(false),
	bCreatedDirectory(pOwner ? false : true),
	bIsFinished(false),
	pause_state(2),
	num_time_slices(0)
{
	exp->pRunObject = this;

	cerr << "RunObject::RunObject() entry " << exp->GetName() << endl;

	SetDataFileName(exp->GetDataFileName());

	if (pOwner)
	{
		cerr << exp->GetName() << " is owned by " << pOwner->exp->GetName() << endl;

//		theApp->console.OpenLogFile(OutputDirectory + exp->GetDataFileName() + "_log.txt");
	}
	else
		cerr << exp->GetName() << " is unowned." << endl;

	if (owner)
		owner->Owns(this);

	cerr << "[RunObject::RunObject] exit " << exp->GetName() << endl;
}


RunObject::~RunObject()
{
	cerr << "RunObject::~RunObject() entry " << exp->GetName() << endl;

	if(num_time_slices == 0)
		exp->InitExperimentStart();

	if (owned)
	{
		delete owned;
		owned = 0;
	}

	DefaultExperimentState();

	if (exp)
	{
		exp->FinishRun();
		exp->PostFinish();
	}

	Stop();

	if (owner)
		owner->Owns(0);

	//remove empty directories
	if (bCreatedDirectory)
		working_dir.rmdir(OutputDirectory.c_str());

	if (!owner)
	{
//		theApp->console.CloseLogFile();
//		theApp->console.ConsoleOutputOn();
	}

	cerr << "RunObject::~RunObject() exit " << exp->GetName() << endl;
	cout << "Finished experiment: " << exp->GetName() << endl;
}



bool RunObject::ShouldStop()
{
	if (owner)
		return owner->ShouldStop();
	else
		return bStopNow;
}

void RunObject::Stop()
{
	bStopNow = true;
	SetFinished();
}

//bPause = true for pause, false for continue
void RunObject::UserPause(bool bPause)
{
	pause_state = bPause ? 1 : 2;
}


bool RunObject::IsUserPaused() const
{
	return pause_state == 1;
}

bool RunObject::IsPreempted() const
{
	return pause_state == 2;
}


double RunObject::RunTime() const
{
	return CurrentTime_s() - tStart;
}


//*****************************************************************
//create the output directory
std::string RunObject::CreateOutputDirectory()
{
	time_t now = ttStart;

	char default_dir[] = ".";
	char* data_dir = getenv("ALUMINIZER_DIR");

	if (data_dir == 0)
		data_dir = default_dir;

	stStart = GetDateTimeString(now);
	QDir wd((string(data_dir) + "/archives/" + GetDateString(now, true) + "/" + stStart).c_str());
	wd.mkpath(wd.absolutePath());
	wd.cd(wd.absolutePath());

	std::string dir( wd.path().toAscii() );
	cout << "Created dir: " << dir.c_str() << endl;


	cout << "Writing data to directory " << dir << endl;

	return dir + "/";
}


const std::string& RunObject::GetOutputDirectory() const
{
	return OutputDirectory;
}

const string& RunObject::GetName() const
{
	return exp->GetName();
}


std::string RunObject::GetDataFileName() const
{
	return OutputDirectory + DataFileName;
}

void RunObject::SetDataFileName(const std::string& name)
{
	DataFileName = name;
}


//return output directory with / characters replaced by \ for DOS commands
std::string RunObject::GetOutputDirectoryDOS() const
{
	string dirDOS = OutputDirectory;

	replace(dirDOS.begin(), dirDOS.end(), '/', '\\');

	return dirDOS;
}


void RunObject::DefaultExperimentState()
{
	if (owned)
		owned->DefaultExperimentState();

	cout << "Reverting to default state from: " << exp->GetName() << endl;
	exp->DefaultExperimentState();
}

ExperimentBase::run_status RunObject::RunTimeSlice(unsigned ms, bool bNeedInit)
{
	if (num_time_slices == 0)
	{
		cerr << "[RunObject::RunTimeSlice] is calling " << exp->GetName() << "::InitExperimentStart" << endl;
		cout << "Initializing experiment: " << exp->GetName() << endl;
		exp->InitExperimentStart();

		bNeedInit = true;
	}


	double t0 = CurrentTime_s();
	tLastRan = t0;

	ExperimentBase::run_status r = ExperimentBase::IDLE;

	while (!IsFinished() && !ShouldStop() )
	{
		num_time_slices++;

		if (owned)
		{
			if ( owned->IsFinished() )
			{
				delete owned;
				owned = 0;

				bNeedInit = true;
			}
			else
				owned->RunTimeSlice(ms, bNeedInit);
		}

		if (!owned)
		{
			if (bNeedInit)
			{
				cerr << "[RunObject::RunTimeSlice] is calling " << exp->GetName() << "::InitExperimentSwitch" << endl;
				cout << "Switching to experiment: " << exp->GetName() << endl;
				exp->InitExperimentSwitch();

				bNeedInit = false;
			}

			r = exp->Run();

			if (r == ExperimentBase::FINISHED)
				SetFinished();

			if (r == ExperimentBase::IDLE)
				break;

			if (r == ExperimentBase::NEED_MORE_DATA)
				continue;

			double dt = ms - 1000 * (CurrentTime_s() - t0);

			if (dt <= 0)
				break;
		}
	}

	run_time += CurrentTime_s() - t0;

	return r;
}

