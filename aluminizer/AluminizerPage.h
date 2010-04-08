#pragma once

class ExperimentsSheet;

class AluminizerPage : public QWidget
{
Q_OBJECT

public:
AluminizerPage(ExperimentsSheet* pSheet, const std::string& title);
const std::string& PageTitle() const
{
	return title;
}

void SetID(int i)
{
	id = i;
}
int GetID()
{
	return id;
}

virtual void InitWindow() = 0;

virtual void AddAvailableActions(std::vector<std::string>*);
virtual void AddHelpAction(std::vector<std::string>*);
virtual void on_action(const std::string&);

void PostUpdateActions();
void PostUpdateData();
void LaunchHelp(const std::string& s);
void LaunchEditor(const std::string& fname);

virtual void SaveParams(const std::string& OutputDirectory) = 0;

signals:
void sig_update_actions();
void sig_update_data();

void sig_started(int iSheetPage);
void sig_finished(int iSheetPage);

public slots:
void on_select_page();

protected:
std::string title;
ExperimentsSheet* m_pSheet;
unsigned iSheetPage;
int id;

QProcess* procHelp;
};
