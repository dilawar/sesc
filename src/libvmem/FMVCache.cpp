/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss
		  Luis Ceze

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
#include <strings.h>

#include <algorithm>

#include "FMVCache.h"
#include "Port.h"
#include "TaskContext.h"
#include "TaskHandler.h"
#include "MVCacheState.h"
#include "TraceGen.h"

char *FMVCache::getExtMSHRName(const char *name)
{
  char *n = (char *)malloc(strlen(name)+10);

  sprintf(n,"%sExt",name);

  return n;
}

FMVCache::FMVCache(MemorySystem *gms, const char *section, const char *name) 
  : GMVCache(gms, section, name)
  ,nRestartPush("%s:nRestartPush"    , name) 

{

  extMSHR = MSHR<PAddr,FMVCache>::create(getExtMSHRName(name),
				SescConf->getCharPtr(section, "ExtMSHRtype"),
				SescConf->getInt(section, "ExtMSHRSize"),
				SescConf->getInt(section, "bsize"));

#ifdef DEBUG
  ulong nLines = cache->getNumLines();
  ulong cAssoc = cache->getAssoc();
  for(ulong i = 0; i < nLines; i++) 
    cache->getPLine(i)->setSet(i / cAssoc);
#endif //DEBUG
}

FMVCache::~FMVCache()
{
  // Nothing to do
}

void FMVCache::displaceLine(CacheLine *cl)
{
  I(!cl->isInvalid());
  I(cl->getPAddr());
  PAddr paddr = cl->getPAddr();

  wrLVIDEnergy->inc();

  if (cl->accessLine()) {
    // The cache may be killed or restarted. The cache line state must be
    // updated accordingly
    I(cl->isInvalid());
    return;
  }
  I(!cl->isInvalid());
  I(!cl->isKilled());

  if (cl->isRestarted()) {
    // Note: It is possible to send (cl->isSafe() && !cl->isDirty()), but then
    // the locality is much worse.
    cl->invalidate();
    return;
  }

  //is not dirty, and is safe then can invalidate????
  //If the safe line later become unsafe, and unfortunately,
  //the line is readed by this thread and written by pre-thread
  //then this memory violation may be ignored!!!!! add by hr
  if (!cl->isDirty() && cl->isSafe()) {
    cl->invalidate();
    return;
  }

#ifdef DEBUG
  // If the line displaced is safe, it must displace the safest from the set
  if (cl->isSafe()) {
    ulong index = calcIndex4PAddr(cl->getPAddr());
    for(ulong i=0; i < cache->getAssoc(); i++) {
      CacheLine *ncl = cache->getPLine(index+i);
      if (ncl->isInvalid())
	continue;
      if (!ncl->isHit(cl->getPAddr()))
	continue;
    //  GI(ncl->getVersionRef(), *(ncl->getVersionRef()) >= *(cl->getVersionRef()));
    }
  }
#endif

  // nSharers == 1 means no more sharers for this address one 
  int32_t nSharers = countLinesInSet(cl->getPAddr());
  
  sendPushLine(cl->getVersionDuplicate(), cl->getPAddr(), cl
	       ,nSharers == 1
	       ,0 // No askReq because it was a local decision, not an
		  // answer to a AskPushLine
	       );

  cl->invalidate();

  return;
}

// Returns true if a safe line got displaced
void FMVCache::findSafeLines(PAddr paddr)
{
  // For cache lines in the set:
  //
  // 1-Invalidate killed lines
  //
  // 2-Displace line if safe and finished (commited)

  rdLVIDEnergy->add(cache->getAssoc());

  I(safeLines.empty());

  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);

    if (cl->isInvalid())
      continue;

    if (cl->accessLine()) {
      I(cl->isInvalid());
      wrLVIDEnergy->inc();
      continue;
    }

    if (!cl->isHit(paddr))
      continue;

    I(!cl->isInvalid());

    if (!(cl->isSafe() && cl->isFinished()))
      continue;

    if (cl->isDirty()) {
      I(!cl->isInvalid());
      I(cl->getVersionRef());
      safeLines.push_back(cl);
    }else{
      // Clean line and safe. Destroy at will
      wrLVIDEnergy->inc();
      cl->invalidate();
    }
  }
}

