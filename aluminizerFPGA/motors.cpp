#ifdef ALUMINIZER_SIM
	#include "pulse_controller.h"
#else
extern "C"
{
	#include "pulse_controller.h"
}
#endif

#include "motors.h"

#include <stdio.h>

using namespace std;

vector<motor> motors;

motor::motor(void* pulser, unsigned ttl0, unsigned ttl1, unsigned angle, unsigned nPulses) : pulser(pulser), ttl0(ttl0), ttl1(ttl1), nPulses(nPulses)
{
	setAngle(angle, true);
}


void motor::setAngle(unsigned a, bool bForceUpdate)
{
	if ( (currentAngle != a) || bForceUpdate)
	{
		printf("Motor %d --> %d\n", currentAngle, a);

		//make pulse-train to adjust angle
		//0 ms = 0 deg
		//2 ms = 180 deg
		unsigned t1 = 81000 + (a * 100000) / (2700);

		//pulse-train has 20 ms period
		unsigned t0 = 2000000 - t1;

		for (unsigned i = 0; i < nPulses; i++)
		{
			PULSE_CONTROLLER_pulse(pulser, t1, 0, ttl1);
			PULSE_CONTROLLER_pulse(pulser, t0, 0, ttl0);
		}

		currentAngle = a;
	}
}
