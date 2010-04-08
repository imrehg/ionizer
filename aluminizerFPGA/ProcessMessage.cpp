#ifdef ALUMINIZER_SIM
	#define XPAR_PULSE_CONTROLLER_0_BASEADDR (0)
	#include "pulse_controller.h"
#else
extern "C"
{
	#include "sleep.h"
	#include "xparameters.h"
	#include "pulse_controller.h"
}
#endif


#ifndef WIN32
#ifdef ALUMINIZER_SIM
	#include <arpa/inet.h>
#endif
#endif


void* pulser = (void*)XPAR_PULSE_CONTROLLER_0_BASEADDR;

#include <stdio.h>
#include <stdlib.h>

#include <stdexcept>
#include <iostream>

#include "MessageLoop.h"
#include "ttl_pulse.h"
#include "dds_pulse.h"
#include "dds_pulse_info.h"
#include "remote_params.h"
#include "experiments.h"
#include "dacI.h"
#include "exp_results.h"
#include "voltagesI.h"

#include "host_interface.h"
#include "motors.h"
#include "common.h"

#ifdef CONFIG_AL
#include "Al/exp_Al3P0.h"
#include "Al/transition_info_Al.h"
#endif

using namespace std;

bool bDebugFirstExp = false;
bool bDebugPulses = false;


void dds_reset(const GbE_msg& msg_in, GbE_msg&)
{
	unsigned iDDS = msg_in.extractU(0);

	PULSE_CONTROLLER_dds_reset(pulser, iDDS);
	PULSE_CONTROLLER_set_dds_div2(pulser, iDDS, false);
}

void dds_test(const GbE_msg& msg_in, GbE_msg&)
{
	unsigned nTest = msg_in.extractU(0);

	PULSE_CONTROLLER_self_test(pulser, nTest);
}

void dds_set_freq(const GbE_msg& msg_in, GbE_msg&)
{
	unsigned iDDS = msg_in.extractU(0);
	unsigned ftw = msg_in.extractU(1);

	printf("setfreq(%d): %d\r\n", iDDS, FTW2Hz(ftw));
	PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftw);
}

void dds_get_freq(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	msg_out.insertU(0, PULSE_CONTROLLER_get_dds_freq(pulser,  msg_in.extractU(0)));
}

void dds_set_phase(const GbE_msg& msg_in, GbE_msg&)
{
	unsigned iDDS = msg_in.extractU(0);
	unsigned phase = msg_in.extractU(1);

	PULSE_CONTROLLER_set_dds_phase(pulser, iDDS, phase);
}

void dds_get_phase(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	msg_out.insertU(0, PULSE_CONTROLLER_get_dds_phase(pulser,  msg_in.extractU(0)));
}

void ttl_set(const GbE_msg& msg_in, GbE_msg&)
{
	unsigned high_mask = msg_in.extractU(0);
	unsigned low_mask = msg_in.extractU(1);

//   printf("ttl high mask: %08X, ttl low mask: %08X\r\n", high_mask, low_mask);
	PULSE_CONTROLLER_set_ttl(pulser, high_mask, low_mask);
}

void ttl_get(const GbE_msg&, GbE_msg& msg_out)
{
	unsigned high_mask, low_mask;

	PULSE_CONTROLLER_get_ttl(pulser, &high_mask, &low_mask);

	msg_out.insertU(0, high_mask);
	msg_out.insertU(1, low_mask);
}

void init_motors()
{
	motors.push_back(motor(pulser, 0, TTL_MOTOR_0, 0, 20));
	motors.push_back(motor(pulser, 0, TTL_MOTOR_1, 0, 20));
}

void motor_set(const GbE_msg& msg_in, GbE_msg&)
{
	unsigned iMotor = msg_in.extractU(0);
	unsigned angle = msg_in.extractU(1);

   if(iMotor < motors.size())
	motors[iMotor].setAngle(angle);
}

void motor_get(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned iMotor = msg_in.extractU(0);

   if(iMotor < motors.size())
      msg_out.insertU(0, motors[iMotor].getAngle());
}

void set_debug_1st(const GbE_msg& msg_in, GbE_msg&)
{
	bDebugFirstExp = (msg_in.extractU(0) != 0);
}

void num_exp_params(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned i = msg_in.extractU(0);

	msg_out.insertU(0, global_exp_list.at(i)->getNumParams());
}

void define_exp_param(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned i = msg_in.extractU(0);
	unsigned iParam = msg_in.extractU(1);

	global_exp_list.at(i)->defineParam(iParam, msg_out.extractS(0));
}

void explain_exp_param(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned i = msg_in.extractU(0);
	unsigned iParam = msg_in.extractU(1);

	global_exp_list.at(i)->explainParam(iParam, msg_out.extractS(0), MAX_STR_LENGTH);
}

void get_param_val_str(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned i = msg_in.extractU(0);
	unsigned iParam = msg_in.extractU(1);

	global_exp_list.at(i)->get_param_val_str(iParam, msg_out.extractS(0), MAX_STR_LENGTH);
}

