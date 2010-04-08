#pragma once

namespace physics
{

const double Be9mass =  9.0121821 * m_nuc;			// 9Be mass
const double Be92S12Ahfs = -625008837.048;			// hfs dipole splitting constant in Hz
const double Be92S12Bhfs = 0;						// hfs quadrupole splitting constant in Hz
const double Be9gJ			= 2.00226206;
const double Be9gIprime		= 2.134779853e-4 * Be9gJ;
const double Be9NuclearMagneticMoment = +1.1772645359;//(BohrMagneton/NuclearMagneton) * Be9gI / 1.5; // old: -1.1776;	// in nuclear magnetons

class HFS_Be_II : public HFS_BreitRabi
{
public:
	HFS_Be_II(double F, double J, int L, double S) : 
	  HFS_BreitRabi(HFS("Be_II", Be9mass, F, 3/2., J, L, S, Be9gJ, 0, Be9NuclearMagneticMoment), Be92S12Ahfs, Be92S12Bhfs) {};
};

class HFS_Be_II_S_One_Half : public HFS_Be_II
{
public:
	HFS_Be_II_S_One_Half(double F) : HFS_Be_II(F, 1/2., 0, 1/2.) {};
};

class HFS_Be_II_P_One_Half : public HFS_Be_II
{
public:
	HFS_Be_II_P_One_Half(double F) : HFS_Be_II(F, 1/2., 1, 1/2.) {};
};

class HFS_Be_II_P_Three_Halves : public HFS_Be_II
{
public:
	HFS_Be_II_P_Three_Halves(double F) : HFS_Be_II(F, 3/2., 1, 1/2.) {};
};

} //namespace physics

