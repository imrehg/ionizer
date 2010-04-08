/////////////////////////////////////////////////////////////////////////////
// Name:        lm_lsqr.h
// Purpose:     Levenberg-Marquart nonlinear least squares 2-D curve fitting
// Author:      John Labenski, mostly others (see below)
// Modified by:
// Created:     6/5/2002
// Copyright:   (c) John Labenski, mostly others (see below)
// Licence:     Public domain
/////////////////////////////////////////////////////////////////////////////

/*
 * Solves or minimizes the sum of squares of m nonlinear
 * functions of n variables.
 *
 * From public domain Fortran version
 * of Argonne National Laboratories MINPACK
 *
 * argonne national laboratory. minpack project. march 1980.
 * burton s. garbow, kenneth e. hillstrom, jorge j. more
 *
 * C translation by Steve Moshier http://www.moshier.net/
 * 
 * C++ "translation" for use w/ wxWindows by John Labenski
 */

#ifndef _LM_LEASTSQUARE_H_
#define _LM_LEASTSQUARE_H_

// When SetLM_LeastSquareProgressHandler is called with a non NULL handler it will be
//   called when fitting a curve every SetLM_LeastSquareProgressHandlerTicks
//      text is the function name
//      current is the current iteration, max is max allowed iterations
//          note: current may exceed max by a few iterations in some cases

// Usage: create a function like this
//      void LM_LeastSquareProgressHandler(const wxString &text, int current, int max) {do stuff}
//      have it called by calling SetLM_LeastSquareProgressHandler( LM_LeastSquareProgressHandler );

#include "Numerics.h"
#include "../Fitting.h"

namespace numerics
{

extern "C" {
	typedef bool (*LM_LeastSquareProgressHandler_)(const std::string text, 
                                               int current, 
                                               int max);
extern void SetLM_LeastSquareProgressHandler( LM_LeastSquareProgressHandler_ handler );
extern void SetLM_LeastSquareProgressHandlerTicks( int iterations );
}

//=============================================================================
// LM_LeastSquare - Levenberg-Marquart nonlinear least squares 2-D curve fitting
//=============================================================================

class LM_LeastSquare
{
public:
   
	LM_LeastSquare(int n, const std::vector<double>& x, numerics::FitObject* pFitObject);

    virtual ~LM_LeastSquare();
    
    // Initialize everything, returns sucess
    bool Create();
    // After creation fit the plotFunc's vars to the plotData, returns # iterations
    //   initial_vals are initial guesses for the variables which may be NULL
    int Fit();

    // Get the number of evaluations performed to find best fit
    int GetNumberIterations() const { return m_nfev; }
    // Get the euclidean norm of errors between data and function points
    double GetEuclideanNorm() const { return m_fnorm; }
    // Get the number of variables, i.e. (plotFunc.GetNumberVars() - 1, x is excluded)
    int GetNumberVariables() const { return m_n; }
    // Get the evaluated variables, size is (plotFunc.GetNumberVars() - 1, x is excluded)
	std::vector<double> GetVariables() const { return m_x; }
    // Get an informational message about the results
	std::string GetResultMessage() const;

protected:
    void Init();
    void Destroy();

    virtual void fcn(int m, int n, double x[], double fvec[], int *iflag);

    void lmdif( int m, int n, double x[], double fvec[], double ftol, 
                double xtol, double gtol, int maxfev, double epsfcn,
                double diag[], int mode, double factor, int nprint, int *info, 
                int *nfev, double fjac[], int ldfjac, int ipvt[], double qtf[], 
                double wa1[], double wa2[], double wa3[], double wa4[]);

private:

    void lmpar(int n, double r[], int ldr, int ipvt[], 
               double diag[], double qtb[], double delta, double *par,
               double x[], double sdiag[], double wa1[], double wa2[]);

    void qrfac(int m, int n, double a[], int lda, int pivot, int ipvt[], 
               int lipvt, double rdiag[], double acnorm[], double wa[]);

    void qrsolv(int n, double r[], int ldr, int ipvt[], double diag[], 
                double qtb[], double x[], double sdiag[], double wa[]);

    double enorm(int n, double x[]);

    void fdjac2(int m,int n, double x[], double fvec[], double fjac[], 
                int ldfjac, int *iflag, double epsfcn, double wa[]);

    int    m_n;       // # of variables of plotFunc
    int    m_m;       // # of functions = points in plotData
    int    m_info;    // index of info message strings
    double m_fnorm;   // euclidean norm of errors
    double m_eps;     // resolution of arithmetic
    double m_dwarf;   // smallest nonzero number
    int    m_nfev;    // # iterations completed
    unsigned long m_nan; // # if times function evaluation had a NaN
    double m_ftol;    // relative error in the sum of the squares, if less done
    double m_xtol;    // relative error between two iterations, if less done
    double m_gtol;    // cosine of the angle between fvec and any column of the jacobian, if less done
    double m_epsfcn;  // step length for the forward-difference approximation
    double m_factor;  // initial step bound
    double *m_fvec;   // output of evaluated functions (size m_m)
    double *m_diag;   // multiplicative scale factors for the variables, see m_mode
    int    m_mode;    // =1 the vars scaled internally. if 2, scaling specified by m_diag.
    double *m_fjac;   // output m by n array
    int    m_ldfjac;  // the leading dimension of the array fjac >= m_m
    double *m_qtf;    // output array the first n elements of the vector (q transpose)*fvec
    int    *m_ipvt;   // integer output array of length n
    int    m_maxfev;  // maximum number of iterations to try

	std::vector<double> m_x;
	numerics::FitObject* m_pFitObject;
};

} // namespace numerics

#endif // _LM_LEASTSQUARE_H_
