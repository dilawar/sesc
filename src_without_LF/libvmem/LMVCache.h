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

#ifndef LMVCACHE_H
#define LMVCACHE_H

#include "EnergyMgr.h"
#include "estl.h"
#include "CacheCore.h"
#include "GMVCache.h"
#include "MVCacheState.h"

class VBus;

// Implementation for the lowest level of the version cache system. This cache
// has directory to know the how to get previous versions. It has special
// functionality to perform combine (combine safe lines)

// Last Multi-Version Cache
class LMVCache : public GMVCache {
private:
  GStatsCntr combineHit;
  GStatsCntr combineMiss;
  GStatsCntr combineHalfMiss;

  GStatsCntr nRestartDisp;
  GStatsCntr nRestartAlloc;

  GStatsEnergy *combWriteEnergy; // ETODO: a 4 cache line SRAM

  VBus *vbus;

protected:
  void doReadAck(VMemReadReq *remReadAck, TimeDelta_t latency);

  // BEGIN GMVCache pure virtual
  void cleanupSet(PAddr paddr);
  void displaceLine(CacheLine *cl);
  CacheLine *allocateLine(LVID *lvid, LPAddr addr);
  // END GMVCache pure virtual

  // BEGIN Safe Version Combine Functionality
  typedef HASH_MAP<PAddr, VMemPushLineReq *> CombineMap;
  
  CombineMap cMap;

  void combineInit(PAddr paddr, HVersion *verDup, const VMemState *state);
  void combineInit(const VMemPushLineReq *req);
  void combineInit(CacheLine *cl);
  void combinePushLine(const VMemPushLineReq *req);
  // END Safe Version Combine Functionality

  void writeMemory(PAddr paddr);
  void forwardPushLine(VMemPushLineReq *vreq);
  void ackPushLine(VMemPushLineReq *vreq);

public:
  LMVCache(MemorySystem *gms, const char *section, const char *name, VBus *vb = 0);
  virtual ~LMVCache();

  void returnAccess(MemRequest *mreq);

  void localRead(MemRequest *mreq);
  void localWrite(MemRequest *mreq);

  void read(VMemReadReq *vreq);
  void readAck(VMemReadReq *vreq);

  void writeCheck(VMemWriteReq *mreq);
  void writeCheckAck(VMemWriteReq *vreq);
  
  void pushLine(VMemPushLineReq *vreq);
  void pushLineAck(VMemPushLineReq *vreq);
  void askPushLine(VMemPushLineReq *vreq);

  bool isCombining(PAddr addr) const;
};

#endif // LMVCACHE_H
