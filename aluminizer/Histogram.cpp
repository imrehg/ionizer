#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "AluminizerApp.h"
#include "Histogram.h"
#include "ExperimentPage.h"
#include "FPGA_connection.h"

DataChannel::DataChannel(const string& sName, bool bPlot, int precision) :
	value(0),
	sName(sName),
	CurrentData(0),
	bPlot(bPlot),
	NumPointsInAverage(0),
	NumShots(0),
	NumInStats(0),
	minValue(numeric_limits<int>::min()),
	maxValue(numeric_limits<int>::max()),
	precision(precision),
	total(0),
	mean(1),
	sum_squared_differences(0),
	std_dev(1)
{
}

void DataChannel::StartNewAverage()
{
	value = 0;
	NumShots = 0;
	NumPointsInAverage = 0;
}

double DataChannel::AcquireNewData()
{
	NumShots++;
	return AverageCurrentData();
}

double DataChannel::AcquireSingleShot()
{
	NumShots++;

	return CurrentData;
}

double DataChannel::AverageCurrentData()
{
	//average in the current data

	value = (value * NumPointsInAverage + CurrentData) / (NumPointsInAverage + 1);
	NumPointsInAverage++;

	return value;
}

void DataChannel::UpdateStats(double d)
{
	NumInStats++;

	total += d;
	mean = total / NumInStats;
	sum_squared_differences += (d - mean) * (d - mean);
	std_dev = sqrt(sum_squared_differences / NumInStats);
}

double HistogramChannel::AcquireSingleShot()
{
	return 0;
}


double HistogramChannel::InternalAverage() const
{
	double average = 0;

	for (unsigned i = 0; i < data.size(); i++)
		average += i * data[i];

	if (double sum = data.sum())
		return average / sum;
	else
		return 0;
}

void HistogramChannel::StartNewAverage()
{
	for (size_t j = 0; j < data.size(); j++)
		data[j] = 0;

	DataChannel::StartNewAverage();
}

void HistogramChannel::SetHistogram(const std::vector<unsigned int>& r)
{
	unsigned nMax = data.size();

	for (size_t i = 0; i < r.size(); i++)
		if (r[i] < nMax)
			data[r[i]]++;

	NumShots = r.size();
}
