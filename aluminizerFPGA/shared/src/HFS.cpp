#ifndef NO_HFS

#include "fractions.h"
#include "physics.h"
#include "HFS.h"
#include "string_func.h"

#include <iomanip>

#include <tnt.h>
#include <jama_eig.h>

using namespace std;
using namespace TNT;
using namespace JAMA;
using namespace physics;
using namespace numerics;

const double HFS::m_nuc = 1.66053886e-27;				// nuclear mass (1 u)
const double HFS::BohrMagneton = 1.39962458e6;			// Hz per Gauss
const double HFS::NuclearMagneton = 7.62259371e2;		// Hz per Gauss

HFS::HFS(const string& element_name, double mass, double F, double I, double J,
      int L, double S, double gJ, double gF, double muI)  :
   gF(gF), gJ(gJ), gI(0), mass(mass), F(F), I(I), J(J), S(S), L(L), Bfield(0),
   TwoF(static_cast<int>(2*F)),
   TwoI(static_cast<int>(2*I)),
   TwoJ(static_cast<int>(2*J)),
   TwoS(static_cast<int>(2*S))
{
   //all spins need to be integer or half-integer
   assert(TwoF == floor(2*F));
   assert(TwoI == floor(2*I));
   assert(TwoJ == floor(2*J));
   assert(TwoS == floor(2*S));

   //electronic g-factor
   double gL = 1 - m_e/mass;

   //calculate gJ unless gJ is provided
   if(J != 0 && gJ == 0)
      this->gJ = gL * ( J*(J + 1) + L*(L + 1) - S*(S + 1) ) / ( 2*J*(J + 1) ) +
               gS * ( J*(J + 1) - L*(L + 1) + S*(S + 1) ) / ( 2*J*(J + 1) );

   //nuclear g-factor
   if(I != 0)
      this->gI = muI;


   //calculate gF unless gF is provided
   if ( F != 0 &&  gF == 0)
   {
      double g1 = this->gJ * ( ( F*(F+1) + J*(J + 1) - I*(I+1) ) / ( 2*F*(F + 1) ) );
      double g2 = ( ( F*(F+1) + I*(I + 1) - J*(J+1) ) / ( 2*F*(F + 1) ) );

      this->gF = g1 + this->gI * (NuclearMagneton / BohrMagneton) * g2;
   }

   ostringstream oss;
   oss << element_name << " " << (TwoS + 1) << this->L.symbol() << fraction_txt(J) << "(F=" << fraction_txt(F) << ")";

   name = oss.str();
}


// returns linear Magnetic field shift of specified level in Hz/G
double HFS::sublevel(double mF, double B)
{
   return mF * BohrMagneton * gF * B;
}

void HFS::PrintSublevels(ostream& o, double B)
{
   for(double mF = -F; mF <= F; ++mF)
   {
      o << GetName() << "   mF = " << setw(3) << mF << " :  ";
      o << setw(14) << to_string<double>(sublevel(mF, B), 3) << " Hz" << endl;
   }
}

HFS_BreitRabi::HFS_BreitRabi(const HFS& hfs, double Ahfs, double Bhfs) :
   HFS(hfs),
   Ahfs(Ahfs),
   Bhfs(Bhfs),

   EVhfs0(new Array1D<double>((TwoJ+1)*(TwoI+1), 0.)),
   EVhfs(new Array1D<double>((TwoJ+1)*(TwoI+1), 0.)),
   HFSHamiltonian(new Array2D<double>((TwoJ+1)*(TwoI+1), (TwoJ+1)*(TwoI+1), 0.))
{
   //zero field eigenvalues
   GenerateHFSHamiltonian(0.0);
   Eigenvalue<double> eig0(*HFSHamiltonian);
   eig0.getRealEigenvalues(*EVhfs0);
   SortEigenValues(EVhfs0, 0);
   *EVhfs = *EVhfs0;
}


HFS_BreitRabi::~HFS_BreitRabi()
{
   delete HFSHamiltonian;
   delete EVhfs0;
   delete EVhfs;
}

