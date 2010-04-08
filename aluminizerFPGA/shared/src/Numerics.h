#pragma once

#define _USE_MATH_DEFINES

#include <string>
#include <math.h>

//numeric functions for the Aluminizer program

namespace numerics
{

double omega(double f);
double RabiProbability(double omega, double t, double omegaRabi, double omega0);
double RabiFloppingProbability(double t, double omegaR, double phase, double asym, double tau);

//see Experimental Issues, Eq. 18, section 2.3.1
double motional_rabi_factor(int n1, int n2, double z0, double k);

//parameters as in RabiProbability, but with Ramsey evolution time T
double RamseyProbability(double omega, double t, double omegaRabi, double omega0, double T);

unsigned RandomEvent(double probability);

double PoissonProb(double mean, unsigned n);
double LogPoissonProb(double mean, unsigned n);

//generate a poissonian random variable with given mean 
unsigned RandomVariablePoisson(double mean);

//write a simple fraction in d as a signed fraction
std::string fraction_txt(double d, bool showPlus=false, int max_den=10);

double factorial(unsigned n); 
double logfactorial(unsigned n);

//evaluates the (n,k)th Laguerre polynomial at x
double Laguerre(unsigned int n, unsigned int k, double x);

bool IsHalfInteger(double d);

//! accurate to 2e-6 for all angles
int fast_atan2(short x, short y);

} // namespace numerics


