#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ScanSource.h"
#include "InputParametersGUI.h"
#include "FPGA_connection.h"

std::vector< ScanSource* > g_scan_sources;

void ScanSource::setNonScanningValue(double d)
{
	bHasNonScanningOutput = false;
	nonScanningOutput = d;

	SetScanOutput(d);
}

double ScanSource::getNonScanningValue()
{
	if (bHasNonScanningOutput)
		return GetOutput();
	else
		return nonScanningOutput;
}

void ScanSource::SetScanOutput(double d)
{
	if (bHasNonScanningOutput)
		nonScanningOutput = GetOutput();

	bHasNonScanningOutput = (nonScanningOutput == d);
}

void ScanSource::setDefaultValue()
{
	SetScanOutput(nonScanningOutput);
//crashes here when stopping -- called from if(IsFinished()) block in DefaultExperimentState
//pParam == 0 (todo)

	if (pParam) //quick-fix
	{
		pParam->set_revision(-1);
		pParam->set_gui_revision(-2);
	}
}

std::string ScanSource::getXlabel()
{
	string s = getName();

	if (pParam)
		s = pParam->GetName();

	return s + " [" + GetUnit() + "]";
}

void DoubleScanSource::SetScanOutput(double d)
{
	ScanSource::SetScanOutput(d);

	pDouble->SetValue(d);
	pFPGA->SendParam(page_id, pDouble);
}

double DoubleScanSource::GetOutput()
{
	return pDouble->Value();
}

double DoubleScanSource::GetMin()
{
	return pDouble->bottom;
}

double DoubleScanSource::GetMax()
{
	return pDouble->top;
}

std::string DoubleScanSource::getName()
{
	return pDouble->get_fpga_name();
}

std::string DoubleScanSource::GetUnit()
{
	return "";
}

bool DoubleScanSource::isCompatibleScanType(const std::string& s)
{
	return (s == "LockIn") || (s == "General");
}

void FScanSource::SetScanOutput(double d)
{
	ScanSource::SetScanOutput(d);

	pPulse->setFreq(d);
	pFPGA->SendParam(page_id, pPulse);
}

double FScanSource::GetOutput()
{
	return pPulse->getFreq();
}

double FScanSource::GetMin()
{
	return 0;
}

double FScanSource::GetMax()
{
	return 2e9;
}

std::string FScanSource::getName()
{
	return pPulse->get_fpga_name() + "(F)";
}

std::string FScanSource::GetUnit()
{
	return "MHz";
}

bool FScanSource::isCompatibleScanType(const std::string& s)
{
	return (s == "LockIn") ||  (s == getType());
}

double FScanSource::getPulseTime()
{
	return pPulse->getTime();
}

double FScanSource::getPulseFreq()
{
	return getNonScanningValue();
}

void FScanSource::useFit(const std::string& scan_type, double x)
{
	if (isCompatibleScanType(scan_type) && pPulse)
		pPulse->setFreq(x);
}

void TScanSource::SetScanOutput(double d)
{
	ScanSource::SetScanOutput(d);

	if (pPulseDDS)
	{
		pPulseDDS->setTime(d);
		pFPGA->SendParam(page_id, pPulseDDS);
	}
	else
	{
		pPulseTTL->setTime(d);
		pFPGA->SendParam(page_id, pPulseTTL);
	}

}

double TScanSource::GetOutput()
{
	if (pPulseDDS)
		return pPulseDDS->getTime();
	else
		return pPulseTTL->getTime();
}

double TScanSource::GetMin()
{
	return 0;
}

double TScanSource::GetMax()
{
	return 2e9;
}

std::string TScanSource::getName()
{
	if (pPulseDDS)
		return pPulseDDS->get_fpga_name() + "(T)";
	else
		return pPulseTTL->get_fpga_name() + "(T)";
}

std::string TScanSource::GetUnit()
{
	return "us";
}

bool TScanSource::isCompatibleScanType(const std::string& s)
{
	return (s == "LockIn") || (s == getType());
}

double TScanSource::getPulseTime()
{
	return getNonScanningValue();
}

void TScanSource::useFit(const std::string& scan_type, double x)
{
	if (isCompatibleScanType(scan_type))
	{
		if (pPulseDDS)
			return pPulseDDS->setTime(x);
		else
			return pPulseTTL->setTime(x);
	}
}
