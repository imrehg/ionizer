#ifndef PMTRESULTS_H_
#define PMTRESULTS_H_

#include <vector>
#include "ttl_pulse.h"

//! Keeps track of experiment results from the PMT, as well as timing errors
class exp_results
{
public:
exp_results(GbE_msg* msg_out = 0, unsigned nChannels = 0) :
	nChannels(nChannels),
	tBad(0),
	tChecked(0)
{
	init(msg_out, nChannels);
}

~exp_results();

void init(GbE_msg* msg_out, unsigned nChannels, bool bCumulativeTimingCheck = false);

unsigned num_failures()
{
	return tBad;
}

void print_timing_failures()
{
	if (tChecked && tBad)
		printf("WARNING: %u/%u timing failures!\n", tBad, tChecked);
}

//! begin timing check and raise thread priority
void begin_timing_check(unsigned padding_t, unsigned padding_ttl);

//! finish timing check and restore thread priority
void end_timing_check();


void set_channel_data(unsigned n, unsigned d)
{
	msg->insertU(n + 1, d);
}

void insert_PMT_data(unsigned d)
{
	PMTsum += d;
	nValues++;

	msg->insertU(iNumShots, nValues);
	msg->insertU(j, d);

	j++;
}

unsigned get_result(unsigned n)
{
	return msg->extractU(nChannels + 2 + n);
}

unsigned get_new_data(unsigned scale = 1, bool bStore = true);

unsigned get_total()
{
	return PMTsum;
}

double get_average()
{
	if (nValues)
		return PMTsum / (double)nValues;
	else
		return 0;
}

private:
unsigned nChannels, j;
GbE_msg* msg;
unsigned PMTsum;
unsigned nValues, iNumShots;
unsigned tBad, tChecked;
};

#endif /*PMTRESULTS_H_*/
