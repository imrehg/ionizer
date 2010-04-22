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
	numAl(0), numMg(0), num3P1Pulses(0), num3P0states(0),
	det_means(4),
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

   for(unsigned j=0; j < det_means.size(); j++)
   {
      char buff[256];
      snprintf(buff, 256, "%u  Mg mean", j);

      det_means[j] = new rp_double(buff, &params, "value=0");
   }

   //initialize detection histograms
   initStats();
}

iAl3P0::~iAl3P0()
{
   if(fc)
      delete fc;
}

const char *iAl3P0::state_str(int x)
{
	unsigned nBits = std::min<unsigned>(numAl, 32);

	static char bin_str[33];

	for(unsigned i=0; i<nBits; i++)
		bin_str[nBits-i-1] = ((x >> i) & 1) ? 'P' : 'S';

	bin_str[nBits] = 0;

	return bin_str;
}


//! initialize detection statistics (allocate histograms, etc.)
void iAl3P0::initStats()
{
   unsigned i=0;

   num3P0states = 1 << numAl;

   //number of different detection pulse types.  
   //works for Mg Al and Mg Al Al, but may need modification eventually
   num3P1Pulses = numAl; 


   unsigned newSize=2*num3P0states*num3P1Pulses;

   if(newSize == det_stats.size())
	   return;

   for(unsigned k=0; k < det_stats.size(); k++)
	   for(unsigned j=0; j < det_stats[k].gui_weights.size(); j++)
	   {
		   if(det_stats[k].gui_weights[j])
		   {
			   delete det_stats[k].gui_weights[j];
			   det_stats[k].gui_weights[j] = 0;
		   }
	   }

   det_stats.clear();
   det_stats.resize(newSize);

   for(int mF2=-5; mF2<=5; mF2+=10)
   {
      for(unsigned j=0; j<num3P0states; j++)
      {
		  for(unsigned k=0; k<num3P1Pulses; k++)
		  {
			  unsigned iDet = getHistIndex(j,mF2,k);
			  det_stats[iDet].set_num_poissonians(numMg+1);

			  for(unsigned q=0; q < numMg; q++)
			  {
				 char buff0[256];
				 char buff1[256];

				 snprintf(buff0, 256, "pulse %d, Al %s, mF=%d/2", k, state_str(j), mF2);
				 snprintf(buff1, 256, "pulse %d, Al %s, mF=%d/2 (%d Mg) weight", k, state_str(j), mF2, q);

				 det_stats[iDet].setName(buff0);

				 if(q == 0)
					det_stats[iDet].gui_weights[q] = new rp_double(buff1, &params, "value=1");
				 else
				    det_stats[iDet].gui_weights[q] = new rp_double(buff1, &params, "value=0");

			  }
		  }
      }

      i++;
   }

   for(unsigned j=0; j <= numMg; j++)
   {
      for(unsigned k=0; k < det_stats.size(); k++)
         det_stats[k].gui_means[j] = det_means[j];
   }
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


unsigned iAl3P0::getNumPlots()
{
	return det_stats.size();
}


//! Return data for plots on a GUI page.  The format is msg_out.m[0] = num_points, msg_out.m[1] = data[0], ...
void iAl3P0::getPlotData(unsigned iPlot, unsigned /* iStart */, GbE_msg& msg_out)
   {
//	printf("[exp_3P0::getHistogramData] iHist=%u\r\n", iHist);

   if(iPlot < det_stats.size() )
   {
      unsigned maxC = det_stats[iPlot].getMaxCounts();
      msg_out.insertU(0, maxC);

      for(unsigned j=0; j<maxC; j++)
      {
         msg_out.insertU(j+1, static_cast<unsigned>(1000*det_stats[iPlot].probability(j)));
   //		printf("[exp_3P0::getHistogramData] p(%u)=%u/1000\r\n", j+1, m[j+1]);
      }
   }
   else
	   msg_out.insertU(0, 0);

}


void iAl3P0::resetStats(const GbE_msg&, GbE_msg&)
{
   resetStats();
}

void iAl3P0::resetStats()
{
   for(unsigned j=0; j<det_stats.size(); j++)
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

unsigned iAl3P0::getNum3P0states()
{
	return num3P0states;
}

unsigned iAl3P0::getHistIndex(unsigned n3P0, int mF2, unsigned pulse_type)
{
	int imF = (mF2 == -5 ? 0 : 1);
	return imF*(num3P0states)*(num3P1Pulses) + pulse_type*(num3P0states) + n3P0;
}

double iAl3P0::getProb(unsigned n3P0, int mF2, unsigned pulse_type, unsigned n)
{
    return det_stats[getHistIndex(n3P0,mF2,pulse_type)].probability(n);
}

//! return mean value of histogram "iHist"
double iAl3P0::getMean(unsigned n3P0, int mF2, unsigned pulse_type)
{
   return det_stats[getHistIndex(n3P0,mF2,pulse_type)].getOverallMean();
}

const std::string& iAl3P0::getHistName(unsigned n3P0, int mF2, unsigned pulse_type)
{
   return det_stats[getHistIndex(n3P0,mF2,pulse_type)].getName();
}

void iAl3P0::getHistName(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	unsigned iHist = msg_in.extractU(1);

	strcpy(msg_out.extractS(0), det_stats.at(iHist).getName().c_str());
}

void iAl3P0::updateHist(unsigned n3P0, int mF2, unsigned pulse_type, unsigned n)
{
   det_stats[getHistIndex(n3P0,mF2,pulse_type)].update(n);
}

void iAl3P0::updateHistMemory(unsigned n3P0, int mF2, unsigned pulse_type, double det_memory)
{
   det_stats[getHistIndex(n3P0,mF2,pulse_type)].update_memory(det_memory);
}

void iAl3P0::setIonXtal(const char* name)
{
	numAl = numOccurences(name, "Al");
	numMg = numOccurences(name, "Mg");

	initStats();
}


#endif //CONFIG_AL
