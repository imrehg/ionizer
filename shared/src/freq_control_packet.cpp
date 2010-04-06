#include "common.h"

#include "freq_control_packet.h"

#include <trlib.h>

using namespace std;

freq_control_packet::freq_control_packet(rfs_state rfs, packet_type type) :
	id(0),
	type(type),
	rfs(rfs),
	tSent(CurrentTime_s()),
	tReceived(0),
	tApplied(0)
{
}

std::string freq_control_packet::description()
{
	switch (type)
	{
	case (SET_STATE):  return "SET_STATE ";
	case (GET_STATE):  return "GET_STATE ";
	case (SHIFT_FREQ):  return "SHIFT_FREQ";
	case (CLEAR_HISTORY):   return "CLEAR_HIST";
	case (NEW_ERR_SIG):      return "NEW_ERR_SIG";
	default:             return "UNKNOWN";
	}
}


ostream& operator<<(ostream& o, const freq_control_packet& p)
{
	o << "type = " << p.type << ", " << setprecision(3) << fixed;
	o << "freq/delta = " << p.rfs.freq_or_delta << ", ";
	o << "rate = " << p.rfs.ramp_rate << ", ";
	o << "t_const = " << p.rfs.ramp_time_const << ", ";
	o << "window = " << p.rfs.ramp_window << ", ";
	o << "tSent = " << p.tSent;

	return o;
}
