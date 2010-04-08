//#ifdef PRECOMPILED_HEADER
//#include "common.h"
//#endif

#include "Ovens.h"
#include "FPGA_connection.h"
#include "MotorsPage.h"

#include <iostream>

#ifdef _HAS_OVENS
#include "../include/cbw.h"
#endif

using namespace std;

const int egun_heater_word = (1 << 1);
const int egun_bias_word   = (1 << 4);

const int Al_oven_word     = (1 << 2);
const int Mg_oven_word     = (1 << 3);

class SleepHelper : public QThread
{
public:
static void msleep(int ms)
{
	QThread::msleep(ms);
}
};


RunOvens::RunOvens(const string& sPageName, ExperimentsSheet* pSheet) :
	ExperimentPage(sPageName, pSheet, 999),
	BoardNumber("Board number",  &m_TxtParams, "0", &m_vParameters),
	LoadTime("Load time [s]", &m_TxtParams, "60", &m_vParameters),
	egun_delay_Mg("Mg e-gun delay [s]", &m_TxtParams, "0", &m_vParameters),
	egun_delay_Al("Al e-gun delay [s]", &m_TxtParams, "0", &m_vParameters),
	shutter_Mg("Mg motor", &m_TxtParams, "0", &m_vParameters),
	shutter_Al("Al shutter TTL",   &m_TxtParams, "0", &m_vParameters),
	ElapsedTime("Elapsed time [s]", &m_TxtParams, "0", &m_vParameters, true),
	Progress("Progress", &m_TxtParams, "0", &m_vParameters),
	Running("Running", &m_TxtParams, "0", &m_vParameters, true),
	current_output_word(0),
	output_word(2),
	output_time(2),
	shutter_ttl(99)
{
	ElapsedTime.SetValue(0);
	Progress.SetValue(0);

	init_dio();
	set_output(0);
}

RunOvens::~RunOvens()
{
	set_output(0);

#ifdef _HAS_OVENS
	cbErrHandling(DONTPRINT, DONTSTOP);
#endif
}

void RunOvens::AddAvailableActions(std::vector<std::string>* v)
{
	if (!m_pSheet->scan_scheduler.IsRunning(this, 0))
	{
		v->push_back("LOAD Mg");
		v->push_back("LOAD Al");
	}
	else
		v->push_back("STOP");
}

void RunOvens::on_action(const std::string& s)
{
	if (s == "LOAD Mg")
	{
		shutter_ttl = shutter_Mg;

		output_word.at(0) = Mg_oven_word;

		if (LoadTime.Value() > egun_delay_Mg.Value())
			output_word.at(0) = output_word.at(0) | egun_heater_word;

		output_time.at(0) = std::min(LoadTime.Value(), egun_delay_Mg.Value());

		output_word.at(1) = Mg_oven_word | egun_heater_word | egun_bias_word;
		output_time.at(1) = LoadTime;

		m_pSheet->scan_scheduler.StartExperiment(this, 0);
	}

	if (s == "LOAD Al")
	{
		shutter_ttl = shutter_Al;

		output_word.at(0) = Al_oven_word;

		if (LoadTime.Value() > egun_delay_Al.Value())
			output_word.at(0) = output_word.at(0) | egun_heater_word;

		output_time.at(0) = std::min(LoadTime.Value(), egun_delay_Al.Value());

		output_word.at(1) = Al_oven_word | egun_heater_word | egun_bias_word;
		output_time.at(1) = LoadTime;

		m_pSheet->scan_scheduler.StartExperiment(this, 0);
	}

	if (s == "STOP")
		m_pSheet->scan_scheduler.FinishExperiment(this, 0);
}



void RunOvens::init_dio()
{
#ifdef _HAS_OVENS
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
#endif
}

void RunOvens::setMgOven(bool b)
{
	if (b)
	{
		int new_word = current_output_word | Mg_oven_word;
		set_output(new_word);
	}
	else
	{
		int new_word = current_output_word & (~Mg_oven_word);
		set_output(new_word);
	}
}

void RunOvens::setAlOven(bool b)
{
	if (b)
	{
		int new_word = current_output_word | Al_oven_word;
		set_output(new_word);
	}
	else
	{
		int new_word = current_output_word & (~Al_oven_word);
		set_output(new_word);
	}
}



void RunOvens::set_output(int word)
{

#ifdef _HAS_OVENS
	int ULStat;
	ULStat = cbDOut(BoardNumber, FIRSTPORTCL, (~word & 0x0f) );
	ULStat = cbDOut(BoardNumber, FIRSTPORTCH, (~word & 0xf0) >> 4 );
#else
	printf("WARNING: NO OVEN SUPPORT\r\n");
#endif

	current_output_word = word;

	std::string str_run;

	if (word & Al_oven_word)
		str_run = str_run + "* Al oven *";

	if (word & Mg_oven_word)
		str_run = str_run + "* Mg oven *";

	if (word & egun_heater_word)
		str_run = str_run + "* e-gun heater *";

	if (word & egun_bias_word)
		str_run = str_run + "* e-gun bias *";

	if (word == 0)
		str_run = "nothing";

	Running.SetValue(str_run);
	Running.PostUpdateGUI();
}

void RunOvens::setAlPI(bool b)
{
	if (b)
		pFPGA->SetLogicState(shutter_Al, 1);
	else
		pFPGA->SetLogicState(shutter_Al, 2);
}

ExperimentBase::run_status RunOvens::Run()
{
	printf("Running ovens.\r\n");

	set_output(output_word.at(0));

	MotorsPage* pMotors = dynamic_cast<MotorsPage*>(m_pSheet->FindPage("Motors"));

	if (shutter_ttl < 32)
	{
		if (output_word.at(0) == Al_oven_word)
			pFPGA->SetLogicState(shutter_ttl, 1);
		else
		if (pMotors)
			pMotors->setMotorPosition(shutter_ttl, 1);
	}

	double t0 = CurrentTime_s();
	double dt = 0;

	//current output index
	unsigned iOut = 0;

	while (!pRunObject->ShouldStop())
	{
		dt = CurrentTime_s() - t0;

		if (dt >= output_time.at(iOut))
		{
			iOut++;

			if (iOut < output_word.size())
				set_output(output_word[iOut]);
			else
				break;
		}

		ElapsedTime.SetValue( dt );
		Progress.SetValue( dt / LoadTime.Value() );

		ElapsedTime.PostUpdateGUI();
		Progress.PostUpdateGUI();

		SleepHelper::msleep(10);
	}

	printf("Stopped at t=%4.3f s.\r\n", dt);

	set_output(0);

	if (shutter_ttl < 32)
	{
		if (output_word.at(0) == Al_oven_word)
			pFPGA->SetLogicState(shutter_ttl, 2);
		else
		if (pMotors)
			pMotors->setMotorPosition(shutter_ttl, 0);
	}

	return FINISHED;
}