bool FMVCache::displaceSafeLines(PAddr paddr, VMemPushLineReq *askReq)
{
  GI(askReq, paddr == askReq->getPAddr());
  if (safeLines.empty()) 
    return false;

  bool safeDisp = false;
  int32_t  nSharers = countLinesInSet(paddr);

  I(nSharers>=(int)safeLines.size());

  std::sort(safeLines.begin(), safeLines.end(), MVCacheStateMore());

  while(!safeLines.empty()) {
    I(nSharers>0);

    nSharers--;
    wrLVIDEnergy->inc();

    CacheLine *cl = safeLines.back();
    safeLines.pop_back();

    I(cl->isHit(paddr));

    I(!cl->isInvalid());
    
    // If there are two consecutive versions. Not necessary to
    // displace both, only the most spec (as long as both are dirty)
    if (!safeLines.empty()) {
      if (cl->getVersionRef()->getNextRef() == safeLines.back()->getVersionRef()) {
	I(cl->isDirty());
	I(safeLines.back()->isDirty());
	cl->invalidate();
	continue;
      }
    }

    safeDisp = true;

    sendPushLine(cl->getVersionDuplicate(), cl->getPAddr(), cl
		 ,nSharers  == 0, askReq);
    
    cl->invalidate();
  }

  I(safeLines.empty());
  return safeDisp;
}

void FMVCache::cleanupSet(PAddr paddr)
{
  findSafeLines(paddr);
  displaceSafeLines(paddr);
}

FMVCache::CacheLine *FMVCache::allocateLine(LVID *lvid, LPAddr addr)
{
  // Displacement order:
  //
  // 1-Try find invalid line
  //
  // 2-Otherwise find a task that has no wrmask & xrdmask (prefetched line)
  //
  // 3-Otherwise invalidate the most spec line that got restarted
  // 
  // 4-Otherwise displace the most speculative line (more spec that current task)
  //
  // 5-Otherwise displace using LRU

  CacheLine *cl = cache->findLine(addr);
  if (cl) {
    if (cl->accessLine())
      wrLVIDEnergy->inc();
    
    return cl; // Already allocated :)
  }

  cl = cache->findLine2Replace(addr);
  if (cl) {
    if( cl->accessLine() ) {
      wrLVIDEnergy->inc();
      return cl;
    }else if (cl->isInvalid())
      return cl;
    else if (!cl->hasState()) {
      wrLVIDEnergy->inc();
      cl->invalidate();
      return cl;
    }
  }

  PAddr paddr = lvid->calcPAddr(addr);
  LPAddr index = calcIndex4PAddr(paddr);
  if (cl == 0) {
    // Look for a restarted line from another task
    for(ulong i=0; i < cache->getAssoc(); i++) {
      CacheLine *ncl = cache->getPLine(index+i);
      I(!ncl->isInvalid());
      if (ncl->isRestarted()) {
	if (cl==0)
	  cl = ncl;
	else if ( *(ncl->getVersionRef()) > *(cl->getVersionRef()) )
	  cl = ncl;
      }
      if (!ncl->hasState()) {
	cl = ncl;
	break;
      }
    }
  }

  if (!cl) {
    // Look for a more spec task
    cl = getLineMoreSpecThan(lvid->getVersionRef(), paddr);
  }else if (cl->isRestarted() || !cl->hasState()) {
    wrLVIDEnergy->inc();
    cl->invalidate();
    return cl;
  }

  // Otherwise get a random line (even locked lines are valid)
  if (cl==0) 
    cl = cache->findLine2Replace(addr, true);
  I(cl);
  if( cl->accessLine() ) {
    wrLVIDEnergy->inc();
    return cl;
  }else if (!cl->hasState()) {
    cl->invalidate();
    return cl;
  }
  
#ifdef DEBUG
  if (cl->isSafe()) {
    I(!cl->isInvalid());
    // There must be not more safe lines for the same address
    bool moreSafe = false;
    PAddr paddr2 = cl->getPAddr();
    I(index == calcIndex4PAddr(paddr2));
    for(ulong i=0; i < cache->getAssoc(); i++) {
      CacheLine *ncl = cache->getPLine(index+i);
      if (ncl->isInvalid() || ncl == cl)
	continue;

      if (!ncl->isHit(paddr2))
	continue;

      moreSafe = moreSafe || ncl->isSafe();
      GLOG(!ncl->isSafe(), "failed. Another safe line at addr [0x%llx] vs [0x%llx]" , ncl->getLPAddr(), cl->getLPAddr());
    }
  }
#endif

  I(!cl->isInvalid());
  displaceLine(cl);//notice this entry. this is the pushline due to cache miss. add by hr
  I(cl->isInvalid());

  return cl;
}

