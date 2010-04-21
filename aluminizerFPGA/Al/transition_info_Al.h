#ifndef TRANSITION_INFO_AL_H
#define TRANSITION_INFO_AL_H

#include "../transition_info.h"
#include "../detection_stats.h"

#include "../FluorescenceChecker.h"

class iAl3P1 : public transition_info
{
public:
   iAl3P1(list_t* exp_list, const std::string& name);
   virtual  ~iAl3P1() {}

   virtual dds_params* new_pulse(const pulse_spec& p, params_t* pparams);

   //Al3P1_pulse* getPulse(int mFg, int pol);
   void prepState(int mFg_target);

   //Al3P1_pulse& getSB(int sb, int pol); // positive for BSB, negative for RSB, 0 for carrier

   rp_matrix motion;

   rp_double waveplate_sp, waveplate_sm;
   //Al3P1_pulse Al3P1BSB1_sp, Al3P1BSB1_sm, Al3P1BSB2_sp, Al3P1BSB2_sm;
   std::vector<Al3P1_pulse*> Al3P1_sp_pulses;
   std::vector<Al3P1_pulse*> Al3P1_sm_pulses;

   int mFg_current;
};

class FluorescenceChecker;

class iAl3P0 : public transition_info
{
public:
   iAl3P0(list_t* exp_list, const std::string& name);
   virtual  ~iAl3P0();

   virtual void setIonXtal(const char* name);

   virtual dds_params* new_pulse(const pulse_spec& p, params_t* pparams);

   FluorescenceChecker* getFC();

   //return current Al clock state as string in msg_out
   //e.g. 1S0 mF=5/2
   //or   3P0 mF=-5/2
   void getClockState(const GbE_msg& msg_in, GbE_msg& msg_out);
   double getClockState();

     void setClockState(double d);

   unsigned getNum3P0states();
   virtual unsigned getNumPlots();

   //! Return data for plots on a GUI page.  The format is msg_out.m[0] = num_points, msg_out.m[1] = data[0], ...
   virtual void getPlotData(unsigned iPlot, unsigned iStart, GbE_msg& msg_out);

   void resetStats();
   void resetStats(const GbE_msg& msg_in, GbE_msg& msg_out);

   //! initialize detection statistics (allocate histograms, etc.)
   void initStats();

   //! return probability for "n" counts in histogram for n3P0 excitations, and mF2 Zeeman state
   double getProb(unsigned n3P0, int mF2, unsigned pulse_type, unsigned n);

   //! return mean value of a histogram
   double getMean(unsigned n3P0, int mF2, unsigned pulse_type);

   //! return name value of a histogram
   const std::string& getHistName(unsigned n3P0, int mF2, unsigned pulse_type);
   void getHistName(const GbE_msg& msg_in, GbE_msg& msg_out);

   //! record observation of "n" PMT counts in histogram
   void updateHist(unsigned n3P0, int mF2, unsigned pulse_type, unsigned n);

   //! reduce impact of older data
   void updateHistMemory(unsigned n3P0, int mF2, unsigned pulse_type, double det_memory);

   Al3P0_pulse* getSB(int mFg, int sb, int pol);

protected:
   unsigned getHistIndex(unsigned n3P0, int mF2, unsigned pulse_type);
   unsigned numAl, numMg; //number of Al+ and Mg+ ions
   unsigned num3P1Pulses; //number of different types of 3P1 detection pulses
   unsigned num3P0states;

   std::vector<Al3P0_pulse*> Al3P0_pi_pulses;

   vector<detection_stats> det_stats;
   vector<rp_double*> det_means;

   /* clock state: number of 3P0 excitations.  negative means indeterminate*/
   double clock_state;

   rp_unsigned check_interval;
   rp_double dark_rate, bright_rate, min_ok_prob;
   FluorescenceChecker* fc; //monitors fluorescence during clock pulse
};


#endif // TRANSITION_INFO_AL_H
