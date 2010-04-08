#pragma once

#ifdef HAS_HFS

#include <map>

#include "Numerics.h"
#include "physics.h"
#include "HFS.h"

using namespace std;

namespace physics
{

class ElectronicTransition
{
public:
   ElectronicTransition(HFS* g, HFS* e, const std::string& xname) :
     g(g), e(e), xname(xname), f0(0) {} ;
   virtual ~ElectronicTransition() { delete g; delete e;};

   class IllegalTransition
   {
   public:
      IllegalTransition(const string& name) : name(name) {};
      string name;
   };

   class IllegalPiTimeRatio
   {
   public:
      IllegalPiTimeRatio(const string& name1, const string& name2)
         : name1(name1), name2(name2) {};

      string name1;
      string name2;
   };

   //return a string representing the transition
   string TransitionName(const line& l, bool bShort=true) const;

   //return a string representing the transition
   string TransitionName() const;

   //calibrate B field and line center using two transition frequencies
   //accuracy in Hz
   //return B field.
   double CalibrateFrequency(const line& l1, const line& l2,
                       double f1, double f2, double accuracy=1e-3);

   //calibrate B field using using an absolute transition frequency
   //accuracy in Hz
   //return B field.
   double CalibrateFrequency(const line& l, double f, double f0, double accuracy=1e-3);

   //return the transition frequency given the magnetic field
   double Frequency(const line& l, double MagneticField);

   //return sublevel energies given B-field
   double g_sublevel(double mF, double B);
   double e_sublevel(double mF, double B);

   //return the ratio of Pi times between two transitions for the same
   //light intensity using Clebsch-Gordan coefficients
   virtual double PiTimeRatio(const line& l1, const line& l2);

   void SetPiTime(double t) { tPi = t; }
   double GetPiTime() const { return tPi; }

   //return the number of ground states
   int NumGroundSates() const { return g->NumStates(); }

   bool IsTransitionLegal(double mFg, double mFe) const;
   bool IsTransitionLegal(const line& l) const;

   double GetF0() const { return f0; }
   void SetF0(double f) { f0 = f; }

   line stretchedPlus(int sb=0) const;
   line stretchedMinus(int sb=0) const;

   HFS* g;
   HFS* e;

protected:
   std::string xname;
   double f0;	//line center
   double tPi; //carrier pi time (for now let the user decide for which transition)
};

class MotionalTransition : public ElectronicTransition
{
public:
   MotionalTransition(HFS* g, HFS* e, const std::string& xname, int num_axial_modes)
      : ElectronicTransition(g, e, xname), num_axial_modes(num_axial_modes)
   {
      modes["COM"] = NormalMode(0);
   }

/*	MotionalTransition(auto_ptr<HFS> g, auto_ptr<HFS> e, double COMFrequency, double StretchFrequency)
      : ElectronicTransition(g, e)
   {
      modes["COM"]	 = NormalMode(COMFrequency);
      modes["Stretch"] = NormalMode(StretchFrequency);
   }
*/
   virtual ~MotionalTransition() {};

   //return the ratio of Pi times between two transitions for the same
   //light intensity using Clebsch-Gordan coefficients and Lamb-Dicke params
   virtual double PiTimeRatio(const line& l1, const line& l2);

   //return sublevel energies given B-field, motional state
   double g_sublevel(double mF, double B, int sb);
   double e_sublevel(double mF, double B, int sb);

   //return a string representing the transition
   string TransitionName(int sideband) const;
   string TransitionName(const line& l, bool bShort=true) const;

   void SetModeFrequency(double f, int mode);
   void SetModeLambDicke(double eta, int mode);

   double getModeLambDicke(int mode);

   //calibrate trap frequency
   double CalibrateModeFrequency(double fBSB, double fRSB, const string& mode = "COM");

   //return the transition frequency given the magnetic field (B) and delta_n
   double Frequency(const line& l, double B);
   double Frequency(int delta_n);

//	double CalibratePiTimeRatio(const string& mode, double t);
   double GetPiTime(const string& mode);

protected:


   class NormalMode
   {
   public:
      NormalMode() : Frequency(0), Eta(0) { }
      NormalMode(double Frequency) : Frequency(Frequency) {}

      double Frequency;
      double Eta;  //Lamb-Dicke parameter
   };

   map<string, NormalMode> modes;

   int num_axial_modes;
};

ostream& operator<<(ostream& o, const ElectronicTransition::IllegalTransition& i);
ostream& operator<<(ostream& o, const ElectronicTransition::IllegalPiTimeRatio& i);

} //namespace physics

#endif //HAS_HFS
