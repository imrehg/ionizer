#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "FPGA_GUI.h"
#include "FPGA_connection.h"
#include "FPGA_TCP.h"
#include "ExperimentsSheet.h"

FPGA_connection*  FPGA_GUI::pFPGA(0);

FPGA_GUI::FPGA_GUI(const string& sPageName,
                   ExperimentsSheet* pSheet,
                   unsigned page_id) :
	ParamsPage(pSheet, sPageName),
	page_id(page_id),
	need_init_remote_actions(true)
{
}

void FPGA_GUI::AddParams()
{
	ParamsPage::AddParams();

	if (page_id < 100)
		AddRemoteParams();

	//  SafeRecalculateParameters();
}

void FPGA_GUI::setupRemoteParam(GUI_double*)
{
}

void FPGA_GUI::AddRemoteParams()
{
	unsigned nParams = pFPGA->NumRemoteParams(get_page_id());

	for (unsigned i = 0; i < nParams; i++)
	{
		string s = pFPGA->DefineRemoteParam(get_page_id(), i);

//		cout << "[AddParam]  " << s << endl;

		unsigned type = 0;

		ParameterGUI_Base* pParam = 0;

		sscanf(s.c_str(), "type=%u", &type);

		size_t spaceLoc = s.find(' ');
		string s2 = s.substr(spaceLoc + 1);

		if (type == RP_UNSIGNED)
			pParam = new GUI_unsigned(s2, &m_TxtParams, &m_vParameters);

		if (type == RP_INT)
			pParam = new GUI_int(s2, &m_TxtParams, &m_vParameters);

		if (type == RP_STRING)
			pParam = new GUI_string(s2, &m_TxtParams, &m_vParameters);

		if (type == RP_LCD)
			pParam = new GUI_doubleLCD(s2, &m_TxtParams, &m_vParameters);

		if (type == RP_DOUBLE)
		{
			GUI_double* pGUI = new GUI_double(s2, &m_TxtParams, &m_vParameters);
			pParam = pGUI;

			pGUI->setPrecision(4);

			if ( !(pGUI->getFlags() & RP_FLAG_NOLINK) )
				m_pSheet->LinkParamSource(pGUI->GetName(), pGUI);

			setupRemoteParam(pGUI);
		}

		if (type == RP_BOOL)
			pParam = new GUI_bool(s2, &m_TxtParams, &m_vParameters);

		if (type == RP_MATRIX)
			pParam = new GUI_matrix(s2, &m_TxtParams, &m_vParameters);

		if (type == RP_TTL_PULSE)
		{
			GUI_ttl* pPulseGUI = new GUI_ttl(s2, &m_TxtParams, &m_vParameters);
			pParam = pPulseGUI;

			if ( !(pPulseGUI->getFlags() & RP_FLAG_NOLINK) )
				m_pSheet->LinkParamSource(pPulseGUI->GetName(), pPulseGUI);
		}

		if (type == RP_DDS_PULSE)
		{
			GUI_dds* pPulseGUI = new GUI_dds(s2, &m_TxtParams, &m_vParameters);
			pParam = pPulseGUI;

			if ( !(pPulseGUI->getFlags() & RP_FLAG_NOLINK) )
				m_pSheet->LinkParamSource(pPulseGUI->GetName(), pPulseGUI);
		}

		if (pParam)
		{
			pParam->set_fpga_id(i);
			string sToolTip = pFPGA->explainRemoteParam(get_page_id(), i);
			pParam->setToolTip(sToolTip);
			FPGA_params.push_back(pParam);

			if (pParam->getFlags() & RP_FLAG_FPGA_UPDATES)
				pParam->SetValueFromString(pFPGA->getParamValueString(get_page_id(), i));
		}
	}

	if (needInitFPGA())
		pFPGA->SendParams(page_id, FPGA_params);
}

