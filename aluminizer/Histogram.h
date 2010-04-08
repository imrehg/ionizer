#pragma once

using namespace std;

/*
   DataChannels are ring accumulate data for the current scan.
   They are grouped together and managed by the DataFeed.
 */

class DataChannel
{
public:
DataChannel(const string& sName, bool bPlot, int precision = 3);

virtual ~DataChannel()
{
}

void SetName(const string& s)
{
	sName = s;
}
const string& GetName() const
{
	return sName;
}

void SetCurrentData(double d)
{
	CurrentData = d;
}
virtual void StartNewAverage();

void SetMinValue(int mv)
{
	minValue = mv;
}
void SetMaxValue(int mv)
{
	maxValue = mv;
}

double GetAverage() const
{
	return value;
}

virtual double AverageCurrentData();

int GetPrecision() const
{
	return precision;
}

void LogShot()
{
	cerr << sName << " = " << setprecision(3) << setw(6) << CurrentData << "   ";
}
void LogAverage()
{
	cerr << sName << " = " << setprecision(3) << setw(6) << GetAverage() << "   ";
}

void UpdateStats(double);
void SetPlot(bool b)
{
	bPlot = b;
}

friend class DataFeed;

double GetValue()
{
	return value;
}
double GetCurrentData() const
{
	return CurrentData;
}

protected:
virtual void reset()
{
}

virtual double AcquireNewData();
virtual double AcquireSingleShot();

bool ShouldPlot() const
{
	return bPlot;
}

double value;

protected:

string sName;
double CurrentData;

bool bPlot;
unsigned NumPointsInAverage;     //the number of data points that make up this average
unsigned NumShots;               //the number of shots of the experiment that make up this average
unsigned NumInStats;


int minValue;
int maxValue;

int precision;

public:
double total;
double mean;
double sum_squared_differences;
double std_dev;
};

class HistogramChannel : public DataChannel
{
public:
HistogramChannel(unsigned size, const string& sName, int precision = 3, bool uses_new_fpga = false) :
	DataChannel(sName, true, precision),
	data(0u, size),
	uses_new_fpga(uses_new_fpga)
{
}

virtual ~HistogramChannel()
{
}

virtual double AcquireSingleShot();
virtual void StartNewAverage();
virtual void SetHistogram(const std::vector<unsigned int>& r);

const std::valarray<unsigned>& GetHistogram() const
{
	return data;
}

protected:
double InternalAverage() const;

private:
string Name;

std::valarray<unsigned int> data;
bool uses_new_fpga;
};

class ConstantChannel : public DataChannel
{
public:
ConstantChannel(const string& sName, bool bPlot, int precision = 9) :
	DataChannel(sName, bPlot, precision)
{
}

virtual ~ConstantChannel()
{
}

void SetCurrentAverage(double d)
{
	value = d;
	NumPointsInAverage = 1;
}

virtual double AverageCurrentData()
{
	value = CurrentData;
	NumPointsInAverage++;
	return value;
}
};


class DataFeed
{
public:
DataFeed();
virtual ~DataFeed();

void Clear();
DataChannel* AddChannel(DataChannel* pChannel)
{
	channels.push_back(pChannel); return pChannel;
}

void StartNewAverage(double x);
void AcquireNewData();
void AcquireSingleShot();
void LogPMTShots(ofstream&);

int SaveValidData(const vector<ostream*>& v_os, vector< vector<double> >* pvValidData);

const string& GetName(size_t i) const;

size_t NumChannels() const
{
	return channels.size();
}

DataChannel* FindChannel(const string& name);

bool ShouldPlot(size_t i) const
{
	return channels.at(i)->ShouldPlot();
}

void RecordChannelNames(ofstream&);
double getAverage(size_t i)
{
	return channels.at(i)->GetAverage();
}

protected:
DataChannel* FindChannelNC(const string& name) const;

vector<DataChannel*> channels;

ConstantChannel* xChannel;    //contains x-coordinate information for the scan
};

