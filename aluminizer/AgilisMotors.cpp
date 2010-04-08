#include "AluminizerApp.h"
#include "AgilisMotors.h"
#include "ExpAl.h"

AgilisMotorsPage* ExpAl::pMirrorMotors;

AgilisMotorsPage::AgilisMotorsPage(const std::string& sPageName, ExperimentsSheet* pSheet) :
	ParamsPage(pSheet, sPageName),
	nMotors(2),
	Names(nMotors),
	Positions(nMotors),
	InternalPos(nMotors),
	corrections_needed(nMotors, 0),
	Increments(nMotors),
	Decrements(nMotors),
	Corrections(nMotors),
	Response("Response", &m_TxtParams, "nothing", &m_vParameters),
	DebugAgilis("Debug commands", &m_TxtParams, "0", &m_vParameters),
	SingleStep("Single step",  &m_TxtParams, "0", &m_vParameters),
	ImmediateUpdates("Immediate", &m_TxtParams, "0", &m_vParameters),
	AutoCorrect("Auto-correct",   &m_TxtParams, "0", &m_vParameters),
	rs232("COM7", 921600),
	iCurrentMotor(0xffffffff)
{
	SingleStep.setToolTip("Divide all motion into single steps");
	AutoCorrect.setToolTip("Automatically compensate for unequal step-size");

	Response.setFlags(RP_FLAG_READ_ONLY);
	ImmediateUpdates.setFlags(RP_FLAG_NOPACK);
	SingleStep.setFlags(RP_FLAG_NOPACK);
	AutoCorrect.setFlags(RP_FLAG_NOPACK);

	for (unsigned i = 0; i < nMotors; i++)
	{
		char sbuff[64];

		snprintf(sbuff, 64, "[COM6::%d]", i + 1);
		Names[i] = new GUI_string(sbuff, &m_TxtParams, "Motor name", &m_vParameters);
		Names[i]->setInputWidth(10);
		m_vAllocatedParams.push_back(Names[i]);

		snprintf(sbuff, 64, "Position  (%d)", i);
		Positions[i] = new GUI_int_no_label(sbuff,   &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(Positions[i]);
		Positions[i]->setPrecision(0);
		Positions[i]->setRange(-1e9, 1e9);
		Positions[i]->setIncrement(1);

		InternalPos[i] = Positions[i]->Value();

		snprintf(sbuff, 64, "Increments  (%d)", i);
		Increments[i] = new GUI_int_no_label(sbuff,  &m_TxtParams, "0", &m_vParameters);
		Increments[i]->setFlags(RP_FLAG_READ_ONLY);

		snprintf(sbuff, 64, "Decrements  (%d)", i);
		Decrements[i] = new GUI_int_no_label(sbuff,  &m_TxtParams, "0", &m_vParameters);
		Decrements[i]->setFlags(RP_FLAG_READ_ONLY);

		snprintf(sbuff, 64, "Corrections  (%d)", i);
		Corrections[i] = new GUI_int_no_label(sbuff, &m_TxtParams, "0", &m_vParameters);
		Corrections[i]->setFlags(RP_FLAG_READ_ONLY);
	}

	for (unsigned i = 0; i < nMotors; i++)
		g_scan_sources.push_back(new MScanSource(i, this));


	initController();

	ExpAl::pMirrorMotors = this;
}


void AgilisMotorsPage::initController()
{
	sendCmd("MR\r");  //remote mode
	sendCmd("CC1\r"); //remote mode

	//Set step sizes.  These are amplitudes for the voltage pulses coming out of the Newport controller.
	for (unsigned i = 0; i < nMotors; i++)
	{
		char sbuff[64];

		//increment sizes
		snprintf(sbuff, 64, "%dSU+42\r", i + 1);
		sendCmd(sbuff);

		//decrement sizes
		snprintf(sbuff, 64, "%dSU-40\r", i + 1);
		sendCmd(sbuff);
	}
}

AgilisMotorsPage::~AgilisMotorsPage()
{
}

std::string AgilisMotorsPage::sendCmd(const std::string& s)
{
	try
	{
		bool bRetry = true;

		while (bRetry)
		{
			rs232.sendCmd(s);
			SleepHelper::msleep(5);

			if (DebugAgilis)
				cout << "[Agilis] <<< " << s << endl;;

			string r = rs232.sendCmd_getResponse("TE\r"); //check error code
			Response.SetValue(r);

			if (DebugAgilis)
				cout << "[Agilis] >>> " << r;

			if (r.find("TE0") != string::npos)
				bRetry = false;
			else
				SleepHelper::msleep(15);
		}
	}
	catch (runtime_error e)
	{
		cerr << "[AgilisMotorsPage::sendCmd] RUNTIME ERROR: " << e.what() << endl;
	}

	return "";
}

void AgilisMotorsPage::updatePositions()
{
	for (size_t i = 0; i < InternalPos.size(); i++)
		driveTo(i, Positions[i]->Value());
}

void AgilisMotorsPage::driveTo(unsigned iMotor, double pos)
{
	if (InternalPos[iMotor] != pos)
	{
		//update position
		char buff[256];
		double delta = pos - InternalPos[iMotor];
		delta = floor(delta + 0.5);

		// automatic corrections for unequal step size
		// position = I*d0 - D*d1 (I, D = #Increments, #Decrements and d0, d1 = inc, dec step size)
		// let r = d0/d1 (unknown)
		// position = I*d0 - D*d0/r = 0 (assuming no long term drift)
		// then r = D/I
		// Let D' = D*r  (D' = corrected number of steps)
		// Then the actual movement D'*d1 = D*r*d1 = D*d0 (desired movement)
		double corr = 0;

		if (AutoCorrect && (delta < 0) && (Decrements[iMotor]->Value() > 1000) && (Increments[iMotor]->Value() > 1000))
		{
			double r = Decrements[iMotor]->Value() / (double)( Increments[iMotor]->Value() );
			corrections_needed[iMotor] += delta * (r - 1);

			corr = floor(corrections_needed[iMotor] + 0.5); // round to nearest integer
			corrections_needed[iMotor] -= corr;
			Corrections[iMotor]->SetValue( Corrections[iMotor]->Value() + corr );

			printf("Agilis correction = %f\r\n", corr);
		}

		if (SingleStep)
		{
			for (int i = 0; i < abs(delta + corr); i++)
			{
				if (delta >= 0)
					snprintf(buff, 255, "%dPR+1\r", iMotor + 1);
				else
					snprintf(buff, 255, "%dPR-1\r", iMotor + 1);

				sendCmd(buff);
			}
		}
		else
		{
			if (delta >= 0)
				snprintf(buff, 255, "%dPR+%d\r", iMotor + 1, (int)(delta));
			else
				snprintf(buff, 255, "%dPR%d\r", iMotor + 1, (int)(delta + corr));

			sendCmd(buff);
		}

		if (delta > 0)
			Increments[iMotor]->SetValue( Increments[iMotor]->Value() + delta);
		else
			Decrements[iMotor]->SetValue( Decrements[iMotor]->Value() - delta - corr);

		InternalPos[iMotor] += delta;

		SleepHelper::msleep(50);
	}
}



unsigned AgilisMotorsPage::num_columns()
{
	return 6;
}


void AgilisMotorsPage::AddAvailableActions(std::vector<std::string>* p)
{
	p->push_back("RUN");
	p->push_back("CLEAR");
}

void AgilisMotorsPage::on_action(const std::string& s)
{
	if (s == "RUN")
		updatePositions();

	if (s == "CLEAR")
	{
		for (unsigned i = 0; i < nMotors; i++)
		{
			Increments[i]->SetValue(0);
			Decrements[i]->SetValue(0);
			Corrections[i]->SetValue(0);
		}

		emit sig_update_data();
	}
}

bool AgilisMotorsPage::RecalculateParameters()
{
	if (ImmediateUpdates)
		updatePositions();

	return false;
}
