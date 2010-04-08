#include <vector>
#include <algorithm>

#include "experiments.h"
#include "host_interface.h"
#include "Numerics.h"
#include "motors.h"

#include "exp_recover.h"

using namespace std;

experiment::experiment(list_t* exp_list, const std::string& name) :
	info_interface(exp_list, name),
	auto_recover("Auto recover", &params, "value=0"),
	debug_level("Debug level", &params, "value=0"),
	num_exp("Experiments", &params, "value=100 min=0 max=200"),
	Padding(TTL_START_EXP, "Mg Padding",  &params, "t=20"),
	rcSignal(channels, COOLING_ION_NAME + std::string(" signal (s)")),
	rcTiming(channels, "Timing err."),
	rcDDSerrors(channels, "DDS err."),
	exp_flags(0)
{
	Padding.setFlag(RP_FLAG_CAN_HIDE);
}

void experiment::init_exp_sequence(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	this->msg_in = &msg_in;
	this->msg_out = &msg_out;

	g_debug_level = debug_level;

	iMsg = 0;

	this->exp_flags = msg_in.extractU(1);

	pmt.init(&msg_out, getNumChannels(), false);

	init();

	reshuffle();

	//make sure ion is bright
	if (auto_recover && eRecover)
		eRecover->recover_ion();

	prep_ions();
}

bool experiment::debug_exp(unsigned i)
{
	return (i == 0) && (exp_flags & EXP_FLAG_DEBUG);
}

void experiment::put_results(GbE_msg& msg_out)
{
	for (size_t i = 0; i < channels.size(); i++)
		msg_out.insertU(i + 1, static_cast<unsigned>(1000 * (channels[i]->result + 1000000)));
}

void experiment::init_exp(unsigned i)
{
	bDebugPulses = debug_exp(i);

	if (bDebugPulses)
	{
		sprintf(host->buffDebug, "Start pulse sequence for experiment: %s\n", name.c_str());
		host->sendDebugMsg(host->buffDebug);
	}
}


void experiment::run(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	init_exp_sequence(msg_in, msg_out);

	rcDDSerrors.result = 0;

	for (unsigned i = 0; i < num_exp; i++)
	{
		init_exp(i);

		//start timing check.  disables interrupts
		pmt.begin_timing_check(Padding.t, Padding.ttl);

		//run the experiment
		run_exp(i);

		//stop timing check. enables interrupts
		pmt.end_timing_check();


		//get PMT results & check timing
		unsigned N = getNumDetect();

		for (unsigned j = 0; j < N; j++)
			pmt.get_new_data();

		if (bDebugPulses)
		{
			sprintf(host->buffDebug, "Finish pulse sequence for experiment: %s\n\n\n", name.c_str());
			host->sendDebugMsg(host->buffDebug, true);
		}

		bDebugPulses = false;

		//count DDS errors
		//  rcDDSerrors.result += (1 - PULSE_CONTROLLER_check_all_dds(pulser));
	}

	post_exp_calc_results();
	finish_exp_sequence(msg_out);
	post_exp_sequence(rcSignal.result);
}

void experiment::finish_exp_sequence(GbE_msg& msg_out)
{
	pmt.print_timing_failures();

	rcSignal.result = pmt.get_average();
	rcTiming.result = pmt.num_failures();

	put_results(msg_out);
}

void exp_detect::run_exp(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();
	Detect.detection_pulse();
	Precool.pulseStayOn();
}

void exp_detect::detectBkg()
{
	//background detection
	unsigned ftwOn_old = Detect.ftwOn;

	Detect.ftwOn = Detect.ftwOff;
	Detect.detection_pulse();
	Detect.ftwOn = ftwOn_old;
}

void exp_detectN::run_exp(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();

	unsigned N = getNumDetect();
	for (unsigned i = 0; i < N; i++)
		Detect.detection_pulse();

	Precool.pulseStayOn();
}

