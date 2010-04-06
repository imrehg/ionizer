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
	std::valarray<double> values;
};

class analog_out
{
public:
        analog_out(unsigned numAO) : new_aOut(numAO, 0), old_aOut(numAO, -1) {}
	virtual void updateOutputs() = 0;

        void setOutput(unsigned i, double d) { new_aOut.at(i) = d; updateOutputs(); }
		  void setOutputNoUpdate(unsigned i, double d) { new_aOut.at(i) = d; }
			double* aoPtr(unsigned i) { return &(new_aOut[i]); }

        std::vector<double> new_aOut;
        std::vector<double> old_aOut;
};

class digital_out
{
public:
        digital_out(unsigned numDO) : new_dOut(numDO), old_dOut(numDO) {}
	virtual bool updateOutputs() = 0;

		  void setOutputNoUpdate(unsigned i, bool b) { new_dOut[i] = b; }
        bool* doPtr(unsigned i) { return &(new_dOut[i]); }

        std::valarray<bool> new_dOut, old_dOut;
};

#endif
