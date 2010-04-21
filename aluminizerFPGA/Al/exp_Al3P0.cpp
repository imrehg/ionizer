#ifdef CONFIG_AL

#include <vector>
#include <algorithm>
#include <sstream>

#include "../common.h"
#include "../host_interface.h"
#include "../ttl_pulse.h"
#include "../dds_pulse.h"
#include "dds_pulse_info.h"

#include "../remote_params.h"
#include "exp_Al3P0.h"
#include "../shared/src/Numerics.h"
#include "../exp_recover.h"


using namespace std;

extern exp_3P0_lock* e3P0LockPQ;
extern exp_3P0_lock* e3P0LockMQ;

exp_3P0::exp_3P0(list_t* exp_list, const std::string& name) :
        exp_3P1(exp_list, name),
   alt_xfer_sb("Alt. xfer sb", &params, "value=2"),
   min_prob("Min. probability", &params, "value=0.99"),
   det_memory("Det. memory", &params, "value=1000"),
   //clockTTL("Clock TTL", &params, "value=0"),
   exp_pulse	("experiment pulse", &params),
   checked_pulse ("3P0 Fl. check", &params, "value=0"),
   mod3P1("3P1 modulation [1/tPi]", &params, "value=0.1"),
   gain3P1("3P1 gain", &params, "value=0.1"),
   off_detuning("Off detuning [Hz]", &params, "value=0.1"),
   Ramsey		(0, "Ramsey",			&params, "t=0"),
   RamseyPhase ("Ramsey phase [deg.]",	&params, "value=0"),
   rcClockState(channels, "3P0 state"),
   rcXition(channels, "Clock xition (s)"),
   rcNumDetections(channels, "# det."),
   rcFC(channels, "Flour. check [counts / 100 us]"),
   rc3P1corr(channels, "3P1 corr. [kHz]"),
   probe_dir(1)
{
   rcSignal.name = COOLING_ION_NAME + std::string(" signal");

   alt_xfer_sb.setExplanation("used to determine state of 2nd (inner) Al+");
   mod3P1.setExplanation("to extract 3P1 freq. servo signal");
   gain3P1.setExplanation("3P1 freq. servo integral gain");
   off_detuning.setExplanation("Detuning used to switch off light (for bi-directional probes)");
}

//! figure out correct polarizations, etc
void exp_3P0::init()
{
   //exp_pulse.ttl = clockTTL;

   exp_pulse.set_port(exp_pulse_port);

   pol3P1 = exp_pulse_gs > 0 ? 1 : -1;

   //make sure FluorescenceChecker is available
   fc = gpAl3P0->getFC();

   rcXition.result = 0;
   rc3P1corr.result = 0;

   exp_3P1::init();
}


void exp_3P1::prepAndCheck(int mFg2)
{
   if(gpAl3P1->mFg_current != mFg2)
   {
      unsigned nDet = 0;
      unsigned nTries = 0;
      unsigned nMax = 20;
      double max3P0 = 1e-3;

     if(debug_level > 0)
      printf("checking for Al+ 1S0 mFg=%d/2 state\r\n", mFg_target);

    //use 3P0 lock experiment for state checking
    exp_3P0* pExp3P0 = dynamic_cast<exp_3P0*>(this);

    if(pExp3P0 == 0)
    {
       pExp3P0 = mFg2 > 0 ? e3P0LockPQ : e3P0LockMQ ;

       //make sure params are initialized correctly
       /*[TR] 10/28/2009 bug discovered.  This line causes probe-direction changes to be inconsistent.
         What does init actually do?
       */

       pExp3P0->init();
    }

    while(pExp3P0->get_clock_state(&nDet, false, false) > max3P0 && nTries < nMax)
      {
       if(debug_level > 0)
         printf("re-prepare via Al+ 3P1\r\n");

         for(unsigned j=0; j<100; j++)
         {
            DopplerCool.pulse();
            gpAl3P1->prepState(mFg_target);
         }

         nTries++;
      }

      if(nTries < nMax)
         printf("success!");
      else
         printf("failure!");

      gpAl3P1->mFg_current = mFg2;
   }
}

