#include "common.h"
#include "dac.h"

#ifdef ALUMINIZER_SIM
	#define XPAR_OPB_SPI_0_BASEADDR (0)
	#define XST_SUCCESS (0)
typedef unsigned XStatus;
typedef unsigned Xuint32;
#else
	#include "xparameters.h"
	#include "xspi.h"
	#include "xstatus.h"
#endif

#include <stdio.h>
#include <string>

#include "experiments.h"


//set DAC voltage for channels 0-31
//if channel > 31, set offset voltage
void SetDAC_AD5532(XSpi* spi, unsigned channel, unsigned dacWord)
{
	unsigned status = 0;
	unsigned tx = 0;
	unsigned rc = 0;

	if (channel > 31)
		tx = 0x50000000;
	else
	{
		tx = 0x40000000;
		tx |= (channel & 0x1F) << 22;
	}

	tx |= (dacWord & 0x3FFF) << 8;

//	printf("DAC<%u> = %08X (tx = %08X)\n", channel, dacWord, tx);
	status = SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-31
void SetDAC_AD5535(XSpi* spi, unsigned channel, unsigned dacWord)
{
	//repeat first write twice
	static bool bNeedInit = true;

	if (bNeedInit)
	{
		bNeedInit = false;
		SetDAC_AD5535(spi, channel, dacWord);
	}


	unsigned status = 0;
	unsigned tx = 0;
	unsigned rc = 0;

	tx = (channel & 0x1F) << 27;     //address bits
	tx |= (dacWord & 0x3FFF) << 13;  // 14-bit DAC word

//	printf("AD5535 DAC<%u> = %08X (tx = %08X)\n", channel, dacWord, tx);

	status = SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-7
void SetDAC_AD5668(XSpi* spi, unsigned channel, unsigned dacWord)
{
	static bool bNeedInit = true;

	if (bNeedInit)
	{
		bNeedInit = false;

		unsigned tx = 0;
		unsigned rc = 0;

		tx = (0x06) << 24;   // command to load LDAC
		tx |= 0xFF;          // LDAC word

		printf("DAC init (tx = %08X)\n", tx);

		SPI_Transmit(spi, &tx, &rc, 4);
	}

	unsigned status = 0;
	unsigned tx = 0;
	unsigned rc = 0;

/*	tx =0xF;
   tx |=(0x2&channel)<<22;
   tx |= (dacWord&0xF) << 8;
 */

	tx = (0x02) << 24;               // command to write & update one channel
	tx |= (channel & 0x7) << 20;     //address bits
	tx |= (dacWord & 0xFFFF) << 4;   // 16-bit DAC word

//	printf("DAC<%u> = %08X (tx = %08X)\n", channel, dacWord, tx);

	status = SPI_Transmit(spi, &tx, &rc, 4);
}

void cmdAD5370(XSpi* spi, unsigned mode, unsigned channel, unsigned data)
{
/* AD5370 takes 24-bit SPI commands

   I23 I22 I21 I20 I19 I18 I17 I16 I15 I14 I13 I12 I11 I10 I9 I8 I7 I6 I5 I4 I3 I2 I1 I0
   M1   M0  A5  A4  A3  A2  A1  A0 D15 D14 D13 D12 D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0

 */
	unsigned tx = 0;
	unsigned rc = 0;

	//there are 5 groups of 8 channels (group = 0 addresses all groups at once)
	unsigned group = (1 + channel / 8) & 0x7;

	//combine group bits with the 3 low bits of the channel#
	unsigned addr = (group << 3) | (channel & 0x7);

	tx  = (mode & 0x0003) << 30;
	tx |=  addr << 24;
	tx |= (data & 0xFFFF) << 8;

//	printf("AD5370 DAC mode=%u addr=%u data=%u (tx = %08X)\n", mode, addr, data, tx);
	SPI_Transmit(spi, &tx, &rc, 3);
}

void setAD5370offset(XSpi* spi, unsigned reg, unsigned data)
{
/* AD5370 takes 24-bit SPI commands

   I23 I22 I21 I20 I19 I18 I17 I16 I15 I14 I13 I12 I11 I10 I9 I8 I7 I6 I5 I4 I3 I2 I1 I0
   M1   M0  A5  A4  A3  A2  A1  A0 D15 D14 D13 D12 D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0

 */
	unsigned tx = 0;
	unsigned rc = 0;

	tx  = (reg + 2) << 24;
	tx |= (data & 0x3FFF) << 8;

//	printf("AD5370 DAC set offset reg. (tx = %08X)\n", tx);
	SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-40
void SetDAC_AD5370(XSpi* spi, unsigned channel, unsigned dacWord)
{
	static bool bNeedInit = true;

	if (bNeedInit)
	{
		bNeedInit = false;

		setAD5370offset(spi, 0, 0x1FFF);
		setAD5370offset(spi, 1, 0x1FFF);

		for (unsigned i = 0; i < 40; i++)
		{
			cmdAD5370(spi, 2, i, 0x8000); //set offset register
			cmdAD5370(spi, 1, i, 0xFFFF); //set gain register
		}
	}

	cmdAD5370(spi, 3, channel, dacWord);
}

//optimized copy of the Xilinx driver function
int SPI_SetSlaveSelect(XSpi * InstancePtr, u32 SlaveMask)
{
	/*
	 * A single slave is either being selected or the incoming SlaveMask is
	 * zero, which means the slave is being deselected. Setup the value to
	 * be  written to the slave select register as the inverse of the slave
	 * mask.
	 */

	InstancePtr->SlaveSelectReg = ~SlaveMask;
	XSpi_mSetSlaveSelectReg(InstancePtr, InstancePtr->SlaveSelectReg);

	return XST_SUCCESS;
}

//start a conversion on previously spec'd ADC channel
void startConversion_AD7689(XSpi* spi)
{
	unsigned tx = 0; //(0xF1C40000 | (channel << 25));
	unsigned rc = 0;

	SPI_Transfer2(spi, tx);
}

//get ADC conversion result and configure for next conversion
unsigned short getResult_AD7689(XSpi* spi, unsigned next_channel)
{
/*
   u16 tx0 = 0xF1C4 | (next_channel << 9);
   u16 tx1 = 0;
   u16 rc0, rc1;

   //send configuration for next conversion
   //initiate conversion when SS/CNV goes high at end of xfer
   //transfer should start at least 4.2 us (tCONV) after previous xfer ended
   rc0 = SPI_Transfer2(spi, tx0);

   //receive LSB of data.  are these valid?
   //transfer should end within 1.2 us (tDATA) of end of previous xfer
   rc1 = SPI_Transfer2(spi, tx1);

   return rc1;
 */
	/*
   unsigned tx = 0xF1C40000 | (next_channel << 25);
   unsigned rc = 0;

   XStatus s = XSpi_Transfer(spi, (u8*)&tx, (u8*)&rc, 2);
 */

	u16 tx = 0xF1C4 | (next_channel << 9);

	SPI_SetSlaveSelect(spi, 1);
	u16 rc = SPI_Transfer2(spi, tx);
	SPI_SetSlaveSelect(spi, 0);

	return rc;

}


XStatus SPI_Transmit(XSpi* spi, unsigned* dataTX, unsigned* dataRC, unsigned nBytes)
{
#ifdef ALUMINIZER_SIM
	return 0;
#else

	Xuint8* tx = (Xuint8*)dataTX;
	Xuint8* rc = (Xuint8*)dataRC;

	if (g_debug_spi)
		printf("spi <- tx =%08x\r\n", *dataTX);

	SPI_SetSlaveSelect(spi, 1);
	XStatus s = XSpi_Transfer(spi, tx, rc, nBytes);
	SPI_SetSlaveSelect(spi, 0);
/*
   switch(s){
      case XST_DEVICE_IS_STOPPED : printf("SPI failure.  XST_DEVICE_IS_STOPPED\n"); break;
      case XST_DEVICE_BUSY : printf("SPI failure.  XST_DEVICE_BUSY\n"); break;
      case XST_SPI_NO_SLAVE : printf("SPI failure.  XST_SPI_NO_SLAVE\n"); break;
   }
 */
	if (g_debug_spi)
		printf("spi -> rc =%08x\r\n", *dataRC);

	return s;
#endif
}



void SPI_init(XSpi* spi, unsigned id, bool bActiveLow)
{
	XStatus s = XSpi_Initialize(spi, id);

	//not sure why the driver sometimes returns XST_DEVICE_IS_STARTED on startup
	if (XST_DEVICE_IS_STARTED == s)
	{
		printf("spi<%d> already started. Stopping...\r\n", id);

		//Xilinx docs say to stop device and re-initialize
		XSpi_Stop(spi);
		s = XSpi_Initialize(spi, id);
	}

	if (XST_SUCCESS == s)
		printf("spi<%d> initialized successfully\r\n", id);
	else
		printf("spi<%d> failed to initialize\r\n", id);

	/*
	 * Set the Spi device as a master, and toggle SSELECT manually.
	 */
	unsigned options = XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION;

//	unsigned options = XSP_MASTER_OPTION;

	if (bActiveLow) //Data is clocked in on falling clock edges.
		options |= XSP_CLK_ACTIVE_LOW_OPTION;

	XSpi_SetOptions(spi, options);
	XSpi_Start(spi);

	/*
	 * Disable Global interrupt to use polled mode operation
	 */
	XSpi_mIntrGlobalDisable(spi);

	//turn off inhibit
	u16 ControlReg;
	ControlReg = XSpi_mGetControlReg(spi);
	ControlReg &= ~XSP_CR_TRANS_INHIBIT_MASK;
	XSpi_mSetControlReg(spi,  ControlReg);

///	SPI_SetSlaveSelect(spi, 1);
}

/******************************************************************************/
u16 SPI_Transfer2(XSpi * InstancePtr, u16 tx)
{
	u16 rcv;
	u8 StatusReg;

	/*
	 * Set the busy flag, which will be cleared when the transfer
	 * is completely done.
	 */
	InstancePtr->IsBusy = TRUE;


	//TX should be empty from previous transmission
	//transfer data to SPI transmitter
	XIo_Out16(InstancePtr->BaseAddr + XSP_DTR_OFFSET - 1, tx);


	/*
	 * Wait for the transfer to be done by polling the transmit
	 * empty status bit
	 */
	do
		StatusReg = XSpi_mGetStatusReg(InstancePtr);
	while ((StatusReg & XSP_SR_TX_EMPTY_MASK) == 0);


	do
	{
		rcv = XIo_In16(InstancePtr->BaseAddr + XSP_DRR_OFFSET - 1);
		StatusReg = XSpi_mGetStatusReg(InstancePtr);
	}
	while ((StatusReg & XSP_SR_RX_EMPTY_MASK) == 0);

	InstancePtr->IsBusy = FALSE;


	return rcv;
}
