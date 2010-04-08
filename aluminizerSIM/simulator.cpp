#include "simulator.h"
#include "MessageLoop.h"
#include "host_interface.h"

#include <malloc.h>
#include <vector>
#include <iostream>

#ifndef ALUMINIZER_SIM
#include <arpa/inet.h>
#endif

using namespace std;

host_interface* host;

extern QTcpSocket* pGbE_msg_TCP;

simulator_connection::simulator_connection(QTcpServer* tcp_server, simulator* sim) :
	tcp_server(tcp_server),
	sim(sim)
{
        unsigned msgSize = MIN_MSG_LENGTH;
        printf("Message length = %d bytes\r\n", msgSize);

	host = new host_interface(0);
}
	
simulator_connection::~simulator_connection()
{
	delete host;
	host=0;
}


void simulator_connection::on_newConnection()
{
        printf("Accepting new connection.\r\n");

	tcp_sock = tcp_server->nextPendingConnection();
	pGbE_msg_TCP = tcp_sock;
	QObject::connect(tcp_sock, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
}

void simulator_connection::on_readyRead()
{
	GbE_msg msg_in;
	GbE_msg msg_out;

	//read in linked-list of messages
	//printf("reading message ... \r\n");
	msg_in.rcv();
	
	unsigned iQuit = process_message(msg_in, msg_out, false);

	msg_out.snd();

   	 if(iQuit)
	 {
		 printf("Closing socket.\r\n");
   	 	 tcp_sock->close();
	 }
}


simulator::simulator()
{
}

void SetDAC(unsigned iChannel, unsigned dacWord)
{
	cout << "DAC<" << iChannel << "> = " << (0.05 + (((double)dacWord)/0x3FFF) * 2.95) * 3.52 - 0.05 * 2.52 << endl;
}
