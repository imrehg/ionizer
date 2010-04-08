#pragma once

#define KEY_SWITCH_PANEL         "SwitchPanel"


#include "DigitalOut.h"
#include "InputParameters.h"
#include "AluminizerPage.h"

// SwitchPanel dialog

class switch_struct : QObject
{
Q_OBJECT

public:
switch_struct(QWidget* parent, unsigned nBit, const std::string& sCaption);
~switch_struct();

void SetLogicState(unsigned state);
unsigned GetLogicState();

void UpdateStateGUI();
void UpdateStateFromGUI();

void UpdateLabelGUI();
void UpdateLabelFromGUI();

QCheckBox* cb;
QLineEdit* le;

int state;    //0->unchecked, 1->checked, 2->other
std::string label;



public slots:
void on_cb_change(bool);

protected:
static QPalette pal[3];
unsigned nBit;
};

class SwitchPanel : public AluminizerPage
{
public:
// standard constructor
// specify the width of the switch panel in uNumSwitchesH and the height in uNumSwitchesV
SwitchPanel(ExperimentsSheet* pSheet, unsigned uNumSwitchesH = 4, unsigned uNumSwitchesV = 8);

virtual ~SwitchPanel();

int FindNamedSwitch(const std::string& name);

void SetLogicState(unsigned nBit, unsigned state);
unsigned GetLogicState(unsigned nBit);

void InitWindow()
{
}

virtual bool RecalculateParameters();

void AddAvailableActions(std::vector<std::string>*);
void on_action(const std::string&);

public:
unsigned NumSwitches();             //returns the number of switches in the array

protected:
unsigned m_uNumSwitchesH;     //Horizontal size of switch array
unsigned m_uNumSwitchesV;     //Vertical size of switch array


//the switches
std::vector<switch_struct*> m_vSwitches;

//convert 2D to 1D switch index
//override this function to achieve a different layout
virtual unsigned hv2i(unsigned h, unsigned v);

virtual void SaveParams(const std::string& OutputDirectory);

//restore state info
void RestoreLastState();

//transfer internal switch states to the UI
void SetButtonStates();
void SetButtonState(unsigned i);
//transfer UI switch states to the internal state
void GetButtonStates();
void GetButtonState(unsigned i);

//transfer internal label data to the UI
void SetLabelEdts();
//transfer UI labels the internal state
void GetLabelEdts();

//generate registry key strings from an index value
const std::string GetLabelKey(unsigned i);
const std::string GetStateKey(unsigned i);

public:
void InitSwitches();

//chooses switch label background brush depending on the switch state
//	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

protected:
QGridLayout grid;
TxtParameters params;
};


class TTL_switch
{
public:
TTL_switch(SwitchPanel* pSwitches, const std::string& name);
virtual ~TTL_switch()
{
}

void on();
void off();

void SetLogicState(unsigned state);
unsigned GetLogicState();

protected:
void check_index();

std::string name;
SwitchPanel* pSwitches;
int index;
};

