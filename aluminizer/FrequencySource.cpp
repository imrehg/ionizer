#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "FrequencySource.h"
#include "ionizer_utils.h"

#include <iostream>

using namespace std;

FrequencySource::FrequencySource(double dMinFrequency, double dMaxFrequency, double dMaxRampRate, const string &name) :
	dMinFreq(dMinFrequency),
	dMaxFreq(dMaxFrequency),
	dMaxRampRate(dMaxRampRate),
	name(name),
	frequency(0),
	tLastFreqUpdate(0)
{
}


void FrequencySource::ShiftFrequency(double dShift)
{
	if (IsValidFrequency( frequency + dShift) )
		SetDeviceFrequency(frequency += dShift);
}

void FrequencySource::SweepTo(double dNewFreq)
{
	if (frequency == dNewFreq || !IsValidFrequency(dNewFreq))
		return;

	try
	{

		if (dMaxRampRate < 0)
			SetDeviceFrequency(dNewFreq);

		cout << " f = " << fixed << dNewFreq << endl;

		if ((dNewFreq > dMaxFreq) || (dNewFreq < dMinFreq))
		{
			cerr << "DDSsweeper::SweepTo f = " << dNewFreq << " Hz is out of range." << endl;
			dNewFreq = (dMaxFreq + dMinFreq) / 2;
			cerr << "DDSsweeper::SweepTo sweeping to f = " << dNewFreq << " Hz" << endl;
		}
	}
	catch (runtime_error e)
	{
		cout <<  "[RemoteFrequencySource::SweepTo] error: " << e.what() << endl;
	}

	RefreshDisplay();
}

double FrequencySource::SetInternalFreq(double f)
{
	frequency = f;

#ifdef _DEBUG
//	cerr << "[" << name << "::SetFreq] " << fixed << f << " Hz" << endl;
#endif

	return tLastFreqUpdate = CurrentTime_s();
}

SweepingFrequencySource::SweepingFrequencySource(double dMinFreq,
                                                 double dMaxFreq,
                                                 double dMaxRampRate,
                                                 const string &name) :
	FrequencySource(dMinFreq, dMaxFreq, dMaxRampRate, name)
{
}

double SweepingFrequencySource::GetEstimatedFrequency()
{
	double f, t;

	f = GetFrequency(&t);
	return f + (CurrentTime_s() - t) * GetRampRate();
}
