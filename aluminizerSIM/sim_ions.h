#pragma once

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdlib.h>

#include <vector>
#include <map>
#include <deque>

#include <QThread>
#include <QTime>
#include <QSharedMemory>

#include "Transition.h"
#include "HFS_Mg.h"
#include "HFS_Al.h"

class sim_ions;

#define Mg_Detect_f0 (1.1e15)
#define Mg_HFS       (1789.0e6)
#define Al_3P1_f0    (-240.0e6)
#define Al_3P0_f0    (2*85e6 + -217.0e6)

class SleepHelper: public QThread
{
public:
   static void msleep(int ms)
   {
      QThread::msleep(ms);
   }
};

class laser_AOM
{
public:
   laser_AOM(unsigned iDDS, int order) : iDDS(iDDS), order(order) {}

   double getFrequencyShift(sim_ions* sim);

protected:
   unsigned iDDS;
   int order;
};

class laser_beam
{
public:
   //f0 is the frequency offset used in calculating the transition prob.
   //the total returned by getFrequency() is the sum of f0 and the various AOM shifts
   laser_beam(double f0, int pol) : f0(f0), pol(pol) {}

   void addAOM(const laser_AOM& aom) { AOMs.push_back(aom); }

   double getFrequency(sim_ions* sim);
   int getPol() const { return pol; }
   void setPol(int p) { pol = p; }

protected:
   double f0;
   int pol;
   std::vector<laser_AOM> AOMs;
};


class Mg_cycling
{
public:
   Mg_cycling(double f0, double scattering_rate, laser_beam* laser) :
            omega0(2*M_PI*f0),
            scattering_rate(scattering_rate),
            laser(laser),
            gamma(40e6)
   {}


   unsigned pulse(double t, sim_ions* sim);

   double omega0;
   double scattering_rate;
   laser_beam* laser;
   double gamma;
};

typedef unsigned int motion_t;

class sim_transition
{
public:
   sim_transition(const std::string& name,
              physics::HFS* g,
              physics::HFS* e,
              double f0, double tPi,
              std::vector<double> fZ,
              std::vector<double> eta,
              laser_beam* laser,
              double lifetime);

   void set_state(unsigned state_index);

   //convert from multi-dimensional state spec to linear state-index
   unsigned si(unsigned i, double mF, motion_t m) const;

   //convert a pair of si indeces to a 1-D key for the coupling matrix
   unsigned cm_key(unsigned i1, unsigned i2) const;

   void calc_energies();
   void calc_coupling();

   //return energy of internal state k
   double getEnergy(unsigned k);

   //get electronic state
   unsigned get_IS(unsigned k) const;

   //return mF-value associated with state k
   double get_mF(unsigned k) const;

    //get motional state
   unsigned get_sb(unsigned k) const;

   std::string get_state_name(unsigned k) const;

   void print_current_state();

   virtual void pulse(double t, sim_ions* sim);
   virtual void repump() = 0;
   virtual void decay(double); //simulate spont. decay
   virtual void project();

   motion_t getMotionalState();
   void setMotionalState(motion_t m);

   physics::MotionalTransition X;
   unsigned nZSg, nZSe; //# Zeeman states
   unsigned nIS; //# internal states
   unsigned nMS; //# motional states
   unsigned nStates;

   laser_beam* laser;
   double lifetime;

   std::map<unsigned, double> cm; //sparse coupling matrix. map from cm_key to Rabi rate

public:
   std::vector<double> state_pop; //population of various states
   std::vector<double>* pop_motion;

   std::vector<double> state_E; //energies of various states (divided by hbar)

protected:
   std::vector<unsigned int> driven_states_g; //list of ground states that are driven

public:
   unsigned projected_state;

   std::string name;
   double B; //magnetic field in Tesla
   double tPi; //carrier pi-time
};

