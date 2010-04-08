#ifndef MY_MATRIX_H
#define MY_MATRIX_H

#include <assert.h>

//! Our own matrix class.  Should probably use something standard.  Used for MatrixWidget.
class my_matrix
{
public:
   my_matrix() : nr(0), nc(0), n(0), p(0)
   {
   }

   my_matrix(unsigned nr, unsigned nc) : nr(nr), nc(nc), n(nr*nc)
   {
      p = new double[n];

      for(unsigned i=0; i<n; i++)
         p[i] = 0;
   }

   my_matrix(const my_matrix& m) : nr(0), nc(0), n(0), p(0)
   {
      CopyObj(m);
   }

   ~my_matrix()
   {
      if(p)
         delete []p;
   }

   //! resize by filling w/ zeros if necessary
   bool resize(unsigned nR, unsigned nC);


   unsigned get_binary_length();
   void insert_binary(char* p);
   void extract_binary(const char* p);

   void CopyObj(const my_matrix& m)
   {
      nr = m.nr;
      nc = m.nc;
      n = nr*nc;

      if(p)
         delete []p;

      p = new double[n];

      for(unsigned i=0; i<n; i++)
         p[i] = m.p[i];
   }


   double element(unsigned r, unsigned c) const;
   double& element(unsigned r, unsigned c);
   double* elementP(unsigned r, unsigned c);

   my_matrix& operator=(const my_matrix& rhs)
   {
     CopyObj(rhs);
     return *this;
   }

   void from_product(const my_matrix& f1, const my_matrix& f2);

   unsigned nr, nc, n;
   double* p;
};

bool operator==(const my_matrix&, const my_matrix&);
bool operator!=(const my_matrix&, const my_matrix&);

#endif //MY_MATRIX_H

