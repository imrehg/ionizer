#include "common.h"

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <string>
#include <math.h>

#include "sim_ions.h"
#include "dac.h"

//set DAC voltage for channels 0-31
//if channel > 31, set offset voltage
void SetDAC_AD5532(XSpi* spi, unsigned channel, unsigned dacWord)
{
   unsigned status = 0;
   unsigned tx = 0;
   unsigned rc = 0;

   if(channel > 31)
   {
      tx = 0x50000000;
   }
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

   if(bNeedInit)
   {
      bNeedInit = false;
      SetDAC_AD5535(spi, channel, dacWord);
   }


   unsigned status = 0;
   unsigned tx = 0;
   unsigned rc = 0;

   tx = (channel & 0x1F) << 27; //address bits
   tx |= (dacWord & 0x3FFF) << 13; // 14-bit DAC word

//	printf("AD5535 DAC<%u> = %08X (tx = %08X)\n", channel, dacWord, tx);

   status = SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-7
void SetDAC_AD5668(XSpi* spi, unsigned channel, unsigned dacWord)
{
   static bool bNeedInit = true;

   if(bNeedInit)
   {
      bNeedInit = false;

      unsigned tx = 0;
      unsigned rc = 0;

      tx = (0x06) << 24; // command to load LDAC
      tx |= 0xFF; // LDAC word

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

   tx = (0x02) << 24; // command to write & update one channel
   tx |= (channel & 0x7) << 20; //address bits
   tx |= (dacWord & 0xFFFF) << 4; // 16-bit DAC word

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

   tx  = (reg+2) << 24;
   tx |= (data & 0x3FFF) << 8;

//	printf("AD5370 DAC set offset reg. (tx = %08X)\n", tx);
   SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-40
void SetDAC_AD5370(XSpi* spi, unsigned channel, unsigned dacWord)
{
   static bool bNeedInit = true;

   if(bNeedInit)
   {
      bNeedInit = false;

      setAD5370offset(spi, 0, 0x1FFF);
      setAD5370offset(spi, 1, 0x1FFF);

      for(unsigned i=0; i<40; i++)
      {
         cmdAD5370(spi, 2, i, 0x8000); //set offset register
         cmdAD5370(spi, 1, i, 0xFFFF); //set gain register
      }
   }

   cmdAD5370(spi, 3, channel, dacWord);
}

//optimized copy of the Xilinx driver function
int SPI_SetSlaveSelect(XSpi*, u32)
{
   return XST_SUCCESS;
}


//get ADC conversion result and configure for next conversion
unsigned short getResult_AD7689(XSpi*, unsigned next_channel)
{
	static FILE* fRecorded = 0;

	if(fRecorded == 0)
		fRecorded = fopen("C:\\data\\ClockCavityComp\\20090911_152501\\AI_0.dat", "rb");

	static int t = 0;
	static int channel0 = 0;
	static int channel1 = 0;

	unsigned short r = 0;

	if(fRecorded)
	{
		//play back recorded data
		static short ADC_data[8];
		short new_data[8];

//		static unsigned nRead = 0;

		if(next_channel == 0)
		{
			for(unsigned j=0; j<100; j++)
				fread(new_data, 2, 8, fRecorded);

			ADC_data[6] = new_data[0];
			ADC_data[7] = new_data[1];

			ADC_data[0] = new_data[2];
			ADC_data[1] = new_data[3];
			ADC_data[2] = new_data[4];
			ADC_data[3] = new_data[5];
			ADC_data[4] = new_data[6];
			ADC_data[5] = new_data[7];
		}

		r = ADC_data[channel0] >> 1;
	}
	else
	{
		double mod_freq = 0.05;
		double mod_depth = 0.5;

		double phi_mod  = mod_depth * cos(2*M_PI * mod_freq * t);
		int scale = 1 << 14;

		switch(channel0)
		{
		case 0: r = (phi_mod) * scale; break;
		case 6: r = (0.5*cos(phi_mod)) * scale; break;
		case 7: r = (0.5*sin(phi_mod)) * scale; t++; break;
		}
	}

	channel0 = channel1;
	channel1 = next_channel;

//	r += (rand() % 1000) - 500;

	SleepHelper::msleep(3);

    return r + 0x4000;

}


XStatus SPI_Transmit(XSpi*, unsigned*, unsigned*, unsigned)
{
#ifdef ALUMINIZER_SIM
         return 0;
#else

    Xuint8* tx = (Xuint8*)dataTX;
    Xuint8* rc = (Xuint8*)dataRC;

    if(g_debug_spi)
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
   if(g_debug_spi)
      printf("spi -> rc =%08x\r\n", *dataRC);

    return s;
#endif
}



void SPI_init(XSpi*, unsigned, bool)
{
}

/******************************************************************************/
u16 SPI_Transfer2(XSpi*, u16)
{
   u16 rcv=0;

   return rcv;
}
