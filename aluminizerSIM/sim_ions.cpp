#include "Numerics.h"
#include <dds_pulse.h>
#include <ttl_pulse.h>

#include <iostream>
#include <list>

#include <QThread>

#include "pulse_controller.h"
#include "sim_ions.h"
#include "motors.h"
#include "string_func.h"

//using namespace numerics;

using namespace std;
//using namespace physics;

bool debug_pulses = false;

double laser_AOM::getFrequencyShift(sim_ions* sim)
{
   return order * sim->GetDDSFrequency(iDDS);
}

double laser_beam::getFrequency(sim_ions* sim)
{
   double f = f0;

   for(size_t i=0; i<AOMs.size(); i++)
      f += AOMs[i].getFrequencyShift(sim);

   return f;
}

unsigned Mg_cycling::pulse(double t, sim_ions* sim)
{
   unsigned nPMT = 0;

   if(t > 0)
   {
      double fLaser = laser->getFrequency(sim);

      if(fLaser > 0)
      {
         double detuning = fLaser - omega0/(M_PI*2);

         if(debug_pulses)
            printf("[DETECT] f = %9.0f Hz   t = %6.3f us\r\n", detuning, t*1e6);


         if(fabs(detuning) < 75e6)
         {
            double rate = scattering_rate / (1 + 4*pow(detuning/gamma,2));
            nPMT = numerics::RandomVariablePoisson(rate*t);
         }
      }
   }

   return nPMT;
}

sim_transition::sim_transition(const std::string& name,
                        physics::HFS* g,
                        physics::HFS* e,
                        double f0, double tPi,
                        std::vector<double> fZ,
                        std::vector<double> eta,
                        laser_beam* laser,
                  double lifetime) :
   X(g, e, "Mg", 2),
   nZSg(g->NumStates()),
   nZSe(e->NumStates()),
   nIS(nZSg+nZSe),
   nMS(fZ.size()+1),
   nStates(nMS*nIS),
   laser(laser),
   lifetime(lifetime),
   state_pop(nStates, 0),
   state_E(nStates, 0),
   projected_state(0),
   name(name),
   B(1),
   tPi(tPi)
{
   X.SetF0(f0);

   X.SetModeFrequency(0, 0);
   X.SetModeLambDicke(1, 0);

   for(unsigned i=0; i<fZ.size(); i++)
   {
      X.SetModeFrequency(fZ[i], i+1);
      X.SetModeLambDicke(eta[i], i+1);
   }

  //initialize in 0-state
  if(nStates)
     state_pop[0] = 1;

  calc_energies();
  calc_coupling();
}

//convert state spec to linear index
unsigned sim_transition::si(unsigned i, double mF, motion_t m) const
{
   if(i)
      return (nZSg + mF - X.e->mFmin()) * nMS + m;
   else
      return (mF - X.g->mFmin()) * nMS + m;
}

//get electronic state
unsigned sim_transition::get_IS(unsigned k) const
{
   return (k >= nZSg*nMS) ? 1 : 0;
}

//return mF-value associated with state k
double sim_transition::get_mF(unsigned k) const
{
   if(get_IS(k))
   {
      k -= nZSg * nMS;
      return X.e->mFmin() + (k/nMS);
   }
   else
      return X.g->mFmin() + (k/nMS);
}


//get motional state
unsigned sim_transition::get_sb(unsigned k) const
{
   return k % nMS;
}

//get state name
std::string sim_transition::get_state_name(unsigned k) const
{
   unsigned is = get_IS(k);
   double mF = get_mF(k);
   unsigned sb = get_sb(k);

   char buff[128];

   if(is == 0)
      snprintf(buff, 127, "%24s %4s", X.g->GetName(mF).c_str(), modeName(sb));
   else
      snprintf(buff, 127, "%24s %4s", X.e->GetName(mF).c_str(), modeName(sb));

   return string(buff);
}

//return energy of internal state k
double sim_transition::getEnergy(unsigned k)
{
   if(get_IS(k) == 0)
      return X.g_sublevel(get_mF(k), B, get_sb(k))*2*M_PI;
   else
      return X.e_sublevel(get_mF(k), B, get_sb(k))*2*M_PI;
}

