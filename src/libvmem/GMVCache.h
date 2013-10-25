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

#ifndef GMVCACHE_H
#define GMVCACHE_H

#include <queue>

#include "nanassert.h"
#include "CacheCore.h"
#include "GStats.h"

#include "VCache.h"
#include "MVCacheState.h"
#include "EnergyMgr.h"

class HVersion;
class VMemObj;

class GMVCache : public VCache {
private:
protected:
  typedef CacheGeneric<MVCacheState, LPAddr, false>            CacheType;
  typedef CacheGeneric<MVCacheState, LPAddr, false>::CacheLine CacheLine;
  
  CacheType *cache;

  static GStatsCntr *recycleClk;
  
  // State Machine that traverses cache 
  GStatsCntr nRecycleAllCache; 

  GStatsCntr nReadHit;     // Locally Satisfy
  GStatsCntr nReadHalfHit; // Less Spec Line
  GStatsCntr nReadMiss;    // Data not found
  GStatsCntr nServicedFromFuture; // Data serviced from future clean versions

  VMemReadReq *read(VMemReadReq *vreq, TimeDelta_t latency);
  VMemWriteReq *writeCheck(VMemWriteReq *vreq, VMemObj *mobj);

  // BEGIN Shared interface between MVCache & DirMVCache
  virtual void cleanupSet(PAddr paddr) = 0;
  virtual void displaceLine(CacheLine *cl) = 0;
  virtual CacheLine *allocateLine(LVID *lvid, LPAddr addr) = 0;
  // END Shared interface between MVCache & DirMVCache

  void recycleAllCache(const HVersion *exeVer);

  int32_t countLinesInSet(PAddr paddr);

  CacheLine *getSafeLine(PAddr paddr);
  CacheLine *getLineMoreSpecThan(const HVersion *ver, PAddr paddr);
  CacheLine *locallySatisfyLine(const HVersion *ver, PAddr paddr);
  CacheLine *locallyLessSpecLine(const HVersion *ver, PAddr paddr);
  
  LPAddr calcIndex4PAddr(PAddr paddr) {
#ifdef VMEM_CVBASE_IN_ADDR
    I(0);
#endif
    I(static_cast<LPAddr>(paddr) == lvidTable.getSafestEntry()->calcLPAddr(paddr)); //??? add by hr
    return cache->calcIndex4Addr(static_cast<LPAddr>(paddr));
  }

  PAddr calcPAddr(const LVID *lvid, LPAddr addr) const {
    return lvid->calcPAddr(addr);
  }
  PAddr calcPAddr(const CacheLine *cl) const {
    return calcPAddr(cl->getLVID(), cache->calcAddr4Tag(cl->getTag()));
  }

  GMVCache(MemorySystem *gms, const char *section, const char *name);
  virtual ~GMVCache();

  LVID *findCreateLVID(const VMemReq *req);

  bool checkSet(PAddr paddr);
public:
  LVID *findCreateLVID(HVersion *ver);
};

#endif // GMVCACHE_H
