#ifndef REMOTE_PARAMS_H_
#define REMOTE_PARAMS_H_

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "dds_pulse_info.h"
#include "dds_pulse.h"
//#include "dac.h"
#include "string_func.h"
#include "my_matrix.h"
#include "messaging.h"

//! base class for parameters that get updated remotely
class remote_param
{
public:
remote_param(unsigned type, const std::string& name,
             std::vector<remote_param*>* v,
             const std::string& init_str) :
	type(type),
	name(name),
	init_str(init_str),
	flags(extract_flags(init_str))
{
	if (v)
		v->push_back(this);
}

virtual ~remote_param()
{
}

bool hasFlag(unsigned u)
{
	return (flags & u) != 0;
}
void setFlag(unsigned u)
{
	flags = flags | u;
}
void unsetFlag(unsigned u)
{
	flags = flags & (~u);
}

virtual void update_binary(unsigned, const char*)
{
	throw std::runtime_error("remote_param::update_binary not implemented");
}

//!update the GUI through communications link
void updateGUI(unsigned page_id);

virtual void setValueFromString(const char* s) = 0;
virtual void defineAsString(char* s) = 0;

const std::string& getName() const
{
	return name;
}
virtual int getValueString(char* s, size_t n) = 0;

void setExplanation(const std::string& s)
{
	explanation = s;
}
const std::string& getExplanation() const
{
	return explanation;
}

//! copy paramater explanation (e.g. tool-tip) into s
void explain(char* s, unsigned len)
{
	strncpy(s, explanation.c_str(), len);
}
//	const unsigned get_type() const { return type; }

protected:
const unsigned type;
const std::string name;
std::string init_str, explanation;
unsigned flags;
};

template <class T, unsigned type_id> class rp_simple : public remote_param
{
public:
rp_simple(const std::string& name, std::vector<remote_param*>* v, const std::string& init_str) :
	remote_param(type_id, name, v, init_str)
{
}

virtual ~rp_simple()
{
}

virtual void setValueFromString(const char* s)
{
	if (hasFlag(RP_FLAG_DEBUG))
		printf("[%s::setValueFromString] %s\r\n", name.c_str(), s);

	value = from_string<T>(s);
}

void defineAsString(char* s)
{
	sprintf(s, "type=%u %s flags=%u name=%s", type_id, init_str.c_str(), flags, name.c_str());
}

virtual void set(const T& v)
{
	value = v;

	if (hasFlag(RP_FLAG_DEBUG))
	{
		char buff[128];
		getValueString(buff, 128);
		printf("[%s::set] %s\r\n", name.c_str(), buff);
	}
}

const T& get_value() const
{
	return value;
}
operator const T& () const { return value; }

int getValueString(char* s, size_t n)
{
	int i = to_string<T>(value, s, n);

	if (hasFlag(RP_FLAG_DEBUG))
		printf("[%s::getValueString] %s\r\n", name.c_str(), s);

	return i;
}

protected:
T value;
};

typedef rp_simple<unsigned, RP_UNSIGNED>  rp_unsigned;
typedef rp_simple<int,     RP_INT>        rp_int;
typedef rp_simple<double,  RP_DOUBLE>     rp_double;
typedef rp_simple<double,  RP_LCD>        rp_lcd;
typedef rp_simple<bool,    RP_BOOL>    rp_bool;
typedef rp_simple<std::string,   RP_STRING>     rp_string;

class rp_matrix : public remote_param
{
public:
rp_matrix(unsigned nRows, unsigned nCol, const std::string& name,
          std::vector<remote_param*>* v,
          const std::string& init_str = "") :
	remote_param(RP_MATRIX, name, v, init_str),
	value(nRows, nCol),
	inverse(nRows, nCol)
{
	setFlag(RP_FLAG_BINARY_XFER);
}

virtual void setValueFromString(const char* s)
{
	value = from_string<my_matrix>(s);
}
int getValueString(char* s, size_t n)
{
	return to_string<my_matrix>(value, s, n);
}

void updateInverse();

virtual void update_binary(unsigned length, const char* bin);

void defineAsString(char* s)
{
	sprintf(s, "type=%u %s nr=%u nc=%u flags=%u name=%s", type, init_str.c_str(), value.nr, value.nc, flags, name.c_str());
}

my_matrix value;
my_matrix inverse;
};

class ttl_params : public remote_param
{
public:
ttl_params(unsigned ttl, const std::string& name,
           std::vector<remote_param*>* v,
           const std::string& init_str = "t=1") :
	remote_param(RP_TTL_PULSE, name, v, init_str),
	ttl(ttl)
{
}

void defineAsString(char* s)
{
	sprintf(s, "type=%u ttl=%u %s  flags=%u name=%s", type, ttl, init_str.c_str(), flags, name.c_str());
}

virtual void setValueFromString(const char* s)
{
	value.updateFromString(s);
	t = us2TW(value.t);
	bEnabled = value.isEnabled();
}

inline void pulse()
{
	if (bEnabled)
		TTL_pulse(t, ttl);
}

int getValueString(char* s, size_t n)
{
	return value.to_string(s, n);
}

unsigned t, ttl;
bool bEnabled;

ttl_pulse_info value;
};


