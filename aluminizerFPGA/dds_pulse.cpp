#include "ttl_pulse.h"
#include "dds_pulse.h"
#include "host_interface.h"

unsigned FTW2Hz(unsigned ftw)
{
	return (unsigned)floor(ftw * 1.0e9 * pow(2., -32) + 0.5);
}

double FTW2HzD(unsigned ftw)
{
	return ftw * 1.0e9 * pow(2., -32);
}

unsigned int Hz2FTW(double f)
{
	return static_cast<unsigned int>(floor(0.5 + (f * pow(2., 32.) / 1e9)));
}

unsigned int MHz2FTW(double f)
{
	return Hz2FTW(f * 1e6);
}

void print_pulse_info(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned t, const char* info)
{
	if (info)
		sprintf(host->buffDebug, "%s DDS(%u) fOn = %9u, fOff = %9u, t = %8.2f us (%s)\n", DDS_name(iDDS), iDDS, FTW2Hz(ftwOn), FTW2Hz(ftwOff), 0.01 * (double)t, info);
	else
		sprintf(host->buffDebug, "%s DDS(%u) fOn = %9u, fOff = %9u, t = %8.2f us\n", DDS_name(iDDS), iDDS, FTW2Hz(ftwOn), FTW2Hz(ftwOff), 0.01 * (double)t);

	host->sendDebugMsg(host->buffDebug);
}