void run_exp(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned i = msg_in.extractU(0);
	experiment* exp = dynamic_cast<experiment*>(global_exp_list.at(i));

	if (exp)
	{
		if (exp->needInitDDS())
			PULSE_CONTROLLER_reinit_DDS(pulser, NDDS);

		exp->run(msg_in, msg_out);
	}
	else
		throw runtime_error("can't run: " + global_exp_list.at(i)->name);
}

void num_experiments(const GbE_msg&, GbE_msg& msg_out)
{
	msg_out.insertU(0, global_exp_list.size());
}

void exp_name(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned i = msg_in.extractU(0);

	strcpy(msg_out.extractS(0), global_exp_list.at(i)->getName().c_str());
}

void getNumDataChannels(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned i = msg_in.extractU(0);

	msg_out.insertU(0, global_exp_list.at(i)->getNumChannels());
}

void getDataChannelName(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned iExp = msg_in.extractU(0);
	unsigned iChannel = msg_in.extractU(1);

	strcpy(msg_out.extractS(0), global_exp_list.at(iExp)->getChannelName(iChannel).c_str());
}

void setVoltages(const GbE_msg& msg_in, const GbE_msg&)
{
	unsigned nUpdates = msg_in.extractU(0);
	unsigned j = 1;

	for (unsigned i = 0; i < nUpdates; i++)
	{
		unsigned iPage = msg_in.extractU(j++);
		unsigned iChannel = msg_in.extractU(j++);
		unsigned uV = msg_in.extractU(j++);

		dacI* p = dynamic_cast<dacI*>(global_exp_list.at(iPage));

		if (p)
			p->setVoltage(iChannel, uV);
	}
}


void rampVoltages(const GbE_msg& msg_in, const GbE_msg&)
{
	unsigned j = 0;

	unsigned page_id = msg_in.extractU(j++);
	unsigned settings_id = msg_in.extractU(j++);
	unsigned come_back = msg_in.extractU(j++);

	voltagesI* v = dynamic_cast<voltagesI*>(global_exp_list.at(page_id));

	if (v)
		v->rampTo(settings_id, come_back, true);
	else
		cout << "WARNING: Can't find voltagesI*" << endl;

}


void setCalibrationWord(const GbE_msg& msg_in, const GbE_msg&)
{
	unsigned iExp = msg_in.extractU(0);
	dacI* p = dynamic_cast<dacI*>(global_exp_list.at(iExp));

	if (p)
		p->setCalibrationWord();
}

void change_param_bin(list_t& exps, const GbE_msg& msg_in, GbE_msg&)
{
//	printf("update_params: %08X parameters total\n", msg_in.extractU(0));
	unsigned page_id = msg_in.extractU(0);
	unsigned param_id = msg_in.extractU(1);
	unsigned length = msg_in.extractU(2);

	exps.at(page_id)->update_param_binary(param_id, length, msg_in.extractSC(3));
	exps.at(page_id)->updateParams();
}

void change_params(list_t& exps, const GbE_msg& msg_in, GbE_msg&)
{
	unsigned numParams = msg_in.extractU(0);
	unsigned iExp = msg_in.extractU(1);

	if (iFPGA->debug_params.get_value())
		printf("(exp %d) change_params: %08X parameters total\r\n", iExp, numParams);

	const char* p_name = msg_in.extractSC(2);
	const char* p_end = msg_in.extractSC(MSG_STD_PAYLOAD_SIZE);

	for (unsigned i = 0; i < numParams; i++)
	{
		if (p_name >= p_end)
			throw runtime_error("change_params -- too long");

		const char* p_value = p_name + strlen(p_name) + 1;

		if (p_value >= p_end)
			throw runtime_error("change_params -- too long");

		exps.at(iExp)->updateParam(p_name, p_value);

		if (iFPGA->debug_params.get_value())
			cout << "[" << exps.at(iExp)->name << "] " << p_name << " = " << p_value << "(l=" << strlen(p_name) + strlen(p_value) << ")" << endl;

		p_name = p_value + strlen(p_value) + 1;
	}

	exps.at(iExp)->updateParams();
}

void setIonXtal(const GbE_msg& msg_in, GbE_msg&)
{
	const char* p_name = msg_in.extractSC(1);

	printf("Set ion configuration: %s \n", p_name);

	for (unsigned i = 0; i < global_exp_list.size(); i++)
	{
		transition_info* pPage = dynamic_cast<transition_info*>( global_exp_list.at(i) );

		if (pPage)
			pPage->setIonXtal(p_name);
	}
}

