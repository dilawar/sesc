/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze
                  Jose Renau
                  Karin Strauss
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

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <iostream>

#include "nanassert.h"

#include "SescConf.h"
#include "MemorySystem.h"
#include "Cache.h"
#include "MSHR.h"
#include "OSSim.h"
#include "GProcessor.h"
#ifdef TS_TIMELINE
#include "TraceGen.h"
#include "HVersion.h"
#endif

#include "SMPCache.h"


static const char 
  *k_numPorts="numPorts", *k_portOccp="portOccp",
  *k_hitDelay="hitDelay", *k_missDelay="missDelay";

Cache::Cache(MemorySystem *gms, const char *section, const char *name)
  : MemObj(section, name)
#ifdef SESC_SMP
  ,inclusiveCache(SescConf->checkBool(section, "inclusive") ?
		  SescConf->getBool(section, "inclusive") : true)
#else
  ,inclusiveCache(true)
#endif
  ,readHalfMiss("%s:readHalfMiss", name)
  ,writeHalfMiss("%s:writeHalfMiss", name)
  ,writeMiss("%s:writeMiss", name)
  ,readMiss("%s:readMiss", name)
  ,readHit("%s:readHit", name)
  ,writeHit("%s:writeHit", name)
  ,writeBack("%s:writeBack", name)
  ,lineFill("%s:lineFill", name)
  ,linePush("%s:linePush", name)
  ,nForwarded("%s:nForwarded", name)
  ,nWBFull("%s:nWBFull", name)
  ,avgPendingWrites("%s_avgPendingWrites", name)
  ,avgMissLat("%s_avgMissLat", name)
  ,rejected("%s:rejected", name)
  ,rejectedHits("%s:rejectedHits", name)
#ifdef MSHR_BWSTATS
  ,secondaryMissHist("%s:secondaryMissHist", name)
  ,accessesHist("%s:accessHistBySecondaryMiss", name)
  ,mshrBWHist(100, "%s:mshrAcceses", name)
