#pragma once

#include <trlib.h>

#include "Experiment.h"
#include "CriticalSection.h"

class ExperimentsSheet;


class ScanScheduler : public QThread
{
public:
ScanScheduler(ExperimentsSheet* pSheet);
~ScanScheduler();

//	void Start(); //starts up the scheduler
bool Stop();      //signals the scheduler to shut down.
                  //returns whether or not it has finished

void SetSheet(ExperimentsSheet* pSheet);

static bool IsShuttingDown()
{
	return bStopLoop;
}
bool IsRunning(ExperimentBase*, CriticalSectionOwner*);
bool IsUserPaused(ExperimentBase*, CriticalSectionOwner*);

void StartExperiments(const std::vector<ExperimentBase*>&, CriticalSectionOwner*);
void StartExperiment(ExperimentBase*, CriticalSectionOwner*);
void StopExperiment(RunObject*, CriticalSectionOwner*);
void FinishExperiment(ExperimentBase*, CriticalSectionOwner*);
void UserPauseExperiment(ExperimentBase*, CriticalSectionOwner*);
void UserContinueExperiment(ExperimentBase*, CriticalSectionOwner*);

void SetNextExperiment(RunObject*);

//place exp2 after exp1 in the running experiments list if they are both running
void MoveAfter(ExperimentBase* exp1, ExperimentBase* exp2, CriticalSectionOwner*);

void SetTimeSlice(unsigned t);

protected:
std::string RunningExpsString();

bool IsPreempted(ExperimentBase*, CriticalSectionOwner*);

//write running experiment names & dirs to o
void WriteExperiments(std::ostream& o, CriticalSectionOwner*);

RunObject* FindExperiment(ExperimentBase* exp, CriticalSectionOwner*);
RunObject* GetNextExperiment(RunObject* pCurrentScan);

std::vector<RunObject*> interleaved_experiments;
typedef std::vector<RunObject*>::iterator ie_iterator_t;

//	std::ofstream log;

private:
void run();

//	uintptr_t hThread;
//	unsigned threadID;

ExperimentsSheet* m_pSheet;
unsigned time_slice;
NamedCriticalSection csChangeInterleavedExperiments;
static bool bStopLoop;
};


