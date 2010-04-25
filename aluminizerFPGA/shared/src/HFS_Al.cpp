#ifndef NO_HFS
#include "physics.h"
#include "HFS_Al.h"

using namespace numerics;

namespace physics
{

const double HFS::Al27mass					   = 26.98153844 * 1.66053886e-27;		// 27Al mass
const double HFS::Al27NuclearMagneticMoment	= 3.6415;					// in nuclear magnetons

const double HFS::Al27_1S0gF		= -7.925e-4;						    //calibrated 9/2006
const double HFS::Al27_3P0gF		= -7.925e-4 - 8288.7/(5*1.39962458e6); //calibrated 9/2006


//const double Al27_3P1gJ		= 1.50115;						// 3P1 gJ factor

// with this g-factor, the 3P1/3P0 Zeeman splitting ratio is 506.8,
// which is the ratio we observe when running the clock
const double HFS::Al27_3P1gJ		= 1.50115;
const double HFS::Al27_3P1Ahfs	= 1339.971e6;					// hfs dipole splitting constant in Hz
const double HFS::Al27_3P1Bhfs	= -15.971e6;					// hfs quadrupole splitting constant in Hz


HFS_Al_II_SingletS0::HFS_Al_II_SingletS0() :
   HFS("Al_II",
      Al27mass,
      5/2.,	// F
      5/2.,	// I
      0,				// J
      0,				// L
      0,				// S
      0,				// gJ
      Al27_1S0gF,		// gF
      0)				// muI
{}

HFS_Al_II_TripletP0::HFS_Al_II_TripletP0() :
   HFS("Al_II",
      Al27mass,
      5/2.,	// F
      5/2.,	// I
      0,				// J
      1,				// L
      1,				// S
      0,
      HFS::Al27_3P0gF,
      0)
{}


HFS_Al_II_TripletP1::HFS_Al_II_TripletP1(double F) :
   HFS_BreitRabi(HFS("Al_II",
                  Al27mass,
                  F,
                  5/2.,
                  1,
                  1,
                  1,
                  Al27_3P1gJ,
                  0,
                  Al27NuclearMagneticMoment),

               Al27_3P1Ahfs,
               Al27_3P1Bhfs)
{}


}

#endif //NO_HFS
