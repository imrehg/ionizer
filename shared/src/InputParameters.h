/*  ___
 |R  */

#pragma once

#include <map>
#include "CriticalSection.h"

#include <string_func.h>
#include <iostream>
#include <string>
#include <stdexcept>

//parameter data represented as a string in the back-end DB
class ip_data
{
public:
ip_data() : was_touched(false), revision(0)
{
}

std::string value;
bool was_touched;
int revision;
};

//a string-to-string map that serves as a simple parameter database
//the data can be written to an ostream and recovered from an istream.
class InputParameters : private std::map<std::string, ip_data>
{
public:
//constructor for istream text source
InputParameters(std::ifstream* p_is);
virtual ~InputParameters();

//data access functions are protected by a critical section to ensure thread safety

//Updates the "value" associate with the "key".
//If the new value is different from the stored one the revision number is incremented.
bool UpdatePair(const std::string& key, const std::string& value, int new_revision, bool touch = true);

//Returns the value associated with the "key"
std::string GetValue(const std::string& key);
int GetRevision(const std::string& key);

//save data to an ostream
void SaveState(std::ostream* p_os);

//returns whether or not the map contains at least one non-empty value
bool HasValidData();

virtual std::string Name()
{
	return "";
}

protected:
//process line to extract name/value pair
bool processOld(const std::string& sLine, std::string& sName, std::string& sValue);
bool processNew(const std::string& sLine, std::string& sName, std::string& sValue);

//for thread safety only allow one data operation at a time.
NamedCriticalSection cs;
};

//a non-volatile derivative of "InputParameters: that uses text files to store the data
class TxtParameters : public InputParameters
{
public:
//constructor for text file input
TxtParameters(const std::string& sTxtFileName);
virtual ~TxtParameters();

void SaveState(const std::string&  Directory);
std::string Name() const
{
	return sFileName;
}
protected:
std::string sFileName;
};

class Parameter_Base
{
public:
Parameter_Base(const std::string& name) :
	revision(-1),
	name(name),
	fpga_name(name)
{
}

virtual ~Parameter_Base()
{
}

virtual bool IsInitialized() = 0;
int Revision() const
{
	return revision;
}
void set_revision(int r)
{
	revision = r;
}

//"display label" is to allow different DB names w/ the same display label;
void set_display_label(const std::string& l)
{
	display_label = l;
}

const std::string& getPrefixedName() const
{
	return IGetName();
}

const std::string& get_display_label() const
{
	if (display_label == "")
		return IGetName();
	else
		return display_label;
}

//"FPGA name" is to allow linking to new names w/ for the same FPGA param;
//this name can't be changed after initialization
const std::string& get_fpga_name() const
{
	if (fpga_name == "")
		return IGetName();
	else
		return fpga_name;
}

protected:

virtual const std::string& IGetName() const
{
	return name;
}
virtual bool ISetName(const std::string& name);
void SetRevision(int i)
{
	revision = i;
}

int revision;

private:
std::string name;
std::string display_label;
const std::string fpga_name;
};

template<class T> class Parameter : public virtual Parameter_Base
{
public:
Parameter(const std::string& name) : Parameter_Base(name)
{
}

virtual ~Parameter()
{
};

operator T(){
	return Value();
}

T Value()
{
	UpdateValueFromBackend();
	//std::cout << IGetName() << " = " << value << std::endl;
	return value;
}

virtual bool SetValue(const T& v)
{
	if (value != v)
	{
		revision++;
		value = v;

		return true;
	}
	else
		return false;
}

protected:

virtual void UpdateValueFromBackend()
{
}

T value;
};

class Uninitialized
{
public:
Uninitialized(const std::string& name) : Name(name)
{
};
std::string Name;
};


std::ostream& operator<<(std::ostream& o, const Uninitialized& u);

template <class T> class InputParameter : public Parameter<T>
{
public:
InputParameter(const std::string& sName, InputParameters* pIPs, const std::string& sDefault = "") :
	Parameter_Base(sName),
	Parameter<T>(sName),
	m_pIPs(pIPs)
{
	SetDefault(sDefault);
	UpdateValueFromBackend();
}

virtual ~InputParameter()
{
}

void SetDefault(const std::string& sDefault)
{
	if (!IsInitialized())
		m_pIPs->UpdatePair(Parameter_Base::IGetName(), sDefault, 0);
}

virtual bool SetValue(const T& v)
{
	if (Parameter<T>::SetValue(v))
	{
		m_pIPs->UpdatePair(Parameter_Base::IGetName(), to_string<T>(v), -1);    //Parameter_Base::Revision());

//			std::cerr << "[SetValue][" << m_pIPs->Name() << "] : " << IGetName() << " = " << v << std::endl;

		return true;
	}
	else
		return false;
}

int getBackendRevision()
{
	return m_pIPs->GetRevision(Parameter_Base::IGetName());
}

virtual void UpdateValueFromBackend()
{
	int backend_revision = getBackendRevision();

	if (backend_revision > Parameter_Base::Revision())
	{
		std::string s = m_pIPs->GetValue(Parameter_Base::IGetName());

		if (!s.length())
			std::cerr << Parameter_Base::IGetName() << " is uninitialized." << std::endl;

		Parameter<T>::SetValue(from_string<T>(s));

		Parameter_Base::SetRevision( backend_revision );
	}
}

virtual bool IsInitialized()
{
	return m_pIPs->GetValue(Parameter_Base::IGetName()).length() > 0;
}

virtual bool LinkTo(InputParameter<T>* pIP)
{
	if (!pIP)
		return false;

	if (m_pIPs == pIP->m_pIPs)
		if (Parameter_Base::IGetName() == pIP->IGetName())
			return false;

	Parameter_Base::SetRevision(-1);
	m_pIPs = pIP->m_pIPs;
	ISetName(pIP->IGetName());
	UpdateValueFromBackend();

	return true;
}

virtual bool LinkTo(InputParameters* pIPs, const std::string& sName)
{
	if (!pIPs)
	{
		std::cerr << "[InputParameter_Base::LinkTo] error: pIPs == 0" << std::endl;
		throw std::runtime_error("[InputParameter_Base::LinkTo] error: pIPs == 0");
	}

	if (m_pIPs == pIPs && Parameter_Base::IGetName() == sName)
		return false;

	Parameter_Base::SetRevision(-1);
	m_pIPs = pIPs;
	Parameter_Base::ISetName(sName);
	UpdateValueFromBackend();

	return true;
}

protected:
InputParameters* m_pIPs;
};

typedef InputParameter<std::string> param_string;
typedef InputParameter<double> param_double;
typedef InputParameter<int> param_int;

template <class T> bool operator==(const T& t, InputParameter<T>& p)
{
	return t == static_cast<T>(p);
}

template <class T> bool operator==(InputParameter<T>& p, const T& t)
{
	return t == p;
}

template <class T> std::ostream& operator<<(std::ostream& o, InputParameter<T>& p)
{
	return o << static_cast<T>(p);
}
