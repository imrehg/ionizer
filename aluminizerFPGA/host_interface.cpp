#include "host_interface.h"

using namespace std;

#include "experiments.h"

void host_interface::send_message(GbE_msg* msg_out, GbE_msg* msg_in)
{
	msg_out->id = id++;
	msg_out->snd();
	msg_in->rcv();
}

void host_interface::send_msg(unsigned what, experiment*)
{
	GbE_msg msg_out;

	msg_out.what = what;

	GbE_msg msg_in;
	send_message(&msg_out, &msg_in);

	while (msg_in.what != C2S_ACK)
	{
		process_message(msg_in, msg_out, false);
		send_message(&msg_out, &msg_in);
	}
}

void host_interface::sendDebugMsg(const char* str, bool force_flush)
{
	if (bDebugPulses)
	{
		const char* p = str;

		do
		{
			unsigned L = MSG_STD_PAYLOAD_SIZE * 4;
			unsigned spaceAvailable = L - currDebugMsgLength - 1;

			unsigned nToCopy = std::min<unsigned>(strlen(p), spaceAvailable);
			strncpy(currDebugMsg + currDebugMsgLength, p, nToCopy);

			p += nToCopy;
			currDebugMsgLength += nToCopy;

			if (currDebugMsgLength == (L - 1))
				flushDebugMsg();
		}
		while (strlen(p));

		if (force_flush)
			flushDebugMsg();
	}
}

void host_interface::flushDebugMsg()
{
	GbE_msg msg_out;

	msg_out.what = S2C_DEBUG_MESSAGE;
	strncpy((char*)(msg_out.extractS(0)), currDebugMsg, currDebugMsgLength);
	((char*)(msg_out.extractS(0)))[currDebugMsgLength] = 0; //null-terminate the string

	GbE_msg msg_reply;
	send_message(&msg_out, &msg_reply);

	currDebugMsgLength = 0;
}

void host_interface::sendInfoMsg(unsigned msg_type, const char* s)
{
	GbE_msg msg_out;

	msg_out.what = S2C_INFO_MESSAGE;
	msg_out.insertU(0, msg_type);
	msg_out.insertS(1, s);

	GbE_msg msg_reply;
	send_message(&msg_out, &msg_reply);
}

void host_interface::updateGUI_param(unsigned page_id, const char* sName, const char* sValue)
{
	GbE_msg msg_out;

	msg_out.what = S2C_UPDATE_PARAM;
	msg_out.insertU(0, page_id);
	msg_out.insertU(1, 1); //one update
	msg_out.insert2S(2, sName, sValue);

	GbE_msg msg_reply;
	send_message(&msg_out, &msg_reply);
}
