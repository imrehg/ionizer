#ifndef VOLTAGESI_H_
#define VOLTAGESI_H_

#include "info_interface.h"

//! Base interface to control trap voltages and E-fields
class voltagesI : public info_interface
{
public:
voltagesI(list_t* exp_list, const std::string& name);
virtual ~voltagesI();

virtual void voltagesForSetting(unsigned iSetting, bool bHV, my_matrix& ao_new) = 0;

virtual void set_voltages(const my_matrix& ao_new) = 0;

virtual void set_voltage(unsigned iChannel, double V) = 0;
virtual double get_voltage(unsigned iChannel) = 0;

virtual void rampDownXtallize();
virtual void rampUpXtallize();
virtual void rampTo(unsigned settings_id, unsigned come_back, bool bUpdateGUI);
};

extern voltagesI* iVoltages;

#endif /*VOLTAGESI_H_*/
