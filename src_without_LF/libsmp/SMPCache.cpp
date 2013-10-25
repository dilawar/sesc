/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Karin Strauss

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

#include "SMPCache.h"
#include "SMPCacheState.h"
#include "Cache.h"

#include "MESIProtocol.h"

// This cache works under the assumption that caches above it in the memory
// hierarchy are write-through caches

// There is a major simplifying assumption for this cache subsystem:
// if there are two requests to the same cache line at the same time,
// only one of them is propagated to the lower levels. Once this
// request is serviced, the other one will be allowed to proceed. This
// is implemented by the mutExclBuffer.

// When this cache is not used as the last level of caching before
// memory, we are assuming all levels between this cache and memory
// are not necessarily inclusive. This does not mean they are
// exclusive.

// Another important assumption is that every cache line in the memory
// subsystem is the same size. This is not hard to fix, though. Any
// candidates?

MSHR<PAddr, SMPCache> *SMPCache::mutExclBuffer = NULL;

#ifdef SESC_ENERGY
unsigned SMPCache::cacheID = 0;
#endif

SMPCache::SMPCache(SMemorySystem *dms, const char *section, const char *name)
  : MemObj(section, name)
  , readHit("%s:readHit", name)
  , writeHit("%s:writeHit", name)
  , readMiss("%s:readMiss", name)
  , writeMiss("%s:writeMiss", name)  
  , readHalfMiss("%s:readHalfMiss", name)
  , writeHalfMiss("%s:writeHalfMiss", name)
  , writeBack("%s:writeBack", name)
  , linePush("%s:linePush", name)
  , lineFill("%s:lineFill", name) 
  , readRetry("%s:readRetry", name)
  , writeRetry("%s:writeRetry", name)
  , invalDirty("%s:invalDirty", name)
  , allocDirty("%s:allocDirty", name)
{
  MemObj *lowerLevel = NULL;

  I(dms);
  lowerLevel = dms->declareMemoryObj(section, "lowerLevel");

  if (lowerLevel != NULL)
    addLowerLevel(lowerLevel);

  cache = CacheType::create(section, "", name);
  I(cache);

  const char *prot = SescConf->getCharPtr(section, "protocol");
  if(!strcasecmp(prot, "MESI")) {
    protocol = new MESIProtocol(this, name);
  } else {
    MSG("unknown protocol, using MESI");
    protocol = new MESIProtocol(this, name);    
  }

  SescConf->isInt(section, "numPorts");
  SescConf->isInt(section, "portOccp");

  cachePort = PortGeneric::create(name, 
                                  SescConf->getInt(section, "numPorts"), 
                                  SescConf->getInt(section, "portOccp"));

  // MSHR is used as an outstanding request buffer
  // even hits are added to MSHR
  char *outsReqName = (char *) malloc(strlen(name) + 2);
  sprintf(outsReqName, "%s", name);

  const char *mshrSection = SescConf->getCharPtr(section,"MSHR");
  
  outsReq = MSHR<PAddr,SMPCache>::create(outsReqName, mshrSection);

  if (mutExclBuffer == NULL)
    mutExclBuffer = MSHR<PAddr,SMPCache>::create("mutExclBuffer", 
				  SescConf->getCharPtr(mshrSection, "type"),
                                  32000,
                                  SescConf->getInt(mshrSection, "bsize"));
  
  SescConf->isInt(section, "hitDelay");
  hitDelay = SescConf->getInt(section, "hitDelay");

  SescConf->isInt(section, "missDelay");
  missDelay = SescConf->getInt(section, "missDelay");

#ifdef SESC_ENERGY

  myID = cacheID;
  cacheID++;

  rdEnergy[0] = new GStatsEnergy("rdHitEnergy",name
                                 ,myID
                                 ,MemPower
                                 ,EnergyMgr::get(section,"rdHitEnergy"));
    
  rdEnergy[1] = new GStatsEnergy("rdMissEnergy",name
                                 ,myID
                                 ,MemPower
                                 ,EnergyMgr::get(section,"rdMissEnergy"));

  wrEnergy[0]  = new GStatsEnergy("wrHitEnergy",name
                                  ,myID
                                  ,MemPower
                                  ,EnergyMgr::get(section,"wrHitEnergy"));
  
  wrEnergy[1] = new GStatsEnergy("wrMissEnergy",name
                                 ,myID
                                 ,MemPower
                                 ,EnergyMgr::get(section,"wrMissEnergy"));
#endif
}

SMPCache::~SMPCache() 
{
  // do nothing
}

Time_t SMPCache::getNextFreeCycle() const
{
  return cachePort->calcNextSlot();
}

bool SMPCache::canAcceptStore(PAddr addr)
{
  return outsReq->canAcceptRequest(addr);
}

