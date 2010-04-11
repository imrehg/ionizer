/*****************************************************************************
* Filename:          C:\trosen\svn\control_trunk\FPGA\customIP\MyProcessorIPLib/drivers/pulse_controller_v3_00_a/src/pulse_controller.c
* Version:           3.00.a
* Description:       pulse_controller Driver Source File
* Date:              Sun Aug 23 16:11:52 2009 (by Create and Import Peripheral Wizard)
*****************************************************************************/


/***************************** Include Files *******************************/

#include "pulse_controller.h"
#include <stdio.h>

#ifndef ALUMINIZER_SIM
   #include "pulse_controller_io.h"
#endif

unsigned nDDS_boards = 0;
unsigned PULSE_CONTROLLER_vacancy = 0;

unsigned last_PMT_value = 0;

unsigned extra_flags = 0;

void (*idle_func)(void);

#define PULSE_CONTROLLER_MAX_NDDS (8)

unsigned int ddsFTW[PULSE_CONTROLLER_MAX_NDDS];
unsigned short ddsPhase[PULSE_CONTROLLER_MAX_NDDS];

#define ENABLE_TIMING_CHECK    (0x08000000)

/************************** Function Definitions ***************************/

/**
 *
 * Write data to PULSE_CONTROLLER write packet FIFO module.
 *
 * @param   baseaddr_p is the base address of the PULSE_CONTROLLER device.
 * @param   data is a point to a given Xuint64 structure for fetching or storing value.
 *
 * @return  None.
 *
 * @note    data should be allocated by the caller.
 *
 */

void PULSE_CONTROLLER_WriteToFIFO(void * baseaddr_p, Xuint64 * data)
{
  Xuint32 baseaddr;
  baseaddr = (Xuint32) baseaddr_p;

  XIo_Out32(((Xuint32)baseaddr)+PULSE_CONTROLLER_WRFIFO_DATA_OFFSET, data->Upper);
  XIo_Out32(((Xuint32)baseaddr)+PULSE_CONTROLLER_WRFIFO_DATA_OFFSET+0x4, data->Lower);
}


void PULSE_CONTROLLER_unsafe_pulse(void* base_addr, const unsigned control, const unsigned operand)
{
   PULSE_CONTROLLER_vacancy--;
   __asm__ volatile ("stw %0,0(%2); stw %1,4(%2); eieio" : : "r" (control),  "r" (operand), "b" (base_addr + PULSE_CONTROLLER_WRFIFO_DATA_OFFSET));
}

void PULSE_CONTROLLER_debug_regs(void* base_addr)
{
	unsigned i;

	for(i=0; i<4; i++)
	{
		xil_printf("Slave register %d = %08X (hi) %08X (lo)\r\n", i, 
			PULSE_CONTROLLER_read_slave_reg(base_addr, i, 0),
			PULSE_CONTROLLER_read_slave_reg(base_addr, i, 4));
	}

	xil_printf("Write FIFO status register = %08X vacancy = %d\r\n",
		(unsigned)XIo_In32(((Xuint32)base_addr)+(PULSE_CONTROLLER_WRFIFO_SR_OFFSET)),
		(int)(PULSE_CONTROLLER_mWriteFIFOVacancy((Xuint32)base_addr)));

	xil_printf(" Read FIFO status register = %08X occupancy = %d\r\n", 
		(unsigned)XIo_In32(((Xuint32)base_addr)+(PULSE_CONTROLLER_RDFIFO_SR_OFFSET)),
		(int)(PULSE_CONTROLLER_mReadFIFOOccupancy((Xuint32)base_addr)));
}

void PULSE_CONTROLLER_init(void* base_addr, unsigned nDDS)
{
   char iDDS;
   unsigned r = 0;

   nDDS_boards = nDDS;

   xil_printf("PULSE_CONTROLLER_init... reset write FIFO\r\n");
   PULSE_CONTROLLER_mResetWriteFIFO((Xuint32)base_addr);

   xil_printf("PULSE_CONTROLLER_init... reset read FIFO\r\n");
   PULSE_CONTROLLER_mResetReadFIFO((Xuint32)base_addr);

   xil_printf("PULSE_CONTROLLER_init... disable timing check\r\n");
   PULSE_CONTROLLER_disable_timing_check(base_addr);

   xil_printf("PULSE_CONTROLLER_init... reset DDS\r\n");
   for(iDDS=0; iDDS<nDDS_boards; iDDS++)
   {
      PULSE_CONTROLLER_dds_reset(base_addr, iDDS);
      PULSE_CONTROLLER_set_dds_div2(base_addr, iDDS, 0);
   }

   PULSE_CONTROLLER_debug_regs(base_addr);

   xil_printf("PULSE_CONTROLLER_init... get PMT\r\n");
   r = PULSE_CONTROLLER_get_PMT(base_addr);
   xil_printf("Result = %08X\r\n", r);
}

