#ifdef PCH
#include "common.h"
#endif

#include "ExpList.h"

ExpList::ExpList(const std::string& sPageName, ExperimentsSheet* pSheet) :
	ExperimentPage(sPageName, pSheet, 999),
	exp_names(num_experiments)
{
	for (int i = 0; i < num_experiments; i++)
	{
		exp_names[i] = new GUI_string("Experiment " + to_string<int>(i + 1), &m_TxtParams, "", &m_vParameters, false, false);
		m_vAllocatedParams.push_back(exp_names[i]);
	}
}

ExperimentBase::run_status ExpList::Run()
{
	vector<ExperimentBase*> exps;

	for (int i = 0; i < num_experiments; i++)
	{
		if (exp_names[i]->IsInitialized())
		{
			if (exp_names[i]->Value().length() > 0)
			{
				ExperimentBase* exp = m_pSheet->FindExperiment(exp_names[i]->Value());

				if (exp)
					exps.push_back(exp);
				else
					cout << "Unable to find: " << exp_names[i]->Value() << endl;
			}
		}
	}

	m_pSheet->scan_scheduler.StartExperiments(exps, 0);

	return FINISHED;
}


