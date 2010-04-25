#ifdef WIN32
   #define _USE_MATH_DEFINES
   #define _snprintf snprintf
#endif

#include <math.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>

#include "Numerics.h"

//numeric functions for the Aluminizer program

using namespace std;

namespace numerics
{

double omega(double f)
{
   return 2 * M_PI * f;
}

double RabiProbability(double omega, double t, double omegaRabi, double omega0)
{
   double omegaRabiSquared = omegaRabi * omegaRabi;
   double detuning = omega - omega0;
   double omegaPrimeSquared = detuning*detuning + omegaRabiSquared;
   double sinTerm = sin(sqrt(omegaPrimeSquared) * t / 2);

   return (omegaRabiSquared / omegaPrimeSquared) * sinTerm * sinTerm;
}

double RamseyProbability(double omega, double t, double omegaRabi, double omega0, double T)
{
   double deltaOmega = omega - omega0;
   double omegaPrime = sqrt( deltaOmega * deltaOmega + omegaRabi * omegaRabi );
   double ramseyTerm = pow( cos( deltaOmega * T / 2 ) - (deltaOmega / omegaPrime) * sin( deltaOmega * T / 2 ) , 2);

   return RabiProbability(omega, t, omegaRabi, omega0) * ramseyTerm;
}

double RabiFloppingProbability(double t, double omegaR, double phase, double asym, double tau)
{
   return (0.5 + ( pow( sin( fabs(omegaR) * t / 2 + phase / 2), 2 ) + asym - 0.5) * exp( - t / tau ) );
}

unsigned RandomEvent(double probability)
{
   return rand() > (probability * RAND_MAX) ? 0 : 1;
}

double PoissonProb(double mean, unsigned n)
{
   return exp(-1*mean) * pow(mean, (double)n) / factorial(n);
}

double LogPoissonProb(double mean, unsigned n)
{
   return -1*mean + n*log(mean) - logfactorial(n);
}

//generate a poissonian random variable with given mean
unsigned RandomVariablePoisson(double mean)
{
   double product = 1;
   double minimum_product = exp(-mean);

   unsigned n = 0;

   do
   {
      product *= rand();
      product /= RAND_MAX;

      n++;
   } while(product >= minimum_product);

   return n-1;
}

std::string fraction_txt(double d, bool bShowPlus, int max_den)
{
   int num = 0;
   int den;

   for(den=1; den<max_den; den++)
   {
      num = static_cast<int>(floor(den*d));
      if(num == den*d)
         break;
   }

   char buff1[100];

   char plusStr[] = "+";
   if(!bShowPlus)
      plusStr[0] = 0;

   if(num > 0)
      sprintf(buff1, "%s%d", plusStr, num);
   else
      sprintf(buff1, "%d", num);

   char buff2[100];
   if(den > 1)
      sprintf(buff2, "%s/%d", buff1, den);
   else
      sprintf(buff2, "%s", buff1);

   return string(buff2);
}


//evaluates the (n,k)th Laguerre polynomial at x
double Laguerre(unsigned int n, unsigned int k, double x)
{
   double minusXtotheM=1;
   double L=0;
   unsigned int m=0;


   for(m = 0; m <= n; m++)
   {
      L += minusXtotheM/(factorial(n-m)*factorial(k+m)*factorial(m));
      minusXtotheM *= -x;
   }

   L *= factorial(n+k);
   return L;
}

//copied from old numerics.cpp

//#include "numerics.h"
//#include <stdlib.h>

const double amu = 1.66053886e-27;
const double hbar = 1.0546e-34;
//const double m_e = 9.1093826e-31;					// electron mass in kg
//const double m_nuc = 1.66053886e-27;				// nuclear mass (1 u)
//const double gS =  2.0023193043718;				// electron g-factor
//const double BohrMagneton = 1.39962458e6;			// Hz per Gauss
//const double NuclearMagneton = 7.62259371e2;		// Hz per Gauss

int sign(int i)
{
   return i > 0 ? 1 : -1;
}

double factorial(unsigned n)
{
   double f=1;
   for(;n>1;n--) f*=n;
   return f;
}
 
double logfactorial(unsigned n)
{
   double f=1;
   for(;n>1;n--) f += log((double)n);
   return f;
}

double mass(unsigned n)
{
   return n*amu;
}

double ModeAmplitude(double fZ)
{
   double omegaZ = fZ * 2 * M_PI;
   return sqrt(hbar / (2 * mass(25) * omegaZ) );
}


double motional_rabi_factor(int n1, int n2, double z0, double k)
{
   //see Experimental Issues, Eq. 18, section 2.3.1
   double eta = z0*k;
   return exp(-0.5*eta*eta) * sqrt(factorial(n1)/ factorial(n2)) * pow(eta,(int)(n2-n1)) * fabs(Laguerre(n1, n2-n1, eta*eta));
}

} // namespace numerics

