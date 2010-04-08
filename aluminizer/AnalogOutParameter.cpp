#include "InputParametersGUI.h"
#include "AnalogOutParameter.h"

using namespace std;

AnalogOutParameter::AnalogOutParameter(std::auto_ptr<AnalogOut> pAO,
                                       double v0,
                                       const std::string& name, InputParameters* pIPs,
                                       std::vector<ParameterGUI_Base*> * pv,
                                       double bias,
                                       double gain,
                                       std::ofstream* /*pLog*/) :
	Parameter_Base(name),
	GUI_doubleLCD(name, pIPs, to_string<double>(v0), pv),
	v0(v0),
	bias(bias),
	gain(gain),
	pAO(pAO),
	pLog(0)
{
	setPrecision(6);
}

AnalogOutParameter::~AnalogOutParameter()
{
}

void AnalogOutParameter::UpdateOutput()
{
	try
	{

		double v = Value() * gain + bias;

		pAO->SetOutput( v );


		if (pLog)
			*pLog << setprecision(3) << fixed << CurrentTime_s() << ", " << pAO->GetChannel() << ", " << pAO->GetOutput() << endl;
	}
	catch (AnalogOut::bad_output_exception bv)
	{
		cerr << "[" << GetName() << "::SetValue] bad output: " << bv.v << endl;
	}
}

double AnalogOutParameter::shift(double delta)
{
	SetValue(Value() + delta);
	return Value();
}

bool AnalogOutParameter::SetValue(double d)
{
	double dBiased = d * gain + bias;

	//confine voltage to allowed range
	if ( dBiased < pAO->GetMinOutput())
	{
		cerr <<  "[" << GetName() << "::SetValue] warning: clipping " << dBiased << " to " << pAO->GetMinOutput() << endl;
		cout <<  "[" << GetName() << "::SetValue] warning: clipping " << dBiased << " to " << pAO->GetMinOutput() << endl;
		d = (pAO->GetMinOutput() - bias) / gain;

		creepy_speak("Warning!  Analog output too low.");
	}

	if (dBiased > pAO->GetMaxOutput())
	{
		cerr <<  "[" << GetName() << "::SetValue] warning: clipping " << dBiased << " to " << pAO->GetMaxOutput() << endl;
		cout <<  "[" << GetName() << "::SetValue] warning: clipping " << dBiased << " to " << pAO->GetMaxOutput() << endl;
		d = (pAO->GetMaxOutput() - bias) / gain;

		creepy_speak("Warning!  Analog output too high.");
	}

	if (GUI_doubleLCD::SetValue(d))
	{
//		UpdateOutput();
//		UpdateGUI();

		return true;
	}

	return false;
}

bool AnalogOutParameter::Reset()
{
	cout << GetName() << " = " << v0 << endl;
	return SetValue(v0);
}
