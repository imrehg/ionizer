#ifndef EXP_CORRELATE_H_
#define EXP_CORRELATE_H_

#include "experiments.h"

#define N_BINS 16
#define N_BITS 16

#define PLOT_FFT 1

class exp_correlate : public exp_detect
{
public:
exp_correlate(list_t* exp_list, const std::string& name = "Correlate");

virtual ~exp_correlate()
{
}

virtual bool needInitDDS()
{
	return false;
}
virtual void getPlotData(unsigned iPlot, unsigned iStart, GbE_msg& msg_out);
virtual unsigned getNumPlots();

virtual void updateParams();

protected:
virtual void run_exp(int iExp);
virtual void init_exp(unsigned i);
virtual void post_exp_calc_results();

virtual unsigned getNumDetect() const
{
	return 1;
}

rp_bool removeFromQueue;
rp_unsigned clockDivider;
rp_double fftFrequency;

dds_params Heat;

unsigned histogram[N_BINS];
result_channel rcBin1FFT;
std::vector<result_channel*> rcBins;
};

#endif /*EXP_CORRELATE_H_*/
