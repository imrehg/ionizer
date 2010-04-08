#ifndef TRANSITION_INFO_MG_H
#define TRANSITION_INFO_MG_H

#include "../transition_info.h"
#include "../config_local.h"

class iMg : public transition_info
{
public:
   iMg(list_t* exp_list, const std::string& name);
   virtual  ~iMg() {}

   virtual dds_params* new_pulse(const pulse_spec& p, params_t* pparams);

   void calcStretchFactors();

   raman_pulse* getSB(int sb); // positive for BSB, negative for RSB, 0 for carrier
   raman_pulse* getCoCarrier();

   virtual void update_param_binary(unsigned param_id, unsigned length,const  char* bin);
   virtual void updateParam(const char* name, const char* value);

   virtual void setIonXtal(const char* name);

   double eta(unsigned iMode);

   // stretch_factor[i][n] =
   // (Rabi rate for 0->1) / (Rabi rate for n->n+1) for motional mode i+1
   double stretch_factor[N_MAX_COOLING_MODES][N_STRETCH_FACTORS];

   ttl_params Padding;

   rp_unsigned num_repump;
   rp_unsigned cooling_start;
   rp_unsigned cooling_reps;

   rp_matrix motion;
   vector<double> old_eta;

  // std::vector<raman_pulse*> Mg_rsb_pulses, Mg_carrier_pulses, Mg_bsb_pulses;
};

#endif // TRANSITION_INFO_MG_H
