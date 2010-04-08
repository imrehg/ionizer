#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "Numerics.h"

#include <string>

using namespace std;

namespace physics
{

extern const double hbar;
extern const double m_e;
extern const double gS;

// a class to convert between SPDF lettering and integers
class OrbitalL
{
public :
   OrbitalL(int iL) : L(std::min(iL, 6)) {}
//   OrbitalL(char cL) : L(string(spdf()).find(cL)) {}

   char spdf(int i) const { const char s[] = "SPDFGHI"; return s[std::min<int>(i,6)]; }

   operator int() const { return static_cast<int>(L); };
   char symbol() const { return spdf(L); }

protected:
   int L;
};

/* a class to convert between strings and integers for polarization */
class polarization
{
public:
   polarization() : p(0) {}
   polarization(int i) : p(i) {}
   operator const int& () const { return p; }
   polarization& operator=(int i) { p = i; return *this; }
//	operator int () const { return p; }

   const static string SIGMA;
   const static string SIGMA_PLUS;
   const static string SIGMA_MINUS;
   const static string PI;

private:

   int p;
};

ostream& operator<<(ostream& o, const polarization& p);
istream& operator>>(istream& i, polarization& p);

typedef int sideband;

/* a class to convert between strings and integers for sidebands */
/*
class sideband
{
public:
   sideband(int sb) : sb(sb) {}
//	operator int& () { return sb; }
   operator int () const { return sb; }
   int integer() const { return sb; } // TODO

private:
   int sb;
};
*/

//ostream& operator<<(ostream& o, const sideband& sb);
//istream& operator>>(istream& i, sideband& sb);

class line
{
public:
   line() : mFg(0), mFe(0), sb(0) {}

   line(const string& s) : mFg(extract_mFg(s)), mFe(extract_mFe(s)), sb(extract_sb(s))
   {}

   line(double mFg, double mFe, int sb=0, double angle=M_PI) :
      mFg(mFg), mFe(mFe), sb(sb), angle(angle), Ex(0), Ey(0) {}

   line(double mFg, polarization p, int sb=0, double angle=M_PI) :
      mFg(mFg), mFe(mFg + p), sb(sb), angle(angle), Ex(0), Ey(0) {}

   virtual ~line() {}

   int delta_m() const { return static_cast<int>(mFe - mFg); }
   polarization Polarization() const { return static_cast<int>(mFe - mFg); }

   double extract_mFg(const string& s);
   double extract_mFe(const string& s);
   int extract_sb(const string& s);

   void InvertAngularMomentum()
   {
      mFg = -mFg;
      mFe = -mFe;
   }

   double mFg;
   double mFe;

   sideband sb;

   double f;
   double t;

   double angle;

   double Ex, Ey;
};

}

