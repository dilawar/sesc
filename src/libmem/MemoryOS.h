/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Basilio Fraguela
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
#ifndef MEMORYOS_H
#define MEMORYOS_H

#include "nanassert.h"

#include <queue>
#include <iostream>

#include "estl.h"
#include "GStats.h"
#include "GProcessor.h"
#include "MemRequest.h"
#include "MemObj.h"
#include "MemorySystem.h"

#include "TLB.h"

typedef HASH_MAP<uint32_t, uint32_t> PageMapper;

class MemoryOS : public GMemoryOS {
public:
  static uint32_t numPhysicalPages;
protected:
  TLB DTLB;
  TLB ITLB;
  GMemorySystem *memorySystem;

  GStatsCntr missesDTLB;
  GStatsCntr missesITLB;

  GStatsCntr TLBTime;
  Time_t     startTime;

  TimeDelta_t period;
  bool busy;

  void fillDTLB(int32_t vAddr, int32_t phPage) {
    DTLB.insert(vAddr, GMemorySystem::calcFullPage(phPage));
  }

  void fillITLB(int32_t vAddr, int32_t phPage){
    ITLB.insert(vAddr, GMemorySystem::calcFullPage(phPage));
  }

  void launchReq(MemRequest *mreq, int32_t phaddr) const {
     mreq->setPAddr(phaddr);
     mreq->getCurrentMemObj()->access(mreq);
  }

  void completeReq(MemRequest *mreq, int32_t vAddr, int32_t phPage) {
    GLOG(DEBUGCONDITION,"[C %llu] %u. %i -> %i", globalClock, mreq->getVaddr(), GMemorySystem::calcPage(vAddr), phPage);
    if(mreq->isDataReq()) 
      fillDTLB(vAddr, phPage);
    else 
      fillITLB(vAddr, phPage);

    // address translation is turned off
    // launchReq(mreq, GMemorySystem::calcPAddr(GMemorySystem::calcFullPage(phPage), mreq->getVaddr()));
    launchReq(mreq,vAddr);
  }

public:
  MemoryOS(GMemorySystem *ms, const char *section);

  virtual ~MemoryOS() { }

  int32_t TLBTranslate(VAddr vAddr);
  int32_t ITLBTranslate(VAddr vAddr);

};

class PageTable {
public:
  typedef struct {
    Time_t lastTimeToTLB;
    uint32_t virtualPage;
    int16_t status;
  } IntlPTEntry;

protected:

  enum {
    ValidPageStatus  = 1,
    SystemPageStatus = 2,
    PinnedPageStatus = 4,
    MigrPageStatus   = 8
  };

  typedef struct {
    uint32_t physicalPage;
    int16_t status;
  } L1IntlPTEntry;

  PageMapper vToPMap;
  IntlPTEntry   *invertedPT;
  L1IntlPTEntry *L1PT;

  uint32_t maskEntriesPage;
  uint32_t log2EntriesPage;

  GStatsCntr numOSPages;
  GStatsCntr numUsrPages;

/* We will assume 32 bit page table entries */
  static const uint16_t BytesPerEntry = 4;

  IntlPTEntry *getReplCandidate(uint16_t first_bank, uint16_t search_step);

public:
  PageTable();
  ~PageTable();

  IntlPTEntry * getReplPhPageL2Entry() {
    return getReplCandidate(0, 1);
  }

  uint32_t getL1EntryNum(const uint32_t vPage) const {
    return (vPage >> log2EntriesPage);
  }

  void assignL1Entry(uint32_t vPage, uint32_t pPage);
  void assignL2Entry(uint32_t vPage, uint32_t pPage);
  void evictL2Entry(IntlPTEntry *p);

  uint32_t L2PTEntryToPhPage(const IntlPTEntry *p) const {
    return p - invertedPT;
  }

  uint32_t getL1PTEntryPhysicalAddress(const uint32_t vPage) const {
    return getL1EntryNum(vPage) * BytesPerEntry + 0xFFFF;
  }

  int32_t getL2PTEntryPhysicalAddress(const uint32_t vPage) const {
    L1IntlPTEntry *p = L1PT + getL1EntryNum(vPage);
    if (!(p->status & ValidPageStatus) )
      return -1;
    else 
      return (int)((p->physicalPage) | ((vPage & maskEntriesPage) * BytesPerEntry)) + 0xFFFF;
  }

  void stampPhPage(const uint32_t pPage) {
    // This should really imply a write op, but...
    invertedPT[pPage].lastTimeToTLB = globalClock;
  }

  int32_t translate(const uint32_t vPage) {
    PageMapper::const_iterator it = vToPMap.find(vPage);
    if (it == vToPMap.end())
      return -1;
    else {
      stampPhPage((*it).second);
		return (int)(*it).second;
    }
  }
};

class StdMemoryOS : public MemoryOS {
 private:
  PageTable::IntlPTEntry *getReplPTEntry();

  void accessL1PT(MemRequest *origReq, PAddr paddr);
  void accessL2PT(MemRequest *origReq, PAddr paddr);

  StaticCallbackMember2<StdMemoryOS, MemRequest *, PAddr, &StdMemoryOS::accessL1PT> accessL1PTCB;
  StaticCallbackMember2<StdMemoryOS, MemRequest *, PAddr, &StdMemoryOS::accessL2PT> accessL2PTCB;

  StaticCBMemRequest accessL1PTReq;
  StaticCBMemRequest accessL2PTReq;

  uint32_t getNewPhysicalPage();
protected:
  static PageTable *PT; // Shared by all the MemoryOS

  std::vector<MemRequest *> pendingReqs;
  uint32_t     nextPhysicalPage; //Marker to find new free physical pages
  MemObj   *cacheObj;
  TimeDelta_t minTLBMissDelay;

  uint32_t getFreePhysicalPage();
  void serviceRequest(MemRequest*);
  void attemptToEmptyQueue(uint32_t vaddr, uint32_t phPage);

public:
  StdMemoryOS(GMemorySystem *ms, const char *descr_section);
  ~StdMemoryOS();

  /*long translate(long vAddr);*/

  void solveRequest(MemRequest *r);
  void boot();
  void report(const char *str) {};
};


#endif
