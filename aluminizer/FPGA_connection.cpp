#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <QMutex>
#include <QString>
#include <QStringList>

#include "AluminizerApp.h"
#include "CommonExperimentsSheet.h"

#include "FPGA_connection.h"
#include "FPGA_TCP.h"
#include "ParameterGUI_Base.h"
#include "ScanObject.h"

using namespace std;

FPGA_connection::FPGA_connection(const std::string& server_name, unsigned short port) :
#ifdef RECURSIVE_QT_LOCKS
	fpga_lock(QReadWriteLock::Recursive),
#else
	fpga_lock(),
#endif
	server_name(server_name),
	port(port)
{
//	start(QThread::TimeCriticalPriority);
}

FPGA_connection::~FPGA_connection()
{
	//tell FPGA that we are shutting down
	rfc(C2S_QUIT);

	//finish the event loop
	QThread::exit(0);

	//wait for thread to finish
	wait(ULONG_MAX);
}


unsigned FPGA_connection::rfc(unsigned msg_type)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, msg_type);

	doXferWaitForAck(&eX);
	return eX.reply(0);
}

unsigned FPGA_connection::rfc(unsigned msg_type, const std::string& is)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, msg_type);

	eX.insertOutU(0, is.length());
	strcpy(eX.sOut() + sizeof(unsigned), is.c_str());
	doXferWaitForAck(&eX);
	return eX.reply(0);
}

unsigned FPGA_connection::rfc(unsigned msg_type, unsigned iu)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, msg_type);

	eX.insertOutU(0, iu);
	doXferWaitForAck(&eX);
	return eX.reply(0);
}

unsigned FPGA_connection::rfc(unsigned msg_type, unsigned iu0, unsigned iu1)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, msg_type);

	eX.insertOutU(0, iu0);
	eX.insertOutU(1, iu1);

	doXferWaitForAck(&eX);
	return eX.reply(0);
}

unsigned FPGA_connection::rfc(unsigned msg_type, unsigned iu0, unsigned iu1, unsigned iu2)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, msg_type);

	eX.insertOutU(0, iu0);
	eX.insertOutU(1, iu1);
	eX.insertOutU(2, iu2);

	doXferWaitForAck(&eX);
	return eX.reply(0);
}

std::string FPGA_connection::rfc_os(unsigned msg_type, unsigned iu0, unsigned iu1)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, msg_type);

	eX.insertOutU(0, iu0);
	eX.insertOutU(1, iu1);
	doXferWaitForAck(&eX);

	return std::string(eX.sIn());
}


double FPGA_connection::measure_latency()
{
	if (!fpga_tcp->is_connected())
		return -1;

	double t0 = CurrentTime_s();
	int N = 1000;

	for (int i = 0; i < N; i++)
		GetLogicState(0);

	double dt = (CurrentTime_s() - t0) / N;

	printf("round-trip latency: %3.2f ms\r\n", dt * 1e3);
	return dt;
}


unsigned int us2TW(double t)
{
	return static_cast<unsigned int>(t * 100);
}


unsigned int ms2TW(double t)
{
	return us2TW(t * 1e3);
}


unsigned int Hz2FTW(double f)
{
	return static_cast<unsigned int>(floor(0.5 + (f * pow(2., 32.) / 1e9)));
}

unsigned int MHz2FTW(double f)
{
	return Hz2FTW(f * 1e6);
}

double FTW2Hz(unsigned int ftw)
{
	return ftw * 1.e9 * pow(2., -32.);
}

double FTW2MHz(unsigned int ftw)
{
	return FTW2Hz(ftw) * 1e-6;
}

void FPGA_connection::SetLogicState(unsigned nBit, int s)
{
	if (nBit > 31)
		throw runtime_error("[FPGA_connection::SetLogicState] bad bit number");

	unsigned high_mask, low_mask;
	GetTTL(&high_mask, &low_mask);

	if (s == 0)
	{
		high_mask &= ~(1 << nBit);
		low_mask  |=  (1 << nBit);
	}

	if (s == 1)
	{
		high_mask |=  (1 << nBit);
		low_mask  &= ~(1 << nBit);
	}

	if (s == 2)
	{
		high_mask &= ~(1 << nBit);
		low_mask  &= ~(1 << nBit);
	}

	SetTTL(high_mask, low_mask);
}