void SMPCache::access(MemRequest *mreq)
{
  PAddr addr;
  IS(addr = mreq->getPAddr());

  GMSG(mreq->getPAddr() < 1024,
       "mreq dinst=0x%p paddr=0x%x vaddr=0x%x memOp=%d ignored",
       mreq->getDInst(),
       (uint32_t) mreq->getPAddr(),
       (uint32_t) mreq->getVaddr(),
       mreq->getMemOperation());
  
  I(addr >= 1024); 

  switch(mreq->getMemOperation()){
  case MemRead:  read(mreq);          break; 
  case MemWrite: /*I(cache->findLine(mreq->getPAddr())); will be transformed
                   to MemReadW later */
  case MemReadW: write(mreq);         break; 
  case MemPush:  I(0);                break; // assumed write-through upperlevel
  default:       specialOp(mreq);     break;
  }

  // for reqs coming from upper level:
  // MemRead  means "I want to read"
  // MemReadW means "I want to write, and I missed"
  // MemWrite means "I want to write, and I hit"
  // MemPush  means "I am writing data back" 
  // (this will never happen if upper level is write-through cache,
  // which is what we are assuming)
}

void SMPCache::read(MemRequest *mreq)
{  
  PAddr addr = mreq->getPAddr();

  if (!outsReq->issue(addr)) {
    outsReq->addEntry(addr, doReadCB::create(this, mreq), 
                            doReadCB::create(this, mreq));
    readHalfMiss.inc();
    return;
  }

  doReadCB::scheduleAbs(nextSlot(), this, mreq);
}

void SMPCache::doRead(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();
  Line *l = cache->readLine(addr);

  if (l && l->canBeRead()) {
    readHit.inc();
#ifdef SESC_ENERGY
    rdEnergy[0]->inc();
#endif    
    outsReq->retire(addr);
    mreq->goUp(hitDelay);
    return;
  }
    
  if (l && l->isLocked()) {
    readRetry.inc();
    Time_t nextTry = nextSlot();
    if (nextTry == globalClock)
      nextTry++;
    doReadCB::scheduleAbs(nextTry, this, mreq);
    return;
  }

  GI(l, !l->isLocked());

  readMiss.inc(); 

#ifdef SESC_ENERGY
  rdEnergy[1]->inc();
#endif

  if (!mutExclBuffer->issue(addr)) {
    mutExclBuffer->addEntry(addr, sendReadCB::create(this, mreq),
                                  sendReadCB::create(this, mreq));
    return;
  }

  sendRead(mreq);
}

void SMPCache::sendRead(MemRequest* mreq)
{
  protocol->read(mreq);
}

void SMPCache::write(MemRequest *mreq)
{  
  PAddr addr = mreq->getPAddr();
  
  if (!outsReq->issue(addr)) {
    outsReq->addEntry(addr, doWriteCB::create(this, mreq), 
                            doWriteCB::create(this, mreq));
    writeHalfMiss.inc();
    return;
  }
  
  doWriteCB::scheduleAbs(nextSlot(), this, mreq);
}

void SMPCache::doWrite(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();
  Line *l = cache->writeLine(addr);

  if (l && l->canBeWritten()) {
    writeHit.inc();
#ifdef SESC_ENERGY
    wrEnergy[0]->inc();
#endif
    protocol->makeDirty(l);
    outsReq->retire(addr);
    mreq->goUp(hitDelay);  
    return;
  }

  if (l && l->isLocked()) {
    writeRetry.inc();
    mreq->mutateWriteToRead();
    Time_t nextTry = nextSlot();
    if (nextTry == globalClock)
      nextTry++;
    doWriteCB::scheduleAbs(nextTry, this, mreq);
    return;
  }

  GI(l, !l->isLocked());

  // this should never happen unless this is highest level because
  // SMPCache is inclusive of all other caches closer to the
  // processor; there is only one case in which this could happen when
  // SMPCache is used as an L2: 1) line is written in L1 and scheduled
  // to go down to L2 2) line is invalidated in both L1 and L2 3)
  // doWrite in L2 is executed after line is invalidated
  if(!l && mreq->getMemOperation() == MemWrite) {
    mreq->mutateWriteToRead();
  }

  writeMiss.inc();

#ifdef SESC_ENERGY
  wrEnergy[1]->inc();
#endif

  if (!mutExclBuffer->issue(addr)) {
    mutExclBuffer->addEntry(addr, sendWriteCB::create(this, mreq),
                                  sendWriteCB::create(this, mreq));
    return;
  }

  sendWrite(mreq);
}

void SMPCache::sendWrite(MemRequest* mreq)
{
  protocol->write(mreq);
}

