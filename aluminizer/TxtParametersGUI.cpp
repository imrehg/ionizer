#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include <QMutexLocker>

#include "TxtParametersGUI.h"
#include "InputParametersGUI.h"
#include "ExperimentsSheet.h"

//#include "physics.h"
//#include "Transition.h"

#include "DDS_Pulse_Widget.h"

using namespace std;

unsigned TxtParametersGUI::gridV_spacing = -2;
unsigned TxtParametersGUI::default_num_columns = 4;

//constructor for text file input
TxtParametersGUI::TxtParametersGUI(ExperimentsSheet* pSheet, const std::string& title) :
	AluminizerPage(pSheet, title),
	title(title),
	m_bSettingText(false),
	GUI_Revision(0),
	DataRevision(1),
	csRecalculating(title + "::csRecalculating"),
	top_layout(this),
	bIgnoreGUISignals(false),
	page_lock(QReadWriteLock::Recursive)
{
	QObject::connect(this, SIGNAL(sig_update_data()), this, SLOT(UpdateData()), Qt::QueuedConnection);
	top_layout.addLayout(&grid);
}

TxtParametersGUI::~TxtParametersGUI()
{
	while (!m_vAllocatedParams.empty())
	{
		vector<ParameterGUI_Base*>::iterator it;
		it = std::find(m_vParameters.begin(), m_vParameters.end(), m_vAllocatedParams.back());
		if (it != m_vParameters.end())
			m_vParameters.erase(it);

		delete m_vAllocatedParams.back();
		m_vAllocatedParams.pop_back();
	}
}


bool TxtParametersGUI::UpdateData()
{
	CriticalSectionOwner cso(&csRecalculating, 0);

	QWriteLocker lock1(&page_lock);
	QMutexLocker lock2(&mutexPU);

	unsigned OldRevision = DataRevision;

	try
	{
		int max_recalculations = 10;

		while (SafeRecalculateParameters() && --max_recalculations)
			DataRevision++;

		if (!max_recalculations)
			cout << "The maximum number of page recalculations for " << Title() << " has been exceeded!" << endl;
	}
	catch (Uninitialized u)
	{
		cout << "\"" << u.Name << "\" is uninitialized." << endl;
	}

	SetDialogContents();

//	m_pSheet->UpdateButtons();

	page_updated.wakeAll();

	return OldRevision != DataRevision;
}

void TxtParametersGUI::OnUpdateData()
{
	SetDialogContents();
}

void TxtParametersGUI::OnRun()
{
	GetDialogContents();
}

void TxtParametersGUI::SetDialogContents()
{
	if (GUI_Revision > DataRevision)
		return;

	m_bSettingText = true;

	for (size_t i = 0; i < m_vParameters.size(); i++)
	{
		try
		{
			m_vParameters[i]->UpdateGUI();
		}
		catch (Uninitialized u)
		{
			cout << u << endl;
		}
		catch (runtime_error e)
		{
			cout << "Run-time error:" << endl;
			cout << e.what() << endl;
		}
	}

	m_bSettingText = false;

	GUI_Revision = DataRevision;
}

ParameterGUI_Base* TxtParametersGUI::FindParameter(const std::string& name) const
{
	vector<ParameterGUI_Base*>::const_iterator cit;

	for (cit = m_vParameters.begin(); cit != m_vParameters.end(); cit++)
		if ((*cit)->GetName() == name)
			return *cit;

	return 0;
}

ParameterGUI_Base* TxtParametersGUI::FindFPGAParameter(const std::string& name) const
{
	vector<ParameterGUI_Base*>::const_iterator cit;

	for (cit = m_vParameters.begin(); cit != m_vParameters.end(); cit++)
		if ((*cit)->get_fpga_name() == name)
			return *cit;

	return 0;
}

ParameterGUI_Base* TxtParametersGUI::FindParameterContaining(const std::string& s1, const std::string& s2) const
{
	vector<ParameterGUI_Base*>::const_iterator cit;

	for (cit = m_vParameters.begin(); cit != m_vParameters.end(); cit++)
		if ((*cit)->GetName().find(s1) != string::npos)
			if ((*cit)->GetName().find(s2) != string::npos)
				return *cit;

	return 0;
}

