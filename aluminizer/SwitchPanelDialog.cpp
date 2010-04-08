// SwitchPanelDialog.cpp : implementation file
//

#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "AluminizerApp.h"
#include "CommonExperimentsSheet.h"
#include "SwitchPanelDialog.h"
#include "InputParameters.h"
#include "ExperimentsSheet.h"
#include "ExperimentPage.h"
#include "FPGA_connection.h"

using namespace std;


QPalette switch_struct::pal[3];

switch_struct::switch_struct(QWidget* parent, unsigned nBit, const std::string& sCaption) :
	cb(new QCheckBox(parent)),
	le(new QLineEdit(parent)),
	state(0),
	nBit(nBit)
{
	cb->setText(sCaption.c_str());
	cb->setTristate(true);
	le->setAlignment(Qt::AlignHCenter);

	QObject::connect(cb, SIGNAL(clicked(bool)), this, SLOT(on_cb_change(bool)), Qt::AutoConnection);

	pal[0].setColor(QPalette::Base, QColor(255, 80, 80));
	pal[1].setColor(QPalette::Base, QColor(80, 255, 80));
	pal[2].setColor(QPalette::Base, QColor(160, 160, 160));

}


void switch_struct::on_cb_change(bool)
{
	UpdateStateFromGUI();
}

switch_struct::~switch_struct()
{
	delete cb;
	delete le;
}

void switch_struct::UpdateStateGUI()
{
	switch (state)
	{
	case 0:  cb->setCheckState(Qt::Unchecked); break;
	case 1:  cb->setCheckState(Qt::Checked); break;
	case 2:  cb->setCheckState(Qt::PartiallyChecked); break;
	}

	le->setPalette(pal[state]);
}

void switch_struct::UpdateStateFromGUI()
{
	int s = cb->checkState();

	switch (s)
	{
	case Qt::Unchecked: state = 0; break;
	case Qt::Checked: state = 1; break;
	case Qt::PartiallyChecked: state = 2; break;
	}

	le->setPalette(pal[state]);

	SetLogicState(state);
}

void switch_struct::UpdateLabelGUI()
{
	le->setText(label.c_str());
}

void switch_struct::UpdateLabelFromGUI()
{
	label = string(le->text().toAscii());
}


unsigned switch_struct::GetLogicState()
{
	return state = theApp->fpga->GetLogicState(nBit);
}


void switch_struct::SetLogicState(unsigned s)
{
	theApp->fpga->SetLogicState(nBit, state = s);
}


SwitchPanel::SwitchPanel(ExperimentsSheet* pSheet,
                         unsigned uNumSwitchesH /*=4*/,
                         unsigned uNumSwitchesV /*=6*/)
	:
	AluminizerPage(pSheet, "Switches"),
	grid( this ),
	params("params_switches.txt")
{
	m_uNumSwitchesH = uNumSwitchesH;
	m_uNumSwitchesV = uNumSwitchesV;

	InitSwitches();

	ExperimentPage::pSwitches = this;
}

void SwitchPanel::AddAvailableActions(std::vector<std::string>* p)
{
	p->push_back("DEFAULT");
}


void SwitchPanel::on_action(const std::string& s)
{
	if (s == "DEFAULT")
	{
		for (unsigned i = 0; i < m_vSwitches.size(); i++)
		{
			m_vSwitches[i]->SetLogicState(2);
			m_vSwitches[i]->UpdateStateGUI();
		}
	}
	else
		AluminizerPage::on_action(s);
}

void SwitchPanel::InitSwitches()
{
	for (unsigned i = 0; i < m_uNumSwitchesV; i++)
	{
		for (unsigned j = 0; j < m_uNumSwitchesH; j++)
		{
			char sCaption[5];
			unsigned uSwitch = hv2i(j, i);
			snprintf(sCaption, 4, "%02u", uSwitch);

			m_vSwitches.push_back(new switch_struct(this, uSwitch, sCaption));

			grid.addWidget(m_vSwitches.back()->cb, 3 * i, j, Qt::AlignHCenter | Qt::AlignBottom);
			grid.addWidget(m_vSwitches.back()->le, 3 * i + 1, j, Qt::AlignTop);
		}
	}

	grid.addItem(new QSpacerItem(600, 600, QSizePolicy::Maximum, QSizePolicy::Maximum), 3 * m_uNumSwitchesV, 0, 1, m_uNumSwitchesH);
//	grid.addItem(new QSpacerItem(600, 600, QSizePolicy::Maximum, QSizePolicy::Maximum), 0, m_uNumSwitchesH, m_uNumSwitchesV, 1);

	RestoreLastState();
	show();
}


