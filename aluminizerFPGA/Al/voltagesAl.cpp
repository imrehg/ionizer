#ifdef CONFIG_AL

#include <vector>
#include <algorithm>

#include "../common.h"
#include "../config_local.h"
#include "voltagesAl.h"
#include "../dacI.h"
#include "../exp_recover.h"

using namespace std;

#ifdef ALUMINIZER_SIM
   void usleep(int);
#else
   extern "C"
   {
   #include "sleep.h"
   }
#endif

void rp_ao::set_ao(double V)
{
   pDAC->setVoltage(iChannel, (int)(V*1e6));
   set(V);
}

double rp_ao::get_ao()
{
   return get_value();
}

voltagesAl::voltagesAl(list_t* exp_list, const std::string& name) :
 voltagesI(exp_list, name),
 NoUpdates("No updates", &params, "value=0"),
 Debug("Debug", &params, "value=0"),
 XtallizeReorder("Xtallize reorder", &params, "value=0"),
 XtallizeSettings("Xtallize settings", &params, "value=0"),
 VcerfExy(2, 2, "V[CE,RF] for E[x,y]", &params, "range=1000 hr0=Ex hr1=Ey hc0=CE hc1=RF"),
 VcerfEhv(2, 2, "V[CE,RF] for E[h,v]", &params, "range=1000 hr0=Eh hr1=Ev hc0=CE hc1=RF"),
 Eall(18, 8, "Eall", &params, "range=1000 hc0=Run hc1=Low hc2=Load hc3=MgAl hc4=AlMg hc5=X28 hc6=3MHz hc7=1Trap hr0=Ex hr1=Ey hr2=Eh hr3=Ev hr4=muEC hr5=rEC hr6=twEC hr7=cmCE hr8=cmRF hr9=trapRF hr10=V7 hr11=dwell hr12=steps hr13=hold hr14=wait hr15=ECcEZ hr16=ECEZ hr17=cool"),
 ao_lcd(NUM_OUT_VOLTAGES),
 ao(1,NUM_OUT_VOLTAGES),
 Vcerf_old(2,1),
 current_settings(0)
{
   XtallizeSettings.setFlag(RP_FLAG_READ_ONLY);
   XtallizeSettings.setFlag(RP_FLAG_NOPACK);
   XtallizeSettings.setExplanation("Reorder settings for use after xtallize");

   remote_actions.push_back("Crystallize");
   remote_actions.push_back("MgAl");
   remote_actions.push_back("AlMg");
   remote_actions.push_back("DUMP HEAVY");
   remote_actions.push_back("DUMP");

   //make all GUI changes immediate
   for(unsigned i=0; i<params.size(); i++)
      params[i]->setFlag(RP_FLAG_UPDATE_IMMEDIATE);


   ao_lcd[0] = (new rp_ao("CE LZ* [V]", &params, "value=0", iVoltages5370, 0));
   ao_lcd[1] = (new rp_ao("CE LZ [V]", &params, "value=0", iVoltages5370, 1));
   ao_lcd[2] = (new rp_ao("RF+ [V]", &params, "value=0", iVoltages5370, 2));
   ao_lcd[3] = (new rp_ao("RF- [V]", &params, "value=0", iVoltages5370, 3));
   ao_lcd[4] = (new rp_ao("EC LZ [V]", &params, "value=0", iVoltages5370, 4));

   ao_lcd[5] = (new rp_ao("ECc LZ [V]", &params, "value=0", iVoltages5535, 0));
   ao_lcd[6] = (new rp_ao("ECc LZ* [V]", &params, "value=0", iVoltages5535, 1));
   ao_lcd[7] = (new rp_ao("V6 [V]", &params, "value=0", iVoltages5370, 5));
   ao_lcd[8] = (new rp_ao("V7 [V]", &params, "value=0", iVoltages5370, 6));
   ao_lcd[9] = (new rp_ao("ECc EZ [V]", &params, "value=0", iVoltages5535, 2));
   ao_lcd[10] = (new rp_ao("CE EZ [V]", &params, "value=0", iVoltages5535, 3));
}

unsigned voltagesAl::remote_action(const char* s)
{
   if(strcmp(s, "Crystallize") == 0)
   {
      rampTo(1, 1, false);

     if(XtallizeReorder)
      {
         rampTo(XtallizeSettings, 1, false);
      }
   }

   if(strcmp(s, "MgAl") == 0)
      rampTo(3, 1, false);

   if(strcmp(s, "AlMg") == 0)
      rampTo(4, 1, false);

   if(strcmp(s, "DUMP HEAVY") == 0)
      rampTo(5, 1, false);

   if(strcmp(s, "DUMP") == 0)
      dump();

   return 0;
}

void voltagesAl::set_voltage(unsigned iChannel, double V)
{
   ao_lcd[iChannel]->set_ao(V);
}

double voltagesAl::get_voltage(unsigned iChannel)
{
   return ao_lcd[iChannel]->get_ao();
}

void voltagesAl::dump()
{
   double old_ao = get_voltage(7);
   set_voltage(7, 0);
   usleep(1000);
   set_voltage(7, old_ao);
   printf("DUMP\r\n");
   fflush(stdout);
}

