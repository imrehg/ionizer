#ifndef NUMERICS_H_
#define NUMERICS_H_

#define _USE_MATH_DEFINES
#include <math.h>

extern const double amu;
extern const double hbar;
extern const double m_e;
extern const double m_nuc;
extern const double gS;
extern const double BohrMagneton;
extern const double NuclearMagneton;

int sign(int i);
double motional_rabi_factor(int n1, int n2, double z0, double k);
double factorial(int n);
double mass(unsigned n);
double ModeAmplitude(double fZ);
double PoissonProb(double mean, unsigned n);

//evaluates the (n,k)th Laguerre polynomial at x
double Laguerre(unsigned int n, unsigned int k, double x);

double omega(double f);
double RabiProbability(double omega, double t, double omegaRabi, double omega0);

//generate a poissonian random variable with given mean
unsigned RandomVariablePoisson(double mean);

#endif /*NUMERICS_H_*/