void PULSE_CONTROLLER_reinit_DDS(void* base_addr, unsigned nDDS)
{
   char iDDS;
   unsigned ftw;

   for(iDDS=0; iDDS<nDDS; iDDS++)
   {
      ftw = PULSE_CONTROLLER_get_dds_freq(base_addr, iDDS);
      PULSE_CONTROLLER_dds_reset(base_addr, iDDS);
      PULSE_CONTROLLER_set_dds_div2(base_addr, iDDS, 0);
	  PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, ftw);
   }

   PULSE_CONTROLLER_get_PMT(base_addr);
}

//! Make sure there is space for at least n pulses on the FIFO
void PULSE_CONTROLLER_ensure_vacancy(void* base_addr, unsigned n)
{
   while(PULSE_CONTROLLER_vacancy < n)
      PULSE_CONTROLLER_vacancy = PULSE_CONTROLLER_mWriteFIFOVacancy((Xuint32)base_addr);
}

//! Is the read FIFO empty?
int PULSE_CONTROLLER_read_empty(void* base_addr)
{
   return PULSE_CONTROLLER_mReadFIFOEmpty((Xuint32)base_addr);
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSE_CONTROLLER_set_ttl(void* base_addr, unsigned high_mask, unsigned low_mask)
{
   PULSE_CONTROLLER_write_slave_reg(base_addr, 0, 0, high_mask);
   PULSE_CONTROLLER_write_slave_reg(base_addr, 0, 4, low_mask);
}

//! TTL functions: pulse_io = (ttl_out | high_mask) & (~low_mask);
void PULSE_CONTROLLER_get_ttl(void* base_addr, unsigned* high_mask, unsigned* low_mask)
{
   *high_mask = PULSE_CONTROLLER_read_slave_reg(base_addr, 0, 0);
   *low_mask = PULSE_CONTROLLER_read_slave_reg(base_addr, 0, 4);
}

//! DDS functions - reset DDS i
void PULSE_CONTROLLER_dds_reset(void* base_addr, char i)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10004000 | (i << 8), 0);

   ddsFTW[i] = 0;
   ddsPhase[i] = 0;
}

//! DDS functions - set whether GHz DDS clock is divided by 2
void PULSE_CONTROLLER_set_dds_div2(void* base_addr, char i, int b)
{
   if(b)
      PULSE_CONTROLLER_short_pulse(base_addr, 0x10002000 | (i << 8), 0x18000000);
   else
      PULSE_CONTROLLER_short_pulse(base_addr, 0x10002000 | (i << 8), 0x58000000);
}

//! DDS functions - get byte from address on DDS i
unsigned PULSE_CONTROLLER_get_dds_byte(void* base_addr, char i, unsigned address)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10003000 | (i << 8), address << 18);
   return (PULSE_CONTROLLER_pop_result(base_addr) & 0xff);
}

//! enable timing check for pulses
void PULSE_CONTROLLER_enable_timing_check(void* base_addr)
{
   extra_flags = extra_flags | ENABLE_TIMING_CHECK;
}

//! disable timing check for pulses
void PULSE_CONTROLLER_disable_timing_check(void* base_addr)
{
   extra_flags = extra_flags & ~ENABLE_TIMING_CHECK;
}

//! clear timing check (clear failures)
void PULSE_CONTROLLER_clear_timing_check(void* base_addr)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x30000000, 0);
}

//! were there any timing failures?
int  PULSE_CONTROLLER_timing_ok(void* base_addr)
{
   return !(PULSE_CONTROLLER_read_slave_reg(base_addr, 2, 0) & 0x80000000);
}

//! get status register from write FIFO
unsigned PULSE_CONTROLLER_get_write_status(void* base_addr)
{
   return PULSE_CONTROLLER_mReadReg((Xuint32)base_addr, PULSE_CONTROLLER_WRFIFO_SR_OFFSET);
}

//! get status register from read FIFO
unsigned PULSE_CONTROLLER_get_read_status(void* base_addr)
{
   return PULSE_CONTROLLER_mReadReg((Xuint32)base_addr, PULSE_CONTROLLER_RDFIFO_SR_OFFSET);
}

