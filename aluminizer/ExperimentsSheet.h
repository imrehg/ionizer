#pragma once

#include "Experiment.h"
#include "AluminizerPage.h"
#include "ScanScheduler.h"
#include "FPGA_page.h"
#include "SwitchPanelDialog.h"

class GUI_dds;
class GbE_msg_exchange;

class ExperimentsSheet : public QMainWindow
{
Q_OBJECT

public:
ExperimentsSheet();
virtual ~ExperimentsSheet();

virtual void closeEvent(QCloseEvent * event );
virtual void showEvent(QShowEvent * event );

unsigned AddPage(AluminizerPage* page);
void InitPages();

//! Dispatch message originating from FPGA S2C...
void DispatchMsg(GbE_msg_exchange* eX, unsigned m);

ExperimentBase* FindExperiment(const std::string& name);
void RunExperiment(const std::string& name);

AluminizerPage* FindPage(const std::string& name);
void SaveAllParameters(const std::string OutputDirectory);

size_t NumPages() const;
AluminizerPage* GetPage(size_t i);

void SetStatusBarText(const std::string& s);

void on_select_page(AluminizerPage*);

template <class T> bool LinkParamSource(const std::string& name, T* pGUI);

public slots:
void UpdateButtons();

void on_action();
void slot_exp_started(int iPage);
void slot_exp_finished(int iPage);

public:
ScanScheduler scan_scheduler;
const static unsigned id0 = 500;
static int nPages;
size_t current_page;

protected:

std::vector<AluminizerPage*> pages;
std::vector<QToolButton*> buttons;

std::string status_text;

QPalette palUnselected;
QPalette palSelected;

protected:
//	ExperimentBase* GetActiveExperiment() const;

void OnCancel();

virtual void PrepareCloseWindow()
{
};

public:
QToolBar pagesBar;
QToolBar actionBar;
QWidget central_window;

QGridLayout* central_grid;

std::vector<std::string> vActions;
std::vector<QAction*> vQActions;
};