class Mg_Raman : public sim_transition
{
public:
   Mg_Raman(double f0, double tPi, std::vector<double> fZ, std::vector<double> eta, laser_beam* laser) :
     sim_transition("Mg Raman",
        (new physics::HFS_Mg25_II_S_One_Half(3)),
        (new physics::HFS_Mg25_II_S_One_Half(2)),
        f0, tPi, fZ, eta, laser, 1e99)
   {
      driven_states_g.push_back(si(0, -3, 0));
      driven_states_g.push_back(si(0, -3, 1));
      driven_states_g.push_back(si(0, -3, 2));

      calc_coupling();
   }

   virtual void repump();
};

class Al;

class Al_1S0_3P1 : public sim_transition
{
public:
   Al_1S0_3P1(double f0, double tPi, std::vector<double> fZ, std::vector<double> eta, laser_beam* laser);

   virtual void repump();
};

class Al_1S0_3P0 : public sim_transition
{
public:
   Al_1S0_3P0(double f0, double tPi, laser_beam* laser) :
      sim_transition("Al 3P0",
        (new physics::HFS_Al_II_SingletS0()),
        (new physics::HFS_Al_II_TripletP0()),
        f0, tPi, std::vector<double>(0), std::vector<double>(0), laser, 20)
   {
       driven_states_g.push_back(si(0, -2.5, 0));
      driven_states_g.push_back(si(0, 2.5, 0));

      calc_coupling();

   }

   virtual void repump();
};

class Mg
{
public:
   Mg(laser_beam* detection_laser, laser_beam* raman_laser) :
      detection_laser(detection_laser),
      raman_laser(raman_laser),
      detection_transition(Mg_Detect_f0, 4/(100e-6), detection_laser),
      raman_transition(Mg_HFS, 1e-6, get_fZ(), get_eta(), raman_laser)
   {}

   static std::vector<double> get_fZ();
   static std::vector<double> get_eta();

   void init();
   void cool();
   void pulse(double t, unsigned ttl, sim_ions* sim);

   laser_beam* detection_laser;
   laser_beam* raman_laser;

   Mg_cycling detection_transition;
   Mg_Raman raman_transition;
};

class Al
{
public:
   Al(laser_beam* laser3P1, laser_beam* laser3P0) :
      laser3P1(laser3P1),
      laser3P0(laser3P0),
      xition3P1(Al_3P1_f0, 1e-6, get_fZ(), get_eta(), laser3P1),
      xition3P0(Al_3P0_f0, 1e-3, laser3P0),
     old3P1pol(0)
   {}

   std::vector<double> get_fZ();
   std::vector<double> get_eta();

   void init();
   void cool();
   void pulse(double t, unsigned ttl, sim_ions* sim);
   bool is1S0();
   void transfer_pop_1S0();

   laser_beam* laser3P1;
   laser_beam* laser3P0;

   Al_1S0_3P1 xition3P1;
   Al_1S0_3P0 xition3P0;

   int old3P1pol;
};


class sim_ions
{
public:
   sim_ions(unsigned nDDSs);
   ~sim_ions();

   void RunExperiment();
   void UpdateTransitions();

   void SetDDSFrequency(unsigned iDDS, double f);
   void SetDDSPhase(unsigned iDDS, double p);

   double GetDDSFrequency(unsigned iDDS);
   double GetDDSPhase(unsigned iDDS) const;

   void pulse(double t, unsigned flags, unsigned ttl);

   void push_result(unsigned n);
   unsigned pop_result();

   double get_compensation_freq();

   friend class xition;
   friend class xitionAl3P0;

private:

   double TimeSinceStart();

   double PMTdark_probability;

   unsigned PMTresult;

   unsigned NumExperiments;
   unsigned currentDDS;

   std::vector<double> DDSfrequencies;
   std::vector<double> DDSphases;

   QTime tStart;
   QSharedMemory freq_memory_shared;

   laser_beam laser_Mg_Detect, laser_Mg_Raman, laser_Al_3P1, laser_Al_3P0;

   Mg Mg_ion;
   Al Al_ion;

public:
   bool bCounting;
   std::deque<unsigned> PMT;

   motion_t motional_state;
};

