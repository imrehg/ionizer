#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "MCC_AnalogOut.h"

#ifndef _NO_MCC_AO
#include <cbw.h>
#endif

using namespace std;

void InitMCC_once()
{
#ifndef _NO_MCC_AO
	static int nCalls = 0;

	if (!nCalls)
	{
		float RevLevel = (float)CURRENTREVNUM;

		/* Declare UL Revision Level */
		cbDeclareRevision(&RevLevel);

		/* Initiate error handling
		   Parameters:
		      PRINTALL :all warnings and errors encountered will be printed
		      DONTSTOP :program will continue even if error occurs.
		               Note that STOPALL and STOPFATAL are only effective in
		               Windows applications, not Console applications.
		 */
		cbErrHandling(PRINTALL, DONTSTOP);

		nCalls++;
	}
#endif //_NO_MCC_AO
}

MCC_AnalogOut::MCC_AnalogOut(int iDevice, int iChannel) :
	AnalogOut(0, 20),

	iDev(iDevice),
	iChannels(1, iChannel),
	dOutput(1e-9)
{
	InitMCC_once();
}

MCC_AnalogOut::MCC_AnalogOut(int iDevice, std::vector<int> iChannels) :
	AnalogOut(0, 20. * iChannels.size()),

	iDev(iDevice),
	iChannels(iChannels),
	dOutput(1e-9)
{
	InitMCC_once();
}

void MCC_AnalogOut::SetOutput(double d)
{
	//don't set voltages that are already set
	if (dOutput == d)
		return;

	if ( !IsValidOutput(d) )
		throw bad_output_exception(d, min_output, max_output);

#ifndef _NO_MCC_AO

	double dActual = 0;
	unsigned short sVal = floor(65535. * d / (20. * iChannels.size()));

	for (int i = 0; i < iChannels.size(); i++)
	{
		if (i + 1 == iChannels.size())
			sVal = floor(0.5 + 65535. * (d - dActual) / 20.);

		int ULStat = cbAOut(iDev, iChannels[i], MA0TO20, sVal);
		dActual += sVal * 20. / 65535.;
	}

	cout << "[MCC " << iDev << ", " << iChannels[0] << "] = " << setprecision(9) << dActual << " mA" << endl;
#endif //_NO_MCC_AO

	dOutput = d;
};

