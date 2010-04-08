#define NOMINMAX

#include "win_io.h"

#include <iostream>
#include <stdexcept>

using namespace std;


#ifndef NO_MCC

#include <cbw.h>

void InitMCC_once()
{
	static int nCalls = 0;

	if(!nCalls)
	{
		float    RevLevel = (float)CURRENTREVNUM;

		/* Declare UL Revision Level */
		cbDeclareRevision(&RevLevel);

		/* Initiate error handling
			Parameters:
				PRINTALL :all warnings and errors encountered will be printed
				DONTSTOP :program will continue even if error occurs.
							Note that STOPALL and STOPFATAL are only effective in 
							Windows applications, not Console applications. 
		*/
		cbErrHandling (PRINTALL, DONTSTOP);

		nCalls++;
	}
}

MCC_analog_in::MCC_analog_in(int BoardNum) : 
analog_in(4),
BoardNum(BoardNum), 
data(4)
{
	InitMCC_once();
}

void MCC_analog_in::getData()
{
//	int cbAInScan(int BoardNum, int LowChan, int HighChan, long Count, long *Rate, int Range, int MemHandle, int Options)

	long Rate = 1200;
	int ULstat = cbAInScan(BoardNum, 0, data.size()-1, data.size(), &Rate, BIP5VOLTS, &(data[0]), FOREGROUND);

	if(ULstat != 0)
		cout << "[MCC_analog_in::getData] cbAInScan ERROR: " << ULstat << endl;

	for(unsigned i=0; i<data.size(); i++)
		values[i] = 5.0 * (data[i] / 2048.0 - 1);
}


MCC_analog_digital_out::MCC_analog_digital_out(int BoardNum) : BoardNum(BoardNum)
{
	InitMCC_once();

	//confiog digital output
	int PortNum = FIRSTPORTA;
    int Direction = DIGITALOUT;
    cbDConfigPort (BoardNum, PortNum, Direction);
}

void MCC_analog_digital_out::setOutputs(const std::valarray<double>& analog, unsigned digital)
{
//	int cbAOutScan(int BoardNum, int LowChan, int HighChan, 
//  long NumPoints, long *Rate, int Range, int MemHandle, int Options)

	int ULstat = cbDOut(BoardNum, FIRSTPORTA, (unsigned char)(digital & 0xFF));

	if(ULstat != 0)
		cout << "[MCC_analog_in::setOutputs] cbDOut ERROR" << endl;

	valarray<short> DAData(analog.size());

	for(unsigned i=0; i<analog.size(); i++)
		DAData[i] = (short)(0.5 + (2.5+analog[i]) * 0.2 * 1024);

	long Rate = 1000;							/* sampling rate (samples per second) */
    int Options = FOREGROUND | SINGLEEXEC;     /* data collection options */

	ULstat = cbAOutScan(BoardNum, 0, 1, 2, &Rate, BIP5VOLTS, &(DAData[0]), Options);

	if(ULstat != 0)
		cout << "[MCC_analog_in::setOutputs] cbAOutScan ERROR" << endl;
}
#endif

#ifndef NO_NIDAQ

void DAQmxErrChk(int e) 
{
	if( DAQmxFailed(e) ) 
	{
		char errBuff[2048];
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		cout << "DAQmx Error: " << errBuff << endl;
		throw runtime_error(errBuff); 
	}
}

NI_analog_in::NI_analog_in(const std::string& channels, double minV, double maxV) :
analog_in(4),
taskHandleAI(0),
data(0.0, 10000)
{
	try
	{
		DAQmxErrChk (DAQmxCreateTask("",&taskHandleAI));
		DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandleAI,channels.c_str(),"",DAQmx_Val_Cfg_Default,minV,maxV,DAQmx_Val_Volts,NULL));
		DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandleAI,NULL,1250,DAQmx_Val_Rising,DAQmx_Val_ContSamps,data.size()/4));
		DAQmxErrChk (DAQmxSetRealTimeReportMissedSamp(taskHandleAI, false)); //don't report an error for missed input samples
	}
	catch(runtime_error e) {}
}

