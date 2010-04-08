#ifdef ALUMINIZER_SIM
   #define XPAR_PULSE_CONTROLLER_0_BASEADDR (0)
   #include "pulse_controller.h"
#else
   extern "C"
   {
   #include "xparameters.h"
   #include "pulse_controller.h"
   }
#endif

extern void* pulser;

#include "pulse_controller.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "dds_pulse.h"

#define PULSE_CONTROLLER_NDDS (8)

unsigned dds_ftw[PULSE_CONTROLLER_NDDS];
unsigned dds_phase[PULSE_CONTROLLER_NDDS];

unsigned PMT_val = 0;

unsigned TTL_low_mask = 0;
unsigned TTL_high_mask = 0;

#include "sim_ions.h"


sim_ions sim(8);

unsigned PULSE_CONTROLLER_get_PMT(void*)
{
   return sim.pop_result();
}

//TTL functions
void PULSE_CONTROLLER_set_ttl(void*, unsigned high_mask, unsigned low_mask)
{
//	printf("[SET TTL] low_mask = %08X  high_mask = %08X\r\n", low_mask, high_mask);
   TTL_low_mask = low_mask;
   TTL_high_mask = high_mask;
}

void PULSE_CONTROLLER_get_ttl(void*, unsigned* high_mask, unsigned* low_mask)
{
//	printf("[GET TTL] low_mask = %08X  high_mask = %08X\r\n", TTL_low_mask, TTL_high_mask);
   *low_mask = TTL_low_mask;
   *high_mask = TTL_high_mask;
}

//DDS functions
void PULSE_CONTROLLER_dds_reset(void*, char i)
{
   if(i < PULSE_CONTROLLER_NDDS)
   {
      dds_ftw[(unsigned)i] = 0;
      dds_phase[(unsigned)i] = 0;
   }
}

void PULSE_CONTROLLER_set_dds_div2(void*, char, int)
{
}

void PULSE_CONTROLLER_set_dds_freq(void*, char i, unsigned int freq)
{
   if(i < PULSE_CONTROLLER_NDDS)
   {
      dds_ftw[(unsigned)i] = freq;
      sim.SetDDSFrequency(i, FTW2Hz(freq));
   }
}

void PULSE_CONTROLLER_set_dds_phase(void*, char i, unsigned short phase)
{
   if(i < PULSE_CONTROLLER_NDDS)
   {
      dds_phase[(unsigned)i] = phase;
      sim.SetDDSPhase(i, phase);

   //	printf("DDS(%u) ptw = %u (%6.3f deg.)\n", (unsigned)i, phase * 360.0 /pow(2.0,-14));

   }
}

unsigned int PULSE_CONTROLLER_get_dds_freq(void*, char i)
{
   if(i < PULSE_CONTROLLER_NDDS)
      return dds_ftw[(unsigned)i];
   else
      return 0;
}

unsigned int PULSE_CONTROLLER_get_dds_phase(void*, char i)
{
   if(i < PULSE_CONTROLLER_NDDS)
      return dds_ftw[(unsigned)i];
   else
      return 0;
}

unsigned PULSE_CONTROLLER_pulse(void* base_addr, unsigned t, const unsigned flags, const unsigned operand)
{
   unsigned t_max = 0x001FFFFF;
   unsigned t_big = 0x001FFFF0;

   unsigned nPulses = 1;

   while(t > t_max)
   {
      PULSE_CONTROLLER_short_pulse(base_addr, t_big | flags, operand);
      t -= t_big;
      nPulses++;
   }

   PULSE_CONTROLLER_short_pulse(base_addr, t | flags, operand);

   return nPulses;
}

void PULSE_CONTROLLER_short_pulse(void*, const unsigned control, const unsigned operand)
{
   unsigned t = control & 0x001FFFFF;
   unsigned flags = control & 0xFF000000;

   sim.pulse(t * 1e-8, flags, operand);
}

void PULSE_CONTROLLER_enable_timing_check(void*)
{
}

void PULSE_CONTROLLER_disable_timing_check(void*)
{
}

void PULSE_CONTROLLER_clear_timing_check(void*)
{
}

int PULSE_CONTROLLER_timing_ok(void*)
{
   return 1;
}

void PULSE_CONTROLLER_self_test(void*, int)
{
}

void PULSE_CONTROLLER_reinit_DDS(void*, unsigned)
{
}

void (*idle_func)(void) = 0;

void PULSE_CONTROLLER_set_idle_function(void (*new_idle_func)(void))
{
   idle_func = new_idle_func;
}

void PULSE_CONTROLLER_write_slave_reg(void*, char, unsigned int, unsigned int)
{}

unsigned PULSE_CONTROLLER_read_slave_reg(void*, char, unsigned int)
{
    return 0;
}

unsigned PULSE_CONTROLLER_is_finished(void*)
{
    return 1;
}

void PULSE_CONTROLLER_wait_for_finished(void*)
{
    return;
}

void yield_execution() {}
void enable_interrupts() {}
void disable_interrupts() {}
void usleep(int) {}
