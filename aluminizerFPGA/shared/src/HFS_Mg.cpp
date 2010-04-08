#ifdef HAS_HFS

#include "physics.h"
#include "HFS_Mg.h"

namespace physics
{

//TODO: there is a potential problem here if Mg25_mass is initialied before m_nuc
const double HFS_Mg25_II::Mg25_mass =  24.985837 * HFS::m_nuc;			// 25Mg mass
const double HFS_Mg25_II::Mg25_2S12Ahfs = -596254376 + 123;		// hfs dipole splitting constant in Hz
const double HFS_Mg25_II::Mg25_2S12Bhfs = 0;						// hfs quadrupole splitting constant in Hz
const double HFS_Mg25_II::Mg25_gJ		= 2.0022606;
const double HFS_Mg25_II::Mg25_gIprime	= 9.299484e-5 * HFS_Mg25_II::Mg25_gJ;
const double HFS_Mg25_II::Mg25_NuclearMagneticMoment = (HFS::BohrMagneton/HFS::NuclearMagneton) * HFS_Mg25_II::Mg25_gIprime;

} //namespace physics

#endif //HAS_HFS
