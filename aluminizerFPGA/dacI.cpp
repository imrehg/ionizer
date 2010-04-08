#include "common.h"
#include "dacI.h"
#include "info_interface.h"

#include <assert.h>

dacI::dacI(list_t* exp_list, const std::string& name, unsigned spi_device_id) :
	info_interface(exp_list, name),
	spi_device_id(spi_device_id),
	calibrationWord("Cal. word", &params, "value=0 min=0 max=65535"),
	doCalibration("Calibrate", &params, "value=0"),
	Debug("Debug", &params, "value=0 min=0 max=65535")
{
	SPI_init(&spi, spi_device_id, true),
	calibrationWord.setFlag(RP_FLAG_UPDATE_IMMEDIATE);
	doCalibration.setFlag(RP_FLAG_UPDATE_IMMEDIATE);
}


void dacI::setCalibrationWord()
{
	unsigned u = calibrationWord;

	printf("calibrating DAC: %08X\r\n", u);
	for (size_t i = 0; i < pDAC.size(); i++)
		pDAC[i]->setDAC(u);
}

bool dacI::isInitialized(unsigned iChannel)
{
	return pDAC[iChannel]->bVoltageInitialized;
}

dacI5668::dacI5668(list_t* exp_list, const std::string& name) :
	dacI(exp_list, name, XPAR_SPI_0_DEVICE_ID)
{

	pDAC.resize(8);

	for (unsigned i = 0; i < pDAC.size(); i++)
	{
		char buff[256];
		snprintf(buff, 256, "DAC<%u>", i);
		pDAC[i] = new AD5668_prm(&spi, i, buff, &params, "value=0 min=-1 max=10");
		pDAC[i]->setFlag(RP_FLAG_UPDATE_IMMEDIATE);

		pDAC[i]->cal(0, 0, 0xFFFF, 5.0);
	}
}

void dacI5668::setDAC(unsigned iChannel, unsigned short dacWord)
{
	pDAC[iChannel]->setDAC(dacWord);
}


dacI5535::dacI5535(list_t* exp_list, const std::string& name) :
	dacI(exp_list, name, XPAR_SPI_1_DEVICE_ID)
{
	pDAC.resize(4);

	for (unsigned i = 0; i < pDAC.size(); i++)
	{
		char buff[256];
		snprintf(buff, 256, "DAC<%u>", i);
		pDAC[i] = new AD5535_prm(&spi, i, buff, &params, "value=0 min=-1 max=1000");
		pDAC[i]->setFlag(RP_FLAG_UPDATE_IMMEDIATE);
		pDAC[i]->cal(0, 0.0, 0x3FFF, 200.0);
	}

	//Port C
	pDAC[0]->cal(1000, 12.209, 15000, 186.034);
	pDAC[1]->cal(1000, 12.375, 15000, 186.615);
	pDAC[2]->cal(1000, 12.555, 15000, 186.477);
	pDAC[3]->cal(1000, 12.299, 15000, 186.147);

	/*
	   //calibration table
	   pDAC[0]->cal(10, 10.434, 190, 195.38);
	   pDAC[1]->cal(10, 10.242, 190, 194.15);
	   pDAC[2]->cal(10, 10.409, 190, 193.71);
	   pDAC[3]->cal(10, 10.056, 190, 195.15);
	   pDAC[4]->cal(10, 10.588, 190, 196.16);
	   pDAC[5]->cal(10, 10.590, 190, 195.67);
	   pDAC[6]->cal(10, 10.397, 190, 195.04);
	   pDAC[7]->cal(10, 10.453, 190, 195.75);
	   pDAC[8]->cal(10, 10.426, 190, 194.78);
	   pDAC[9]->cal(10,  9.924, 190, 194.46);
	   pDAC[10]->cal(10, 10.525, 190, 195.02);
	   pDAC[29]->cal(10, 10.240, 190, 194.17);

	   pDAC[30]->cal(10, 10.509, 50, 51.279);
	   pDAC[31]->cal(10, 10.215, 50, 50.886); */
}

dacI5532::dacI5532(list_t* exp_list, const std::string& name) :
	dacI(exp_list, name, XPAR_SPI_0_DEVICE_ID),
	offsetWord("Offset",   &params, "value=0")
{
	offsetWord.setFlag(RP_FLAG_UPDATE_IMMEDIATE);
	pDAC.resize(32);

	for (unsigned i = 0; i < pDAC.size(); i++)
	{
		char buff[256];
		snprintf(buff, 256, "DAC<%u>", i);
		pDAC[i] = new AD5532_prm(&spi, i, buff, &params, "value=0 min=-1 max=10");
		pDAC[i]->setFlag(RP_FLAG_UPDATE_IMMEDIATE);
	}
}

void dacI5532::updateParams()
{
	dacI::updateParams();

	SetDAC_AD5532(0, 33, offsetWord);
}

void dacI::setVoltages(const std::vector<unsigned>& uV)
{
	for (unsigned i = 0; i < uV.size(); i++)
		pDAC[i]->setVoltage(static_cast<int>(uV[i]) * 1.0e-6);
}

int dacI::get_uV(unsigned iChannel)
{
	return static_cast<int>((*pDAC[iChannel]) * 1e6);
}


void dacI::setVoltage(unsigned iChannel, int uV)
{
//	printf("[dacI::setVoltage] (%s) %u %u\r\n", name.c_str(), iChannel, uV);

	if (iChannel < pDAC.size())
		pDAC[iChannel]->setVoltage(uV * 1.0e-6);
}

dacI5370::dacI5370(list_t* exp_list, const std::string& name) :
	dacI(exp_list, name, XPAR_SPI_0_DEVICE_ID)
{
	pDAC.resize(8);

	for (unsigned i = 0; i < pDAC.size(); i++)
	{
		char buff[256];
		snprintf(buff, 256, "DAC<%u>", i);

		if (i == 4)
			pDAC[i] = new AD5370_prm(&spi, i, buff, &params, "value=0 min=0 max=800");
		else
			pDAC[i] = new AD5370_prm(&spi, i, buff, &params, "value=0 min=-40 max=40");

		pDAC[i]->setFlag(RP_FLAG_UPDATE_IMMEDIATE);
		pDAC[i]->cal(0, -10, 0xFFFF, 10);
	}

	//Port A
	pDAC[0]->cal(13000, -30.9734, 53000, 31.6961);
	pDAC[1]->cal(13000, -30.9847, 53000, 31.7295);
	pDAC[2]->cal(13000, -31.0319, 53000, 31.7594);
	pDAC[3]->cal(13000, -30.9578, 53000, 31.7256);

	//Port B
	pDAC[4]->cal(33000, 6.94, 59000, 805.68);
	pDAC[5]->cal(0, -10.00345 - 0.0075, 0xFFFF, 10.01259 - 0.0075);
	pDAC[6]->cal(0, -10.0033, 0xFFFF, 10.01248);
	pDAC[7]->cal(0, -10.01075, 0xFFFF, 10.00779);
}

