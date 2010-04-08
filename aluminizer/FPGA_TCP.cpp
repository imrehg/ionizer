#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "FPGA_TCP.h"

//#ifndef WIN32
//#include <arpa/inet.h>
//#endif

extern QTcpSocket* pGbE_msg_TCP;
using namespace std;


unsigned FPGA_TCP_connection::socket_state()
{
	return tcp_sock.state();
}

bool FPGA_TCP_connection::is_connected()
{
	return tcp_sock.state() == QAbstractSocket::ConnectedState;
}

class SleepHelper : public QThread
{
public:
static void msleep(int ms)
{
	QThread::msleep(ms);
}
};


FPGA_TCP_connection::FPGA_TCP_connection(const std::string& server_name, unsigned short port) :
	server_name(server_name),
	port(port),
	msg_id(0),
	tcp_sock()
{
	printf("message_length = %u \r\n", (unsigned)MIN_MSG_LENGTH);
	connect_to_fpga();
}

FPGA_TCP_connection::~FPGA_TCP_connection()
{
	tcp_sock.close();
}


void FPGA_TCP_connection::SendFPGAMessage(GbE_msg* msg_out, std::vector<GbE_msg>* msg_in)
{
	pGbE_msg_TCP = &(this->tcp_sock);

	//make sure there is a connection
	while (!tcp_sock.waitForConnected(200) )
	{
		printf("Connecting to %s \n", server_name.c_str());
		SleepHelper::msleep(1000);

		if (tcp_sock.state() != QAbstractSocket::ConnectingState)
			if (!is_connected())
				connect_to_fpga();
	}

	msg_out->id = msg_id++;
	msg_out->snd();

	//wait for reply
	if (msg_in)
	{
		GbE_msg* p_msg_in = &((*msg_in)[0]);
		p_msg_in->rcv();
	}
}

void FPGA_TCP_connection::connect_to_fpga()
{
	cout << "connecting to FPGA at address: " << server_name << endl;
	QHostAddress ha(QString(server_name.c_str()));
	tcp_sock.connectToHost(ha, port);
}


GbE_msg_exchange::GbE_msg_exchange(QReadWriteLock* fpga_lock, FPGA_TCP_connection* fpga_tcp, unsigned out_what) :
	msg_in(1),
	fpga_locker(fpga_lock)
{
	QObject::connect(this, SIGNAL(NewFPGAMessage(GbE_msg *, std::vector<GbE_msg>*)),
	                 fpga_tcp, SLOT(SendFPGAMessage(GbE_msg *, std::vector<GbE_msg>*)),
	                 Qt::BlockingQueuedConnection);


	msg_out.what = out_what;
}

GbE_msg_exchange::~GbE_msg_exchange()
{
}

void GbE_msg_exchange::setOutMsg(unsigned what)
{
	msg_out.what = what;
}

unsigned GbE_msg_exchange::reply(unsigned i)
{
	unsigned j = i / MSG_STD_PAYLOAD_SIZE;
	unsigned k = i % MSG_STD_PAYLOAD_SIZE;

	return msg_in.at(j).extractU(k);
}

void GbE_msg_exchange::insertOutU(unsigned loc, unsigned u)
{
	assert(loc < MSG_STD_PAYLOAD_SIZE);
	msg_out.insertU(loc, u);
}


unsigned GbE_msg_exchange::doXfer()
{
	//Inform fpga_tcp that there's new data waiting.
	//Blocks until the messaging is done.
	//msg_in contains the FPGA's reply to msg_out
	emit NewFPGAMessage(&msg_out, &msg_in);

	return msg_in[0].what;
}

//return reply as a null-terminated string
const char* GbE_msg_exchange::sIn()
{
	return msg_in[0].extractS(0);
}


char* GbE_msg_exchange::sOut()
{
	return msg_out.extractS(0);
}

