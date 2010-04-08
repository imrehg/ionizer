#include "remote_params.h"
#include "FluorescenceChecker.h"
#include "shared/src/Numerics.h"


bool FluorescenceChecker::checkThis(unsigned t, unsigned nPMT)
{
	if (t == 0)
		return true;

	//calculate that likelihood that something with a poissonian count rate
	//of expected_count_rate would yield this few counts
	//
	//P(mean, n) = exp(-1*mean) * pow(mean, (double)n) / factorial(n)

	double observed_rate = nPMT / (double)t;

	if (g_debug_level > 0)
		printf("[FluorescenceChecker::checkThis] t=%u nPMT=%u observed_rate=%f\r\n", t, nPMT, observed_rate);


	double logP[2];

	for (unsigned i = 0; i < 2; i++)
		logP[i] = numerics::LogPoissonProb(t * count_rates[i], nPMT);

	double log_odds_dark = logP[0] - logP[1];

	log_running_ratio += log_odds_dark;

	if (log_running_ratio < 0)
	{
		//this ion is probably bright
		count_rates[1] = (1 - learning_rate) * count_rates[1] + learning_rate * observed_rate;

		//reset the running ratio
		log_running_ratio = 0;
	}
	else
		//this ion is probably dark
		count_rates[0] = (1 - learning_rate) * count_rates[0] + learning_rate * observed_rate;

	double max_ratio = -1 * log(min_ok_prob);

	if (g_debug_level > 0)
		printf("[FluorescenceChecker::checkThis] log_running_ratio=%f  (max=%f)\r\n", log_running_ratio, max_ratio);

	return log_running_ratio < max_ratio;
}
