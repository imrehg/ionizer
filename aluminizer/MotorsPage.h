#pragma once

using namespace std;

#include "ExperimentPage.h"
#include "ScanSource.h"
#include "RS232device.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

class AgilisMotorsPage;

class MotorsPage : public ParamsPage
{
Q_OBJECT

public:
MotorsPage(const string& sPageName, ExperimentsSheet* pSheet);
virtual ~MotorsPage();

void setMotorPosition(int, unsigned);
void setMgPI(bool);

virtual bool RecalculateParameters();

protected slots:
void replyFinished(QNetworkReply*);

protected:

void SetAngle(int i, double a);
double GetAngle(int i);

void AddAvailableActions(std::vector<std::string>*);
void on_action(const std::string&);

virtual unsigned num_columns();

int status_2042_2034();
int status_2108_2034();

protected:
unsigned nMotors;

vector<GUI_string*>     Names;
vector<GUI_double_no_label*>  Angles;
vector<GUI_double*>  Pos0;
vector<GUI_double*>  Pos1;

QNetworkAccessManager manager;
GUI_int MgPI_2108, MgPI_2042;
GUI_string MgPI_status;
};

/*
   class PicoMotorsPage : public ParamsPage
   {
   public:
   PicoMotorsPage(const string& sPageName, ExperimentsSheet* pSheet);
   virtual ~PicoMotorsPage();

   virtual bool RecalculateParameters();
   void AddAvailableActions(std::vector<std::string>* p);
   void on_action(const std::string& s);

   std::string getName(unsigned i) { return Names.at(i)->Value(); }
   void setScanOutput(unsigned i, double d) { Positions.at(i)->SetValue(d); updatePositions(); }
   double getScanOutput(unsigned i) { return Positions.at(i)->Value(); }

   protected:

   virtual unsigned num_columns();

   void updatePositions();
   void setDriveChannel(unsigned iMotor);
   void driveTo(unsigned iMotor, double pos);
   std::string sendCmd(const std::string& s);

   protected:
   unsigned nMotors;

   vector<GUI_string*>					Names;
   vector<GUI_double_no_label*>		Positions, Velocities;
   vector<double>						InternalPos, InternalV;
   GUI_string							Response;
   GUI_double							Asymmetry;
   GUI_bool							ImmediateUpdates;

   RS232device rs232;
   unsigned iCurrentMotor;
   };
 */