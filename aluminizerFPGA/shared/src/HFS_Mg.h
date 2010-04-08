#ifdef HAS_HFS

#include "physics.h"
#include "HFS.h"

namespace physics
{

class HFS_Mg25_II : public HFS_BreitRabi
{
public:
   HFS_Mg25_II(double F, double J, int L, double S) :
     HFS_BreitRabi(HFS("Mg25_II", Mg25_mass, F, 5/2., J, L, S, Mg25_gJ, 0, Mg25_NuclearMagneticMoment), Mg25_2S12Ahfs, Mg25_2S12Bhfs) {};

    static const double Mg25_mass, Mg25_2S12Ahfs, Mg25_2S12Bhfs, Mg25_gJ, Mg25_gIprime, Mg25_NuclearMagneticMoment;

};

class HFS_Mg25_II_S_One_Half : public HFS_Mg25_II
{
public:
   HFS_Mg25_II_S_One_Half(double F) : HFS_Mg25_II(F, 1/2., 0, 1/2.) {};
};

class HFS_Mg25_II_P_One_Half : public HFS_Mg25_II
{
public:
   HFS_Mg25_II_P_One_Half(double F) : HFS_Mg25_II(F, 1/2., 1, 1/2.) {};
};

class HFS_Mg25_II_P_Three_Halves : public HFS_Mg25_II
{
public:
   HFS_Mg25_II_P_Three_Halves(double F) : HFS_Mg25_II(F, 3/2., 1, 1/2.) {};
};

} //namespace physics

#endif //HAS_HFS
