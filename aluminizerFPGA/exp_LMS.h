#ifndef EXP_LMS_H_
#define EXP_LMS_H_

#include "experiments.h"
#include <valarray>

#include "calcLMS.h"

class exp_LMS : public experiment
{
public:
exp_LMS(list_t* exp_list, const std::string& name = "ReadADC");

virtual ~exp_LMS()
{
}

virtual unsigned remote_action(const char* s);
virtual bool needInitDDS()
{
	return false;
}
virtual void getPlotData(unsigned iPlot, unsigned iStart, GbE_msg& msg_out);
virtual unsigned getNumPlots();
virtual void setCoefficients(const GbE_msg& msg_in, GbE_msg& msg_out);

protected:
void resize_taps();
void clear_filter();
virtual void run_exp(int iExp);    //there will generally only be one experiment
void readADC(int* data);
virtual unsigned getNumDetect() const
{
	return 0;
}

rp_unsigned delay, num_taps, time_steps, right_shift1, right_shift2;
rp_int delta_f;
rp_bool enableLMS;
rp_int phaseMultiplier;
rp_int G;    // gain for loose lock to LO.
rp_unsigned update_period;

XSpi spi;
std::vector<result_channel*> rcADC;

int ring_index;

std::valarray<X_t> W;      //Wiener coefficients.
std::valarray<W_t> X;      //Measurements

int acc1[NUM_ACC];
int accHP1[NUM_ACC];

int phase[2];
int dk1;       //previous phase measurement (so we can unwrap 2 pi jumps)
int dkHP1;     //previous high-passed phase value for Butterworth filter
int phase_wraps;

int yk1;    // previous prediction

int iX;     //current write location of X ring buffer. Start at back.
int ek;
int T;
unsigned ftw0;
unsigned timer;

unsigned long long tLastUpdate;
};


#endif /*EXP_LMS_H_*/
