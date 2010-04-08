#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "Bfield.h"
#include "MCC_AnalogOut.h"

#include "ExperimentPage.h"
//#include "AluminizerApp.h"

#include <tnt.h>
#include <jama_lu.h>

Bfield* gBfield = 0;


Bfield::Bfield(const std::string& sPageName, ExperimentsSheet* pSheet) :
	ParamsPage(pSheet, sPageName),
	B0gui(3),
	Bgui(3),
	Igui(3),
	B2Igui(9),
	B2I(3, 3, 0.),
	B(3, 1, 0.),
	I(3, 1, 0.)
{
	gBfield = this;

	int iDevice = 1;
	char name[64];

	for (int i = 0; i < 3; i++)
	{
		snprintf(name, 63, "Bo(%d)", i);

		B0gui[i] = new GUI_double(name,  &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(B0gui[i]);
		B0gui[i]->SetReadOnly(true);
		B0gui[i]->setRange(-1e5, 1e5);
		B0gui[i]->setSuffix(" mG");
	}

	for (int i = 0; i < 3; i++)
	{
		snprintf(name, 63, "B(%d)", i);

		Bgui[i] = new GUI_double(name,   &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(Bgui[i]);
		Bgui[i]->setRange(-1e5, 1e5);
		Bgui[i]->setSuffix(" mG");
	}

	for (int i = 0; i < 3; i++)
	{
		vector<int> output_channels(4);
		for (int j = 0; j < 4; j++)
			output_channels[j] = 4 * i + j;

		snprintf(name, 63, "I(%d)", i);
		Igui[i] = new AnalogOutParameter(auto_ptr<AnalogOut>(new MCC_AnalogOut(iDevice, output_channels)), 0,
		                                 name, &m_TxtParams,  &m_vParameters, 0, 1, 0);

		m_vAllocatedParams.push_back(Igui[i]);

		Igui[i]->SetReadOnly(true);
		Igui[i]->setRange(-1e4, 1e4);
		Igui[i]->setSuffix(" mA");
	}

	for (int i = 0; i < 9; i++)
	{
		int r = i / 3;
		int c = i % 3;

		snprintf(name, 63, "B2I(%d,%d)", r, c);
		B2Igui[i] = new GUI_double(name, &m_TxtParams, "1", &m_vParameters, RP_FLAG_READ_ONLY);

		B2I[r][c] = B2Igui[i]->Value();

		m_vAllocatedParams.push_back(B2Igui[i]);
		B2Igui[i]->setRange(-1e4, 1e4);
	}

	for (int i = 0; i < 3; i++)
		g_scan_sources.push_back(new BScanSource(i, this));

	SafeRecalculateParameters();
}

Bfield::~Bfield()
{
}

void Bfield::SetB(unsigned i, double B)
{
	Bgui.at(i)->SetValue(B);
	Update();
}

double Bfield::GetB(unsigned i)
{
	return Bgui.at(i)->Value();
}

void Bfield::SetB0(unsigned i, double B)
{
	B0gui.at(i)->SetValue(B);
	Update();
}

double Bfield::GetB0(unsigned i)
{
	return B0gui.at(i)->Value();
}

void Bfield::Update()
{
	for (int i = 0; i < 3; i++)
		B[i][0] = Bgui[i]->Value() + B0gui[i]->Value();

	I = matmult(B2I, B);

	for (int i = 0; i < 3; i++)
		Igui[i]->SetValue(I[i][0]);

	for_each(Igui.begin(), Igui.end(), mem_fun(&AnalogOutParameter::UpdateOutput));
}

bool Bfield::RecalculateParameters()
{
	Update();

	return false;
}