//calculate energies of states
void sim_transition::calc_energies()
{
     for(unsigned k=0; k<nStates; k++)
     {
        if(get_state_name(k).find("3P0") != string::npos)
           cout << get_state_name(k) << endl;

         state_E.at(k) = getEnergy(k);
 //        printf("%s E = %12.0f Hz \r\n", get_state_name(k).c_str(), state_E.at(k)/(2*M_PI));
     }
}

//calculate coupling matrix elements
void sim_transition::calc_coupling()
{
  cm.clear();

  double strength = M_PI/tPi;

  for(unsigned ig=0; ig<driven_states_g.size(); ig++)
   {
      unsigned si1 = driven_states_g[ig];

      unsigned sb1 = get_sb(si1);
      double mFg = get_mF(si1);
      double mFe = mFg + laser->getPol();

     //double Eg = state_E.at(si1);

      if(mFe >= X.e->mFmin() && mFe <= X.e->mFmax())
      {
        for(unsigned sb2=0; sb2<nMS; sb2++)
        {
           //do not consider motional transitions between two excited motional states
           if(sb1 == 0 || sb2 == 0)
           {
                 unsigned si2 = si(1, mFe, sb2);

             //todo: include Clebsch-Gordan coefficients here
                 double s = strength * X.getModeLambDicke(sb1) * X.getModeLambDicke(sb2);
                 cm[cm_key(si1, si2)] = s;

             //double Ee = state_E.at(si2);

             //    printf("matrix element %s -> %s  tPi = %f  dE = %12.0f\r\n",
            //        get_state_name(si1).c_str(),
            //        get_state_name(si2).c_str(), 1e6*M_PI/s, (Ee - Eg)/(2*M_PI));
             }
          }
       }
    }
}

unsigned sim_transition::cm_key(unsigned i1, unsigned i2) const
{
   if(i1 <= i2)
      return i1*nStates + i2;
   else
      return i2*nStates + i1;
}

void sim_transition::pulse(double t, sim_ions* sim)
{
   double omega = 2*M_PI*laser->getFrequency(sim);

   if(debug_pulses)
   {
      printf("[%s::pulse] f = %12.0f Hz   t = %6.3f us\r\n", name.c_str(), omega/(2*M_PI), t*1e6);
     print_current_state();
   }

   //this is crude, but will do the job for now
   for(unsigned iig=0; iig<driven_states_g.size(); iig++)
   {
      unsigned ig = driven_states_g[iig];
      unsigned sbg = get_sb(ig);
      double mFg = get_mF(ig);
      double mFe = mFg + laser->getPol();

      if(mFe >= X.e->mFmin() && mFe <= X.e->mFmax())
      {
         unsigned sbMax = sbg == 0 ? nMS : 1;

         for(unsigned sbe=0; sbe<sbMax; sbe++)
         {
            unsigned ie = si(1, mFe, sbe);

             double omega0 = state_E.at(ie) - state_E.at(ig);
             double omegaR = 0;

             unsigned cmk = cm_key(ie, ig);

             if(cm.find(cmk) != cm.end())
             {
               omegaR = cm[cmk];

               double p = numerics::RabiProbability(omega, t, omegaR, omega0);
               double dp = p*(state_pop[ie] - state_pop[ig]);
               state_pop[ig] += dp;
               state_pop[ie] -= dp;
             }
         }
      }
   }

   if(debug_pulses)
   {
     printf("new state after pulse:\r\n");
     print_current_state();
   }

}

unsigned project_qs(std::vector<double>& sp)
{
   //normalize probabilities to add up to 1
   double pSum = 0;

   for(unsigned i=0; i<sp.size(); i++)
   {
      sp[i] = std::max<double>(sp[i], 0.0);
      pSum += sp[i];
   }

   for(unsigned i=0; i<sp.size(); i++)
   {
      if(pSum == 0)
         sp[i] = 1.0 / sp.size();
      else
         sp[i] /= pSum;
   }

   //generate random number between 0 and 1
   double r = rand()/(double)RAND_MAX;

   pSum = 0;
   unsigned ps = 0;

   for(unsigned i=0; i<sp.size(); i++)
   {
      pSum += sp[i];

      if(pSum >= r)
      {
         ps = i;
         break;
      }
   }

   return ps;
}

