#pragma once

#include "CommonExperimentsSheet.h"
#include "RefCavityPage.h"

class MgAlExperimentsSheet : public CommonExperimentsSheet
{
public:

MgAlExperimentsSheet();
virtual ~MgAlExperimentsSheet();

//statically allocated pages
//	ElsnerPage			m_ElsnerPage;
//	ULE88Page			m_ULE88Page;


protected:
vector<FPGA_GUI*> vPages;
};


