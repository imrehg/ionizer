#pragma once

using namespace std;

#include "FPGA_GUI.h"

std::string getTcpStatusText(unsigned s);

class FPGA_page : public FPGA_GUI
{
public:
FPGA_page(const string& sPageName, ExperimentsSheet* pSheet, unsigned page_id);
virtual ~FPGA_page();

void AddAvailableActions(std::vector<std::string>*);
void on_action(const std::string&);

static bool matchTitle(const std::string& s)
{
	return s.find("FPGA") != string::npos;
}

protected:

virtual bool RecalculateParameters();

protected:
GUI_string server;
GUI_int port;
GUI_string status_text;
GUI_double Latency;
};
