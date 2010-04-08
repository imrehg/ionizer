#pragma once

#include "AnalogOutput.h"

class MCC_AnalogOut : public AnalogOut
{
public:
MCC_AnalogOut(int iDevice, int iChannel);
MCC_AnalogOut(int iDevice, std::vector<int> iChannels);

virtual ~MCC_AnalogOut()
{
}


virtual void SetOutput(double);
virtual double GetOutput()
{
	return dOutput;
}

int GetChannel()
{
	return iChannels[0];
}

protected:
short iDev;
std::vector<int> iChannels;

double dOutput;
};