void exp_3P0::run(const GbE_msg& msg_in, GbE_msg& msg_out)
{
   init_exp_sequence(msg_in, msg_out);

   init_exp(0);

   //start timing check
   pmt.begin_timing_check(Padding.t, Padding.ttl);

   //pre-cool
   Precool.pulse();
   Precool.ddsOff();

   //3P0 pulse
   makeClockPulse(0, 0);

   //stop timing check
   pmt.end_timing_check();

   unsigned num_detections = 0;
   double corr3P1 = 0;
   double new_state = get_clock_state(&num_detections, &corr3P1, true);
   double old_state = gpAl3P0->getClockState();

   rcClockState.result =10*new_state;
   rcXition.result = 10.*fabs(new_state-old_state);
   rcNumDetections.result = (0.1*num_detections);
   rc3P1corr.result = 1e-3 * gain3P1 * corr3P1;

   gpAl3P0->setClockState(new_state);

   if(bDebugPulses)
   {
      sprintf(host->buffDebug, "Finish pulse sequence for experiment: %s\n\n\n", name.c_str());
      host->sendDebugMsg(host->buffDebug, true);
   }

   bDebugPulses = false;

   finish_exp_sequence(msg_out);
}

void exp_3P0::makeClockPulse(double dF0, double dF1)
{

   Repump.pulse();
   DopplerCool.pulseStayOn();

   if(!Ramsey.bEnabled)
   {
        if(checked_pulse)
        {
         unsigned n3P0Counts = 0;
         if(exp_pulse.checked_pulse(fc, &n3P0Counts))
         rcFC.result = 10000.0 * n3P0Counts / (double)exp_pulse.t;
        else
        {
           if(debug_level > 0)
            printf("[exp_3P0::run] checked_pulse returned false.  recovering ions ... \r\n");

           rcFC.result = -1;
        }
        }
        else
        {
           if(exp_pulse_port == 2)
         {
              exp_pulse.bi_directional_pulse(dF0, dF1);

           if(debug_level > 0)
              printf("[exp_3P0::makeClockPulse] bi-directional clock pulse, df0 = %6.3f df1 = %6.3f Hz\r\n", dF0, dF1);
         }
           else
              exp_pulse.shifted_pulse(exp_pulse_port == 0 ? dF0 : dF1);

           rcFC.result = 0;
        }


   }
   else
   {
      exp_pulse.ramsey_pulse(&Ramsey, (double)RamseyPhase);
   }

    DopplerCool.ddsOff();
}

void exp_3P0::experiment_pulses(int) {}

bool operator<(const state_prob& p1, const state_prob& p2)
{
	return p1.P > p2.P; //flip < to >, to achieve descending order with std::sort
}

unsigned exp_3P0::decide_next_pulse_type(vector<state_prob>& P)
{
	//only two states to distinguish (Mg Al)
	if(P.size() <= 2)
		return 0;

	//Four states to distinguish (Mg Al Al)

	//sort state probabilities in descending order
	std::sort(P.begin(), P.end());

	//the "2" bit refers to the inner Al+ ion
	//the "1" bit refers to the outer Al+ ion

	//trying to determine state of outer ion
	if( (P[0].iState % 2) != (P[1].iState % 2) )
	{
		return 0; //pulse type 0 will drive the stretch mode
	}

	//trying to determine state of inner ion
	if( (P[0].iState / 2) != (P[1].iState / 2) )
	{
		return 1; //pulse type 0 will drive the egyptian mode
	}

	//shouldn't be possible to get here
	return 0;
}