void exp_load_Mg::run(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	init_exp_sequence(msg_in, msg_out);

	int deltaN = 0;

	//stretch detection time
	unsigned tOld = Detect.t;
	Detect.t = static_cast<unsigned>(Detect.t * detection_stretch);

	for (unsigned i = 0; i < num_exp; i++)
	{

		init_exp(i);

		//provide some padding so that the processor gets a head start in
		//filling the timing buffer
		Padding.pulse();

		//run the experiment
		run_exp(i);

		//get PMT results & check timing
		int n1 = pmt.get_new_data();
		Precool.ddsOff();
		DopplerCool.pulse();

		detectBkg();

		Precool.pulseStayOn();
		int n2 = pmt.get_new_data(1, false);

		deltaN += (n1 - n2);

		if (bDebugPulses)
		{
			sprintf(host->buffDebug, "Finish pulse sequence for experiment: %s\n\n\n", name.c_str());
			host->sendDebugMsg(host->buffDebug, true);
		}

		bDebugPulses = false;
	}

	//unstretch
	Detect.t = tOld;

	rcMgCounts.result = ((double)deltaN) / ((double)num_exp);

	finish_exp_sequence(msg_out);
}


void exp_scanDDS::updateParams()
{
	info_interface::updateParams();
	pulse_dds.dds = dds_num;
}


void exp_scanDDS::run_exp(int)
{
	pulse_dds.pulseStayOn();
}

void exp_repump::run_exp(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();
	Carrier.pulse();
	PrecoolR.pulse();
	Repump.pulse();
	Detect.detection_pulse();
	Precool.pulseStayOn();
}

void exp_heat::init()
{
	exp_detect::init();
	Heat.ttl = HeatTTL;
}


void exp_heat::run_exp(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();
// new
	Heat.pulse();

	if (!disableCarrier)
		Carrier90.stretched_pulse((unsigned)(CarrierStretch * 1000));

// old
/*   Heat.stretched_pulse((unsigned)(CarrierStretch*1000));

   if(!disableCarrier)
      Carrier90.pulse();
 */

	Detect.detection_pulse();
	Precool.pulseStayOn();
}

void exp_load_Al::run(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	init_exp_sequence(msg_in, msg_out);

	int deltaN1 = 0;
	int deltaN2 = 0;
	int deltaN3 = 0;

	//stretch detection time
	unsigned tOld = Detect.t;
	Detect.t = static_cast<unsigned>(Detect.t * detection_stretch);

	for (unsigned i = 0; i < num_exp; i++)
	{
		init_exp(i);

		//provide some padding so that the processor gets a head start in
		//filling the timing buffer
		Padding.pulse();

		run_exp_Bkg(i);
		int n0 = pmt.get_new_data(1);

		run_exp_MgCounts(i);
		int n1 = pmt.get_new_data(1, false);

		run_exp_MgCountsNoRF(i);
		int n2 = pmt.get_new_data(1, false);

		run_exp_MgCountsRF(i);
		int n3 = pmt.get_new_data(1, false);

		deltaN1 += (n1 - n0);
		deltaN2 += (n2 - n0);
		deltaN3 += (n3 - n0);

		if (bDebugPulses)
		{
			sprintf(host->buffDebug, "Finish pulse sequence for experiment: %s\n\n\n", name.c_str());
			host->sendDebugMsg(host->buffDebug, true);
		}

		bDebugPulses = false;
	}

	Detect.t = tOld;

	rcMgCounts.result = ((double)deltaN1) / ((double)num_exp);
	rcMgCountsNoRF.result = ((double)deltaN2) / ((double)num_exp);
	rcMgCountsRF.result = ((double)deltaN3) / ((double)num_exp);

	finish_exp_sequence(msg_out);
}

void exp_load_Al::run_exp_Bkg(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();
	detectBkg();
	Precool.pulseStayOn();
}

void exp_load_Al::run_exp_MgCounts(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();
	Detect.detection_pulse();
	Precool.pulseStayOn();
}

void exp_load_Al::run_exp_MgCountsNoRF(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();
	Carrier90.pulse();
	Detect.detection_pulse();
	Precool.pulseStayOn();
}

void exp_load_Al::run_exp_MgCountsRF(int)
{
	Precool.ddsOff();
	DopplerCool.pulse();
	Heat.pulse();
	Carrier90.pulse();
	Detect.detection_pulse();
	Precool.pulseStayOn();
}
