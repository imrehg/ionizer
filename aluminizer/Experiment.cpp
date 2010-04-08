#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

//#include "AluminizerApp.h"
//#include "Console.h"
#include "InputParameters.h"
#include "ExperimentsSheet.h"
//#include "MgAlExperimentsSheet.h"
#include "Experiment.h"
#include "physics.h"
#include "Transition.h"

using namespace std;
using namespace physics;

ExperimentBase::ExperimentBase(const std::string& name) :
	pRunObject(0),
	name(name)
{
}

ExperimentBase::~ExperimentBase()
{

}

RunObject* ExperimentBase::RunOwnedThread(RunObject* pOwner)
{
	return pOwner->owned = new RunObject(this, pOwner);
}


std::string ExperimentBase::GetDataFileName()
{
	return GetName();
}