int NI_analog_in::getData()
{
	int32 read = 0;

	try
	{
		//read in all available new data
		DAQmxErrChk (DAQmxReadAnalogF64(taskHandleAI,-1,10.0,DAQmx_Val_GroupByScanNumber,&(data[0]),data.size(),&read,NULL));

		for(unsigned i=0; i<values.size(); i++)
			values[i] = data[i];
	}
	catch(runtime_error e) 
	{
		//restart task on error
	//	stop();
	//	start();
	//	ignore errors
	}

	return read;
}

void NI_analog_in::start()
{
	try
	{
		DAQmxErrChk (DAQmxStartTask(taskHandleAI));
	}
	catch(runtime_error e) {}
}

void NI_analog_in::stop()
{
	try
	{
		DAQmxErrChk (DAQmxStopTask(taskHandleAI));
	}
	catch(runtime_error e) {}
}

NI_analog_out::NI_analog_out(const std::string& Achannels, unsigned numAO, 
							 double minV, double maxV, double offset) :
analog_out(numAO),
minV(minV),
maxV(maxV),
offset(offset)
{
	try
	{
		DAQmxErrChk (DAQmxCreateTask("",&taskHandleAO));
		DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandleAO,Achannels.c_str(),"",minV,maxV,DAQmx_Val_Volts,""));

		DAQmxErrChk (DAQmxStartTask(taskHandleAO));
	}
	catch(runtime_error e) {}
}

void NI_analog_out::updateAnalogOutputs()
{
	//do analog update if values have changed
	valarray<bool> check_equalAO(new_aOut.size());
	check_equalAO = (new_aOut == old_aOut);

	if(check_equalAO.min() == false)
	{
		old_aOut = new_aOut;

		valarray<int32> written(new_aOut.size());
		valarray<double> a_o(new_aOut.size());

		//shift range from -2.5 ... 2.5 to 0 ... 5 V (for NI USB6008)
		for(unsigned i=0; i<new_aOut.size(); i++)
			a_o[i] = offset+new_aOut[i];

		try
		{
			DAQmxErrChk (DAQmxWriteAnalogF64(taskHandleAO,1,0,10.0,DAQmx_Val_GroupByChannel,&(a_o[0]),&(written[0]),NULL));
		}
		catch(runtime_error e) {}
	}
}

NI_digital_out::NI_digital_out(const std::string& Dchannels, unsigned numDO) :
digital_out(numDO)
{
	try
	{
		DAQmxErrChk (DAQmxCreateTask("",&taskHandleDO));
		DAQmxErrChk (DAQmxCreateDOChan(taskHandleDO,Dchannels.c_str(),"",DAQmx_Val_ChanForAllLines));
		
		DAQmxErrChk (DAQmxStartTask(taskHandleDO));
	}
	catch(runtime_error e) {}
}

bool NI_digital_out::updateDigitalOutputs()
{
	bool bDifferent = false;
	unsigned dOut = 0;

    for(unsigned i=0; i<new_dOut.size(); i++)
	{
		if(new_dOut[i])
			dOut |= (1 << i);

        if(old_dOut[i] != new_dOut[i])
		{
            bDifferent = true;
			old_dOut[i] = new_dOut[i];
		}
	}


	//do digital update if values have changed
	if(bDifferent)
	{
		uInt8 d_o = (unsigned char)(dOut & 0xFF);

		try
		{
			DAQmxErrChk (DAQmxWriteDigitalU8(taskHandleDO,1,1,10.0,DAQmx_Val_GroupByChannel,&d_o,NULL,NULL));
		}
		catch(runtime_error e) {}

		return true;
	}

	return false;
}

#endif // NO_NIDAQ
