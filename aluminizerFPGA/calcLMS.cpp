#include "calcLMS.h"
#include "Cordic.h"

int updateLMS(unsigned M, W_t* W, X_t* X, int iXi, int eki, unsigned right_shift1, unsigned right_shift2)
{
	register int yk = 0;
	register int ek1 = eki; //previous ek value, e_{k-1}

	register int i = M;     //start at the back
	register int iX = iXi;  //iX is index of current sample.  (iX+1)%M is previous sample

	register X_t xki = 0;   //x^{k-i]
	register W_t wk;
	register X_t xki1;      //x^{k-1-i]

	//Combine the filter coefficient update loop with the
	//correction (y_k) calculation loop for better performance.
	//This could be made almost 2 x faster by loading two 16-bit W and X values at once as 32-bit integers,
	//and then using mulhhw and maclhw for the high word calc.
	while (--i >= 0) //loop M_2 times
	{
		//When iX reaches front of X ring buffer, wrap around.
		if (--iX < 0)
			iX = M - 1;

		xki1 = X[iX]; //x^{k-1-i]

		//update filter coefficients.
		//w^k_i = w^{k-1}_i + 2*mu*e_{k-1}*x_{k-1-i}
		wk = W[i] + ((ek1 * xki1) >> right_shift2);
		W[i] = wk;

		//y_k += w^k * x_{k-i};
		//inline-assembly because the C++ compiler (gcc) doesn't seem to know this instruction on the PPC405
		//maclhws = Multiply Accumulate Low Halfword to Word Saturate Signed
#ifdef ASSEMBLY_MAC
		__asm__("maclhws %0,%1,%2" : "=r" (yk) :  "r" (wk), "r" (xki));
#else
		yk += wk * xki;
#endif

		xki = xki1;
	}

	//y_k += w^k * x_{k-i}; for current data (i=0)
#ifdef ASSEMBLY_MAC
	__asm__("maclhws %0,%1,%2" : "=r" (yk) :  "r" (wk), "r" (xki));
#else
	yk += wk * xki;
#endif

	//new yk
	return yk;
}

int updateLMS_slow(unsigned M, W_t* W, X_t* X, int iX, int dk, int ek, unsigned right_shift1, unsigned right_shift2, bool bEnableUpdates)
{
	//get new yk
	int yk = 0;
	unsigned iW = 0;

	//do this in two parts.
	//first part of X ring buffer
	while (iX < M)
	{
		register int wk = W[iW++];
		yk += (wk * (int)X[iX++]) >> right_shift1;
	}

	//second part of X ring buffer
	iX = 0;
	while (iW < M)
	{
		register int wk = W[iW++];
		yk += (wk * (int)X[iX++]) >> right_shift1;
	}

	//iX should be where it started now

	//int ek = dk - yk;

	if (bEnableUpdates)
	{
		int sumW = 0;

		//update W.
		//run through W & X in two parts as before

		//first part of X ring buffer
		iW = 0;
		while (iX < M)
		{
			W[iW] += (ek * (int)X[iX++]) >> right_shift2;
			sumW += W[iW];
			iW++;
		}

		//second part of X ring buffer
		iX = 0;
		while (iW < M)
		{
			W[iW] += (ek * (int)X[iX++]) >> right_shift2;
			sumW += W[iW];
			iW++;
		}

		//force the filter to be zero-mean (DC insensitive)
/*		sumW /= M;

      for(iW = 0; iW<M; iW++)
         W[iW] -= sumW; */
	}

	return yk;
}

int updateLMSpred_slow(unsigned M, W_t* W, X_t* X, int iX, unsigned right_shift1)
{
	//get new yk
	int yk = 0;
	unsigned iW = 0;

	//do this in two parts.
	//first part of X ring buffer
	while (iX < M)
	{
		register int wk = W[iW++];
		yk += (wk * (int)X[iX++]) >> right_shift1;
	}

	//second part of X ring buffer
	iX = 0;
	while (iW < M)
	{
		register int wk = W[iW++];
		yk += (wk * (int)X[iX++]) >> right_shift1;
	}

	//iX should be where it started now

	return yk;
}

void updateLMStaps_slow(unsigned M, W_t* W, X_t* X, int iX, int dk, int ek, unsigned right_shift2)
{
	unsigned iW = 0;

	//update W.
	//run through W & X in two parts as before

	//first part of X ring buffer
	iW = 0;
	while (iX < M)
	{
		W[iW] += (ek * (int)X[iX++]) >> right_shift2;
		iW++;
	}

	//second part of X ring buffer
	iX = 0;
	while (iW < M)
	{
		W[iW] += (ek * (int)X[iX++]) >> right_shift2;
		iW++;
	}
}
