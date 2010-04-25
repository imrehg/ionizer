#pragma once

#include "InputParametersModel.h"
#include "InputParametersGUI.h"
#include "AluminizerPage.h"
#include "CriticalSection.h"

//this is the class that will host the various parameters in the GUI

class TxtParametersGUI : public AluminizerPage
{
Q_OBJECT

public:
//constructor for text file input
TxtParametersGUI(ExperimentsSheet* pSheet, const std::string& title);

virtual ~TxtParametersGUI();

const std::string& Title()
{
	return title;
}

void HighlightParameter(ParameterGUI_Base* p);

ParameterGUI_Base* FindParameter(const std::string& name) const;
ParameterGUI_Base* FindFPGAParameter(const std::string& name) const;
ParameterGUI_Base* FindParameterContaining(const std::string& name) const;
ParameterGUI_Base* FindParameterContaining(const std::string& s1, const std::string& s2) const;

void InitWindow();
void PlaceWindows();
virtual void PostCreateGUI()
{
}

virtual void AddParams()
{
}
virtual unsigned num_columns();

protected:

bool SafeRecalculateParameters();

virtual void OnGUIButton(ParameterGUI_Base*)
{
}

void OnRun();
void OnUpdateData();
void OnClose();
virtual void OnEditChange(ParameterGUI_Base*);

void SetDialogContents();
bool GetDialogContents(ParameterGUI_Base* p = 0);

//add a parameter to the list
void AddParameter(ParameterGUI_Base*);

//remove a parameter from the list
void RemoveParameter(ParameterGUI_Base*);

bool ChangeParameterName(ParameterGUI_Base* p, const std::string& name);
bool ReplaceParameter(ParameterGUI_Base* pOld, ParameterGUI_Base* pNew);

virtual bool RecalculateParameters()
{
	return false;
}
virtual bool needsBottomSpacer()
{
	return true;
}

std::vector<ParameterGUI_Base*>* GetParamsVector()
{
	return &m_vParameters;
}

public slots:
void on_GUI_user_change(ParameterGUI_Base*);
void on_GUI_button_was_pressed(ParameterGUI_Base*);

protected: /* protected data */
std::string title;

static int nColumns;
public:
std::vector<ParameterGUI_Base*> m_vParameters;

protected:
std::vector<ParameterGUI_Base*> m_vAllocatedParams;

std::vector<bool> m_bPacked;

bool m_bSettingText;

unsigned GUI_Revision;
unsigned DataRevision;

//protect RecalculateParameters() from multi-threading
NamedCriticalSection csRecalculating;

QHBoxLayout top_layout;
QGridLayout grid;
std::vector<QHBoxLayout*> hgrids;

bool bIgnoreGUISignals;
QReadWriteLock page_lock;
QWaitCondition page_updated;
QMutex mutexPU;
QSpacerItem* bottomSpacer;

static unsigned gridV_spacing, default_num_columns;

private slots:
bool UpdateData();

};

