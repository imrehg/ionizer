#ifndef COMEDI_IO_H
#define COMEDI_IO_H

#include "analog_io.h"

#include <comedilib.h>

struct parsed_options
{
        double value;
        int channel;
        int aref;
        int range;
        int physical;
        int verbose;
        int n_chan;
        double freq;
};

//acquire data continuously
class comedi_ai : public analog_in
{
public:
        comedi_ai(comedi_t* dev, unsigned range=1, unsigned stop_src=TRIG_NONE, unsigned stop_arg=1000, double freq=1000.0, unsigned nAI=8);
        virtual ~comedi_ai();

        virtual void start();
        virtual void stop();

        virtual int getData();


protected:
        int getAvailableData();

        int prepare_cmd_lib(int n_chan, unsigned scan_period_nanosec);

        double get_datum(lsampl_t raw, int channel_index);

        comedi_t *dev;
        comedi_cmd c, *cmd;
        int subdevice;

        int subdev_flags;
        parsed_options options;
        bool isSampling;
        int total, numRead;
        unsigned stop_src, stop_arg;
        double freq;
};


class comedi_ao : public analog_out
{
public:
    comedi_ao(comedi_t* dev) : analog_out(4), dev(dev), min_ao(-4.096), max_ao(4.094) {}

    virtual void updateOutputs();

    comedi_t* dev;
    double min_ao;
    double max_ao;
};

class comedi_do : public digital_out
{
public:
    comedi_do(comedi_t* dev) : digital_out(4), dev(dev)
    {
        //configure DIO for output
        for(unsigned i=0; i<new_dOut.size(); i++)
            comedi_dio_config(dev,2,i,COMEDI_OUTPUT);
    }

    virtual bool updateOutputs();

    comedi_t* dev;
};


#endif
