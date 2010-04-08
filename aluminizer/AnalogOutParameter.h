#pragma once

#include "AnalogOutput.h"
#include "InputParametersGUI.h"

class AnalogOutParameter : public GUI_doubleLCD
{
public:
AnalogOutParameter(std::auto_ptr<AnalogOut> pAO, double v0,
                   const std::string& name, InputParameters* pIPs,
                   std::vector<ParameterGUI_Base*> * pv, double bias, double gain, std::ofstream* pLog);

virtual ~AnalogOutParameter();

//shift the voltage by delta, return new voltage
double shift(double delta);

bool SetValue(double);
bool Reset();
void UpdateOutput();    //updates the analog output to the set value

double v0;
double bias, gain;
std::auto_ptr<AnalogOut> pAO;
std::ofstream* pLog;
};
