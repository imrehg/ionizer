#pragma once


class FPGA_TCP_connection;
class ParameterGUI_Base;
class GbE_msg_exchange;

struct vUpdate
{
	unsigned pageID, channel, uV;
};

class Scan_Base;

class FPGA_connection : public QThread
{
Q_OBJECT

public:
FPGA_connection(const std::string& server_name, unsigned short port);
virtual ~FPGA_connection();

double measure_latency();

//remote-parameter functions

//return ID number of named interface
unsigned InterfaceID(const std::string& interface_name);

unsigned NumExp();
std::string ExpName(unsigned i);

unsigned getNumDataChannels(unsigned iExp);
std::string getDataChannelName(unsigned iExp, unsigned iChannel);

unsigned getNumRemoteActions(unsigned iPage);
std::string getRemoteActionName(unsigned iPage, unsigned iAction);
unsigned callRemoteAction(unsigned iPage, unsigned iAction);

void getCurrentClockState(double& n3P0, double& mF);

//number of remote paramters for the named interface
unsigned NumRemoteParams(unsigned id);

std::string DefineRemoteParam(unsigned id, unsigned iParam);

//! return paremater explanation (e.g. tool-tip)
std::string explainRemoteParam(unsigned id, unsigned iParam);

//! get paremater value as string from FPGA
std::string getParamValueString(unsigned id, unsigned iParam);

unsigned RemoteParamType(unsigned iid, unsigned iParam);
std::string RemoteParamName(unsigned iid, unsigned iParam);

//DDS functions
void TestDDS(unsigned nTest);
unsigned GetDDS_FTW(unsigned iDDS);
void SetDDS_FTW(unsigned iDDS, unsigned ftw);

unsigned GetDDS_Phase(unsigned iDDS);
void SetDDS_Phase(unsigned iDDS, unsigned phase);

void ResetDDS(unsigned iDDS);

void SendIRQ(unsigned u);

//TTL output functions
void SetTTL(unsigned high_mask, unsigned low_mask);
void GetTTL(unsigned *high_mask, unsigned *low_mask);

void SetLogicState(unsigned nBit, int s);
unsigned GetLogicState(unsigned nBit);

//Voltage functions (set voltages in uV units)
void rampVoltages(unsigned page_id, unsigned settings_id, unsigned come_back = 0);
void setVoltage(unsigned page_id, unsigned channel, unsigned uV);
void calibrateDAC(unsigned page_id);
void collectVoltageUpdates(bool bCollect, unsigned nSteps = 0, unsigned dwell = 0, unsigned rampBack = 0);


//Motor control functions (angles in arc-minutes)
unsigned GetMotorAngle(unsigned iMotor);
void SetMotorAngle(unsigned iMotor, unsigned angle);

void GenericExperiment(Scan_Base* pScan, unsigned exp_id, unsigned exp_flags, std::vector<double>& channelData);

void SendParam(unsigned iPage, ParameterGUI_Base* param);
void SendParams(unsigned iPage, const std::vector<ParameterGUI_Base*>& params);

const std::vector<unsigned int>& getResultsVector()
{
	return PMT_results;
}

void resetStats();

//! get plot data for plots on a particular GUI page
unsigned numPagePlots(unsigned page_id);
std::valarray<double> getPagePlotData(unsigned page_id, unsigned iPlot);
std::string getPlotLabel(unsigned page_id, unsigned iPlot);

//! change the current ion configuraion (changes which sideband pulses are generated)
void setIonXtal(const std::string& s);

void infoMsg(unsigned msg_type, const char* s);

QReadWriteLock fpga_lock;

signals:
void debugMsg(QString);
void FPGA_ready();

protected:
friend class FPGA_page;

unsigned doXferWaitForAck(GbE_msg_exchange* eX, Scan_Base* pScan = 0);
void DispatchMsg(Scan_Base* pScan, GbE_msg_exchange* eX, unsigned m);

std::string server_name;
unsigned short port;

FPGA_TCP_connection* fpga_tcp;
std::vector<unsigned int> PMT_results;


private:
//rfc = remote function call
unsigned rfc(unsigned msg_type);
unsigned rfc(unsigned msg_type, const std::string& is);
unsigned rfc(unsigned msg_type, unsigned iu);
unsigned rfc(unsigned msg_type, unsigned iu1, unsigned iu2);
unsigned rfc(unsigned msg_type, unsigned iu1, unsigned iu2, unsigned iu3);

std::string rfc_os(unsigned msg_type, unsigned iu0, unsigned iu1);

void run();
};

unsigned int us2TW(double t);
unsigned int ms2TW(double t);
unsigned int MHz2FTW(double f);
unsigned int Hz2FTW(double f);

double   FTW2MHz(unsigned int ftw);
double   FTW2Hz(unsigned int ftw);
