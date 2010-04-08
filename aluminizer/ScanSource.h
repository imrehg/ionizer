#pragma once

#include "InputParametersGUI.h"

//! An adjustable signal source for scans, e.g. a frequency synthesizer, or a voltage source
/*! The experimental conditions are changed via objects of this class
 * to generate scans.  Some housingkeeping is done so that the state can be
 * returned to "normal" when this experiment is over or on hold.
 */
class ScanSource
{
public:
ScanSource(ParameterGUI_Base* pParam) : pParam(pParam), bHasNonScanningOutput(true), nonScanningOutput(0)
{
}
virtual ~ScanSource()
{
}

virtual void useFit(const std::string& scan_type, double x) = 0;

//! Should a sine function be fitted to this scan?
virtual bool fitsSine()
{
	return false;
}

//! Should a Lorentzian be fitted to this scan?
virtual bool fitsLorentzian()
{
	return true;
}

void setDefaultValue();
std::string getXlabel();

//! Set the "non-scanning" default value of this parameter.
/*! Doesn't this clash with scan_variable::default_state ?
 *  Are we keeping track of the "same" information in two places ?
 *  TODO: check for potential bug.
 */
void setNonScanningValue(double d);

//! Return "non-scanning" default value of this parameter.
double getNonScanningValue();

virtual void SetScanOutput(double d);

virtual double GetOutput() = 0;
virtual double GetMin() = 0;
virtual double GetMax() = 0;
virtual std::string getName() = 0;
virtual std::string getType() = 0;
virtual std::string GetUnit() = 0;
virtual unsigned GetMode()
{
	return 2;
}

virtual bool isCompatibleScanType(const std::string&) = 0;

ParameterGUI_Base* pParam;

protected:
bool bHasNonScanningOutput;
double nonScanningOutput;
};

class FPGA_connection;

//! Base class for scan sources that are FPGA parameters
class FPGAScanSource : public ScanSource
{
public:
FPGAScanSource(FPGA_connection* pFPGA, unsigned page_id, ParameterGUI_Base* pParam) :
	ScanSource(pParam),
	pFPGA(pFPGA), page_id(page_id)
{
}

virtual ~FPGAScanSource()
{
}

protected:
FPGA_connection* pFPGA;
unsigned page_id;
};

//! Base class for scan sources that are FPGA pulse parameters
class PulseScanSource : public FPGAScanSource
{
public:
PulseScanSource(FPGA_connection* pFPGA, unsigned page_id, ParameterGUI_Base* pParam) :
	FPGAScanSource(pFPGA, page_id, pParam)
{
}

virtual ~PulseScanSource()
{
}

//! Return pulse time (non-scanning)
virtual double getPulseTime() = 0;
};

class DoubleScanSource : public FPGAScanSource
{
public:
DoubleScanSource(FPGA_connection* pFPGA, unsigned page_id, GUI_double* pDouble) :
	FPGAScanSource(pFPGA, page_id, pDouble),
	pDouble(pDouble)
{
}

//! Should a sine wave be fitted to this scan?
virtual bool fitsSine()
{
	return getName().find("phase") != std::string::npos;
}

//! Should a Lorentzian be fitted to this scan?
virtual bool fitsLorentzian()
{
	return !fitsSine();
}

virtual ~DoubleScanSource()
{
}

virtual void SetScanOutput(double d);
virtual double GetOutput();
virtual double GetMin();
virtual double GetMax();
virtual std::string getName();
virtual std::string getType()
{
	return "General";
}
virtual std::string GetUnit();
virtual bool isCompatibleScanType(const std::string&);

virtual void useFit(const std::string& scan_type, double x)
{
	if (isCompatibleScanType(scan_type) && pDouble)
		pDouble->SetValue(x);
}

protected:
GUI_double* pDouble;
};

//! Connects to freq. pulse parameters on FPGA
class FScanSource : public PulseScanSource
{
public:
FScanSource(FPGA_connection* pFPGA, unsigned page_id, GUI_dds* pPulse) :
	PulseScanSource(pFPGA, page_id, pPulse),
	pPulse(pPulse)
{
}

virtual ~FScanSource()
{
}

//!Should a Lorentzian be fitted to this scan?
virtual bool fitsLorentzian()
{
	return getName().find("Detect") != std::string::npos;
}

virtual void SetScanOutput(double d);
virtual double GetOutput();
virtual double GetMin();
virtual double GetMax();
virtual std::string getName();
virtual std::string getType()
{
	return "Frequency";
}
virtual std::string GetUnit();
virtual unsigned GetMode()
{
	return 1;
}

virtual bool isCompatibleScanType(const std::string&);

//!Return pulse time (non-scanning)
virtual double getPulseTime();

//!Return pulse freq. (non-scanning)
virtual double getPulseFreq();

virtual void useFit(const std::string& scan_type, double x);

protected:
GUI_dds* pPulse;
};


//! Connects to time pulse parameters on FPGA
class TScanSource : public PulseScanSource
{
public:
TScanSource(FPGA_connection* pFPGA, unsigned page_id, GUI_dds* pPulseDDS) :
	PulseScanSource(pFPGA, page_id, pPulseDDS),
	pPulseDDS(pPulseDDS),
	pPulseTTL(0)
{
}

TScanSource(FPGA_connection* pFPGA, unsigned page_id, GUI_ttl* pPulseTTL) :
	PulseScanSource(pFPGA, page_id, pPulseTTL),
	pPulseDDS(0),
	pPulseTTL(pPulseTTL)
{
}

virtual ~TScanSource()
{
}

//! Should a Lorentzian be fitted to this scan?
virtual bool fitsLorentzian()
{
	return false;
}

virtual void SetScanOutput(double d);
virtual double GetOutput();
virtual double GetMin();
virtual double GetMax();
virtual std::string getName();
virtual std::string getType()
{
	return "Time";
}

virtual std::string GetUnit();
virtual bool isCompatibleScanType(const std::string&);

virtual double getPulseTime();

virtual void useFit(const std::string& scan_type, double x);

protected:
GUI_dds* pPulseDDS;
GUI_ttl* pPulseTTL;
};


extern std::vector< ScanSource* > g_scan_sources;

