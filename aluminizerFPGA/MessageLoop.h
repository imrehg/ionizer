#ifndef _MESSAGE_LOOP_H_
#define _MESSAGE_LOOP_H_

extern "C" { void* socket_process_thread(void* arg); }

#include <vector>

class GbE_msg;

int process_message(const GbE_msg& msg_in, GbE_msg& msg_out, bool bDebug);
void message_loop(int s);

//void free_message(GbE_msg* p_msg);
//void grow_message(GbE_msg* p_msg);

#endif //_MESSAGE_LOOP_H_