unsigned FPGA_connection::GetLogicState(unsigned nBit)
{
	unsigned high_mask, low_mask;

	GetTTL(&high_mask, &low_mask);

	unsigned bit_mask = 1 << nBit;

	if (bit_mask & low_mask)
		return 0;

	if (bit_mask & high_mask)
		return 1;

	return 2;
}

void FPGA_connection::SendIRQ(unsigned u)
{
	rfc(C2S_SEND_IRQ, u);
}

void FPGA_connection::SetTTL(unsigned high_mask, unsigned low_mask)
{
	rfc(C2S_SET_TTL, high_mask, low_mask);
}

void FPGA_connection::GetTTL(unsigned *high_mask, unsigned *low_mask)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, C2S_GET_TTL);

	doXferWaitForAck(&eX);
	*high_mask = eX.reply(0);
	*low_mask = eX.reply(1);
}


unsigned FPGA_connection::GetDDS_FTW(unsigned iDDS)
{
	return rfc(C2S_GET_DDS_FREQ, iDDS);
}

unsigned FPGA_connection::GetDDS_Phase(unsigned iDDS)
{
	return rfc(C2S_GET_DDS_PHASE, iDDS);
}

void FPGA_connection::ResetDDS(unsigned iDDS)
{
	rfc(C2S_RESET_DDS, iDDS);
}

void FPGA_connection::TestDDS(unsigned nTest)
{
	rfc(C2S_TEST_DDS, nTest);
}

void FPGA_connection::SetDDS_FTW(unsigned iDDS, unsigned ftw)
{
	rfc(C2S_SET_DDS_FREQ, iDDS, ftw);
}

void FPGA_connection::SetDDS_Phase(unsigned iDDS, unsigned phase)
{
	rfc(C2S_SET_DDS_PHASE, iDDS, phase);
}

void FPGA_connection::SendParam(unsigned iPage, ParameterGUI_Base* param)
{
	QWriteLocker l(&fpga_lock);

	if (param->getFlags() & RP_FLAG_BINARY_XFER)
	{
		GbE_msg_exchange eX(&fpga_lock, fpga_tcp, C2S_CHANGE_PARAM_BIN);

		unsigned l = param->get_binary_length();

		if ((l + 12) >= MAX_STR_LENGTH)
			throw runtime_error("[FPGA_connection::SendParam] binary msg is too long: " + param->get_fpga_name());

		if (l == 0)
			throw runtime_error("[FPGA_connection::SendParam] can't send binary: " + param->get_fpga_name());

		//binary param xfer format:
		//m[0]: iPage, m[1]: iParam, m[2]: data length m[3...]: binary data
		eX.insertOutU(0, iPage);
		eX.insertOutU(1, param->get_fpga_id());
		eX.insertOutU(2, l);
		param->insert_binary(eX.sOut() + 3 * 4);

		doXferWaitForAck(&eX);
	}
	else
	{
		vector<ParameterGUI_Base*> params(1);
		params[0] = param;

		SendParams(iPage, params);
	}
}

unsigned FPGA_connection::getNumDataChannels(unsigned iPage)
{
	return rfc(C2S_NUM_DATA_CHANNELS, iPage);
}

std::string FPGA_connection::getDataChannelName(unsigned iPage, unsigned iChannel)
{
	return rfc_os(C2S_DATA_CHANNEL_NAME, iPage, iChannel);
}

unsigned FPGA_connection::getNumRemoteActions(unsigned iPage)
{
	return rfc(C2S_NUM_REMOTE_ACTIONS, iPage);
}

std::string FPGA_connection::getRemoteActionName(unsigned iPage, unsigned iAction)
{
	//Main thread hangs here.
	//Scan thread is waiting for experiment data.
	return rfc_os(C2S_REMOTE_ACTION_NAME, iPage, iAction);
}

unsigned FPGA_connection::callRemoteAction(unsigned iPage, unsigned iAction)
{
	return rfc(C2S_CALL_REMOTE_ACTION, iPage, iAction);
}

void FPGA_connection::getCurrentClockState(double& n3P0, double& mF)
{
	string s = rfc_os(C2S_GET_AL_STATE, 0, 0);

	int mFi;

	sscanf(s.c_str(), "state=%lf mF=%d", &n3P0, &mFi);

	mF = mFi / 2.0;
}

