#include "xmk.h"
#include "xtime_l.h"

#include <pthread.h>
#include <stdio.h>
#include <math.h>

void disable_interrupts()
{
	//PIT = programmable interval timer
	//The Xilinx kernel uses this to schedule switches between threads.
	//see xtime_l.h, timer_int_handler.c, ppc_hw.c
	XTime_PITDisableInterrupt();
}

void enable_interrupts()
{
	XTime_PITEnableInterrupt();
}

//0 = highest priority, 1 = DEFAULT_THREAD_PRIO
void set_priority(int prio)
{
	pthread_t this_thread;
	sched_param sp;

	this_thread = pthread_self();
	sp.sched_priority = prio;
	pthread_setschedparam(this_thread, 0, &sp);
}

int get_priority()
{
	pthread_t this_thread;
	sched_param sp;

	int policy = 0; //not sure what this paramter does

	this_thread = pthread_self();
	pthread_getschedparam(this_thread, &policy, &sp);

	return sp.sched_priority;
}

#include <sys/process.h>

void yield_execution()
{
	yield();
}
