#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "AluminizerApp.h"
#include "ScanScheduler.h"
//#include "ExpSCAN.h"
//#include "AxialPosition.h"
#include "ExperimentsSheet.h"

#include "Numerics.h"
#include "Transition.h"
#include "physics.h"

using namespace std;

bool ScanScheduler::bStopLoop = false;

ScanScheduler::ScanScheduler(ExperimentsSheet* pSheet) :
//log(("scheduler_" + g_t0s + ".csv").c_str()),
	m_pSheet(pSheet),
	time_slice(2000),
	csChangeInterleavedExperiments("ScanScheduler::csChangeInterleavedExperiments")
{
}

ScanScheduler::~ScanScheduler()
{
	//stop scheduler thread
	Stop();
}

void ScanScheduler::SetSheet(ExperimentsSheet* pSheet)
{
	m_pSheet = pSheet;
}

bool ScanScheduler::Stop()
{
	//stop scheduler thread
	bStopLoop = true;
	return wait(ULONG_MAX);
}

std::string ScanScheduler::RunningExpsString()
{
	std::string s;

	if (interleaved_experiments.size())
	{
		for (size_t i = 0; i < interleaved_experiments.size(); i++)
		{
			if (i > 0)
				s = s + ", ";

			s = s + interleaved_experiments[i]->GetName();
		}
	}
	else
		s = "none";

	return s;
}

void ScanScheduler::run()
{
	ScanScheduler* ss = this;

	RunObject* pCurrentExp = 0;

	bool bNeedInit = true;

	while ( !bStopLoop )
	{
		if (ss->m_pSheet)
		{
			string s_running;

			if (ss->csChangeInterleavedExperiments.try_enter())
			{
				s_running = "Running: " + ss->RunningExpsString();
				ss->csChangeInterleavedExperiments.leave();
			}

			if (!s_running.empty())
				ss->m_pSheet->SetStatusBarText(s_running);
		}

		//if there is a current experiment, run it
		if (pCurrentExp)
		{
			try
			{
//				AluminizerPage* pPage = dynamic_cast<AluminizerPage*>(pCurrentExp->exp);
//				ss->log << setprecision(3) << fixed << CurrentTime_s() << ", " << pPage->GetID() << ", "   << endl;

				ExperimentBase::run_status r = pCurrentExp->RunTimeSlice(ss->time_slice, bNeedInit);

				switch (r)
				{
				case ExperimentBase::IDLE: cout << pCurrentExp->GetName() << " is IDLE" << endl; break;
				case ExperimentBase::OK: cout << pCurrentExp->GetName() << " is OK" << endl; break;
				case ExperimentBase::FINISHED: cout << pCurrentExp->GetName() << " is FINISHED" << endl; break;
				case ExperimentBase::NEED_MORE_DATA: cout << pCurrentExp->GetName() << " is NEED_MORE_DATA" << endl; break;
				}
			}
			catch (Uninitialized u) {
				cout << "[ScanScheduler] has caught the following exception" << endl;
				cout << "\twhile running " << pCurrentExp->GetName() << endl;
				cout << "\t\t" << u << endl;

				pCurrentExp->SetFinished();
			}
			catch (const char* szException) {
				cout << "[ScanScheduler] has caught the following exception" << endl;
				cout << "\twhile running " << pCurrentExp->GetName() << endl;
				cout << "\t\t" << szException << endl;
				cout << "\t\t\texiting main..." << endl;

				pCurrentExp->SetFinished();
			}
			catch (runtime_error re)
			{
				cout << "[ScanScheduler] has caught the following runtime error" << endl;
				cout << "\twhile running " << pCurrentExp->GetName() << endl;
				cout << re.what() << endl;

				pCurrentExp->SetFinished();
			}
			catch (physics::ElectronicTransition::IllegalTransition i)
			{
				cout << "[ScanScheduler] has caught the following exception:" << endl;
				cout << "\twhile running " << pCurrentExp->GetName() << endl;
				cout << "\t\t" << i << endl;

				pCurrentExp->SetFinished();
			}
			catch (physics::ElectronicTransition::IllegalPiTimeRatio i)
			{
				cout << "[ScanScheduler] has caught the following exception:" << endl;
				cout << "\twhile running " << pCurrentExp->GetName() << endl;
				cout << "\t\t" << i << endl;

				pCurrentExp->SetFinished();
			}


			//if the experiment is done remove it from the list
			if (pCurrentExp->IsFinished())
			{
				ss->StopExperiment(pCurrentExp, 0);
				pCurrentExp = 0;
			}
		}
		else
			msleep(100);

		{
			//if the scan that just finished isn't a save ions scan then run save ions
			//unless there aren't any scans at all
			if (!bStopLoop)
			{
				RunObject* pNextExp = ss->GetNextExperiment(pCurrentExp);

				if (pCurrentExp)
					cerr << "[ScanScheduler::RunScans] pCurrentExp = " << pCurrentExp->GetName() << endl;
				//else
				//	cerr << "[ScanScheduler::RunScans] pCurrentExp = " << 0 << endl;

				if (pNextExp)
					cerr << "[ScanScheduler::RunScans] pNextExp = " << pNextExp->GetName() << endl;
				//else
				//	cerr << "[ScanScheduler::RunScans] pNextExp = " << 0 << endl;

				if (pNextExp != 0 && pCurrentExp != pNextExp)
				{
					if (pCurrentExp != 0)
						pCurrentExp->DefaultExperimentState();

					bNeedInit = true;

					pCurrentExp = pNextExp;
				}
				else
					bNeedInit = false;
			}
		}
	}

	cerr << "[ScanScheduler::RunScans] exiting scheduler loop" << endl;

	{

		CriticalSectionOwner cso(&ss->csChangeInterleavedExperiments, 0);

		if (pCurrentExp)
			pCurrentExp->DefaultExperimentState();

		while (ss->interleaved_experiments.size())
		{
			delete ss->interleaved_experiments.back();
			ss->interleaved_experiments.pop_back();
		}
	}
}

