/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Basilio Fraguela
                  Jose Renau
                  Rithin Kumar Shetty rkshetty@ncsu.edu

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
#include "SescConf.h"

#include "MemoryOS.h"
#include "UglyMemRequest.h"

#include "globals.h"

// TODO: Make this a parameter in sesc.conf
uint32_t MemoryOS::numPhysicalPages;
PageTable *StdMemoryOS::PT=0;

/** Note: bit field distribution according to PageSize:
          L1TP  L2TP   Offset
     4K     10    10    12     (minimum)
     8K      8    11    13
    16K      6    12    14
    32K      4    13    15
**/

MemoryOS::MemoryOS(GMemorySystem *ms, const char *section) 
  : GMemoryOS(ms->getId())
    ,DTLB(SescConf->getCharPtr(section,"dtlb"), true, ms->getId())
    ,ITLB(SescConf->getCharPtr(section,"itlb"), false, ms->getId())
    ,memorySystem(ms)
    ,missesDTLB("P(%i)_DTLB:nMiss", ms->getId())
    ,missesITLB("P(%i)_ITLB:nMiss",ms->getId())
    ,TLBTime("P(%i):TLBTime",ms->getId())
    ,period(0)
    ,busy(false)
{
  numPhysicalPages = 65536;
}

int32_t MemoryOS::TLBTranslate(VAddr vAddr)
{
  int32_t phPage = DTLB.translate(vAddr);
  if (phPage == -1)
    return -1;
  return GMemorySystem::calcPAddr(phPage, vAddr);
}

int32_t MemoryOS::ITLBTranslate(VAddr vAddr)
{
  int32_t phPage = ITLB.translate(vAddr);
  if (phPage == -1)
    return -1;
  return GMemorySystem::calcPAddr(phPage, vAddr);
}

PageTable::PageTable()
  : numOSPages("MemoryOS:numOSPages"),
    numUsrPages("MemoryOS:numUsrPages")
{
  invertedPT = new IntlPTEntry[MemoryOS::numPhysicalPages];

  uint32_t tmp_numEntriesPage = GMemorySystem::getPageSize() / BytesPerEntry;

  maskEntriesPage = tmp_numEntriesPage - 1;
  log2EntriesPage = log2i(tmp_numEntriesPage);

  /* We assume a 32 bit virtual address */
  int32_t numL1PTEntries = GMemorySystem::calcPage(0x80000000) >> (log2EntriesPage-1);

  L1PT = new L1IntlPTEntry[numL1PTEntries];

  // L1 Page Table
  invertedPT[0].status = SystemPageStatus | ValidPageStatus; 

  for (uint32_t i = 1; i < MemoryOS::numPhysicalPages; i++)
    invertedPT[i].status = 0;

  for (int32_t i = 0; i < numL1PTEntries; i++)
    L1PT[i].status = 0;
}

PageTable::~PageTable()
{
  delete invertedPT; 
  delete L1PT;
}

void PageTable::assignL1Entry(uint32_t vPage, uint32_t pPage)
{
  L1IntlPTEntry *pL1 = L1PT + getL1EntryNum(vPage);

  pL1->physicalPage = GMemorySystem::calcFullPage(pPage);
  pL1->status = SystemPageStatus | ValidPageStatus;

  invertedPT[pPage].status = SystemPageStatus | ValidPageStatus;

  numOSPages.inc();
}

void PageTable::assignL2Entry(uint32_t vPage, uint32_t pPage)
{
  vToPMap[vPage] = pPage;
  invertedPT[pPage].virtualPage = vPage;
  invertedPT[pPage].status = ValidPageStatus;
  numUsrPages.inc();
}

/* The page least recently copied to the TLB is replaced. Notice that
  if it was moved a long time ago and it has not been replaced from
  the TLB it may be chosen for replacement. The StdMemoryOS takes care
  of this.  Besides we do not check if the page has been modified or
  not; and one should avoid to write back the written pages */ 
PageTable::IntlPTEntry *PageTable::getReplCandidate(uint16_t first_bank, uint16_t search_step)
{ 
  Time_t minTime = globalClock;
  PageTable::IntlPTEntry *pend = invertedPT + MemoryOS::numPhysicalPages;
  PageTable::IntlPTEntry *q = 0;

  for(PageTable::IntlPTEntry *p = invertedPT + first_bank; p < pend; p += search_step) {
    int32_t status = p->status;
    if (!(status & ValidPageStatus))
      return p;
    else
      if (!(status & (SystemPageStatus | PinnedPageStatus)) && p->lastTimeToTLB < minTime) {
	q = p;
	minTime = p->lastTimeToTLB;
      }
  }

  I(q); // Full memory allocated to system, pinned, or in TLB. Unable to perform replacement!

  return q;
}

void PageTable::evictL2Entry(IntlPTEntry *p)
{
  p->status = 0;
  vToPMap.erase(p->virtualPage);
}

StdMemoryOS::StdMemoryOS(GMemorySystem *ms, const char *section)
  : MemoryOS(ms, section)
  ,accessL1PTCB(this)
  ,accessL2PTCB(this)
  ,accessL1PTReq(&accessL1PTCB)
  ,accessL2PTReq(&accessL2PTCB)
  ,nextPhysicalPage(1) //ph page 0 reserved for L1 page table
{ 
  cacheObj = memorySystem->getDataSource();
#ifdef TASKSCALAR
  // cacheObj == L1
  cacheObj = (*cacheObj->getLowerLevel())[0]; // VBus
  cacheObj = (*cacheObj->getLowerLevel())[0]; // Shared L2 (non-version)
  I(cacheObj);
#endif

  minTLBMissDelay = SescConf->getInt(section, "minTLBMissDelay");
  SescConf->isBetween(section, "minTLBMissDelay", 0, 4096);
}

