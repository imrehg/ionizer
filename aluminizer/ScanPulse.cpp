#ifdef PCH
#include "common.h"
#endif

#include "ScanPulse.h"
#include "TransitionPage.h"
#include "AluminizerApp.h"
#include "FPGACard.h"

using namespace std;
using namespace physics;
using namespace numerics;

const unsigned scan_pulse::DEFAULT = RAND_PHASE | FSCAN | TSCAN;

scan_pulse::scan_pulse(double f,
                       double t,
                       const string& pulse,
                       const string& scan,
                       unsigned scan_type) :
	physics::line(0, 0, 0, pulse),
	scan(scan),
	pTransitionPage(0),
	scan_type(scan_type)
{
	this->f = f;
	this->t = t;
}

scan_pulse::scan_pulse(const physics::line& l,
                       const string& scan,
                       unsigned scan_type) :
	physics::line(l),
	scan(scan),
	pTransitionPage(0),
	scan_type(scan_type)
{
}

scan_pulse::scan_pulse( double mFg,
                        double mFe,
                        int sb,
                        const string& pulse,
                        double angle,
                        const string& scan,
                        unsigned scan_type) :
	physics::line(mFg, mFe, sb, pulse, angle),
	scan(scan),
	pTransitionPage(0),
	scan_type(scan_type)
{
}

/*
   synchronous scanning:

   FSCAN:
   1. All scans marked as SYNC_SCAN and FSCAN are shifted together.
   2. All scans marked as TSCAN are updated from the transition page

   TSCAN:
   1. All scans marked as SYNC_SCAN and TSCAN which don't have a transition page are set to the scan variable
   2. All scans marked as TSCAN are updated from the transition page.
   3. All scans marked as FSCAN are update from the transition page.

 */

void scan_pulse::UpdateFromTransitionPage(unsigned scan_type)
{
	if (pTransitionPage)
	{
		pTransitionPage->CalculatePulse(*this);

		//update the frequency from the transition page if this isn't a synchronous FSCAN
		if ( !IsSyncScan() || !(scan_type & FSCAN) )
			OffsetFreq(0);

		SetTimeFPGA();
	}
}

void scan_pulse::OffsetFreq(double) const
{
	throw runtime_error("[scan_pulse::OffsetFreq] old FPGA function");
}


void scan_pulse::SetTimeFPGA(double) const
{
	throw runtime_error("[scan_pulse::SetTimeFPGA] old FPGA function");
}

void scan_pulse::SetPhaseFPGA(double) const
{
	throw runtime_error("[scan_pulse::SetPhaseFPGA] old FPGA function");
}

void scan_pulse::SetTimeFPGA() const
{
	throw runtime_error("[scan_pulse::SetTimeFPGA] old FPGA function");
}

ostream& operator<<(ostream& o, const scan_pulse& p)
{
	o << "scan_type = ";

	if ( p.IsDDSScan() )
		o << "*DDS";

	if ( p.IsAC_SynthScan() )
		o << "*AC_SYNTH*";

	if ( p.IsFScan() )
		o << "*FSCAN*";

	if ( p.IsFScan() )
		o << "*FSCAN*";

	if ( p.IsTScan() )
		o << "*TSCAN*";

	if ( p.IsPScan() )
		o << "*PSCAN*";

	if ( p.IsSyncScan() )
		o << "*SYNC_SCAN*";

	if ( p.IsRandPhase() )
		o << "*RAND_PHASE*";

	o << "  ";

	o << static_cast<physics::line>(p);

	return o;
}

PulseFile& operator<<(PulseFile& pf, const scan_pulse& p)
{
	if (p.dds.empty())
		throw runtime_error("[PulseFile << scan_pulse] Pulses must specify which DDS they use.");

	pf.InsertComment("*** Begin scan_pulse ***", 1, 0);
	pf.InsertComment(p, 0, 0);

	if (p.IsDDSScan())
		pf.InsertSetSelDDS(p.dds);

	if (p.scan.empty())
	{
		if (p.IsDDSScan())
			pf.InsertSetFrequency(p.f);

		pf.InsertPulse(p.pulse, p.t);
	}
	else
	{
		if (p.UsesSetRF() && p.IsDDSScan())
			pf.InsertSetRF(p.f, 0, p.scan);
		else if (p.IsFScan() && p.IsDDSScan())
			pf.InsertSetFrequency(p.f, p.scan);
		//** used to be in the else branch of p.UsesSetRF
		if (p.IsTScan())
			pf.InsertTSCAN(p.pulse, p.t, p.scan);
		else
			pf.InsertPulse(p.pulse, p.t, "", false); //** "" was missing
	}

	pf.InsertComment("***   End scan_pulse ***", 0, 1);

	return pf;
}

PulseFile& operator<<(PulseFile& pf, const tscan_pulse& p)
{
	pf.InsertComment("*** Begin tscan_pulse ***", 1, 0);

	pf.InsertTSCAN(p.pulse, p.t, p.scan, false);

	pf.InsertComment("***   End scan_pulse ***", 0, 1);

	return pf;
}

PulseFile& operator<<(PulseFile& pf, const ramsey_pulse_sequence& p)
{
	pf.InsertComment("*** Begin ramsey_pulse_sequence ***", 1, 0);

	//set LO frequency
	pf.InsertSetSelDDS(p.dds);
	pf.InsertSetRF(p.f, 0, p.scan);

	//first pi/2 pulse
	pf.InsertPulse(p.pulse, p.t, false);

	//Ramsey time
	pf.InsertTSCAN(p.pulse, p.RamseyT, p.scan, false);

	//second pi/2 pulse
	pf.InsertPulse(p.pulse, p.t, false);

	pf.InsertComment("***    End ramsey_pulse_sequence ***", 1, 0);

	return pf;
}
