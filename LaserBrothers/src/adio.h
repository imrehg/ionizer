#ifndef ADIO_H
#define ADIO_H

#include "InputParameters.h"
#include "analog_io.h"

//! Analog and digital IO device.
class adio
{
public:
	adio(InputParameters* params);
	void setupIO();

	double get_ai(unsigned iDev, unsigned iChan);
	void set_ao(unsigned iDev, unsigned iChan, double d);
	void set_do(unsigned iDev, unsigned iChan, bool b);

protected:
	std::vector<analog_out*> aOut;
	std::vector<digital_out*> dOut;
	std::vector<analog_in*> aIn;

	param_string ioDevice0, ioDevice1;
	param_string ioDeviceAI0, ioDeviceAO0, ioDeviceDO0;
	param_string ioDeviceAI1, ioDeviceAO1, ioDeviceDO1;
	param_double minAI0, maxAI0, minAI1, maxAI1;
};

#endif // ADIO_H