void SMPCache::doWriteBack(PAddr addr)
{
  // FIXME: right now we are assuming cache line sizes are same in every cache

  writeBack.inc();
  protocol->sendWriteBack(addr, concludeWriteBackCB::create(this, globalClock));
}

void SMPCache::concludeWriteBack(Time_t initialTime)
{
  // add here actions that need to be performed after writeback is
  // acknoledged by memory
}

void SMPCache::specialOp(MemRequest *mreq)
{
  mreq->goUp(1); 
}

void SMPCache::invalidate(PAddr addr, ushort size, MemObj *oc)
{
  Line *l = cache->findLine(addr);

  I(oc);
  I(pendInvTable.find(addr) == pendInvTable.end());
  pendInvTable[addr].outsResps = getNumCachesInUpperLevels();
  pendInvTable[addr].cb = doInvalidateCB::create(oc, addr, size);
  pendInvTable[addr].invalidate = true;
  //  pendInvTable[addr].writeback = true;

  if (l)
    protocol->preInvalidate(l);

  if (!isHighestLevel()) {
    invUpperLevel(addr, size, this);
    return;
  }

  doInvalidate(addr, size);
}

void SMPCache::doInvalidate(PAddr addr, ushort size)
{
  I(pendInvTable.find(addr) != pendInvTable.end());
  CallbackBase *cb = 0;
  bool invalidate = false;
  bool writeBack = false;

  PendInvTable::iterator it = pendInvTable.find(addr);
  Entry *record = &(it->second);
  if(--(record->outsResps) <= 0) {
    cb = record->cb;
    invalidate = record->invalidate;
    writeBack = record->writeback;
    pendInvTable.erase(addr);
  }

  if(invalidate)
    realInvalidate(addr, size, writeBack);

  if(cb)
    EventScheduler::schedule((TimeDelta_t) 2, cb);
}

void SMPCache::realInvalidate(PAddr addr, ushort size, bool writeBack)
{
  while(size) {

    Line *l = cache->findLine(addr);
    
    if (l) {
      nextSlot(); // counts for occupancy to invalidate line
      I(l->isValid());
      if (l->isDirty()) {
        invalDirty.inc();
	if(writeBack)
	  doWriteBack(addr);
      } 
      l->invalidate();
    }
    addr += cache->getLineSize();
    size -= cache->getLineSize();
  }
}

// interface with protocol

// sends a request to lower level
void SMPCache::sendBelow(SMPMemRequest *sreq)
{
  sreq->goDown(missDelay, lowerLevel[0]);
}

// sends a response to lower level
// writes data back if protocol determines so
void SMPCache::respondBelow(SMPMemRequest *sreq)
{
  PAddr addr = sreq->getPAddr();

  if(sreq->getSupplier() == this && sreq->needsWriteDown()) {
    doWriteBack(addr);
  }

  lowerLevel[0]->access(sreq);
}

// receives requests from other caches
void SMPCache::receiveFromBelow(SMPMemRequest *sreq) {
  doReceiveFromBelowCB::scheduleAbs(nextSlot() + missDelay, this, sreq);
}

void SMPCache::doReceiveFromBelow(SMPMemRequest *sreq)
{
  MemOperation memOp = sreq->getMemOperation();

  // request from other caches, need to process them accordingly

  if(memOp == MemRead) {
    protocol->readMissHandler(sreq);
    return;
  }

  if(memOp == MemReadW) {
    if(sreq->needsData()) 
      protocol->writeMissHandler(sreq);
    else
      protocol->invalidateHandler(sreq);
    return;
  }

  if(memOp == MemWrite) {
    I(!sreq->needsData());
    protocol->invalidateHandler(sreq);
    return;
  }

  I(0); // this routine should not be called for any other memory request
}

void SMPCache::returnAccess(MemRequest *mreq)
{
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
  MemOperation memOp = sreq->getMemOperation();

  if(sreq->getRequestor() == this) {
    // request from this cache (response is ready)

    if(memOp == MemRead) {
      protocol->readMissAckHandler(sreq);
    } 
    else if(memOp == MemReadW) {
      if(sreq->needsData()) {
        protocol->writeMissAckHandler(sreq);
      } else {
	I(sreq->needsSnoop());
        protocol->invalidateAckHandler(sreq);
      }
    } 
    else if(memOp == MemWrite) {
      I(!sreq->needsData());
      I(sreq->needsSnoop());
      protocol->invalidateAckHandler(sreq);
    }
    else if(memOp == MemPush) {
      protocol->writeBackAckHandler(sreq);
    }
  } else {
    receiveFromBelow(sreq);
  }
}

