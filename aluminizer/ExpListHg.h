#pragma once

#include "ExperimentPage.h"

//a list of experiments that all start at once when the user presses "run"

class ExpList : public ExperimentPage
{
public:
ExpList(const std::string& sPageName, ExperimentsSheet* pSheet);
virtual ~ExpList()
{
}

protected:
virtual run_status Run();

//called once when the experiment is first started
virtual void InitExperimentStart()
{
}

//called every time before the experiment continues, after another experiment was running
virtual void InitExperimentSwitch()
{
}

virtual void DefaultExperimentState()
{
}

//called once when the experiment has finished
virtual void FinishRun()
{
}


const static int num_experiments = 10;

vector< GUI_string* > exp_names;
};