void FMVCache::createNewLine(const VRWReq *vreq, const VMemState *state)
{
  MemRequest *mreq = vreq->getMemRequest();
  LVID       *lvid = static_cast<LVID *>(mreq->getLVID());
  I(!lvid->canRecycleLVID()); // LVID can't get recycled while there is a memory request.

  if (lvid->isKilled())
    return;

  I(vreq->getPAddr());
  // Cache line should not previously exist
  LPAddr      addr = lvid->calcLPAddr(vreq->getPAddr());
  CacheLine  *cl   = allocateLine(lvid, addr);
  if(cl==0) {
    I(lvid->isKilled());
    return;
  }

  if(cl->isInvalid()) {
    cl->resetState(addr, lvid);
  } else {
    I(cl->isHit(vreq->getPAddr()) && cl->getLVID() == lvid);//this condition is important. add by hr 
  }
  cl->combineStateFrom(state);
  cl->updateWord(mreq->getMemOperation(), vreq->getPAddr());

  cl->setMsgSerialNumber(vreq->getSerialNumber());
}

void FMVCache::initWriteCheck(LVID *lvid, MemRequest *mreq, bool writeHit)
{ 
  VMemWriteReq *vreq = VMemWriteReq::createWriteCheck(this
						      ,lvid->getVersionDuplicate()
						      ,mreq->getPAddr()
						      ,mreq
						      ,writeHit);
  
  if (writeHit) {// why sent data when cachehit????
    vreq->setCacheSentData();
  }

  // add the request
  vreq->setLatency(cachePort->nextSlotDelta()+missDelay);
  static_cast<VMemObj *>(lowerLevel[0])->writeCheck(vreq);
}