int process_message(const GbE_msg& msg_in, GbE_msg& msg_out, bool bDebug)
{
	static int num_messages = 0;

	num_messages++;

	if (num_messages == 1)
		host->sendInfoMsg(IM_VOICE, "Wellcome to aluminizer.");

	msg_out.what = S2C_OK;
	msg_out.id   = msg_in.id;

	if (motors.size() == 0)
		init_motors();

	try
	{
		if (bDebug)
			printf("processing message: %08X\n", msg_in.what);

		switch (msg_in.what)
		{
		case C2S_QUIT: printf("process_message: C2S_QUIT\r\n"); num_messages = 0; return 1;
		case C2S_ACK: printf("process_message: C2S_ACK\r\n"); return 0;

		case C2S_SET_DDS_FREQ: dds_set_freq(msg_in, msg_out); return 0;
		case C2S_GET_DDS_FREQ: dds_get_freq(msg_in, msg_out); return 0;
		case C2S_SET_DDS_PHASE: dds_set_phase(msg_in, msg_out); return 0;
		case C2S_GET_DDS_PHASE: dds_get_phase(msg_in, msg_out); return 0;
		case C2S_RESET_DDS: dds_reset(msg_in, msg_out); return 0;
		case C2S_TEST_DDS: dds_test(msg_in, msg_out); return 0;

		case C2S_SET_TTL: ttl_set(msg_in, msg_out); return 0;
		case C2S_GET_TTL: ttl_get(msg_in, msg_out); return 0;

		case C2S_SET_MOTOR_ANGLE: motor_set(msg_in, msg_out); return 0;
		case C2S_GET_MOTOR_ANGLE: motor_get(msg_in, msg_out); return 0;

		case C2S_SET_DEBUG_1ST: set_debug_1st(msg_in, msg_out); return 0;

		case C2S_CHANGE_PARAMS: change_params(global_exp_list, msg_in, msg_out);   return 0;
		case C2S_CHANGE_PARAM_BIN: change_param_bin(global_exp_list, msg_in, msg_out);   return 0;
		case C2S_NUM_EXP: num_experiments(msg_in, msg_out);   return 0;
		case C2S_EXP_NAME: exp_name(msg_in, msg_out);   return 0;

		case C2S_NUM_REMOTE_ACTIONS:
			msg_out.insertU(0, global_exp_list.at(msg_in.extractU(0))->getNumRemoteActions());
			return 0;

		case C2S_REMOTE_ACTION_NAME:
			strcpy(msg_out.extractS(0),
			       global_exp_list.at(msg_in.extractU(0))->getRemoteActionName(msg_in.extractU(1)).c_str());
			return 0;

		case C2S_CALL_REMOTE_ACTION:
			msg_out.insertU(0, global_exp_list.at(msg_in.extractU(0))->remote_action(msg_in.extractU(1)));
			return 0;

		case C2S_NUM_EXP_PARAMS: num_exp_params(msg_in, msg_out);   return 0;
		case C2S_DEFINE_EXP_PARAM: define_exp_param(msg_in, msg_out);   return 0;
		case C2S_EXPLAIN_EXP_PARAM: explain_exp_param(msg_in, msg_out);   return 0;
		case C2S_GET_PARAM_VAL_STR: get_param_val_str(msg_in, msg_out);   return 0;
		case C2S_RUN_EXP: run_exp(msg_in, msg_out);   return 0;
		case C2S_NUM_DATA_CHANNELS: getNumDataChannels(msg_in, msg_out);   return 0;
		case C2S_DATA_CHANNEL_NAME: getDataChannelName(msg_in, msg_out);   return 0;
 
		case C2S_RAMP_VOLTAGES: rampVoltages(msg_in, msg_out);   return 0;
		case C2S_SET_DAC_CALIBRATION: setCalibrationWord(msg_in, msg_out);   return 0;

		case C2S_NUM_DATA_PLOTS: msg_out.insertU(0, global_exp_list.at(msg_in.extractU(0))->getNumPlots()); return 0;
		case C2S_GET_PLOT_DATA: global_exp_list.at(msg_in.extractU(0))->getPlotData(msg_in.extractU(1), msg_in.extractU(2), msg_out); return 0;
		case C2S_SET_COEFFICIENTS: global_exp_list.at(msg_in.extractU(0))->setCoefficients(msg_in, msg_out); return 0;

		case CS2_SET_ION_XTAL: setIonXtal(msg_in, msg_out);   return 0;

#ifdef CONFIG_AL
		case C2S_DETECTION_HISTOGRAM: gpAl3P0->getHistogramData(msg_in, msg_out); return 0;
		case C2S_HIST_NAME:           gpAl3P0->getHistName(msg_in, msg_out); return 0;
		case C2S_RESET_STATS:         gpAl3P0->resetStats(msg_in, msg_out); return 0;
		case C2S_GET_AL_STATE:      gpAl3P0->getClockState(msg_in, msg_out); return 0;
#endif

		default: printf("unknown message: %08X\n", msg_in.what); msg_out.what = S2C_ERROR; return 0;
		}
	}
	catch (runtime_error e)
	{
		msg_out.what = S2C_ERROR;

		printf("*** process_message caught the following runtime error:\n***  %s\n\n", e.what());
		snprintf(msg_out.extractS(0), MAX_STR_LENGTH, "[process_message] runtime error: %s", e.what());
	}



	return 0;
}