class dds_params : public remote_param
{
public:
dds_params(unsigned dds, const std::string& name,
           std::vector<remote_param*>* v,
           unsigned ttl = 0, const std::string& init_str = "t=1 fOn=200 fOff=0",
           int fDiv = 1, double fOffset = 0, bool bUseFTWoff = true) :
	remote_param(RP_DDS_PULSE, name, v, init_str),
	dds(dds),
	ttl(ttl),
	fDiv(fDiv),
	fOffset(fOffset),
	bUseFTWoff(bUseFTWoff),
	bRamsey(false),
	bX90Y180X90(false),
	ramseyTTL(0),
	padding(5),
	ramseyPhase(0)
{
}

void setX90Y180X90(bool b)
{
	bX90Y180X90 = b;
}


void defineAsString(char* s)
{
	sprintf(s, "type=%u dds=%u %s flags=%u name=%s", type, dds, init_str.c_str(), flags, name.c_str());
}

void setValueFromString(const char* s)
{
	value.updateFromString(s);

	t     = us2TW(value.t);
	ftwOn = MHz2FTW((value.fOn - fOffset) / fDiv);
	ftwOff  = MHz2FTW((value.fOff - fOffset) / fDiv);
	bEnabled = value.isEnabled();
}

double get_time()
{
	return value.t * 1e-6;
}
double get_freq()
{
	return value.fOn * 1e6;
}
unsigned get_ftw()
{
	return ftwOn;
}
void set_ftw(unsigned ftw)
{
	ftwOn = ftw;
}

int getValueString(char* s, size_t n)
{
	return value.to_string(s, n);
}

void setRamsey(ttl_params* ramsey)
{
	bRamsey = ramsey->bEnabled;
	ramseyTTL = ramsey;
}

unsigned shift_fOn(double dfHz)
{
	unsigned ftwOld = ftwOn;

	ftwOn = MHz2FTW((value.fOn - fOffset + dfHz * 1e-6) / fDiv);

	return ftwOld;
}

//freq. shifted pulse (in Hz)
void shifted_pulse(double dfHz)
{
	unsigned ftwOld = ftwOn;

	ftwOn = MHz2FTW((value.fOn - fOffset + dfHz * 1e-6) / fDiv);
	pulse();

	ftwOn = ftwOld;
}

void pulse()
{
	//TODO: fix this with a flag for fOn/fOff switching
	if (bEnabled)
	{
		if (bDebugPulses)
		{
			host->sendDebugMsg(" *** ");
			host->sendDebugMsg(name.c_str());
			host->sendDebugMsg("\n");
		}

		if (bRamsey)
		{
			ramsey_pulse(ramseyTTL, ramseyPhase);
			return;
		}

		if (bUseFTWoff)
			DDS_long_pulse(dds, ftwOn, ftwOff, t, ttl);
		else
		{
			if (bX90Y180X90)
				pulse_X90_Y180_X90();
			else
				simple_pulse();
		}
	}
}

inline void simple_pulse()
{
	if (bEnabled)
	{
		TTL_pulse(padding, 0);
		DDS_set_freq(dds, ftwOn);
		TTL_pulse(padding, 0);
		TTL_pulse(t, ttl);
		TTL_pulse(padding, 0);
	}
}

void pulse_X90_Y180_X90()
{
	if (bEnabled)
	{
		DDS_set_phase(dds, PHASE_0);
		DDS_set_freq(dds, ftwOn);
		TTL_pulse(padding, 0);
		TTL_pulse((t >> 1), ttl);     //X-90
		DDS_set_phase(dds, (PHASE_90 / fDiv));
		TTL_pulse(t, ttl);            //Y-180
		DDS_set_phase(dds, PHASE_0);
		TTL_pulse((t >> 1), ttl);     //X-90
	}
}

inline void randomizePhase()
{
	if (bEnabled)
		DDS_set_phase(dds, rand() % 16384);
}

inline void SetPhase(double phase)                             //to set phase for the pulse - CWC 02032009
{
	while(phase < 0)
		phase += 360;
		
	while(phase > 360)
		phase -= 360;
	
	unsigned ptw = static_cast<unsigned>(phase * 45.5111111111 + 0.5);     // phase tuning word

	DDS_set_phase(dds, ptw);
}

void ramsey_pulse(ttl_params* ramsey, double phase /*degrees*/)
{
	//TODO: fix this with a flag for fOn/fOff switching
	if (bEnabled)
	{
		if (bUseFTWoff)
		{
			//first pi/2 pulse
			DDS_pulse(dds, ftwOn, ftwOff, t >> 1, ttl);

			//free evolution
			ramsey->pulse();

			//second pi/2 pulse
			DDS_pulse(dds, ftwOn, ftwOff, t >> 1, ttl);
		}
		else
		{
			//first pi/2 pulse
			//	TTL_pulse(padding, 0);
			DDS_set_freq(dds, ftwOn);
			DDS_set_phase(dds, 0);
			//	TTL_pulse(padding, 0);
			TTL_pulse((t >> 1), ttl);

			//free evolution
			ramsey->pulse();

			//second pi/2 pulse
			SetPhase(phase / fDiv);
			
			//	TTL_pulse(100, 0);
			TTL_pulse((t >> 1), ttl);
			//	TTL_pulse(100, 0);
			//	TTL_pulse(padding, 0);
		}
	}
}

//! stretch pulse time by (stretch / 1000) factor
inline void stretched_pulse(unsigned stretch)
{
	if (bEnabled)
	{
		TTL_pulse(padding, 0);
		DDS_set_freq(dds, ftwOn);
		TTL_pulse(padding, 0);
		TTL_pulse((stretch * t) / 1000, ttl);
		TTL_pulse(padding, 0);
	}
}

inline void set_freq()
{
	DDS_set_freq(dds, ftwOn);
}

inline void pulse_ttl_only()
{
	if (t > 4)
		TTL_pulse(t, ttl);
}

inline void detection_pulse()
{
	::detection_pulse(ftwOn, ftwOff, t);
}

inline void pulseStayOn()
{
	if (bEnabled)
	{
		if (ttl == 0)
			DDS_pulse(dds, ftwOn, ftwOn, t);
		else
		{
			DDS_set_freq(dds, ftwOn);
			TTL_pulse(t, ttl);
		}
	}
}

inline void ddsOff()
{
	DDS_off(dds);
}


unsigned dds;

dds_pulse_info value;

unsigned t, ftwOn, ftwOff, ttl;
int fDiv;

double fOffset;
bool bUseFTWoff;
bool bEnabled;
bool bRamsey;
bool bX90Y180X90;
ttl_params* ramseyTTL;
unsigned padding;
double ramseyPhase;
};