void sim_transition::project()
{
   if(debug_pulses)
   {
     printf("[%s::project]\r\n", name.c_str());
     print_current_state();
   }

   projected_state = project_qs(state_pop);

   if(debug_pulses)
      printf("[%s::project] state = %d\r\n", name.c_str(), projected_state);

    set_state(projected_state);
}

void sim_transition::set_state(unsigned state_index)
{
   if(debug_pulses)
      printf("[%s::set_state] %d\r\n", name.c_str(), state_index);

   for(unsigned i=0; i<state_pop.size(); i++)
   {
      state_pop[i] = 0;
   }

   state_pop.at(state_index) = 1;
}

motion_t sim_transition::getMotionalState()
{
   std::vector<double> motional_pop(nMS, 0);

   for(unsigned i=0; i<nStates; i++)
   {
      motion_t m = get_sb(i);
       motional_pop[m] += state_pop[i];
   }

   motion_t mp = project_qs(motional_pop);

   if(debug_pulses)
      printf("[%s::getMotionalState] m = %d\r\n", name.c_str(), mp);

   setMotionalState(mp);

   return mp;
}


//collapse motional state w/o affecting internal state
void sim_transition::setMotionalState(motion_t m)
{
   if(debug_pulses)
      printf("[%s::setMotionalState] m = %d\r\n", name.c_str(), m);

   for(unsigned k=0; k<nStates; k++)
   {
      double mF = get_mF(k);
     unsigned is = get_IS(k);

      for(motion_t m2=0; m2<nMS; m2++)
      {
         if(m2 != m)
         {
           //transfer population to motional state m
            state_pop[si(is,mF,m)] += state_pop[si(is,mF,m2)];
            state_pop[si(is,mF,m2)] = 0;
         }
      }
   }
}

void sim_transition::print_current_state()
{
   list<double> state_pop_sorted;
   list<unsigned> states_sorted;

   size_t maxL = 3;

   state_pop_sorted.insert(state_pop_sorted.begin(), state_pop[0]);
   states_sorted.insert(states_sorted.begin(), 0);

   for(unsigned s=1; s<nStates; s++)
   {
      list<double>::iterator it1 = state_pop_sorted.begin();
      list<unsigned>::iterator it2 = states_sorted.begin();
      for(; it1!=state_pop_sorted.end(); it1++, it2++)
      {
         if( *it1 < state_pop[s])
         {
            state_pop_sorted.insert(it1, state_pop[s]);
            states_sorted.insert(it2, s);

            if(state_pop_sorted.size() > maxL)
            {
               state_pop_sorted.pop_back();
               states_sorted.pop_back();
            }

            break;
         }
      }
   }

   list<double>::iterator it1 = state_pop_sorted.begin();
   list<unsigned>::iterator it2 = states_sorted.begin();
   for(; it1!=state_pop_sorted.end(); it1++, it2++)
   {
      double p = *it1;
      unsigned s = *it2;

      if(p > 0)
         printf("%s (p=%4.3f)\r\n", get_state_name(s).c_str(), p);
   }

}


//simulate spont. decay
//doesn't include branching ratios yet
void sim_transition::decay(double t)
{
   double prob_decay = t/lifetime;
   bool bDecayed = prob_decay >= (rand() / (double)RAND_MAX);

   if(bDecayed)
   {
      if(debug_pulses)
      {
         printf("spontaneous decay.  old state:\r\n");
          print_current_state();
      }

        //transfer excited state to ground state
        //todo: include branching ratios here
        for(double mF = X.e->mFmin(); mF <= X.e->mFmax(); mF++)
         for(unsigned ms=0; ms<nMS; ms++)
         {
            double mFg = std::min<double>(mF, X.g->mFmax());
            mFg = std::max<double>(mFg, X.g->mFmin());

            state_pop[si(0,mFg,ms)] += state_pop[si(1,mF,ms)];
            state_pop[si(1,mF,ms)] = 0;
         }

      unsigned ps = project_qs(state_pop);
      set_state(ps);
   }
}


