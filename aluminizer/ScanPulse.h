#pragma once

#include "physics.h"
#include "PulseFile.h"
#include "FrequencySource.h"

class TransitionPage;

/**

   the laser pulses for the experiment are stored as scan_pulse objects.
   scan_pulses contain the following attributes:

 *information about the transition they drive (physics::line)
 *a pointer to the TransitionPage that can recalculate the frequency and pi-time
 *dds name
 *scan type, e.g. time, frequency, phase
 *extended scan information, e.g. synchronous scan, randomize phase

   scan_pulses can insert themselves into pulse_files using the << operator (to be implemented)
   they also know how to update the FPGA during a scan as well as for preparation

 **/


class scan_pulse : public physics::line
{
public:
scan_pulse()
{
}

scan_pulse(double f,
           double t,
           const string& pulse,
           const string& scan = "",
           unsigned scan_type = DEFAULT);

scan_pulse(const physics::line& l,
           const string& scan = "",
           unsigned scan_type = DEFAULT);

scan_pulse(double mFg,
           double mFe,
           int sb,
           const string& pulse,
           double angle = M_PI,
           const string& scan = "",
           unsigned scan_type = DEFAULT);

virtual ~scan_pulse()
{
}


bool IsTScan() const
{
	return (scan_type & TSCAN) != 0;
}
bool IsFScan() const
{
	return (scan_type & FSCAN) != 0;
}
bool IsPScan() const
{
	return (scan_type & PSCAN) != 0;
}
bool IsRandPhase() const
{
	return (scan_type & RAND_PHASE) != 0;
}
bool IsSyncScan() const
{
	return (scan_type & SYNC_SCAN) != 0;
}
bool UsesSetRF() const
{
	return (IsFScan() && IsPScan()) || IsRandPhase();
}
bool IsDDSScan() const
{
	return (scan_type & NOT_DDS_SCAN) == 0;
}
bool IsAC_SynthScan() const
{
	return (scan_type & AC_SYNTH_SCAN) != 0;
}

void SetDDSScan(bool b)
{
	scan_type = b ? scan_type & ~NOT_DDS_SCAN : scan_type | NOT_DDS_SCAN;
}
void SetAC_SynthScan(bool b)
{
	scan_type = b ? scan_type | AC_SYNTH_SCAN : scan_type & ~AC_SYNTH_SCAN;
}
void SetTScan(bool b)
{
	scan_type = b ? scan_type | TSCAN : scan_type & ~TSCAN;
}
void SetFScan(bool b)
{
	scan_type = b ? scan_type | FSCAN : scan_type & ~FSCAN;
}
void SetPScan(bool b)
{
	scan_type = b ? scan_type | PSCAN : scan_type & ~PSCAN;
}
void SetRandPhase(bool b)
{
	scan_type = b ? scan_type | RAND_PHASE : scan_type & ~RAND_PHASE;
}
void SetSyncScan(bool b)
{
	scan_type = b ? scan_type | SYNC_SCAN : scan_type & ~SYNC_SCAN;
}

void SetTransitionPage(TransitionPage* p)
{
	pTransitionPage = p;
}
void UpdateFromTransitionPage(unsigned scan_type);

void SetTimeFPGA() const;
void SetTimeFPGA(double t) const;
void SetPhaseFPGA(double p) const;
void OffsetFreq(double df) const;

string scan;
string dds;

//	static auto_ptr<FrequencySource> pAC_Synth;


protected:
TransitionPage* pTransitionPage;
unsigned scan_type;

public:

const static unsigned DEFAULT;
const static unsigned TSCAN =       0x0001;
const static unsigned FSCAN =       0x0002;
const static unsigned PSCAN =       0x0004;
const static unsigned SETRF =       0x0008;
const static unsigned RAND_PHASE =     0x0010;
const static unsigned SYNC_SCAN =      0x0020;
const static unsigned NOT_DDS_SCAN =   0x0040;
const static unsigned AC_SYNTH_SCAN =  0x0080;
};

class tscan_pulse : public scan_pulse
{
public:

tscan_pulse(double t,
            const string& pulse,
            const string& scan,
            unsigned scan_type = TSCAN) :
	scan_pulse(0, t, pulse, scan, scan_type)
{
}

virtual ~tscan_pulse()
{
}
};

class ramsey_pulse_sequence : public scan_pulse
{
public:
ramsey_pulse_sequence(double f,
                      double t,
                      double RamseyT,
                      const string& pulse,
                      const string& scan = "",
                      unsigned scan_type = DEFAULT)
	:
	scan_pulse(f, t, pulse, scan, scan_type),
	RamseyT(RamseyT)
{
}

double RamseyT;
};

ostream& operator<<(ostream& o, const scan_pulse& l);
PulseFile& operator<<(PulseFile&, const scan_pulse&);
PulseFile& operator<<(PulseFile& pf, const tscan_pulse&);
PulseFile& operator<<(PulseFile& pf, const ramsey_pulse_sequence&);
