#include "NelderMead_opt.h"

#include <algorithm>
#include <iostream>

#ifdef WIN32
#define  snprintf  _snprintf
#endif

using namespace std;

double gsl_wrapper_measure_f(const gsl_vector* params, void* p)
{
	return ((NelderMead_opt*) p)->measure_f(params);
}



NelderMead_opt::NelderMead_opt(unsigned N, unsigned num_to_measure, double dx0, opt_actuator* act) : 
 Actuator_opt(N, act),
 num_to_measure(num_to_measure), num_measured(0),
 fminimizer(0), f_total(0), simplex_size(0), f_max(0), bMeasuring(false), bHadFault(false)
{
	/* Starting point */
	x = gsl_vector_alloc(N);
    gsl_vector_set_all (x, 0);


    /* Set initial step sizes to  */
	ss = gsl_vector_alloc(N);
    gsl_vector_set_all (ss, dx0);

	 /* Initialize method and iterate */
    minex_func.n = N;
	minex_func.f = gsl_wrapper_measure_f;
    minex_func.params = this;

    fminimizer = gsl_multimin_fminimizer_alloc (gsl_multimin_fminimizer_nmsimplex, N);
}	

 NelderMead_opt::~NelderMead_opt()
 {
   bStop = true;
	is_measurement_complete.wakeAll();
	this->wait();

    gsl_multimin_fminimizer_free (fminimizer);
 }

 //! Tell the optimizer what the current function value is.
void NelderMead_opt::measured_f(double f, bool bFault)
{
	if(!bMeasuring)
		return;

	if(num_measured == 0)
	{
		bHadFault = false;
		f_total = 0;
		f_max = 0;
	}

	bHadFault = bFault || bHadFault;

	if(f_max < f)
		f_max = f;

	f_total += f;

	num_measured++;

	if(num_measured >= num_to_measure)
	{
		if(!bHadFault)
			f_max = f_total / num_measured;

		bMeasuring = false;
		num_measured = 0;
		is_measurement_complete.wakeAll();
	}
}

double NelderMead_opt::measure_f(const gsl_vector* params)
{
	if(bStop)
		return 0;

	tMeasured.start();
	
	vector<double> xnew(nDimensions);

	for(size_t i=0; i<nDimensions; i++)
		xnew[i] = gsl_vector_get(params, i);

	act->measure(xnew);
	bMeasuring = true;

	mtx.lock();
	is_measurement_complete.wait(&mtx);
	mtx.unlock();

	cout << "[NelderMead_opt::measure_f] f_max = " << f_max << endl;

	simplex_size = gsl_multimin_fminimizer_size (fminimizer);

	if(fLog)
	{
		fprintf(fLog, "%u, %6.4f, %8.0f", tRun.elapsed(), f_max, simplex_size);

		for(unsigned i=0; i<nDimensions; i++)
			fprintf(fLog, ", %12.4f", gsl_vector_get (params, i));

		fprintf(fLog, "\n");
		fflush(fLog);
	}

    return -1*f_max;
}

void NelderMead_opt::iterate(unsigned iter)
{
   if(iter == 0)
		gsl_multimin_fminimizer_set (fminimizer, &minex_func, x, ss);

	gsl_multimin_fminimizer_iterate(fminimizer);

	simplex_size = gsl_multimin_fminimizer_size (fminimizer);

	num_to_measure = 3000000 / (std::max<double>(fabs(simplex_size), 200.0));
	num_to_measure = std::max<unsigned>(num_to_measure, 1000);

	printf ("(%5d) %10.3e %10.3e f() = %7.6f size = %.5f\n", (int)iter, gsl_vector_get (fminimizer->x, 0), gsl_vector_get (fminimizer->x, 1), fminimizer->fval, simplex_size);

	if(simplex_size < 50)
	{
		//restart optimizer from last position
		gsl_vector_set_all (ss, 300);

		for(size_t i=0; i<nDimensions; i++)
			gsl_vector_set(x, i, act->x[i]);

		gsl_multimin_fminimizer_set (fminimizer, &minex_func, x, ss);
	}
}
