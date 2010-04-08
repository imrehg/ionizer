#pragma once

#include "ParamsPage.h"
#include "messaging.h"

class FPGA_connection;
class GbE_msg_exchange;

//class that provides a GUI for the FPGA
//connects to info_interface class on the FPGA
class FPGA_GUI : public ParamsPage
{
Q_OBJECT

public:
FPGA_GUI(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);

virtual void AddParams();
void AddRemoteParams();

unsigned get_page_id()
{
	return page_id;
}

virtual void AddAvailableActions(std::vector<std::string>*);
virtual void on_action(const std::string& s);

virtual void OnEditChange(ParameterGUI_Base*);

//! Dispatch message originating from FPGA S2C...
void DispatchMsg(GbE_msg_exchange* eX, unsigned m);

protected:
virtual void setupRemoteParam(GUI_double*);
virtual void setupRemoteParam(GUI_matrix*)
{
}

virtual bool needInitFPGA()
{
	return true;
}                                               //initialize FPGA params at startup

unsigned page_id;
std::vector<ParameterGUI_Base*> FPGA_params;
std::vector<std::string> remote_actions;

public:
static FPGA_connection* pFPGA;
bool need_init_remote_actions;
};