void SMPCache::concludeAccess(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();

  mreq->mutateReadToWrite(); /*
                              Hack justification: It makes things much
                              nicer if we can call mutateWriteToRead()
                              in write() if the line is displaced from
                              the cache between the time the write is
                              processed in the L1 to the time it is
                              processed in the L2.

                              BUT from the L2, we don't call
                              mreq->goDown(), so we can't rely on
                              mreq->goUp() to restore the memOp.
                            */
  mreq->goUp(0);

  outsReq->retire(addr);
  mutExclBuffer->retire(addr);

}

SMPCache::Line *SMPCache::allocateLine(PAddr addr, CallbackBase *cb, 
				       bool canDestroyCB)
{
  PAddr rpl_addr = 0;
  I(cache->findLineDebug(addr) == 0);
  Line *l = cache->findLine2Replace(addr);

  if(!l) {
    // need to schedule allocate line for next cycle
    doAllocateLineCB::scheduleAbs(globalClock+1, this, addr, 0, cb);
    return 0;
  }

  rpl_addr = cache->calcAddr4Tag(l->getTag());
  lineFill.inc();

  nextSlot(); // have to do an access to check which line is free

  if(!l->isValid()) {
    if(canDestroyCB)
      cb->destroy();
    l->setTag(cache->calcTag(addr));
    return l;
  }
  
  if(isHighestLevel()) {
    if(l->isDirty()) {
      allocDirty.inc();
      doWriteBack(rpl_addr);
    } 

    if(canDestroyCB)
      cb->destroy();
    l->invalidate();
    l->setTag(cache->calcTag(addr));
    return l;
  }

  I(pendInvTable.find(rpl_addr) == pendInvTable.end());
  pendInvTable[rpl_addr].outsResps = getNumCachesInUpperLevels();
  pendInvTable[rpl_addr].cb = doAllocateLineCB::create(this, addr, rpl_addr, cb);
  pendInvTable[rpl_addr].invalidate = false;
  pendInvTable[rpl_addr].writeback = true;

  protocol->preInvalidate(l);

  invUpperLevel(rpl_addr, cache->getLineSize(), this);

  return 0;
}

void SMPCache::doAllocateLine(PAddr addr, PAddr rpl_addr, CallbackBase *cb)
{
  // this is very dangerous (don't do it at home) if rpl_addr is zero,
  // it means allocateLine was not able to allocate a line at its last
  // try probably because all lines in the set were
  // locked. allocateLine has scheduled a callback to doAllocateLine
  // in the next cycle, with rpl_addr zero. The code below will call
  // allocateLine again, which will schedule another doAllocateLine if
  // necessary. If that happens, allocateLine will return 0, which
  // means doAllocateLine shouldn't do anything else. If allocateLine
  // returns a line, then the line was successfully allocated, and all
  // that's left is to call the callback allocateLine has initially
  // received as a parameter
  if(!rpl_addr) {
    Line *l = allocateLine(addr, cb, false);

    if(l) {
      I(cb);
      l->setTag(calcTag(addr));
      l->changeStateTo(SMP_TRANS_RSV);
      cb->call();
    }

    return;
  }

  Line *l = cache->findLine(rpl_addr); 
  I(l && l->isLocked());

  if(l->isDirty()) {
    allocDirty.inc();
    doWriteBack(rpl_addr);
  }

  I(cb);
  l->setTag(cache->calcTag(addr));
  l->changeStateTo(SMP_TRANS_RSV);
  cb->call();
}

SMPCache::Line *SMPCache::getLine(PAddr addr)
{
  nextSlot(); 
  return cache->findLine(addr);
}

void SMPCache::writeLine(PAddr addr) {
  Line *l = cache->writeLine(addr);
  I(l);
}

void SMPCache::invalidateLine(PAddr addr, CallbackBase *cb, bool writeBack) 
{
  Line *l = cache->findLine(addr);
  
  I(l);

  I(pendInvTable.find(addr) == pendInvTable.end());
  pendInvTable[addr].outsResps = getNumCachesInUpperLevels();
  pendInvTable[addr].cb = cb;
  pendInvTable[addr].invalidate = true;
  pendInvTable[addr].writeback = writeBack;

  protocol->preInvalidate(l);

  if(!isHighestLevel()) {
    invUpperLevel(addr, cache->getLineSize(), this);
    return;
  }

  doInvalidate(addr, cache->getLineSize());
}

#ifdef SESC_SMP_DEBUG
void SMPCache::inclusionCheck(PAddr addr) {
  const LevelType* la = getUpperLevel();
  MemObj* c  = (*la)[0];

  const LevelType* lb = c->getUpperLevel();
  MemObj*    cc = (*lb)[0];

  I(!((Cache*)cc)->isInCache(addr));
}
#endif
