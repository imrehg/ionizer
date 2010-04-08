#pragma once

/* RunObjects get created when an ExperimentPage is launched (the run button is presssed).
   They are destroyed when the ExperimentPage is stopped (stop is pressed or
   enough scans were completed, or an exception was thrown.

   Note that the terminology here is somewhat inconsistent.
   On the FPGA, an "experiment" is generally a single pulse sequence & detection.
   100 or so individual "experiments" are combined into a "DataPoint"
   "DataPoints" are combined into "scans"
 */

class RunObject
{
public:
RunObject(ExperimentBase* exp, RunObject* pOwner = 0);

virtual ~RunObject();

ExperimentBase::run_status RunTimeSlice(unsigned ms, bool bNeedInit);

//returns whether or not the thread should stop
bool ShouldStop();

//make this and all sub-threads stop ASAP
void Stop();

//called by scheduler to return the experiment to a good state
void DefaultExperimentState();

double RunTime() const;

void UserPause(bool);

void Owns(RunObject* ro)
{
	owned = ro;
}

bool IsUserPaused() const;
bool IsPreempted() const;

void SetFinished()
{
	bIsFinished = true;
}
bool IsFinished() const
{
	return bIsFinished;
}
bool OwnsActiveObject()
{
	return owned != 0;
}

void SetDataFileName(const std::string& name);
std::string GetDataFileName() const;
const std::string& GetOutputDirectory() const;
const std::string& GetName() const;
std::string GetOutputDirectoryDOS() const;

ExperimentBase* GetCurrentExperiment()
{
	return exp;
}

//   RunObject& operator=(const RunObject&);

time_t GetStartTime() const
{
	return ttStart;
}
std::string GetStartTimeS() const
{
	return stStart;
}

protected:
friend class ExperimentBase;
friend class ScanScheduler;

std::string CreateOutputDirectory();

ExperimentBase* exp;
RunObject* owner;
RunObject* owned;

double tStart, tLastRan;
time_t ttStart;
std::string stStart;

std::string OutputDirectory;
std::string DataFileName;
QDir working_dir;


double run_time;
private:
bool bStopNow;
bool bCreatedDirectory;
bool bIsFinished;

int pause_state;
int num_time_slices;
};