#endif
{
  MemObj *lower_level = NULL;
  char busName[512];
  char tmpName[512];
  
  if(SescConf->checkInt(section,"nBanks")) 
    nBanks = SescConf->getInt(section,"nBanks");
  else
    nBanks = 1;

  int32_t nMSHRsharers = 1;
  if(SescConf->checkInt(section, "nMSHRsharers"))
    nMSHRsharers = SescConf->getInt(section, "nMSHRsharers");

  nAccesses = new GStatsCntr *[nBanks];
  cacheBanks = new CacheType *[nBanks];
  bankMSHRs  = new MSHR<PAddr,Cache> *[nBanks];

  const char* mshrSection = SescConf->getCharPtr(section,"MSHR");

  sprintf(tmpName, "%s_MSHR%d", name, 0);
  cacheBanks[0] = CacheType::create(section, "", tmpName);
  bankMSHRs[0] = MSHR<PAddr,Cache>::create(tmpName, mshrSection);
  nAccesses[0] = new GStatsCntr("%s_B%d:nAccesses", name, 0);
  
  for(int32_t b = 1; b < nBanks; b++) {
    sprintf(tmpName, "%s_B%d", name, b);
    cacheBanks[b] = CacheType::create(section, "", tmpName);

    sprintf(tmpName, "%s_MSHR%d", name, b);

    if((b % nMSHRsharers) == 0)
      bankMSHRs[b] = MSHR<PAddr,Cache>::attach(tmpName, mshrSection, bankMSHRs[0]);
    else
      bankMSHRs[b] = bankMSHRs[b - (b % nMSHRsharers)];

    nAccesses[b] = new GStatsCntr("%s_B%d:nAccesses", name, b);
  }
  
  I(gms);
  lower_level = gms->declareMemoryObj(section, k_lowerLevel);

  if(SescConf->checkBool(section,"SetMSHRL2") 
     && SescConf->getBool(section,"SetMSHRL2")) {
    const char *cName = SescConf->getCharPtr(section,"l2cache");
    Cache *temp = dynamic_cast<Cache*>(gms->searchMemoryObj(true,cName));

    for(int32_t b = 0; b < nBanks; b++) {
      bankMSHRs[b]->setLowerCache(temp);
    }
  }

  int32_t cacheNumPorts = SescConf->getInt(section, k_numPorts);
  int32_t cachePortOccp = SescConf->getInt(section, k_portOccp);
  int32_t bankNumPorts = cacheNumPorts;
  int32_t bankPortOccp = cachePortOccp;

  if(SescConf->checkInt(section, "bankNumPorts")) {
    bankNumPorts = SescConf->getInt(section, "bankNumPorts");
  }
  if(SescConf->checkInt(section, "bankPortOccp")) {
    bankPortOccp = SescConf->getInt(section, "bankPortOccp");
  }

  int32_t mshrNumPorts = bankNumPorts;
  int32_t mshrPortOccp = bankPortOccp;

  if(SescConf->checkInt(section, "mshrNumPorts")) {
    mshrNumPorts = SescConf->getInt(section, "mshrNumPorts");
  }
  if(SescConf->checkInt(section, "mshrPortOccp")) {
    mshrPortOccp = SescConf->getInt(section, "mshrPortOccp");
  }

  cachePort = PortGeneric::create(name, cacheNumPorts, cachePortOccp);
  bankPorts = new PortGeneric *[nBanks];
  mshrPorts = new PortGeneric *[nBanks];

  for(int32_t b = 0; b < nBanks; b++) {
    sprintf(tmpName, "%s_B%d", name, b);
    bankPorts[b] = PortGeneric::create(tmpName, bankNumPorts, bankPortOccp);
    if((b % nMSHRsharers) == 0) {
      sprintf(tmpName, "%s_MSHR_B%d", name, b);
      mshrPorts[b] = PortGeneric::create(tmpName, mshrNumPorts, mshrPortOccp);
    } else {
      mshrPorts[b] = mshrPorts[b - (b % nMSHRsharers)];
    }
  }

  SescConf->isInt(section, k_hitDelay);
  hitDelay = SescConf->getInt(section, k_hitDelay);

  SescConf->isInt(section, k_missDelay);
  missDelay = SescConf->getInt(section, k_missDelay);

  if (SescConf->checkBool(section, "wbfwd"))
    doWBFwd = SescConf->getBool(section, "wbfwd");
  else
    doWBFwd = true;

  if (SescConf->checkInt(section, "fwdDelay")) 
    fwdDelay = SescConf->getInt(section, "fwdDelay");
  else
    fwdDelay = missDelay;

  pendingWrites = 0;
  if (SescConf->checkInt(section, "maxWrites")) 
    maxPendingWrites = SescConf->getInt(section, "maxWrites");
  else
    maxPendingWrites = -1;
  
  defaultMask  = ~(cacheBanks[0]->getLineSize()-1);


#ifdef MSHR_BWSTATS
  if(SescConf->checkBool(section, "parallelMSHR")) {
    parallelMSHR = SescConf->getBool(section, "parallelMSHR");
  } else {
    parallelMSHR = false;
  }
#endif

  if (lower_level != NULL)
      addLowerLevel(lower_level);
}

Cache::~Cache()
{
  delete [] nAccesses;
  for(int32_t b = 0; b < nBanks; b++)
    cacheBanks[b]->destroy();
  delete [] cacheBanks;
  delete [] bankMSHRs;
  delete [] bankPorts;
  delete [] mshrPorts;
}

void Cache::access(MemRequest *mreq) 
{
  mreq->setClockStamp((Time_t) - 1);
  if(mreq->getPAddr() <= 1024) { // TODO: need to implement support for fences
    mreq->goUp(0); 
    return;
  }

  nAccesses[getBankId(mreq->getPAddr())]->inc();

  switch(mreq->getMemOperation()){
  case MemReadW:
  case MemRead:    read(mreq);       break;
  case MemWrite:   write(mreq);      break;
  case MemPush:    pushLine(mreq);   break;
  default:         specialOp(mreq);  break;
  }
}

