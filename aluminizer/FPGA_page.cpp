#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "FPGA_connection.h"
#include "FPGA_TCP.h"
#include "FPGA_page.h"
#include "ExperimentPage.h"
#include "AluminizerApp.h"

std::string getTcpStatusText(unsigned s)
{
	switch (s)
	{
	case QAbstractSocket::UnconnectedState:  return "Unconnected";
	case QAbstractSocket::HostLookupState:      return "HostLookup";
	case QAbstractSocket::ConnectingState:      return "Connecting";
	case QAbstractSocket::ConnectedState:    return "Connected";
	case QAbstractSocket::BoundState:        return "Bound";
	case QAbstractSocket::ClosingState:      return "Closing";
	case QAbstractSocket::ListeningState:    return "Listening";
	}

	return "";
}

FPGA_page::FPGA_page(const std::string& sPageName, ExperimentsSheet* pSheet, unsigned page_id) :
	FPGA_GUI(sPageName, pSheet, page_id),
	server("Server",           &m_TxtParams, "", &m_vParameters, RP_FLAG_READ_ONLY),
	port("Port",          &m_TxtParams, "0", &m_vParameters, RP_FLAG_READ_ONLY),
	status_text("Status",           &m_TxtParams, "Disconnected", &m_vParameters, RP_FLAG_READ_ONLY),
	Latency("Latency [ms]",     &m_TxtParams, "0", &m_vParameters, RP_FLAG_READ_ONLY)
{
	SafeRecalculateParameters();
}

FPGA_page::~FPGA_page()
{
}

bool FPGA_page::RecalculateParameters()
{
	bool bChanged = false;

	bChanged |= server.SetValue(theApp->fpga->server_name);
	bChanged |= port.SetValue(theApp->fpga->port);
	bChanged |= status_text.SetValue(getTcpStatusText(theApp->fpga->fpga_tcp->socket_state()));

	return bChanged;
}

void FPGA_page::AddAvailableActions(std::vector<std::string>* p)
{
	FPGA_GUI::AddAvailableActions(p);
	p->push_back("TEST LATENCY");
}

void FPGA_page::on_action(const std::string& s)
{
	if (s == "TEST LATENCY")
	{
		Latency.SetValue(theApp->fpga->measure_latency() * 1000);
		Latency.PostUpdateGUI();
		return;
	}

	FPGA_GUI::on_action(s);
}
