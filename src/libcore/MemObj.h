
/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela

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

#ifndef MEMOBJ_H
#define MEMOBJ_H

#include <vector>

#include "nanassert.h"
#include "callback.h"

#include "ThreadContext.h"

#ifndef DEBUGCONDITION
#ifdef DEBUG
#define DEBUGCONDITION 0
#else
#define DEBUGCONDITION 0
#endif
#endif

class MemRequest;      // Memory Request (from processor to cache)

class MemObj {
public:
  typedef std::vector<MemObj*> LevelType;
private:
  bool highest;

protected:
  
  uint32_t nUpperCaches;
  LevelType upperLevel;
  LevelType lowerLevel;

  const char *descrSection;
  const char *symbolicName;

  void addLowerLevel(MemObj *obj) { 
    I( obj );
    lowerLevel.push_back(obj);
    obj->addUpperLevel(this);
  }

  void addUpperLevel(MemObj *obj) { 
    upperLevel.push_back(obj);
  }

  void invUpperLevel(PAddr addr, ushort size, MemObj *oc) {

    I(oc);
    
	 for(uint32_t i=0; i<upperLevel.size(); i++)
      upperLevel[i]->invalidate(addr, size, oc);    
  }


public:
  MemObj(const char *section, const char *sName);
  MemObj();
  virtual ~MemObj();

  bool isHighestLevel() const { return highest; }
  void setHighestLevel() {
    I(!highest);
    highest = true;
  }

  const char *getDescrSection() const { return descrSection; }
  const char *getSymbolicName() const { return symbolicName; }

  const LevelType *getLowerLevel() const { return &lowerLevel; }
  const LevelType *getUpperLevel() const { return &upperLevel; }

  const uint32_t getUpperLevelSize() const { return upperLevel.size(); }

  virtual const bool isCache() const { return false; }

  const uint32_t getNumCachesInUpperLevels() const {
    return nUpperCaches;
  }

  void computenUpperCaches();

  //This assumes single entry point for object, which I do not like,
  //but it is still something that is worthwhile.
  virtual Time_t getNextFreeCycle() const = 0;

  virtual void access(MemRequest *mreq) = 0;
  virtual void returnAccess(MemRequest *mreq) = 0;

  virtual void invalidate(PAddr addr, ushort size, MemObj *oc) = 0;
  virtual void doInvalidate(PAddr addr, ushort size) { I(0); }

  typedef CallbackMember2<MemObj, PAddr, ushort,
                         &MemObj::doInvalidate> doInvalidateCB;

  // When the buffers in the cache are full and it does not accept more requests
  virtual bool canAcceptStore(PAddr addr) = 0;
  virtual bool canAcceptLoad(PAddr addr) { return true; }

  // Print stats
  virtual void dump() const;
};

class DummyMemObj : public MemObj {
private:
protected:
  Time_t getNextFreeCycle() const;
  void access(MemRequest *req);
  bool canAcceptStore(PAddr addr);
  void invalidate(PAddr addr, ushort size, MemObj *oc);
  void doInvalidate(PAddr addr, ushort size);
  void returnAccess(MemRequest *req);
public:
  DummyMemObj();
  DummyMemObj(const char *section, const char *sName);
};

#endif // MEMOBJ_H
