/* experiments / pages for lms-experiment */

#ifdef CONFIG_LMS

#include "../common.h"
#include "../experiments.h"
#include "../dacI.h"
#include "../voltagesI.h"

list_t global_exp_list;

infoFPGA* 		iFPGA;
voltagesI*       iVoltages = 0;

//! Initialize info_interfaces.  Gets called before any remote I/O takes place.
void init_remote_interfaces()
{	
	iFPGA 		= new infoFPGA(&global_exp_list, "FPGA");
	new exp_scanDDS (&global_exp_list, "Scan DDS");
}
	
#endif //CONFIG_LMS
