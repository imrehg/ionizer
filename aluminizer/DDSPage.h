#pragma once

using namespace std;

#include "ExperimentPage.h"

class DDSPage : public ParamsPage
{
public:
DDSPage(const string& sPageName, ExperimentsSheet* pSheet);
virtual ~DDSPage();

protected:

void SetFreq(int i, double f);
double GetFreq(int i);

void SetPhase(int i, double p);
double GetPhase(int i);

void AddAvailableActions(std::vector<std::string>*);
void on_action(const std::string&);

virtual bool RecalculateParameters();
virtual unsigned num_columns();
virtual void OnGUIButton(ParameterGUI_Base* pIP);

protected:
unsigned nDDS;

vector<GUI_string*>     Names;
vector<GUI_bool*>    SetDDS;
vector<GUI_bool*>    OnOff;
vector<GUI_double_no_label*>     Frequencies;
vector<GUI_double_no_label*>     Phases;
vector<GUI_textButton*> Reset;


QPalette pal[3];
};
