#ifdef CONFIG_AL

#include <vector>
#include <algorithm>

#include "../common.h"
#include "../ttl_pulse.h"
#include "../dds_pulse.h"
#include "dds_pulse_info.h"
#include "../remote_params.h"
#include "../exp_results.h"
#include "../host_interface.h"
#include "../shared/src/Numerics.h"
#include "../motors.h"
#include "../exp_recover.h"
#include "exp_Al3P1.h"

using namespace std;


exp_3P1::exp_3P1(list_t* exp_list, const std::string& name) :
   exp_GSC(exp_list, name),
   xfer_sb("Xfer sb",			&params, "value=2"),
 //  xfer_pol("Xfer pol",			&params, "value=1"),
   nSigmaPump("Pumping pulses", &params, "value=0"),
   invert_Mg("Invert Mg", &params, "value=0"),
   depump_Al("Depump Al", &params, "value=0"), //turn the depump pulse on and off - CWC 01212009
   composite3P1 ("Composite 3P1", &params, "value=0"),
   checkPrep("Check prep.", &params, "value=1")
{
   checkPrep.setExplanation("Check state prep. via clock histrograms");
}

void exp_3P1::driveAl3P1SB_composite(int sb, int pol) // positive for BSB, negative for RSB, 0 for carrier
{
   getAl3P1SB(sb, pol)->pulse_X90_Y180_X90();
}

Al3P1_pulse* exp_3P1::getAl3P1SB(int sb, int pol) // positive for BSB, negative for RSB, 0 for carrier
{
   int mFg2 = pol  > 0 ? 5 : -5;
    return dynamic_cast<Al3P1_pulse*>(gpAl3P1->getPulse(mFg2, pol, sb));
}

void exp_3P1::init()
{
   exp_GSC::init();

   //set waveplate to correct angle for this polarization
   double a = exp_pulse_gs > 0 ? gpAl3P1->waveplate_sp : gpAl3P1->waveplate_sm;
   motors.at(0).setAngle(static_cast<unsigned>(floor(60.*a)));

   mFg_target = (int)(2*exp_pulse_gs);
   xfer_pol = exp_pulse_pol > 0 ? 1 : -1;
}

void exp_3P1::prep_ions()
{
   if(checkPrep)
      prepAndCheck(mFg_target);
}

void exp_3P1::pulse3P1(int mFg2, int pol, int sb)
{
   gpAl3P1->getPulse(mFg2, pol, sb)->pulse();
}

void exp_3P1::preparation_pulses()
{
   //turn off pre-cooling
   Precool.ddsOff();

   //Doppler cool
   DopplerCool.pulse();

   host->sendDebugMsg("*** Begin Al 3P1 optical pumping\n");

   //3P1 optical pumping
   int prep_pol = exp_pulse_gs > 0 ? 1 : -1;
   int mFg0 = prep_pol*(5-2*nSigmaPump);
   int mFg1 = prep_pol*5;

   int mFg=mFg0;

   while (mFg != mFg1)
   {
      if(abs(mFg1-mFg) != 4) //don't drive to 1/2-3/2 if we are pumping into 5/2
         pulse3P1(mFg, prep_pol, 0);

      mFg += 2*prep_pol;
   }

   host->sendDebugMsg("***   End Al 3P1 optical pumping\n");

   //ground-state cool
   ground_state_cool();

   repump();
}



exp_3P1_test::exp_3P1_test(list_t* exp_list,
            const std::string& name) :
   exp_3P1(exp_list, name),
   exp_pulse("experiment pulse", &params, TTL_3P1_SIGMA, "t=1 fOn=1 fOff=0 nohide"),
   Ramsey		(0, "Ramsey",			&params, "t=0"),
   RamseyPhase  ("Ramsey phase [deg.]", &params, "value=0"),
   RamseyTTL	("Ramsey TTL",			&params, "value=0")
{
}

void exp_3P1_test::make_exp_pulse()
{
   exp_pulse.pulse();
}

void exp_3P1_test::init()
{
   exp_3P1::init();

   exp_pulse.set_port(exp_pulse_port);
}

void exp_3P1_test::rsb_exp(int sb)
{
   sb = abs(sb);

   //if this is an rsb experiment, first insert a quantum w/ Mg+ bsb
   gpMg->getSB(sb)->pulse();

   make_exp_pulse(); //Al+ rsb

   //read-out w/ 2nd bsb pulse
   gpMg->getSB(sb)->pulse();
}

void exp_3P1_test::bsb_exp(int sb)
{
   sb = abs(sb);

   make_exp_pulse(); //Al+ bsb

   //read-out w/ rsb pulse
   gpMg->getSB(-1*sb)->pulse();
}


void exp_3P1_test::carrier_exp()
{
   make_exp_pulse(); //Al+ carrier

   //transfer sequence
   if(xfer_sb < 0)
      gpMg->getSB(-1*xfer_sb)->pulse();

   getAl3P1SB(xfer_sb, xfer_pol)->pulse();
   gpMg->getSB(-1*xfer_sb)->pulse();
}

