#ifndef CONFIG_LOCAL_H_
#define CONFIG_LOCAL_H_

#include <vector>

void init_remote_interfaces();

#ifdef CONFIG_AL
#include "Al/config_Al.h"
#endif

#ifdef CONFIG_HG
#include "Hg/config_Hg.h"
#endif

#ifdef CONFIG_LMS
#include "lms/config_lms.h"
#endif

#endif /*CONFIG_LOCAL_H_*/
