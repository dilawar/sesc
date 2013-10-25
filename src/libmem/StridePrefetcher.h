/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze
                  Jose Renau

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

#ifndef STRIDE_PREFETCHER_H
#define STRIDE_PREFETCHER_H

#include <queue>
#include <deque>
#include <stack>

#include "CacheCore.h"
#include "callback.h"
#include "estl.h"
#include "GStats.h"
#include "MemObj.h"
#include "MemRequest.h"
#include "Port.h"

class BState : public StateGeneric<> {
};

class PfState : public StateGeneric<> {
 public:
  uint32_t stride;
  bool goingUp;
  
  PAddr nextAddr(CacheGeneric<PfState,PAddr> *c) {
    PAddr prevAddr = c->calcAddr4Tag(getTag());
    return (goingUp ? (prevAddr + stride) : (prevAddr - stride));
  }
};

class StridePrefetcher: public MemObj {
private:
  typedef CacheGeneric<BState,PAddr> BuffType;
  typedef CacheGeneric<BState,PAddr>::CacheLine bLine;

  typedef CacheGeneric<PfState,PAddr> PfTable;
  typedef CacheGeneric<PfState,PAddr>::CacheLine pEntry;

  typedef HASH_MAP<PAddr, std::queue<MemRequest *> *> penReqMapper;
  typedef HASH_SET<PAddr> penFetchSet;

  penReqMapper pendingRequests;

  penFetchSet pendingFetches;

  BuffType *buff;
  PfTable  *table;

  std::deque<PAddr> lastMissesQ;

  PortGeneric *buffPort;
  PortGeneric *tablePort;

  int32_t numStreams;
  int32_t streamAssoc;
  int32_t depth;
  int32_t numBuffPorts;
  int32_t buffPortOccp;
  int32_t numTablePorts;
  int32_t tablePortOccp;
  int32_t hitDelay;
  int32_t missDelay;
  int32_t learnHitDelay;
  int32_t learnMissDelay;
  uint32_t missWindow;
  uint32_t maxStride;
  static const int32_t pEntrySize = 8; // size of an entry in the prefetching table
  
  int32_t defaultMask;

  GStatsCntr halfMiss;
  GStatsCntr miss;
  GStatsCntr hit;
  GStatsCntr predictions;
  GStatsCntr accesses;
  GStatsCntr unitStrideStreams;
  GStatsCntr nonUnitStrideStreams;
  GStatsCntr ignoredStreams;

public:
  StridePrefetcher(MemorySystem* current, const char *device_descr_section,
  const char *device_name = NULL);
  ~StridePrefetcher() {}
  void access(MemRequest *mreq);
  void read(MemRequest *mreq);
  void returnAccess(MemRequest *mreq);
  bool canAcceptStore(PAddr addr);
  void invalidate(PAddr addr,ushort size,MemObj *oc);
  Time_t getNextFreeCycle() const;
  
  void learnHit(PAddr addr);
  void learnMiss(PAddr addr);
  void prefetch(pEntry *pe, Time_t lat);

  Time_t nextBuffSlot() {
    return buffPort->nextSlot();
  }

  Time_t nextTableSlot() {
    return tablePort->nextSlot();
  }

  void processAck(PAddr addr);
  typedef CallbackMember1<StridePrefetcher, PAddr, &StridePrefetcher::processAck> processAckCB;

};

#endif
