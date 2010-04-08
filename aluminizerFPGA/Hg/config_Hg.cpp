/* experiments / pages for Hg-experiment */

#ifdef CONFIG_HG

#include "../common.h"
#include "../experiments.h"
#include "../exp_recover.h"
#include "../dacI.h"
#include "../voltagesI.h"
#include "../exp_LMS.h"
#include "../exp_correlate.h"

list_t global_exp_list;

infoFPGA* 		iFPGA;
voltagesI*  	iVoltages;
dacI5668*		iAD5668;

//exp_detect  eZeroBx  (&global_exp_list, "Zero Bx");
//exp_detect  eZeroBy  (&global_exp_list, "Zero By");
//exp_detect  eZeroBz  (&global_exp_list, "Zero Bz");

//exp_scanDDS eScanDDS (&global_exp_list, "Scan DDS");

//! Initialize info_interfaces.  Gets called before any remote I/O takes place.
void init_remote_interfaces()
{	
	iFPGA 		= new infoFPGA(&global_exp_list, "FPGA");
	eRecover 	= new exp_recover(&global_exp_list);
	iVoltages   = 0;
	iAD5668 	= new dacI5668(&global_exp_list, "AD5668");
	
	new exp_detect	(&global_exp_list);
	new exp_heat    (&global_exp_list, "Heat Z");
	new exp_scanDDS (&global_exp_list, "Scan DDS");
	new exp_LMS		(&global_exp_list);
	new exp_correlate(&global_exp_list);
}
	
#endif //CONFIG_HG
