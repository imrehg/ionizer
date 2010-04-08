#include "../common.h"
#include "transition_info_Mg.h"

#include "../shared/src/Numerics.h"

#include <vector>
#include <stdexcept>

using namespace std;

const char headersMg[] =   "range=99 hr0=MHz hr1=eta hc0=z hc1=x hc2=y";
const char headersMgAl[] = "range=99 hr0=MHz hr1=eta hc0=zCOM hc1=zSTR hc2=xSTR hc3=xCOM hc4=ySTR hc5=yCOM";

unsigned numSB(const std::string& name)
{
	if(strcmp(name.c_str(), "Mg") == 0)
		return 3;
	   
	if(strcmp(name.c_str(), "Mg Al") == 0)
		return 6;

        return 6;
}

const char* tableHeaders(const std::string& name)
{
	if(strcmp(name.c_str(), "Mg") == 0)
		return headersMg;
	   
                return headersMgAl;
}

iMg::iMg(list_t* exp_list, const std::string& name) :
     transition_info(exp_list, name, "Mg", numSB(name), 6),
     Padding(TTL_START_EXP, "Mg Padding",       &params, "t=20"),
     num_repump  ("Repump pulses [#]",			&params, "value=0"),
     cooling_start("Cooling start n=",			&params, "value=0"),
     cooling_reps ("Cooling reps [#]",			&params, "value=0"),
	 motion(2, numSB(name), "motion", &params, tableHeaders(name)),
	 old_eta(numSB(name), 0)
{
    Padding.setFlag(RP_FLAG_NOLINK);

    add_new_pulse(pulse_spec(-2*1, -1, 0), &params);
    add_new_pulse(pulse_spec(-2*2, 0, 0), &params);

    for(int sb=-1*sbMax; sb<=sbMax; sb++)
            add_new_pulse(pulse_spec(-3*2, 1, sb), &params);
}

void iMg::setIonXtal(const char* xtal_name)
{
    if(strcmp(this->name.c_str(), xtal_name) == 0)
    {
        printf("Using page %s for pulse information.\n", xtal_name);
        gpMg = this;
    }
}

raman_pulse* iMg::getCoCarrier()
{
	return dynamic_cast<raman_pulse*>(getPulse(-2*2, 0, 0));
}

dds_params* iMg::new_pulse(const pulse_spec& p, params_t* pparams)
{
    dds_params* dp;

    if(p.pol == 0 && p.sb == 0)
            dp = new raman_pulse(pulse_name(p.mFg2, 0, 0), pparams, TTL_RAMAN_CO);
    else
            dp = new raman_pulse(pulse_name(p.mFg2, p.pol, p.sb), pparams, TTL_RAMAN_90);

    dp->setFlag(RP_FLAG_NOLINK);

    return dp;
}

raman_pulse* iMg::getSB(int sb) // positive for BSB, negative for RSB, 0 for carrier
{
	return dynamic_cast<raman_pulse*>(getPulse(-2*3, 1, sb));
}

void iMg::calcStretchFactors()
{
   //calculate gs-cooling pulses
   unsigned nMax = N_STRETCH_FACTORS;
   unsigned nModes = motion.value.nc;

   std::vector<double> stretch0(nModes);

   for(unsigned i=0; i<nModes; i++)
   {
	   //only recalculate if eta changed
	   if(old_eta[i] == eta(i))
		   continue;

	   old_eta[i] = eta(i);

      printf("eta(sb%d) = %6.3f\n", i, eta(i));
	  stretch0[i] = 1000*numerics::motional_rabi_factor(0, 1, eta(i), 1);

      for(unsigned n=0; n<nMax; n++)
      {
         stretch_factor[i][n] = static_cast<unsigned>(floor(0.5+stretch0[i]/numerics::motional_rabi_factor(n, n+1, eta(i), 1)));
         printf("sb(%d) %d -> %d  stretch = %6.3f\n", i, n+1, n, stretch_factor[i][n]*0.001);
      }
   }
}


double iMg::eta(unsigned iMode)
{
	if(iMode >= motion.value.nc)
		throw runtime_error("[iMg::eta] no such mode");

	return motion.value.element(1, iMode);
}

void iMg::updateParam(const char* name, const char* value)
{
	transition_info::updateParam(name, value);

	if(strcmp(name, "motion") == 0)
	{
		calcStretchFactors();
	}
}

void iMg::update_param_binary(unsigned param_id, unsigned length, const  char* bin)
{
	transition_info::update_param_binary(param_id, length, bin);

	if(strcmp(params.at(param_id)->getName().c_str(), "motion") == 0)
	{
		calcStretchFactors();
	}
}
