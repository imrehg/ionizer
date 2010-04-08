#ifndef MOTORS_H_
#define MOTORS_H_

#include <vector>

class motor
{
public:
motor(void* pulser, unsigned ttl0, unsigned ttl1, unsigned angle, unsigned nPulses = 40);

void setAngle(unsigned a, bool bForceUpdate = false);
unsigned getAngle() const
{
	return currentAngle;
}

void* pulser;
unsigned ttl0, ttl1;
unsigned currentAngle, nPulses;
};

extern std::vector<motor> motors;

#endif /*MOTORS_H_*/