ParameterGUI_Base* TxtParametersGUI::FindParameterContaining(const std::string& name) const
{
	vector<ParameterGUI_Base*>::const_iterator cit;

	for (cit = m_vParameters.begin(); cit != m_vParameters.end(); cit++)
		if ((*cit)->GetName().find(name) != string::npos)
			return *cit;

	return 0;
}

void TxtParametersGUI::RemoveParameter(ParameterGUI_Base* p)
{
//	if(GetSafeHwnd())
//		throw "Can't delete parameters from an existing window.";

	vector<ParameterGUI_Base*>::iterator it;

	it = std::find(m_vParameters.begin(), m_vParameters.end(), p);

	if (it != m_vParameters.end())
		m_vParameters.erase(it);
}

bool TxtParametersGUI::ChangeParameterName(ParameterGUI_Base* p, const std::string& name)
{
	vector<ParameterGUI_Base*>::iterator it;

	it = std::find(m_vParameters.begin(), m_vParameters.end(), p);
	if (it != m_vParameters.end())
		if ((*it)->GetName() != name)
		{
			(*it)->SetName(name);
			return true;
		}

	return false;
}


bool TxtParametersGUI::GetDialogContents(ParameterGUI_Base* p)
{
//	QWriteLocker lock(&page_lock);

	bool Changed = false;

	if (page_lock.tryLockForWrite())
	{
		if (GUI_Revision > DataRevision)
		{
			if (p)
			{
				if (!p->IsReadOnly() )
					Changed |= p->UpdateFromGUI();
			}
			else
			{
				for (size_t i = 0; i < m_vParameters.size(); i++)
					if (!m_vParameters[i]->IsReadOnly() )
						Changed |= m_vParameters[i]->UpdateFromGUI();
			}

			DataRevision = GUI_Revision;

			UpdateData();
		}

		page_lock.unlock();
	}

	return Changed;
}


void TxtParametersGUI::InitWindow()
{
	AddParams();

	//place the controls
	PlaceWindows();

	PostCreateGUI();

	//set the contents
	UpdateData();
}

unsigned TxtParametersGUI::num_columns()
{
	return default_num_columns;
}

