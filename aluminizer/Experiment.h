#pragma once

#include <string>

class RunObject;

class ExperimentBase
{
public:
ExperimentBase(const std::string& Name);
virtual ~ExperimentBase();

friend class ExperimentSheet;
friend class RunObject;

public:
RunObject* RunOwnedThread(RunObject* pOwner = 0);

//return the data file name (w/o extension)
virtual std::string GetDataFileName();

virtual unsigned getPriority()
{
	return 1;
}

const std::string& GetName() const
{
	return name;
}

//called once when the experiment is first started
virtual void InitExperimentStart() = 0;

//called every time before the experiment continues, after another experiment was running
virtual void InitExperimentSwitch() = 0;

enum run_status { IDLE, FINISHED, OK, NEED_MORE_DATA };

//called every time the experiment should continue, possibly several times in sequence
virtual run_status Run() = 0;

//called every time the experiment should pause.
//put the apparatus back into a useful state here so other experiments can run
virtual void DefaultExperimentState() = 0;

//called once when the experiment has finished
virtual void FinishRun() = 0;

virtual void PostInitialize()
{
}
virtual void PostFinish()
{
}

virtual bool DeferRun()
{
	return false;
}
virtual bool PreemptOK() const
{
	return true;
}

protected:

//throw a QuitMain() exception to quit from main without further consequences
class QuitMain {};



private:

protected:

class LostIons
{
public:
LostIons(double counts) : counts(counts)
{
}
double counts;
};

protected: /* protected data */

friend class ScanScheduler;

RunObject* pRunObject;

std::string name;
};


#include "RunObject.h"