void Mg_Raman::repump()
{
   set_state(si(0,-3,0));
}

Al_1S0_3P1::Al_1S0_3P1(double f0, double tPi, std::vector<double> fZ, std::vector<double> eta, laser_beam* laser) :
  sim_transition("Al 3P1",
     (new physics::HFS_Al_II_SingletS0()),
     (new physics::HFS_Al_II_TripletP1(3.5)),
     f0, tPi, fZ, eta, laser, 3e-4)
{
   for(double mF = X.g->mFmin(); mF <= X.g->mFmax(); mF++)
      for(unsigned ms=0; ms<nMS; ms++)
         driven_states_g.push_back(si(0, mF, ms));

   calc_coupling();
}

void Al_1S0_3P1::repump()
{
   set_state(si(0,-2.5,0));
}


void Al_1S0_3P0::repump()
{
   set_state(si(0,-2.5,0));
}

void Mg::cool()
{
   raman_transition.setMotionalState(0);
}

std::vector<double> Mg::get_fZ()
{
   std::vector<double> fZ(3);
   fZ[0] = 3.02e6;
   fZ[1] = 5.2e6;
   fZ[2] = 59e6;

   return fZ;
}

std::vector<double> Mg::get_eta()
{
   std::vector<double> eta(3);
   eta[0] = 0.13;
   eta[1] = 0.08;
   eta[2] = 0.1;

   return eta;
}

void Mg::pulse(double t, unsigned ttl, sim_ions* sim)
{
   if(t > 0)
   {
      if(sim->bCounting)
      {
       raman_transition.setMotionalState(0);
         raman_transition.project();

         if(raman_transition.projected_state == 0)
            detection_transition.scattering_rate = 4/(100e-6);
         else
            detection_transition.scattering_rate = 0.1/(100e-6);

         unsigned nPMT = detection_transition.pulse(t, sim);
         sim->push_result(nPMT);
      }

      if(ttl & TTL_RAMAN_90)
         raman_transition.pulse(t, sim);

      if(ttl & TTL_REPUMP)
         raman_transition.repump();
   }
}

std::vector<double> Al::get_fZ()
{
   return Mg::get_fZ();
}

std::vector<double> Al::get_eta()
{
   std::vector<double> eta(3);
   eta[0] = 0.08;
   eta[1] = 0.15;
   eta[2] = 0.1;

   return eta;
}

bool Al::is1S0()
{
   xition3P0.project();
   return xition3P0.get_IS(xition3P0.projected_state) == 0;
}


void Al::pulse(double t, unsigned ttl, sim_ions* sim)
{
   if(t > 0)
   {
      if(motors.size())
      {
         if(motors.at(0).getAngle() == 3060)
           laser3P1->setPol(-1);
         else
           laser3P1->setPol(1);
      }

      if(old3P1pol != laser3P1->getPol())
      {
         old3P1pol = laser3P1->getPol();
         xition3P1.calc_coupling();
      }


      if(ttl & TTL_3P1_SIGMA)
      {
         if(is1S0())
       {
            xition3P1.pulse(t, sim);
            transfer_pop_1S0();
       }
         else
            if(debug_pulses)
               printf("[Al::pulse] skipping 3P1 pulse due to 3P0 state\r\n");
      }

      if(ttl & TTL_3P0)
      {
         xition3P0.pulse(t, sim);
         SleepHelper::msleep(t*1000);
      }

     xition3P1.decay(t);
     xition3P0.decay(t);
   }
}

//transfer 1S0 Zeeman state pop from 3P1 to 3P0 structure info
//should only be called if Al+ is in the 1S0 state, so don't worry about 3P0 and 3P1
void Al::transfer_pop_1S0()
{
     for(double mF = -2.5; mF <= 2.5; mF++)
     {
         double pop = 0;
         for(unsigned ms=0; ms<xition3P1.nMS; ms++)
            pop += xition3P1.state_pop[xition3P1.si(0,mF,ms)];

         xition3P0.state_pop[xition3P0.si(0,mF,0)] = pop;
     }
}


