#ifndef SIMULATOR_H_
#define SIMULATOR_H_

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "messaging.h"

class simulator;

class simulator_connection : public QObject
{
	Q_OBJECT
public:
	simulator_connection(QTcpServer* tcp_server, simulator* sim);
	~simulator_connection();
	
	unsigned write_GbE(const GbE_msg* msg);
	unsigned read_GbE(GbE_msg* msg);

public slots:
	void on_newConnection();
	void on_readyRead();
	
protected:
	QTcpServer* tcp_server;
	QTcpSocket* tcp_sock;	
	
	simulator* sim;	
	
	GbE_msg msg_in, msg_out;
};

class simulator
{
public:
	simulator();
};

void SetDAC(unsigned iChannel, unsigned dacWord);

extern simulator_connection* sim_con;

#endif /*SIMULATOR_H_*/