// generate HFS Hamiltonian in Hz
// Bz: Mag. field in G
// I use |I J m_I m_J> basis
void HFS_BreitRabi::GenerateHFSHamiltonian(double Bz)
{
   // electronic coupling constant
   double ecc=gJ*Bz*BohrMagneton;
   // nuclear coupling constant
   double ncc=gI*Bz*NuclearMagneton;
   // HFS coupling constant
   double IJshort=J*(J+1)+I*(I+1);
   double AHFScc=Ahfs/2;
   double AHFSdiagterm = AHFScc*IJshort;
   // QP HFS coupling constant
   double BHFScc = 0;
   if ( I>1/2. && J>1/2. ) // term vanishes for I<=1/2 or J<=1/2
      BHFScc=3/8.*Bhfs /( I*(2*I-1)*J*(2*J-1) );
   double BHFSdiagterm = -4/3.*BHFScc*( I*(I+1)*J*(J+1) );

   int nJ=TwoJ+1; // # of J components
   int nI=TwoI+1; // # of I components

   //zero out HFSHamiltonian
   for(int i = 0; i < HFSHamiltonian->dim1(); i++)
      for(int j = 0; j < HFSHamiltonian->dim2(); j++)
         (*HFSHamiltonian)[i][j] = 0;

   // setup diagonal (I_z, J_z, I^2, J^2) part
   // matrix index: ii+nI*jj
   for ( int mi=0; mi<nI; ++mi )
   {
      for ( int mj=0; mj<nJ; ++mj )
      {
         (*HFSHamiltonian)[mi+nI*mj][mi+nI*mj]=ecc*(mj-J)+ncc*(mi-I) - AHFSdiagterm - BHFSdiagterm;
      }
   }

   // setup off-diagonal (F^2) matrix elements
   double CGKtrans = 0;
   double K = 0;

   for ( int mi1=0; mi1<nI; ++mi1 )
   {
      for ( int mj1=0; mj1<nJ; ++mj1 )
      {
         for ( int mi2=0; mi2<nI; ++mi2 )
         {
            //speed up by only evaluating non-zero terms
            //CGKtrans is only non-zero if mi1 + mj1 == mi2 + mj2
            int mj2 = mi1 + mj1 - mi2;
            if( (mj2 >= 0) && (mj2 < nJ) )
            {
               //CGKtrans is only non-zero if mf == mi1 + mj1 - I - J
               double mf = mj1 + mi1 - I - J;

               for ( double F=std::max( abs(I-J), abs(mf) ); F<=I+J; ++F )
               {
                  CGKtrans = CGK(J, mj1-J, I, mi1-I, F, mf)*CGK(J, mj2-J, I, mi2-I, F, mf);
                  K = F*(F+1) - IJshort;
                  (*HFSHamiltonian)[mi1+nI*mj1][mi2+nI*mj2] += AHFScc*(F*(F+1))*CGKtrans + BHFScc * K*(K+1) * CGKtrans;
               }
            }
         }
      }
   }
}

//order eigenvalues (does only sorting now; could be more sophisticated)
void HFS_BreitRabi::SortEigenValues(Array1D<double>* ev, double /*B*/) const
{
   for ( int i = 0; i < ev->dim(); ++i )
      for ( int j = i; j < ev->dim(); ++j )
      {
//			if(B >= 0)
         {
            if ( (*ev)[j] > (*ev)[i] )
            {
               swap((*ev)[i], (*ev)[j]);
            }
         }
/*			else
         {
            if ( (*ev)[j] < (*ev)[i] )
            {
               swap((*ev)[i], (*ev)[j]);
            }
         }
*/
      }
}


// calculate level shift [Hz] in magnetic field [G]
// problem: eigenvalues are sorted by size, so if there is a level crossing
//			the state-labelling algorithm fails!
double HFS_BreitRabi::sublevel(double  mF, double B)
{
   if(J == 0)
      return HFS::sublevel(mF, B);

   //only recalculate if the magnetic field has changed
   if(B != Bfield)
   {
      Bfield = B;

      // get Bz=Magneticfield level energy
      GenerateHFSHamiltonian(B);
      Eigenvalue<double> eig(*HFSHamiltonian);

      eig.getRealEigenvalues(*EVhfs);
      SortEigenValues(EVhfs, B);
   }

   // find our F manifold
   int Findex = 0;

   // count all mF sublevels in the lower F manifolds
   for ( double iF=abs(I-J); iF<F; ++iF )
      Findex += static_cast<int>(2*iF+1);

   // now count inside the current F manfold
   int mFindex=(gF<0)?static_cast<int>(F+mF):static_cast<int>(F-mF);

   // A<0: F states are ordered with decreasing energy
   if ( Ahfs>0 )
      Findex = HFSHamiltonian->dim1() - (Findex+TwoF+1);

   int index = Findex + mFindex;

   return (*EVhfs)[index] - (*EVhfs0)[index];
}
#endif //NO_HFS
