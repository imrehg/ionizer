#ifdef CONFIG_AL
 #define COOLING_ION_NAME "Mg+"
 #define ION std::string("Mg")
 #define NDDS (8)
#endif

#ifdef CONFIG_HG
 #define COOLING_ION_NAME "Hg+"
 #define ION std::string("Hg")
 #define NDDS (2)
#endif

extern void* pulser;
extern bool g_debug_spi;


#ifdef ALUMINIZER_SIM
void usleep(int);
#endif

//#include "config_local.h"
