#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

//#include  "../fpgart/fpgart.h"
#include "GlobalsPage.h"
#include "TransitionPage.h"
#include "MgPage.h"

#include <QMessageBox>
#include "FPGA_GUI.h"
#include "FPGA_connection.h"

using namespace std;


//return whether or not the specified thing should be debuggged
bool debugQ(const std::string& sCategory, const std::string& sMsg)
{
	std::string debug_stuff;

//	debug_stuff.append("[TransitionPage::RecalculateParameters]");
//	debug_stuff.append("[TxtParametersGUI::RecalculateParameters]");

	bool b = debug_stuff.find(sCategory) != string::npos;

	if (b && sMsg.length())
		cout << sCategory << " -- " << sMsg << endl;

	return b;
}

//construct the Experiment object and initialize variables
GlobalsPage::GlobalsPage(const std::string& sPageName, ExperimentsSheet* pSheet) :
	ParamsPage(pSheet, sPageName),
	FontSize("Font size",              &m_TxtParams, "9", &m_vParameters),
	WidgetSpacing("Widget spacing (V)",           &m_TxtParams, "2", &m_vParameters),
	NumColumns("Columns [#]",            &m_TxtParams, "2", &m_vParameters),
	HistogramSize("Histogram size",               &m_TxtParams, "32", &m_vParameters),
	ReplotModulo("Replot modulo",             &m_TxtParams, "1",   &m_vParameters),
	TimeSlice("Time slice [ms]",              &m_TxtParams, "2000",   &m_vParameters),
	PlotLineWidth("Plot line width [#]",          &m_TxtParams, "3", &m_vParameters),
	ScatterScan("Scatter scan",           &m_TxtParams, "false", &m_vParameters),
	DDS0Hz("DDS 0 Hz",               &m_TxtParams, "false", &m_vParameters),
	SavePlotType("Save plots as", "None\nPDF\nSVG\n",     &m_TxtParams, "None", &m_vParameters),
	IonXtal("Ion crystal", "", &m_TxtParams, "Mg", &m_vParameters),
	recordCameraXY("Record camera XY", &m_TxtParams, "0", &m_vParameters),
	xpos("xpos", &m_TxtParams, "0"),
	ypos("ypos", &m_TxtParams, "0"),
	startup_IonXtal(IonXtal.Value())
{
	recordCameraXY.setToolTip("Save X/Y data from webcamQt in scan logs.");
	ExperimentPage::pGlobals = this;

	SafeRecalculateParameters();

	gridV_spacing = WidgetSpacing;
	default_num_columns = NumColumns * 2;
}

GlobalsPage::~GlobalsPage()
{

}

void GlobalsPage::PostCreateGUI()
{
	for(unsigned i=0; i<m_pSheet->NumPages(); i++)
	{
		MgPage* pPage = dynamic_cast<MgPage*>(m_pSheet->GetPage(i));

		if(pPage)
			IonXtal.AddChoice(pPage->PageTitle());
	}
}

unsigned GlobalsPage::NumAl()
{
	return numOccurences(GetIonXtal(), "Al");
}

unsigned GlobalsPage::NumMg()
{
	return numOccurences(GetIonXtal(), "Mg");
}

std::string GlobalsPage::GetIonXtal()
{
	return IonXtal.Value();
}

void GlobalsPage::SetIonXtal(const std::string s)
{
	cout << "IonXtal = " << s << endl;

	IonXtal.SetValue(s);
}

void GlobalsPage::RememberWindowPos()
{
	/*
	   RECT rWnd;
	   m_pSheet->GetWindowRect(&rWnd);

	   xpos.SetValue(rWnd.left);
	   ypos.SetValue(rWnd.top);
	 */
}

bool GlobalsPage::RecalculateParameters()
{
	bool Changed = false;

	m_pSheet->scan_scheduler.SetTimeSlice(static_cast<unsigned>(TimeSlice));
	FPGA_GUI::pFPGA->setIonXtal(GetIonXtal());

	if (startup_IonXtal != GetIonXtal())
	{
		QMessageBox msgBox(QMessageBox::Information,
		                   "Ion configuration change",
		                   "Please restart this program.",
		                   QMessageBox::Ok);

		msgBox.exec();
	}

	return Changed;
}


