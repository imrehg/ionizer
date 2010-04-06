#include "actuator_opt.h"

#include <algorithm>
#include <iostream>
#include <cstdio>

using namespace std;

opt_actuator::opt_actuator(unsigned N) : x(N, 0)
{
	t0.start();
}

void opt_actuator::measure(const std::vector<double>& xnew)
{
	cout << t0.elapsed() * 0.001 << " [opt_actuator::measure] dx = ";

	vector<double> dx(x.size());

	for(size_t i=0; i<x.size(); i++)
	{
		dx[i] = xnew[i] - x[i];
		x[i] = xnew[i];

		cout << dx[i] << " ";
	}

	cout << endl;

	shift(dx);
}

Actuator_opt::Actuator_opt(unsigned N, opt_actuator* act) :
 nDimensions(N), nIterations(0), bStop(false), act(act), fLog(0)
{
}

 Actuator_opt::~Actuator_opt()
 {
   bStop = true;
	is_measurement_complete.wakeAll();
	this->wait();
 }

 void Actuator_opt::run()
{
	tRun.start();

	char buff[64];
	string str = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss").toStdString();
	snprintf(buff, 64, "nm_opt_%s.txt", str.c_str());
	cout << buff << endl;
	fLog = fopen(buff, "w");

	while(! bStop)
	{
		iterate(nIterations);
		nIterations++;
	}
 }

