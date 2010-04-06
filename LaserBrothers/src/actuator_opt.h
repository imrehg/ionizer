#ifndef ACTUATOR_OPT_H
#define ACTUATOR_OPT_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>

#include <vector>

class opt_actuator;

/* General base class that optimizes a signal by adjusting actuators */
class Actuator_opt : public QThread
{
public:
	Actuator_opt(unsigned N, opt_actuator* act);
	virtual ~Actuator_opt();

	//! Tell the optimizer what the current function value is.
	virtual void measured_f(double f, bool bFault) = 0;

	void run();

protected:

	//! iterate the optimization
	virtual void iterate(unsigned iter) = 0;

	unsigned nDimensions, nIterations;
	bool bStop;
	opt_actuator* act;

	FILE* fLog;
	QTime tRun;

	QMutex mtx;
	QWaitCondition is_measurement_complete;
};

/* General base class for actuators that are controlled by the optimizer*/
class opt_actuator
{
public:
	opt_actuator(unsigned N);

	void measure(const std::vector<double>& x_new);
	virtual void shift(const std::vector<double>& dx) = 0;

	std::vector<double> x; //current position

	QTime t0;
};

#endif // ACTUATOR_OPT_H
