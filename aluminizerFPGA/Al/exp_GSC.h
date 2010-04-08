#ifndef EXP_GSC_H
#define EXP_GSC_H

#include "../experiments.h"
#include "transition_info_Mg.h"

#include "../config_local.h"

//experiment with ground-state cooling
class exp_GSC : public exp_detect
{
public:
   exp_GSC(list_t* exp_list, const std::string& name="");
   virtual ~exp_GSC() {}

protected:
   virtual void init();

   void ground_state_cool();
   void repump();

   virtual void run_exp(int iExp);

   virtual void preparation_pulses();
   virtual void experiment_pulses(int iExp) = 0;

   unsigned get_repump_time();

   rp_bool     optimize_cooling;
   rp_int	   exp_pulse_sb;
   rp_int      exp_pulse_pol;
   rp_int      exp_pulse_port;
   rp_double   exp_pulse_gs;
   rp_bool     enable_gs_cool;

   rp_unsigned numCoolingSB; //number of modes to cool

   //how much extra cooling at the end of the GSC sequence for the most critical mode
   rp_int extraCoolingSB;
   rp_unsigned extraCoolingNum;
   rp_bool		debug_GSC;

   ttl_params Repump;

   //store sideband pulse pointers here for faster access.  
   //bsbMg[0] == rsbMg[0] == carrier
   raman_pulse* bsbMg[N_MAX_COOLING_MODES];
   raman_pulse* rsbMg[N_MAX_COOLING_MODES];
   raman_pulse* pCoCarrier;
};

class exp_raman : public exp_GSC
{
public:
   exp_raman(list_t* exp_list, const std::string& name="Raman");
   virtual ~exp_raman() {}

protected:
   virtual void init();
   virtual void experiment_pulses(int iExp);

   raman_pulse exp_pulse;

   rp_bool fancy_pulse;
   ttl_params Ramsey;
   rp_double RamseyPhase;
   rp_unsigned RamseyTTL;
   ttl_params Wait;
};


class exp_raman_RF : public exp_raman
{
public:
   exp_raman_RF(list_t* exp_list, const std::string& name);
   virtual ~exp_raman_RF() {}

protected:
   virtual void experiment_pulses(int iExp);

   RF_pulse RF0;
};


class exp_rf : public exp_detect
{
public:
   exp_rf(list_t* exp_list, const std::string& name="RF 3-2");
   virtual ~exp_rf() {}

   void AddParams();

protected:
   virtual void run_exp(int iExp);
   virtual void init();

   std::vector<RF_pulse*> RF_pulses;

   rp_bool    ShutterEnabled;
   rp_unsigned motorID;
   rp_double motorAngle;

   ttl_params  Repump;
   raman_pulse CarrierCo;
   raman_pulse Raman90;
   ttl_params  ShutterClose;
   ttl_params  ShutterOpen;
   ttl_params  Ramsey;
};

class exp_BS : public exp_GSC
{
public:
   exp_BS(list_t* exp_list, const std::string& name="BS");
   virtual ~exp_BS() {}

protected:
   virtual void run_exp(int iExp);
   virtual void experiment_pulses(int iExp);

   raman_pulse Raman;

};

class exp_HiFi_Detect : public exp_GSC
{
public:
   exp_HiFi_Detect(list_t* exp_list, const std::string& name="HiFi Detect");
   virtual ~exp_HiFi_Detect() {}

protected:
   virtual void run_exp(int iExp);
   virtual void experiment_pulses(int iExp);

   raman_pulse Raman;

};
#endif //EXP_GSC_H
