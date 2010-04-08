#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "Numerics.h"
#include "Histogram.h"
#include "detection_stats.h"

using namespace numerics;
using namespace std;

double detection_stats::mean[2] = { 0, 0 };

//sorting operator detection stats
bool operator<(const detection_index& di1, const detection_index& di2)
{
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
	string sZS = ZeemanState < 0 ? "-" : "+";
	string sNState = to_string<int>(nState);
	string sNAl = to_string<int>(nAl) + "Al";
	string st = "t" + to_string<int>(rotation_angle);

	return sNAl + sZS + "_" + sNState + "_" + st;
}

detection_stats::detection_stats(size_t max_counts) :
	counts(max_counts + 1, 0),
	total_detections(0),
	min_detections(1000)
{
	assert(max_counts > 0);
}


//set the mean which is used when stats are unavailable
void detection_stats::set_mean(int i, double m)
{
	mean[i] = m;
}

double detection_stats::get_mean(int i)
{
	return mean[i];
}

void detection_stats::set_weight0(double w)
{
	weight0 = w;
	reset();
}

double detection_stats::get_weight0() const
{
	return weight0;
}

void detection_stats::write(string fname) const
{
	ofstream ofs(fname.c_str());

	for (size_t i = 0; i < counts.size(); i++)
		ofs << (int)i << ", " << counts[i] << endl;
}

double detection_stats::probability(size_t n) const
{
	//Use observed probabilites after minimum stats have been accumulated.
	//If the observed probability is 0, or there aren't enough stats,
	//use a Poissonian distribution.
/*	if( enough_stats() && n < counts.size())
   {
      double nn = counts[n];

      if(nn)
         return nn / total_detections;
   }
 */
	return weight0 * PoissonProb(mean[0], n) + (1 - weight0) * PoissonProb(mean[1], n);
}

bool detection_stats::enough_stats() const
{
	return total_detections > min_detections;
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

	total_detections = 0;

	for (size_t i = 0; i < counts.size(); i++)
		total_detections += counts[i];

	//update means and weights after minimum stats have been recorded
	if (enough_stats())
	{
		unsigned cutoff = 2;
		double new_mean[2] = { 0, 0 };
		double total[2] = { 0, 0 };

		for (size_t i = 0; i < counts.size(); i++)
		{
			int j = i < cutoff ? 0 : 1;
			new_mean[j] += counts[i] * i;
			total[j] += counts[i];
		}

		for (int j = 0; j < 2; j++)
		{
			if (total[j])
			{
				new_mean[j] /= total[j];
				mean[j] = mean[j] * 0.99 + new_mean[j] * 0.01;
			}
		}

		weight0 = total[0] / (total[0] + total[1]);
	}
}

void detection_stats::reset()
{
	counts.assign(counts.size(), 0);
	total_detections = 0;
}
