#ifdef ALUMINIZER_SIM
#else
#include "sleep.h"
extern "C"
{
#include "xtime_l.h"
}
#endif

#include "exp_LMS.h"

extern "C"
{
#include "Cordic.h"
}

#include "Hg/config_Hg.h"
#include "dacI.h"

exp_LMS::exp_LMS(list_t* exp_list, const std::string& name) :
	experiment(exp_list, name),
	delay("Delay", &params, "value=2"),
	num_taps("Num. taps", &params, "value=100"),
	time_steps("Time steps", &params, "value=1000"),
	right_shift1("Right shift 1", &params, "value=16"),
	right_shift2("Right shift 2", &params, "value=16"),
	enableLMS("LMS", &params, "value=0"),
	phaseMultiplier("Phase multiplier", &params, "value=0"),
	delta_f("Delta f", &params, "value=0"),
	G("PLL Inv gain", &params, "value=1250"),
	update_period("Update period [us]", &params, "value=1000"),
	rcADC(NUM_CHANNELS),
	dk1(0),
	dkHP1(0),
	phase_wraps(0),
	ek(0),
	iX(0),
	T(0),
	ftw0(Hz2FTW(160e6)),
	timer(1),
	tLastUpdate(0)
{
	clear_filter();

	right_shift1.setExplanation("yk += (wk * xki) >> right_shift1");
	right_shift2.setExplanation("wk += (ek * xki) >> right_shift2");

	SPI_init(&spi, 2, false);

	for (unsigned j = 0; j < rcADC.size(); j++)
	{
		char name[64];

		if (j < 6)
			snprintf(name, 64, "ADC(%d)", j);
		else
			snprintf(name, 64, "ADC(%d) (s)", j);

		rcADC[j] = new result_channel(channels, name);
	}

	remote_actions.push_back("CLEAR");

	//setup to read channel 0 two reads later
	getResult_AD7689(&spi, 0);
}

unsigned exp_LMS::remote_action(const char* s)
{
	if (strcmp(s, "CLEAR") == 0)
		clear_filter();

	return 0;
}

void exp_LMS::clear_filter()
{
	X = 0;
	W = 0;
	ek = 0;
	dk1 = 0;
	dkHP1 = 0;
	iX = 0;

	phase[0] = 0;
	phase[1] = 0;
	phase_wraps = 0;

	for (int i = 0; i < NUM_ACC; i++)
	{
		acc1[i] = 0;
		accHP1[i] = 0;
	}

	yk1 = 0;

}

void exp_LMS::readADC(int* data)
{
	//     read ch0 (too old, ignore data)
	//configure ch1
	getResult_AD7689(&spi, 1);
	usleep(delay); //wait for conversion to finish

	//     read ch0,1,2,3,4,5,6
	//configure ch2,3,4,5,6,7,0
	for (int j = 0; j < 7; j++)
	{
		data[j] = getResult_AD7689(&spi, (j + 2) % 8);
		usleep(delay); //wait for conversion to finish
	}

	//     read ch7
	//configure ch0 (conversion takes place in next read sequence)
	data[7] = getResult_AD7689(&spi, 0);

	//convert data[0..5] to new coordinates
//     int v_acc[6];
//
//    v_acc[0] = (-425*data[0] - data[1] - 252*data[2] - 890*data[3] + 748*data[4]) >> 10; //coefficients to be scaled by 1024/1000
//    v_acc[1] = (-233*data[0] + 577*data[1] - 577*data[2] - 233*data[3] + 467*data[5]) >> 10;
//    v_acc[2] = (-250*data[0] - 250*data[3] - 500*data[5]) >> 10;
//    v_acc[3] = (-1310*data[0] - 1310*data[3] + 2620*data[5]) >> 10;
//    v_acc[4] = (2390*data[0] - 2390*data[3]) >> 10;
//    v_acc[5] = (1320*data[1] + 1320*data[2] + 1320*data[4]) >> 10;


//   for(int j=0;j<6;j++)
//	    data[j] = v_acc[j];
}


