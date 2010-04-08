#pragma once


class PulseFile
{
public:
PulseFile(const std::string& file_name);

virtual ~PulseFile();

//	void InsertSectionLabelBegin(const std::string& label);
//	void InsertSectionLabelEnd(const std::string& label);

void InsertBeginIf(int pmt, int threshold, const std::string& op);
void InsertElse();
void InsertEndIf();

//Insert a setfrequency pulse into os for frequency f in Hz
void InsertSetFrequency(double f, const std::string& scan = "", double delay = SET_RF_DELAY);
void InsertSetFrequency(const std::string& f, double delay = SET_RF_DELAY);

//Insert a setrf pulse into os for frequency f in Hz and phase p in rad
void InsertSetRF(double f, double p, const std::string& scan = "");
void InsertSetRF(const std::string& f, const std::string& p);

void InsertSetPhase(double p);

//Insert a pulse of given name and length
void InsertPulse(const std::string& name, double length, const std::string& scan = "", bool bPadded = true);
void InsertPadding(double length);

void InsertTSCAN(const std::string& pulse, double t0, const std::string& name, bool bPadded = true);
void InsertFSCAN(double f0, const std::string& name, const std::string& pulse = "nopulse", double t = SET_RF_DELAY);
void InsertFPSCAN(double f0, const std::string& name, const std::string& pulse = "nopulse", double t = SET_RF_DELAY);
void InsertFPTSCAN(double f0, const std::string& name, const std::string& pulse = "nopulse", double t = SET_RF_DELAY);


void InsertSetVariable(const std::string& name, double value);
void InsertSetName(const std::string& name, const std::string& value);

void InsertSaveIon(unsigned PMT);

//detection at BD detect freq, unless bCool is true
void InsertDetect(unsigned PMT, bool bCool = false);

void InsertPrecoolAl(unsigned nAl = 1);
void InsertDopplerCool(double length);

void InsertRepump();

void InsertSetSelDDS(const std::string& name);
void InsertInclude(const std::string& name);

void InsertBeginRepeat(unsigned n);
void InsertEndRepeat();

template<class T> void InsertComment(const T& t, int lines_before = 1, int lines_after = 1)
{
	for (int i = 0; i < lines_before; i++)
		file << endl;

	file << "// " << t << endl;

	for (int i = 0; i < lines_after; i++)
		file << endl;
}

//call spec'd function.  set t to the run-time if known to maintain accurate run-time total.
void InsertFunction(const std::string&, double t = 0);

double GetRunTime() const
{
	return run_time;
}
void IncrementRunTime(double dt)
{
	run_time += dt;
}

class CommentSection
{
public:
CommentSection(const std::string& label, PulseFile& pf) : pf(pf), label(label)
{
	pf.file << endl << endl << "//**************************** BEGIN:    " << label << "   *****************************" << endl;
}

~CommentSection()
{
	pf.file << "//****************************   END:    " << label << "   *****************************" << endl << endl << endl;
}

PulseFile& pf;
std::string label;
};

class RepeatSection
{
public:
RepeatSection(unsigned nReps, PulseFile& pf) : pf(pf), nReps(nReps)
{
	tStart = pf.GetRunTime();

	if (nReps > 0)
	{
		pf.InsertPadding(BRACE_DELAY);
		pf.file << "block repeat(" << nReps << ")" << endl << "{" << endl;
		pf.InsertPadding(BRACE_DELAY);
	}
}

~RepeatSection()
{
	if (nReps > 0)
	{
		pf.InsertPadding(BRACE_DELAY);
		pf.file << "}" << endl;
		pf.InsertPadding(BRACE_DELAY);

		double dt = pf.GetRunTime() - tStart;
		pf.IncrementRunTime(dt * (nReps - 1));
	}
}

protected:
PulseFile& pf;
unsigned nReps;
double tStart;
};

class FunctionDefinition
{
public:
FunctionDefinition(const std::string& name, PulseFile& pf) : pf(pf)
{
	pf.InsertPadding(BRACE_DELAY);
	pf.file << "function " << name << "()" << endl << "{" << endl;
	pf.InsertPadding(BRACE_DELAY);
}

~FunctionDefinition()
{
	pf.InsertPadding(BRACE_DELAY); pf.file << "}" << endl;
}

PulseFile& pf;
};

private:
void InsertPulseInternal(const std::string& pulse, double t);

private:
std::string current_dds;
std::ofstream file;

const static double BRACE_DELAY;
const static double SET_RF_DELAY;
const static double SET_FREQ_PADDING;
const static double SET_DDS_PADDING;

double run_time;

public:
double post_set_freq_padding;
};

