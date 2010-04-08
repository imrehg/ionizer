/* experiments / pages for Al-experiment */

#ifdef CONFIG_AL

#define N_MAX_COOLING_MODES 7
#define N_STRETCH_FACTORS 30

#include "voltagesAl.h"

class dacI5370;
class dacI5535;
class iMg;
class iAl3P1;
class iAl3P0;

extern dacI5370*   iVoltages5370;
extern dacI5535*   iVoltages5535;

//Global pointer to Mg+, Al+ 3P1, and Al+ 3P0 transition info pages.
//These pointers can change, so don't save them beween experiments!
extern iMg* gpMg;
extern iAl3P1* gpAl3P1;
extern iAl3P0* gpAl3P0;

#endif //CONFIG_AL
