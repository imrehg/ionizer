#ifndef EXP_AL3P1_H_
#define EXP_AL3P1_H_

#include "exp_GSC.h"

#include "transition_info_Al.h"

class exp_3P1 : public exp_GSC
{
public:
    exp_3P1(list_t* exp_list, const std::string& name="Al+ 3P1");
   virtual ~exp_3P1() {}

   virtual void init();
   virtual void prep_ions();

   void pulse3P1(int mFg2, int pol, int sb);

protected:
   virtual void preparation_pulses();
   Al3P1_pulse* getAl3P1SB(int sb, int pol); // positive for BSB, negative for RSB, 0 for carrier
   void driveAl3P1SB_composite(int sb, int pol); // positive for BSB, negative for RSB, 0 for carrier

   //! prepare and check new 1S0 ground state 
   void prepAndCheck(int mFg2);

protected:
   rp_int xfer_sb;
   int xfer_pol;

   //number of sigma-polarized optical pumping pulses per experiment
   rp_unsigned nSigmaPump;

   rp_bool invert_Mg, depump_Al; //, fancy_bsb_pulse;
   rp_bool composite3P1;
   rp_bool checkPrep;
   rp_bool carrierExp;

   int mFg_target;
};

class exp_3P1_test : public exp_3P1
{
public:
   exp_3P1_test(list_t* exp_list, const std::string& name="3P1 scan");
   virtual ~exp_3P1_test() {}

   virtual void init();
protected:
   virtual void experiment_pulses(int iExp);


   void carrier_exp();
    void rsb_exp(int sb); //make rsb experiment
   void bsb_exp(int sb); //make bsb experiment

   void make_exp_pulse();


   Al3P1_pulse exp_pulse;

   ttl_params Ramsey;
   rp_double RamseyPhase;
   rp_unsigned RamseyTTL;
};

/*
class exp_3P1_ent : public exp_3P1
{
public:
   exp_3P1_ent(list_t* exp_list, const std::string& name="3P1 ent.");
   virtual ~exp_3P1_ent() {}

protected:
   virtual void experiment_pulses(int iExp);

   raman_pulse Readout, Invert, Analysis; // Add two carrier pulse - CWC 02032009
   ttl_params Ramsey;
   rp_double  AnalysisPhase, RamseyPhase, ReadoutPhase;
   rp_bool    randomRSBphase;

        std::vector<Al3P1_pulse*> Al3P1bsb;
        std::vector<Al3P1_pulse*> Al3P1rsb;
   std::vector<Al3P1_pulse*> Al3P1_pulses;

};

//same as exp_3P1_ent, but using Mg RF pulses where possible
class exp_3P1_entRF : public exp_3P1
{
public:
   exp_3P1_entRF(list_t* exp_list, const std::string& name="3P1 ent. RF");
   virtual ~exp_3P1_entRF() {}

protected:
   virtual void experiment_pulses(int iExp);

   raman_pulse Readout;
   RF_pulse    RF32, Invert, Analysis; // Add two carrier pulse - CWC 02032009
   ttl_params  Ramsey;
   rp_double   RamseyPhase, ReadoutPhase;
   rp_bool     randomRSBphase;

   std::vector<Al3P1_pulse*> Al3P1_pulses;
};
*/

#endif //EXP_AL3P1_H_
