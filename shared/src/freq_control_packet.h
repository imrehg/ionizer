#pragma once

class rfs_state
{
public:
rfs_state() : freq_or_delta(0), ramp_rate(0), ramp_window(0), ramp_time_const(0)
{
}
double freq_or_delta, ramp_rate, ramp_window, ramp_time_const;
};

struct freq_control_packet
{
	enum packet_type { SHIFT_FREQ, SET_STATE, GET_STATE, CLEAR_HISTORY, NEW_ERR_SIG };

	freq_control_packet()
	{
	}
	freq_control_packet(rfs_state, packet_type type);

	std::string description();

	int id;
	packet_type type;
	rfs_state rfs;

	double gainP;
	double gainI;
	double gainDrift;

	double tSent;
	double tReceived;
	double tApplied;
};

std::ostream& operator<<(std::ostream& o, const freq_control_packet& c);