void Cache::read(MemRequest *mreq)
{ 
#ifdef MSHR_BWSTATS
  if(parallelMSHR)
    mshrBWHist.inc();
#endif
  //enforcing max ops/cycle for the specific bank
  doReadBankCB::scheduleAbs(nextBankSlot(mreq->getPAddr()), this, mreq);
}

void Cache::doReadBank(MemRequest *mreq)
{ 
  // enforcing max ops/cycle for the whole cache
  doReadCB::scheduleAbs(nextCacheSlot(), this, mreq);
}

void Cache::doRead(MemRequest *mreq)
{
  Line *l = getCacheBank(mreq->getPAddr())->readLine(mreq->getPAddr());

  if (l == 0) {
    if(isInWBuff(mreq->getPAddr())) {
      nForwarded.inc();
      mreq->goUp(fwdDelay);
      return;
    }
    readMissHandler(mreq);
    return;
  }

  readHit.inc();
  l->incReadAccesses();

  mreq->goUp(hitDelay);
}

void Cache::readMissHandler(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();

#ifdef MSHR_BWSTATS
  if(!parallelMSHR)
    mshrBWHist.inc();
#endif


  mreq->setClockStamp(globalClock);
  nextMSHRSlot(addr); // checking if there is a pending miss
  if(!getBankMSHR(addr)->issue(addr, MemRead)) {
    getBankMSHR(addr)->addEntry(addr, doReadQueuedCB::create(this, mreq), 
                                activateOverflowCB::create(this, mreq), MemRead);
    return;
  }

  // Added a new MSHR entry, now send request to lower level
  readMiss.inc();
  sendMiss(mreq);
}

void Cache::doReadQueued(MemRequest *mreq)
{
  PAddr paddr = mreq->getPAddr();
  readHalfMiss.inc();


  avgMissLat.sample(globalClock - mreq->getClockStamp());
  mreq->goUp(hitDelay);

  // the request came from the MSHR, we need to retire it
  getBankMSHR(paddr)->retire(paddr);
}

// this is here just because we have no feedback from the memory
// system to the processor telling it to stall the issue of a request
// to the memory system. this will only be called if an overflow entry
// is activated in the MSHR and the is *no* pending request for the
// same line.  therefore, sometimes the callback associated with this
// function might not be called.
void Cache::activateOverflow(MemRequest *mreq)
{
  if(mreq->getMemOperation() == MemRead || mreq->getMemOperation() == MemReadW) {

    Line *l = getCacheBank(mreq->getPAddr())->readLine(mreq->getPAddr());
    
    if (l == 0) {
      // no need to add to the MSHR, it is already there
      // since it came from the overflow
      readMiss.inc();
      sendMiss(mreq);
      return;
    }

    readHit.inc();
    PAddr paddr = mreq->getPAddr();
    mreq->goUp(hitDelay);
    
    // the request came from the MSHR overflow, we need to retire it
    getBankMSHR(paddr)->retire(paddr);
    
    return;
  }
    
  I(mreq->getMemOperation() == MemWrite);

  Line *l = getCacheBank(mreq->getPAddr())->writeLine(mreq->getPAddr());

  if (l == 0) {
    // no need to add to the MSHR, it is already there
    // since it came from the overflow
    writeMiss.inc();
    sendMiss(mreq);
    return;
  }

  writeHit.inc();
#ifndef SESC_SMP
  l->makeDirty();
#endif

  PAddr paddr = mreq->getPAddr();
  mreq->goUp(hitDelay);
  
  // the request came from the MSHR overflow, we need to retire it
  wbuffRemove(paddr);
  getBankMSHR(paddr)->retire(paddr);
}

void Cache::write(MemRequest *mreq)
{ 
#ifdef MSHR_BWSTATS
  if(parallelMSHR)
    mshrBWHist.inc();
#endif
  doWriteBankCB::scheduleAbs(nextBankSlot(mreq->getPAddr()), this, mreq);
}

void Cache::doWriteBank(MemRequest *mreq)
{
  doWriteCB::scheduleAbs(nextCacheSlot(), this, mreq);
}

