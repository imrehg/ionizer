#include <string>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <time.h>
#include <cstdlib>
#include <cstdio>

#include "string_func.h"

using namespace std;

const char* polName(int p)
{
   switch(p)
   {
    case -1 : return "s-";
    case  0 : return "pi";
    case  1 : return "s+";
   }

   throw runtime_error("unknown polarization");
}

const char* modeName(int i)
{
   switch(abs(i))
   {
      case  0 : return "carrier";
      case  1 : return "COM";
      case  2 : return "STR";
      case  3 : return "SB3";
      case  4 : return "SB4";
      case  5 : return "SB5";
      case  6 : return "SB6";
      case  7 : return "SB7";
      case  8 : return "SB8";
      case  9 : return "SB9";
   }

   throw runtime_error("no mode name");
}

string sbName(int i)
{
   if(i < 0)
      return string("rsb ") + string(modeName(i));

   if(i > 0)
      return string("bsb ") + string(modeName(i));

   return  string(modeName(i));
}

#ifdef WIN32
#define snprintf _snprintf
#endif


std::string pulse_name(const std::string& base_name, int mFg2, int pol, int sb)
{
   char sp[] = "+";
   if(mFg2 < 0)
      sp[0] = 0;


   char sHalf[] = "/2";
   if(mFg2 % 2 == 0)
   {
      mFg2 /= 2;
      sHalf[0] = 0;
   }

   char sbuff[64];
   snprintf(sbuff, 63, "%s %s %s mFg = %s%d%s", base_name.c_str(),sbName(sb).c_str(), polName(pol), sp, mFg2, sHalf);

   return string(sbuff);
}

template<> void get_sprintf_fmt(char* s, const bool*) { strcpy(s, "%u"); }
template<> void get_sprintf_fmt(char* s, const int*) { strcpy(s, "%d"); }
template<> void get_sprintf_fmt(char* s, const unsigned*) { strcpy(s, "%u"); }
template<> void get_sprintf_fmt(char* s, const double*) { strcpy(s, "%.15g"); }
template<> void get_sprintf_fmt(char* s, const string*) { strcpy(s, "%s"); }

template<class T> int to_string(const T& value, char* s, size_t n)
{
   char fmt[256];
   get_sprintf_fmt(fmt, &value);
   return snprintf(s, n, fmt, value);
}

template<> int to_string(const bool& value, char* s, size_t n)
{
   unsigned u = value ? 1 : 0;
   return snprintf(s, n, "%u", u);
}

template<> int to_string(const string& value, char* s, size_t n)
{
   return snprintf(s, n, "%s", value.c_str());
}

template int to_string(const unsigned& value, char* s, size_t n);
template int to_string(const int& value, char* s, size_t n);
template int to_string(const double& value, char* s, size_t n);

template<class T> T from_string(const std::string& s)
{
   T value;

   istringstream(s) >> value;

   return value;
}

template<> bool from_string<bool>(const std::string& s)
{
   if(s == "true")
      return true;

   if(s == "false")
      return false;

   return from_string<unsigned>(s) != 0;
}

static void eat_non_numeric_or_zero(istream* i)
{
   while(1)
   {
      char c = i->peek();
      if(strchr("123456789", c))
         break;
      else
         i->get(c);
   }
}

template<> time_t from_string(const std::string& s)
{
   tm t;
   istringstream is(s);

   eat_non_numeric_or_zero(&is);
   is >> t.tm_hour;

   eat_non_numeric_or_zero(&is);
   is >> t.tm_min;

   eat_non_numeric_or_zero(&is);
   is >> t.tm_sec;

   eat_non_numeric_or_zero(&is);
   is >> t.tm_mon;

   eat_non_numeric_or_zero(&is);
   is >> t.tm_mday;

   eat_non_numeric_or_zero(&is);
   is >> t.tm_year;

   t.tm_mon -= 1;
   t.tm_year -= 1900;
   t.tm_isdst = -1;
   return mktime(&t);
}


template<> std::string from_string<std::string>(const std::string& s) { return s; }

template double from_string<double>(const std::string& s);
template int from_string<int>(const std::string& s);
template unsigned from_string<unsigned>(const std::string& s);

template<class T> std::string to_string(const T& value, int)
{
   string s;
   s.resize(128);
   to_string<T>(value, &(s[0]), 127);
   return s;
}

template std::string to_string<unsigned>(const unsigned&, int);
template std::string to_string<int>(const int&, int);
template std::string to_string<double>(const double&, int);
template std::string to_string<bool>(const bool&, int);
template std::string to_string<string>(const string&, int);


template<> void get_sscanf_fmt(char* s, int*) { strcpy(s, "%d"); }
template<> void get_sscanf_fmt(char* s, unsigned*) { strcpy(s, "%u"); }
template<> void get_sscanf_fmt(char* s, double*) { strcpy(s, "%lf"); }
template<> void get_sscanf_fmt(char* s, std::string*) { strcpy(s, "%s"); }


template<class V> bool extract_val(const std::string& s, const std::string& name, V* value)
{
   size_t pos = s.find(name);

   if(pos !=  string::npos)
   {
      string s2 = s.substr(pos+name.length());
      char fmt[16];

      get_sscanf_fmt<V>(fmt, value);

      return (sscanf(s2.c_str(), fmt, value) != 0);
   }

   return false;
}

template<> bool extract_val(const std::string& s, const std::string& name, std::string* value)
{
   size_t pos = s.find(name);

   if(pos !=  string::npos)
   {
      string s2 = s.substr(pos+name.length());
      size_t pos2 = s2.find(" ");

      if(pos2 == string::npos)
         pos2 = s2.length();

      *value = s2.substr(0, pos2);
      return value->length() > 0;
   }

   return false;
}

template<class V> V extract_val(const std::string& s, const std::string& name)
{
   V value;
   extract_val<V>(s, name, &value);

   return value;
}

template bool extract_val(const std::string&, const std::string&, int*);
template bool extract_val(const std::string&, const std::string&, unsigned*);
template bool extract_val(const std::string&, const std::string&, double*);
//template bool extract_val(const std::string&, const std::string&, std::string*);

template unsigned extract_val(const std::string&, const std::string&);


//! Turn a comma-delimeted string into a vector of strings
std::vector<std::string> commaDelimStrings2Vector(const std::string& s)
{
   vector<string> v;

   size_t i0 = 0;
   for(unsigned i=0; i<s.length(); i++)
   {
	   if( (i+1) == s.length() )
			v.push_back(s.substr(i0));
	   else
	   {
		   if( (s[i] == ',') )
		   {
			   v.push_back( s.substr(i0, i-i0) );
			   i0 = i+1;
		   }
	   }
   }

   return v;
}
