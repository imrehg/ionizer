#ifdef PCH
#include "common.h"
#endif

#include "AluminizerApp.h"
#include "CommonExperimentsSheet.h"

#include "PulseFile.h"

using namespace std;

const double PulseFile::BRACE_DELAY = 0.2;
const double PulseFile::SET_RF_DELAY = 1.0;
const double PulseFile::SET_FREQ_PADDING = PulseFile::SET_RF_DELAY;
const double PulseFile::SET_DDS_PADDING = 1.0;


PulseFile::PulseFile(const std::string& file_name) :
	file(file_name.c_str()),
	run_time(0),
	post_set_freq_padding(10)
{
	if (!file.is_open())
		throw runtime_error("[PulseFile::PulseFile] failed to open " + file_name);

	struct tm *tmt;
	time_t tt;

	time( &tt );               // Get time in seconds
	tmt = localtime( &tt );    // Convert time to struct tm form

	file << "// Created: " << asctime(tmt) << endl;
}


PulseFile::~PulseFile()
{
	if (file.is_open())
		file << "// run time: " << run_time << " us" << endl;

}

void PulseFile::InsertSetFrequency(const std::string& f, double delay)
{
	InsertPulseInternal("nopulse", delay);
	file << " setfreq(" << fixed << setprecision(6) << f << ");" << endl;
}

void PulseFile::InsertSetFrequency(double f, const std::string& scan, double delay)
{
	if (!scan.empty())
		InsertFSCAN(f * 1e-6, scan, "nopulse", delay);
	else
	{
		ostringstream oss;
		oss << fixed << setprecision(6) << f * 1e-6;
		InsertSetFrequency(oss.str(), delay);
	}
}

void PulseFile::InsertSetRF(const std::string& f, const std::string& p)
{
	InsertPulseInternal("nopulse", SET_FREQ_PADDING);
	file << " setrf(" << fixed << setprecision(6) << f << ", " << fixed << setprecision(6) << p << ");" << endl;
}

void PulseFile::InsertSetRF(double f, double p, const std::string& scan)
{
	if (!scan.empty())
		InsertFPSCAN(f * 1e-6, scan);
	else
	{
		ostringstream ossf;
		ostringstream ossp;
		ossf << fixed << setprecision(6) << f * 1e-6;
		ossp << fixed << setprecision(6) << p;
		InsertSetRF(ossf.str(), ossp.str());
	}
}

void PulseFile::InsertSetPhase(double p)
{
	InsertPulseInternal("nopulse", SET_FREQ_PADDING);
	file << " setphase(" << fixed << setprecision(6) << p << ");" << endl;
}

void PulseFile::InsertTSCAN(const std::string& pulse, double t0, const std::string& name, bool bPadded)
{
	InsertPulseInternal(pulse, t0);
	file << " tscan(" << name << ");" << endl;

	if (bPadded)
		InsertPadding(2.0);
}

void PulseFile::InsertFSCAN(double f0, const std::string& name, const std::string& pulse, double t)
{
	InsertPulseInternal(pulse, t);
	file << " setfreq(" << fixed << setprecision(6) << f0 << ")" << " fscan(" << name << ");" << endl;
	if (post_set_freq_padding)
		InsertPadding(post_set_freq_padding);
}

void PulseFile::InsertFPSCAN(double f0, const std::string& name, const std::string& pulse, double t)
{
	InsertPulseInternal(pulse, t);
	file << " setrf(" << fixed << setprecision(6) << f0 << ", 0) " << "fscan(" << name << ");" << endl;
	if (post_set_freq_padding)
		InsertPadding(post_set_freq_padding);
}

void PulseFile::InsertFPTSCAN(double f0, const std::string& name, const std::string& pulse, double t)
{
	InsertPulseInternal(pulse, t);
	file <<  " setrf(" << fixed << setprecision(6) << f0 << ", 0) fscan(" << name << ") tscan(" << name << ");" << endl;
}

void PulseFile::InsertPulse(const std::string& name, double length, const std::string& scan, bool bPadded)
{
	if (!scan.empty())
		InsertTSCAN(name, length, scan);
	else
	{
		InsertPulseInternal(name, length);
		file << ";" << endl;

		if (bPadded)
			InsertPadding(SET_RF_DELAY);
	}
}

void PulseFile::InsertSetSelDDS(const std::string& name)
{
	if (name != current_dds)
	{
		InsertPulseInternal("nopulse", SET_DDS_PADDING);
		file << " setsel(" << name << ");" << endl;
		current_dds = name;
	}
}

