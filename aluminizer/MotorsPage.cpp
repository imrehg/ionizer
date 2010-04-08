#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "MotorsPage.h"
#include "ExperimentPage.h"
#include "FPGA_connection.h"
#include "AluminizerApp.h"

#include <QMessageBox>

MotorsPage::MotorsPage(const std::string& sPageName, ExperimentsSheet* pSheet) :
	ParamsPage(pSheet, sPageName),
	nMotors(2),
	Names(nMotors),
	Angles(nMotors),
	Pos0(nMotors),
	Pos1(nMotors),
	manager(this),
	MgPI_2108("Mg PI (2042,2034) - 2108", &m_TxtParams, "0", &m_vParameters),
	MgPI_2042("Mg PI 2042 - 2034", &m_TxtParams, "0", &m_vParameters),
	MgPI_status("Mg PI status", &m_TxtParams, "unknown", &m_vParameters)
{
	MgPI_2108.setFlags(RP_FLAG_READ_ONLY | RP_FLAG_NOPACK);
	MgPI_2042.setFlags(RP_FLAG_READ_ONLY | RP_FLAG_NOPACK);
	MgPI_status.setFlags(RP_FLAG_READ_ONLY | RP_FLAG_NOPACK);


	for (unsigned i = 0; i < nMotors; i++)
	{
		char sbuff[64];

		snprintf(sbuff, 64, "[%d]", i);
		Names[i] = new GUI_string(sbuff, &m_TxtParams, "Motor name", &m_vParameters);
		Names[i]->setInputWidth(10);
		m_vAllocatedParams.push_back(Names[i]);


		snprintf(sbuff, 64, "Angle  (%d)", i);
		Angles[i] = new GUI_double_no_label(sbuff,   &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(Angles[i]);
		Angles[i]->setPrecision(1);
		Angles[i]->setRange(0, 60);
		Angles[i]->setIncrement(1);
		Angles[i]->setSuffix(" deg");

		snprintf(sbuff, 64, "Pos0  (%d)", i);
		Pos0[i] = new GUI_double(sbuff,  &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(Pos0[i]);
		Pos0[i]->set_display_label("pos. 0 = ");
		Pos0[i]->setPrecision(1);
		Pos0[i]->setRange(0, 60);
		Pos0[i]->setIncrement(1);
		Pos0[i]->setSuffix(" deg");

		snprintf(sbuff, 64, "Pos1  (%d)", i);
		Pos1[i] = new GUI_double(sbuff,  &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(Pos1[i]);
		Pos1[i]->set_display_label("pos. 1 = ");
		Pos1[i]->setPrecision(1);
		Pos1[i]->setRange(0, 60);
		Pos1[i]->setIncrement(1);
		Pos1[i]->setSuffix(" deg");
	}

	connect(&manager, SIGNAL(finished(QNetworkReply*)),
	        this, SLOT(replyFinished(QNetworkReply*)));

	on_action("RUN");
	on_action("QUERY");
}

MotorsPage::~MotorsPage()
{
}

int MotorsPage::status_2042_2034()
{
	if ( (MgPI_2042.Value() % 36000) ==  2000)
		return 2034;

	if ( (MgPI_2042.Value() % 36000) ==  20000)
		return 2042;

	return -1;
}

int MotorsPage::status_2108_2034()
{
	if ( (MgPI_2108.Value() % 36000) ==  7000 )
		return 2034;

	if ( (MgPI_2108.Value() % 36000) ==  25000 )
		return 2108;

	return -1;
}

void MotorsPage::replyFinished(QNetworkReply* r)
{
	size_t n = r->bytesAvailable();

	string buff;

	buff.resize(n);

	r->read(&(buff[0]), n);

	n = buff.find("Current position: ");

	if (n != string::npos)
	{
		int d = -1;
		sscanf(buff.substr(n).c_str(), "Current position: %d", &d);
		cout << "Mg+ PI pos = " << d << endl;

		if (r->url().toString().indexOf("movestage2") != -1)
			MgPI_2042.SetValue(d);
		else
			MgPI_2108.SetValue(d);
	}
	else
		cout << "Bad network reply from Mg+ PI waveplate controller" << endl;

	r->deleteLater();

	MgPI_status.SetValue("Unknown");

	if (2108 == status_2108_2034())
		MgPI_status.SetValue("2108");

	if (2034 == status_2108_2034())
	{
		if (2034 == status_2042_2034())
			MgPI_status.SetValue("2034");

		if (2042 == status_2042_2034())
			MgPI_status.SetValue("2042");
	}

	emit sig_update_data();
}

void MotorsPage::setMgPI(bool b)
{
	setMotorPosition(1, b ? 1 : 0);
}

void MotorsPage::setMotorPosition(int i, unsigned p)
{
	if (p > 1)
		throw runtime_error("unkown motor position");

	SetAngle(i, p == 0 ? Pos0[i]->Value() : Pos1[i]->Value());
}

void MotorsPage::SetAngle(int i, double a)
{
	Angles[i]->SetValue( a );
	ExperimentPage::pFPGA->SetMotorAngle(i, a * 60);
	Angles[i]->PostUpdateGUI();
}

double MotorsPage::GetAngle(int i)
{
	return ExperimentPage::pFPGA->GetMotorAngle(i) / 60.0;
}

void MotorsPage::AddAvailableActions(std::vector<std::string>* p)
{
	p->push_back("RUN");
	p->push_back("HOME");
	p->push_back("QUERY");
	p->push_back("PI2034");
	p->push_back("PI2042");
	p->push_back("PI2108");
}

bool MotorsPage::RecalculateParameters()
{
	on_action("RUN");

	return false;
}

void MotorsPage::on_action(const std::string& s)
{
	if (s == "HOME")
		for (unsigned i = 0; i < nMotors; i++)
			SetAngle(i, 0);

	if (s == "RUN")
		for (unsigned i = 0; i < nMotors; i++)
			SetAngle(i, Angles[i]->Value());

	if (s == "QUERY")
	{
		MgPI_2042.SetValue(-1);
		MgPI_2108.SetValue(-1);

		manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage?status")));
		manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage2?status")));

		for (unsigned i = 0; i < nMotors; i++)
		{
			Angles[i]->SetValue(GetAngle(i));
			Angles[i]->PostUpdateGUI();
		}
	}

	if (s == "PI2034")
	{
		if (MgPI_status.Value() == "unknown")
		{
			QMessageBox::warning(this, tr("Aluminizer"),
			                     tr("WARNING: Mg PI beam status is unknown.\n"
			                        "Please press QUERY to determine status."),
			                     QMessageBox::Ok);

			return;
		}

		if (MgPI_status.Value() == "2108")
		{
			manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage?cw45")));

			if (2042 == status_2042_2034())
				manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage2?cw45")));
		}

		if (MgPI_status.Value() == "2042")
			manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage2?cw45")));
	}

	if (s == "PI2042")
	{
		if (MgPI_status.Value() == "unknown")
		{
			QMessageBox::warning(this, tr("Aluminizer"),
			                     tr("WARNING: Mg PI beam status is unknown.\n"
			                        "Please press QUERY to determine status."),
			                     QMessageBox::Ok);

			return;
		}

		if (MgPI_status.Value() == "2108")
		{
			manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage?cw45")));

			if (2034 == status_2042_2034())
				manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage2?ccw45")));
		}

		if (MgPI_status.Value() == "2034")
			manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage2?ccw45")));
	}

	if (s == "PI2108")
	{
		if (MgPI_status.Value() == "unknown")
		{
			QMessageBox::warning(this, tr("Aluminizer"),
			                     tr("WARNING: Mg PI beam status is unknown.\n"
			                        "Please press QUERY to determine status."),
			                     QMessageBox::Ok);

			return;
		}

		if (MgPI_status.Value() == "2034" || MgPI_status.Value() == "2042")
			manager.get(QNetworkRequest(QUrl("http://847ionswitch.bw.nist.gov/cgi-bin/movestage?ccw45")));
	}
}

unsigned MotorsPage::num_columns()
{
	return 8;
}

/*
   PicoMotorsPage::PicoMotorsPage(const std::string& sPageName, ExperimentsSheet* pSheet) :
   ParamsPage(pSheet, sPageName),
   nMotors(2),
   Names(nMotors),
   Positions(nMotors),
   Velocities(nMotors),
   InternalPos(nMotors),
   InternalV(nMotors),
   Response("Response",	&m_TxtParams, "nothing", &m_vParameters),
   Asymmetry("Asymmetry",	&m_TxtParams, "1.2", &m_vParameters),
   ImmediateUpdates("Immediate",	&m_TxtParams, "0", &m_vParameters),
   rs232(7, 19200),
   iCurrentMotor(0xffffffff)
   {
   Response.setFlags(RP_FLAG_READ_ONLY);
   ImmediateUpdates.setFlags(RP_FLAG_NOPACK);

   for(unsigned i=0; i<nMotors; i++)
   {
      char sbuff[64];

     snprintf(sbuff, 64, "[%d]", i);
      Names[i] = new GUI_string(sbuff,	&m_TxtParams, "Motor name", &m_vParameters);
      Names[i]->setInputWidth(10);
      m_vAllocatedParams.push_back(Names[i]);

     snprintf(sbuff, 64, "Position  (%d)", i);
      Positions[i] = new GUI_double_no_label(sbuff,	&m_TxtParams, "0", &m_vParameters);
      m_vAllocatedParams.push_back(Positions[i]);
      Positions[i]->setPrecision(0);
      Positions[i]->setRange(-1e9, 1e9);
      Positions[i]->setIncrement(10);

      InternalPos[i] = Positions[i]->Value();

     snprintf(sbuff, 64, "Velocity  (%d)", i);
      Velocities[i] = new GUI_double_no_label(sbuff,	&m_TxtParams, "100", &m_vParameters);
      m_vAllocatedParams.push_back(Velocities[i]);
      Velocities[i]->setPrecision(0);
      Velocities[i]->setRange(0, 2000);
      Velocities[i]->setIncrement(10);

      InternalV[i] = -1;
   }

   for(unsigned i=0; i<nMotors; i++)
      g_scan_sources.push_back(new MScanSource(i, this));


   sendCmd("VER");
   }

   PicoMotorsPage::~PicoMotorsPage()
   {
   }

   std::string PicoMotorsPage::sendCmd(const std::string& s)
   {
   string r = rs232.sendCmd_getResponse(s);
   Response.SetValue(r);

   return r;
   }

   void PicoMotorsPage::updatePositions()
   {
   for(size_t i=0; i<InternalPos.size(); i++)
   {
      driveTo(i, Positions[i]->Value());
   }
   }

   void PicoMotorsPage::setDriveChannel(unsigned iMotor)
   {
   if(iMotor != iCurrentMotor)
   {
      char buff[256];
      snprintf(buff, 255, "CHL A1=%u", iMotor);
      sendCmd(buff);
      SleepHelper::msleep(300);

      iCurrentMotor = iMotor;
   }
   }

   void PicoMotorsPage::driveTo(unsigned iMotor, double pos)
   {
   if(InternalPos[iMotor] != pos)
   {
      setDriveChannel(iMotor);

      //update velocity
      if(InternalV[iMotor] != Velocities[iMotor]->Value())
      {
         char buff[256];

         snprintf(buff, 255, "VEL A1 %d=%.0f", iMotor, Velocities[iMotor]->Value());
         sendCmd(buff);

         InternalV[iMotor] = Velocities[iMotor]->Value();
      }

      //update position
      char buff[256];
      double delta = pos - InternalPos[iMotor];

      if(delta < 0)
         delta *= Asymmetry.Value();

      delta = floor(delta);

      snprintf(buff, 255, "REL A1 %.0f", delta);
      sendCmd(buff);
      sendCmd("GO A1");

      //wait for motor to finish
      int p = 0;

      do
      {
         SleepHelper::msleep(100);
         string s = sendCmd("POS A1");
         sscanf(s.c_str(), "A1=%d\r\n", &p);
      } while(p != delta);

      InternalPos[iMotor] = pos;
   }
   }



   unsigned PicoMotorsPage::num_columns()
   {
   return 5;
   }


   void PicoMotorsPage::AddAvailableActions(std::vector<std::string>* p)
   {
   p->push_back("RUN");
   }

   void PicoMotorsPage::on_action(const std::string& s)
   {
   if(s == "RUN")
   {
      updatePositions();
   }
   }

   bool PicoMotorsPage::RecalculateParameters()
   {
   if(ImmediateUpdates)
      updatePositions();

   return false;
   }
 */
