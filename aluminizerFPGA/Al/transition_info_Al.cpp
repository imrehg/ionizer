#ifdef CONFIG_AL

#include "../common.h"
#include "../config_local.h"

#include "transition_info_Al.h"

#include "../shared/src/Numerics.h"

#include <vector>
#include <stdexcept>

using namespace std;

iAl3P1::iAl3P1(list_t* exp_list, const std::string& name) :
     transition_info(exp_list, name, "Al3P1", 4, 5),
	 motion(4, 6, "motion", &params, "range=99 hr0=MHz hr1=eta(0) hr2=eta(1) hr3=eta(2) hc0=zCOM hc1=zSTR hc2=xSTR hc3=xCOM hc4=ySTR hc5=yCOM"),
     waveplate_sp("Al3P1 Waveplate s+", &params, "value=0"),
     waveplate_sm("Al3P1 Waveplate s-", &params, "value=0"),
    mFg_current(0)
{
   for(int sb=-1*sbMax; sb<=sbMax; sb++)
      add_new_pulse(pulse_spec(-5, -1, sb), &params);

   add_new_pulse(pulse_spec(-5, 1, 0), &params);

   for(int mFg2=-3; mFg2<=3; mFg2+=2)
      for(int pol=-1; pol<=1; pol+=2)
         add_new_pulse(pulse_spec(mFg2, pol, 0), &params);

   add_new_pulse(pulse_spec(5, -1, 0), &params);

   for(int sb=-1*sbMax; sb<=sbMax; sb++)
      add_new_pulse(pulse_spec(5, 1, sb), &params);
}

dds_params* iAl3P1::new_pulse(const pulse_spec& p, params_t* pparams)
{
   return new Al3P1_pulse(pulse_name(p.mFg2, p.pol, p.sb), pparams, TTL_3P1_SIGMA, "t=1 fOn=240 fOff=0");
}

//optically pump +5/2 or -5/2 state
//skip over 1/2 -> 3/2 transition
void iAl3P1::prepState(int mFg_target)
{
   if(mFg_target > 0)
   {
       for(int mFg2=-5; mFg2<5; mFg2+=2)
          if(mFg2 != 1)
            getPulse(mFg2, 1, 0)->pulse();
   }
   else
   {
      for(int mFg2=5; mFg2>-5; mFg2-=2)
          if(mFg2 != -1)
            getPulse(mFg2, -1, 0)->pulse();
   }
}

iAl3P0::iAl3P0(list_t* exp_list, const std::string& name) :
     transition_info(exp_list, name, "Al3P0", 0, 5),
    clock_state(0),
    check_interval("Check interval [us]", &params, "value=1000"),
    dark_rate("Dark count rate [1/ms]", &params, "value=1"),
    bright_rate("Bright count rate [1/ms]", &params, "value=20"),
     min_ok_prob("Min. bright prob. [10^]", &params, "value=-5"),
    fc(0)
{
   //pi-polarized pulses
   for(int mFg2=-5; mFg2<=5; mFg2+=2)
   {
      Al3P0_pi_pulses.push_back(new Al3P0_pulse(pulse_name(mFg2, 0, 0), &params));
   }

   //initialize detection histograms

   unsigned i=0;
   for(int mF2=-5; mF2<=5; mF2+=10)
   {
      for(unsigned j=0; j<2; j++)
      {
         char buff[256];

         if(j == 1)
            snprintf(buff, 256, "3P0 mF=%d/2 (0 Mg) weight", mF2);
         else
            snprintf(buff, 256, "1S0 mF=%d/2 (0 Mg) weight", mF2);

         rp_double* p = new rp_double(buff, &params, "value=0.5");
         det_stats[j+2*i].gui_weights[0] = p;
      }

      i++;
   }

   for(unsigned j=0; j<2; j++)
   {
      char buff[256];
      snprintf(buff, 256, "%u  Mg mean", j);

      rp_double* p = new rp_double(buff, &params, "value=0");
      det_means[j] = p;

      for(unsigned k=0; k<numHist; k++)
         det_stats[k].gui_means[j] = p;
   }
}

