#ifndef HOST_INTERFACE_H
#define HOST_INTERFACE_H

#include "shared/src/messaging.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include "MessageLoop.h"

extern bool bDebugPulses;

class experiment;

//! class to perform communications with the host PC
class host_interface
{
public:
host_interface(int s) : s(s), irq(0), id(0), currDebugMsgLength(0)
{
}

void sendInfoMsg(unsigned msg_type, const char* s);
void sendDebugMsg(const char* str, bool force_flush = false);
void flushDebugMsg();

void updateGUI_param(unsigned page_id, const char* sName, const char* sValue);

unsigned buffDebugSize() const
{
	return 4 * (MSG_STD_PAYLOAD_SIZE);
}

protected:

void send_message(GbE_msg* p_msg_out, GbE_msg* p_msg_in);
void send_msg(unsigned what, experiment* exp = 0);

int s;
unsigned irq;
unsigned id;

public:
char buffDebug[4 * (MSG_STD_PAYLOAD_SIZE)];
char currDebugMsg[4 * (MSG_STD_PAYLOAD_SIZE)];
unsigned currDebugMsgLength;
};

extern host_interface* host;

#endif //HOST_INTERFACE_H
