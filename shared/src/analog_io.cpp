#define NOMINMAX

#include "analog_io.h"

#include <iostream>
#include <stdexcept>

using namespace std;

analog_in::analog_in(unsigned nValues) : values(nValues)
{
}
