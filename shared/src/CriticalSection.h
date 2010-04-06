#pragma once

//a class to associate windows CRITICAL_SECTIONs with a name
//the CRITICAL_SECTION is aquired during construction and released during destruction.
//optional logging to cerr for all entries / exits helps debug race conditions

#ifdef NO_CS
class cs_t
{
public:
void lock()
{
}
void unlock()
{
}
bool tryLock()
{
	return true;
}
};


#else
	#include <QMutex>
typedef QMutex cs_t;
#endif

#include <string>


class NamedCriticalSection
{
public:
NamedCriticalSection(const std::string& name, bool log = false);
~NamedCriticalSection();

bool try_enter();
void enter();
void leave();

protected:
void cs_init();
void cs_destroy();
void cs_enter();
void cs_leave();
bool cs_try_enter();
private:

cs_t cs;
std::string name;
bool log;
};


//encapsulate the use of NamedCriticalSections as automatic (stack) objects
//that way they get released automatically as the object goes out of scope
class CriticalSectionOwner
{
public:
CriticalSectionOwner(NamedCriticalSection* pCS, CriticalSectionOwner* pCSO = 0);
~CriticalSectionOwner()
{
	if (bEntered) pCS->leave();
}

protected:
bool bEntered;
NamedCriticalSection* pCS;
};
