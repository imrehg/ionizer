#ifdef CONFIG_AL

#include "shared/src/messaging.h"
#include "experiments.h"
#include "host_interface.h"
#include "shared/src/Numerics.h"
#include "detection_stats.h"

#include <fstream>
#include <numeric>

using namespace std;

/*
//sorting operator detection stats
bool operator<(const detection_index& di1, const detection_index& di2)
{
	if (di1.m != di2.m)
		return di1.m < di2.m;

	if (di1.nAl != di2.nAl)
		return di1.nAl < di2.nAl;

	if (di1.ZeemanState != di2.ZeemanState)
		return di1.ZeemanState < di2.ZeemanState;

	if (di1.nState != di2.nState)
		return di1.nState < di2.nState;

	return di1.rotation_angle < di2.rotation_angle;
}



   string detection_index::name() const
   {
   string sMethod = m == push ? "PUSH " : "SB ";
   string sZS = ZeemanState < 0 ? "-" : "+";
   string sNState = to_string<int>(nState);
   string sNAl = to_string<int>(nAl) + "Al";
   string st = "t" + to_string<int>(rotation_angle);

   return sMethod + sNAl + sZS + "_" + sNState + "_" + st;
   }
 */

detection_stats::detection_stats(size_t max_counts) :
	num_poissonians(0),
	total_detections(0),
	min_detections(1000),
	counts(max_counts + 1, 0)
{
	printf("[detection_stats::detection_stats] max_counts=%u   num_poissonians=%u\r\n",
	       (unsigned)max_counts, (unsigned)num_poissonians);
}

void detection_stats::setName(const std::string& n)
{
	name = n;
}

const std::string& detection_stats::getName()
{
	return name;
}

//set the mean which is used when stats are unavailable
void detection_stats::set_mean(unsigned i, double m)
{
	printf("[detection_stats::set_mean] m(%u) = %f.\r\n", i, m);
	means.at(i) = m;
}

double detection_stats::get_mean(unsigned i)
{
	return means.at(i);
}

void detection_stats::set_weight(unsigned i, double w)
{
	printf("[detection_stats::set_weight] w(%u) = %f.\r\n", i, w);
	weights.at(i) = w;
}

double detection_stats::get_weight(unsigned i) const
{
	return weights.at(i);
}

void detection_stats::fwrite(string fname) const
{
	ofstream ofs(fname.c_str());

	for (size_t i = 0; i < counts.size(); i++)
		ofs << (int)i << ", " << counts[i] << endl;
}

double detection_stats::calcProb(size_t n) const
{
	//sum of Poissonians
	double P = 0;

	//Weight of last Poissonians is computed as remaining weight
	//after all the others are added up
	double finalWeight = 1;

	for (size_t i = 0; i < weights.size(); i++)
	{
		P += weights[i] * numerics::PoissonProb(means[i], n);
		finalWeight -= weights[i];
	}

	finalWeight = std::max(0.0, finalWeight);

	P += finalWeight * numerics::PoissonProb(means[weights.size()], n);

	return P;
}


double detection_stats::probability(size_t n)
{
//	printf("[detection_stats::probability] P(%u)\r\n", n);

	double P = 0;

	if (total_detections == 0)
		recalc();

	if (n < counts.size())
		P = counts[n] / total_detections;
	else
		P = calcProb(n);

	return std::max(0.001, P);
}

//update statistics for this state with the supplied count
void detection_stats::update(unsigned c)
{
	if (c < counts.size())
		counts[c]++;
}

void detection_stats::update_memory(double memory)
{
	for (size_t i = 0; i < counts.size(); i++)
		counts[i] *= (1 - 1.0 / memory);

	total_detections = accumulate(counts.begin(), counts.end(), 0.);
}

double detection_stats::getOverallMean()
{
	double sumCounts = 0;
	double sum = 0;

	for (size_t i = 0; i < counts.size(); i++)
	{
		sum += i * counts[i];
		sumCounts += counts[i];
	}

	return sum / sumCounts;
}

void detection_stats::recalc()
{
	reset();

	for (unsigned j = 0; j < gui_weights.size(); j++)
		set_weight(j, *(gui_weights[j]));

	for (unsigned j = 0; j < gui_means.size(); j++)
		set_mean(j, *(gui_means[j]));

	total_detections = 1000;

	for (size_t i = 0; i < counts.size(); i++)
		counts[i] = calcProb(i) * total_detections;
}

void detection_stats::set_num_poissonians(unsigned n)
{
	num_poissonians = std::max<unsigned>(n, 2); 
	reset();
}

void detection_stats::reset()
{
	if(num_poissonians != means.size())
	{
		gui_means.resize(num_poissonians, 0);
		means.resize(num_poissonians, 0);

		gui_weights.resize(num_poissonians-1, 0);
		weights.resize(num_poissonians-1, 0);
	}

	for (size_t i = 0; i < means.size(); i++)
		means[i] = 0.3 + 7 * i;

	for (size_t i = 0; i < means.size(); i++)
		means[i] = 0.3 + 7 * i;

	total_detections = 0;
}

#endif //CONFIG_AL
