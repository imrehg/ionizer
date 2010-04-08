#pragma once

#include <string>

/*
   a general frequency source
   all units are in Hz and seconds

   any class derived from FrequencySource must make sure that it calls
   SetInternalFreq(f) with the latest frequency whenever that changes.
 */

class FrequencySource
{
public:
FrequencySource(double dMinFreq, double dMaxFreq, double dMaxRamp, const std::string& name);
virtual ~FrequencySource()
{
}

void SetScanOutput(double d)
{
	SetDeviceFrequency(d * 1e6);
}
double GetOutput()
{
	return GetFrequency() * 1e-6;
}
std::string GetName()
{
	return name;
}
std::string GetUnit()
{
	return "MHz";
}
double GetMin()
{
	return dMinFreq;
}
double GetMax()
{
	return dMaxFreq;
}

//return the frequency of the device.
double GetFrequency(double* t = 0)
{
	if (t) *t = tLastFreqUpdate;return frequency;
}

//set the frequency of the device
virtual void SetDeviceFrequency(double dNewFreq) = 0;

//set the output power/voltage
virtual void SetOutputPower(double)
{
}
virtual void SetOutputVoltage(double)
{
}
virtual void SetOutputVoltage(double, double)
{
}

//sweep to a new frequency, while respecting dMaxRampRate
void SweepTo(double dNewFrequency);

//shift the current frequency
virtual void ShiftFrequency(double dShift);

bool IsValidFrequency(double f)
{
	return f >= dMinFreq && f <= dMaxFreq;
}

protected:
double SetInternalFreq(double f);
double GetLastUpdateTime() const
{
	return tLastFreqUpdate;
}
virtual void RefreshDisplay()
{
}
public:
double dMinFreq;
double dMaxFreq;
double dMaxRampRate;

std::string name;

private:
double frequency, tLastFreqUpdate;
};

/* an extension to the general frequency source that allows
   a linear ramp of the frequency */
class SweepingFrequencySource : public FrequencySource
{
public:
SweepingFrequencySource(double dMinFrequency, double dMaxFrequency, double dMaxRampRate, const std::string& name);

virtual ~SweepingFrequencySource()
{
}

// return an estimate of the current frequency using the last known frequency and an internal drift update_period
// this may differ slightly from the actual current frequency of the device
virtual double GetEstimatedFrequency();

// ask the device for its current frequency and return that
// this value will include the linear ramp, but may be slightly dated by the time the function returns
virtual double GetCurrentDeviceFrequency() = 0;

// set/get ramp update_period
virtual double GetRampRate() = 0;
virtual double GetWindow() = 0;
virtual double GetTimeConst() = 0;
virtual std::string getStatusText() = 0;
virtual void newErrSig(double err_sig) = 0;
virtual void ClearHistory()
{
};

protected:
double dTimeConst, dWindow, dRampRate;
};




