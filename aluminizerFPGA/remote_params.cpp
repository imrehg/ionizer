#include "remote_params.h"
#include "FluorescenceChecker.h"

#include <assert.h>

using namespace std;

unsigned g_debug_level = 0;

void remote_param::updateGUI(unsigned page_id)
{
	char buff[128];

	getValueString(buff, 128);
	host->updateGUI_param(page_id, name.c_str(), buff);
}

void rp_matrix::updateInverse()
{
	assert(value.nr == 2);
	assert(value.nc == 2);

	double d = value.element(0, 0) * value.element(1, 1) - value.element(0, 1) * value.element(1, 0);

	if (d != 0)
	{
		double invd = 1 / d;
		inverse.element(0, 0) = invd * value.element(1, 1);
		inverse.element(0, 1) = -1 * invd * value.element(0, 1);
		inverse.element(1, 0) = -1 * invd * value.element(1, 1);
		inverse.element(1, 1) = invd * value.element(0, 0);
	}
}

void rp_matrix::update_binary(unsigned length, const char* bin)
{
	if (length != value.get_binary_length())
		throw runtime_error("[rp_matrix::update_binary] wrong data length");

	value.extract_binary(bin);
}

//! make a 3P0 pulse with light from both ports on simultaneously
void Al3P0_pulse::bi_directional_pulse(double dF0, double dF1)
{
	TTL_pulse(padding, 0);

	unsigned ftw0 = MHz2FTW((value.fOn - fOffset + dF0 * 1e-6) / fDiv);
	unsigned ftw1 = MHz2FTW((value.fOn - fOffset + dF1 * 1e-6) / fDiv);

	DDS_set_freq(DDS_3P0,    ftw0);
	DDS_set_freq(DDS_3P0_BR, ftw1);

	TTL_pulse(t, TTL_3P0);

	DDS_set_freq(DDS_3P0_BR, 0);
	DDS_set_freq(DDS_3P0,    0);

}

bool Al3P0_pulse::checked_pulse(FluorescenceChecker* fc, unsigned* nCounts)
{
	TTL_pulse(padding, 0);
	DDS_set_freq(dds, ftwOn);
	TTL_pulse(padding, 0);

	//assume that the cooling/detection beam is already on

	unsigned tRemaining = this->t;

	unsigned nPulses = 0;
	unsigned nChecks = 0;

	*nCounts = 0;

	unsigned tPrevPulse = 0;
	unsigned tPulse = 0;

	while (tRemaining > 0 || nPulses > nChecks)
	{
		tPrevPulse = tPulse;
		tPulse = std::min(fc->get_check_interval(), tRemaining);

		if (tPulse)
		{
			PULSE_CONTROLLER_short_pulse(pulser, tPulse | PULSE_CONTROLLER_COUNTING_PULSE_FLAG, ttl | TTL_DETECT_MON);
			nPulses++;

			if (bDebugPulses)
				print_pulse_info(tPulse, ttl | TTL_DETECT_MON, "DETECT");

			tRemaining -= tPulse;
		}

		//Let the pulse sequence get ahead by one step.
		//This way the checking calculations for the previous interval take place
		//while the current interval runs (to keep a continuous pulse running).
		if (nPulses > 1 || tRemaining == 0)
		{
			//get new PMT data
			unsigned nPMT = PULSE_CONTROLLER_get_PMT(pulser);
			*nCounts += nPMT;

			nChecks++;

			if ( !fc->checkThis(tPrevPulse, nPMT) )
			{
				//need to wait for remaining pulses to finish (should be just one)
				while (nChecks < nPulses)
				{
					PULSE_CONTROLLER_get_PMT(pulser);
					//ignore the fact that the ion may be bright again
					nChecks++;
				}

				return false; // abort
			}
		}
	}

	TTL_pulse(padding, 0);

	return true;
}
