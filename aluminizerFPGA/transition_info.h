#ifndef TRANSITION_INFO_H
#define TRANSITION_INFO_H

#include "info_interface.h"
#include "detection_stats.h"

class pulse_spec
{
public:
pulse_spec() : mFg2(0), pol(0), sb(0)
{
}
pulse_spec(int mFg2, int pol, int sb) : mFg2(mFg2), pol(pol), sb(sb)
{
}
int mFg2, pol, sb;
};


class transition_info : public info_interface
{
public:
//base_name prefixes all pulse names
transition_info(list_t* exp_list, const std::string& name, const std::string& base_name, int sbMax, int mFg2Max);
virtual ~transition_info()
{
}

dds_params* add_new_pulse(const pulse_spec& p, params_t* pparams);

std::string pulse_name(int mFg2, int pol, int sb);
std::string pulse_name(unsigned i);

pulse_spec getPulseSpec(unsigned i);

unsigned getPulseIndex(int mFg2, int pol, int sb, bool bNoThrow = false);
unsigned getPulseIndex(const pulse_spec&);

dds_params* getPulse(int mFg2, int pol, int sb);
dds_params* getPulse(const pulse_spec&);

int get_sbMax() const
{
	return sbMax;
}

private:
std::string base_name;
char sbuffPulseName[64];

protected:
virtual dds_params* new_pulse(const pulse_spec& p, params_t* pparams) = 0;

int sbMax, mFg2Max;
unsigned nPulses;
std::vector<dds_params*> pulses;
};

#endif // TRANSITION_INFO_H