//use maximum likelihood method to determine Bayesian mean of ion state
//returns number of excited Al+ ions
double exp_3P0::get_clock_state(unsigned* num_detections, double* pCorr3P1, bool bStoreData, bool bUpdateStats)
{
   vector<state_prob> P(gpAl3P0->getNumPlots());

   double P0 = 1.0/P.size();

   for(unsigned i=0; i<P.size(); i++)
   {
	   P[i].iState = i;
	   P[i].P = P0;
   }

   double Psum, Pmax;

   *num_detections = 0;

   unsigned nMax  = num_exp-10;
   unsigned iPmax;

   //store PMT values here to update histograms
   pmt_array.resize(nMax);
   pulse_type.resize(nMax);

   //get pointer to 3P1 sb pulse
   dds_params* pulse3P1xfer = gpAl3P1->getPulse(mFg_target, pol3P1, abs(xfer_sb));
   dds_params* pulse3P1alt = gpAl3P1->getPulse(mFg_target, pol3P1, abs(alt_xfer_sb));

   //calculate 3P1 freq. offsets for servo signal
   double shift3P1 = mod3P1/(TIME_UNIT*(pulse3P1xfer->t));
   double servoSignal3P1 = 0; //3P1 servo signal

    int mF2 = (int)(2*exp_pulse_gs);

	unsigned curr_pulse_type = 0;

   double pmtMean3P0 = gpAl3P0->getMean(1, mF2, curr_pulse_type);
   double pmtMean1S0 = gpAl3P0->getMean(0, mF2, curr_pulse_type);

   shift3P1 *= (2*(rand() % 2) - 1);


   if(debug_level > 1)
      printf("Check ion state ...\n");
   do
   {
	  curr_pulse_type = decide_next_pulse_type(P);

      //start timing check
      pmt.begin_timing_check(Padding.t, Padding.ttl);

      //3P1 pumping and ground-state cooling
      preparation_pulses();

	  //qubit transfer sequence
	  if(curr_pulse_type = 0)
	  {
		  pulse3P1xfer->shifted_pulse(shift3P1);
		  gpMg->getSB(-1*abs(xfer_sb))->pulse();
	  }
	  else
	  {
		  pulse3P1xfer->shifted_pulse(shift3P1); //bsb stretch (removes outer Al+ from the picture)
		  pulse3P1alt->shifted_pulse(0); //bsb egyptian (only goes if inner Al+ is in 1S0)
		  gpMg->getSB(-1*abs(alt_xfer_sb))->pulse();
	  }

      Detect.detection_pulse();
      Precool.pulseStayOn();

      //stop timing check
      pmt.end_timing_check();

      unsigned n = pmt.get_new_data(1, bStoreData);
      pmt_array[(*num_detections)] = n;
	  pulse_type[(*num_detections)] = curr_pulse_type;

      Psum = 0;
      Pmax = 0;
      iPmax = 0;

      for(unsigned i=0; i<P.size(); i++)
      {
         P[i].P *= gpAl3P0->getProb(P[i].iState, mF2, curr_pulse_type, n);
         Psum += P[i].P;

         if(P[i].P > Pmax)
         {
            Pmax = P[i].P;
            iPmax = P[i].iState;
         }
      }

     servoSignal3P1 += shift3P1 * (n - pmtMean3P0);

      if(debug_level > 1)
         printf("%u counts, P[%u]: %e\n", n, iPmax, Pmax/Psum);

      (*num_detections)++;
     shift3P1 *= -1; //flip 3P1 shift

   } while( (Pmax/Psum < min_prob) && ((*num_detections) < nMax) );

   //calc Bayesian mean
   double m = 0;
   for(unsigned i=0; i<P.size(); i++)
      m += P[i].iState * P[i].P;

   if(bUpdateStats && ((*num_detections) <= nMax))
   {
      for(unsigned i=0; i<(*num_detections); i++)
	  {
         gpAl3P0->updateHist(iPmax, mF2, pulse_type[i], pmt_array[i]);
		 gpAl3P0->updateHistMemory(iPmax, mF2, pulse_type[i], det_memory * (*num_detections) );
	  }
   }

   m/= Psum;

   if(debug_level > 1)
      printf("Bayesian mean = %e\n", m);

   //if the ion is likely in the ground state, extract 3P1 correction signal
   if(m < 0.5 && pCorr3P1)
      *pCorr3P1 = servoSignal3P1 / ((*num_detections) * (pmtMean3P0 - pmtMean1S0));

   return m;
}