bool FMVCache::performAccess(MemRequest *mreq)
{
  // Perform either a read or a write access

  LVID    *lvid   = static_cast<LVID *>(mreq->getLVID());
  I(lvid);
  if(lvid->isKilled()) 
    return true;

  //these two addrs have any difference?
  const PAddr    paddr  = mreq->getPAddr();
  const LPAddr    addr  = lvid->calcLPAddr(paddr);
    
  
  // retrieve data
  CacheLine *cl = cache->findLine(addr);
  if (cl == 0) {
    CacheLine *pred = locallySatisfyLine(lvid->getVersionRef(), paddr);
    if (pred == 0) {
      cleanupSet(paddr);// what this func do? I guess it just find a safe line to repalce. add by hr
      handleMiss(mreq);
      return false;
    }
    
    I(!pred->isInvalid());
    // local miss means no same version
    I(pred->getVersionRef() != lvid->getVersionRef());

    if (pred && pred->isSafe() && pred->isFinished()) {
      // Just promote the cache line to the new version
      // Keep the specBits
      pred->invalidate();
      pred->resetState(addr, lvid, pred);
      pred->clearMasks();
      cl = pred;
      wrLVIDEnergy->inc(); // New LVID
      wrLVIDEnergy->inc(); // invalidation of pred
    }else{
      cl = allocateLine(lvid, addr);
      if(cl==0) {
	I(lvid->isKilled());
	return true;
      }
      I(cl->isInvalid() || cl->getLVID() == lvid);
      
      if (!cl->isInvalid() && cl->getLVID() != lvid)
        cl->invalidate();

      cl->resetState(addr, lvid);

      if(pred) {
	if (pred->isInvalid()) {
	  // pred got killed, this means that it was more
	  // speculative. The mostSpecLine bit can be set
	  cl->setMostSpecLine();
	}else{
         if (*(pred->getVersionRef()) < *(lvid->getVersionRef()))
           pred->forwardStateTo(cl);
	}
      }
    }
    
    I(!cl->isInvalid());
  } else {
    rdHitEnergy->inc();
  }

  // Cache line hit
  if (cl->accessLine()) {
    I(cl->isInvalid());
    wrLVIDEnergy->inc();
    cl->resetState(addr, lvid);
  }
  
  I(!cl->isInvalid());
  I(!cl->isRestarted()); // accessLine calls promote if needed
  
  cl->updateWord(mreq->getMemOperation(), paddr);

  if(mreq->getMemOperation() == MemWrite) {
    if (cl->isMostSpecLine())
      return true;

    initWriteCheck(lvid, mreq, true);
    return false;
  }
 
  // return for hit 
  return true; 
}

void FMVCache::sendPushLine(HVersion *verDup, PAddr paddr, const VMemState *state
			    ,bool noMoreSharers, VMemPushLineReq *askReq)
{
  I(verDup);

  VMemPushLineReq *vreq = VMemPushLineReq::createPushLine(this
							  ,verDup
							  ,paddr
							  ,state
							  ,noMoreSharers
							  ,askReq);
  if (askReq) {
    I(askReq->getType() == VAskPushLine);
    askReq->incPendingMsg();
  }

  if (!extMSHR->issue(paddr)) {
    //put it first in the extMSHR. add by hr
    extMSHR->addEntry(paddr, doSendPushLineCB::create(this, vreq));
  }else{
    doSendPushLine(vreq);
  }
}

void FMVCache::doSendPushLine(VMemPushLineReq *vreq)
{
  static_cast<VMemObj *>(lowerLevel[0])->pushLine(vreq);
}

void FMVCache::handleMiss(MemRequest *mreq)
{
  PAddr paddr = mreq->getPAddr();

  LVID *lvid = static_cast<LVID *>(mreq->getLVID());
  I(!lvid->isKilled());

  missInc(mreq->getMemOperation());

  if(mreq->getMemOperation() == MemRead) {
    // READ MISS

    VMemReadReq *vreq = VMemReadReq::createRead(this, lvid->getVersionDuplicate(), mreq);
    vreq->incPendingMsg();

    vreq->setLatency(cachePort->nextSlotDelta()+missDelay);
    static_cast<VMemObj *>(lowerLevel[0])->read(vreq);
    return;
  }
  I(mreq->getMemOperation() == MemWrite);

  // WRITE MISS
  
  // A write miss can be operated like a read miss, but the protocol
  // can be optimized if the write sends at the same time a writeCheck
  // (very likely it would have sent a writeCheck after the miss, so
  // why to wait?)
  
  initWriteCheck(lvid, mreq, false);
}

void FMVCache::doAccess(MemRequest *mreq)  
{  
  bool done = performAccess(mreq);
  if (!done) { // it was a miss
    return;
  }

  mshr->retire(mreq->getPAddr());

  hitInc(mreq->getMemOperation());

  I(mreq->isTopLevel());
  mreq->goUp(hitDelay);
}

void FMVCache::doAccessQueued(MemRequest *mreq)  
{
  bool done = performAccess(mreq);
  if(!done) { // it was a miss
    // Bad luck. The request was waiting to be serviced, and it still
    // misses the cache, so it has to wait. ahhhhh!!
    return;
  }

  mshr->retire(mreq->getPAddr());

  halfMissInc(mreq->getMemOperation());

  I(mreq->isTopLevel());
  mreq->goUp(0);
}

