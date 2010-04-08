#ifndef CALCLMS_H_
#define CALCLMS_H_

#define NUM_ACC 6
#define NUM_CHANNELS 8

typedef int X_t;
typedef int W_t;

int updateLMS(unsigned num_taps, W_t* W, X_t* X, int iX, int eki, unsigned right_shift);
int updateLMS_slow(unsigned M, W_t* W, X_t* X, int iX, int dk, int ek, unsigned right_shift1, unsigned right_shift2, bool bEnableUpdates);
int updateLMSpred_slow(unsigned M, W_t* W, X_t* X, int iX, unsigned right_shift1);
void updateLMStaps_slow(unsigned M, W_t* W, X_t* X, int iX, int dk, int ek, unsigned right_shift2);

#endif /*CALCLMS_H_*/
