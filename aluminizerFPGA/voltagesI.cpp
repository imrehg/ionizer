#include "voltagesI.h"
#include "dacI.h"

voltagesI::voltagesI(list_t* exp_list, const std::string& name) :
	info_interface(exp_list, name)
{
}

voltagesI::~voltagesI()
{
}


void voltagesI::rampDownXtallize()
{
}

void voltagesI::rampUpXtallize()
{
}

void voltagesI::rampTo(unsigned, unsigned, bool)
{
}
