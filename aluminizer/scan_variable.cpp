#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "scan_variable.h"

#include "ScanSource.h"


void source_scan_variable::set_scan_positionI(double d)
{
	pSource->SetScanOutput(d);
}

void source_scan_variable::set_default_state(double d)
{
	scan_variable::set_default_state(d);
	pSource->setNonScanningValue(d);
}
