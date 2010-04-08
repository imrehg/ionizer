#ifndef MOTIONPAGE_H_
#define MOTIONPAGE_H_

#include "ExperimentPage.h"

class MotionPage : public ParamsPage
{
public:

MotionPage(const std::string& title, ExperimentsSheet* pSheet);
virtual ~MotionPage();


//Mode frequency in Hz
bool  SetModeFrequency(int delta_n, double f);

//Mode frequency in Hz
double  GetModeFrequency(int delta_n);

//Mode heating time in us
bool  SetModeHeatTime(int delta_n, double );

//Mode heating time in us
double  GetModeHeatTime(int delta_n);

//Mode frequency parameter
GUI_double*  GetModeFrequencyParam(int delta_n);

//Mode heating time parameter
GUI_double*  GetModeHeatTimeParam(int delta_n);

std::string ModeName(int sb);
virtual unsigned num_columns();
};

#endif /*MOTIONPAGE_H_*/
