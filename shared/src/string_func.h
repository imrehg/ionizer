#ifndef STRING_FUNC_H
#define STRING_FUNC_H

#include <string>
#include <vector>

#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

/*
template for conversion between strings and binary types
*/

template<class V> void get_sprintf_fmt(char* s, const V*);
template<class T> int to_string(const T& value, char* s, size_t n);
template<class T> T from_string(const std::string& s);

template<class T> std::string to_string(const T& value, int precision=-1);
template<class T> T from_string(const std::string& s);


const char* polName(int p);
const char* modeName(int i);
std::string sbName(int i);
std::string pulse_name(const std::string& base_name, int mFg2, int pol, int sb);
void name2line(const std::string& name, const std::string& base_name, int* mFg2, int* pol, int* sb);

//! put the correct format string into s to extract a value of type V using sscanf
template<class V> void get_sscanf_fmt(char* s, V*);

template<class V> bool extract_val(const std::string& s, const std::string& name, V* value);
template<class V> V extract_val(const std::string& s, const std::string& name);

//! Turn a comma-delimeted string into a vector of strings
std::vector<std::string> commaDelimStrings2Vector(const std::string& s);

#endif // STRING_FUNC_H
