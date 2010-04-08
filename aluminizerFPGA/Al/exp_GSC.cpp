#ifdef CONFIG_AL

#include "../common.h"
#include "../motors.h"
#include "exp_GSC.h"

exp_GSC::exp_GSC(list_t* exp_list, const std::string& name) :
   exp_detect(exp_list, name),
   optimize_cooling("Optimize cooling",		&params, "value=0"),
   exp_pulse_sb	("Sideband",			&params, "value=0"),
   exp_pulse_pol("Polarization",		&params, "value=1 min=-1 max=1"),
   exp_pulse_port("Port",				&params, "value=0 min=0 max=5"),
   exp_pulse_gs("Ground state",			&params, "value=0 min=-10 max=10"),
   enable_gs_cool("Sideband cooling", &params, "value=1"),
   numCoolingSB    ("Cooled modes [#]",		&params, "value=2"),
   extraCoolingSB  ("Extra cooling [SB]",	&params, "value=2"),
   extraCoolingNum  ("Extra cooling [#]",	&params, "value=2"),
   debug_GSC	   ("Debug GSC",				&params, "value=0"),
   Repump	   (TTL_REPUMP, "Mg Repump",			&params, "t=100")
{
   Detect.setFlag(RP_FLAG_CAN_HIDE);
   DopplerCool.setFlag(RP_FLAG_CAN_HIDE);
   Precool.setFlag(RP_FLAG_CAN_HIDE);
   Repump.setFlag(RP_FLAG_CAN_HIDE);
}

unsigned exp_GSC::get_repump_time()
{
   return gpMg->num_repump*(Repump.t + 20 + gpMg->getCoCarrier()->t) + 50 + Repump.t;
}

void exp_GSC::init()
{
    if(gpMg == 0)
        throw runtime_error("gpMg = 0! Unknown ion configuration.");

   //make sure pulse pointers for ground-state cooling are available
   //using stored pointers vs. getSB for every pulse reduces required padding significantly
   for(int n=0; n<(gpMg->get_sbMax()+1); n++)
   {
      rsbMg[n] = gpMg->getSB(-1*n);
      bsbMg[n] = gpMg->getSB(n);
   }

   pCoCarrier = gpMg->getCoCarrier();
}

void exp_GSC::repump()
{
   pCoCarrier->set_freq();
   for(unsigned iRepump=0; iRepump<gpMg->num_repump; iRepump++)
   {
      Repump.pulse();
      TTL_pulse(20, 0);	//200 ns padding
      pCoCarrier->pulse_ttl_only();
   }

   Repump.pulse();
}


void exp_GSC::ground_state_cool()
{
   if(! enable_gs_cool)
      return;

   bool bOldDebugPulses = bDebugPulses;

   if(bDebugPulses && !debug_GSC)
      host->sendDebugMsg("[GROUND STATE COOLING]\n");

   bDebugPulses = bDebugPulses && debug_GSC;

   int nMax = std::min<int>(gpMg->cooling_start, N_STRETCH_FACTORS) - 1;
   unsigned nModes = std::min<unsigned>(numCoolingSB, N_MAX_COOLING_MODES);

//	bool bOpt = optimize_cooling;

   host->sendDebugMsg("*** Begin ground-state-cool \n");

   //Raman sideband cooling
   for(int n=nMax; n>=0; n--)
   {
      //enhanced cooling near n=0
      unsigned nEnhance = (n < 2) ? 2 : 1;

      for(unsigned j=0; j<(nEnhance*gpMg->cooling_reps); j++)
      {
         for(unsigned i=1; i<=nModes; i++)
         {
            //red sideband
            rsbMg[i]->stretched_pulse(static_cast<unsigned>(gpMg->stretch_factor[i-1][n]));
            repump();
         }
      }
   }

   for(unsigned i=0; i<extraCoolingNum; i++)
   {
      gpMg->getSB(-1*abs(extraCoolingSB))->pulse();
      repump();
   }

   host->sendDebugMsg("***   End ground-state-cool \n");

   bDebugPulses = bOldDebugPulses;
}

void exp_GSC::preparation_pulses()
{
   Precool.ddsOff();
   DopplerCool.pulse();

   ground_state_cool();

   repump();
}

void exp_GSC::run_exp(int iExp)
{
   host->sendDebugMsg("*** Begin preparation \n");

   preparation_pulses();

   host->sendDebugMsg("***   End preparation \n**********************\n");
   host->sendDebugMsg("*** Begin experiment \n");

   experiment_pulses(iExp);

   host->sendDebugMsg("***   End experiment \n**********************\n");
   host->sendDebugMsg("*** Begin detection \n");

   Detect.detection_pulse();

   host->sendDebugMsg("***   End detection \n**********************\n");

   Precool.pulseStayOn();
}

