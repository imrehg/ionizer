#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "InputParametersGUI.h"

using namespace std;

std::string extractName(const std::string& sIni)
{
	size_t locName = sIni.find("name=");

	if (locName != std::string::npos)
		return sIni.substr(locName + 5);
	else
		return "";
}

QWidget* GUI_combo::MakeInputControl(QWidget* parent, unsigned id)
{
	InputParameterGUI<QComboBox, std::string>::MakeInputControl(parent, id);

	int iSelect = -1;

	for (size_t i = 0; i < choices.size(); i++)
	{
		if (Value() == choices[i])
			iSelect = i;

		input->addItem(choices[i].c_str());
	}

	QObject::connect(input, SIGNAL(currentIndexChanged(int)), this, SLOT(GUI_value_change()), Qt::AutoConnection);

	if (iSelect > 0)
		input->setCurrentIndex(iSelect);

	return input;
}

void GUI_combo::clearChoices()
{
	QWriteLocker l(&lockChangeChoices);

	choices.clear();
	bSyncChoicesGUI = true;
}


void GUI_combo::AddChoices(const std::string& sChoices)
{
	QWriteLocker l(&lockChangeChoices);

	size_t start = 0;
	size_t stop = sChoices.find("\n", start);

	while (stop != string::npos)
	{
		AddChoice(sChoices.substr(start, stop - start));
		start = stop + 1;
		stop = sChoices.find("\n", start);
	}
}

bool GUI_combo::AddChoice(const std::string& sChoice)
{
	QWriteLocker l(&lockChangeChoices);

	//return immediately if the choice is already there
	if ( std::find(choices.begin(), choices.end(), sChoice) != choices.end() || !sChoice.length())
		return false;

	choices.push_back(sChoice);
	bSyncChoicesGUI = true;

	return true;
}

bool GUI_combo::RemoveChoice(const std::string& sChoice)
{
	QWriteLocker l(&lockChangeChoices);

	vector<string>::iterator it = std::find(choices.begin(), choices.end(), sChoice);

	//return immediately if the choice is not already there
	if (it == choices.end() || !sChoice.length())
		return false;

	choices.erase(it);
	bSyncChoicesGUI = true;

	return true;
}


void GUI_combo::UpdateGUI_Value()
{
	if (bSyncChoicesGUI)
		gui_revision = -2;

	if (gui_revision >= revision)
		return;

	QWriteLocker l(&lockChangeChoices);

	try
	{
		std::string v = Value();

		if (bSyncChoicesGUI)
		{
			//synchronize list of choices in GUI here
			input->clear();

			for (unsigned i = 0; i < choices.size(); i++)
				input->addItem(choices[i].c_str(), 0);

			bSyncChoicesGUI = false;
		}

		int i = input->findText(v.c_str());

		if (i >= 0)
			input->setCurrentIndex(i);

	}
	catch (Uninitialized) {}
}

int GUI_matrix::min_height()
{
	return 28 * (1 + nR);
}

GUI_matrix::GUI_matrix(const std::string& sIni,
                       InputParameters* pIPs,
                       std::vector<ParameterGUI_Base*> * pv) :
	Parameter_Base(extractName(sIni)),
	InputParameterGUI<MatrixWidget, matrix_t, false>(extractName(sIni), pIPs, "", pv, extract_flags(sIni)),
	sIni(sIni),
	nR(extract_val<unsigned>(sIni, "nr=")),
	nC(extract_val<unsigned>(sIni, "nc="))
{
	// setFlags(getFlags() | RP_FLAG_NOPACK);

	//make sure matrix is initialized & the correct size
	try{
		matrix_t m = Value();
		if (m.resize(nR, nC))
			SetValue(m);
	}
	catch (Uninitialized u)
	{
		matrix_t m(nR, nC);
		SetValue(m);
	}
}

GUI_matrix::GUI_matrix(const std::string& sIni,
                       InputParameters* pIPs,
                       unsigned nR, unsigned nC,
                       std::vector<ParameterGUI_Base*> * pv) :
	Parameter_Base(extractName(sIni)),
	InputParameterGUI<MatrixWidget, matrix_t, false>(extractName(sIni), pIPs, "", pv, extract_flags(sIni)),
	sIni(sIni),
	nR(nR),
	nC(nC)
{
	// setFlags(getFlags() | RP_FLAG_NOPACK);

	//make sure matrix is initialized & the correct size
	try{
		matrix_t m = Value();
		if (m.resize(nR, nC))
			SetValue(m);
	}
	catch (Uninitialized u)
	{
		matrix_t m(nR, nC);
		SetValue(m);
	}
}

unsigned GUI_matrix::get_binary_length()
{
	return value.get_binary_length();
}

void GUI_matrix::insert_binary(char* p)
{
	value.insert_binary(p);
}