void exp_LMS::resize_taps()
{
	//Make sure the buffers are correctly sized.
	//Check how to align this data for best cache performance.

	int M = num_taps;

	//For accelerometer j = 0..5: w^k_i = W[j*M + i]
	if (W.size() != NUM_ACC * M)
	{
		W.resize(NUM_ACC * M, 0);

		//X is a ring buffer.  iX is the current write index.
		//For accelerometer j = 0..5: x_{ki} = X[j*M + (iX + i)%M]
		X.resize(NUM_ACC * M, 0);

		clear_filter();

		//   W[0] = 1;
	}

	if (iX >= M)
		iX = 0;
}

void exp_LMS::run_exp(int)
{
	//Follow D.L. Jones 2004 notation [http://cnx.org/content/m11829/1.1/]
	int M = num_taps;

	//printf("get = %d\n", M);

	resize_taps();

	//current sample
	int current_data[8];

	int accHP0[NUM_ACC];

	int dk0 = 0;      // current phase
	int dkHP0 = 0;    // current high-passed phase

	int yk = 0;

	//high-pass 1st order Butterworth filter
	int a[2] = { 4096, -4056 };
	int b[2] = { 4076, -4076 };

	tLastUpdate = 0;

	for (unsigned i = 0; i < time_steps; i++)
	{
		//take current sample
		readADC(current_data);

		//calculate laser phase (dk) with CORDIC
		//range should be +/- 2^15 <=> +/- pi
		phase[0] = phase_wraps + FxAtan2((int)(current_data[6]) - 0x4000, (int)(current_data[7]) - 0x4000) - delta_f * T;

		//make new phase close to previous phase by adding/subtracting
		//2^16 (2 pi) increments
		if (phase[1] - phase[0] > 0x8000)
		{
			phase_wraps += 0x10000;
			phase[0] += 0x10000;
		}

		if (phase[0] - phase[1] > 0x8000)
		{
			phase_wraps -= 0x10000;
			phase[0] -= 0x10000;
		}

		dk0 = phase[0]; // - phase[1]; //convert to frequency
		phase[1] = phase[0];

		for (int j = 0; j < NUM_ACC; j++)
		{
			int dacc = acc1[j] - current_data[j];
			accHP0[j] = ((dacc * (-1019)) >> 10) + ((accHP1[j] * 507) >> 9);

			accHP1[j] = accHP0[j];
			acc1[j] = current_data[j];
		}

		int
		dkHP0 = dk0; //((ddk * (-1019)) >> 10) + ((dkHP1 * 507) >> 9);

		dkHP1 = dkHP0;
		dk1 = dk0;

		yk = 0;                                //phase estimate
		for (unsigned j = 0; j < NUM_ACC; j++) //j is accelerometer index
		{
			X[j * M + iX] = accHP0[j];          // (int)current_data[j] - 0x4000; //put current data in ring buffer.  0 V <==> 0x4000
			//yk += updateLMS(M, &(W[j*M]), &(X[j*M]), iX, ek, right_shift);

			yk += updateLMSpred_slow(M, &(W[j * M]), &(X[j * M]), iX, right_shift1);
		}

		//14 bit phase update
		if (!enableLMS)
			PULSE_CONTROLLER_set_dds_phase(pulser, 0, ((phaseMultiplier * yk) >> 2) & 0x3FFF);

#ifndef ALUMINIZER_SIM
		if (tLastUpdate == 0)
			XTime_GetTime(&tLastUpdate);
		else
		{
			XTime tNow;
			unsigned tStep = update_period * 200;

			do
				XTime_GetTime(&tNow);
			while ( tNow <=  (tStep + tLastUpdate) ); //this point in the code is timed to update every tStep clock cycles

			tLastUpdate += tStep;
		}
#endif

		//frequency update for loose lock to LO
		PULSE_CONTROLLER_set_dds_freq(pulser, 0, (int)ftw0 + dk0 / G);

		if (phaseMultiplier != 0)
			ek = dkHP0 - yk;
		else
			ek = dkHP0 + yk;

		yk1 = ((yk >> 1) << 1);


		//channel # on box = DAC+1
		//  iAD5668->setDAC(0, (dkHP0 >> 4) + 32000);
		//  iAD5668->setDAC(1, (yk) + 32000);
//      iAD5668->setDAC(3 , (timer << 15));
		//  iAD5668->setDAC(2, (ek >> 4) + 32000);
//	if(!enableLMS)
//	{
//		iAD5668->setDAC(5, dk0/G );
//		iAD5668->setDAC(6, yk );
//	}
//	else
//	{
//	  iAD5668->setDAC(5, dk0/G);
		//}
		//printf("%i, %i\n", timer, (timer << 15));

		timer = !timer;

		if (!enableLMS)
			ek = 0;
		for (unsigned j = 0; j < NUM_ACC; j++) //j is accelerometer index

			updateLMStaps_slow(M, &(W[j * M]), &(X[j * M]), iX, dkHP0, ek, right_shift2);

		iX--;

		if (iX < 0)
			iX = M - 1;
	}



/*
   for(unsigned j=0; j<6; j++)
   {
      rcADC[j]->result = (int)(current_data[j]) - 0x4000;
   }


   rcADC[0]->result = X[iX];	// accelerometer 0
   //   rcADC[1]->result = dk0;		// phase
   rcADC[2]->result = dkHP0;	// high-passed phase
   rcADC[3]->result = yk;		// prediction
   rcADC[4]->result = ek;		// error
   rcADC[7]->result = W[0];		// filter tap

 */
//   rcADC[1]->result = dk0;
	rcADC[6]->result = dkHP0 >> 5;
	rcADC[7]->result = yk >> 5;
	rcADC[4]->result = ek >> 5;
//   rcADC[5]->result = W[0];

/*   rcADC[6]->result = (int)(current_data[6]) - 0x4000;
   rcADC[7]->result = (int)(current_data[7]) - 0x4000;
 */
	T++;
}

