#ifndef DDS_PULSE_H_
#define DDS_PULSE_H_

#include "ttl_pulse.h"

#include <float.h>
#include <math.h>
#include <algorithm>

#include "shared/src/messaging.h"
#include "host_interface.h"
#include "common.h"

#ifdef WIN32
#define scalb _scalb
#define snprintf _snprintf
#endif

extern bool bDebugPulses;

#define DDS_PRECOOL     (0)
#define DDS_RF       (2)
#define DDS_3P1x2    (1)
#define DDS_DETECT      (3)
#define DDS_3P0_BR      (4)
#define DDS_HEAT     (5)
#define DDS_RAMAN    (6)
#define DDS_3P0         (7)

#define DDS_NONE     (100)

#define PHASE_0         (0)
#define PHASE_90     (1 << 12)
#define PHASE_180    (1 << 13)
#define PHASE_270    (PHASE_90 + PHASE_180)

inline const char* DDS_name(unsigned iDDS)
{
	switch (iDDS)
	{
	case DDS_PRECOOL:     return "  Precool";
//		case DDS_RF:			 return "       RF";
	case DDS_3P1x2:          return "  3P1 x 2";
	case DDS_DETECT:      return "   Detect";
	case DDS_HEAT:        return "   RF/Heat";
	case DDS_RAMAN:          return "    Raman";
	case DDS_3P0:         return "  3P0    ";
	case DDS_3P0_BR:       return "  3P0(BR)";
	default:           return "  unknown";

	}
}

unsigned FTW2Hz(unsigned ftw);
double FTW2HzD(unsigned ftw);
unsigned int Hz2FTW(double f);
unsigned int MHz2FTW(double f);

void print_pulse_info(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned t, const char* info = 0);

//make an RF pulse of specified frequency and duration
inline void DDS_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned t, unsigned ttl = 0)
{
	if (t > 4)
	{
		PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftwOn);
		PULSE_CONTROLLER_short_pulse(pulser, t, ttl);
		PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftwOff);

		if (bDebugPulses)
			print_pulse_info(iDDS, ftwOn, ftwOff, t);
	}
}

inline void DDS_long_pulse(unsigned iDDS, unsigned ftwOn, unsigned ftwOff, unsigned t, unsigned flags, unsigned ttl = 0)
{
	if (t > 4)
	{
		PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftwOn);
		PULSE_CONTROLLER_pulse(pulser, t, flags, ttl);
		PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftwOff);

		if (bDebugPulses)
			print_pulse_info(iDDS, ftwOn, ftwOff, t, "LONG");
	}
}


/*
   inline void DDS_pulse(dds_pulse_params* pe)
   {
   DDS_pulse(p->iDDS, p->ftwOn, p->ftwOff, p->t);
   }
 */

inline void detection_pulse(unsigned ftwOn, unsigned ftwOff, unsigned t)
{
	t = std::max<unsigned>(t, (unsigned)5);

	unsigned iDDS = DDS_DETECT;
	PULSE_CONTROLLER_short_pulse(pulser, 2000, 0); //padding (20 us seems like a lot?)
	PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftwOn);
	PULSE_CONTROLLER_short_pulse(pulser, t | PULSE_CONTROLLER_COUNTING_PULSE_FLAG, TTL_DETECT_MON);
	PULSE_CONTROLLER_short_pulse(pulser, 5, 0); //padding
	PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftwOff);

	if (bDebugPulses)
		print_pulse_info(iDDS, ftwOn, ftwOff, t, "DETECT");
}

inline void DDS_off(unsigned iDDS)
{
	PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, 0);

	if (bDebugPulses)
	{
		sprintf(host->buffDebug, "%s DDS(%u) off %30s t = %8.2f us\n",  DDS_name(iDDS), iDDS, "", (double)0.5);
		host->sendDebugMsg(host->buffDebug);
	}
}

inline void DDS_set_freq(unsigned iDDS, unsigned ftw)
{
	PULSE_CONTROLLER_set_dds_freq(pulser, iDDS, ftw);

	if (bDebugPulses)
	{
		sprintf(host->buffDebug, "%s DDS(%u) f   = %09u (ftw=%09X)    t = %8.2f us\n",  DDS_name(iDDS), iDDS, FTW2Hz(ftw), ftw, (double)0.5);
		host->sendDebugMsg(host->buffDebug);
	}
}

inline void DDS_set_phase(unsigned iDDS, unsigned phase)
{
	PULSE_CONTROLLER_set_dds_phase(pulser, iDDS, phase);

	if (bDebugPulses)
	{
		sprintf(host->buffDebug, "%s DDS(%u) p   = %9.3f  degrees %9s t = %8.2f us\n",  DDS_name(iDDS), iDDS, (phase * 360.0) / (1 << 14), "", (double)0.5);
		host->sendDebugMsg(host->buffDebug);
	}
}

#endif /*DDS_PULSE_H_*/
