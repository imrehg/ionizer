#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "AluminizerApp.h"
#include "CommonExperimentsSheet.h"
#include "NI_AnalogOut.h"
#include "FPGA_connection.h"
#include "FPGA_GUI.h"

//#define _NO_NIDAQ

#ifndef _NO_NIDAQ
int NI_error = 0;
	#define DAQmxErrChk(functionCall) { if ( DAQmxFailed(NI_error = (functionCall)) ) { char errBuff[2048]; DAQmxGetExtendedErrorInfo(errBuff, 2048); throw runtime_error(errBuff); } }
#endif

#define _NO_TWIST_VOLTAGES

using namespace std;

TriggererdNIAnalogOut::TriggererdNIAnalogOut(int iDevice) :
	nChannels(8),
	iDevice(iDevice),
	baseline(nChannels, 0),
	offset(8, 0),
	taskHandle(0),
	csDAQ("NI_DAQ"),
	bHoldUpdates(true)
{

	wv = new double[2 * nChannels];

	SetupSingleGeneration();
}

TriggererdNIAnalogOut::~TriggererdNIAnalogOut()
{
#ifndef _NO_NIDAQ
	if (taskHandle)
	{
		//stop and clear output task
		DAQmxClearTask(taskHandle);
		taskHandle = 0;
	}
#endif

	delete [] wv;
}

std::string TriggererdNIAnalogOut::ChannelList() const
{
	return "Dev" + to_string<int>(iDevice) + "/ao0:7";
}

void TriggererdNIAnalogOut::SetupSingleGeneration()
{
#ifndef _NO_NIDAQ
	DAQmxErrChk(DAQmxClearTask(taskHandle));
	DAQmxErrChk(DAQmxCreateTask(("NI" + to_string<int>(iDevice)).c_str(), &taskHandle));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandle, ChannelList().c_str(), "", -10.0, 10.0, DAQmx_Val_Volts, NULL) );
#endif //_NO_NIDAQ
}


void TriggererdNIAnalogOut::HoldUpdates(bool b)
{
	bHoldUpdates = b;
}

void TriggererdNIAnalogOut::ForceUpdates()
{
#ifndef _NO_NIDAQ
	int32 written = 0;
/*
   printf("V= ");
   for(size_t j=0; j<baseline.size(); j++)
      printf("%6.3f  ", baseline[j]);

   printf("\n");
 */
	DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 1, 1, 1, DAQmx_Val_GroupByScanNumber, &(baseline[0]), &written, 0) );
#endif
}


void TriggererdNIAnalogOut::SetBaselineOutput(int channel, double v)
{
	if (channel >= 0)
		//	printf("V(%d) = %f\n", channel, v);
		baseline.at(channel) = v;

#ifndef _NO_NIDAQ
	CriticalSectionOwner cso(&csDAQ, 0);

	if (!bHoldUpdates)
		ForceUpdates();
#endif
}

NI_AnalogOut::NI_AnalogOut(TriggererdNIAnalogOut* tNI, int iChannel, double min_output, double max_output, double gain, double offset) :
	AnalogOut(min_output, max_output, gain, offset),
	iChannel(iChannel),
	dVoltage(2 * max_output),
	tNI(tNI)
{
}

NI_AnalogOut::~NI_AnalogOut()
{
}

void NI_AnalogOut::SetOutput(double voltage)
{
	//don't set voltages that are already set
	if (dVoltage == voltage)
		return;

	if ( !IsValidOutput(voltage) )
		throw bad_output_exception(voltage, min_output, max_output);

	tNI->SetBaselineOutput(iChannel, (voltage - offset) / gain);

	dVoltage = voltage;
}

/*
   FPGA_AnalogOut::FPGA_AnalogOut(FPGA_connection* pFPGA, const std::string& pageName, unsigned iChannel, double min_output, double max_output, double gain, double offset) :
   AnalogOut(min_output, max_output, gain, offset),
   pFPGA(pFPGA),
   pageName(pageName),
   iChannel(iChannel),
   dVoltage(2*max_output),
   iPage(-1)
   {
   }

   FPGA_AnalogOut::~FPGA_AnalogOut()
   {
   }

   void FPGA_AnalogOut::SetOutput(double voltage)
   {
   if(iPage < 0)
   {
      if(theApp->m_pExperimentsSheet)
      {
         FPGA_GUI* pPage = dynamic_cast<FPGA_GUI*>(theApp->m_pExperimentsSheet->FindPage(pageName));

         if(pPage)
            iPage = pPage->get_page_id();
      }
   }

   if(iPage >= 0)
   {
      //don't set voltages that are already set
      if(dVoltage == voltage)
         return;

      if( !IsValidOutput(voltage) )
         throw bad_output_exception(voltage, min_output, max_output);

      pFPGA->setVoltage(iPage, iChannel, 1e6*(voltage-offset)/gain);

      dVoltage = voltage;
   }
   else
      cout << "No voltage update.  Can't find DAC page: " << pageName << endl;
   }


 */