exp_raman::exp_raman(list_t* exp_list,
                 const std::string& name) :
   exp_GSC(exp_list, name),
   exp_pulse	("experiment pulse", &params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0"),
   fancy_pulse ("X90 Y180 X90",		&params, "value=0"),
   Ramsey		(0, "Ramsey",			&params, "t=0"),
   RamseyPhase ("Ramsey phase [deg.]",	&params, "value=0"),
   RamseyTTL	("Ramsey TTL",			&params, "value=0"),
   Wait			(0,	"Wait",	&params, "t=1")
{
}

void exp_raman::init()
{
   exp_GSC::init();
   exp_pulse.setPolarization(exp_pulse_pol);
   exp_pulse.setX90Y180X90(fancy_pulse);

   Ramsey.ttl = RamseyTTL;
}

void exp_raman::experiment_pulses(int)
{
   Wait.pulse();
	
   //for GS-cooled RSB or co-carrier experiments, first make a carrier pi-pulse
   if((enable_gs_cool && exp_pulse_sb < 0) || exp_pulse_pol == 0)
      gpMg->getSB(0)->pulse();

   if(!Ramsey.bEnabled)
      exp_pulse.pulse();
   else
   {
      exp_pulse.ramsey_pulse(&Ramsey, (double)RamseyPhase);
   }
}

exp_raman_RF::exp_raman_RF(list_t* exp_list,
                     const std::string& name) :
   exp_raman(exp_list, name),
   RF0			("RF (-3) - (-2)",	&params, "t=4 fOn=1789 fOff=0")
{
}

void exp_raman_RF::experiment_pulses(int iExp)
{
   exp_raman::experiment_pulses(iExp);

   // RF0.pulse(); slow?
   
   DDS_pulse(RF0.dds, RF0.ftwOn, RF0.ftwOff, RF0.t, RF0.ttl);
}

exp_rf::exp_rf(list_t* exp_list,
            const std::string& name) :
   exp_detect(exp_list, name),
   ShutterEnabled ("Use shutter",				&params, "value=0"),
   motorID ("Motor ID",						&params, "value=0"),
   motorAngle ("Motor Angle",						&params, "value=0"),
   Repump		(TTL_REPUMP, "Repump",			&params, "t=100"),
   CarrierCo	(			 "Co-carrier",		&params, TTL_RAMAN_CO, "t=1 fOn=297 fOff=0"),
   Raman90		(			 "RSB1",			&params, TTL_RAMAN_90, "t=1 fOn=297 fOff=0"),
   ShutterClose(TTL_MG_SHUTTER_CLOSE, "Shutter close",			&params, "t=5000"),
   ShutterOpen (TTL_MG_SHUTTER_OPEN,  "Shutter open",			&params, "t=5000"),
   Ramsey		(0, "Ramsey",					&params, "t=0")
{
   AddParams();
}


void exp_rf::AddParams()
{
   unsigned nPulses = 0;

   //figure out number of RF pulses from the number of '-' characters in the title
   for(size_t i=0; i<name.length(); i++)
      if(name[i] == '-')
         nPulses++;

   double mFg = -3;
   double mFe = -2;

   for(unsigned i=0; i<nPulses; i++)
   {
      if(i%2 == 0)
      {
         //every even pulse is up, sigma+
         mFe = mFg+1;
      }
      else
      {
         //every odd pulse down, pi
         mFg = mFe;
      }

      char name[100];
      sprintf(name, "Mg RF (%1.0lf) - (%1.0lf)", mFg, mFe);
        RF_pulses.push_back(new RF_pulse(name, &params, "t=4 fOn=1789 fOff=1780"));
   }
}

void exp_rf::init()
{
   exp_detect::init();

   //set shutter
   for(size_t i=0; i<RF_pulses.size(); i++)
   {
      RF_pulses[i]->padding = 0;
      RF_pulses[i]->ttl = TTL_HF_RF;
      RF_pulses[i]->ttl |= ShutterEnabled ? TTL_MG_SHUTTER_CLOSE : 0;
   }

   Ramsey.ttl = ShutterEnabled ? TTL_MG_SHUTTER_CLOSE : 0;
   RF_pulses.back()->setRamsey(&Ramsey);

   motors.at(motorID).setAngle(static_cast<unsigned>(motorAngle*60.0));
}

void exp_rf::run_exp(int)
{

   Precool.ddsOff();
   DopplerCool.pulse();

   if(ShutterEnabled)
      ShutterClose.pulse();

   for_each(RF_pulses.begin(), RF_pulses.end(), pulse_dds);

   if(ShutterEnabled)
      ShutterOpen.pulse();


   CarrierCo.pulse();
   Raman90.pulse();
   Repump.pulse();
   Detect.detection_pulse();
   Precool.pulseStayOn();
}

exp_BS::exp_BS(list_t* exp_list,
                 const std::string& name) :
   exp_GSC(exp_list, name),
   Raman		("Raman", &params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0")
{
}


void exp_BS::run_exp(int iExp)
{
   if(!iExp)
   {
      Precool.ddsOff();
      DopplerCool.pulse();

      if(numCoolingSB > 0)
         ground_state_cool();

      Raman.pulse();
   }

   Detect.detection_pulse();

   //turn on pre-cooling after last experiment
   if(iExp+1 == (int)num_exp)
   {
      Precool.pulseStayOn();
   }
}

void exp_BS::experiment_pulses(int)
{
}

exp_HiFi_Detect::exp_HiFi_Detect(list_t* exp_list,
                 const std::string& name) :
   exp_GSC(exp_list, name),
   Raman		("Raman", &params, TTL_RAMAN_90, "t=1 fOn=1789 fOff=0")
{
}

void exp_HiFi_Detect::run_exp(int iExp)
{
   if(!iExp)
   {
      Precool.ddsOff();
      DopplerCool.pulse();

      if(numCoolingSB > 0)
         ground_state_cool();

      Raman.pulse();
   }

   Detect.detection_pulse();

}

void exp_HiFi_Detect::experiment_pulses(int)
{
}

#endif //CONFIG_AL