void FMVCache::returnAccess(MemRequest *mreq)
{
  I(0);
}

void FMVCache::localRead(MemRequest *mreq)
{
  I(mreq->getMemOperation() == MemRead);

  PAddr paddr = mreq->getPAddr();
  I(paddr);

  if (!mshr->issue(paddr)) {
    mshr->addEntry(paddr, doAccessQueuedCB::create(this, mreq));
    return;
  }

  doAccessCB::scheduleAbs(cachePort->nextSlot(), this, mreq);
}

void FMVCache::localWrite(MemRequest *mreq)
{
  I(mreq->getMemOperation() == MemWrite);

/*
  mreq->goUp(0); //why??????????!!!!!!!!!!!!!!!!!
  return;
*/  
//CHECK:
  PAddr paddr = mreq->getPAddr();
  I(paddr);
  
  if (!mshr->issue(paddr)) {
    mshr->addEntry(paddr, doAccessQueuedCB::create(this, mreq));
    return;
  }

  doAccessCB::scheduleAbs(cachePort->nextSlot(), this, mreq);
}

void FMVCache::read(VMemReadReq *readReq)
{
  PAddr paddr = readReq->getPAddr();

  if (!extMSHR->issue(paddr)) {
    extMSHR->addEntry(paddr, doReadCB::create(this, readReq, true));
    return;
  }
  
  doRead(readReq, false);
}

void FMVCache::doRead(VMemReadReq *readReq, bool wait4PushLine)
{
  TimeDelta_t  latency = cachePort->nextSlotDelta();
  VMemReadReq *readAckReq= GMVCache::read(readReq, latency);

  rdRevLVIDEnergy->inc();

  if (readAckReq == 0) {
    // if it got here, there is no data to send, so send an empty packet
    nReadMiss.inc();
    readAckReq = VMemReadReq::createReadAck(this, readReq, 0);
    readReq->incPendingMsg();
    readAckReq->setLatency(latency+missDelay);
  }

  readReq->decPendingMsg();

  I(readReq->hasPendingMsg()); // At least the outstanding msg

  if (wait4PushLine) {
    readAckReq->setWait4PushLine();
  }

  extMSHR->retire(readAckReq->getPAddr());

  static_cast<VMemObj *>(lowerLevel[0])->readAck(readAckReq);
}

void FMVCache::readAck(VMemReadReq *readAckReq)
{
  //  LOG("%lld %s 0x%lx readAck", globalClock, getSymbolicName(), readAckReq->getPAddr());

  VMemReadReq *readReq = readAckReq->getOrigRequest();

  I(readReq);
  I(readReq->getType() == VRead);
  I(readAckReq->getType() == VReadAck);

  readReq->decPendingMsg();

  if (readReq->hasMemRequestPending()) { // what is the meaning of this condition???? add by hr
    bool doIt = false;
    MemRequest *mreq = readReq->getMemRequest();

    LVID    *lvid   = static_cast<LVID *>(mreq->getLVID());
    if (!lvid->isKilled() && readAckReq->getVersionRef()) {
      // Only if sending some data
      I(readReq->hasCacheSentData() // from another cache
	|| readReq->hasMemSentData());      // from memory
      // If same Version, just use it :)
      doIt = doIt || *(readReq->getVersionRef()) == *(readAckReq->getVersionRef());
      // mostSpecBit set, we are lucky :)
      doIt = doIt || (readAckReq->getStateRef()->isMostSpecLine() 
		      && *(readReq->getVersionRef()) > *(readAckReq->getVersionRef()));
    }
    // It is the last request
    doIt = doIt || !readReq->hasPendingMsg();// why use || ??? add by hr
    // I think if use ||, then if two or more FMVCache have the different version cacheline
    // then , what should this func do? createNewLine more times??? 

    if (doIt) {
      createNewLine(readReq, readAckReq->getStateRef());
      if(!readReq->hasPendingMsg()) { 
	mshr->retire(readReq->getPAddr());
	readReq->markMemRequestAck();
        mreq->goUp(0);
      }
    }
  }

  // destroy original request, if it is no longer needed
  if (!readReq->hasPendingMsg())
    readReq->destroy();
  
  readAckReq->destroy();
}