void FPGA_connection::SendParams(unsigned iPage, const std::vector<ParameterGUI_Base*>& params)
{
	QWriteLocker fpga_locker(&fpga_lock);

	//first do all binary transfers separately
	for (size_t i = 0; i < params.size(); i++)
	{
		ParameterGUI_Base* pParam = params[i];

		if (pParam->getFlags() & RP_FLAG_BINARY_XFER)
			SendParam(iPage, pParam);
	}

	GbE_msg_exchange* eX = 0;

	char* p0 = 0;
	char* p = 0;
	char* p_end = 0;
	unsigned numParams = 0;

	for (size_t i = 0; i < params.size(); i++)
	{
		ParameterGUI_Base* pParam = params[i];

		if (pParam->getFlags() & RP_FLAG_BINARY_XFER)
			continue;

		string name = pParam->get_fpga_name();
		string value = pParam->getValueString();
		//	string value = params[i]->GetGUIString();


		if (p && p_end && eX)
		{
			if ( (p + name.length() + value.length() + 2) >= p_end )
			{
				eX->insertOutU(0, numParams);
				eX->insertOutU(1, iPage);

				doXferWaitForAck(eX);

				delete eX;

				eX = 0;
				p = 0;
				p_end = 0;
			}
		}

		if (eX == 0)
		{
			eX = new GbE_msg_exchange(&fpga_lock, fpga_tcp, C2S_CHANGE_PARAMS);

			numParams = 0;
			p = (char*)(eX->sOut() + 2 * sizeof(unsigned));
			p0 = p;
			p_end = (char*)(eX->sOut() + MSG_STD_PAYLOAD_SIZE * sizeof(unsigned));
		}

		if ( (p + name.length() + value.length() + 2) < p_end )
		{
			strcpy(p, name.c_str());
			p += strlen(p) + 1;

			strcpy(p, value.c_str());
			p += strlen(p) + 1;

			numParams++;

			//	 printf("[FPGA_connection::SendParams] %s = %s (%d bytes total)\n", name.c_str(), value.c_str(), p-p0);
		}
		else
			printf("[FPGA_connection::SendParams] overflow error:\n %s = %s\n", name.c_str(), value.c_str());
	}

	if (eX)
	{
		eX->insertOutU(0, numParams);
		eX->insertOutU(1, iPage);

		doXferWaitForAck(eX);

		delete eX;
	}
}

unsigned FPGA_connection::doXferWaitForAck(GbE_msg_exchange* eX, Scan_Base* pScan)
{
	QWriteLocker fpga_locker(&fpga_lock);

	unsigned reply = eX->doXfer();

	while (reply != S2C_OK)
	{
		DispatchMsg(pScan, eX, reply);
		reply = eX->doXfer();

		if (reply == S2C_ERROR)
		{
			creepy_speak(string(eX->sIn()));
			throw runtime_error("[FPGA_connection::doXferWaitForAck] S2C_ERROR:\r\n" + string(eX->sIn()));
		}
	}

	return reply;
}

void FPGA_connection::infoMsg(unsigned msg_type, const char* s)
{
	printf("[FPGA INFO] %s\r\n", s);

	if (msg_type == IM_VOICE)
		creepy_speak(string(s));
}

void FPGA_connection::DispatchMsg(Scan_Base* pScan, GbE_msg_exchange* eX, unsigned m)
{
	QWriteLocker fpga_locker(&fpga_lock);

	switch (m)
	{
	case S2C_OK: break;
	case S2C_DEBUG_MESSAGE: assert(pScan); pScan->debugMsg(eX->sIn()); break;
	case S2C_INFO_MESSAGE: infoMsg(eX->get_msg_in()->extractU(0), eX->get_msg_in()->extractS(1)); break;
	case S2C_ERROR: creepy_speak(string(eX->sIn())); throw runtime_error("[FPGA_connection::GenericExperiment] S2C_ERROR:\r\n" + string(eX->sIn()));
	default: if (gSheet) gSheet->DispatchMsg(eX, m);
	}

	eX->setOutMsg(C2S_ACK);
}

