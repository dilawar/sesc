/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
		  Milos Prvulovic

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <string.h>
#include <set>

#include "GMemorySystem.h"

#include "MemObj.h"
#include "MemRequest.h"

class Setltstr {
public:
  bool operator()(const char* s1, const char* s2) const {
    return strcasecmp(s1, s2) < 0;
  }
};

MemObj::MemObj(const char *section, const char *sName)
  :descrSection(section)
  ,symbolicName(sName)
{
  highest = false;
  nUpperCaches = 0;
  
#ifdef DEBUG
  static std::set<const char *, Setltstr> usedNames;
  if( sName ) {
    // Verify that one else uses the same name
    if( usedNames.find(sName) != usedNames.end() ) {
      MSG("Creating multiple memory objects with the same name '%s' (rename one of them)",sName);
    }else{
      usedNames.insert(sName);
    }
  }
#endif
}

MemObj::~MemObj() 
{
  if(descrSection != 0) 
    delete [] descrSection;
  if(symbolicName != 0) 
    delete [] symbolicName;
}

void MemObj::computenUpperCaches() 
{
  nUpperCaches = 0;

  for(uint32_t j=0; j < upperLevel.size(); j++) {
    if(upperLevel[j]->isCache())
      nUpperCaches++;
    else 
      nUpperCaches += upperLevel[j]->getNumCachesInUpperLevels();
  }

  // top-down traversal of the memory objects
  for(uint32_t i=0; i < lowerLevel.size(); i++) {
    lowerLevel[i]->computenUpperCaches();
  }
}

void MemObj::dump() const
{
  LOG("MemObj name [%s]",symbolicName);
}

// DummyMemObj

DummyMemObj::DummyMemObj() 
  : MemObj("dummySection", "dummyMem")
{ 
}

DummyMemObj::DummyMemObj(const char *section, const char *sName)
  : MemObj(section, sName)
{
}

Time_t DummyMemObj::getNextFreeCycle() const
{ 
  return globalClock; 
}

void DummyMemObj::access(MemRequest *req)    
{ 
  req->goUp(1); 
}

bool DummyMemObj::canAcceptStore(PAddr addr)
{ 
  return true;  
}

void DummyMemObj::invalidate(PAddr addr, ushort size, MemObj *oc)
{
  invUpperLevel(addr, size, oc);
}

void DummyMemObj::doInvalidate(PAddr addr, ushort size)
{
  I(0);
}

void DummyMemObj::returnAccess(MemRequest *req) 
{
  I(0);
}
