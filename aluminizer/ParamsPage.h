#pragma once

#include "TxtParametersGUI.h"
#include "InputParametersModel.h"

class ExperimentsSheet;
class QTableView;

class ParamsPage : public TxtParametersGUI
{
public:
ParamsPage(ExperimentsSheet* pSheet, const std::string& sPageName);
virtual ~ParamsPage();

virtual void SaveParams(const std::string& OutputDirectory);

virtual void on_action(const std::string& s);
virtual void AddAvailableActions(std::vector<std::string>* v);
std::string ParamsFileName();
void createModelView();

TxtParametersModel* GetTxtParams()
{
	return &m_TxtParams;
}

public:

TxtParametersModel m_TxtParams;
QTableView* view;
};

