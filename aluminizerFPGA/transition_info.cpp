#include "transition_info.h"

#include "shared/src/Numerics.h"

#include <vector>
#include <stdexcept>

using namespace std;

pulse_spec transition_info::getPulseSpec(unsigned i)
{
	int numSB = 1 + 2 * sbMax;
	int numPol = 3;

	//invert this:

	/*
	   return (sb + sbMax) + (pol + 1)*numSB + (mFg2 + mFg2Max)*numSB*numPol;
	 */

	pulse_spec p;

	p.sb = (i % numSB) - sbMax;
	p.pol = (i / numSB) - 1;
	p.mFg2 = i / (numSB * numPol) - mFg2Max;

	unsigned i2 = getPulseIndex(p);

	if (i2 != i)
		throw runtime_error("pulse index conversion error");

	return p;
}

transition_info::transition_info(list_t* exp_list,
                                 const std::string& name,
                                 const std::string& base_name,
                                 int sbMax, int mFg2Max) :
	info_interface(exp_list, name),
	base_name(base_name),
	sbMax(sbMax),
	mFg2Max(mFg2Max),
	nPulses(1 + getPulseIndex(mFg2Max, 1, sbMax, true)),
	pulses(nPulses, 0)
{
}

dds_params* transition_info::add_new_pulse(const pulse_spec& p, params_t* pparams)
{
	unsigned i = getPulseIndex(p);

	pulses.at(i) = new_pulse(p, pparams);

	return pulses.at(i);
}

std::string transition_info::pulse_name(int mFg2, int pol, int sb)
{
	return ::pulse_name(base_name, mFg2, pol, sb);
}

unsigned transition_info::getPulseIndex(const pulse_spec& p)
{
	return getPulseIndex(p.mFg2, p.pol, p.sb);
}


unsigned transition_info::getPulseIndex(int mFg2, int pol, int sb, bool bNoThrow)
{
	unsigned numSB = 1 + 2 * sbMax;
	unsigned numPol = 3;

	unsigned i = (sb + sbMax) + (pol + 1) * numSB + (mFg2 + mFg2Max) * numSB * numPol;

	if (i >= nPulses && !bNoThrow)
		throw runtime_error("[transition_info::getPulseIndex]: " + pulse_name(mFg2, pol, sb) + " (out of range)");
	else
		return i;
}

dds_params* transition_info::getPulse(int mFg2, int pol, int sb)
{
	unsigned i = getPulseIndex(mFg2, pol, sb);

	if ( i > pulses.size() )
		throw runtime_error("[transition_info::getPulse]: " + pulse_name(mFg2, pol, sb) + " (out of range)");
	else
	{
		dds_params* d = pulses[i];

		if (d == 0)
			throw runtime_error("[transition_info::getPulse]: " + pulse_name(mFg2, pol, sb) + " (uninitialized)");
		else
			return d;
	}
}

dds_params* transition_info::getPulse(const pulse_spec& p)
{
	return getPulse(p.mFg2, p.pol, p.sb);
}


