#pragma once

#include "InputParametersGUI.h"

class detection_count
{
public:
detection_count(unsigned counts, unsigned t) : counts(counts), t(t)
{
}

unsigned counts;
unsigned t;
};

struct likelihood_t
{
	double l;   // likelihood
	int nState; // state
};

class detection_index
{
public:
detection_index(int nAl, double ZeemanState, int nState, int rotation_angle) :
	nAl(nAl),
	ZeemanState(ZeemanState),
	nState(nState),
	rotation_angle(rotation_angle)
{
}

string name() const;

int nAl;
double ZeemanState;
int nState;
int rotation_angle;
};

bool operator<(const detection_index& di1, const detection_index& di2);

//maintains detection stats for a particular state
class detection_stats
{
public:
detection_stats(size_t max_counts = 20);

double probability(size_t n) const;

void update(unsigned c);
void update_memory(double memory);

void reset();

//set the mean which is used when stats are unavailable
//stats are reset
static void set_mean(int i, double m);
static double get_mean(int i);

void set_weight0(double m);
double get_weight0() const;

bool enough_stats() const;
void write(string fname) const;

GUI_double* gui_param;

protected:
vector<double> counts;
double total_detections;

//the distribution is a sum of two Poissonians, the Be bright and dark distributions of photon counts
//make the means static, to share these among all states
static double mean[2];
double weight0;

const unsigned min_detections;
};

typedef  std::map<detection_index, detection_stats> det_stats_map;