unsigned exp_LMS::getNumPlots()
{
	return NUM_ACC;
}

void exp_LMS::getPlotData(unsigned iPlot, unsigned iStart, GbE_msg& msg_out)
{
	if (iPlot < NUM_ACC)
	{
		int m = W.size() / NUM_ACC - iStart; //# of values to transfer

		if (m < 0)
		{
			msg_out.insertU(0, 0);
			return;
		}
		else
		{
			int numPoints = std::min<int>(m, MSG_STD_PAYLOAD_SIZE - 1);

			msg_out.insertU(0, numPoints);


			for (unsigned j = 0; j < numPoints; j++)
			{
				int k = iPlot * num_taps + j + iStart;
				msg_out.insertU(j + 1, static_cast<unsigned>(W[k]));
				// printf("get W[%d (%d,%d)] = %d\n", k, iPlot, j+iStart, W[k]);
			}
		}
	}
}

void exp_LMS::setCoefficients(const GbE_msg& msg_in, GbE_msg& msg_out)
{
	//assume num_taps has been updated
	int M = num_taps;

	if (W.size() != NUM_ACC * M)
		resize_taps();

	unsigned iAcc = msg_in.extractU(1);
	unsigned iStart = msg_in.extractU(2);

	for (unsigned j = 0; (j + iStart < M) && (j + 3 < MSG_STD_PAYLOAD_SIZE); j++)
	{
		int k = iAcc * M + j + iStart;
		W[k] = msg_in.extractU(j + 3);
		//   printf("set W[%d (%d,%d)] = %d\n", k, iAcc, j+iStart, W[k]);
	}
}
