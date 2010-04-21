#ifndef DETECTION_STATS_H
#define DETECTION_STATS_H

#include <map>
#include <iostream>
#include "remote_params.h"

using namespace std;

class detection_info
{
public:
detection_info(unsigned counts, unsigned t3P1, double f3P0, double t3P0, double p3P0) :
	counts(counts), t3P1(t3P1), f3P0(f3P0), t3P0(t3P0), p3P0(p3P0)
{
}

void log(ostream& o)
{
	o << counts << ", " << t3P1 << ", " << f3P0 << ", " << t3P0 << ", " << p3P0;
}

unsigned counts;
unsigned t3P1;

double f3P0, t3P0, p3P0;
};

struct likelihood_t
{
	double l;   // likelihood
	int nState; // state
};

/*
class detection_index
{
public:
enum method { sideband, push };

detection_index(method m, int nAl, double ZeemanState, int nState, int rotation_angle) :
	m(m),
	nAl(nAl),
	ZeemanState(ZeemanState),
	nState(nState),
	rotation_angle(rotation_angle)
{
}

string name() const;

method m;
int nAl;
double ZeemanState;
int nState;
int rotation_angle;
};

bool operator<(const detection_index& di1, const detection_index& di2);
*/

/* Maintains detection stats for a particular state
   Probability histograms are stored, and a sum-of-possonians model is used
   to initialize the histograms.  The number of Poissonians corresponds to 
   one plus the number of fluorescing ions.  The number of weights for these
   Poissonians has to add up to one, so the number of weights equals the number of fluorecing ions.
*/

class detection_stats
{
public:
detection_stats(size_t max_counts = 20);

void setName(const std::string& n);
const std::string& getName();

double calcProb(size_t n) const;
double probability(size_t n);

//! calculates overall mean for this set of detection statistics
double getOverallMean();

void update(unsigned c);
void update_memory(double memory);

void reset();

//! set the mean of component "i" to "m" (used when stats are unavailable).  stats are reset
void set_mean(unsigned i, double m);
double get_mean(unsigned i);

void set_num_poissonians(unsigned n);
void set_weight(unsigned i, double m);
double get_weight(unsigned i) const;

void fwrite(string fname) const;

void recalc();

unsigned num_poissonians;

protected:

double total_detections;
unsigned min_detections;

public:
vector<rp_double*> gui_weights;
vector<rp_double*> gui_means;

unsigned getMaxCounts()
{
	return counts.size();
}

protected:
vector<double> counts;


//the distribution is a sum of num_poissonians Poissonians,
//the Mg+ distributions of photon counts
vector<double> means;

vector<double> weights;

std::string name;
};

//typedef  std::map<detection_index, detection_stats> det_stats_map;

#endif //DETECTION_STATS_H
