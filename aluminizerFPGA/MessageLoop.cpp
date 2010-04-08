/* Xilinx Includes */
#include "xexception_l.h"
#include "xtime_l.h"
#include "xcache_l.h"
#include "xparameters.h"
#include "xintc.h"
//#include "xtemac_l.h"
#include "xtime_l.h"

/* lwIP Includes */
//#include "netif/xtemacif.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include <cstdio>
#include <stdexcept>
#include <sleep.h>

#include "MessageLoop.h"
#include "host_interface.h"

using namespace std;

host_interface* host;

extern int GbE_socket;

void message_loop(int s)
{
	GbE_socket = s;

	GbE_msg msg_in;
	GbE_msg msg_out;

	int iQuit = 0;

	host = new host_interface(s);

	usleep(1000000);
	printf("\nsocket_process_thread() entry\n");
	printf("message_length = %u bytes\n", MIN_MSG_LENGTH);

	try
	{
		while (iQuit == 0)
		{
			unsigned nRead = msg_in.rcv();

			if (nRead == MIN_MSG_LENGTH)
			{
				iQuit = process_message(msg_in, msg_out, false);
				msg_out.snd();
			}
		}
	}
	catch (runtime_error e)
	{
		printf("RUNTIME ERROR: %s\n", e.what());
	}

	delete host;

	print("SOCKET: Closing socket...");
	close(s);
	print("done.\n");
}
