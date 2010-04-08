#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "AluminizerApp.h"
#include "FPGA_connection.h"
#include "CommonExperimentsSheet.h"
#include "MgAlExperimentsSheet.h"
#include "InputParameters.h"

#include <QTimer>

using namespace std;

bool g_StopAll = false;


#ifdef CONNECT_TO_SIM
const char sFPGA_Address[] = "127.0.0.1";
#else
const char sFPGA_Address[] = "192.168.1.10";
#endif

// The one and only AluminizerApp object
AluminizerApp* theApp;
QDir appDir, helpDir, soundDir;
CommonExperimentsSheet* gSheet = 0;

AluminizerApp::AluminizerApp(int & argc, char ** argv) :
	QApplication(argc, argv),
	wd(),
	fpga(new FPGA_connection(sFPGA_Address, 6007)),
	m_pExperimentsSheet(0)
{
	appDir = QDir::current();
	helpDir = QDir::current();
	soundDir = QDir::current();
	soundDir.cd("sounds");

	cout << "sound Dir = " << string(soundDir.absoluteFilePath("smb3_1-up.wav").toAscii()) << endl;
	char default_dir[] = "c:/data/clock";
	char* app_dir = getenv("ALUMINIZER_DIR");

	if (app_dir == 0)
		app_dir = default_dir;

	wd.setPath(app_dir);
	wd.mkpath(wd.absolutePath());
	wd.setCurrent(wd.absolutePath());


	QObject::connect(fpga, SIGNAL(FPGA_ready()), this, SLOT(FPGA_ready()));

#ifndef CONNECT_TO_SIM
	fpga->start(QThread::HighestPriority); //QThread::TimeCriticalPriority);
#else
	fpga->start(QThread::TimeCriticalPriority);
#endif

	thread()->setPriority(QThread::HighestPriority);
}

AluminizerApp::~AluminizerApp()
{
	delete m_pExperimentsSheet;
	delete fpga;
}

void AluminizerApp::FPGA_ready()
{
	FPGA_GUI::pFPGA = fpga;

	m_pExperimentsSheet = new MgAlExperimentsSheet();
	gSheet = m_pExperimentsSheet;

	//had to add this delay, because m_vParameters in 3P0 lock (-) was getting hosed
	//PlaceWindows() kept crashing.  This is on Qt Creator under Linux.
	SleepHelper::msleep(100);
	InitPages();
}

void AluminizerApp::InitPages()
{
	m_pExperimentsSheet->InitPages();

	unsigned font_size = m_pExperimentsSheet->m_GlobalsPage.FontSize;

	setFont(QFont("Helvetica", font_size, QFont::Bold));
	m_pExperimentsSheet->setFont(QFont("Helvetica", font_size, QFont::Bold));
	m_pExperimentsSheet->show();
	m_pExperimentsSheet->scan_scheduler.start(QThread::HighestPriority);
}

/*
   void AluminizerApp::showDebugLog()
   {
   QFont font;
   font.setStyleHint(QFont::TypeWriter);
   QString name = font.defaultFamily();
   textEdt.setFont(QFont(name));

   textEdt.setText(debug_txt);
   textEdt.resize(600,800);
   textEdt.show();
   }
 */

