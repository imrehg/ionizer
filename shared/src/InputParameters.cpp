/*  ___
 |R  */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <string_func.h>
#include "InputParameters.h"

using namespace std;

//Read name/value string pairs from an input stream into the map
//The file consists of lines of format:
// name1 = value1 ;
// name2 = value2 ;
// ...

InputParameters::InputParameters(std::ifstream* p_is) :
	cs("InputParameters::cs", false)
{

	//loop through the file
	while (p_is->good() && !p_is->eof())
	{
		string sLine("");

		//read until ';'
		getline(*p_is, sLine, ';');

		string sName, sValue;
		time_t tChanged  = 0;
		if (!processNew(sLine, sName, sValue, tChanged))
			if (!processOld(sLine, sName, sValue))
				continue;


		//add the pair sName, sValue to the map
		UpdatePair(sName, sValue, 0, false, tChanged);
		//	cerr << sName << " = " << sValue << endl;

	}

	delete p_is;
}

InputParameters::~InputParameters()
{
}

bool InputParameters::processOld(const std::string& sLine, std::string& sName, std::string& sValue)
{
	string sWhite("\r\n ");

	//find the last '='
	size_t lastEqual = sLine.rfind('=');

	if (lastEqual != string::npos)
	{
		//copy the part before '=' into sName
		sName = string(sLine, 0, lastEqual);

		//copy the part after '=' into sValue
		sValue = string(sLine, lastEqual + 1, sLine.length() - (lastEqual + 1));

		//clear beginning and trailing white space
		size_t pos = sName.find_first_not_of(sWhite);
		size_t n = sName.find_last_not_of(sWhite) - pos + 1;

		if (pos == string::npos)
			return false;

		sName = string(sName, pos, n);

		pos = sValue.find_first_not_of(sWhite);
		n = sValue.find_last_not_of(sWhite) - pos + 1;
		if (pos != string::npos)
			sValue = string(sValue, pos, n);
		else
			sValue = "";
	}

	return true;
}

bool InputParameters::processNew(const std::string& sLine, std::string& sName, std::string& sValue, time_t& tChanged)
{
	string sWhite("\r\n ");

	//find left/right brace surrounding the name
	size_t lbN = sLine.find('{');
	size_t rbN = sLine.find('}');

	if (lbN == string::npos || rbN == string::npos)
		return false; //can't process

	//copy the bracketed part into sName
	sName = string(sLine, lbN + 1, rbN - lbN - 1);



	//find left/right brace surrounding the value
	size_t lbV = sLine.find('{', rbN + 1);
	size_t rbV = sLine.rfind('}');

	if (lbV == string::npos || rbV == string::npos || lbV > rbV)
		return false; //can't process

	sValue = string(sLine, lbV + 1, rbV - lbV - 1);

	sscanf(sLine.substr(rbV+1).c_str(), ", %u", &tChanged);

	return true;
}

bool InputParameters::HasValidData()
{
	CriticalSectionOwner cso(&cs, 0);

	for (const_iterator cit = begin(); cit != end(); cit++)
		if (cit->second.value.length())
			return true;

	return false;
}


std::string InputParameters::GetValue(const std::string& key)
{
	CriticalSectionOwner cso(&cs, 0);

	string value = "";

	if (this->end() != this->find(key))
	{
		value = (*this)[key].value;
		(*this)[key].was_touched = true;
	}

	return value;
}

int InputParameters::GetRevision(const std::string& key)
{
	CriticalSectionOwner cso(&cs, 0);

	if (this->end() != this->find(key))
		return (*this)[key].revision;
	else
		return -1;

}


void InputParameters::SaveState(std::ostream* p_os)
{
	if (!p_os->good())
	{
		delete p_os;
		return;
	}

	CriticalSectionOwner cso(&cs, 0);

	for (const_iterator cit = begin(); cit != end(); cit++)
		if (cit->second.value.length() > 0)
		{
			//	if(cit->second.was_touched)
			*p_os << "{" << cit->first.c_str() << "} = {";
			*p_os << cit->second.value.c_str() << "}, " << cit->second.tChanged << ";" << endl;
		}

	delete p_os;
}


bool InputParameters::UpdatePair(const std::string& sName, const std::string& sValue, 
								 int new_revision, bool touch, time_t tChanged)
{
	if(sName.length() == 0)
		return false;

	CriticalSectionOwner cso(&cs, 0);

	(*this)[sName].was_touched |= touch;

	if ((*this)[sName].value != sValue)
	{
		(*this)[sName].value = sValue;

		if(tChanged > 0)
			(*this)[sName].tChanged = tChanged;
		else
			time( & ((*this)[sName].tChanged) );

		if (new_revision >= 0)
			(*this)[sName].revision = new_revision;
		else
			(*this)[sName].revision++;

		return true;
	}
	else
		return false;
}

/* compiler generates a screwy warning so turn it off temporarily... */
//#pragma warning(push)
//#pragma warning(disable : 4239)

TxtParameters::TxtParameters(const std::string& sTxtFileName) :
	InputParameters(new std::ifstream(sTxtFileName.c_str())),
	sFileName(sTxtFileName)
{
}

TxtParameters::~TxtParameters()
{
	if (char* d = getenv("ALUMINIZER_PARAMS_DIR"))
		SaveState(d);
	else
		SaveState("./");
}

void TxtParameters::SaveState(const std::string& Directory)
{
	//only save the file if there are non-empty values
	if (HasValidData())
		InputParameters::SaveState(new ofstream(string(Directory + sFileName).c_str()));
}

//#pragma warning(pop)

bool Parameter_Base::ISetName(const std::string& name)
{
	if (this->name != name)
	{
		this->name = name;
		revision = -1;
		return true;
	}
	else
		return false;
}

std::ostream& operator<<(std::ostream& o, const Uninitialized& u)
{
	o << "The variable \"" << u.Name << "\" is uninitialized.";

	if (u.Name.find("Hz") != string::npos)
		o << "  Kenneth, what is the frequency?";

	return o;
}

