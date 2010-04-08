#include <vector>
#include <algorithm>

#include "ttl_pulse.h"
#include "dds_pulse.h"
#include "dds_pulse_info.h"
#include "remote_params.h"
#include "exp_recover.h"
#include "host_interface.h"
#include "shared/src/Numerics.h"
#include "motors.h"
#include "config_local.h"
#include "voltagesI.h"

using namespace std;

exp_recover*   eRecover = 0;

exp_recover::exp_recover(list_t* exp_list,
                         const std::string& name) :
	experiment(exp_list, name),
	dark_mean("Dark mean",         &params, "value=0.4"),
	bright_mean("Bright mean",       &params, "value=4"),
	confidence_dark("Dark conf. [10^]",      &params, "value=3"),
	confidence_bright("Bright conf. [10^]",     &params, "value=3"),
	max_reps("Max reps.",        &params, "value=5"),
	ramp_voltages("Ramp voltages",      &params, "value=false"),
	Detect(DDS_DETECT, "Mg Detect", &params, TTL_DETECT_MON, "t=100 fOn=220 fOff=170"),
	DopplerCool(DDS_DETECT, "Mg Doppler cool", &params, TTL_DETECT_MON, "t=100 fOn=218 fOff=190"),
	PrecoolShort(DDS_PRECOOL, "Precool (short)", &params, TTL_PRECOOL_MON, "t=2 fOn=220 fOff=0.000001"),
	PrecoolLong(DDS_PRECOOL, "Precool (long)", &params, TTL_PRECOOL_MON, "t=200 fOn=220 fOff=0.000001"),
	pmt(0),
	P0(20),
	P1(20)
{
}

void exp_recover::run(const GbE_msg&, GbE_msg& msg_out)
{
	PULSE_CONTROLLER_disable_timing_check(pulser);

	pmt.init(&msg_out, getNumChannels());

	recover_ion();
}

void exp_recover::updateParams()
{
	experiment::updateParams();

	odds_dark = pow(10, confidence_dark);
	odds_bright = pow(10, confidence_bright);

	//pre-compute probabilities
	for (unsigned i = 0; i < P0.size(); i++)
		P0[i] = numerics::PoissonProb(dark_mean, i);

	for (unsigned i = 0; i < P1.size(); i++)
		P1[i] = numerics::PoissonProb(bright_mean, i);
}

//probability of state i given n counts
double exp_recover::P(unsigned i, unsigned n)
{
	if (i == 0)
	{
		if (n < P0.size())
			return P0[n];
		else
			return numerics::PoissonProb(dark_mean, n);
	}
	else
	{
		if (n < P1.size())
			return P1[n];
		else
			return numerics::PoissonProb(bright_mean, n);
	}
}

void exp_recover::rampDownVoltages()
{
	if (iVoltages)
		iVoltages->rampDownXtallize();
}

void exp_recover::rampUpVoltages()
{
	if (iVoltages)
		iVoltages->rampUpXtallize();
}

bool exp_recover::recover_ion()
{
	//cout << "Recover ion start." << endl;
	unsigned nMax  = max_reps;
	unsigned nReps = 0;
	unsigned is = 0;

	if (confidence_dark == 0)
		return true;

	PrecoolShort.pulse();
	while (ion_state(odds_dark, odds_bright) != 1 && (nReps < nMax))
	{
		PrecoolShort.pulseStayOn();

		if (ramp_voltages)
		{
			printf("Ramping down voltages\r\n ");
			rampDownVoltages();
		}

//		PrecoolShort.ddsOff();

		for (unsigned j = 0; j < 100; j++)
		{
//			PrecoolLong.ddsOff();
			is = ion_state(odds_dark, odds_bright);

			if (is == 1)
				break;
			else
				PrecoolLong.pulseStayOn();
				//		DopplerCool.pulseStayOn();
		}

		PrecoolShort.pulseStayOn();
		if (ramp_voltages)
		{
			rampUpVoltages();
			printf("Ramped up voltages\r\n");
		}

		if (is > 0)
			break;

		nReps++;
	}

	if (nReps < nMax)
	{
		if (nReps > 0)
		{
			cout << "Recover ion SUCCESS!" << endl;
			host->sendInfoMsg(IM_VOICE, "Ions OK");
		}
	}
	else
	{
		cout << "Recover ion FAILURE!" << endl;
		host->sendInfoMsg(IM_VOICE, "Ions lost");
	}

	return true;
}

//! cool for approximately spec'd microseconds
void exp_recover::cool(unsigned us)
{
    unsigned t = 0;
    unsigned tCycle = (PrecoolShort.t + DopplerCool.t) / 100;

    while(t < us)
    {
        PrecoolShort.pulse();
        DopplerCool.pulse();

        t += tCycle;
    }

    PrecoolShort.pulseStayOn();
}

//use maximum likelihood method to determine ion state
// -1 -- unknown
//  0 -- dark
//  1 -- bright
int exp_recover::ion_state(double min_odds_ratio_dark, double min_odds_ratio_bright)
{
	double likelihoods[2] = { 1, 1 };
	double odds_ratio_bright = 1;
	double odds_ratio_dark = 1;
	unsigned nReps = 0;
	unsigned nMax  = num_exp;

	if (debug_level > 0)
		printf("Check ion state ...\n");

	do
	{
		Padding.pulse();
		PrecoolShort.pulse();
		Detect.detection_pulse();

		unsigned n = pmt.get_new_data(1, false);

		likelihoods[0] *= P(0, n);
		likelihoods[1] *= P(1, n);

		odds_ratio_bright = (likelihoods[1] / likelihoods[0]);
		odds_ratio_dark = 1 / odds_ratio_bright;

		if (debug_level > 0)
			printf("%u counts (dark: p=%f) (bright: p=%f) bright odds: %e\n", n, P(0, n), P(1, n), odds_ratio_bright);

		nReps++;
	}
	while ( (odds_ratio_bright < min_odds_ratio_bright) && (odds_ratio_dark < min_odds_ratio_dark) && (nReps <= nMax));

	if (odds_ratio_dark > 1)
	{
		if (debug_level > 0)
			printf("%6.0f to 1 odds that the ion is DARK (%d measurements).\n", odds_ratio_dark, nReps);

		return 0;
	}
	else
	{
		if (debug_level > 0)
			printf("%6.0f to 1 odds that the ion is BRIGHT (%d measurements).\n", odds_ratio_bright, nReps);

		return 1;
	}
}

void exp_recover::run_exp(int)
{
	recover_ion();
}
