#ifdef HAS_HFS

#include "fractions.h"
#include "Numerics.h"
#include "physics.h"
//#include "HFS.h"
#include "Transition.h"
#include "string_func.h"

#ifdef WIN32
#define snprintf _snprintf
#define hypot _hypot
#define isnan _isnan
#endif


#include <float.h>
#include <math.h>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

using namespace std;
using namespace physics;
using namespace numerics;



string ElectronicTransition::TransitionName(const line& l, bool bShort) const
{
   if( abs(l.delta_m()) > 1 )
      throw IllegalTransition("abs(mFe - mFg) > 1 " + TransitionName());

   ostringstream oss;

   switch(l.delta_m()) {
      case +1 : oss << "s+ "; break;
      case -1 : oss << "s- "; break;
      case  0 : oss << "pi "; break;
   }

   oss << "mFg = ";

   oss << fraction_txt(l.mFg, true);

   if(!bShort)
      oss << " " << TransitionName();

   return oss.str();
}

string ElectronicTransition::TransitionName() const
{
   return g->GetName() + " --> " + e->GetName();
}

//calibrate B field using an absolute transition frequencies
//return B field.  update the transitions map
//accuracy in Hz
double ElectronicTransition::CalibrateFrequency(const line& l, double f, double f0, double accuracy)
{
   this->f0 = f0;

   // get inital guess for magnetic field
   double FoverB = e->sublevel(l.mFe, 0.5) - g->sublevel(l.mFg, 0.5);

   //solve for foverB * B + f0 = f
   double B = FoverB != 0 ? (f-f0) / FoverB : 10;

   // get solution for nonlinear Zeeman effect using crude bisectioning
   double B_lo = B*0.9;
   double B_hi = B*1.1;
   double deltaF = Frequency(l, B) - f;

   int max_steps = 8;
   int n_steps = 0;

   while ( ::fabs(deltaF) >= accuracy && (n_steps <= max_steps) )
   {
      n_steps++;

      FoverB = ( Frequency(l, B_hi) - Frequency(l, B_lo) ) / (B_hi - B_lo);
      deltaF = Frequency(l, B) - f;

      double deltaB = deltaF/FoverB;
      B -= deltaB;

      B_hi = B + fabs(deltaB);
      B_lo = B - fabs(deltaB);
   }

   if(::fabs(deltaF) >= accuracy)
      throw runtime_error("Unable to calculate B after " + to_string<int>(n_steps) + " steps.  Accuracy = " + to_string<double>(::fabs(deltaF)) + " Hz.");

   return B;
}

//calibrate B field and line center using two transition frequencies
//return B field.  update the transitions map
//accuracy in Hz
double ElectronicTransition::CalibrateFrequency(const line& l1, const line& l2,
                                    double f1, double f2, double accuracy)
{
   if(f1 == f2)
      return 0;

   // get inital guess for magnetic field
   double f1overB = e->sublevel(l1.mFe, 1) - g->sublevel(l1.mFg, 1);
   double f2overB = e->sublevel(l2.mFe, 1) - g->sublevel(l2.mFg, 1);

   //solve for f1overB * B + f0 = f1
   //			f2overB * B + f0 = f2
   double B = (f1 - f2) / (f1overB - f2overB);

   // get solution for nonlinear Zeeman effect using crude bisectioning
   double B_lo = B*0.9;
   double B_hi = B*1.1;
   double deltaF = 0;

   int max_steps = 8;
   int n_steps = 0;

   do
   {
      n_steps++;

      f1overB = ( Frequency(l1, B_hi) - Frequency(l1, B_lo) ) / (B_hi - B_lo);
      f2overB = ( Frequency(l2, B_hi) - Frequency(l2, B_lo) ) / (B_hi - B_lo);

      double slope = f1overB - f2overB;

      deltaF = Frequency(l1, B) - Frequency(l2, B) - (f1 -  f2);
      double deltaB = deltaF/slope;
      B -= deltaB;

      B_hi = B + fabs(deltaB);
      B_lo = B - fabs(deltaB);

      if(isnan(B_hi) || isnan(B_lo))
         break;

   } while ( ::fabs(deltaF) >= accuracy && (n_steps <= max_steps) );

   if(::fabs(deltaF) >= accuracy)
      throw runtime_error("Unable to calculate B after " + to_string<int>(n_steps) + " steps.  Accuracy = " + to_string<double>(::fabs(deltaF)) + " Hz.");

   f0 += f1 - Frequency(l1, B);

   return B;
}

//return the transition frequency given the magnetic field
double ElectronicTransition::Frequency(const line& l, double B)
{
   return e_sublevel(l.mFe, B) - g_sublevel(l.mFg, B);
}

//return the ground-state sublevel energy given B field [Hz]
double ElectronicTransition::g_sublevel(double mF, double B)
{
   return g->sublevel(mF, B);
}

//return the excited-state sublevel energy given B field [Hz]
double ElectronicTransition::e_sublevel(double mF, double B)
{
   return f0 + e->sublevel(mF, B);
}