exp_3P0_lock::exp_3P0_lock(list_t* exp_list, const std::string& name, unsigned num_freq) :
      exp_3P0(exp_list, name),
      debug_clock("Debug clock", &params, "value=1"),
      force1S0("Force 1S0", &params, "value=1"),
     gainPoints("Gain points", &params, "value=10"),
      xtraPoints("Extra points", &params, "value=1"),
     lineWidths("Line widths", &params, "value=1"),
      nProbeFreqs(num_freq),
      num_clock_transitions(num_freq),
      rcErr(channels, "FPGA error signal (s)"),
      rcDirection(channels, "probe dir(.)"),
      rcXitionProb(num_freq),
      rcNumProbes(num_freq),
      rcProbeFreq0(num_freq),
     rcProbeFreq1(num_freq)
{
   rcXition.name = "Clock xition";
   rcClockState.name = "3P0 state (h)";

   for(unsigned i=0; i<num_freq; i++)
   {
      char s[32];
      snprintf(s, 30, "xition prob. (%u) (h)", i);
      rcXitionProb[i] = new result_channel(channels, s);

      snprintf(s, 30, "clock probes (%u) [#](h)", i);
      rcNumProbes[i] = new result_channel(channels, s);

     snprintf(s, 30, "probe det0 (%u) [Hz](h)", i);
      rcProbeFreq0[i] = new result_channel(channels, s);

     snprintf(s, 30, "probe det1 (%u) [Hz](h)", i);
      rcProbeFreq1[i] = new result_channel(channels, s);
   }
}

double exp_3P0_lock::getFreq(int i)
{
  double r = freqScale*(i-centerIndex);
 //  printf("[exp_3P0_lock::getFreq] %d, freqScale=%f, result=%f\r\n", i, freqScale, r);
 
   return r;
}

double exp_3P0_lock::getGain(int i)
{
   if(i == lowIndex)
      return -1;

   if(i == highIndex)
      return 1;

   return 0;
}

//! randomize probe freq. sequence & alternate probe direction
void exp_3P0_lock::reshuffle()
{
   exp_3P0::reshuffle();

   for(unsigned i=0; i<nProbeFreqs; i++)
   {
      rcNumProbes[i]->result = 0;
      rcXitionProb[i]->result = 0;
      num_clock_transitions[i] = 0;
   }

   double FWHM = 0.8 / (TIME_UNIT * exp_pulse.t);

   if(Ramsey.bEnabled)
	   FWHM = 0.5 / (TIME_UNIT * Ramsey.t);

   freqScale = FWHM * lineWidths / (nProbeFreqs-1);
   
   printf("Ramsey.t = %f, FWHM = %f, freqScale = %f\r\n", (double)(TIME_UNIT * Ramsey.t), FWHM, freqScale);

   //figure out probe freq's where we will apply gain
   double lowProbeFreq = -0.5*FWHM;
   lowIndex = 0;
   centerIndex = (nProbeFreqs-1)/2;

   //find nearest point
   for(unsigned i=1; i<nProbeFreqs; i++)
   {
      if( fabs(lowProbeFreq - getFreq(i)) > fabs(lowProbeFreq - getFreq(i-1)) )
      {
         lowIndex = i-1;
         break;
      }
   }

   highIndex = 2*centerIndex - lowIndex;

   //randomly shuffle probe order

   //1. figure out total number of probes
   unsigned numProbeTotal = gainPoints + xtraPoints;

   //2. make ordered list of probes with non-zero gain
   probe_order.resize(numProbeTotal);

   unsigned k = 0;
   for(unsigned i=0; i<nProbeFreqs; i++)
      if(getGain(i) != 0)
      {
        for(unsigned j=0; j<(gainPoints/2); j++)
          probe_order[k++] = i;
       }

  //fill rest of list with points where gain == 0
  while(k < probe_order.size())
  {
     //find points with gain == 0 by searching randomly
     unsigned q=0;

     do
     {
        q = rand() % nProbeFreqs;
     } while( getGain(q) != 0);

     probe_order[k++] = q;
  }

   //3. shuffle
   random_shuffle(probe_order.begin(), probe_order.end());

   //4. choose direction at beginning of sequence

   //2 MEANS BI-DIRECTIONAL
   if(exp_pulse_port == 2)
      probe_dir = probe_dir * -1;
   else
      probe_dir = 1 - 2*exp_pulse_port;

   rcDirection.result = probe_dir;

   for(unsigned i=0; i<nProbeFreqs; i++)
   {
      double dF = getFreq(i);
      double dF0 = probe_dir > 0 ? dF : 0;
      double dF1 = probe_dir < 0 ? dF : 0;

     if(exp_pulse_port == 2)
     {
       if(probe_dir > 0)
       {
         dF0 = dF;
         dF1 = dF + off_detuning * (2 * (rand() % 2) - 1);
       }
       else
       {
         dF0 = dF + off_detuning * (2 * (rand() % 2) - 1);
         dF1 = dF;
       }
     }

     rcProbeFreq0[i]->result = dF0;
     rcProbeFreq1[i]->result = dF1;
   }
}
 

