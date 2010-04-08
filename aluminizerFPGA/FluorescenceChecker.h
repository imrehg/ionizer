#ifndef FLUORESCENCE_CHECKER_H
#define FLUORESCENCE_CHECKER_H

/*

   FluorescenceChecker calculates how likely it is that a bright ion would produce the observed
   level of fluorescence.  If the probability is below min_ok_prob, then checkThis returns false.
   This allows long clock probes to abort mid-pulse, and ion recovery to take place.

   count_rates[0] = dark count rate / FPGA time unit ( 10 ns)
   count_rates[1] = bright count rate / FPGA time unit ( 10 ns)

   typically,

   min_ok_prob = 1e-5 (we don't want a lot of false alarms)
   learning_rate = 1e-3

 */

class FluorescenceChecker
{
public:
FluorescenceChecker(unsigned checkInterval,
                    double dark_count_rate,
                    double bright_count_rate,
                    double min_ok_prob,
                    double learning_rate) :
	checkInterval(checkInterval),
	min_ok_prob(min_ok_prob),
	learning_rate(learning_rate),
	log_running_ratio(0)
{
	this->count_rates[0] = dark_count_rate;
	this->count_rates[1] = bright_count_rate;
}

unsigned get_check_interval() const
{
	return checkInterval;
}
bool checkThis(unsigned t, unsigned nPMT);

protected:
unsigned checkInterval;
double count_rates[2];     //count rates for dark(0) and bright(1)
double min_ok_prob;
double learning_rate;
double log_running_ratio;    //keep track of running lekelihood ratio so that dark-state confidence can grow across multiple checks
};

#endif //FLUORESCENCE_CHECKER_H
