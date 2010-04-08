#pragma once

#include "messaging.h"

//class to handle low-level details of the persistent FPGA TCP connection
//slot SendFPGAMessage gets called to do the actual message exchange with the FPGA
class FPGA_TCP_connection : public QObject
{
Q_OBJECT
public:
FPGA_TCP_connection(const std::string& server_name, unsigned short port);
virtual ~FPGA_TCP_connection();

unsigned socket_state();
bool is_connected();
void connect_to_fpga();

public slots:
//exchange messages with the FPGA
//msg_out and msg_in must remain valid during the function call.
//A "Qt::BlockingQueuedConnection" achieves this.
void SendFPGAMessage(GbE_msg* msg_out, std::vector<GbE_msg>* msg_in);

protected:
std::string server_name;
unsigned short port;

unsigned msg_id;
QTcpSocket tcp_sock;
};

//class to handle low-level details of an FPGA message exchange
class GbE_msg_exchange : public QObject
{
Q_OBJECT

public:
GbE_msg_exchange(QReadWriteLock* fpga_lock, FPGA_TCP_connection* fpga_tcp, unsigned out_what);

~GbE_msg_exchange();

unsigned reply(unsigned i);
void setOutMsg(unsigned what);

void insertOutU(unsigned loc, unsigned u);
char* sOut();

unsigned doXfer();
unsigned doXferNoReply();

//return reply as a null-terminated string
const char* sIn();

GbE_msg* get_msg_in()
{
	return &(msg_in.at(0));
}

protected:
bool bConvertByteOrder;

GbE_msg msg_out;
std::vector<GbE_msg> msg_in;
QWriteLocker fpga_locker;

signals:
void NewFPGAMessage(GbE_msg* msg_out, std::vector<GbE_msg>* msg_in);
};


