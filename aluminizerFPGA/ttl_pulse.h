#ifndef TTL_PULSE_H_
#define TTL_PULSE_H_

#ifdef CONFIG_PC
	#define XPAR_PULSE_CONTROLLER_0_BASEADDR (0)
	#include "pulse_controller.h"
#else
extern "C"
{
	#include "xparameters.h"
	#include "pulse_controller.h"
}
#endif

#include <stdio.h>
#include "shared/src/messaging.h"
#include "host_interface.h"
#include "common.h"

#ifdef WIN32
#define scalb _scalb
#define snprintf _snprintf
#endif

extern bool bDebugPulses;


#define TTL_NOTHING     (0)
#define TTL_SHUTTER     (1 << 31)
#define TTL_RAMAN_90 (0x5000000A)
#define TTL_RAMAN_CO (0x30000006)
#define TTL_START_EXP   (0x00000001)
#define TTL_3P1_V     ((1 << 26) | (1 << 23))
#define  TTL_3P1_PI     ((1 << 26) | (1 << 22))
#define TTL_3P1_SIGMA   ((1 << 26) | (1 << 21))
#define TTL_3P0          (1 << 25)
#define TTL_RF_HEAT      (1 << 27)
#define TTL_HF_RF    (1 << 23)
#define TTL_REPUMP      (0x01000010)
#define TTL_PRECOOL_MON (0x00000020)
#define TTL_DETECT_MON  (1 << 12)

#define TTL_MG_SHUTTER_CLOSE (1 << 17)
#define TTL_MG_SHUTTER_OPEN  (0)

#define TTL_MOTOR_0      (1 << 18)
#define TTL_MOTOR_1      (1 << 19)

#define TIME_UNIT    (1e-8)

inline const char* TTL_name(unsigned ttl)
{
	switch (ttl)
	{
	case TTL_NOTHING:     return "  Nothing";
	case TTL_START_EXP:   return "Start exp";
	case TTL_SHUTTER:     return "  Shutter";
	case TTL_RAMAN_90:       return " Raman 90";
	case TTL_RAMAN_CO:       return " Raman Co";
	case TTL_REPUMP:      return "   Repump";
	case TTL_3P1_PI:      return "3P1    pi";
	case TTL_3P1_SIGMA:      return "3P1 sigma";
	case TTL_3P0:         return "3P0";
	case TTL_RF_HEAT:         return "RF Heat";
	case TTL_DETECT_MON:  return " Detect_M";
	case TTL_PRECOOL_MON:    return "Precool_M";
	default:           return "  unknown";
	}
}

inline unsigned int us2TW(double t)
{
	return static_cast<unsigned int>(t * 100);
}

inline unsigned int ms2TW(double t)
{
	return us2TW(t * 1e3);
}

inline void print_pulse_info(unsigned t, unsigned ttl, const char* info = 0)
{
	if (info)
		snprintf(host->buffDebug, host->buffDebugSize(), "%s TTL(%08X) %27s t = %8.2f us (%s)\n", TTL_name(ttl), ttl, "", 0.01 * (double)t, info);
	else
		snprintf(host->buffDebug, host->buffDebugSize(), "%s TTL(%08X) %27s t = %8.2f us\n", TTL_name(ttl), ttl, "", 0.01 * (double)t);

	host->sendDebugMsg(host->buffDebug);
}


//make an RF pulse of specified frequency and duration
inline void TTL_pulse(unsigned t, unsigned ttl = 0)
{
	if (t > 4)
	{
		PULSE_CONTROLLER_pulse(pulser, t, 0, ttl);

		if (bDebugPulses)
			print_pulse_info(t, ttl);
	}
}

#endif /*TTL_PULSE_H_*/