//! Dispatch message originating from FPGA S2C...
void FPGA_GUI::DispatchMsg(GbE_msg_exchange* eX, unsigned)
{
	GbE_msg* msg_in = eX->get_msg_in();
	unsigned page_id = msg_in->extractU(0);

	assert(page_id == get_page_id());

	unsigned num_updates = msg_in->extractU(1);

	const char* p_name = msg_in->extractSC(2);
	const char* p_end = msg_in->extractSC(MSG_STD_PAYLOAD_SIZE);

	for (unsigned i = 0; i < num_updates; i++)
	{
		if (p_name >= p_end)
			throw runtime_error("update_params");

		const char* p_value = p_name + strlen(p_name) + 1;

		if (p_value >= p_end)
			throw runtime_error("update_params");

		ParameterGUI_Base* pParam = FindParameter(p_name);
		assert(pParam != 0);
		pParam->SetValueFromString(p_value);
		pParam->PostUpdateGUI();

		//      cout << "[" << title << "] " << p_name << " = " << p_value << endl;

		p_name = p_value + strlen(p_value) + 1;
	}
}


void FPGA_GUI::AddAvailableActions(std::vector<std::string>* v)
{
	ParamsPage::AddAvailableActions(v);

//	if(FPGA_params.size() > 0)
	v->push_back("UPDATE FPGA");

	if (page_id < 100 && need_init_remote_actions)
	{
		need_init_remote_actions = false;

		unsigned n = pFPGA->getNumRemoteActions(page_id);

		for (unsigned i = 0; i < n; i++)
		{
			string s = pFPGA->getRemoteActionName(page_id, i);
			remote_actions.push_back(s);
		}
	}

	for (unsigned i = 0; i < remote_actions.size(); i++)
		v->push_back(remote_actions[i]);

}

void FPGA_GUI::OnEditChange(ParameterGUI_Base* p)
{
	if (p->getFlags() & RP_FLAG_UPDATE_IMMEDIATE)
		pFPGA->SendParam(page_id, p);

	ParamsPage::OnEditChange(p);
}

void FPGA_GUI::on_action(const std::string& s)
{
	for (size_t i = 0; i < remote_actions.size(); i++)
	{
		if (s == remote_actions[i])
		{
			pFPGA->callRemoteAction(page_id, i);
			return;
		}
	}

	if (s == "UPDATE FPGA")
		pFPGA->SendParams(page_id, FPGA_params);
	else
		ParamsPage::on_action(s);
}

unsigned FPGA_GUI::plot_columns(unsigned /* nPlots */)
{
	return 4;
}


void FPGA_GUI::AddPagePlots()
{
	unsigned nPlots = pFPGA->numPagePlots(page_id);


	if (nPlots == page_plots.size())
		return;

	unsigned nCol = plot_columns(nPlots);
	unsigned nRow = ceil(nPlots / (double)nCol);

	unsigned k=0;

	for (unsigned j = 0; j < nRow; j++)
	{
		hgrids.push_back(new QHBoxLayout());
		grid.addLayout(hgrids.back(), grid.rowCount(), 0, 1, -1);

		for (unsigned i = 0; i < nCol; i++)
		{
			page_plots.push_back(new histogram_plot(this, "", "", pFPGA->getPlotLabel(page_id, k)));
			hgrids.back()->addWidget(page_plots.back());
			page_plots.back()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			
			if(j+1 < nRow)
				page_plots.back()->disableXaxis();

			page_plots.back()->show();

			k++;
		}
	}
}

void FPGA_GUI::update_plots(double min_delay)
{
	if (page_plots.size() == 0)
		AddPagePlots();

	if (min_delay + last_plot_update > CurrentTime_s())
		return;

	for (unsigned i = 0; i < page_plots.size(); i++)
	{
		if (page_plots[i])
		{
			last_plot_update = CurrentTime_s();

			valarray<double> pp = pFPGA->getPagePlotData(page_id, i);

			page_plots[i]->barPlot(pp);
		}
	}
}