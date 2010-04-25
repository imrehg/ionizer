#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <QSettings>
#include <QShowEvent>
#include <QSizePolicy>

#include "AluminizerApp.h"
#include "Experiment.h"
#include "ExperimentsSheet.h"
#include "AluminizerPage.h"
#include "ExperimentPage.h"
#include "TransitionPage.h"
#include "FPGA_TCP.h"

using namespace std;
//using namespace parameters;

int ExperimentsSheet::nPages = 0;

ExperimentsSheet::ExperimentsSheet() :
	scan_scheduler(0),
	current_page(0),
	pagesBar(this),
	central_window(this)
{
	setCentralWidget(&central_window);
//	resize(600, 800);

	central_grid = new QGridLayout(&central_window);

	addToolBar(Qt::LeftToolBarArea, &pagesBar);
	addToolBar(Qt::TopToolBarArea, &actionBar);

	pagesBar.setOrientation(Qt::Vertical);
	pagesBar.setMovable(false);
	pagesBar.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	actionBar.setOrientation(Qt::Horizontal);
	actionBar.setMovable(false);
}


ExperimentsSheet::~ExperimentsSheet()
{
}

//! Dispatch message originating from FPGA S2C...
void ExperimentsSheet::DispatchMsg(GbE_msg_exchange* eX, unsigned m)
{
	unsigned page_id = eX->reply(0);

	for (unsigned i = 0; i < pages.size(); i++)
	{
		FPGA_GUI* pPage = dynamic_cast<FPGA_GUI*>(pages[i]);

		if (pPage)
			if (pPage->get_page_id() == page_id)
				pPage->DispatchMsg(eX, m);
	}
}

void ExperimentsSheet::closeEvent(QCloseEvent *event)
{
	QSettings settings("NIST", "Aluminizer2");

	settings.setValue("main_window/geometry", saveGeometry());

	scan_scheduler.Stop();

	QCoreApplication::quit();

	QMainWindow::closeEvent(event);
}

void ExperimentsSheet::showEvent(QShowEvent * event )
{
	if (event->spontaneous() == false)
	{
		QSettings settings("NIST", "Aluminizer2");
		restoreGeometry(settings.value("main_window/geometry").toByteArray());
	}

	QMainWindow::showEvent(event);
}

void ExperimentsSheet::slot_exp_started(int iPage)
{
	QPalette pal_run;

	pal_run.setColor(QPalette::ButtonText, QColor(200, 0, 0));
	buttons[iPage]->setPalette(pal_run);
}

void ExperimentsSheet::slot_exp_finished(int iPage)
{
	QPalette pal_stop;

	pal_stop.setColor(QPalette::ButtonText, QColor(0, 0, 0));
	buttons[iPage]->setPalette(pal_stop);
}


void ExperimentsSheet::UpdateButtons()
{
	if (current_page >= pages.size())
		return;

	actionBar.clear();
	vActions.clear();

	pages[current_page]->AddAvailableActions(&vActions);
	pages[current_page]->AddHelpAction(&vActions);

//   assert(vActions.size() <= 10);
	vQActions.resize(vActions.size());

	for (size_t i = 0; i < vActions.size(); i++)
	{
		QAction* qa = actionBar.addAction(vActions[i].c_str());
		vQActions[i] = qa;

		QByteArray ba(2, 0);
		ba[0] = current_page;
		ba[1] = i;
		qa->setData(ba);

		if (vActions[i] == "UPDATE FPGA")
			qa->setIcon(QIcon(appDir.filePath("icons/chip-32.png")));

		if (vActions[i] == "PRINT")
			qa->setIcon(QIcon(appDir.filePath("icons/print.png")));

		if (vActions[i] == "RUN")
		{
			qa->setIcon(QIcon(appDir.filePath("icons/next-32.png")));
			qa->setShortcut(Qt::Key_Enter);
		}

		if (vActions[i] == "HELP")
		{
			actionBar.insertSeparator(qa);
			qa->setIcon(QIcon(appDir.filePath("icons/help-32.png")));
			qa->setShortcut(Qt::Key_F1);
		}

		if (vActions[i] == "STOP")
		{
			qa->setIcon(QIcon(appDir.filePath("icons/stop-32.png")));
			qa->setShortcut(Qt::Key_Escape);
		}

		if (vActions[i] == "DEBUG")
		{
			qa->setIcon(QIcon(appDir.filePath("icons/debug.png")));
			qa->setToolTip("Debug");
		}

		if (vActions[i] == "DUMP")
			qa->setIcon(QIcon(appDir.filePath("icons/delete-32.png")));

		QObject::connect(qa, SIGNAL(triggered(bool)), SLOT(on_action()), Qt::AutoConnection);
	}
}



