#include "comedi_io.h"

#include <iostream>
#include <stdexcept>

#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <QTime>

using namespace std;

#define BUFSZ 10000
char buf[BUFSZ];

#define N_CHANS 256
static unsigned int chanlist[N_CHANS];
static comedi_range * range_info[N_CHANS];
static lsampl_t maxdata[N_CHANS];

char *cmdtest_messages[]={
        "success",
        "invalid source",
        "source conflict",
        "invalid argument",
        "argument conflict",
        "invalid chanlist",
};



comedi_ai::comedi_ai(comedi_t* dev, unsigned range, unsigned stop_src, unsigned stop_arg, double freq, unsigned nAI) :
    analog_in(nAI),
    dev(dev),
    subdevice(0),
    isSampling(false),
    total(0),
    numRead(0),
    stop_src(stop_src),
    stop_arg(stop_arg),
    freq(freq)
{
    cmd = &c;

    int ret;
    int i;

    /* The following variables used in this demo
     * can be modified by command line
     * options.  When modifying this demo, you may want to
     * change them here. */
    options.channel = 0;
    options.range = range;
    options.aref = AREF_GROUND;
    options.n_chan = nAI;

    options.freq = freq;
    options.verbose = true;

    if(!dev){
            exit(1);
    }

    // Print numbers for clipped inputs
    comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

    /* Set up channel list */
    for(i = 0; i < options.n_chan; i++){
            chanlist[i] = CR_PACK(options.channel + i, options.range, options.aref);
            range_info[i] = comedi_get_range(dev, subdevice, options.channel, options.range);
            maxdata[i] = comedi_get_maxdata(dev, subdevice, options.channel);
    }

    /* prepare_cmd_lib() uses a Comedilib routine to find a
     * good command for the device.  prepare_cmd() explicitly
     * creates a command, which may not work for your device. */
    prepare_cmd_lib(options.n_chan, 1e9 / options.freq);
//    prepare_cmd(options.n_chan, 1e9 / options.freq);

    /* comedi_command_test() tests a command to see if the
     * trigger sources and arguments are valid for the subdevice.
     * If a trigger source is invalid, it will be logically ANDed
     * with valid values (trigger sources are actually bitmasks),
     * which may or may not result in a valid trigger source.
     * If an argument is invalid, it will be adjusted to the
     * nearest valid value.  In this way, for many commands, you
     * can test it multiple times until it passes.  Typically,
     * if you can't get a valid command in two tests, the original
     * command wasn't specified very well. */
    ret = comedi_command_test(dev, cmd);
    if(ret < 0){
            comedi_perror("comedi_command_test");
            if(errno == EIO){
                    fprintf(stderr,"Ummm... this subdevice doesn't support commands\n");
            }
            exit(1);
    }
    ret = comedi_command_test(dev, cmd);
    if(ret < 0){
            comedi_perror("comedi_command_test");
            exit(1);
    }
    fprintf(stderr,"second test returned %d (%s)\n", ret,
                    cmdtest_messages[ret]);
    if(ret!=0){
            fprintf(stderr, "Error preparing command\n");
            exit(1);
    }
}


comedi_ai::~comedi_ai()
{
    stop();
}

void comedi_ai::start() //virtual
{
    if(!isSampling)
    {
        numRead = 0;

        /* start the command */
        int ret = comedi_command(dev, cmd);
        if(ret < 0)
        {
            comedi_perror("comedi_command");
            isSampling = false;
        }
        else
        {
            isSampling = true;

            if(total == 0)
                subdev_flags = comedi_get_subdevice_flags(dev, subdevice);
        }
    }
}

void comedi_ai::stop() //virtual
{
    if(isSampling)
    {
        isSampling = false;

        comedi_cancel(dev, subdevice);

        //read remaining samples and discard
        int ret = 1;
        while(ret != 0)
        {
            ret = read(comedi_fileno(dev),buf,BUFSZ);
        }
    }
}

int comedi_ai::getAvailableData()
{
    int n = 0;

    if(comedi_get_buffer_contents(dev,subdevice))
    {
        int ret = read(comedi_fileno(dev),buf,BUFSZ);

        if(ret < 0){
                /* some error occurred */
                fprintf(stderr,"ERROR\n");
                perror("read");
                isSampling = false;
        }else
        {
            n += ret;
            numRead += ret;

            if(ret > 0)
            {
                lsampl_t raw;
                static int col = 0;
                int bytes_per_sample;
                total += ret;

                if(subdev_flags & SDF_LSAMPL)
                        bytes_per_sample = sizeof(lsampl_t);
                else
                        bytes_per_sample = sizeof(sampl_t);
                for(int i = 0; i < ret / bytes_per_sample; i++){
                        if(subdev_flags & SDF_LSAMPL) {
                                raw = ((lsampl_t *)buf)[i];
                        } else {
                                raw = ((sampl_t *)buf)[i];
                        }
                        values[col] = get_datum(raw, col);

//                        cout << string(QTime::currentTime().toString().toAscii()) << " V[" << col << "] = " << values[col] << endl;

                        col++;
                        if(col == values.size())
                        {
                                col=0;
                        }
                    }
            }
            else
                fprintf(stderr, "NO SAMPLES\n");
        }
    }

    return n;
}

int comedi_ai::getData() //virtual
{
    int n = 0;

    if(isSampling)
    {
        n = getAvailableData();
    }

    start();

    return n;
}

/*
 * This prepares a command in a pretty generic way.  We ask the
 * library to create a stock command that supports periodic
 * sampling of data, then modify the parts we want. */
int comedi_ai::prepare_cmd_lib(int n_chan, unsigned scan_period_nanosec)
{
        int ret;

        memset(cmd,0,sizeof(*cmd));

        /* This comedilib function will get us a generic timed
         * command for a particular board.  If it returns -1,
         * that's bad. */
        ret = comedi_get_cmd_generic_timed(dev, subdevice, cmd, n_chan, scan_period_nanosec);

        if(ret<0){
                printf("comedi_get_cmd_generic_timed failed\n");
                return ret;
        }

        /* Modify parts of the command */
        /* Modify parts of the command */
        cmd->chanlist           = chanlist;
        cmd->chanlist_len       = n_chan;

        cmd->stop_src =	stop_src;
        cmd->stop_arg =	stop_arg;

        return 0;
}

double comedi_ai::get_datum(lsampl_t raw, int channel_index)
{
    return comedi_to_phys(raw, range_info[channel_index], maxdata[channel_index]);
}

void comedi_ao::updateOutputs()
{
    for(unsigned i=0; i<new_aOut.size(); i++)
    {
        if(old_aOut[i] != new_aOut[i])
        {
            if(new_aOut[i] > max_ao)
                new_aOut[i] = max_ao;

            if(new_aOut[i] < min_ao)
                new_aOut[i] = min_ao;

            old_aOut[i] = new_aOut[i];
            lsampl_t data = 4096 * (new_aOut[i] + 4.096)/8.192;
//            cout << "comedi ao [" << i << "] " << data << " (" << new_aOut[i] << " V)" << endl;
            comedi_data_write(dev,1,i,0,0, data);
        }
    }
}

bool comedi_do::updateOutputs()
{
    bool bDifferent = false;

    for(unsigned i=0; i<new_dOut.size(); i++)
    {
        if(old_dOut[i] != new_dOut[i])
        {
            bDifferent = true;

            old_dOut[i] = new_dOut[i];
            comedi_dio_write(dev,2,i,new_dOut[i] ? 1 : 0);
        }
    }

    return bDifferent;
}
