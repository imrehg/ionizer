#include "CmdLineArgs.h"

#include <vector>
#include <sstream>

using namespace std;

CmdLineArgs::CmdLineArgs(int argc, char* argv[])
{
	m_vArgv = vector<string>(argc);

	for(int i=0; i<argc; i++) {
		m_vArgv[i] = argv[i];
	}
}

CmdLineArgs::CmdLineArgs(const char* szCommandLine)
{
	istringstream ss(szCommandLine);

	do {
		string s;
		ss >> s;
		m_vArgv.push_back(s);
	} while(!(ss.rdstate() & ios::eofbit));
}

int CmdLineArgs::FindString(const string& s, int iAfter /*=0*/) const
{
	for(int i=iAfter; i<(int)m_vArgv.size(); i++)
		if(m_vArgv[i] == s)
			return i;

	return -1;
}

int CmdLineArgs::GetString(std::string& s, int index) const
{
	if((index < 0) || (index >= (int)m_vArgv.size()))
		return -1;

	s = m_vArgv[index];
	return (int)s.length();
}

string CmdLineArgs::GetStringAfter(const std::string& s, const string& sDefault) const
{
	int i = FindString(s);

	if(i < 0) 
		return sDefault;

	string sReturn;

	if(GetString(sReturn, i+1) < 0) 
		return sDefault;
	else 
		return sReturn;
}

string CmdLineArgs::GetStringAfter(const std::string& s) const
{
	int i = FindString(s);
	if(i < 0) 
		throw non_existing_string(s);

	string sReturn;

	if(GetString(sReturn, i+1) < 0) 
		throw non_existing_string(s);
	else 
		return sReturn;
}