void ExperimentsSheet::on_action()
{
	QAction *action = qobject_cast<QAction *>(sender());

	QByteArray ba = action->data().toByteArray();

	unsigned uPage = ba.at(0);
	unsigned uAction = ba.at(1);

	pages[uPage]->on_action(vActions[uAction]);
	UpdateButtons();
}

void ExperimentsSheet::on_select_page(AluminizerPage* page)
{
	size_t new_page = std::find(pages.begin(), pages.end(), page) - pages.begin();

	if (new_page != current_page)
	{
		if (new_page < pages.size())
		{
			if (current_page < pages.size())
			{
				buttons[current_page]->setDown(false);
				buttons[new_page]->setDown(true);

				pages[current_page]->hide();
				central_grid->removeWidget(pages[current_page]);
			}

			pages[new_page]->show();
			central_grid->addWidget(pages[new_page]);

			current_page = new_page;
		}
	}

	pages[current_page]->PostUpdateData();
	UpdateButtons();
}


unsigned ExperimentsSheet::AddPage(AluminizerPage* page)
{
	pages.push_back(page);
	return pages.size() - 1;
}

void ExperimentsSheet::InitPages()
{
	for (size_t i = 0; i < pages.size(); i++)
	{
		QAction* qa = pagesBar.addAction(pages[i]->PageTitle().c_str());
		buttons.push_back(dynamic_cast<QToolButton *>(pagesBar.widgetForAction(qa)));


		if (pages[i]->PageTitle() == ExperimentPage::pGlobals->GetIonXtal())
			buttons.back()->setStyleSheet("color: rgb(255, 140, 0)"); //orange
		else if (dynamic_cast<CalibrationPage*>(pages[i]) != 0)
			buttons.back()->setStyleSheet("color: rgb(50, 150, 50)");

		QObject::connect(qa, SIGNAL(triggered(bool)), pages[i], SLOT(on_select_page()), Qt::AutoConnection);
		pages[i]->InitWindow();


	}

	current_page = pages.size();
	on_select_page(pages[0]);
}

void ExperimentsSheet::RunExperiment(const std::string& name)
{
	ExperimentBase* e = FindExperiment(name);

	if (e)
		scan_scheduler.StartExperiment(e, 0);
}

ExperimentBase* ExperimentsSheet::FindExperiment(const std::string& name)
{
	return dynamic_cast<ExperimentBase*>(FindPage(name));
}

AluminizerPage* ExperimentsSheet::FindPage(const std::string& name)
{
	for (size_t i = 0; i < pages.size(); i++)
		if (AluminizerPage * p = dynamic_cast<AluminizerPage*>(pages[i]))
			if (name == p->PageTitle())
				return p;

	cerr << "[ExperimentsSheet::FindPage] can't find: " << name << endl;
	return 0;
}

size_t ExperimentsSheet::NumPages() const
{
	return pages.size();
}

AluminizerPage* ExperimentsSheet::GetPage(size_t i)
{
	return dynamic_cast<AluminizerPage*>(pages.at(i));
}

/*
   void ExperimentsSheet::OnClose()
   {
   if(scan_scheduler.Stop())
   {
      PrepareCloseWindow();
      DestroyWindow();
   }
   else
   {
      Sleep(100);
      PostMessage(WM_CLOSE, 0, 0);
   }
   }

 */


void ExperimentsSheet::SaveAllParameters(const std::string OutputDirectory)
{
	for (size_t i = 0; i < pages.size(); i++)
		if (AluminizerPage * p = dynamic_cast<AluminizerPage*>(pages[i]))
			p->SaveParams(OutputDirectory);
}

void ExperimentsSheet::SetStatusBarText(const std::string&)
{
}

/*
   ExperimentBase* ExperimentsSheet::GetActiveExperiment() const
   {
   ExperimentBase* p = dynamic_cast<ExperimentBase*>(pages[GetActiveIndex()]);
   return p;
   }

 */


template<class T> bool ExperimentsSheet::LinkParamSource(const std::string& pname, T* pGUI)
{
	for (unsigned i = 0; i < pages.size(); i++)
	{
		TransitionPage* pX = dynamic_cast<TransitionPage*>(pages[i]);

		if (pX)
			if (pX->LinkPulseTo(pGUI, pname))
			{
				//cout << "Linking paramater " << pname << " to page: " << pX->GetName() << endl;
				return true;
			}
	}

	return false;
}

template bool ExperimentsSheet::LinkParamSource(const std::string&, GUI_ttl*);
template bool ExperimentsSheet::LinkParamSource(const std::string&, GUI_dds*);
template bool ExperimentsSheet::LinkParamSource(const std::string&, GUI_double*);