iAl3P0::~iAl3P0()
{
   if(fc)
      delete fc;
}

FluorescenceChecker* iAl3P0::getFC()
{
   if(!fc)
      fc = new FluorescenceChecker(check_interval*100, dark_rate * 1e-5, bright_rate * 1e-5, pow(10.0, min_ok_prob), 1e-2);

   return fc;
}

void iAl3P0::getClockState(const GbE_msg&, GbE_msg& msg_out)
{
   snprintf(msg_out.extractS(0), MAX_STR_LENGTH, "state=%4.2f mF=%d/2", clock_state, gpAl3P1->mFg_current );

   if(iFPGA->debug_clock)
      printf("%s\r\n", msg_out.extractS(0));
}

dds_params* iAl3P0::new_pulse(const pulse_spec& p, params_t* pparams)
{
   return new Al3P0_pulse(pulse_name(p.mFg2, p.pol, p.sb), pparams);
}

Al3P0_pulse* iAl3P0::getSB(int mFg, int sb, int pol)
{
    if(sb != 0 || pol != 0 || abs(mFg) > 5)
        throw runtime_error("[iAl3P0::getSB] error, only sb=0, pol=0, |mFg| <= 5/2 supported");
    else
        return Al3P0_pi_pulses[mFg+5];
}


void iAl3P0::getHistogramData(unsigned iHist, GbE_msg& msg_out)
{
//	printf("[exp_3P0::getHistogramData] iHist=%u\r\n", iHist);

   if(iHist < numHist)
   {
      unsigned maxC = det_stats[iHist].getMaxCounts();
      msg_out.insertU(0, maxC);

      for(unsigned j=0; j<maxC; j++)
      {
         msg_out.insertU(j+1, static_cast<unsigned>(1000*det_stats[iHist].probability(j)));
   //		printf("[exp_3P0::getHistogramData] p(%u)=%u/1000\r\n", j+1, m[j+1]);
      }
   }
}

void iAl3P0::getHistogramData(const GbE_msg& msg_in, GbE_msg& msg_out)
{
   unsigned iHist = msg_in.extractU(0);

   printf("[getHistogramData] iHist=%u\r\n", iHist);
   getHistogramData(iHist, msg_out);
}

void iAl3P0::resetStats(const GbE_msg&, GbE_msg&)
{
   resetStats();
}

void iAl3P0::resetStats()
{
   for(unsigned j=0; j<numHist; j++)
      det_stats[j].recalc();
}

double iAl3P0::getClockState()
{
   return clock_state;
}

void iAl3P0::setClockState(double d)
{
   clock_state = d;
}

unsigned iAl3P0::getHistIndex(unsigned n3P0, int mF2)
{
   if(mF2 == -5)
      return n3P0;

    if(mF2 == 5)
      return n3P0+2;

    throw runtime_error("unknown histogram");
}

double iAl3P0::getProb(unsigned n3P0, int mF2, unsigned n)
{
   if(mF2 == -5)
      return det_stats[getHistIndex(n3P0,mF2)].probability(n);

    if(mF2 == 5)
      return det_stats[getHistIndex(n3P0,mF2)].probability(n);

    throw runtime_error("unknown histogram");
}

//! return mean value of histogram "iHist"
double iAl3P0::getMean(unsigned n3P0, int mF2)
{
   return det_stats[getHistIndex(n3P0,mF2)].getOverallMean();
}

const std::string& iAl3P0::getHistName(unsigned n3P0, int mF2)
{
   return det_stats[getHistIndex(n3P0,mF2)].getName();
}

void iAl3P0::getHistName(const GbE_msg&, GbE_msg&)
{
//not yet implemented
}

void iAl3P0::updateHist(unsigned n3P0, int mF2, unsigned n)
{
   det_stats[getHistIndex(n3P0,mF2)].update(n);
}

void iAl3P0::updateHistMemory(unsigned n3P0, int mF2, double det_memory)
{
   det_stats[getHistIndex(n3P0,mF2)].update_memory(det_memory);
}

#endif //CONFIG_AL