void Cache::doWrite(MemRequest *mreq)
{
  Line *l = getCacheBank(mreq->getPAddr())->writeLine(mreq->getPAddr());

  if (l == 0) {
    writeMissHandler(mreq);
    return;
  }

  writeHit.inc();
  l->makeDirty();

  mreq->goUp(hitDelay);
}

void Cache::writeMissHandler(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();

#ifdef MSHR_BWSTATS
  if(!parallelMSHR)
    mshrBWHist.inc();
#endif

  wbuffAdd(addr);
  mreq->setClockStamp(globalClock);

  if(!getBankMSHR(addr)->issue(addr, MemWrite)) {
    getBankMSHR(addr)->addEntry(addr, doWriteQueuedCB::create(this, mreq),
                                activateOverflowCB::create(this, mreq), MemWrite);
    return;
  }

  writeMiss.inc();
  sendMiss(mreq);
}

void Cache::doWriteQueued(MemRequest *mreq)
{
  PAddr paddr = mreq->getPAddr();

  writeHalfMiss.inc();

  avgMissLat.sample(globalClock - mreq->getClockStamp());
  mreq->goUp(hitDelay);

  wbuffRemove(paddr);

  // the request came from the MSHR, we need to retire it
  getBankMSHR(paddr)->retire(paddr);
}

void Cache::specialOp(MemRequest *mreq)
{
  mreq->goUp(1);
}

void Cache::returnAccess(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();

  // the MSHR needs to be accesed when the data comes back
#ifdef MSHR_BWSTATS
  mshrBWHist.inc();

  // extra BW usage of MSHR for each pending write
  // this does NOT hold for WT caches, so it is under an ifdef for now
  int32_t nPendWrites = getBankMSHR(addr)->getUsedWrites(addr);
  for(int32_t iw = 0; iw < nPendWrites; iw++) {
    mshrBWHist.inc();
    nextMSHRSlot(addr);
  }
#endif

  preReturnAccessCB::scheduleAbs(nextMSHRSlot(addr), this, mreq);
}

void Cache::preReturnAccess(MemRequest *mreq)
{
  if (mreq->getMemOperation() == MemPush) {
    mreq->goUp(0);
    return;
  }

  PAddr addr = mreq->getPAddr();


  Line *l = getCacheBank(mreq->getPAddr())->writeLine(addr);

  if (l == 0) {
    nextBankSlot(addr); // had to check the bank if it can accept the new line
    CallbackBase *cb = doReturnAccessCB::create(this, mreq);
    l = allocateLine(mreq->getPAddr(), cb);
    
    if(l != 0) { 
      // the allocation was successfull, no need for the callback
      cb->destroy();
    } else {
      // not possible to allocate a line, will be called back later
      return;
    }
  }

  int32_t nPendReads = getBankMSHR(addr)->getUsedReads(addr);

#ifdef MSHR_BWSTATS
  mshrBWHist.inc();

  // extra BW usage of MSHR for each 4 pending reads
  // this does NOT hold for WT caches, so it is under an ifdef for now
  for(int32_t ir = 0; ir < (nPendReads/4); ir++) {
    mshrBWHist.inc();
    nextMSHRSlot(addr);
  }
#endif

  doReturnAccess(mreq);
  l->setReadMisses(nPendReads);
}

void Cache::doReturnAccess(MemRequest *mreq)
{
  I(0);
}

