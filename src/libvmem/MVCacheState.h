/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau

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

#ifndef MVCACHESTATE_H
#define MVCACHESTATE_H

#include "nanassert.h"
#include "CacheCore.h"

#include "MemRequest.h"
#include "LVIDTable.h"
#include "VMemObj.h"

class MVCacheState : public VMemState {
private:

  typedef CacheGeneric<MVCacheState,LPAddr, false> CacheType;

  CacheType *cache;
  
  LPAddr tag;
  ID(ulong _set;);

protected:
  LVID  *lvid;  // lvid == 0 means invalid, safest points to LVIDSafest
  SubLVIDType subLVID;

  void promote();

public:
  MVCacheState();

  void setSet(ulong set) {
    IS(_set = set;);
  }

  void promote2Safest(); // == oldest

  bool isSafe() const {
    if (isRestarted())
      return false;
    
    return lvid->isSafe();
  }

  bool isFinished() const {
    if (isRestarted())
      return false;
    
    return lvid->isFinished();
  }

  SubLVIDType getSubLVID() const { return subLVID; }
  LVID *getLVID() const { return lvid; }
  const HVersion *getVersionRef() const {
    I(lvid);
    return lvid->getVersionRef();
  }

  HVersion *getVersionDuplicate() const {
    I(lvid);
    return lvid->getVersionDuplicate();
  }
  
  LPAddr getLPAddr() const { 
    return cache->calcAddr4Tag(tag); 
  }

  bool isHit(PAddr paddr) const {
    return cache->calcTag(paddr) == cache->calcTag(lvid->calcPAddr(cache->calcAddr4Tag(tag)));
  }

  bool isHit(LPAddr addr) const {
    return cache->calcTag(addr) == tag;
  }
  
  PAddr  getPAddr() const  { 
    I(lvid);
    return lvid->calcPAddr(cache->calcAddr4Tag(tag)); 
  }
  
  bool isRestarted() const {
    if (isInvalid())
      return true;
    
    return subLVID != lvid->getSubLVID();
  }
  
  LPAddr getTag() const { return tag; }
  void setTag(LPAddr a) { 
    I(a);
    GI(_set != (ulong) -1 && a, cache->calcSet4Tag(a) == _set);
    GI(tag && a, cache->calcSet4Tag(tag) == cache->calcSet4Tag(a));
    GI(a, static_cast<PAddr>(cache->calcAddr4Tag(a)));
    tag=a; 
  }

  void initialize(CacheType *c);

  bool isKilled() const {
    if (lvid ==0)
      return false;
    return lvid->isKilled();
  }

  bool isInvalid() const { 
    // Note: a cache line has two levels of invalid. isInvalid() and
    // isRestarted(). The former means that the there is no data in the cache
    // line (traditional cache invalid). The second means that the task got a
    // restart, so currently the cache line is invalid, but it can be promoted
    // to become valid (promoted through ::restart()).
    GI(lvid == 0, getTag() == 0);
    GI(lvid, getTag());
    return lvid == 0; 
  }

  bool isValid() const { 
    GI(lvid == 0, getTag() == 0);
    GI(lvid, getTag());
    return lvid;
  }
  
  void invalidate();

  bool isLocked() const;

  void resetState(LPAddr addr, LVID *l, const VMemState *st=0);
  bool accessLine(); // Called each time that the cache line is accessed (may
		     // trigger recycle)


  void updateWord(MemOperation op, PAddr paddr) {
    if(op == MemRead) {
      readWord(paddr);
    }else{
      I(op == MemWrite);
      writeWord(paddr);
    }
  }

  void dump(const char *str);
};

class MVCacheStateLess {
public:
  bool operator()(const MVCacheState *x, const MVCacheState *y){
    return *(x->getVersionRef()) < *(y->getVersionRef());
  }
};

class MVCacheStateMore {
public:
  bool operator()(const MVCacheState *x, const MVCacheState *y){
    return *(x->getVersionRef()) > *(y->getVersionRef());
  }
};

#endif // MVCACHESTATE_H