class raman_pulse : public dds_params
{
public:
raman_pulse(const std::string& name,
            std::vector<remote_param*>* v,
            unsigned ttl = 0, const std::string& init_str = "t=1 fOn=1789 fOff=0") :
	dds_params(DDS_RAMAN, name, v, ttl, init_str, 4, 2 * 300, false)
{
}

void setPolarization(int p)
{
	ttl = (p == 0 ? TTL_RAMAN_CO : TTL_RAMAN_90);
}
};

class Al3P1_pulse : public dds_params
{
public:
Al3P1_pulse(const std::string& name,
            std::vector<remote_param*>* v,
            unsigned ttl = 0, const std::string& init_str = "t=1 fOn=120 fOff=0") :
	dds_params(DDS_3P1x2, name, v, ttl, init_str, -2, 240, false)
{
}

void set_port(unsigned p)
{
	//set TTL for correct port
	switch (p)
	{
	case 0: ttl = TTL_3P1_SIGMA; break;
	case 1: ttl = TTL_3P1_PI; break;
	case 2: ttl = TTL_3P1_V; break;
	default: throw std::runtime_error("[Al3P1_pulse::set_port] unknown port: " + to_string<int>(p));
	}
	;
}
};

class FluorescenceChecker;

//3P0 pulse is special, because it allows fluorescence detection during the pulse
//we want this to monitor the intensity of the cooling beam, and also to recognise if
//the ions have gone dark, so we can abort the pulse & try to recover them
class Al3P0_pulse : public dds_params
{
public:
Al3P0_pulse(const std::string& name,
            std::vector<remote_param*>* v,
            const std::string& init_str = "t=1000 fOn=0 fOff=0 unit=1") :
	dds_params(DDS_3P0, name, v, TTL_3P0, init_str, -1, 217, false)
{
}

//checkedPulse: make a pulse and check fluorescence at the same time
//returns true if fluorescence was ok
//false if pulse was aborted due to low fluorescence
//stores the total number of PMT counts in nCounts
bool checked_pulse(FluorescenceChecker* fc, unsigned* nCounts);

//! make a 3P0 pulse with light from both ports on simultaneously
void bi_directional_pulse(double dF0, double dF1);

void set_port(unsigned p)
{
	//set TTL for correct port
	switch (p)
	{
	case 0: dds = DDS_3P0;    ttl = TTL_3P0;    bUseFTWoff = false; ftwOff = 0; break;
	case 1: dds = DDS_3P0_BR; ttl = 0; bUseFTWoff = true; ftwOff = 0; break;
	case 2: break;
	default: throw std::runtime_error("[Al3P0_pulse::set_port] unknown port: " + to_string<int>(p));
	}
	;
}
};

class RF_pulse : public dds_params
{
public:
RF_pulse(const std::string& name,
         std::vector<remote_param*>* v,
         const std::string& init_str = "t=1 fOn=1789 fOff=1780") :
	dds_params(DDS_RF, name, v, TTL_HF_RF, init_str, 8, 0, true)
{
}
};


inline void pulse_dds(dds_params* d)
{
	d->pulse();
}
inline void pulse_ttl(ttl_params* t)
{
	t->pulse();
}

extern unsigned g_debug_level;

#endif /*REMOTE_PARAMS_H_*/

