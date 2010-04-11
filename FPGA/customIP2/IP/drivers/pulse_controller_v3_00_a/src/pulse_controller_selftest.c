/*****************************************************************************
* Filename:          C:\trosen\svn\control_trunk\FPGA\customIP\MyProcessorIPLib/drivers/pulse_controller_v3_00_a/src/pulse_controller_selftest.c
* Version:           3.00.a
* Description:       Contains a diagnostic self-test function for the pulse_controller driver
* Date:              Sun Aug 23 16:11:52 2009 (by Create and Import Peripheral Wizard)
*****************************************************************************/


/***************************** Include Files *******************************/

#include "pulse_controller.h"
#include "pulse_controller_io.h"
/************************** Constant Definitions ***************************/


/************************** Variable Definitions ****************************/


/************************** Function Definitions ***************************/

/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the PULSE_CONTROLLER instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus PULSE_CONTROLLER_SelfTest(void * baseaddr_p)
{
  int     Index;
  Xuint32 baseaddr;
  Xuint8  Reg8Value;
  Xuint16 Reg16Value;
  Xuint32 Reg32Value;
  
  /*
   * Check and get the device address
   */
  XASSERT_NONVOID(baseaddr_p != XNULL);
  baseaddr = (Xuint32) baseaddr_p;

  xil_printf("******************************\n\r");
  xil_printf("* User Peripheral Self Test\n\r");
  xil_printf("******************************\n\n\r");

  /*
   * Reset the device to get it back to the default state
   */
  xil_printf("Soft reset test...\n\r");
  PULSE_CONTROLLER_mReset(baseaddr);
  xil_printf("   - write 0x%08x to software reset register\n\r", SOFT_RESET);
  xil_printf("   - soft reset passed\n\n\r");

  /*
   * Write to user logic slave module register(s) and read back
   */
  xil_printf("User logic slave module test...\n\r");
  xil_printf("   - write 1 to slave register 0 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg0(baseaddr, 0, 1);
  xil_printf("   - write 2 to slave register 0 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg0(baseaddr, 4, 2);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg0(baseaddr, 0);
  xil_printf("   - read %d from register 0 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 1 )
  {
    xil_printf("   - slave register 0 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg0(baseaddr, 4);
  xil_printf("   - read %d from register 0 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 2 )
  {
    xil_printf("   - slave register 0 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - write 3 to slave register 1 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg1(baseaddr, 0, 3);
  xil_printf("   - write 4 to slave register 1 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg1(baseaddr, 4, 4);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg1(baseaddr, 0);
  xil_printf("   - read %d from register 1 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 3 )
  {
    xil_printf("   - slave register 1 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg1(baseaddr, 4);
  xil_printf("   - read %d from register 1 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 4 )
  {
    xil_printf("   - slave register 1 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - write 5 to slave register 2 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg2(baseaddr, 0, 5);
  xil_printf("   - write 6 to slave register 2 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg2(baseaddr, 4, 6);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg2(baseaddr, 0);
  xil_printf("   - read %d from register 2 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 5 )
  {
    xil_printf("   - slave register 2 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg2(baseaddr, 4);
  xil_printf("   - read %d from register 2 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 6 )
  {
    xil_printf("   - slave register 2 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - write 7 to slave register 3 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg3(baseaddr, 0, 7);
  xil_printf("   - write 8 to slave register 3 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg3(baseaddr, 4, 8);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg3(baseaddr, 0);
  xil_printf("   - read %d from register 3 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 7 )
  {
    xil_printf("   - slave register 3 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg3(baseaddr, 4);
  xil_printf("   - read %d from register 3 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 8 )
  {
    xil_printf("   - slave register 3 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - write 9 to slave register 4 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg4(baseaddr, 0, 9);
  xil_printf("   - write 10 to slave register 4 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg4(baseaddr, 4, 10);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg4(baseaddr, 0);
  xil_printf("   - read %d from register 4 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 9 )
  {
    xil_printf("   - slave register 4 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg4(baseaddr, 4);
  xil_printf("   - read %d from register 4 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 10 )
  {
    xil_printf("   - slave register 4 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - write 11 to slave register 5 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg5(baseaddr, 0, 11);
  xil_printf("   - write 12 to slave register 5 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg5(baseaddr, 4, 12);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg5(baseaddr, 0);
  xil_printf("   - read %d from register 5 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 11 )
  {
    xil_printf("   - slave register 5 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg5(baseaddr, 4);
  xil_printf("   - read %d from register 5 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 12 )
  {
    xil_printf("   - slave register 5 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - write 13 to slave register 6 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg6(baseaddr, 0, 13);
  xil_printf("   - write 14 to slave register 6 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg6(baseaddr, 4, 14);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg6(baseaddr, 0);
  xil_printf("   - read %d from register 6 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 13 )
  {
    xil_printf("   - slave register 6 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg6(baseaddr, 4);
  xil_printf("   - read %d from register 6 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 14 )
  {
    xil_printf("   - slave register 6 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - write 15 to slave register 7 word 0\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg7(baseaddr, 0, 15);
  xil_printf("   - write 16 to slave register 7 word 1\n\r");
  PULSE_CONTROLLER_mWriteSlaveReg7(baseaddr, 4, 16);
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg7(baseaddr, 0);
  xil_printf("   - read %d from register 7 word 0\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 15 )
  {
    xil_printf("   - slave register 7 word 0 write/read failed\n\r");
    return XST_FAILURE;
  }
  Reg32Value = PULSE_CONTROLLER_mReadSlaveReg7(baseaddr, 4);
  xil_printf("   - read %d from register 7 word 1\n\r", Reg32Value);
  if ( Reg32Value != (Xuint32) 16 )
  {
    xil_printf("   - slave register 7 word 1 write/read failed\n\r");
    return XST_FAILURE;
  }
  xil_printf("   - slave register write/read passed\n\n\r");

  /*
   * Write to the Write Packet FIFO and read back from the Read Packet FIFO
   */
  xil_printf("Packet FIFO test...\n\r");
  xil_printf("   - reset write packet FIFO to initial state\n\r");
  PULSE_CONTROLLER_mResetWriteFIFO(baseaddr);
  xil_printf("   - reset read packet FIFO to initial state \n\r");
  PULSE_CONTROLLER_mResetReadFIFO(baseaddr);
  xil_printf("   - push data to write packet FIFO\n\r");
  for ( Index = 0; Index < 4; Index++ )
  {
    xil_printf("     0x%08x", Index*2+1);
    PULSE_CONTROLLER_mWriteToFIFO(baseaddr, 0, Index*2+1);
    xil_printf("     0x%08x", Index*2+2);
    PULSE_CONTROLLER_mWriteToFIFO(baseaddr, 4, Index*2+2);
    xil_printf("\n\r");
  }
  xil_printf("   - write packet FIFO is %s\n\r", ( PULSE_CONTROLLER_mWriteFIFOFull(baseaddr) ? "full" : "not full" ));
  /* 
   * FIFO example in user logic will loop WrFIFO data back to RdFIFO,
   * so we get number of entries from RdFIFO instead of WrFIFO.
   * Reg32Value = PULSE_CONTROLLER_mWriteFIFOVacancy(baseaddr);
   */
  Reg32Value = PULSE_CONTROLLER_mReadFIFOOccupancy(baseaddr);
  xil_printf("   - number of entries is %s %d\n\r", ( Reg32Value == (Xuint32) 4 ? "expected" : "unexpected" ), Reg32Value);
  xil_printf("   - pop data out from read packet FIFO\n\r");
  for ( Index = 0; Index < 4; Index++ )
  {
    Reg32Value = PULSE_CONTROLLER_mReadFromFIFO(baseaddr, 0);
    xil_printf("     0x%08x", Reg32Value);
    if ( Reg32Value != (Xuint32) Index*2+1 )
    {
      xil_printf("\n\r");
      xil_printf("   - unexpected value read from read packet FIFO\n\r");
      xil_printf("   - write/read packet FIFO failed\n\r");
      PULSE_CONTROLLER_mResetWriteFIFO(baseaddr);
      PULSE_CONTROLLER_mResetReadFIFO(baseaddr);
      return XST_FAILURE;
    }
    Reg32Value = PULSE_CONTROLLER_mReadFromFIFO(baseaddr, 4);
    xil_printf("     0x%08x", Reg32Value);
    if ( Reg32Value != (Xuint32) Index*2+2 )
    {
      xil_printf("\n\r");
      xil_printf("   - unexpected value read from read packet FIFO\n\r");
      xil_printf("   - write/read packet FIFO failed\n\r");
      PULSE_CONTROLLER_mResetWriteFIFO(baseaddr);
      PULSE_CONTROLLER_mResetReadFIFO(baseaddr);
      return XST_FAILURE;
    }
    xil_printf("\n\r");
  }
  xil_printf("   - read packet FIFO is %s\n\r", ( PULSE_CONTROLLER_mReadFIFOEmpty(baseaddr) ? "empty" : "not empty" ));
  Reg32Value = PULSE_CONTROLLER_mReadFIFOOccupancy(baseaddr);
  xil_printf("   - number of entries is %s %d \n\r", ( Reg32Value == (Xuint32) 0 ? "expected" : "unexpected" ), Reg32Value);
  PULSE_CONTROLLER_mResetWriteFIFO(baseaddr);
  PULSE_CONTROLLER_mResetReadFIFO(baseaddr);
  xil_printf("   - write/read packet FIFO passed \n\n\r");

  return XST_SUCCESS;
}