Cache::Line *Cache::allocateLine(PAddr addr, CallbackBase *cb)
{
  PAddr rpl_addr=0;
  I(getCacheBank(addr)->findLineDebug(addr) == 0);
  Line *l = getCacheBank(addr)->fillLine(addr, rpl_addr);
  lineFill.inc();

  if(l == 0) {
    doAllocateLineRetryCB::scheduleAbs(globalClock + 100, this, addr, cb);
    return 0;
  }

  if(!l->isValid()) {
    l->validate();
    return l;
  }

  // line was valid
  // at the L1, just write back if dirty and use line
  // in non-inclusiveCache, no need to invalidate line in upper levels
  if(isHighestLevel() || !inclusiveCache) { 
#ifdef MSHR_BWSTATS
    // update some eviction stats
    secondaryMissHist.sample(l->getReadMisses(), 1);
    accessesHist.sample(l->getReadMisses(), l->getReadAccesses());
#endif
    
    if(l->isDirty()) {
      doWriteBack(rpl_addr);
      l->makeClean();
    }
    l->validate();
    return l;
  }

  I(inclusiveCache);

  // not highest level or non-inclusive, 
  // must invalidate old line, which may take a while
  l->lock();
  I(pendInvTable.find(rpl_addr) == pendInvTable.end());
  pendInvTable[rpl_addr].outsResps = getNumCachesInUpperLevels();
  pendInvTable[rpl_addr].cb = doAllocateLineCB::create(this, addr, rpl_addr, cb);
  invUpperLevel(rpl_addr, cacheBanks[0]->getLineSize(), this);
  return 0;
}

void Cache::doAllocateLine(PAddr addr, PAddr rpl_addr, CallbackBase *cb)
{
  Line *l = getCacheBank(addr)->findLineNoEffect(addr);
  I(l);

#ifdef MSHR_BWSTATS
  // update some eviction stats
  secondaryMissHist.sample(l->getReadMisses(), 1);
  accessesHist.sample(l->getReadMisses(), l->getReadAccesses());
#endif

  if(l->isDirty()) {
    doWriteBack(rpl_addr);
    l->makeClean();
  }

  l->validate();

  I(cb);
  cb->call();
}

void Cache::doAllocateLineRetry(PAddr addr, CallbackBase *cb)
{
  Line *l = allocateLine(addr, cb);
  if(l) 
    cb->call();
}

bool Cache::canAcceptStore(PAddr addr)
{
  if(maxPendingWrites > 0 && pendingWrites >= maxPendingWrites) {
    nWBFull.inc();
    return false;
  }

  bool canAcceptReq = getBankMSHR(addr)->canAcceptRequest(addr, MemWrite);

  rejectedHits.add(!canAcceptReq && isInCache(addr) ? 1: 0);
  rejected.add(!canAcceptReq);

  return canAcceptReq;
}

bool Cache::canAcceptLoad(PAddr addr) 
{
  bool canAcceptReq = getBankMSHR(addr)->canAcceptRequest(addr, MemRead);

  rejectedHits.add(!canAcceptReq && isInCache(addr) ? 1: 0);
  rejected.add(!canAcceptReq);

  return canAcceptReq;
}

bool Cache::isInCache(PAddr addr) const
{
  uint32_t index = getCacheBank(addr)->calcIndex4Addr(addr);
  uint32_t tag = getCacheBank(addr)->calcTag(addr);

  // check the cache not affecting the LRU state
  for(uint32_t i = 0; i < cacheBanks[0]->getAssoc(); i++) {
    Line *l = getCacheBank(addr)->getPLine(index+i);
    if(l->getTag() == tag)
      return true;
  }
  return false;
}

void Cache::invalidate(PAddr addr, ushort size, MemObj *lowerCache)
{ 
  I(lowerCache);
  I(pendInvTable.find(addr) == pendInvTable.end());
  pendInvTable[addr].outsResps = getNumCachesInUpperLevels(); 
  pendInvTable[addr].cb = doInvalidateCB::create(lowerCache, addr, size);

  if (isHighestLevel()) {     // highest level, we have only to 
    doInvalidate(addr, size); // invalidate the current cache
    return;
  }
    
  invUpperLevel(addr, size, this);
}