StdMemoryOS::~StdMemoryOS()
{
  if (PT)
    delete PT;
}

void StdMemoryOS::boot()
{
  if(PT==0)
    PT = new PageTable();
}

PageTable::IntlPTEntry *StdMemoryOS::getReplPTEntry()
{ 
  PageTable::IntlPTEntry *p;
  int32_t tmp;

  do {
    p = PT->getReplPhPageL2Entry();
    tmp = GMemorySystem::calcFullPage(p->virtualPage);
    if((TLBTranslate(tmp) != -1 || ITLBTranslate(tmp)) != -1) {
	// if the chosen page was in our TLB we update it in memory
	// so as not to replace it and we repeat the process.
	// Otherwise we could even be replacing a page that has already
	// been translated and involved in a concurrent WR/RD process
	PT->stampPhPage(PT->L2PTEntryToPhPage(p));
	p = 0;
      }
    } while (p == 0);
    return p;
  }

uint32_t StdMemoryOS::getNewPhysicalPage() {
  if (nextPhysicalPage < MemoryOS::numPhysicalPages)
    return nextPhysicalPage++;
  return 0;
}

uint32_t StdMemoryOS::getFreePhysicalPage()
{
  uint32_t ppage = getNewPhysicalPage();
  if (ppage != 0) 
    return ppage;

  PageTable::IntlPTEntry *p = getReplPTEntry();

  // Schedule a write to the memory (traffic model)
  CBMemRequest::create(0
		       ,cacheObj
		       ,MemWrite
		       ,PT->getL2PTEntryPhysicalAddress(p->virtualPage)
		       ,0);

  uint32_t tmp = PT->L2PTEntryToPhPage(p);
  
  PT->evictL2Entry(p);
  
  return tmp;
}

void StdMemoryOS::serviceRequest(MemRequest *mreq)
{
  I(!busy);
  busy = true; // set the busy flag
  startTime = globalClock;

  if(mreq->isDataReq()) {
    missesDTLB.inc();
  }else{
    missesITLB.inc();
  }

  int32_t phAddrL1 = PT->getL1PTEntryPhysicalAddress(GMemorySystem::calcPage(mreq->getVaddr()));

  //  MSG("1.TLB Miss 0x%x page L1PT[0x%x] @%lld", mreq->getVaddr(), phAddrL1, globalClock);

  accessL1PTCB.setParam1(mreq);
  accessL1PTCB.setParam2(phAddrL1);
 
  accessL1PTReq.launch(minTLBMissDelay, cacheObj, MemRead, phAddrL1);
}

 void StdMemoryOS::accessL1PT(MemRequest *origReq, PAddr paddr)
{
  uint32_t vPage = GMemorySystem::calcPage(origReq->getVaddr());

  int32_t l2addr = PT->getL2PTEntryPhysicalAddress(vPage);

  if (l2addr == -1) {
    PT->assignL1Entry(vPage, getFreePhysicalPage());
    CBMemRequest::create(1, cacheObj, MemWrite, paddr, 0);
  }

  //  MSG("2.TLB Miss 0x%x page L2PT[0x%x] @%lld", origReq->getVaddr(), l2addr, globalClock);

  accessL2PTCB.setParam1(origReq);
  accessL2PTCB.setParam2(l2addr);

  accessL2PTReq.launch(1, cacheObj, MemRead, l2addr);
}

void StdMemoryOS::accessL2PT(MemRequest *origReq, PAddr paddr)
{
  VAddr vaddr = origReq->getVaddr();
  
  uint32_t vPage = GMemorySystem::calcPage(vaddr);
  int32_t phPage = PT->translate(vPage);
  
  if (phPage == -1) {
    PT->assignL2Entry(vPage, getFreePhysicalPage());

    CBMemRequest::create(1, cacheObj, MemWrite, paddr, 0);
  }

  completeReq(origReq, vaddr, vPage);
  // reset the busy flag
  busy = false;
  TLBTime.add(globalClock-startTime);
  attemptToEmptyQueue(vaddr, vPage);
}

void StdMemoryOS::attemptToEmptyQueue(uint32_t vaddr, uint32_t phPage)
{
  // attempt a bypass
  int32_t vPage = GMemorySystem::calcPage(vaddr);

  std::vector<MemRequest *>::iterator it = pendingReqs.begin();
  if(it != pendingReqs.end()) {
    MemRequest *mm  = *it;
    int32_t tmpPage  = GMemorySystem::calcPage(mm->getVaddr());
    if(vPage == tmpPage) {
      launchReq(mm, GMemorySystem::calcPAddr(GMemorySystem::calcFullPage(vPage), mm->getVaddr()));
      it = pendingReqs.erase(it);
    }else
      it++ ;
  }

  // wakeup the next request
  if(!pendingReqs.empty()) {
    MemRequest *thisReq = pendingReqs.front();
    pendingReqs.erase(pendingReqs.begin());

    serviceRequest(thisReq);
  }
}

void StdMemoryOS::solveRequest(MemRequest *r)
{
  if (busy) {
    pendingReqs.push_back(r);
  }else{
    serviceRequest(r);
  }
}
