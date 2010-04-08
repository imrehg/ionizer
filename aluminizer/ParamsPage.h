#pragma once

#include "TxtParametersGUI.h"

class ExperimentsSheet;

class ParamsPage : public TxtParametersGUI
{
public:
ParamsPage(ExperimentsSheet* pSheet, const std::string& sPageName);
virtual ~ParamsPage()
{
};

virtual void SaveParams(const std::string& OutputDirectory)
{
	m_TxtParams.SaveState(OutputDirectory);
}

std::string ParamsFileName();

TxtParameters* GetTxtParams()
{
	return &m_TxtParams;
}

public:

TxtParameters m_TxtParams;
};