void Cache::doInvalidate(PAddr addr, ushort size)
{
  I(pendInvTable.find(addr) != pendInvTable.end());
  CallbackBase *cb = 0;

  PendInvTable::iterator it = pendInvTable.find(addr);
  Entry *record = &(it->second);
  record->outsResps--;

  if(record->outsResps <= 0) {
     cb = record->cb;
     pendInvTable.erase(addr);
  }

  int32_t leftSize = size; // use signed because cacheline can be bigger
  while (leftSize > 0) {
    Line *l = getCacheBank(addr)->readLine(addr);
    
    if(l){
      nextBankSlot(addr); // writing the INV bit in a Bank's line
      I(l->isValid());
      if(l->isDirty()) {
        doWriteBack(addr);
        l->makeClean();
      }
      l->invalidate();
    } 

    addr += cacheBanks[0]->getLineSize();
    leftSize -= cacheBanks[0]->getLineSize();
  }

  // finished sending dirty lines to lower level, 
  // wake up callback
  // should take at least as much as writeback (1)
  if(cb)
    EventScheduler::schedule((TimeDelta_t) 2,cb);
}

Time_t Cache::getNextFreeCycle() const // TODO: change name to calcNextFreeCycle
{
  return cachePort->calcNextSlot();
}

void Cache::doWriteBack(PAddr addr)
{
  I(0);
}

void Cache::sendMiss(MemRequest *mreq)
{
  I(0);
}

void Cache::wbuffAdd(PAddr addr)
{
  ID2(WBuff::const_iterator it =wbuff.find(addr));
  
  if(!doWBFwd)
    return;

  wbuff[addr]++;
  pendingWrites++;
  avgPendingWrites.sample(pendingWrites);

  GI(it == wbuff.end(), wbuff[addr] == 1);
}

void Cache::wbuffRemove(PAddr addr)
{
  WBuff::iterator it = wbuff.find(addr);
  if(it == wbuff.end())
    return;

  pendingWrites--;
  I(pendingWrites >= 0);

  I(it->second > 0);
  it->second--;
  if(it->second == 0)
    wbuff.erase(addr);
}

bool Cache::isInWBuff(PAddr addr)
{
  return (wbuff.find(addr) != wbuff.end());
}

void Cache::dump() const
{
  double total =   readMiss.getDouble()  + readHit.getDouble()
    + writeMiss.getDouble() + writeHit.getDouble() + 1;

  MSG("%s:total=%8.0f:rdMiss=%8.0f:rdMissRate=%5.3f:wrMiss=%8.0f:wrMissRate=%5.3f"
      ,symbolicName
      ,total
      ,readMiss.getDouble()
      ,100*readMiss.getDouble()  / total
      ,writeMiss.getDouble()
      ,100*writeMiss.getDouble() / total
      );
}

//
// WBCache: Write back cache, only writes to lower level on displacements
//
WBCache::WBCache(MemorySystem *gms, const char *descr_section, 
        const char *name) 
  : Cache(gms, descr_section, name) 
{
  // nothing to do
}
WBCache::~WBCache() 
{
  // nothing to do
}

void WBCache::pushLine(MemRequest *mreq)
{
  // here we are pretending there is an outstanding pushLine 
  // buffer so operation finishes immediately

  // Line has to be pushed from a higher level to this level
  I(!isHighestLevel());

  linePush.inc();

  nextBankSlot(mreq->getPAddr());
  Line *l = getCacheBank(mreq->getPAddr())->writeLine(mreq->getPAddr());

  if (inclusiveCache || l != 0) {
    // l == 0 if the upper level is sending a push due to a
    // displacement of the lower level
    if (l != 0) {
      l->validate();
      l->makeDirty();
    }
  
    nextCacheSlot();
    mreq->goUp(0);
    return;
  }

  // cache is not inclusive and line does not exist: push line directly to lower level
  I(!inclusiveCache && l == 0);
  mreq->goDown(0, lowerLevel[0]);  
}

void WBCache::sendMiss(MemRequest *mreq)
{
#ifdef TS_TIMELINE
  if (mreq->getDInst())
    TraceGen::add(mreq->getVersionRef()->getId(),"%sMiss=%lld"
                  ,symbolicName
                  ,globalClock
                  );
//     TraceGen::add(mreq->getVersionRef()->getId(),"%sMiss=%lld:%sPC=0x%x:%sAddr=0x%x"
//                ,symbolicName
//                ,globalClock
//                ,symbolicName
//                ,mreq->getDInst()->getInst()->getAddr()
//                ,symbolicName
//                ,mreq->getPAddr()
//                );
#endif

  mreq->mutateWriteToRead();
  mreq->goDown(missDelay + (nextMSHRSlot(mreq->getPAddr())-globalClock), 
               lowerLevel[0]);
}

