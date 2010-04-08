#pragma once

class CommonExperimentsSheet;
class FPGA_connection;

class SleepHelper : public QThread
{
public:
static void msleep(int ms)
{
	QThread::msleep(ms);
}
};

class AluminizerApp : public QApplication
{
Q_OBJECT

public:
AluminizerApp(int & argc, char ** argv);
~AluminizerApp();

public slots:
void FPGA_ready();
void InitPages();


private:
QDir wd;

public:
FPGA_connection* fpga;

CommonExperimentsSheet* m_pExperimentsSheet;
QFont font;
QTextEdit textEdt;
};

extern AluminizerApp* theApp;
extern bool g_StopAll;
extern QDir appDir;
extern QDir helpDir;
extern QDir soundDir;
extern CommonExperimentsSheet* gSheet;
