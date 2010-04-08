#pragma once

class AnalogOut
{
public:
AnalogOut(double min_output, double max_output, double gain = 1, double offset = 0) :
	min_output(min_output),
	max_output(max_output),
	gain(gain),
	offset(offset)
{
}

virtual ~AnalogOut()
{
}

class bad_output_exception
{
public:
bad_output_exception(double v, double vMin, double vMax) : v(v), vMin(vMin), vMax(vMax)
{
}
double v, vMin, vMax;
};

virtual void SetOutput(double) = 0;
virtual double GetOutput() = 0;

virtual int GetChannel() = 0;

double GetMinOutput() const
{
	return min_output;
}
double GetMaxOutput() const
{
	return max_output;
}

bool IsValidOutput(double V) const
{
	return (min_output <= V) && (V <= max_output);
}

protected:
double min_output;
double max_output;

double gain, offset;
};

class FPGA_connection;

/*
   class FPGA_AnalogOut : public AnalogOut
   {
   public:
   FPGA_AnalogOut(FPGA_connection* pFPGA, const std::string& pageName, unsigned iChannel, double min_output, double max_output, double gain=1, double offset=0);
   virtual ~FPGA_AnalogOut();

   virtual void SetOutput(double);
   virtual double GetOutput() { return dVoltage; }

   int GetChannel() { return iChannel; }

   protected:
   FPGA_connection* pFPGA;
   std::string pageName;
   unsigned iChannel;
   double dVoltage;
   int iPage;
   };
 */

class SimAnalogOut : public AnalogOut
{
public:
SimAnalogOut(double min_output, double max_output) :
	AnalogOut(min_output, max_output),
	V(0)
{
}

virtual ~SimAnalogOut()
{
}

virtual void SetVoltage(double V)
{
	this->V = V;
}
virtual double GetVoltage()
{
	return V;
}

protected:
double V;
};