void WBCache::doWriteBack(PAddr addr) 
{
  writeBack.inc();
  //FIXME: right now we are assuming all lines are the same size
  CBMemRequest::create(1, lowerLevel[0], MemPush, addr, 0); 
}

void WBCache::doReturnAccess(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();
  Line *l = getCacheBank(addr)->findLineNoEffect(addr);
  I(l);
  l->validate();

  if(mreq->getMemOperation() != MemRead) {
    l->makeDirty();
    wbuffRemove(addr);
  }
  
  avgMissLat.sample(globalClock - mreq->getClockStamp());
  mreq->goUp(0);
  
  getBankMSHR(addr)->retire(addr);
}


// WTCache: Write through cache, always propagates writes down

WTCache::WTCache(MemorySystem *gms, const char *descr_section, 
                 const char *name) 
  : Cache(gms, descr_section, name) 
{
  // nothing to do 
}

WTCache::~WTCache()
{
  // nothing to do
}

void WTCache::doWrite(MemRequest *mreq)
{
  Line *l = getCacheBank(mreq->getPAddr())->writeLine(mreq->getPAddr());

  if(l == 0) {
    writeMissHandler(mreq);
    return;
  }

  writeHit.inc();

  writePropagateHandler(mreq); // this is not a proper miss, but a WT cache 
                               // always propagates writes down
}

void WTCache::writePropagateHandler(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();

  wbuffAdd(addr);

  if(!getBankMSHR(addr)->issue(addr))
    getBankMSHR(addr)->addEntry(addr, reexecuteDoWriteCB::create(this, mreq),
                   /*doWriteQueuedCB::create(this, mreq)*/
                   reexecuteDoWriteCB::create(this, mreq));
  else
    propagateDown(mreq);
}

void WTCache::reexecuteDoWrite(MemRequest *mreq)
{
  Line *l = getCacheBank(mreq->getPAddr())->findLine(mreq->getPAddr());

  if(l == 0)
    sendMiss(mreq);
  else
    propagateDown(mreq);
}

void WTCache::propagateDown(MemRequest *mreq)
{
  mreq->goDown(missDelay + (nextMSHRSlot(mreq->getPAddr()) - globalClock), 
               lowerLevel[0]);
}

void WTCache::sendMiss(MemRequest *mreq)
{
  Line *l = getCacheBank(mreq->getPAddr())->readLine(mreq->getPAddr());
  if (!l)
    mreq->mutateWriteToRead();
  mreq->goDown(missDelay, lowerLevel[0]);
}

void WTCache::doWriteBack(PAddr addr) 
{
  I(0);
  // nothing to do
}

void WTCache::pushLine(MemRequest *mreq)
{
  I(0); // should never be called
}

void WTCache::doReturnAccess(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();
  Line *l = getCacheBank(addr)->findLineNoEffect(addr);
  I(l);
  l->validate();

  if(mreq->getMemOperation() != MemRead) {
    wbuffRemove(addr);
  }
  
  avgMissLat.sample(globalClock - mreq->getClockStamp());
  mreq->goUp(0);
  
  getBankMSHR(addr)->retire(addr);
}

void WTCache::inclusionCheck(PAddr addr) {
  const LevelType* la  = getLowerLevel();
  MemObj*    c  = (*la)[0];
  const LevelType* lb = c->getLowerLevel();
  MemObj*    cc = (*lb)[0];
  //I(((SMPCache*)cc)->findLine(addr));
}

//SVCache versioning speculative cache 
//only for cava for now

