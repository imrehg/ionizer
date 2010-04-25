#include "Numerics.h"
#include "fractions.h"

#include <sstream>
#include <stdexcept>

//numeric functions for the Aluminizer program

using namespace std;

namespace numerics
{

double factorial(fraction rhs)
{
	int n=rhs.integer();
	if ( n>=0 )
	{
		int lhs = 1;
		for( int x = 1; x <= n; ++x )
		{
			lhs *= x;
		}
		return lhs;
	}
	throw "Factorial needs to be >=0!";
}

double Wigner3jSymbol(fraction j1, fraction m1, fraction j2, fraction m2, fraction j, fraction m)
{
	// error checking
	if ( !j1.ishalfinteger() || !j2.ishalfinteger() || !j.ishalfinteger() || !m1.ishalfinteger() || !m2.ishalfinteger() || !m.ishalfinteger() )
		throw runtime_error("Wigner3jSymbol: All arguments to must be integers or half-integers.");

	if ( j1 - m1 != ( j1 - m1 ).integer() )
		throw runtime_error("Wigner3jSymbol: 2*j1 and 2*m1 must have the same parity");

	if ( j2 - m2 != ( j2 - m2 ).integer() )
		throw runtime_error("Wigner3jSymbol: 2*j2 and 2*m2 must have the same parity");

	if ( j - m != ( j - m ).integer() )
		throw runtime_error("Wigner3jSymbol: 2*j and 2*m must have the same parity");

	if ( j > j1 + j2 || j.result() < fabs((j1 - j2).result()) )
		throw runtime_error("Wigner3jSymbol: j is out of bounds.");

	if ( fabs(m1.result()) > j1.result() )
		throw runtime_error("Wigner3jSymbol: m1 is out of bounds.");

	if ( fabs(m2.result()) > j2.result() )
		throw runtime_error("Wigner3jSymbol: m2 is out of bounds.");

	if ( fabs(m.result()) > j.result() )
		throw runtime_error("Wigner3jSymbol: m1 is out of bounds.");

	// check if physically meaningful
	//cout << m1 << "\t" << m2 << "\t" << m << endl;

	if ( m1+m2+m == 0 )
	{
		double Delta = factorial(j1+j2-j)*factorial(j1-j2+j)*factorial(-j1+j2+j)/factorial(j1+j2+j+1);
		fraction t1 = j2 - m1 - j;
		fraction t2 = j1 + m2 - j;
		fraction t3 = j1 + j2 - j;
		fraction t4 = j1 - m1;
		fraction t5 = j2 + m2;

		fraction tmin = std::max( fraction(0), std::max( t1, t2 ) );
		fraction tmax = std::min( t3, std::min( t4, t5 ) );

		double Wigner3jSum=0;
		for ( int t = tmin.integer(); t<=tmax.integer() ; ++t)
		{
			Wigner3jSum += (t % 2 ? 1.0 : -1.0) / ( factorial(t)*factorial(t-t1)*factorial(t-t2)*factorial(t3-t)*factorial(t4-t)*factorial(t5-t) );
		}
		return Wigner3jSum*pow(-1.,(j1-j2-m).integer())*sqrt( Delta )*sqrt( factorial(j1+m1)*factorial(j1-m1)*factorial(j2+m2)*factorial(j2-m2)*factorial(j+m)*factorial(j-m) );
	} else
		return 0;
}

double CGK(fraction j1, fraction m1, fraction j2, fraction m2, fraction j, fraction m)
{
	return pow(-1.,(m+j1-j2).integer())*sqrt(static_cast<double>((2*j+1).integer()))*Wigner3jSymbol(j1,m1,j2,m2,j,-m);
}

fraction operator +(fraction f1, fraction f2)
{ 
	fraction f = f1;
	return f += f2;
}

fraction operator -(fraction f)
{ 
	return 0-f;
}

fraction operator -(fraction f1, fraction f2)
{ 
	fraction f = f1;
	return f -= f2;
}

fraction operator *(fraction f1, fraction f2)
{ 
	fraction f = f1;
	return f *= f2;
}

fraction operator /(fraction f1, fraction f2)
{ 
	fraction f = f1;
	return f /= f2;
}

ostream& operator << (ostream& o, fraction f)
{
	f.reduce();

	if(f.num > 0)
		o << "+";

	o << f.num;

	if(f.den != 1)
		o << "/" << f.den;

	return o;
}

istream& operator>> (istream& s, fraction& f)
{
	int num = 0;
	int den = 1;

	s >> num;

	if( ! s.eof() )
	{
		s.ignore(1, '/');
		if( ! s.eof() )
			s >> den;
	}

	f = fraction(num, den);

	return s;
}

const fraction& fraction::GetValueFromString(const std::string& s)
{
	istringstream iss(s.c_str());
	iss >> *this;

	return *this;
}

int fraction::integer()
{
	reduce();

	return num/den;
}

bool fraction::ishalfinteger()
{
	reduce();

	return ( den==2 || den==1 );
}

const fraction& fraction::reduce()
{
	//reduced form should always have a positive denominator
	if(den < 0) {
		den *= -1;
		num *= -1;
	}

	C g = gcd(num, den);
	num /= g;
	den /= g;

	return *this;
}

fraction::C fraction::gcd(fraction::C c1, fraction::C c2)
{
	//gcd for 0,0 should be 1 I suppose
	if((c1 == 0) && (c2 == 0))
		return 1;

	//only use positive numbers for now.
	if(c1 < 0) c1 *= -1;
	if(c2 < 0) c2 *= -1;

	//special case
	if(c1 == c2)
		return c1;

	//make sure c1 > c2
	if(c1 < c2)
		swap(c1, c2);
	
	while(c2 != 0) {
		c1 %= c2;
		swap(c1, c2);
	}
		
	return c1;
}

fraction abs(fraction f)
{ 
	return f > 0 ? f : -1*f; 
}

}