void PULSE_CONTROLLER_write_slave_reg(void* base_addr, char n, unsigned offset, unsigned val)
{
   switch(n)
   {
      case 0 : PULSE_CONTROLLER_mWriteSlaveReg0((Xuint32)base_addr, offset, val); break;
      case 1 : PULSE_CONTROLLER_mWriteSlaveReg1((Xuint32)base_addr, offset, val); break;
      case 2 : PULSE_CONTROLLER_mWriteSlaveReg2((Xuint32)base_addr, offset, val); break;
      case 3 : PULSE_CONTROLLER_mWriteSlaveReg3((Xuint32)base_addr, offset, val); break;
	  case 4 : PULSE_CONTROLLER_mWriteSlaveReg4((Xuint32)base_addr, offset, val); break;
      case 5 : PULSE_CONTROLLER_mWriteSlaveReg5((Xuint32)base_addr, offset, val); break;
      case 6 : PULSE_CONTROLLER_mWriteSlaveReg6((Xuint32)base_addr, offset, val); break;
      case 7 : PULSE_CONTROLLER_mWriteSlaveReg7((Xuint32)base_addr, offset, val); break;
   }
}

unsigned PULSE_CONTROLLER_read_slave_reg(void* base_addr, char n, unsigned offset)
{
   switch(n)
   {
      case 0 : return PULSE_CONTROLLER_mReadSlaveReg0((Xuint32)base_addr, offset);
      case 1 : return PULSE_CONTROLLER_mReadSlaveReg1((Xuint32)base_addr, offset);
      case 2 : return PULSE_CONTROLLER_mReadSlaveReg2((Xuint32)base_addr, offset);
      case 3 : return PULSE_CONTROLLER_mReadSlaveReg3((Xuint32)base_addr, offset);
	  case 4 : return PULSE_CONTROLLER_mReadSlaveReg4((Xuint32)base_addr, offset);
      case 5 : return PULSE_CONTROLLER_mReadSlaveReg5((Xuint32)base_addr, offset);
      case 6 : return PULSE_CONTROLLER_mReadSlaveReg6((Xuint32)base_addr, offset);
      case 7 : return PULSE_CONTROLLER_mReadSlaveReg7((Xuint32)base_addr, offset);
   }

   return 0;
}

void PULSE_CONTROLLER_self_test(void* base_addr, int nIO)
{
   unsigned ftw [PULSE_CONTROLLER_MAX_NDDS];
   unsigned cycle = 0;
   unsigned nBad = 0;
   char iDDS;

   PULSE_CONTROLLER_test_slave_registers(base_addr);
   xil_printf("\r\n");

   if(nIO > 0)
   {
      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
         PULSE_CONTROLLER_test_dds(base_addr, iDDS);

      xil_printf("Testing %d random read/writes on DDS boards 0-%d ... ", nIO, nDDS_boards-1);

      //initialize to 0 Hz
      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
      {
         ftw[iDDS] = 0;
         PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, ftw[iDDS]);
      }

      while(cycle < nIO)
      {
         iDDS = rand() % nDDS_boards;
         unsigned ftw_read = PULSE_CONTROLLER_get_dds_freq(base_addr, iDDS);

         if(ftw_read != ftw[iDDS])
         {
            xil_printf("\r\n");
            xil_printf("ERROR on DDS %d : wrote FTW %08X\r\n", (int)iDDS, ftw[iDDS]);
            xil_printf("                   read FTW %08X\r\n", ftw_read);
            nBad++;
         }

         ftw[iDDS] = rand();
         PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, ftw[iDDS]);

         cycle++;
      }

      for(iDDS=0; iDDS<nDDS_boards; iDDS++)
      {
         PULSE_CONTROLLER_set_dds_freq(base_addr, iDDS, 0);
      }

      if(nBad == 0)
         xil_printf("OK\r\n");
      else
         xil_printf("FAILURE: %d errors\r\n", nBad);
   }
}

int PULSE_CONTROLLER_test_slave_registers(void* base_addr)
{
   int sr_ok = 1;
   unsigned test_val, read;
   int i, j, k;

   for(i=0; i<2; i++)
   {
      test_val = 0;

      for(j=0; j<8; j++)
         test_val = test_val + ((i*0xF) << (j*4));

      xil_printf("Testing %08X   ", test_val);
      for(k=0; k<8; k++)
      {
         PULSE_CONTROLLER_write_slave_reg(base_addr, k, 0, test_val);

         read = PULSE_CONTROLLER_read_slave_reg(base_addr, k, 0);

         xil_printf("SR%d = %08X   ", k, read);

         sr_ok = sr_ok && (read == test_val);
      }

      if(sr_ok)
         xil_printf("OK\r\n");
      else
         xil_printf("FAILED\r\n");
   }

   return sr_ok;
}

