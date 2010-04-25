#include "fractions.h"
#include "Numerics.h"
#include "physics.h"
#include "string_func.h"

#include <stdexcept>
#include <cstdio>

#ifdef WIN32
#define snprintf _snprintf
#define hypot _hypot
#define isnan _isnan
#endif


#include <float.h>
#include <math.h>
#include <sstream>

using namespace std;
using namespace physics;
using namespace numerics;


const double physics::hbar = 1.0546e-34;
const double physics::m_e = 9.1093826e-31;					// electron mass in kg
const double physics::gS =  2.0023193043718;				// electron g-factor




istream& physics::operator>>(istream& i, physics::polarization& p)
{
   int c;
        while (isspace(c = i.get()) && !i.eof()) ;

   if(!i.eof())
      i.putback(static_cast<char>(c));
   else
      return i;

   string s;
   i >> s;

   if(s == physics::polarization::SIGMA)
   {
      string sign;
      i >> sign;

      if(sign.find("-") != string::npos)
         p = physics::polarization(-1);
      else
         p = physics::polarization(+1);
   }
   else
   {
      if(s == physics::polarization::PI)
         p = physics::polarization(0);
      else
         throw runtime_error("Illegal polarization: " + s);
   }

   return i;
}

const string physics::polarization::SIGMA = "s";
const string physics::polarization::SIGMA_PLUS = "s+";
const string physics::polarization::SIGMA_MINUS = "s-";
const string physics::polarization::PI = "pi";

double physics::line::extract_mFg(const string& s)
{
   size_t loc = s.find("mFg = ");

   if(loc != string::npos)
   {
	  string s2 = s.substr(loc+6);
      double d = from_string<double>(s2);

	  if(s.find("/2") != string::npos)
		  return d/2;
	  else
		  return d;
   }
   else
      return 0;
}

double physics::line::extract_mFe(const string& s)
{
   double g = extract_mFg(s);

   if(s.find("s+") != string::npos)
      return g+1;

   if(s.find("s-") != string::npos)
      return g-1;

   return g;
}

int physics::line::extract_sb(const string& s)
{
   int sb = 0;

   if(s.find("SB") != string::npos)
   {
      size_t i = s.find("SB");
      sscanf(s.substr(i).c_str(), "SB%i", &sb);
   }
   else
   {
      if(s.find("COM") != string::npos)
         sb = 1;

      if(s.find("STR") != string::npos)
         sb = 2;
   }

   if(s.find("rsb") != string::npos)
      sb *= -1;

   return sb;

}