void exp_3P1_test::experiment_pulses(int)
{
   if(depump_Al)
      pulse3P1(exp_pulse_pol*5, -1*exp_pulse_pol, 0);

   if(exp_pulse_sb == 0 || abs(exp_pulse_sb) > 2)
      carrier_exp();
   else
   {
      if(exp_pulse_sb < 0)
         rsb_exp(abs(exp_pulse_sb));

      if(exp_pulse_sb > 0)
         bsb_exp(exp_pulse_sb);
   }
}

/*
exp_3P1_ent::exp_3P1_ent(list_t* exp_list,
            const std::string& name) :
   exp_3P1(exp_list, name),
        Readout		("Readout",			&params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0"),
        Invert		("Invert",			&params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0"),
        Analysis	("Analysis",			&params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0"),
   Ramsey		(0, "Ramsey",					&params, "t=0"),
   AnalysisPhase("Analysis phase [deg.]",		&params, "value=0"),
   RamseyPhase ("Ramsey phase [deg.]",		&params, "value=0"),
   ReadoutPhase ("Readout phase [deg.]",		&params, "value=0"),
   randomRSBphase ("Random RSB phase",			&params, "value=0")
{
        Al3P1_pulses.push_back(new Al3P1_pulse("3P1 v", &params, TTL_3P1_V, "t=1 fOn=240 fOff=0"));
        Al3P1_pulses.push_back(new Al3P1_pulse("3P1 pi", &params, TTL_3P1_PI, "t=1 fOn=240 fOff=0"));
        Al3P1_pulses.push_back(new Al3P1_pulse("3P1 sigma", &params, TTL_3P1_SIGMA, "t=1 fOn=240 fOff=0"));
}

void exp_3P1_ent::experiment_pulses(int)
{
   //run through 3P1 pulses
   for_each(Al3P1_pulses.begin(), Al3P1_pulses.end(), pulse_dds);

   if(invert_Mg)
      Invert.pulse(); //So we can start with the up-up state -CWC 02032009

   //insert a motional quantum w/ 3P1 bsb
   if(composite3P1)
      driveAl3P1SB_composite(abs(sideband), polarization);
   else
      getAl3P1SB(abs(sideband), polarization)->pulse();

   //transfer excitation to Mg+ with the readout pulse (rsb)
   if(randomRSBphase)  //make the randomRSBphase functional - CWC 02032009
      Readout.randomizePhase();
   else
      Readout.SetPhase(ReadoutPhase); //set the phase for the readout pulse -CWC 02032009

   Readout.pulse();

   Analysis.SetPhase(AnalysisPhase);
   Analysis.pulse(); //so we can measure the population after the 1st Ramsey pulse -CWC 02032009

   if(Ramsey.bEnabled)
   {
      //Mg+ carrier Ramsey exp
      pMg->getSB(0)->ramsey_pulse(&Ramsey, RamseyPhase);
   }
}


exp_3P1_entRF::exp_3P1_entRF(list_t* exp_list, const std::string& name) :
   exp_3P1(exp_list, name),
   Readout		("Readout",			&params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0 nohide"),
   RF32		("Mg RF (3) - (2)", &params, "t=4 fOn=1789 fOff=0 nohide"),
   Invert		("Invert",			&params, "t=4 fOn=1789 fOff=0 nohide"),
   Analysis	("Analysis",		&params, "t=4 fOn=1789 fOff=0 nohide"),
   Ramsey		(0, "Ramsey",					&params, "t=0"),
   RamseyPhase ("Ramsey phase [deg.]",		&params, "value=0"),
   ReadoutPhase ("Readout phase [deg.]",		&params, "value=0"),
   randomRSBphase ("Random RSB phase",			&params, "value=0")
{
   Al3P1_pulses.push_back(new Al3P1_pulse("3P1 v", &params, TTL_3P1_V, "t=1 fOn=240 fOff=0 nohide"));
   Al3P1_pulses.push_back(new Al3P1_pulse("3P1 pi", &params, TTL_3P1_PI, "t=1 fOn=240 fOff=0 nohide"));
   Al3P1_pulses.push_back(new Al3P1_pulse("3P1 sigma", &params, TTL_3P1_SIGMA, "t=1 fOn=240 fOff=0 nohide"));
}

void exp_3P1_entRF::experiment_pulses(int)
{
   //run through 3P1 pulses
   for_each(Al3P1_pulses.begin(), Al3P1_pulses.end(), pulse_dds);

   if(invert_Mg)
      Invert.pulse(); //So we can start with the up-up state -CWC 02032009

   //insert a motional quantum w/ 3P1 bsb
   if(composite3P1)
      driveAl3P1SB_composite(abs(sideband), polarization);
   else
      getAl3P1SB(abs(sideband), polarization)->pulse();

   //transfer excitation to Mg+ with the readout pulse (rsb)
   if(randomRSBphase)  //make the randomRSBphase functional - CWC 02032009
      Readout.randomizePhase();
   else
      Readout.SetPhase(ReadoutPhase); //set the phase for the readout pulse -CWC 02032009

   Readout.pulse();

   Analysis.pulse(); //so we can measure the population after the 1st Ramsey pulse -CWC 02032009

   if(Ramsey.bEnabled)
   {
      //Mg+ carrier Ramsey exp
      RF32.ramsey_pulse(&Ramsey, RamseyPhase);
   }
}
*/

#endif //CONFIG_AL