void TxtParametersGUI::PlaceWindows()
{
	//grid coordinates of widgets
	unsigned r = 0;   //row
	unsigned c = 0;   //column

	//grid.setVerticalSpacing(gridV_spacing);
	grid.setSpacing(gridV_spacing);

	unsigned columns_in_row = num_columns();

	for (size_t i = 0; i < m_vParameters.size(); i++)
	{
		bool bPlaceOutside = false;

		ParameterGUI_Base* pParam = m_vParameters[i];

		//don't show parameters that are linked from somewhere else
		if (pParam->isLinked() && pParam->canHide())
			continue;

		unsigned input_columns_needed = pParam->getNumGUIColumns();
		unsigned label_columns_needed = 0;
		unsigned rows_needed = pParam->getNumGUIRows();

		//bPacked = false means the widget will use an entire row
		//and the label uses two columns
		if (!pParam->IsPacked() || pParam->new_row)
		{
			label_columns_needed = 2;
			c = num_columns();
		}

		if (pParam->getFlags() & RP_FLAG_PLACE_EAST)
		{
			label_columns_needed = 0;
			input_columns_needed = 0;
			bPlaceOutside = true;
		}

		if (pParam->HasLabel() && !bPlaceOutside)
			label_columns_needed = 1;

		unsigned columns_needed = label_columns_needed + input_columns_needed;
		//move to next row if there's not enough space
		if ( (c + columns_needed) > columns_in_row && !bPlaceOutside )
		{
			c = 0;
			r = grid.rowCount();

			columns_in_row = num_columns();

			if (pParam->getFlags() & RP_FLAG_3_COLUMN)
				columns_in_row = std::max<unsigned>(6, columns_in_row);

			if (pParam->getFlags() & RP_FLAG_4_COLUMN)
				columns_in_row = std::max<unsigned>(8, columns_in_row);

			if (pParam->getFlags() & RP_FLAG_5_COLUMN)
				columns_in_row = std::max<unsigned>(10, columns_in_row);

			if (pParam->getFlags() & RP_FLAG_6_COLUMN)
				columns_in_row = std::max<unsigned>(12, columns_in_row);
		}

		if (c == 0 && !bPlaceOutside )
		{
			//hgrids.push_back(new QHBoxLayout());
			//grid.addLayout(hgrids.back());
		}

		if (pParam->HasLabel())
		{
			pParam->MakeLabelControl(this, 0);

			if (pParam->label_width)
				pParam->label_widget()->setMinimumSize(pParam->label_width, 0);

			grid.addWidget(pParam->label_widget(), r, c, 1, label_columns_needed);
			c += label_columns_needed;
		}


		pParam->MakeInputControl(this, 0);

		if ( !bPlaceOutside )
		{
			pParam->input_widget()->setMinimumSize(pParam->input_width, pParam->min_height());
			//grid.setStretch(grid.count()-1, pParam->getVstretch());

			if (pParam->IsPacked())
			{
				grid.addWidget(pParam->input_widget(), r, c, rows_needed, input_columns_needed);
				c += input_columns_needed;
			}
			else
			{
				grid.addWidget(pParam->input_widget(), r, c, 1, -1);
				//hgrids.back()->addWidget(pParam->input_widget());
				c = columns_in_row;
			}
		}
		else
			top_layout.addWidget(pParam->input_widget());

		pParam->UpdateGUI();

		QObject::connect(pParam, SIGNAL(GUI_user_value_change(ParameterGUI_Base*)), this, SLOT(on_GUI_user_change(ParameterGUI_Base*)), Qt::AutoConnection);
		QObject::connect(pParam, SIGNAL(GUI_button_was_pressed(ParameterGUI_Base*)), this, SLOT(on_GUI_button_was_pressed(ParameterGUI_Base*)), Qt::AutoConnection);
	}

	//add lower spacer
	if (needsBottomSpacer())
		grid.addItem(new QSpacerItem(200, 2000, QSizePolicy::Maximum, QSizePolicy::Maximum), r + 1, 0, 1, num_columns() - 2);

	//grid.addSpacerItem(new QSpacerItem(2000, 2000, QSizePolicy::Minimum, QSizePolicy::Maximum));

	//add left spacer
	//grid.addSpacerItem(new QSpacerItem(600, 600, QSizePolicy::Maximum, QSizePolicy::Maximum), 0, num_columns(), r+1, 1);
}

void TxtParametersGUI::on_GUI_user_change(ParameterGUI_Base* p)
{
	if (!bIgnoreGUISignals)
		OnEditChange(p);
}

void TxtParametersGUI::on_GUI_button_was_pressed(ParameterGUI_Base* p)
{
	if (!bIgnoreGUISignals)
		OnGUIButton(p);
}

void TxtParametersGUI::OnClose()
{
	GetDialogContents();
}

void TxtParametersGUI::OnEditChange(ParameterGUI_Base* p)
{
	if (!m_bSettingText && GUI_Revision >= DataRevision)
	{
		GUI_Revision++;
		GetDialogContents(p);
	}
}

bool TxtParametersGUI::ReplaceParameter(ParameterGUI_Base* pOld, ParameterGUI_Base* pNew)
{
	assert(pOld);
	assert(pNew);

	vector<ParameterGUI_Base*>::iterator it = std::find(m_vParameters.begin(), m_vParameters.end(), pOld);

	if (it != m_vParameters.end())
	{
		*it = pNew;
		return true;
	}

	return false;
}

bool TxtParametersGUI::SafeRecalculateParameters()
{
	bool Changed = false;

	try
	{
		debugQ("[TxtParametersGUI::RecalculateParameters]", Title());
		//crashed here
		Changed = RecalculateParameters();
	}
	catch (Uninitialized u)
	{
		cout << u << endl;
	}
	catch (runtime_error e)
	{
		cout << "Run-time error:" << endl;
		cout << e.what() << endl;
	}

	return Changed;
}

