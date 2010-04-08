#pragma once

#include <string>
#include <math.h>

namespace numerics
{


class fraction
{
protected:
   typedef int C;
   typedef double D;

public:
   fraction() : num(0), den(1) {};
   fraction(C num, C den=1) : num(num), den(den) {};
   fraction(D d)
   {
      for(den=1; den<10; den++)
      {
         num = static_cast<C>(floor(den*d));
         if(num == den*d)
            break;
      }
   }

   const fraction& GetValueFromString(const std::string& s);
   const fraction& reduce();
   int integer();
   bool ishalfinteger();

//	operator D() const { return result(); }

   D result() const { return static_cast<D>(num) / static_cast<D>(den); }

   fraction operator +=(fraction f)
   {
      num = num*f.den + den*f.num;
      den *= f.den;
      reduce();
      return *this;
   }

   fraction operator -=(fraction f)
   {
      num = num*f.den - den*f.num;
      den *= f.den;
      reduce();
      return *this;
   }

   fraction operator *=(fraction f)
   {
      num *= f.num;
      den *= f.den;
      reduce();
      return *this;
   }

   fraction operator /=(fraction f)
   {
      num *= f.den;
      den *= f.num;
      reduce();
      return *this;
   }

   fraction operator ++() { return *this += 1; }
   fraction operator --() { return *this -= 1; }

   bool operator ==(const fraction& f) const
   {
      return (den == 0 || f.den == 0) ? (num == f.num && den == f.den) : num*f.den == f.num*den;
   }

   bool operator !=(const fraction& f) const { return !(f == *this); }
   bool operator <(const fraction& f) const {return (f.den*den < 0) ^ (num*f.den < f.num*den); }
   bool operator >(const fraction& f) const {return (f.den*den < 0) ^ (num*f.den > f.num*den); }
   bool operator <=(const fraction& f) const {return (*this < f) || (*this == f); }
   bool operator >=(const fraction& f) const {return (*this > f) || (*this == f); }

   friend std::ostream& operator << (std::ostream& o, fraction f);

   static C gcd(C c1, C c2);

protected:
   C num;
   C den;
};

fraction abs(fraction f);

std::istream& operator>> (std::istream& s, fraction& f);

fraction operator +(fraction f1, fraction f2);
fraction operator -(fraction f);
fraction operator -(fraction f1, fraction f2);
fraction operator *(fraction f1, fraction f2);
fraction operator /(fraction f1, fraction f2);

double factorial(fraction rhs);
double Wigner3jSymbol(fraction j1, fraction m1, fraction j2, fraction m2, fraction j, fraction m);
double CGK(fraction j1, fraction m1, fraction j2, fraction m2, fraction j, fraction m);

std::ostream& operator << (std::ostream& o, fraction f);

}
