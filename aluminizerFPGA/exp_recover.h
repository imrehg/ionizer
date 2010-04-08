#ifndef EXP_RECOVER_H_
#define EXP_RECOVER_H_

#include "experiments.h"
#include "detection_stats.h"

/*
   Check if ion is bright or dark using Baysian analysis.
   If the ion is dark, try to recover by cooling & adjusting
   trap parameters.
 */

class exp_recover : public experiment
{
public:
exp_recover(list_t* exp_list,
            const std::string& name = "Recover");

virtual ~exp_recover()
{
}

virtual void updateParams();

virtual void run(const GbE_msg& msg_in, GbE_msg& msg_out);

//use maximum likelihood method to determine ion state
// -1 -- unknown
//  0 -- dark
//  1 -- bright
int ion_state(double min_odds_ratio_dark, double min_odds_ratio_bright);
bool recover_ion();

//! cool for approximately spec'd microseconds
void cool(unsigned us);
protected:

void rampDownVoltages();
void rampUpVoltages();

//probability of state i given n counts
double P(unsigned i, unsigned n);

virtual void run_exp(int iExp);


rp_double dark_mean, bright_mean, confidence_dark, confidence_bright;
rp_unsigned max_reps;
rp_bool ramp_voltages;
dds_params Detect, DopplerCool, PrecoolShort, PrecoolLong;

double odds_dark, odds_bright;
exp_results pmt;
vector<double> P0, P1;
};

extern exp_recover* eRecover;

#endif //EXP_RECOVER_H_