SVCache::SVCache(MemorySystem *gms, const char *descr_section, 
                 const char *name)
  : WBCache(gms, descr_section, name) ,
    nInvLines("%s:nInvLines", name),
    nCommitedLines("%s:nCommitedLines", name),
    nSpecOverflow("%s:nSpecOverflow", name)
{
  if(SescConf->checkBool(descr_section,"doMSHROpt")) 
    doMSHRopt = SescConf->getBool(descr_section,"doMSHROpt");
}

SVCache::~SVCache() 
{
}

void SVCache::doWrite(MemRequest *mreq)
{
  DInst *dinst = mreq->getDInst();
  PAddr addr = mreq->getPAddr();

  
  WBCache::doWrite(mreq);

}

void SVCache::doReturnAccess(MemRequest *mreq)
{
  DInst *dinst = mreq->getDInst();
  PAddr addr = mreq->getPAddr();


  WBCache::doReturnAccess(mreq);

}

void SVCache::preReturnAccess(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();

  if(doMSHRopt && getBankMSHR(addr)->isOnlyWrites(addr)) {
    //do the only writes optimization here

    wbuffRemove(addr);

    avgMissLat.sample(globalClock - mreq->getClockStamp());
    mreq->goUp(0);
  
    nextMSHRSlot(addr);
    getBankMSHR(addr)->retire(addr);
    doWriteBack(addr);
    return;
  }

  if (mreq->getMemOperation() != MemPush) {
    // checking if the whole set is locked. 
    uint32_t index = getCacheBank(addr)->calcIndex4Addr(addr);
    bool allLocked = true;
    Line *lastLine = 0;
    for(uint32_t i = 0; i < getCacheBank(addr)->getAssoc(); i++) {
      Line *l = getCacheBank(addr)->getPLine(index+i);
      if(l->isValid() && l->isSpec()) {
        I(l->isLocked());
        lastLine = l;
      } else {
        allLocked = false;
        break;
      }
    }

    if(allLocked && lastLine) {
      // very bad solution. this is just do avoid having the
      // cache tell the processor to restart. it should be rare
      lastLine->invalidate();
      nSpecOverflow.inc();
    }
  }

  WBCache::preReturnAccess(mreq);
}
void SVCache::ckpRestart(uint32_t ckpId)
{
  for(int32_t b = 0; b < nBanks; b++) {
         for(uint32_t i = 0; i < cacheBanks[b]->getNumLines(); i++) {
      Line *l = cacheBanks[b]->getPLine(i);
      if(l->isValid() && l->isSpec()) {
        I(l->isLocked());
        if(l->getCkpId() == ckpId) {
          nInvLines.inc();
          l->invalidate();
        }
      }
    }
  }
}

void SVCache::ckpCommit(uint32_t ckpId)
{
  for(int32_t b = 0; b < nBanks; b++) {
         for(uint32_t i = 0; i < cacheBanks[b]->getNumLines(); i++) {
      Line *l = cacheBanks[b]->getPLine(i);
      if(l->isValid() && l->isSpec()) {
        I(l->isLocked());
        if(l->getCkpId() == ckpId) {
          nCommitedLines.inc();
          l->setSpec(false);
          l->setCkpId(0);
          l->validate(); //unlock
        }
      }
    }
  }
}

// NICECache is a cache that always hits

NICECache::NICECache(MemorySystem *gms, const char *section, const char *name)
  : Cache(gms, section, name) 
{
}

NICECache::~NICECache()
{
  // Nothing to do
}

void NICECache::read(MemRequest *mreq)
{
  readHit.inc(); 
  mreq->goUp(hitDelay);
}

void NICECache::write(MemRequest *mreq)
{
  writeHit.inc();
  mreq->goUp(hitDelay);
}

void NICECache::returnAccess(MemRequest *mreq)
{
  I(0);
}

void NICECache::sendMiss(MemRequest *mreq)
{
  I(0);
}

void NICECache::doWriteBack(PAddr addr)
{
  I(0);
}

void NICECache::specialOp(MemRequest *mreq)
{
  mreq->goUp(1);
}

void NICECache::pushLine(MemRequest *mreq)
{
  linePush.inc();
  mreq->goUp(0);
}
