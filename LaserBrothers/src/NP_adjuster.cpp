#include "NP_adjuster.h"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <stdexcept>

using namespace std;

#ifdef WIN32
#define  snprintf  _snprintf
#endif

NP_adjuster::NP_adjuster(const std::string& port, unsigned baud, const std::string& cmd, double minT, double maxT, double deltaT) :
 RS232device(port, baud),
 cmd(cmd),
 minT(minT),
 maxT(maxT),
 T(0),
 deltaT(deltaT),
 numValidT(0),
 numQueries(0)
{
    bDebugRS232 = true;

    try
    {
            if(isPortOpen())
            {
                    idn = sendCmd_getResponse("*IDN?\r", '\n');
                    sendCmd(":SYSTem:PASSword:CENable \"NP\"\r"); //unlock protected commands

                    timeOfLastAdj.start();
                    timeOfLastT.start();
            }

    }
    catch(runtime_error e)
    {
            cerr << "RUNTIME ERROR: " << endl;
            cerr << "\t" << e.what() << endl;
    }

    bDebugRS232 = false;
}

NP_adjuster::~NP_adjuster()
{
}

double NP_adjuster::getTemperature()
{
    if(numValidT > 5)
        return T;

    if(numQueries>0 && timeOfLastT.elapsed() < 1000 && timeOfLastT.elapsed() > 100)
    {
        double t = 0;
        string s = getResponse('\n');
        numQueries--;

        istringstream iss(s.c_str());

        iss >> t;

        if(t != 0 && t > minT && t < maxT)
        {
            numValidT++;
            timeOfLastT.restart();
            T = t;
        }

        cout << idn << " T = " << t << "  minT = " << minT << "  maxT = " << maxT <<  " numValidT = " << numValidT << endl;
    }
    else
    {
        if(timeOfLastT.elapsed() > 500)
        {
            timeOfLastT.restart();
            getTempCmd();
            numQueries++;
        }
    }

    return T;
}

void NP_adjuster::getTempCmd()
{
    sendCmd(cmd + ":SPO?\r");
}

double NP_adjuster::shiftTemperature(double dT)
{
    T = getTemperature();

    if(T != 0)
    {
        T = setTemperature(T+dT);
    }

    return T;
}

double NP_adjuster::setTemperature(double newT)
{
    if(T == newT)
        return newT;

    timeOfLastAdj.restart();

    T = restrict(newT, minT, maxT);

    char buff[128];
    snprintf(buff, 127, "%s:SPO %f\r", cmd.c_str(), T);
    sendCmd(buff);

    return T;
}

double NP_adjuster::shiftTemperatureUp()
{
	
	return shiftTemperature(deltaT);
}

double NP_adjuster::shiftTemperatureDown()
{
	return shiftTemperature(-1*deltaT);
}


double restrict(double v, double bottom, double top)
{
	if(v > top)
		v = top;

	if(v < bottom)
		v = bottom;

	return v;
}