int PULSE_CONTROLLER_test_dds(void* base_addr, char nDDS)
{
   int ftw_ok = 1;
   int phase_ok = 1;
   unsigned phase = 0;
   int i, j;
   unsigned read, test_val;

   xil_printf("Testing DDS %d ... ", (int)nDDS);
   for(i=0; i<2; i++)
   {
      test_val = 0;

      for(j=0; j<8; j++)
         test_val = test_val + ((i*0xF) << (j*4));

      PULSE_CONTROLLER_set_dds_freq(base_addr, nDDS, test_val);
      read = PULSE_CONTROLLER_get_dds_freq(base_addr, nDDS);
      ftw_ok = ftw_ok && (read == test_val);

      if(read != test_val)
      {
         xil_printf("ERROR !\r\n", nDDS);
         xil_printf("wrote FTW %08X\r\n", test_val);
         xil_printf(" read FTW %08X\r\n", read);
      }
   }

   while(phase < 0x4000)
   {
      PULSE_CONTROLLER_set_dds_phase(base_addr, nDDS, phase);
      read = PULSE_CONTROLLER_get_dds_phase(base_addr, nDDS);

      phase_ok = phase_ok && (read == phase);

      if(read != phase)
      {
         xil_printf("ERROR on DDS %d !\r\n", nDDS);
         xil_printf("wrote PHASE %04X\r\n", test_val);
         xil_printf(" read PHASE %04X\r\n", read);
      }

      phase++;
   }

   if(ftw_ok && phase_ok)
      xil_printf("OK\r\n", (int)nDDS);
   else
      xil_printf("DDS %d FAILED\r\n", (int)nDDS);

   return ftw_ok && phase_ok;
}

unsigned PULSE_CONTROLLER_get_PMT(void* base_addr)
{
   unsigned new_PMT_value, d;

   PULSE_CONTROLLER_short_pulse(base_addr, (0x20000000), 0);
	new_PMT_value = PULSE_CONTROLLER_pop_result(base_addr);
    d = new_PMT_value - last_PMT_value;

   last_PMT_value = new_PMT_value;
   return d;
}

unsigned PULSE_CONTROLLER_read_sr(void* base_addr, unsigned i)
{
   return PULSE_CONTROLLER_mReadReg((Xuint32)base_addr, PULSE_CONTROLLER_RDFIFO_SR_OFFSET + i);
}

//make timed pulses
//if t > t_max, subdivide into shorter pulses
//returns number of pulses made
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

//make short timed pulses
//FPGA can only handle pulse lengths up to t_max = 0x001FFFFF (about 40 ms)
void PULSE_CONTROLLER_short_pulse(void* base_addr, const unsigned control, const unsigned operand)
{
   if(!PULSE_CONTROLLER_vacancy)
      PULSE_CONTROLLER_ensure_vacancy(base_addr, 1);

   PULSE_CONTROLLER_unsafe_pulse(base_addr, control | extra_flags, operand);
}


void PULSE_CONTROLLER_set_idle_function(void (*new_idle_func)(void))
{
	idle_func = new_idle_func;
}

unsigned PULSE_CONTROLLER_pop_result(void* base_addr)
{
	unsigned r;

//	while(PULSE_CONTROLLER_read_empty(base_addr)) {}

   //crashed here
//   r =  PULSE_CONTROLLER_mReadFromFIFO((Xuint32)base_addr, 0);

   while(PULSE_CONTROLLER_read_empty(base_addr)) 
   {
	   if(idle_func)
		   idle_func();
   }

   r = PULSE_CONTROLLER_mReadFromFIFO((Xuint32)base_addr, 4);

 //  xil_printf("result = %08X ... done\r\n", r);

   return r;
}

void PULSE_CONTROLLER_set_dds_freq(void* base_addr, char i, unsigned freq)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10000000 | (i << 8), freq);

   ddsFTW[i] = freq;
}

void PULSE_CONTROLLER_set_dds_phase(void* base_addr, char i, unsigned short phase)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10001000 | (i << 8), phase);

   ddsPhase[i] = phase;
}

int PULSE_CONTROLLER_check_all_dds(void* base_addr)
{
   char i;

   for(i=0; i<nDDS_boards; i++)
   {
      if(!PULSE_CONTROLLER_check_dds(base_addr, i))
      {
         xil_printf("ERROR on DDS %d !\r\n", (int)i);
         return 0;
      }
   }

   return 1;
}

int PULSE_CONTROLLER_check_dds(void* base_addr, char i)
{
   return (ddsFTW[i] == PULSE_CONTROLLER_get_dds_freq(base_addr, i)) && (ddsPhase[i] == PULSE_CONTROLLER_get_dds_phase(base_addr, i));
}

unsigned PULSE_CONTROLLER_get_dds_freq(void* base_addr, char i)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10005000 | (i << 8), 0);
   return PULSE_CONTROLLER_pop_result(base_addr);
}

unsigned PULSE_CONTROLLER_get_dds_phase(void* base_addr, char i)
{
   PULSE_CONTROLLER_short_pulse(base_addr, 0x10006000 | (i << 8), 0);
   return PULSE_CONTROLLER_pop_result(base_addr) & 0x3fff;
}


