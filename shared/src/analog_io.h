#ifndef IO_H
#define IO_H

#include <QThread>

#include <valarray>
#include <vector>
#include <string>

class SleepHelper: public QThread
{
public:
   static void msleep(int ms)
   {
      QThread::msleep(ms);
   }
};

class analog_in
{
public:
	analog_in(unsigned nValues);
	virtual ~analog_in() {}

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual int getData() = 0;

	double data(unsigned i) {return values[i]; }
	double* ptrData(unsigned i) {return &(values[i]); }

protected:
	std::vector<double> values;
};

class analog_out
{
public:
	analog_out(unsigned numAO) : new_aOut(numAO, 0), old_aOut(numAO, -1) {}
	virtual void updateAnalogOutputs() = 0;

	void setOutput(unsigned i, double d) { new_aOut.at(i) = d; updateAnalogOutputs(); }
	void setOutputNoUpdate(unsigned i, double d) { new_aOut.at(i) = d; }
	double* aoPtr(unsigned i) { return &(new_aOut[i]); }

	std::vector<double> new_aOut;
	std::vector<double> old_aOut;
};

class digital_out
{
public:
	digital_out(unsigned numDO) : new_dOut(numDO), old_dOut(numDO) {}
	virtual bool updateDigitalOutputs() = 0;

	void setOutputNoUpdate(unsigned i, bool b) { new_dOut[i] = b; }
	bool* doPtr(unsigned i) { return &(new_dOut[i]); }

	std::valarray<bool> new_dOut, old_dOut;
};

class sim_io : public analog_in, public analog_out, public digital_out
{
public:
	sim_io(unsigned nAI, unsigned nAO, unsigned nDO) : analog_in(nAI), analog_out(nAO), digital_out(nDO)
	{}

	virtual void start() {}
	virtual void stop() {}
	virtual int getData() { return values.size(); }

	virtual void updateAnalogOutputs() {for(unsigned i=0; i<values.size(); i++) values[i] = 0.3*(i+1)*new_aOut[i/2]*((i+1)%2);}
	virtual bool updateDigitalOutputs() { return true; }
};

#endif