SwitchPanel::~SwitchPanel()
{
	SaveParams("");

	while (!m_vSwitches.empty())
	{
		delete m_vSwitches.back();
		m_vSwitches.pop_back();
	}
}


unsigned SwitchPanel::NumSwitches()
{
	return m_uNumSwitchesV * m_uNumSwitchesH;
}

unsigned SwitchPanel::hv2i(unsigned h, unsigned v)
{
//	return (m_uNumSwitchesH-h-1)*m_uNumSwitchesV + v;
	return v * m_uNumSwitchesH + h;
}



void SwitchPanel::SaveParams(const std::string& dir)
{
	GetButtonStates();
	GetLabelEdts();

	string sKey;
	for (unsigned i = 0; i < NumSwitches(); i++)
	{
		params.UpdatePair(GetStateKey(i), to_string<int>(m_vSwitches[i]->state), -1);
		params.UpdatePair(GetLabelKey(i), m_vSwitches[i]->label, -1);
	}

	params.SaveState(dir);
}

unsigned SwitchPanel::GetLogicState(unsigned nBit)
{
	return m_vSwitches.at(nBit)->GetLogicState();
}


void SwitchPanel::SetLogicState(unsigned nBit, unsigned state)
{
	m_vSwitches.at(nBit)->SetLogicState(state);
}



void SwitchPanel::RestoreLastState()
{
	string sKey;

	for (unsigned i = 0; i < NumSwitches(); i++)
	{
		m_vSwitches[i]->state = this->GetLogicState(i);

		this->SetLogicState(i, m_vSwitches[i]->state);

		try
		{
			m_vSwitches[i]->label = params.GetValue(GetLabelKey(i));
		}
		catch (Uninitialized u)
		{}
	}

	SetButtonStates();
	SetLabelEdts();

	cout << "setup switches...done" << endl;
}

void SwitchPanel::SetButtonStates()
{
	for (unsigned i = 0; i < NumSwitches(); i++)
		m_vSwitches[i]->UpdateStateGUI();
}


void SwitchPanel::GetButtonStates()
{
	for (unsigned i = 0; i < NumSwitches(); i++)
		m_vSwitches[i]->UpdateStateFromGUI();
}


void SwitchPanel::SetLabelEdts()
{
	for (unsigned i = 0; i < NumSwitches(); i++)
		m_vSwitches[i]->UpdateLabelGUI();
}

void SwitchPanel::GetLabelEdts()
{
	for (unsigned i = 0; i < NumSwitches(); i++)
		m_vSwitches[i]->UpdateLabelFromGUI();
}

const std::string SwitchPanel::GetStateKey(unsigned i)
{
	char szKey[20];

	snprintf(szKey, 19, "switch%02ustate", i);
	return szKey;
}

const std::string SwitchPanel::GetLabelKey(unsigned i)
{
	char szKey[20];

	snprintf(szKey, 19, "switch%02ulabel", i);
	return string(szKey);
}

bool SwitchPanel::RecalculateParameters()
{
	SetButtonStates();

	return false;
}

int SwitchPanel::FindNamedSwitch(const std::string& name)
{
	for (unsigned i = 0; i < NumSwitches(); i++)
		if (m_vSwitches[i]->label == name)
			return i;

	return -1;
}

TTL_switch::TTL_switch(SwitchPanel* pSwitches, const std::string& name) :
	name(name),
	pSwitches(pSwitches)
{
	index = pSwitches->FindNamedSwitch(name);

	if (index < 0)
		cerr << "[TTL_switch::TTL_switch] warning: can't find switch named " << name << endl;
}

void TTL_switch::check_index()
{
	if (index < 0)
		index = pSwitches->FindNamedSwitch(name);

	if (index < 0)
		throw runtime_error("[TTL_switch] error: can't find switch named " + name);
}

void TTL_switch::on()
{
	check_index();
	pSwitches->SetLogicState(index, 1);
}

void TTL_switch::off()
{
	check_index();
	pSwitches->SetLogicState(index, 0);
}

void TTL_switch::SetLogicState(unsigned state)
{
	check_index();
	pSwitches->SetLogicState(index, state);
}

unsigned TTL_switch::GetLogicState()
{
	check_index();
	return pSwitches->GetLogicState(index);
}

