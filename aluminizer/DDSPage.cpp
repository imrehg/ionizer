#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <QMessageBox>

#include "DDSPage.h"
#include "ExperimentPage.h"
#include "FPGA_connection.h"

#ifdef CONFIG_HG
#define NDDS 2
#endif

#ifdef CONFIG_AL
#define NDDS 8
#endif

DDSPage::DDSPage(const std::string& sPageName, ExperimentsSheet* pSheet) :
	ParamsPage(pSheet, sPageName),
	nDDS(NDDS),
	Names(nDDS),
	SetDDS(nDDS),
	OnOff(nDDS),
	Frequencies(nDDS),
	Phases(nDDS),
	Reset(nDDS)
{
	pal[0].setColor(QPalette::Base, QColor(255, 80, 80));
	pal[1].setColor(QPalette::Base, QColor(80, 255, 80));
	pal[2].setColor(QPalette::Base, QColor(160, 160, 160));

	for (unsigned i = 0; i < nDDS; i++)
	{
		char sbuff[64];

		snprintf(sbuff, 64, "[%d]", i);
		Names[i] = new GUI_string(sbuff, &m_TxtParams, "DDS Name", &m_vParameters);
		Names[i]->setInputWidth(100);
		m_vAllocatedParams.push_back(Names[i]);

		snprintf(sbuff, 64, "Set (%d)", i);
		SetDDS[i] = new GUI_bool(sbuff,  &m_TxtParams, "false", &m_vParameters);
		SetDDS[i]->set_display_label("Set");
		m_vAllocatedParams.push_back(SetDDS[i]);

		snprintf(sbuff, 64, "On (%d)", i);
		OnOff[i] = new GUI_bool(sbuff,   &m_TxtParams, "true", &m_vParameters);
		OnOff[i]->set_display_label("On");
		m_vAllocatedParams.push_back(OnOff[i]);

		snprintf(sbuff, 64, "Freq (%d)", i);
		Frequencies[i] = new GUI_double_no_label(sbuff, &m_TxtParams, "0", &m_vParameters);
		m_vAllocatedParams.push_back(Frequencies[i]);
		Frequencies[i]->setPrecision(9);
		Frequencies[i]->setRange(0, 500);
		Frequencies[i]->setIncrement(FTW2MHz(Hz2FTW(1000)));
		Frequencies[i]->setSuffix(" MHz");

		snprintf(sbuff, 64, "Phase (%d)", i);
		Phases[i] = new GUI_double_no_label(sbuff,   &m_TxtParams, "0", &m_vParameters);
		Phases[i]->set_display_label("Phase");
		m_vAllocatedParams.push_back(Phases[i]);
		Phases[i]->setRange(0, 360);
		Phases[i]->setSuffix(" deg");
		Phases[i]->setIncrement(1);

		snprintf(sbuff, 64, "Reset (%d)", i);
		Reset[i] = new GUI_textButton(sbuff,   &m_TxtParams, "false", &m_vParameters);
		Reset[i]->set_display_label("Reset");
		m_vAllocatedParams.push_back(Reset[i]);
	}

	SafeRecalculateParameters();
}

DDSPage::~DDSPage()
{
}

void DDSPage::SetFreq(int i, double f)
{
	ExperimentPage::pFPGA->SetDDS_FTW(i, Hz2FTW(f));
}

void DDSPage::SetPhase(int i, double p)
{
	ExperimentPage::pFPGA->SetDDS_Phase(i, p);
}

void DDSPage::AddAvailableActions(std::vector<std::string>* p)
{
	ParamsPage::AddAvailableActions(p);
	p->push_back("TEST");

}


void DDSPage::on_action(const std::string& s)
{
	if (s == "TEST")
	{
		int ret = QMessageBox::warning(this, tr("Aluminizer2"),
		                               tr("WARNING: DDS boards will jump to random frequencies.\n"
		                                  "Please shut off the output stage amplifiers before pressing OK."),
		                               QMessageBox::Ok | QMessageBox::Cancel);

		if (ret == QMessageBox::Ok)
		{
			ExperimentPage::pFPGA->TestDDS(1000000);
			RecalculateParameters();

			QMessageBox::information(this, tr("Aluminizer2"),
			                         tr("DDS test finished.\n"
			                            "It is now safe to turn on the output stage amplifiers."),
			                         QMessageBox::Ok);
		}
	}
	else
		ParamsPage::on_action(s);

}

bool DDSPage::RecalculateParameters()
{
	bool bChanged = false;

	for (unsigned i = 0; i < nDDS; i++)
	{
		Frequencies[i]->SetReadOnly(!SetDDS[i]->Value());
		Phases[i]->SetReadOnly(!SetDDS[i]->Value());

		int palette_index = 0;

		if (OnOff[i]->Value())
		{
			if (SetDDS[i]->Value())
			{
				//round to nearest achievable freq
				Frequencies[i]->SetValue(FTW2MHz(MHz2FTW(Frequencies[i]->Value())));

				palette_index = 1;
				printf("set freq (%d) %15.9f MHz\n", i, Frequencies[i]->Value());
				SetFreq(i, 1e6 * Frequencies[i]->Value());
				SetPhase(i, (Phases[i]->Value() / 360.0) * pow(2.0, 14.0));
			}
			else
			{
				palette_index = 2;
				bChanged |= Frequencies[i]->SetValue(FTW2MHz(ExperimentPage::pFPGA->GetDDS_FTW(i)));
				bChanged |= Phases[i]->SetValue((360.0 * (ExperimentPage::pFPGA->GetDDS_Phase(i))) / pow(2.0, 14.0));
			}
		}
		else
		{
			palette_index = 0;
			printf("set freq (%d) %15.9f MHz\n", i, 0.);
			SetFreq(i, 0);
		}

		Frequencies[i]->setInputPalette(pal[palette_index]);
		Phases[i]->setInputPalette(pal[palette_index]);

	}

	return false;
}

unsigned DDSPage::num_columns()
{
	return 10;
}

void DDSPage::OnGUIButton(ParameterGUI_Base* pIP)
{
	for (unsigned i = 0; i < nDDS; i++)
		if (pIP == Reset[i])
			ExperimentPage::pFPGA->ResetDDS(i);
}
