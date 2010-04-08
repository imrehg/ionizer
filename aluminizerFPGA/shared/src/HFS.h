#ifdef HAS_HFS
#pragma once

#include <math.h>

#include "physics.h"
#include "Numerics.h"
#include "string_func.h"

namespace TNT
{
   template<class T> class Array2D;
   template<class T> class Array1D;
}

using namespace std;

namespace physics
{

/*
An HFS object represents one hyperfine state of the atom
It includes the electronic configuration and uses g factors
to calculate the level splitting in Hz/gauss
*/

class HFS
{
public:
   HFS(const string& element_name,
      double mass,
      double F,
      double I,
      double J,
      int L,
      double S,
      double gJ,
      double gF,
      double muI);

   virtual ~HFS() {}

//	numerics::fraction mFmin() const { return -F; }
//	numerics::fraction mFmax() const { return F; }

   double mFmin() const { return -F; }
   double mFmax() const { return F; }

   //return the number of mF states
   int NumStates() const { return (TwoF + 1); }

   //return a string describing the HFS, e.g. 3P0(F=5/2)
   const string& GetName() const { return name; }

   //return a string describing the HFS, e.g. 3P0(F=5/2 m=-3/2)
   string GetName(double mF) const { return name + " m=" + numerics::fraction_txt(mF,true); }
   string get_mF_string(double mF) const { return (TwoF % 2)?to_string<int>(2*mF)+"/2" : to_string<int>(mF); }

   //return the distance of sublevel mF from mF=0 in Hz for B gauss
   virtual double sublevel(double mF, double B);

   //print all sublevels to o
   void PrintSublevels(ostream& o, double MagneticField);

   //return whether or not the state is legal
   bool IsStateLegal(double mF) const { return ( fabs(mF) <= F ); }

public:

   double gF;				//g factor for linear Zeeman effect hyperfine splitting
   double gJ;
   double gI;

   string name;

   double mass;

public:
   const double F;			//total atomic angular momentum

protected:
   const double I;			//nuclear spin
   const double J;			//electronic angular momentum
   const double S;			//total electronic spin
   const OrbitalL L;			//orbital angular momentum

   double Bfield;		//in gauss

   const int TwoF;
   const int TwoI;
   const int TwoJ;
   const int TwoS;

   static const double BohrMagneton, NuclearMagneton, m_nuc;

   static const double Al27mass, Al27NuclearMagneticMoment;
   static const double Al27_1S0gF, Al27_3P0gF;
   static const double Al27_3P1gJ, Al27_3P1Ahfs, Al27_3P1Bhfs;

};

class HFS_BreitRabi : public HFS
{
public:
   HFS_BreitRabi(const HFS_BreitRabi&);

   HFS_BreitRabi(const HFS& hfs, double Ahfs, double Bhfs);

   virtual ~HFS_BreitRabi();

   //return Breit-Rabi level shift from Bz=0 in Hz
   virtual double sublevel(double mF, double B);


protected:
   // generate HFS Hamiltonian
   // Bz: Mag. field in G
   // I use |I J m_I m_J> basis
   void GenerateHFSHamiltonian(double B);

   //order eigenvalues (does only sorting now; could be more sophisticated)
   void SortEigenValues(TNT::Array1D<double>* ev, double B) const;

   unsigned mij2rc(double mi, double mj);

protected:

   const double Ahfs;		//hfs magnetic splitting constant in Hz
   const double Bhfs;		//hfs electronic qudarupole moment splitting constant in Hz

   TNT::Array1D<double>* EVhfs0;		  //zero field eigenvalues for HFS hamiltonian
   TNT::Array1D<double>* EVhfs;		  //eigenvalues for HFS hamiltonian for current Bfield
   TNT::Array2D<double>* HFSHamiltonian; //the HFS hamiltonian matrix
};

}

#endif //HAS_HFS
