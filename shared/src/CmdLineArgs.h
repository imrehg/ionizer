#ifndef CMDLINEARGS_H
#define CMDLINEARGS_H

#include <vector>

#include "string_func.h"

/*
a class to ease command line parsing
At the moment it handles -key_name key_value combinations and flags.
E.g. "program.exe -gain 4 -hide" would use GetValAfter<int>("-gain") and FindString("-hide").

___
 |R [5/14/2004]

*/

class CmdLineArgs
{
public:
	//constructor for argc/argv style command line
	CmdLineArgs(int argc, char* argv[]);

	//constructor for windows style command line
	CmdLineArgs(const char* lpCommandLine);

	//exception class for non existing strings
	class non_existing_string
	{
	public:
		non_existing_string(const std::string& s) : s(s) {}
		const std::string& what() const {return s;}
	private:
		std::string s;
	};

	//return the index of matching string s after index iAfter, or -1 on failure.
	int FindString(const std::string& s, int iAfter=0) const;

	//return the value after s.
	template<class T> T GetValAfter(const std::string& s, const std::string& sDefault="") const
	{
		std::string sVal;

		if(sDefault.length() > 0)
			sVal = GetStringAfter(s, sDefault);
		else
			sVal = GetStringAfter(s);

		return from_string<T>(sVal);
	}

	//return the string after string s, or sDefault if s is not found
	std::string GetStringAfter(const std::string& s) const;

	std::string GetStringAfter(const std::string& s, const std::string& sDefault) const;

	//place the string at index into s.  return string length on success, -1 on failure.
	int GetString(std::string& s, int index) const;

protected:
	std::vector<std::string> m_vArgv;
};


#endif //CMDLINEARGS_H
