#ifndef EXPERIMENTS_H_
#define EXPERIMENTS_H_

#include "common.h"
#include "shared/src/messaging.h"
#include "exp_results.h"
#include "remote_params.h"
#include "info_interface.h"
#include "dac.h"


//void exp_raman(GbE_msg& msg_in, GbE_msg& msg_out);
//void exp_rf_drive(GbE_msg& msg_in, GbE_msg& msg_out);


class experiment : public info_interface
{
public:
experiment(list_t* exp_list, const std::string& name);
virtual ~experiment()
{
}

//! Run the experiment a spec'd number of times, i.e. starts an exp_sequence
//the default implementation calls run_exp() num_exp times, and transfers results to msg_out
virtual void run(const GbE_msg& msg_in, GbE_msg& msg_out);

//! Called at the end of init_exp_sequence().  Can do optical pump & check here.
virtual void prep_ions()
{
}

//! Re-initialize all DDS at start of each exp. sequence.  This causes phase jumps.
virtual bool needInitDDS()
{
	return true;
}
protected:

//! Run one iteration  of the exp.
virtual void run_exp(int iExp) = 0;

//! Runs after each exp_sequence is completed (at the end of run() )
virtual void post_exp_sequence(double /* signal */)
{
}

//! get results and copy them to msg_out (via put_results)
void finish_exp_sequence(GbE_msg& msg_out);

//! copy results into msg_out
void put_results(GbE_msg& msg_out);

virtual void post_exp_calc_results()
{
}

//! return how many PMT results should be recorded for each experiment
virtual unsigned getNumDetect() const
{
	return 1;
}

//! Initialize certain things like pulse-direction and polarization at the start of each exp_sequence
virtual void init()
{
}

//! Randomize and adjust whatever needs tto change every exp_sequence
virtual void reshuffle()
{
}

//! Called at the beginning of each exp. Overrides should call the base class function.
virtual void init_exp(unsigned i);

bool debug_exp(unsigned i);    //should the ith exp provide debug info?

exp_results pmt;

rp_bool auto_recover;
rp_unsigned debug_level;
rp_unsigned num_exp;

const GbE_msg* msg_in;
GbE_msg* msg_out;

ttl_params Padding;
result_channel rcSignal, rcTiming, rcDDSerrors;
unsigned iMsg, exp_flags;

protected:

//! Called at the beginning of each exp_sequence
void init_exp_sequence(const GbE_msg& msg_in, GbE_msg& msg_out);
};

class exp_scanDDS : public experiment
{
public:
exp_scanDDS(list_t* exp_list,
            const std::string& name = "Detect") :
	experiment(exp_list, name),
	dds_num("DDS", &params, "value=0"),
	pulse_dds(DDS_DETECT, "Pulse (stays on)", &params, TTL_DETECT_MON, "t=100 fOn=221 fOff=0")
{
}

virtual ~exp_scanDDS()
{
}

virtual void updateParams();

protected:

virtual void run_exp(int iExp);

rp_unsigned dds_num;
dds_params pulse_dds;
};

class exp_detect : public experiment
{
public:
exp_detect(list_t* exp_list,
           const std::string& name = "Detect") :
	experiment(exp_list, name),
	Detect(DDS_DETECT, ION + " Detect", &params, TTL_DETECT_MON, "t=100 fOn=222 fOff=190"),
	DopplerCool(DDS_DETECT, ION + " Doppler cool", &params, TTL_DETECT_MON, "t=100 fOn=218 fOff=190"),
	Precool(DDS_PRECOOL, ION + " Precool", &params, TTL_PRECOOL_MON, "t=100 fOn=220 fOff=0.000001")
{
}

virtual ~exp_detect()
{
}

protected:
virtual void run_exp(int iExp);
void detectBkg();

dds_params Detect, DopplerCool, Precool;

};

class exp_detectN : public exp_detect
{
public:
exp_detectN(list_t* exp_list,
            const std::string& name = "DetectN") :
	exp_detect(exp_list, name),
	nDetect("num det.", &params, "value=0")
{
}

virtual ~exp_detectN()
{
}

protected:
virtual void run_exp(int iExp);

//return how many PMT results should be recorded for each experiment
virtual unsigned getNumDetect() const
{
	return nDetect;
}

rp_unsigned nDetect;
};

class exp_load : public exp_detect
{
public:
exp_load(list_t* exp_list, const std::string& name) :
	exp_detect(exp_list, name),
	detection_stretch("Detection stretch", &params, "value=1")
{
}

virtual ~exp_load()
{
}

protected:
rp_double detection_stretch;
};

class exp_load_Mg : public exp_load
{
public:
exp_load_Mg(list_t* exp_list,
            const std::string& name = "Load Mg+") :
	exp_load(exp_list, name),
	rcMgCounts(channels, "Mg+ sig. (Bkg corr.)")
{
}

virtual ~exp_load_Mg()
{
}
virtual void run(const GbE_msg& msg_in, GbE_msg& msg_out);

protected:
result_channel rcMgCounts;
};

class exp_repump : public exp_detect
{
public:
exp_repump(list_t* exp_list,
           const std::string& name = "Repump") :
	exp_detect(exp_list, name),
	Carrier("Mg carrier s+ mFg = -3", &params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0"),
	PrecoolR(DDS_PRECOOL, "Precool (repump)", &params, TTL_PRECOOL_MON, "t=100 fOn=221 fOff=0.000001"),
	Repump(TTL_REPUMP, "Mg Repump",  &params, "t=1")
{
	Detect.setFlag( RP_FLAG_NOLINK );
}

virtual ~exp_repump()
{
}

protected:
virtual void run_exp(int iExp);

raman_pulse Carrier;
dds_params PrecoolR;
ttl_params Repump;
};

class exp_heat : public exp_detect
{
public:
exp_heat(list_t* exp_list, const std::string& name = "Heat") :
	exp_detect(exp_list, name),
	HeatTTL("Heat TTL", &params, "value=0"),
	disableCarrier("Disable carrier", &params, "value=0"),
	Heat(DDS_HEAT, "Heat", &params, 0, "t=100 fOn=1.58 fOff=0"),
	Carrier90("Mg carrier s+ mFg = -3", &params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0"),
	CarrierStretch("Carrier stretch", &params, "value=1.5")
{
	CarrierStretch.setExplanation("Carrier pulse time stretch factor");
}

virtual ~exp_heat()
{
}

protected:
virtual void init();
virtual void run_exp(int iExp);

rp_unsigned HeatTTL;
rp_bool disableCarrier;
dds_params Heat;
raman_pulse Carrier90;
rp_double CarrierStretch;
};

class exp_load_Al : public exp_heat
{
public:
exp_load_Al(list_t* exp_list,
            const std::string& name = "Load Al+") :
	exp_heat(exp_list, name),
	detection_stretch("Detection stretch", &params, "value=1"),
	rcMgCounts(channels, "Mg+ sig. (Bkg corr.)"),
	rcMgCountsNoRF(channels, "Mg+ w/o RF (Bkg corr.)"),
	rcMgCountsRF(channels, "Mg+ w/ RF (Bkg corr.)")
{
}

virtual ~exp_load_Al()
{
}
virtual void run(const GbE_msg& msg_in, GbE_msg& msg_out);

protected:
void run_exp_Bkg(int iExp);
void run_exp_MgCounts(int iExp);
void run_exp_MgCountsNoRF(int iExp);
void run_exp_MgCountsRF(int iExp);

rp_double detection_stretch;
result_channel rcMgCounts, rcMgCountsNoRF, rcMgCountsRF;
};


#endif /*EXPERIMENTS_H_*/

