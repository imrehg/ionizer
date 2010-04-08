#include <string>
#include <sstream>
#include <cstdio>
#include <assert.h>
#include <math.h>
#include "my_matrix.h"
#include "string_func.h"

using namespace std;

void swap_byte_order(const char* src, char* dest, unsigned size)
{
   for(unsigned i=0; i<size; i++)
      dest[i] = src[size-i-1];
}

unsigned my_matrix::get_binary_length()
{
   return nr*nc*sizeof(float);
}

void my_matrix::insert_binary(char* c)
{
   unsigned size = sizeof(int);

   for(unsigned i=0; i<nr; i++)
      for(unsigned j=0; j<nc; j++)
      {
         int d = (int)(floor(element(i,j)*1.0e6 + 0.5));
#ifdef LITTLE_ENDIAN
         swap_byte_order((char*)&d, c, size);
#else
         memcpy((void*)c, (void*)&d, size);
#endif
         c += size;
      }
}

void my_matrix::extract_binary(const char* c)
{
   unsigned size = sizeof(int);
   int d;

   for(unsigned i=0; i<nr; i++)
      for(unsigned j=0; j<nc; j++)
      {

#ifdef LITTLE_ENDIAN
         swap_byte_order(c, (char*)&d, size);
#else
         memcpy((char*)&d, (void*)c, size);
#endif
         element(i,j) = d*1.0e-6;
         c += size;
      }

}

double my_matrix::element(unsigned r, unsigned c) const
{
   unsigned i = r*nc + c;
   assert(i < n);
   return p[i];
}

double& my_matrix::element(unsigned r, unsigned c)
{
   unsigned i = r*nc + c;
   assert(i < n);
   return p[i];
}

double* my_matrix::elementP(unsigned r, unsigned c)
{
   unsigned i = r*nc + c;
   assert(i < n);
   return p+i;
}

template<> std::string to_string(const my_matrix& m, int)
{
   ostringstream os;

   os << "(" << m.nr << "x" << m.nc << ") ";

   for(unsigned i=0; i<m.nr; i++)
      for(unsigned j=0; j<m.nc; j++)
         os << m.element(i,j) << " ";

   return os.str();
}

template<> int to_string(const my_matrix& m, char* s, size_t n)
{
   std::string buff = to_string<my_matrix>(m);
   return snprintf(s, n, "%s", buff.c_str());
}

template<> my_matrix from_string(const std::string& s)
{
   unsigned nR=0;
   unsigned nC=0;

   sscanf(s.c_str(), "(%ux%u)", &nR, &nC);
   my_matrix m(nR,nC);

   unsigned start = s.find(")");
   istringstream is(s.substr(start+1));

   for(unsigned i=0; i<m.nr; i++)
      for(unsigned j=0; j<m.nc; j++)
      {
         double t = 0;
         is >> t;

         m.element(i,j) = t;
      }

   return m;
}

bool operator==(const my_matrix& m1, const my_matrix& m2)
{
   return !(m1 != m2);
}

bool operator!=(const my_matrix& m1, const my_matrix& m2)
{
   if(m1.nr != m2.nr)
      return true;

   if(m1.nc != m2.nc)
      return true;

   for(unsigned i=0; i<m1.nr; i++)
      for(unsigned j=0; j<m1.nc; j++)
         if(m1.element(i,j) != m2.element(i,j))
            return true;

   return false;
}

void my_matrix::from_product(const my_matrix& f1, const my_matrix& f2)
{
   assert(f1.nc == f2.nr);
   assert(nr == f1.nr);
   assert(nc == f2.nc);

   for(unsigned rp=0; rp<nr; rp++)
      for(unsigned cp=0; cp<nc; cp++)
      {
         element(rp, cp) = 0;
         for(unsigned c1=0; c1<f1.nc; c1++)
            element(rp, cp) = element(rp, cp) + f1.element(rp,c1) * f2.element(c1,cp);
      }
}

bool my_matrix::resize(unsigned nR, unsigned nC)
{
   if(nr != nR || nc != nC)
   {
      unsigned nNew = nR*nC;
      double* pNew =  new double[nNew];

      unsigned iMax = std::min<unsigned>(nNew, n);
      unsigned i=0;

      for(; i<iMax; i++)
         pNew[i] = p[i];

      for(; i<nNew; i++)
         pNew[i] = 0;

      delete []p;
      p = pNew;
      n = nNew;

      nr = nR;
      nc = nC;

      return true;
   }

   return false;
}