void FPGA_connection::GenericExperiment(Scan_Base* pScan, unsigned exp_id, unsigned exp_flags, std::vector<double>& channelData)
{
	QWriteLocker fpga_locker(&fpga_lock);

	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, C2S_RUN_EXP);

	eX.insertOutU(0, exp_id);
	eX.insertOutU(1, exp_flags);

	doXferWaitForAck(&eX, pScan);

	//ignore the data if this was a "debug" experiment
	if ( !(exp_flags & EXP_FLAG_DEBUG) )
	{
		//otherwise copy FPGA data into results "channels"
		unsigned nChannels = eX.reply(0);
		channelData.resize(nChannels);

		for (unsigned i = 0; i < nChannels; i++)
			channelData[i] = (eX.reply(1 + i) / 1000.0 - 1000000);

		unsigned nShots = eX.reply(nChannels + 1);
		unsigned i0 = nChannels + 2;
		double sum = 0;

		PMT_results.resize(nShots);
		for (size_t i = 0; i < nShots; i++)
		{
			PMT_results[i] = eX.reply(i + i0);
			sum += PMT_results[i];
		}
	}
}

unsigned FPGA_connection::NumExp()
{
	return rfc(C2S_NUM_EXP);
}

std::string FPGA_connection::ExpName(unsigned i)
{
	return rfc_os(C2S_EXP_NAME, i, 0);
}

//number of remote paramters for the named interface
unsigned FPGA_connection::NumRemoteParams(unsigned id)
{
	return rfc(C2S_NUM_EXP_PARAMS, id);
}

std::string FPGA_connection::DefineRemoteParam(unsigned id, unsigned iParam)
{
	return rfc_os(C2S_DEFINE_EXP_PARAM, id, iParam);
}

std::string FPGA_connection::explainRemoteParam(unsigned id, unsigned iParam)
{
	return rfc_os(C2S_EXPLAIN_EXP_PARAM, id, iParam);
}

//! get paremater value as string from FPGA
std::string FPGA_connection::getParamValueString(unsigned id, unsigned iParam)
{
	return rfc_os(C2S_GET_PARAM_VAL_STR, id, iParam);
}

unsigned FPGA_connection::RemoteParamType(unsigned iid, unsigned iParam)
{
	return rfc(C2S_RP_TYPE, iid, iParam);
}

std::string FPGA_connection::RemoteParamName(unsigned iid, unsigned iParam)
{
	return rfc_os(C2S_RP_NAME, iid, iParam);
}

void FPGA_connection::run()
{
	fpga_tcp = new FPGA_TCP_connection(server_name, port);

	emit FPGA_ready();

	//run the event loop
	QThread::exec();

	delete fpga_tcp;
}

//Motor control functions (angles in arc-minutes)
unsigned FPGA_connection::GetMotorAngle(unsigned iMotor)
{
	return rfc(C2S_GET_MOTOR_ANGLE, iMotor);
}

void FPGA_connection::SetMotorAngle(unsigned iMotor, unsigned angle)
{
	rfc(C2S_SET_MOTOR_ANGLE, iMotor, angle);
}

void FPGA_connection::resetStats()
{
	rfc(C2S_RESET_STATS);
}

unsigned FPGA_connection::numPagePlots(unsigned page_id)
{
	return rfc(C2S_NUM_DATA_PLOTS, page_id);
}

std::valarray<double> FPGA_connection::getPagePlotData(unsigned page_id, unsigned iPlot)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, C2S_GET_PLOT_DATA);

	eX.insertOutU(0, page_id);
	eX.insertOutU(1, iPlot);
	eX.insertOutU(2, 0);

	doXferWaitForAck(&eX);

	valarray<double> v(eX.reply(0));

	for (unsigned i = 0; i < v.size(); i++)
		v[i] = ((int)(eX.reply(i + 1))) * 0.001;

	return v;
}

std::string FPGA_connection::getPlotLabel(unsigned page_id, unsigned iPlot)
{
	string s = "";
	s = rfc_os(C2S_HIST_NAME, page_id, iPlot);
	return s;
}

void FPGA_connection::calibrateDAC(unsigned page_id)
{
	rfc(C2S_SET_DAC_CALIBRATION, page_id);
}


void FPGA_connection::rampVoltages(unsigned page_id, unsigned settings_id, unsigned come_back)
{
	GbE_msg_exchange eX(&fpga_lock, fpga_tcp, C2S_RAMP_VOLTAGES);

	unsigned j = 0;

	eX.insertOutU(j++, page_id);
	eX.insertOutU(j++, settings_id);
	eX.insertOutU(j++, come_back);

	doXferWaitForAck(&eX);
}

void FPGA_connection::setIonXtal(const std::string& s)
{
	rfc(CS2_SET_ION_XTAL, s);
}