void voltagesAl::updateInverseMatrices()
{
   VcerfExy.updateInverse();
   VcerfEhv.updateInverse();
}


void  voltagesAl::updateGUI()
{
   for(unsigned i=0; i<ao_lcd.size(); i++)
      ao_lcd[i]->updateGUI(page_id);
}

//! called after parameters have been updated
void voltagesAl::updateParams()
{
   info_interface::updateParams();

   if(updateVoltages(false))
   {
      updateGUI();
   }
}

void voltagesAl::voltagesForSetting(unsigned iSetting, bool bHV, my_matrix& ao_new)
{
   my_matrix Vcerf(2,1);

   my_matrix Ehv(2,1);
   Ehv.element(0,0) = Eall.value.element(2, iSetting);
   Ehv.element(1,0) = Eall.value.element(3, iSetting);

   my_matrix Exy(2,1);
   Exy.element(0,0) = Eall.value.element(4, iSetting);
   Exy.element(1,0) = Eall.value.element(5, iSetting);

   if(bHV)
      Vcerf.from_product(VcerfEhv.value, Ehv);
   else
      Vcerf.from_product(VcerfExy.value, Exy);

   double ECmean = Eall.value.element(4, iSetting);
   double ECratio = Eall.value.element(5, iSetting);
   double ECtwist = Eall.value.element(6, iSetting);
   double cmCE = Eall.value.element(7, iSetting);
   double cmRF = Eall.value.element(8, iSetting);

   for(int i=0; i<2; i++)
   {
      ao_new.element(0,i) = (i-0.5) * Vcerf.element(0,0) + cmCE; //CE
      ao_new.element(0,i+2) = (i-0.5) * Vcerf.element(1,0) + cmRF; //RF
   }

   ao_new.element(0,4) = (2*ECmean/(1+1/ECratio));

    ao_new.element(0,5) = (2*ECmean/(1+ECratio)+0.5*ECtwist);
    ao_new.element(0,6) = (2*ECmean/(1+ECratio)-0.5*ECtwist);

   ao_new.element(0,7) = Eall.value.element(9, iSetting);
   ao_new.element(0,8) = Eall.value.element(10, iSetting);
   
   ao_new.element(0,9) = Eall.value.element(15, iSetting);
   ao_new.element(0,10) = Eall.value.element(16, iSetting);
}

bool voltagesAl::updateVoltages(bool bForceUpdate, unsigned ramp_steps, unsigned dwell, unsigned cooling)
{
   if(NoUpdates)
      return false;

 //  updateInverseMatrices();

   my_matrix ao_new(1,ao.nc);

   voltagesForSetting(current_settings, true, ao_new);

   if(bForceUpdate || ao_new != ao)
   {
      for(unsigned i=0; i<ramp_steps; i++)
      {
         if(Debug) printf("%3d: ", i);

         for(unsigned j=0; j<ao_lcd.size(); j++)
         {
            double v0 = ao.element(0,j);
            double v1 = ao_new.element(0,j);
            double delta = (v1-v0)/ramp_steps;
            set_voltage(j, v0 + (i+1)*delta);

            if(Debug) printf("% 8.3f ", get_voltage(j));
         }

         if(Debug) printf("\r\n");

         if(cooling)
             eRecover->cool(dwell);
         else
             usleep(dwell);
      }

      ao = ao_new;

      return true;
   }

   return false;
}

void voltagesAl::rampDownXtallize()
{
   rampTo(1, 0, false);
}

void voltagesAl::rampUpXtallize()
{
   rampTo(0, 0, false);

   if(XtallizeReorder)
   {
      rampTo(XtallizeSettings, 1, false);
   }
}

void voltagesAl::rampTo(unsigned settings_id, unsigned come_back, bool bUpdateGUI)
{
   unsigned old_settings = current_settings;

   current_settings = settings_id;
   printf("[voltagesAl::rampTo] %d -> %d\r\n", old_settings, settings_id);

   int dwell = 1000 * std::max<int>(0, (int) Eall.value.element(11, settings_id));
   int nSteps = std::max<int>(1,  (int) Eall.value.element(12, settings_id));
   int hold = 1000 * std::max<int>(0, (int) Eall.value.element(13, settings_id));
   int wait = 1000 * std::max<int>(0, (int) Eall.value.element(14, settings_id));
   int cooling = 1000 * std::max<int>(0, (int) Eall.value.element(17, settings_id));

   if(old_settings == current_settings)
      nSteps = 1;

   updateVoltages(true, nSteps, dwell, cooling);

   if(come_back > 0)
   {
      current_settings = old_settings;

      if(cooling)
           eRecover->cool(hold);
      else
            usleep( hold );

      updateVoltages(true, nSteps, dwell, cooling);

      if(cooling)
          eRecover->cool(wait);
      else
          usleep( wait );

      printf("[voltagesAl::rampTo] %d -> %d\r\n", settings_id, old_settings);
   }

   if(old_settings != current_settings)
         usleep( wait );

   if(bUpdateGUI)
      updateGUI();

   fflush(stdout);
}
#endif //CONFIG_AL