void Al::cool()
{
   xition3P1.setMotionalState(0);
   xition3P0.setMotionalState(0);
}

void Al::init()
{
   xition3P1.repump();
   xition3P0.repump();
}


sim_ions::sim_ions(unsigned nDDSs) :
DDSfrequencies(nDDSs, 0),
DDSphases(nDDSs, 0),
tStart(QTime::currentTime ()),
freq_memory_shared("freq_mem_compensation_synth"),
laser_Mg_Detect(Mg_Detect_f0 - 4*220e6, -1),
laser_Mg_Raman(600e6, 1),
laser_Al_3P1(0.0, -1),
laser_Al_3P0(0.0, 0),
Mg_ion(&laser_Mg_Detect, &laser_Mg_Raman),
Al_ion(&laser_Al_3P1, &laser_Al_3P0),
bCounting(false),
PMT(0),
motional_state(0)
{
   if(!freq_memory_shared.attach(QSharedMemory::ReadOnly))
      freq_memory_shared.create(sizeof(double), QSharedMemory::ReadOnly);

   Al_ion.init();

   laser_Mg_Detect.addAOM(laser_AOM(DDS_DETECT, 4));
   laser_Mg_Raman.addAOM(laser_AOM(DDS_RAMAN, 4));
   laser_Al_3P1.addAOM(laser_AOM(DDS_3P1x2, -2));
   laser_Al_3P0.addAOM(laser_AOM(DDS_3P0, -1));
   laser_Al_3P0.addAOM(laser_AOM(100, 2)); //DDS 100 corresponds to special cavity compensation freq AOM (vis)

   get_compensation_freq();
}

sim_ions::~sim_ions()
{
   freq_memory_shared.detach();
}

double sim_ions::get_compensation_freq()
{
   double f = 0;

   freq_memory_shared.lock();
   memcpy(&f, freq_memory_shared.data(), sizeof(double));
   freq_memory_shared.unlock();

   if(f == 0)
      f = 85e6;

//   printf("cavity compensation freq. = %10.2f Hz\r\n", f);

   return f;
}


void sim_ions::push_result(unsigned n)
{
   if(bCounting)
      PMT.push_back(n);
}

unsigned sim_ions::pop_result()
{
   unsigned val = PMT.front();
   PMT.pop_front();

   return val;
}

void sim_ions::pulse(double t, unsigned flags, unsigned ttl)
{
   bCounting = flags & PULSE_CONTROLLER_COUNTING_PULSE_FLAG;

   if(ttl & TTL_DETECT_MON)
   {
      motional_state = 0;
     Mg_ion.cool();
     Al_ion.cool();
   }

   Mg_ion.pulse(t, ttl, this);
   Al_ion.pulse(t, ttl, this);

   if(ttl & TTL_3P1_SIGMA)
   {
      motional_state = Al_ion.xition3P1.getMotionalState();
      Mg_ion.raman_transition.setMotionalState(motional_state);
   }

   bCounting = false;
}

double  sim_ions::TimeSinceStart()
{
   return tStart.msecsTo(QTime::currentTime()) * 1e-3;
}

void sim_ions::SetDDSFrequency(unsigned iDDS, double f)
{
   DDSfrequencies.at(iDDS) = f*1e6;
}

void sim_ions::SetDDSPhase(unsigned iDDS, double p)
{
   DDSphases.at(iDDS) = p;
}

double sim_ions::GetDDSFrequency(unsigned iDDS)
{
   if(iDDS < DDSfrequencies.size())
      return DDSfrequencies[iDDS]*1e-6;
   else
   {
      if(iDDS == 100)
      {
         return get_compensation_freq();
      }
      else
         return 0;
   }
}

double sim_ions::GetDDSPhase(unsigned iDDS) const
{
   return DDSphases.at(iDDS);
}