RunObject* ScanScheduler::GetNextExperiment(RunObject* pCurrentExp)
{
	//if save ions is at the back of the list run that
	if (!interleaved_experiments.size())
		//if the list is empty return 0
		return 0;

	if (pCurrentExp)
		if (!pCurrentExp->GetCurrentExperiment()->PreemptOK())
			return pCurrentExp;

	//make a prioritized list
	vector<double> priorities(interleaved_experiments.size());

	double t = CurrentTime_s();

	for (unsigned i = 0; i < interleaved_experiments.size(); i++)
	{
		double p = interleaved_experiments[i]->GetCurrentExperiment()->getPriority();

		double dt = t - interleaved_experiments[i]->tLastRan;
		priorities[i] = dt / (1 + p);
	}

	size_t j = max_element(priorities.begin(), priorities.end()) - priorities.begin();
	return interleaved_experiments[j];
}

RunObject* ScanScheduler::FindExperiment(ExperimentBase* exp, CriticalSectionOwner* pcso)
{
	CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

	for (size_t i = 0; i < interleaved_experiments.size(); i++)
		if (interleaved_experiments[i]->exp == exp)
			return interleaved_experiments[i];

	return 0;
}

//move exp2 after exp1 in interleaved_experiments if they both exist
void ScanScheduler::MoveAfter(ExperimentBase* exp1, ExperimentBase* exp2, CriticalSectionOwner* pcso)
{
	CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

	if (RunObject * pRO1 = FindExperiment(exp1, pcso))
	{
		if (RunObject * pRO2 = FindExperiment(exp2, pcso))
		{
			vector<RunObject*>::iterator it1 = find(interleaved_experiments.begin(), interleaved_experiments.end(), pRO1);
			interleaved_experiments.insert(it1, pRO2);

			vector<RunObject*>::iterator it2 = find(interleaved_experiments.begin(), interleaved_experiments.end(), pRO2);
			interleaved_experiments.erase(it2);
		}
	}
}

bool ScanScheduler::IsRunning(ExperimentBase* exp, CriticalSectionOwner* pcso)
{
	return FindExperiment(exp, pcso) != 0;
}

bool ScanScheduler::IsUserPaused(ExperimentBase* exp, CriticalSectionOwner* pcso)
{
	CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

	if (RunObject * pRO = FindExperiment(exp, &cso))
		return pRO->IsUserPaused();
	else
		return false;
}

