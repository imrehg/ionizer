#ifndef NELDER_MEAD_OPT_H
#define NELDER_MEAD_OPT_H

#include <gsl/gsl_multimin.h>

#include "actuator_opt.h"


/* Nelder-Mead optimizer based on GSL.
 * Runs in a separate thread.
 */


class NelderMead_opt : public Actuator_opt
{
public:
	NelderMead_opt(unsigned N, unsigned num_to_measure, double dx0, opt_actuator* act);
	virtual ~NelderMead_opt();

	//! Tell the optimizer what the current function value is.
	virtual void measured_f(double f, bool bFault);

	double measure_f(const gsl_vector* params);

protected:

	virtual void iterate(unsigned iter);

	unsigned num_to_measure, num_measured;

    gsl_multimin_fminimizer* fminimizer;
    gsl_multimin_function minex_func;

	 gsl_vector* x; //current position
    gsl_vector* ss; //step size

	double f_max, f_total, simplex_size;

	bool bMeasuring, bHadFault;

	QTime tMeasured;
};

double gsl_wrapper_measure_f(const gsl_vector* params, void* p);

#endif //NELDER_MEAD_OPT_H