void PulseFile::InsertPadding(double length)
{
	InsertPulseInternal("nopulse", length);
	file << ";" << endl;
}

void PulseFile::InsertPulseInternal(const std::string& pulse, double t)
{
	run_time += t;

	file << "pulse " << setw(12) << pulse << " " << fixed << setprecision(6) << setw(12) << t;
}

void PulseFile::InsertSetVariable(const std::string& name, double value)
{
	file << fixed << setprecision(6);
	file << "var " << setw(25) << name << " = " << value << ";" << endl;
}

void PulseFile::InsertSaveIon(unsigned PMT)
{
	file << "block repeat(10){" << endl;
	file << "if ( pmt( " << PMT << ")<1) {" << endl;
	file << "   pulse	precool		1000;"<< endl;
	PulseFile::InsertDetect(PMT);

	file << "} }" << endl;
	file << "pulse	nopulse		0.1;"<< endl;
}

void PulseFile::InsertDopplerCool(double length)
{
	InsertSetSelDDS("BDDDS");
	InsertSetFrequency("BDcoolfreq");
	InsertPulse("cool", length);

	if (theApp->m_pExperimentsSheet->m_GlobalsPage.DDS0Hz)
	{
		file << "//set detect DDS to 0 Hz to avoid Stark shifts" << endl;
		InsertSetFrequency(0);
	}
}

void PulseFile::InsertDetect(unsigned PMT, bool bCool)
{
	if (theApp->m_pExperimentsSheet->m_GlobalsPage.DDS0Hz)
	{
		file << "//set raman DDS to 0 Hz to avoid stray light during detection" << endl;
		InsertSetSelDDS("RamanDDS");
		InsertSetFrequency(0);
	}

	InsertSetSelDDS("BDDDS");
	InsertSetFrequency(bCool ? "BDcoolfreq" : "BDdetectfreq");

	ostringstream detect;
	detect << "detect" << PMT;
	InsertPulse(detect.str(), 200);

	if (theApp->m_pExperimentsSheet->m_GlobalsPage.DDS0Hz)
	{
		file << "//set detect DDS to 0 Hz to avoid Stark shifts" << endl;
		InsertSetFrequency(0);
	}

	if (theApp->m_pExperimentsSheet->m_GlobalsPage.DDS0Hz)
	{
		file << "//set raman DDS back to 216.6 MHz for intensity control" << endl;
		InsertSetSelDDS("RamanDDS");
		InsertSetFrequency(216.6e6);
	}

	InsertPadding(2.0);
}

void PulseFile::InsertPrecoolAl(unsigned nAl)
{
	switch (nAl)
	{
	case 1: run_time += 610; InsertInclude("precoolAl.dc"); break;
	case 2: run_time += 1410; InsertInclude("precoolAl2.dc"); break;
	default: throw runtime_error("[PulseFile::InsertPrecoolAl] Don't know how to precool nAl = " + to_string(nAl));
	}
}

void PulseFile::InsertInclude(const std::string& name)
{
	InsertPadding(BRACE_DELAY);

	file << "#include \"" << name << "\"" << endl;

	current_dds = "";

	InsertPadding(BRACE_DELAY);
}

void PulseFile::InsertRepump()
{
	InsertFunction("repump", 5);
}

void PulseFile::InsertBeginRepeat(unsigned n)
{
	InsertPadding(BRACE_DELAY);
	file << "block repeat(" << n << ")" << endl;
	file << "{" << endl;
	InsertPadding(BRACE_DELAY);
}

void PulseFile::InsertEndRepeat()
{
	InsertPadding(BRACE_DELAY);
	file << "}" << endl;
	InsertPadding(BRACE_DELAY);
}

void PulseFile::InsertFunction(const std::string& f, double t)
{
	run_time += t;

	InsertPadding(BRACE_DELAY);

	file << f << "();" << endl;

	InsertPadding(BRACE_DELAY);

	if (f.find("raman") != string::npos)
		current_dds = "RamanDDS";
	else
		current_dds = "";
}

void PulseFile::InsertSetName(const std::string& name, const std::string& value)
{
	file << "name " << name << " = " << value << ";" << endl;
}

void PulseFile::InsertBeginIf(int pmt, int threshold, const std::string& op)
{
	file << "if ( pmt(" << pmt << ") " << op << " " << threshold << " )" << endl;
	file << "{" << endl;
}

void PulseFile::InsertElse()
{
	file << "}" << endl;
	file << "else" << endl;
	file << "{" << endl;
}

void PulseFile::InsertEndIf()
{
	file << "}" << endl;
}
