#include "CriticalSection.h"

#include <iostream>

void NamedCriticalSection::cs_init()
{
//	pthread_mutex_init(&cs, NULL);
}

void NamedCriticalSection::cs_destroy()
{
//	pthread_mutex_destroy(&cs);
}

void NamedCriticalSection::cs_enter()
{
   cs.lock();
//	pthread_mutex_lock(&cs);
}

void NamedCriticalSection::cs_leave()
{
   cs.unlock();
//	pthread_mutex_unlock(&cs);
}

bool NamedCriticalSection::cs_try_enter()
{
   return cs.tryLock();

//	return pthread_mutex_trylock(&cs) == 0;
}


NamedCriticalSection::NamedCriticalSection(const std::string& name, bool /*log*/) :
#ifndef NO_CS
cs(QMutex::Recursive), 
#endif
name(name), 
log(false)
{
   cs_init();
}

NamedCriticalSection::~NamedCriticalSection()
{
   cs_destroy();
}

bool NamedCriticalSection::try_enter()
{
   bool b = cs_try_enter();

   if(log)
   {
      std::cerr << "[NamedCriticalSection::try_enter] " << name;

      if(b)
         std::cerr << " success";
      else
         std::cerr << " failure";

      std::cerr << std::endl;
   }

   return b;
}

void NamedCriticalSection::enter()
{
   if(log)
      std::cerr << "[NamedCriticalSection::enter] " << name << std::endl;

   cs_enter();
}

void NamedCriticalSection::leave()
{
   cs_leave();

   if(log)
      std::cerr << "[NamedCriticalSection::leave] " << name << std::endl;
}

CriticalSectionOwner::CriticalSectionOwner(NamedCriticalSection* pCS, CriticalSectionOwner* pCSO) :
   pCS(pCS)
{
   if(pCSO)
      if(pCSO->pCS == pCS)
      {
         bEntered = false;
         return;
      }


   pCS->enter();
   bEntered = true;
}