void FMVCache::writeCheck(VMemWriteReq *vreq)
{
  // Write check can NOT use a MSHR, or a deadlock may happen

  VMemObj *mobj = static_cast<VMemObj *>(lowerLevel[0]);
  I(vreq->getType() == VWriteCheck);

  VMemWriteReq *nreq = GMVCache::writeCheck(vreq, mobj);
  I(nreq);
  I(nreq->getOrigRequest() == vreq);

  vreq->decnRequests(); // The just received packet

  nreq->setLatency(cachePort->nextSlotDelta()+hitDelay);
  mobj->writeCheckAck(nreq);
}

void FMVCache::writeCheckAck(VMemWriteReq *vreq)
{
  I(vreq);

  // Yes, this is correct. It is not a VWriteCheck ack so that a new
  // VMemReq is not built.
  I(vreq->getType() == VWriteCheck);
  I(vreq->getnRequests() == 0);
  if(!vreq->isWriteHit())
    createNewLine(vreq, vreq->getStateRef());
  
  MemRequest *mreq = vreq->getMemRequest();

  mshr->retire(vreq->getPAddr());
  mreq->goUp(0);

  // destory the request
  vreq->destroy();
}

void FMVCache::pushLine(VMemPushLineReq *vreq)
{
  I(0); // No pushLine is accepted by L1 cache
}

void FMVCache::pushLineAck(VMemPushLineReq *pushAckReq)
{
  I(pushAckReq->getType() == VPushLineAck);
  I(pushAckReq->getVMem() == this); // Original PushLine originated here
    
  PAddr paddr = pushAckReq->getPAddr();

  extMSHR->retire(paddr);

  if (pushAckReq->getVersionRef()) {
   // It the forward entry, inited from LMVCache, when push req from FMVCache arrive the
   // LMVCache, and it find there are no room(cache no room, or lvid are used out)  to find 
   // it, so, it will forward the req back again. add by hr
    if (pushAckReq->getVersionRef()->isKilled()) {
      // KILLED: just do nothing
    }else if (pushAckReq->getVersionRef()->isSafe()) {
      // SAFE: It got safe while traveling
      int32_t nSharers = countLinesInSet(paddr);

      sendPushLine(pushAckReq->getVersionDuplicate(), paddr, pushAckReq->getStateRef()
		   ,nSharers == 0, 0);
    }else{
      // SPECULATIVE: try to allocate or kill it

      // TODO?: do the try to allocate
      I(pushAckReq->getVersionRef()->getTaskContext());
#ifdef TS_TIMELINE
      TraceGen::add(pushAckReq->getVersionRef()->getId(),"Push=%lld",globalClock);
#endif
      taskHandler->restart(pushAckReq->getVersionRef());
      nRestartPush.inc();
    }
  }

  pushAckReq->destroy();
}

void FMVCache::askPushLine(VMemPushLineReq *askReq)
{
  // askPushLine can NOT use a MSHR, or a deadlock may happen

  rdRevLVIDEnergy->inc();

  PAddr paddr = askReq->getPAddr();
  findSafeLines(paddr);
  bool safeDisp = displaceSafeLines(paddr, askReq);

  if (safeDisp) {
    // Not at the beginning of the fuction because PendingMsg can
    // reach 0.
    askReq->decPendingMsg(); 
    return;
  }

  int32_t nSharers = countLinesInSet(paddr);

  askReq->decPendingMsg(); // For the msg just processed

  sendPushLine(askReq->getVersionDuplicate(), paddr, askReq->getStateRef()
	       ,nSharers == 0, askReq);
}
