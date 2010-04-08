#ifndef DAC_H_
#define DAC_H_

#ifdef ALUMINIZER_SIM
	#define XPAR_OPB_SPI_0_BASEADDR (0)
	#define XST_SUCCESS (0)
	#define XPAR_SPI_0_DEVICE_ID (0)
	#define XPAR_SPI_1_DEVICE_ID (1)
	#define XPAR_SPI_2_DEVICE_ID (2)
typedef unsigned XStatus;
typedef unsigned Xuint32;
typedef unsigned short u16;
typedef unsigned int u32;

class XSpi
{
public: unsigned SlaveSelectReg;
};

#else
	#include "xparameters.h"
	#include "xspi.h"
	#include "xstatus.h"
#endif

//set DAC voltage for channels 0-31 (14-bit)
//if channel > 31, set offset voltage
void SetDAC_AD5532(XSpi* spi, unsigned channel, unsigned dacWord);

//set DAC voltage for channels 0-31, 14 bits, 200V
void SetDAC_AD5535(XSpi* spi, unsigned channel, unsigned dacWord);

//set DAC voltage for channels 0-7 (16-bit)
void SetDAC_AD5668(XSpi* spi, unsigned channel, unsigned dacWord);

//set DAC voltage for channels 0-39 (16-bit)
void SetDAC_AD5370(XSpi* spi, unsigned channel, unsigned dacWord);

//start a conversion on previously spec'd ADC channel
void startConversion_AD7689(XSpi* spi);

//get ADC conversion result and configure for next conversion
unsigned short getResult_AD7689(XSpi* spi, unsigned next_channel);

XStatus SPI_Transmit(XSpi* spi, unsigned* dataTX, unsigned* dataRC, unsigned nBytes);
u16 SPI_Transfer2(XSpi * InstancePtr, u16 tx);
void SPI_init(XSpi* spi, unsigned id, bool bActiveLow);

#endif /*DAC_H_*/
