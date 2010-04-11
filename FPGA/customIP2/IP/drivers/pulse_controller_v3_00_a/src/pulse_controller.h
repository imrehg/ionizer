/*****************************************************************************
* Filename:          C:\trosen\svn\control_trunk\FPGA\customIP\MyProcessorIPLib/drivers/pulse_controller_v3_00_a/src/pulse_controller.h
* Version:           3.00.a
* Description:       pulse_controller Driver Header File
* Date:              Sun Aug 23 16:11:52 2009 (by Create and Import Peripheral Wizard)
*****************************************************************************/

#ifndef PULSE_CONTROLLER_H
#define PULSE_CONTROLLER_H


//byte 0, bits 0-3: timing controller instruction.  0 = timed pulse
//													1 = DDS instruction
//													2 = PMT instruction
//
//byte 0, bit 4: last pulse flag, prevents underflow when FIFO is empty
//byte 0, bit 5: enable PMT counter flag, enables PMT counter during pulse
//
//

//timing controller instructions
//bytes 1,2,3: duration (10 ns units)

//DDS controller instructions
//bytes 2,3: DDS opcode. 0 = set freq, 1 = set phase, 2 = set memory byte, 3 = get memory byte, 4 = reset
//bytes 4,5,6,7: DDS operand

//DDS set freq
//bytes 4,5,6,7: frequency tuning word

//DDS set phase
//bytes 4,5: phase tuning word (14 MSB)
//bytes 6,7: unused

//DDS set byte
//byte 4: memory byte to store
//byte 5: memory address (6 MSB)
//bytes 6,7: unused

//DDS reset
//byte 4,5,6,7: unused

extern unsigned PULSE_CONTROLLER_vacancy;

#define PULSE_CONTROLLER_COUNTING_PULSE_FLAG    (0x04000000)

//! unsafe because FIFO PULSE_CONTROLLER_vacancy is not checked before write. 
//Only call if you *know* there's space on the write FIFO.
inline void PULSE_CONTROLLER_unsafe_pulse(void* base_addr, const unsigned control, const unsigned operand);

//! Set function to be called while waiting for FPGA
void PULSE_CONTROLLER_set_idle_function(void (*new_idle_func)(void));

void PULSE_CONTROLLER_init(void* base_addr, unsigned nDDS);
void PULSE_CONTROLLER_self_test(void* base_addr, int nIO);
int PULSE_CONTROLLER_test_slave_registers(void* base_addr);
int PULSE_CONTROLLER_test_dds(void* base_addr, char nDDS);

//re-initialize all DDS, keeping current frequencies
void PULSE_CONTROLLER_reinit_DDS(void* base_addr, unsigned nDDS);

int PULSE_CONTROLLER_check_dds(void* base_addr, char i);
int PULSE_CONTROLLER_check_all_dds(void* base_addr);

void PULSE_CONTROLLER_write_slave_reg(void* base_addr, char n, unsigned offset, unsigned val);
unsigned PULSE_CONTROLLER_read_slave_reg(void* base_addr, char n, unsigned offset);

void PULSE_CONTROLLER_ensure_vacancy(void* base_addr, unsigned n);
int PULSE_CONTROLLER_read_empty(void* base_addr);

unsigned PULSE_CONTROLLER_read_sr(void* base_addr, unsigned i);
unsigned PULSE_CONTROLLER_pulse(void* base_addr, unsigned t, const unsigned flags, const unsigned operand);
void PULSE_CONTROLLER_short_pulse(void* base_addr, const unsigned control, const unsigned operand);
unsigned PULSE_CONTROLLER_pop_result(void* base_addr);

unsigned PULSE_CONTROLLER_get_write_status(void* base_addr);
unsigned PULSE_CONTROLLER_get_read_status(void* base_addr);

unsigned PULSE_CONTROLLER_get_PMT(void* base_addr);

//TTL functions
void PULSE_CONTROLLER_set_ttl(void* base_addr, unsigned high_mask, unsigned low_mask);
void PULSE_CONTROLLER_get_ttl(void* base_addr, unsigned* high_mask, unsigned* low_mask);

//DDS functions
void PULSE_CONTROLLER_dds_reset(void* base_addr, char i);
void PULSE_CONTROLLER_set_dds_div2(void* base_addr, char i, int b);

void PULSE_CONTROLLER_set_dds_freq(void* base_addr, char i, unsigned freq);
void PULSE_CONTROLLER_set_dds_phase(void* base_addr, char i, unsigned short phase);
unsigned int PULSE_CONTROLLER_get_dds_freq(void* base_addr, char i);
unsigned int PULSE_CONTROLLER_get_dds_phase(void* base_addr, char i);

unsigned PULSE_CONTROLLER_get_dds_byte(void* base_addr, char i, unsigned address);

/*
Timing-check functions to help figure out if experiment timing is being met.
Timing failure will occur, if the pulse buffer underflows.

While timing_check is enabled, all pulses that are sent to the PULSE_CONTROLLER
are considered time-critical.  If a pulse finishes, and there is not another pulse
waiting in the buffer, a timing error is stored in one of the status registers.
This can be detected by calling PULSE_CONTROLLER_timing_ok (returns false if
a timing error occured).  The error status can be cleared by calling PULSE_CONTROLLER_clear_timing_check.
*/

void PULSE_CONTROLLER_enable_timing_check(void* base_addr);
void PULSE_CONTROLLER_disable_timing_check(void* base_addr);

void PULSE_CONTROLLER_clear_timing_check(void* base_addr);
int  PULSE_CONTROLLER_timing_ok(void* base_addr);

#endif /** PULSE_CONTROLLER_H */
