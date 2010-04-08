#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "Histogram.h"

DataFeed::DataFeed()
{
	channels.push_back(xChannel = new ConstantChannel("x", false, 9));
}

DataFeed::~DataFeed()
{
	while (!channels.empty())
	{
		delete channels.back();
		channels.pop_back();
	}
}

void DataFeed::LogPMTShots(ofstream& o)
{
	for (size_t i = 0; i < channels.size(); i++)
		if (dynamic_cast<HistogramChannel*>(channels[i]))
			o << ", " << channels[i]->GetCurrentData();
}



void DataFeed::RecordChannelNames(ofstream& ofs)
{
	for (size_t i = 0; i < channels.size(); i++)
		ofs << channels[i]->GetName() << ", ";

	ofs << "time [s]";
}

void DataFeed::AcquireNewData()
{
	for_each(channels.begin(), channels.end(), mem_fun(&DataChannel::AcquireNewData));
}

void DataFeed::AcquireSingleShot()
{
	for_each(channels.begin(), channels.end(), mem_fun(&DataChannel::AcquireSingleShot));
	for_each(channels.begin(), channels.end(), mem_fun(&DataChannel::AverageCurrentData));
}


void DataFeed::StartNewAverage(double x)
{
	for (size_t i = 0; i < channels.size(); i++)
		channels[i]->StartNewAverage();

	xChannel->SetCurrentData(x);
}

void DataFeed::Clear()
{
	for_each(channels.begin(), channels.end(), mem_fun(&DataChannel::reset));
}

//append valid data to each ostream in v_os, as well as pvValidData
int DataFeed::SaveValidData(const vector<ostream*>& v_os, vector< vector<double> >* pvValidData)
{
	int num_saved = 0;

	//copy valid data into pvValidData

	if (pvValidData)
		pvValidData->push_back(vector<double>(channels.size()));

	for (size_t iChannel = 0; iChannel < channels.size(); iChannel++)
	{
		int precision = channels[iChannel]->GetPrecision();

		for (size_t i_os = 0; i_os < v_os.size(); i_os++)
		{
			*v_os[i_os] << setw(precision + 5) << setprecision(precision) << fixed;
			*v_os[i_os] << channels[iChannel]->value << ", ";
		}

		if (pvValidData)
			pvValidData->back()[iChannel] = channels[iChannel]->value;

		channels[iChannel]->UpdateStats(channels[iChannel]->value);
	}

	for (size_t i_os = 0; i_os < v_os.size(); i_os++)
		*v_os[i_os] << fixed << setprecision(3) << CurrentTime_s() << endl;

	num_saved++;

	return num_saved;
}

DataChannel* DataFeed::FindChannelNC(const string& name) const
{
	for (size_t i = 0; i < channels.size(); i++)
		if (channels[i]->GetName() == name)
			return channels[i];

	throw runtime_error("Unknown data channel: " + name);
}

DataChannel* DataFeed::FindChannel(const string& name)
{
	return FindChannelNC(name);
}


const string& DataFeed::GetName(size_t i) const
{
	return channels.at(i)->GetName();
}

