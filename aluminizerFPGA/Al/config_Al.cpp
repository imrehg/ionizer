/* experiments / pages for Al-experiment */

#ifdef CONFIG_AL

#include "../common.h"

#include "../dacI.h"
#include "../voltagesI.h"
#include "../exp_recover.h"
#include "../exp_correlate.h"
#include "exp_Al3P1.h"
#include "exp_Al3P0.h"

list_t global_exp_list;

infoFPGA* 		iFPGA;
dacI5370*   	iVoltages5370;
dacI5535*   	iVoltages5535;
voltagesI*  	iVoltages;
iMg* 		gpMg;
iAl3P1* 	gpAl3P1;
iAl3P0* 	gpAl3P0;
exp_3P0_lock* 	e3P0LockPQ;
exp_3P0_lock* 	e3P0LockMQ;

//! Initialize info_interfaces.  Gets called before any remote I/O takes place.
void init_remote_interfaces()
{	
    iFPGA = new infoFPGA(&global_exp_list, "FPGA");

    iVoltages5370 = new dacI5370(&global_exp_list, "AD5370");
    iVoltages5535 = new dacI5535(&global_exp_list, "AD5535");

    iVoltages = new voltagesAl(&global_exp_list, "Voltages");

    gpMg = 0;

    new iMg(&global_exp_list, "Mg");
    new iMg(&global_exp_list, "Mg Al");
    new iMg(&global_exp_list, "Mg Al Mg");
    new iMg(&global_exp_list, "Al Mg Al Mg");

    gpAl3P1 = new iAl3P1(&global_exp_list, "Al 3P1");
    gpAl3P0 = new iAl3P0(&global_exp_list, "Al 3P0");

    eRecover = new exp_recover(&global_exp_list);
    new exp_detect(&global_exp_list);
    new exp_detectN(&global_exp_list);
    new exp_correlate(&global_exp_list);

#ifndef ALUMINIZER_SIM
	new exp_load_Mg  (&global_exp_list, "Load Mg+");
	new exp_load_Al  (&global_exp_list, "Load Al+");
	
	new exp_repump  (&global_exp_list);
	new exp_heat    (&global_exp_list, "Heat Z");
	new exp_heat    (&global_exp_list, "Heat XY");
	
	//exp_heat    eHeat2	 (&global_exp_list, "Ex lock");
	//exp_heat    eHeat3	 (&global_exp_list, "Ey lock");
	new exp_heat    (&global_exp_list, "Ex freq");
	new exp_heat    (&global_exp_list, "Ex lock");
	new exp_heat    (&global_exp_list, "Ey freq");
	new exp_heat    (&global_exp_list, "Ey lock");
	new exp_heat    (&global_exp_list, "Ev lock");
#endif //ALUMINIZER_SIM

	new exp_raman	   (&global_exp_list, "Qubit cal 1");
	new exp_raman	   (&global_exp_list, "Qubit cal 2");
	new exp_raman_RF   (&global_exp_list, "Qubit scan");

	new exp_raman	   (&global_exp_list, "Order check");
	
	new exp_3P1_test   (&global_exp_list, "3P1 cal");
	new exp_3P1_test   (&global_exp_list, "3P1 scan");
	new exp_3P1_test   (&global_exp_list, "3P1 MM sigma");
	new exp_3P1_test   (&global_exp_list, "3P1 MM vert.");
	new exp_3P1_test   (&global_exp_list, "3P1 Eh lock");
	new exp_3P1_test   (&global_exp_list, "3P1 Ev lock");


#ifndef ALUMINIZER_SIM
//exp_3P1_ent    e3P1ent			(&global_exp_list);
//exp_3P1_entRF  e3P1entRF		(&global_exp_list);
#endif //ALUMINIZER_SIM

	new exp_3P0        (&global_exp_list, "3P0 cal");
	new exp_3P0        (&global_exp_list, "3P0 scan");
	//exp_3P0_lock   e3P0LockP			(&global_exp_list, "3P0 lock(+)");
	//exp_3P0_lock   e3P0LockM			(&global_exp_list, "3P0 lock(-)");
	
	//exp_3P0_lock   e3P0LockPS			(&global_exp_list, "3P0 lock(+) shifted");
	e3P0LockPQ = new exp_3P0_lock   (&global_exp_list, "3P0 lock(+) HQ", 21);
	e3P0LockMQ = new exp_3P0_lock   (&global_exp_list, "3P0 lock(-) HQ", 21);
	
	new exp_3P0_lock   (&global_exp_list, "3P0 lock(+) HQ2", 21);
	new exp_3P0_lock   (&global_exp_list, "3P0 lock(-) HQ2", 21);

	new exp_rf	       (&global_exp_list, "RF 3-2 a");
	
/*
exp_rf	    eRF0b		 (&global_exp_list, "RF 3-2 b");
exp_rf	    eRF1		 (&global_exp_list, "RF 3-2-2");
exp_rf	    eRF2		 (&global_exp_list, "RF 3-2-2-1");
exp_rf	    eRF3		 (&global_exp_list, "RF 3-2-2-1-1");
exp_rf	    eRF4		 (&global_exp_list, "RF 3-2-2-1-1-0");
exp_rf	    eRF5		 (&global_exp_list, "RF 3-2-2-1-1-0-0");
*/

#ifndef ALUMINIZER_SIM
	new exp_scanDDS (&global_exp_list, "Scan DDS");
	new exp_BS (&global_exp_list);
	new exp_HiFi_Detect (&global_exp_list);
#endif // ALUMINIZER_SIM
}

#endif //CONFIG_AL
