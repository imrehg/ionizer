#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <QTextStream>

#include "AluminizerApp.h"
#include "AluminizerPage.h"
#include "ExperimentsSheet.h"


AluminizerPage::AluminizerPage(ExperimentsSheet* pSheet, const std::string& title) :
	QWidget(&(pSheet->central_window)),
	title(title),
	m_pSheet(pSheet),
	id(-1)
{
	hide();
	cerr << "Adding page: " << PageTitle() << endl;
	iSheetPage = m_pSheet->AddPage(this);

	QObject::connect(this, SIGNAL(sig_update_actions()), m_pSheet, SLOT(UpdateButtons()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(sig_started(int)), m_pSheet, SLOT(slot_exp_started(int)), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(sig_finished(int)), m_pSheet, SLOT(slot_exp_finished(int)), Qt::QueuedConnection);
}


void AluminizerPage::on_select_page()
{
	m_pSheet->on_select_page(this);

	show();
}

void AluminizerPage::AddAvailableActions(std::vector<std::string>*)
{
}

void AluminizerPage::AddHelpAction(std::vector<std::string>* v)
{
	v->push_back("HELP");
}

void AluminizerPage::on_action(const std::string& s)
{
	if (s == "HELP")
		LaunchHelp(PageTitle());

}

void AluminizerPage::LaunchEditor(const std::string& fname)
{
	QString program = "notepad.exe";
	QStringList arguments;

	arguments << fname.c_str();

	procHelp = new QProcess(this);
	procHelp->start(program, arguments);
}

void AluminizerPage::LaunchHelp(const std::string& s)
{
	QString program = "notepad.exe";
	QStringList arguments;
	QString file_name = helpDir.absoluteFilePath(("help" + s + ".txt").c_str());

	//create file if it doesn't exist yet
	QFile f(file_name);

	if (!f.exists())
	{
		f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
		QTextStream out(&f);
		out << "*** " << PageTitle().c_str() << " ***\n\n";
		f.close();
	}

	arguments << file_name;

	procHelp = new QProcess(this);
	procHelp->start(program, arguments);
}

void AluminizerPage::PostUpdateActions()
{
	emit sig_update_actions();
}

void AluminizerPage::PostUpdateData()
{
	emit sig_update_data();
}
