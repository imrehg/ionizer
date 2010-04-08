#include "util.h"
#include "exp_results.h"
#include "pulse_controller.h"

exp_results::~exp_results()
{
	enable_interrupts();
	print_timing_failures();
}

void exp_results::init(GbE_msg* msg_out, unsigned nChannels, bool bCumulativeTimingCheck)
{
	this->nChannels = nChannels;
	this->msg = msg_out;
	this->PMTsum = 0;
	this->nValues = 0;

	if (!bCumulativeTimingCheck)
	{
		this->tBad = 0;
		this->tChecked = 0;
	}

	this->iNumShots = nChannels + 1;
	this->j = nChannels + 2;


	if (msg)
	{
		msg->what = S2C_OK;

		for (unsigned i = 0; i < MSG_STD_PAYLOAD_SIZE; i++)
			msg->insertU(i, 0);

		msg->insertU(0, nChannels);
	}
}

void exp_results::begin_timing_check(unsigned padding_t, unsigned padding_ttl)
{
	tChecked++;

	//set the thread to have max. priority
	//set_priority(0);
	disable_interrupts();

	PULSE_CONTROLLER_clear_timing_check(pulser);

	//provide some padding so that the processor gets a head start in
	//filling the timing buffer
	TTL_pulse(padding_t, padding_ttl);

	PULSE_CONTROLLER_enable_timing_check(pulser);
}

void exp_results::end_timing_check()
{
	PULSE_CONTROLLER_disable_timing_check(pulser);

	//restore old thread priority
/*	if(old_priority >= 0)
   {
      set_priority(old_priority);
      old_priority = -1;
   }
 */

	enable_interrupts();

	if (PULSE_CONTROLLER_timing_ok(pulser) != 1)
		tBad++;
}

unsigned exp_results::get_new_data(unsigned scale, bool bStore)
{
//	while(PULSE_CONTROLLER_read_empty(pulser))
//		yield_execution();

	PULSE_CONTROLLER_set_idle_function(yield_execution);

	unsigned new_data = PULSE_CONTROLLER_get_PMT(pulser);

	if (msg && bStore)
		insert_PMT_data(scale * new_data);

	return new_data;
}
