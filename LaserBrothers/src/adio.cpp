#include "adio.h"
#include <string>

#ifdef WIN32
#define  snprintf  _snprintf
#include "win_io.h"
#else
#include "comedi_io.h"
#endif

using namespace std;

adio::adio(InputParameters* params) :
ioDevice0("IO Device 0", params, "sim_io"),
ioDevice1("IO Device 1", params, ""),
ioDeviceAI0("IO Device ai 0", params, "ai0:3"),
ioDeviceAO0("IO Device ao 0", params, "ao0:3"),
ioDeviceDO0("IO Device do 0", params, "port0/line0:1"),
ioDeviceAI1("IO Device ai 1", params, "ai0:3"),
ioDeviceAO1("IO Device ao 1", params, "ao0:3"),
ioDeviceDO1("IO Device do 1", params, "port0/line0:1"),
minAI0("IO Device ai 0 min V", params, "-5"),
maxAI0("IO Device ai 0 max V", params, "5"),
minAI1("IO Device ai 1 min V", params, "-5"),
maxAI1("IO Device ai 1 max V", params, "5")
{
	setupIO();
}

void adio::setupIO()
{
    if(ioDevice0.Value().find("sim_io") != string::npos)
	{
		sim_io* sio = new sim_io(8,4,4);
		aIn.push_back(sio);
                aOut.push_back(sio);
                dOut.push_back(sio);

		return;
	}

#ifdef HAS_COMEDI_ADIO
	//comedi IO device
	if(ioDevice0.Value().find("comedi") != string::npos)
	{
        /* open the device */
        comedi_t* dev = comedi_open(ioDevice0.Value().c_str());

        if(dev == 0)
        {
            cerr << "Can't open " << ioDevice0 << " ... exiting" << endl;
            exit(1);
        }


        aIn.push_back(new comedi_ai(dev, 1, TRIG_NONE, 0,500.0, 8));
        aOut.push_back(new comedi_ao(dev));
        dOut.push_back(new comedi_do(dev));
	}
#endif //HAS_COMEDI_ADIO

#ifdef HAS_NI_ADIO
	//NI IO device
	if(ioDevice0.Value().find("Dev") != string::npos)
	{
		aIn.push_back(new NI_analog_in(ioDevice0.Value() + ioDeviceAI0.Value(), minAI0, maxAI0));
		aOut.push_back(new NI_analog_out(ioDevice0.Value() + ioDeviceAO0.Value(), 2, 0, 5, 2.5));
		dOut.push_back(new NI_digital_out(ioDevice0.Value() + ioDeviceDO0.Value(), 2));
	}

	if(ioDevice1.Value().find("Dev") != string::npos)
	{
		aIn.push_back(new NI_analog_in(ioDevice1.Value() + ioDeviceAI1.Value(), minAI1, maxAI1));
		aOut.push_back(new NI_analog_out(ioDevice1.Value() + ioDeviceAO1.Value(), 2, 0, 5, 2.5));
		dOut.push_back(new NI_digital_out(ioDevice1.Value() + ioDeviceDO1.Value(), 2));
	}
#endif //HAS_NI_ADIO
}

double adio::get_ai(unsigned iDev, unsigned iChan)
{
        return *(aIn.at(iDev)->ptrData(iChan));
}

void adio::set_ao(unsigned iDev, unsigned iChan, double d)
{
    aOut.at(iDev)->setOutputNoUpdate(iChan, d);
}

void adio::set_do(unsigned iDev, unsigned iChan, bool b)
{
    dOut.at(iDev)->setOutputNoUpdate(iChan, b);
}
