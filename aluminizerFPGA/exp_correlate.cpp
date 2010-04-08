#ifdef ALUMINIZER_SIM
#else
#include "sleep.h"
#endif

#include "exp_correlate.h"

#include "fft.h"
#include <iostream>

using namespace std;

exp_correlate::exp_correlate(list_t* exp_list, const std::string& name) :
	exp_detect(exp_list, name),
	removeFromQueue("Remove photons from queue", &params, "value=1"),
	clockDivider("Clock divider", &params, "value=1 min=0 max=15"),
	fftFrequency("FFT frequency", &params, "value=1."),
	Heat(DDS_HEAT, "Heat", &params, 0, "t=100 fOn=1.58 fOff=0"),
	rcBin1FFT(channels, "Bin1FFT (s)"),
	rcBins(N_BINS)
{
	Detect.setFlag(RP_FLAG_NOLINK);
	rcSignal.name = COOLING_ION_NAME + std::string(" signal");

	for (int i = 0; i < N_BINS; i++)
	{
		char s[32];
		snprintf(s, 30, "bin(%u) (h)", i);
		rcBins[i] = new result_channel(channels, s);

		histogram[i] = 0;
	}
}

void exp_correlate::updateParams()
{
	info_interface::updateParams();
	// printf("writing slave reg 1 [0:31] = 0x%X\n", 0x80000000 | (removeFromQueue << 30) | (clockDivider << 26));
	PULSE_CONTROLLER_write_slave_reg(pulser, 1, 0, 0x80000000 | (removeFromQueue << 30) | (clockDivider << 26));
	PULSE_CONTROLLER_write_slave_reg(pulser, 1, 0, 0x00000000);
}

//! Called at the beginning of each exp. Overrides should call the base class function.
void exp_correlate::init_exp(unsigned i)
{
	if (i == 0)
		for (unsigned j = 0; j < N_BINS; j++)
			histogram[j] = 0;


	exp_detect::init_exp(i);
}

void exp_correlate::post_exp_calc_results()
{
	complex fftsignal[N_BINS];

	for (unsigned j = 0; j < N_BINS; j++)
	{
		rcBins[j]->result = histogram[j];
		fftsignal[j] = (float)histogram[j];
	}

	CFFT::Forward(fftsignal, N_BINS);
	rcBin1FFT.result = fftsignal[1].norm() / 1000;

	exp_detect::post_exp_calc_results();
}

void exp_correlate::run_exp(int)
{
	// run the pulse sequence
	Precool.ddsOff();
	DopplerCool.pulse();
	Heat.pulseStayOn();
	Detect.detection_pulse();
	Heat.ddsOff();
	Precool.pulseStayOn();

	// wait until the pulse sequence is finished
	while (!PULSE_CONTROLLER_is_finished(pulser)) ;

	// get the results
	for (unsigned j = 0; j < N_BINS; j++)
		histogram[j] += PULSE_CONTROLLER_read_slave_reg(pulser, 3 + j / (64 / N_BITS), (N_BITS / 8) * (j % (64 / N_BITS))) >> (32 - N_BITS);
}

unsigned exp_correlate::getNumPlots()
{
	return 1 + PLOT_FFT;
}

void exp_correlate::getPlotData(unsigned iPlot, unsigned iStart, GbE_msg& msg_out)
{
	// sanity check
	unsigned sum = 0;

	for (unsigned j = 0; j < N_BINS; j++)
		sum += histogram[j];

	//printf("sum = %d\n", sum);




	int m = N_BINS - iStart; //# of values to transfer

	if (m < 0)
	{
		msg_out.insertU(0, 0);
		return;
	}

	unsigned numPoints = std::min<unsigned>(m, MSG_STD_PAYLOAD_SIZE - 1);
	msg_out.insertU(0, numPoints);

	switch (iPlot)
	{
	case 0:
		for (unsigned j = 0; j < numPoints; j++)
			msg_out.insertU(j + 1, histogram[j]);
			// printf("get histogram[%d] = %d\n", j, histogram[j]);
		break;
	case 1:
		complex fftsignal[N_BINS];

		for (unsigned j = 0; j < N_BINS; j++)
			fftsignal[j] = (float)histogram[j];

		CFFT::Forward(fftsignal, N_BINS);
		fftsignal[0] = 0.;

		for (unsigned j = 0; j < numPoints; j++)
			msg_out.insertU(j + 1, (int)(fftsignal[j].norm()));
			// printf("get fftsignal[%d].norm() = %d\n", j, histogram[j]);

		break;
	}
}
