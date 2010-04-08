#pragma once

#include "CriticalSection.h"
#include "AnalogOutput.h"

#ifndef _NO_NIDAQ
#include <NIDAQmx.h>
#else
typedef unsigned TaskHandle;
#endif

//analog output that allows triggered updates during pulse programs
class TriggererdNIAnalogOut
{
public:
TriggererdNIAnalogOut(int iDevice);
virtual ~TriggererdNIAnalogOut();

//baseline is generally the steady-state output voltage
void SetBaselineOutput(int channel, double v);
std::vector<double> GetBaselineOutput();

//use baseline voltages as offsets to the waveform spec'd in wv
void LoadOffsetWaveform(double* o, CriticalSectionOwner* pCSO = 0);

int GetNumChannels() const
{
	return nChannels;
}

std::string ChannelList() const;

void HoldUpdates(bool b);
void ForceUpdates();

protected:

void SetupSingleGeneration();
void SetupContinuousGeneration(int);


int nChannels;
int iDevice;
std::vector<double> baseline;
std::vector<double> offset;

TaskHandle taskHandle;

NamedCriticalSection csDAQ;

double* wv;
bool bHoldUpdates;
};


class NI_AnalogOut : public AnalogOut
{
public:
NI_AnalogOut(TriggererdNIAnalogOut* tNI, int iChannel, double min_output, double max_output, double gain = 1, double offset = 0);
virtual ~NI_AnalogOut();

virtual void SetOutput(double);
virtual double GetOutput()
{
	return dVoltage;
}

int GetChannel()
{
	return iChannel;
}

protected:
int iChannel;
double dVoltage;
TriggererdNIAnalogOut* tNI;
};