void exp_3P0_lock::run(const GbE_msg& msg_in, GbE_msg& msg_out)
{
   init_exp_sequence(msg_in, msg_out);

   int iProbe = 0;

   //overtime means we are working extra to reach 1S0
   //running backwards through probe sequence
   bool bOvertime = false;

   double phase_scale = -360.0 * (TIME_UNIT * Ramsey.t);
  
   while(1)
   {
      unsigned probe_index = probe_order[iProbe];
      double dF = getFreq(probe_index);

	  RamseyPhase.set(dF * phase_scale);
	  
      if(debug_level > 0)
      {
         printf("clock pulse (%u) f0 = %7.2f Hz dF = %6.2f Hz phi = %f deg... ", iProbe, exp_pulse.get_freq(), dF, (double)RamseyPhase);
      }

      init_exp(iProbe);

      //calculate detunings
      double dF0 = rcProbeFreq0[probe_index]->result;
      double dF1 = rcProbeFreq1[probe_index]->result;

      //start timing check
      pmt.begin_timing_check(Padding.t, Padding.ttl);

      //pre-cool
      Precool.pulse();
      Precool.ddsOff();

      //3P0 pulse at modulated frequency/phase
      makeClockPulse(dF0, dF1);

      //stop timing check
      pmt.end_timing_check();

      unsigned num_detections = 0;

      double old_state = gpAl3P0->getClockState();
      double corr3P1 = 0;
      double new_state = get_clock_state(&num_detections, &corr3P1, false, true);

     rcNumDetections.result += num_detections;
      rc3P1corr.result += 1e-3 * gain3P1 * corr3P1;

      gpAl3P0->setClockState(new_state);

      double pX = fabs(new_state - old_state);

      rcNumProbes[probe_index]->result += 1;
      num_clock_transitions[probe_index] += pX;

      if(debug_clock)
      {
         printf(" pX = %6.5f (%u det)\r\n", pX, num_detections);
      }

      if(bDebugPulses)
      {
         sprintf(host->buffDebug, "Finish pulse sequence for experiment: %s\n\n\n", name.c_str());
         host->sendDebugMsg(host->buffDebug, true);
      }

      bDebugPulses = false;

      if(bOvertime)
         iProbe--;
      else
         iProbe++;

      if(iProbe == (int)(probe_order.size()))
      {
         if(force1S0)
         {
            bOvertime = true;
            iProbe--;
         }
         else
            break;
      }

      if(bOvertime)
      {
         if(iProbe < 0)
            break;

         if(new_state < (1 - min_prob))
            break;
      }
   }

   double gainSum = 0;

   for(unsigned i=0; i<nProbeFreqs; i++)
      gainSum += fabs(getGain(i));

   if(gainSum == 0)
      gainSum = 1;

   rcErr.result = 0;

   double total_probes = 0;

   for(unsigned i=0; i<num_clock_transitions.size(); i++)
   {
     if(rcNumProbes[i]->result > 0)
     {
      double N = rcNumProbes[i]->result;
     total_probes += N;
      rcXitionProb[i]->result = num_clock_transitions[i]/N;
      rcXition.result += num_clock_transitions[i];
      rcErr.result += rcXitionProb[i]->result * getGain(i) / gainSum;

      if(debug_clock)
      {
         printf(" total pX (%u) = %6.3f (%u shots) ",   i, rcXitionProb[i]->result, (unsigned)N);
         printf("f = (%6.3f + %6.3f) Hz\r\n", exp_pulse.get_freq(), getFreq(i));
      }
     }
     else
        rcXitionProb[i]->result = 0;
   }

   rcNumDetections.result /= (total_probes*100);
   rcXition.result /= total_probes;

   printf(" error signal = %6.3f\r\n", rcErr.result);

   finish_exp_sequence(msg_out);
}

#endif //CONFIG_AL
