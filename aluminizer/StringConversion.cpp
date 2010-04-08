#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

//FPGA and AluminizerSim need stringable.h
#if (ALUMINIZER_SIM || ALUMINIZER_ES)
#include "stringable.h"
#endif

#include "dds_pulse_info.h"

template<> dds_pulse_info from_string(const std::string& s)
{
	return dds_pulse_info(s);
}

template<> ttl_pulse_info from_string(const std::string& s)
{
	return ttl_pulse_info(s);
}

template<> std::string to_string(const dds_pulse_info& value, int)
{
	char s[256];

	value.to_string(s, 256);
	return std::string(s);
}

template<> std::string to_string(const ttl_pulse_info& value, int)
{
	char s[256];

	value.to_string(s, 256);
	return std::string(s);
}
