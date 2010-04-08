#include <analog_io.h>

#ifndef _NO_NIDAQ
#include <NIDAQmx.h>
#endif

#ifndef NO_MCC

class MCC_analog_in : public analog_in
{
public:
	MCC_analog_in(int BoardNum);

	virtual void start() {};
	virtual void stop() {};

	virtual int getData();

protected:
	int BoardNum;
	std::valarray<short> data;
};

#endif

class NI_analog_in : public analog_in
{
public:
	NI_analog_in(const std::string& channels, double minV, double maxV);

	virtual void start();
	virtual void stop();

	virtual int getData();

protected:
	TaskHandle taskHandleAI;
	std::valarray<double> data;
};

class NI_analog_out : public analog_out
{
public:
	NI_analog_out(const std::string& Achannels, unsigned numAO, 
				  double minV, double maxV, double offset=0);

	virtual void updateAnalogOutputs();

protected:
	TaskHandle taskHandleAO;
	double minV, maxV, offset;
};

class NI_digital_out : public digital_out
{
public:
	NI_digital_out( const std::string& Dchannels, unsigned numDO);

	virtual bool updateDigitalOutputs();

protected:
	TaskHandle taskHandleDO;
};
