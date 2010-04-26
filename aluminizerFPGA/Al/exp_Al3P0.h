#ifndef EXP_AL3P0_H_
#define EXP_AL3P0_H_

#include "exp_Al3P1.h"

class FluorescenceChecker;

struct state_prob
{
	double P;
	unsigned iState;
};

class exp_3P0 : public exp_3P1
{
public:
    exp_3P0(list_t* exp_list, const std::string& name);
   virtual ~exp_3P0() {}

   virtual void setIonXtal(const char*);

   //virtual void init_exp_sequence(const GbE_msg& msg_in, GbE_msg& msg_out);
   virtual void init();
   virtual void run(const GbE_msg& msg_in, GbE_msg& msg_out);

     //! Determines Bayesian mean of ion state
   /*! Returns number of excited Al+ ions and
    *  copies 3P1 frequency correction to pCorr3P1.
   */
   double get_clock_state(unsigned* num_detections, double* pCorr3P1, bool bStoreData, bool bUpdateStats=true);


protected:
   //! make clock pulse at shifted freq.
   /*! If the pulse is bi-directional, dF0 and dF1 specify the shifts of the two directions */
   void makeClockPulse(double dF0, double dF1);

   virtual void experiment_pulses(int iExp);
   unsigned decide_next_pulse_type(vector<state_prob>& P);
   
   rp_int alt_xfer_sb;

   rp_double min_prob;
   rp_unsigned det_memory;
   Al3P0_pulse exp_pulse;
   rp_bool checked_pulse;
   rp_double off_detuning;
   ttl_params Ramsey;
   rp_double RamseyPhase;
   rp_double mod3P1, gain3P1;

   vector<result_channel*> rcClockStates;
   vector<result_channel*> rcXitions;
   result_channel rcXition, rcNumDetections, rcFC, rc3P1corr;

   int pol3P1; //polarization of 3P1 pulses
   FluorescenceChecker* fc;
   int probe_dir;

   std::vector<unsigned> pmt_array;
   std::vector<unsigned> pulse_type;

   unsigned numMg, numAl;
};

class exp_3P0_lock : public exp_3P0
{
public:
    exp_3P0_lock(list_t* exp_list, const std::string& name, unsigned num_freq=3);
   virtual ~exp_3P0_lock() {}

   //! Randomize probe order, and switch probe dir.
   virtual void reshuffle();

   virtual void run(const GbE_msg& msg_in, GbE_msg& msg_out);

protected:
   double getFreq(int i);
   double getGain(int i);

   rp_bool debug_clock, force1S0;
   rp_unsigned gainPoints, xtraPoints;
   rp_double lineWidths;

   unsigned nProbeFreqs;
   double freqScale;
   int lowIndex, centerIndex, highIndex;

   //! list of probe orders that keeps getting re-shuffled
   std::vector<unsigned> probe_order;

   std::vector<double> num_clock_transitions;

   result_channel rcErr, rcDirection;

   //! results channels have one entry per probe frequency
   std::vector<result_channel*> rcXitionProb, rcNumProbes, rcProbeFreq0, rcProbeFreq1;
};

#endif //EXP_AL3P0_H_
