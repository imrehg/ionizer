#ifndef VOLTAGES_H_
#define VOLTAGES_H_

#include "info_interface.h"
#include "experiments.h"
#include "remote_params.h"
#include "dac.h"

class dacI;

class rp_ao : public rp_lcd
{
public:
rp_ao(const std::string& name, std::vector<remote_param*>* v, const std::string& init_str,
      dacI* pDAC, unsigned iChannel) :
	rp_lcd(name, v, init_str),
	pDAC(pDAC),
	iChannel(iChannel)
{
}

virtual ~rp_ao()
{
}

void set_ao(double V);
double get_ao();

protected:
dacI* pDAC;
unsigned iChannel;
};

class DAC_param : public rp_double
{
public:
DAC_param(XSpi* board, unsigned iChannel, const std::string& name, std::vector<remote_param*>* v, const std::string& init_str) :
	rp_double(name, v, init_str),
	board(board),
	iChannel(iChannel),
	currentWord(0xFFFFFFFF),
	G(1),
	offset(0),
	useCalibration(false),
	bVoltageInitialized(false)
{
}

virtual void setDAC(unsigned dacWord) = 0;

//calibrate DAC from 2 DAC codes (dacXn) and 2 measured voltages (vXm).
//dac1 > dac0
void cal(unsigned dac0, double v0m, unsigned dac1, double v1m)
{
	G = (dac1 - dac0) / (v1m - v0m);
	offset = v0m - dac0 / G;
}


unsigned calcDACword(double v)
{
	double dacD = (v - offset) * G;    //calculate DAC code

	//	double dacD = std::max<double>(0.0, 0x3FFF * ((V + offset)/3.52 - 0.05)/2.95);

	return static_cast<unsigned>(0.5 + dacD);     //round to nearest integer
}

void setDACVoltage(double vNew)
{
	setDAC(calcDACword(vNew));
}

virtual void setValueFromString(const char* s)
{
	rp_double::setValueFromString(s);
	setDACVoltage(value);
}

void setVoltage(const double& v)
{
	rp_double::set(v);
	setDACVoltage(value);
	bVoltageInitialized = true;
}


XSpi* board;
unsigned iChannel;
unsigned currentWord;

double G, offset;
bool useCalibration;
bool bVoltageInitialized;    //becomes true after first setVoltage call
};

class AD5532_prm : public DAC_param
{
public:
AD5532_prm(XSpi* board, unsigned iChannel, const std::string& name, std::vector<remote_param*>* v, const std::string& init_str) :
	DAC_param(board, iChannel, name, v, init_str)
{
}

virtual void setDAC(unsigned dacWord)
{
	if (currentWord != dacWord)
	{
		SetDAC_AD5532(board, iChannel, dacWord);
		currentWord = dacWord;
	}
}
};
class AD5535_prm : public DAC_param
{
public:
AD5535_prm(XSpi* board, unsigned iChannel, const std::string& name, std::vector<remote_param*>* v, const std::string& init_str) :
	DAC_param(board, iChannel, name, v, init_str)
{
}

virtual void setDAC(unsigned dacWord)
{
	if (currentWord != dacWord)
	{
		SetDAC_AD5535(board, iChannel, dacWord);
		currentWord = dacWord;
	}
}
};

class AD5370_prm : public DAC_param
{
public:
AD5370_prm(XSpi* board, unsigned iChannel, const std::string& name, std::vector<remote_param*>* v, const std::string& init_str) :
	DAC_param(board, iChannel, name, v, init_str)
{
}

virtual void setDAC(unsigned dacWord)
{
	if (currentWord != dacWord)
	{
		SetDAC_AD5370(board, iChannel, dacWord);
		currentWord = dacWord;
	}
}
};

class AD5668_prm : public DAC_param
{
public:
AD5668_prm(XSpi* board, unsigned iChannel, const std::string& name, std::vector<remote_param*>* v, const std::string& init_str) :
	DAC_param(board, iChannel, name, v, init_str)
{
}

virtual void setDAC(unsigned dacWord)
{
	if (currentWord != dacWord)
	{
		SetDAC_AD5668(board, iChannel, dacWord);
		currentWord = dacWord;
	}
}
};

class dacI : public info_interface
{
public:
dacI(list_t* exp_list, const std::string& name, unsigned spi_device_id);
virtual ~dacI()
{
}

bool isInitialized(unsigned iChannel);
virtual void setVoltages(const std::vector<unsigned> &uV);
virtual void setVoltage(unsigned iChannel, int uV);
int get_uV(unsigned iChannel);

void setCalibrationWord();

virtual void updateParam(const char* name, const char* value)
{
	info_interface::updateParam(name, value);

	if (Debug)
		printf("%s = %s\r\n", name, value);

	if (doCalibration)
		setCalibrationWord();
}

unsigned spi_device_id;
XSpi spi;

rp_unsigned calibrationWord;
rp_bool doCalibration;
rp_bool Debug;

std::vector<DAC_param*> pDAC;
};

class dacI5668 : public dacI
{
public:
dacI5668(list_t* exp_list, const std::string& name);
virtual ~dacI5668()
{
}

void setDAC(unsigned iChannel, unsigned short dacWord);
};

class dacI5532 : public dacI
{
public:
dacI5532(list_t* exp_list, const std::string& name);
virtual ~dacI5532()
{
}

virtual void updateParams();

rp_unsigned offsetWord;
};

class dacI5535 : public dacI
{
public:
dacI5535(list_t* exp_list, const std::string& name);
virtual ~dacI5535()
{
}

//	virtual void setVoltages(const std::vector<unsigned> &uV);
};

class dacI5370 : public dacI
{
public:
dacI5370(list_t* exp_list, const std::string& name);
virtual ~dacI5370()
{
}
};

#endif /*VOLTAGES_H_*/
