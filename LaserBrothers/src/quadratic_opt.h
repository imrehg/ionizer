#ifndef QUADRATIC_OPT_H
#define QUADRATIC_OPT_H

#include "actuator_opt.h"

class quadratic_opt : public Actuator_opt
{
public:
    quadratic_opt(unsigned N, opt_actuator* act);
};

#endif // QUADRATIC_OPT_H
