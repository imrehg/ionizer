#ifndef VOLTAGESAL_H_
#define VOLTAGESAL_H_

#include "../voltagesI.h"

#define NUM_OUT_VOLTAGES (11)

class rp_ao;

//! Interface to control trap voltages and E-fields
class voltagesAl : public voltagesI
{
public:
   voltagesAl(list_t* exp_list, const std::string& name);
   virtual ~voltagesAl() {}

   virtual void rampDownXtallize();
   virtual void rampUpXtallize();

   //! called after parameters have been updated
   virtual void updateParams();

	void set_voltage(unsigned iChannel, double V);
	double get_voltage(unsigned iChannel);

   virtual unsigned remote_action(const char* s);

   void voltagesForSetting(unsigned iSetting, bool bHV, my_matrix& ao_new);

   void updateGUI();

   //! ramps from current voltages to those specified by the current_settings column
   bool updateVoltages(bool bForceUpdate, unsigned ramp_steps=1, unsigned dwell=0, unsigned cooling=0);

   void updateInverseMatrices();

   virtual void rampTo(unsigned settings_id, unsigned come_back, bool bUpdateGUI);
   void dump();

protected:
   rp_bool NoUpdates, Debug;
   rp_bool XtallizeReorder;
   rp_unsigned XtallizeSettings;
   rp_matrix VcerfExy, VcerfEhv;
   rp_matrix Eall;

   std::vector<rp_ao*> ao_lcd;

   my_matrix ao, Vcerf_old;
   unsigned current_settings; //column # of current voltage settings
};

#endif /*VOLTAGESAL_H_*/