void ScanScheduler::WriteExperiments(std::ostream& o, CriticalSectionOwner* pcso)
{
	CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

	for (size_t i = 0; i < interleaved_experiments.size(); i++)
		o << interleaved_experiments[i]->GetDataFileName() << endl;
}

void ScanScheduler::StartExperiment(ExperimentBase* exp, CriticalSectionOwner* pcso)
{
	if (exp)
	{
		vector<ExperimentBase*> exps(1, exp);
		StartExperiments(exps, pcso);
	}
}

void ScanScheduler::FinishExperiment(ExperimentBase* exp, CriticalSectionOwner*)
{
	if (exp)
		exp->pRunObject->Stop();
}


void ScanScheduler::StartExperiments(const vector<ExperimentBase*>& exps, CriticalSectionOwner* pcso)
{
	if (exps.size() > 0)
	{
		CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

		for (size_t i = 0; i < exps.size(); i++)
		{
			if (IsRunning(exps[i], &cso))
				continue;


			cerr << "[ScanScheduler::StartExperiment] " << exps[i]->GetName() << endl;

			try
			{
				RunObject* pRO = new RunObject(exps[i], 0);
				interleaved_experiments.push_back(pRO);
				interleaved_experiments.back()->exp->PostInitialize();

				string fname = interleaved_experiments.back()->GetOutputDirectory() + "RunningExperiments.txt";
				ofstream exp_list(fname.c_str());
				WriteExperiments(exp_list, &cso);

			}
			catch (Uninitialized u) {
				cout << "[ScanScheduler::StartExperiment] has caught the following exception" << endl;
				cout << "\twhile running " << exps[i]->GetName() << "::main" << endl;
				cout << "\t\t" << u << endl;
				cout << "\t\t\taborting experiment" << endl;
			}
			catch (const char* szException) {
				cout << "[ScanScheduler::StartExperiment] has caught the following exception" << endl;
				cout << "\twhile initiliazing " << exps[i]->GetName() << ":  " << endl;
				cout << "\t\t" << szException << endl;
				cout << "\t\t\taborting experiment" << endl;
			}
			catch (runtime_error re)
			{
				cout << "[ScanScheduler::StartExperiment] has caught the following runtime error" << endl;
				cout << "\twhile initiliazing " << exps[i]->GetName() << ":  " << endl;
				cout << re.what() << endl;
				cout << "\t\t\taborting experiment" << endl;
			}
			catch (physics::ElectronicTransition::IllegalTransition t)
			{
				cout << "[ScanScheduler::StartExperiment] has caught the following exception" << endl;
				cout << "\twhile initiliazing " << exps[i]->GetName() << ":  " << endl;
				cout << "\t\t" << t << endl;
				cout << "\t\t\taborting experiment" << endl;
			}
			catch (physics::ElectronicTransition::IllegalPiTimeRatio p)
			{
				cout << "[ScanScheduler::StartExperiment] has caught the following exception" << endl;
				cout << "\twhile initiliazing " << exps[i]->GetName() << ":  " << endl;
				cout << "\t\t" << p << endl;
				cout << "\t\t\taborting experiment" << endl;
			}
		}
	}
}

void ScanScheduler::StopExperiment(RunObject* pRO, CriticalSectionOwner* pcso)
{
	CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

	ie_iterator_t i = find(interleaved_experiments.begin(), interleaved_experiments.end(), pRO);

	assert(i != interleaved_experiments.end());
	interleaved_experiments.erase(i);

	delete pRO;
}

void ScanScheduler::UserPauseExperiment(ExperimentBase* exp, CriticalSectionOwner* pcso)
{
	CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

	if (RunObject * pRO = FindExperiment(exp, &cso))
		pRO->UserPause(true);
	else
		cerr << "[ScanScheduler::PauseExperiment] unknown experiment" << endl;
}

//continue means set pause state from user-pause to preempted, so that the
//scan can continue normally
void ScanScheduler::UserContinueExperiment(ExperimentBase* exp, CriticalSectionOwner* pcso)
{
	CriticalSectionOwner cso(&csChangeInterleavedExperiments, pcso);

	if (RunObject * pRO = FindExperiment(exp, &cso))
		pRO->UserPause(false);
	else
		cerr << "[ScanScheduler::ContinueExperiment] unknown experiment" << endl;
}

void ScanScheduler::SetTimeSlice(unsigned t)
{
	time_slice = t;
}