bool ElectronicTransition::IsTransitionLegal(double mFg, double mFe) const
{
   //check range
   if( ! ( g->IsStateLegal(mFg) && e->IsStateLegal(mFe) ) )
      return false;

   double delta_m = mFe - mFg;
   return fabs(delta_m) <= 1;
}

bool ElectronicTransition::IsTransitionLegal(const line& l) const
{
   return IsTransitionLegal(l.mFg, l.mFe);
}

//return the ratio of Pi times between two transitions for the same
//light intensity using Clebsch-Gordan coefficients. Returnxs pi-time ratio t(l1) / t(l2)
double ElectronicTransition::PiTimeRatio(const line& l1, const line& l2)
{
   double delta_m1 = l1.mFe - l1.mFg;
   double delta_m2 = l2.mFe - l2.mFg;

   if( !IsTransitionLegal(l1) )
      throw IllegalTransition(TransitionName(l1));

   if( !IsTransitionLegal(l2) )
      throw IllegalTransition(TransitionName(l2));

   if( fabs(delta_m2) !=  fabs(delta_m1) )
      throw IllegalPiTimeRatio(TransitionName(l1), TransitionName(l2));

   return CGK(g->F, l2.mFg, 1, delta_m2, e->F, l2.mFe) /
         CGK(g->F, l1.mFg, 1, delta_m1, e->F, l1.mFe);
}

line ElectronicTransition::stretchedPlus(int sb) const
{
   return line(g->mFmax(), e->mFmax(), sb);
}

line ElectronicTransition::stretchedMinus(int sb) const
{
   return line(g->mFmin(), e->mFmin(), sb);
}


//return a string representing the transition
string MotionalTransition::TransitionName(int sideband) const
{
   return xname + " " + sbName(sideband) + ElectronicTransition::TransitionName() ;
}

//return a string representing the transition
string MotionalTransition::TransitionName(const line& l, bool bShort) const
{
   ostringstream oss;
    oss << xname << " ";
    oss << sbName(l.sb) << " ";
   oss << ElectronicTransition::TransitionName(l, bShort);

   return oss.str();
}

//calibrate trap frequency
double MotionalTransition::CalibrateModeFrequency(double fBSB, double fRSB, const string& mode)
{
   return modes[mode].Frequency = (fBSB - fRSB) / 2;
}

void MotionalTransition::SetModeFrequency(double f, int mode)
{
   int bsb = abs(mode);
   modes[modeName(bsb)].Frequency = f;
}

void MotionalTransition::SetModeLambDicke(double eta, int mode)
{
   modes[modeName(abs(mode))].Eta = eta;
}

double MotionalTransition::getModeLambDicke(int mode)
{
   return modes[modeName(abs(mode))].Eta;
}

//return the ground-state sublevel energy given B field [Hz]
double MotionalTransition::g_sublevel(double mF, double B, int sb)
{
   return ElectronicTransition::g_sublevel(mF, B) + Frequency(sb);
}

//return the excited-state sublevel energy given B field [Hz]
double MotionalTransition::e_sublevel(double mF, double B, int sb)
{
   return ElectronicTransition::e_sublevel(mF, B) + Frequency(sb);
}


//return the transition frequency given the magnetic field and delta_n
double MotionalTransition::Frequency(const line& l, double B)
{
   return Frequency(l.sb) + ElectronicTransition::Frequency(l, B);
}

//return the motional frequency given  delta_n
double MotionalTransition::Frequency(int delta_n)
{
   return (delta_n > 0 ? 1 : -1) * modes[modeName(delta_n)].Frequency;
}

/*
double MotionalTransition::CalibratePiTimeRatio(const string& mode, double t)
{
   return modes[mode].Eta = t / tPi;
}
*/




double MotionalTransition::GetPiTime(const string& mode)
{
   return exp(-0.5 * pow(modes[mode].Eta,2)) * modes[mode].Eta * tPi;
}

double MotionalTransition::PiTimeRatio(const line& l1, const line& l2)
{
   double cg12 = ElectronicTransition::PiTimeRatio(l1, l2);

   double m1 = 1;
   double m2 = 1;

   if(l1.sb != 0)
   {
      int n=0;
      double eta = getModeLambDicke(l1.sb);
      m1 = exp(-0.5*eta*eta) * sqrt(factorial(n)/ factorial(n+1)) * eta * fabs(Laguerre(n, 1, eta*eta));
   }

   if(l2.sb != 0)
   {
      int n=0;
      double eta = getModeLambDicke(l2.sb);
      m2 = exp(-0.5*eta*eta) * sqrt(factorial(n)/ factorial(n+1)) * eta * fabs(Laguerre(n, 1, eta*eta));
   }

   return  cg12 * m2 / m1;
}




ostream& physics::operator<<(ostream& o, const physics::ElectronicTransition::IllegalTransition& i)
{
   return o << "Illegal transition \"" << i.name << "\"." << endl;
}

ostream& physics::operator<<(ostream& o, const physics::ElectronicTransition::IllegalPiTimeRatio& i)
{
   o << "Illegal pi-time ratio:" << endl;
   o << "\t" << i.name1 << endl;
   o << "\t" << i.name2 << endl;

   return o;
}


#endif //HAS_HFS
