#include <QtCore>
#include <QCoreApplication>
#include <QTcpServer>

#include "simulator.h"
#include "common.h"
#include "config_local.h"

simulator_connection* sim_con;

int main(int argc, char *argv[])
{
   QCoreApplication a(argc, argv);
   unsigned short port = 6007;

   simulator sim;
   QTcpServer tcp_server;

   init_remote_interfaces();

   sim_con = new simulator_connection(&tcp_server, &sim);

   tcp_server.listen(QHostAddress::Any, port);
   QObject::connect(&tcp_server, SIGNAL(newConnection()), sim_con, SLOT(on_newConnection()));

   return a.exec();
}
