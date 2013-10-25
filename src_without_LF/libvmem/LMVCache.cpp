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

#include "LMVCache.h"
#include "Port.h"
#include "MemRequest.h"
#include "TaskHandler.h"
#include "EnergyMgr.h"
#include "VMemReq.h"
#include "VBus.h"
#include "TraceGen.h"

//#define TS_VICTIM_DISPLACE_CLEAN 1

LMVCache::LMVCache(MemorySystem *gms, const char *section, const char *name, VBus *vb) 
  : GMVCache(gms, section, name)
  // Statistics
    ,combineHit("%s:combineHit"     , name) 
    ,combineMiss("%s:combineMiss"    , name) 
    ,combineHalfMiss("%s:combineHalfMiss", name)
    ,nRestartDisp("%s:nRestartDisp"    , name) 
    ,nRestartAlloc("%s:nRestartAlloc"   , name) 
    ,vbus(vb)
{
  combWriteEnergy = new GStatsEnergy("combWriteEnergy"
				     ,name
				     ,0
				     ,MemPower
				     ,EnergyMgr::get("miscEnergy","combWriteEnergy")
				     );

}

LMVCache::~LMVCache()
{
}

void LMVCache::displaceLine(CacheLine *cl)
{
  if (cl->isInvalid())
    return;

  wrLVIDEnergy->inc();

  if (cl->accessLine()) {
    // The cache may be killed or restarted. The cache line state must be
    // updated accordingly
    I(cl->isInvalid());
    return;
  }

  if (!cl->isSafe()) {
#ifdef TS_TIMELINE
    TraceGen::add(cl->getVersionRef()->getId(),"Disp=%lld",globalClock);
#endif
    taskHandler->restart(cl->getVersionRef());
    nRestartDisp.inc();
    cl->invalidate();
    return;
  }

  if (cl->isRestarted() || !cl->hasState()) {
    // Note: It is possible to send (cl->isSafe() && !cl->isDirty()), but then
    // the locality is much worse.
    cl->invalidate();
    return;
  }

#ifndef TS_VICTIM_DISPLACE_CLEAN
  if (!cl->isDirty()) {
    I(cl->isSafe());
    cl->invalidate();
    return;
  }
#endif

  PAddr paddr = cl->getPAddr();

  I(!isCombining(paddr));

#ifdef TS_VICTIM_DISPLACE_CLEAN
  writeMemory(paddr);
#else
  if (cl->isDirty())
    writeMemory(paddr);
#endif
  cl->invalidate();
  I(cl->isInvalid());

  return;
}

void LMVCache::cleanupSet(PAddr paddr)
{
  // For cache lines in the set:
  //
  // 1-Invalidate killed lines

  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;

    if (cl->accessLine()) {
      wrLVIDEnergy->inc();
      continue;
    }
    // accesssLine should change from killed to invalid
    I(!cl->isKilled());
  }
}

LMVCache::CacheLine *LMVCache::allocateLine(LVID *lvid, LPAddr addr)
{
  // Displacement order:
  //
  // 1-Initiate a combine of safe lines
  //
  // 2-Writeback & invalidate a combine line
  //
  // 3-Restart the most speculative line (if no forwarding)

  CacheLine *cl = cache->findLine(addr);
  if (cl)
    return cl; // Already allocated :)

  PAddr paddr = calcPAddr(lvid, addr);
  cl = cache->findLine2Replace(addr);
  if (cl) {
    if (cl->isInvalid())
      return cl;
    wrLVIDEnergy->inc();
    if( cl->accessLine() ) {
      return cl;
    }else if (!cl->hasState()) {
      cl->invalidate();
      return cl;
    }
    I(cl->isLeastSpecLine());
#ifdef TS_VICTIM_DISPLACE_CLEAN
    writeMemory(paddr);
#endif
    cl->invalidate();
    return cl;
  }

  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl2 = cache->getPLine(index+i);
    I(!cl2->isInvalid()); // no findLine2Replace(addr)
    if (cl2->isSafe()) {
      if (cl==0)
	cl = cl2;
      else if (*(cl2->getVersionRef()) < *(cl->getVersionRef()))
	cl = cl2;
    }
  }

  if(cl) {
    I(cl->isSafe());

#ifdef TS_VICTIM_DISPLACE_CLEAN
    I(!isCombining(cl->getPAddr()));
    combineInit(cl);
#else
    if (cl->isDirty()) {
      I(!isCombining(cl->getPAddr()));//delete by hr
      combineInit(cl);
    }else{
      cl->invalidate();
      wrLVIDEnergy->inc();
    }
#endif
    I(cl->isInvalid());
    return cl;
  }

  cl = getLineMoreSpecThan(lvid->getVersionRef(), paddr);
  if (cl==0)
    cl = cache->findLine2Replace(addr, true);

  I(cl);

  I(!cl->isInvalid());
  I(!cl->isKilled());

  if (*(cl->getVersionRef()) <= *(lvid->getVersionRef())) {
    // OPT2: No allocate, too speculative request
    return 0;
  }
  
  // OPT3: Task more Spec (restartit
#ifdef TS_TIMELINE
  TraceGen::add(cl->getVersionRef()->getId(),"Alloc=%lld",globalClock);
#endif
  taskHandler->restart(cl->getVersionRef());
  nRestartAlloc.inc();
  I(!cl->isLocked());
  if (cl->accessLine()) {
    wrLVIDEnergy->inc();
  }else if (!cl->isInvalid()) {
    I(!cl->isDirty() && !cl->hasState());
    wrLVIDEnergy->inc();
    cl->invalidate();
  }
  I(cl->isInvalid());

  return cl;
}

bool LMVCache::isCombining(PAddr paddr) const
{
  CombineMap::const_iterator it = cMap.find(calcLine(paddr));
  
  return it != cMap.end();
}

void LMVCache::combineInit(PAddr paddr, HVersion *verDup, const VMemState *state)
{
  I(verDup);
  VMemPushLineReq *askReq = VMemPushLineReq::createAskPushLine(this
							       ,verDup
							       ,paddr
							       ,state);
  I(askReq->getVersion());
  
  askReq->incPendingMsg();

  I(askReq->getType() == VAskPushLine);

  cMap[calcLine(paddr)] = askReq;

  // Combine All
  // combine all the safe???? add by hr
  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;

    if (cl->accessLine()) {
      wrLVIDEnergy->inc();
      continue;
    }
    if (!cl->isHit(paddr))
      continue;

#if TS_WRITE
    if(!cl->isSafe())
      continue;
#endif

    askReq->getState()->combineStateFrom(cl);
    cl->invalidate();
    wrLVIDEnergy->inc();
  }

  // I guess that , here, the LMVCache actively request the safe cacheline form the upper 
  // level FMVCache. add by hr
  I(askReq->getVersion());

  vbus->askPushLine(askReq);
}

void LMVCache::combineInit(CacheLine *cl)
{
  I(cl);
  I(cl->isSafe());
#ifndef TS_VICTIM_DISPLACE_CLEAN
  I(cl->isDirty());
#endif

  PAddr paddr = cl->getPAddr();

  wrLVIDEnergy->inc();
  combWriteEnergy->inc();

  I(!isCombining(paddr));
  
  if (cl->isLeastSpecLine()) {
#ifndef TS_VICTIM_DISPLACE_CLEAN
    I(cl->isDirty());
#endif
    writeMemory(paddr);
    combineHalfMiss.inc();
    cl->invalidate();
    return;
  }

  combineInit(paddr, cl->getVersionDuplicate(), cl);

  I(cl->isInvalid());
}

void LMVCache::combineInit(const VMemPushLineReq *vreq)
{
  I(vreq);
  I(vreq->getVersionRef()->isSafe());

  PAddr paddr = vreq->getPAddr();
  I(!isCombining(paddr));

  combWriteEnergy->inc();

  if (vreq->getStateRef()->isLeastSpecLine()) {
    // write to a non-version cache
    combineHit.inc();
#ifdef TS_VICTIM_DISPLACE_CLEAN
    writeMemory(paddr);
#else
    if (vreq->getStateRef()->isDirty())
      writeMemory(paddr);
#endif
    return;
  }

  combineInit(paddr, vreq->getVersionDuplicate(), vreq->getStateRef());
}

void LMVCache::combinePushLine(const VMemPushLineReq *pushReq)
{
  // Steps:
  //
  // 1-Add to VCR if dirty and safe
  //
  // 2-Check that non of the local cache lines become safe meanwhile
  //
  // 3-Decrease # pending request if required
  //
  // 4-If last request arrived. Place it on the cache (if possible)

  I(pushReq);

  combWriteEnergy->inc();

  PAddr paddr = pushReq->getPAddr();
  I(paddr);
  I(pushReq->getType() == VPushLine);

  CombineMap::iterator it = cMap.find(calcLine(paddr));
  
  I(it != cMap.end());

  VMemPushLineReq *askPushReq = it->second;
  I(askPushReq->getType() == VAskPushLine);

  if (pushReq->getStateRef()->isDirty() && pushReq->getVersionRef()->isSafe()) {
    // This is a simple implementation of the VCR (Version Combine
    // Register). Real hardware would require a VCR. Combines can not
    // be done using the wrmask because store bytes can not write the
    // whole word. As a result, wrmask is a superset of the writing
    // bytes, not the correct set. I do not model that here because it
    // would not affect performance, just complexity.

    // FIXME: add VCR energy
    if (*(askPushReq->getVersionRef()) < *(pushReq->getVersionRef()))
      askPushReq->changeVersion(pushReq->getVersionDuplicate());
    
    askPushReq->getState()->combineStateFrom(pushReq->getStateRef());
    // why the conbine is done to askPushReq, not the cacheline??? add by hr
  }

  // Maybe we left an address on the case (not safe), and now it has
  // become safe. We should add it to the VCR
  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;

    if (cl->accessLine()) {
      wrLVIDEnergy->inc();
      continue;
    }
    if (!cl->isHit(paddr))
      continue;

#if TS_WRITE
    if(!cl->isSafe())
      continue;
#endif

    askPushReq->getState()->combineStateFrom(cl);
    cl->invalidate();
    wrLVIDEnergy->inc();
  }
  
  // when the pushReq is caused by the displace() in allocateLine() in FMVCache,
  // and it will return here. add by hr
  if (!pushReq->getAskPushReq())
    return;
  
  I(pushReq->getAskPushReq() == askPushReq);

  askPushReq->decPendingMsg();
  if (askPushReq->hasPendingMsg())
    return;

  // combineEnd
  cMap.erase(it);

#if TS_WRITE
  writeMemory(paddr);
#endif

  askPushReq->destroy();
}

void LMVCache::writeMemory(PAddr paddr)
{
//#ifndef TS_VICTIM_DISPLACE_CLEAN
//  I(0); // FIXME when writes activated
//#endif

  CBMemRequest::create(0 // Latency
		       ,vbus->getLowerLevel() // memory or bus
		       ,MemWrite // Displace Line
		       ,paddr // PAddr
		       ,0 // Callback, no ack required
		       );
}

void LMVCache::forwardPushLine(VMemPushLineReq *pushReq)
{
  I(vbus);
  I(!pushReq->getVersionRef()->isSafe());
  I(pushReq->getAskPushReq()==0);

  pushReq->convert2Ack();

  vbus->pushLineAck(pushReq);
}

void LMVCache::ackPushLine(VMemPushLineReq *pushReq)
{
  pushHit.inc();

  pushReq->changeVersion(0); //why set 0?? 
  pushReq->convert2Ack();
  vbus->pushLineAck(pushReq);
}

void LMVCache::localRead(MemRequest *vreq)
{
  I(0); // It should not be called, only MVCache::localRead
}

void LMVCache::returnAccess(MemRequest *mreq)
{
  I(0);
}

void LMVCache::read(VMemReadReq *readReq)
{
  TimeDelta_t  latency   = cachePort->nextSlotDelta();
  VMemReadReq *remReadAck= GMVCache::read(readReq, latency);

  if (remReadAck) {
    // Do nothing, GMVCache::read did it
  }else if (isCombining(readReq->getPAddr())) {

    hitInc(MemRead);

    readReq->setCacheSentData();

    nReadHalfHit.inc();

    remReadAck = VMemReadReq::createReadAck(this
					    ,readReq
					    ,readReq->getVersionDuplicate());
    readReq->incPendingMsg();

    remReadAck->getState()->setLeastSpecLine();
  }else{
    // if it got here, there is no data to send, and other cache has sent data
    nReadMiss.inc();
    remReadAck = VMemReadReq::createReadAck(this, readReq, 0);
    readReq->incPendingMsg();
  }

  remReadAck->setLatency(latency+missDelay);
  
  I(vbus);

  readReq->decPendingMsg();

  vbus->readAck(remReadAck);
}

void LMVCache::readAck(VMemReadReq *readAckReq)
{
  I(0); // Invalid operation. L2 can not initiate read
}

void LMVCache::localWrite(MemRequest *vreq)
{  
  I(0); // It should not be called, only MVCache::write
}
 
void LMVCache::writeCheckAck(VMemWriteReq *writeReq){
  I(0);
}

void LMVCache::writeCheck(VMemWriteReq *vreq)
{ 
  VMemWriteReq *nreq = GMVCache::writeCheck(vreq, vbus);
  I(nreq);

  I(nreq->getOrigRequest() == vreq);

  if (!vreq->hasCacheSentData() ) {
    // If the cache did not provide data, set the cacheSentData bit so
    // that no readMiss is performed
      
    if (isCombining(vreq->getPAddr()))
      vreq->setCacheSentData();
  }

  I(vreq->getType() == VWriteCheck);
  I(nreq->getType() == VWriteCheckAck);

  vreq->decnRequests(); // The just received packet

  nreq->setLatency(cachePort->nextSlotDelta()+hitDelay);
  vbus->writeCheckAck(nreq);
}

void LMVCache::pushLine(VMemPushLineReq *pushReq)
{
  PAddr paddr = pushReq->getPAddr();
  I(paddr);

  rdRevLVIDEnergy->inc();

  // pushLine steps (LMVCache)
  //
  // 1-Check if combining that address was initiated
  //
  // 2-cleanupSet (may free LVIDs and/or lines)
  //
  // 3-Otherwise forwardLine

  if (isCombining(paddr) && (pushReq->getVersionRef()->isSafe())) {
    // Already combining, so add this line too
    // when the first push cauesd by displace() in allocateLine() in FMVCache,
    // if it hit the combine, it will enter this func.
    // Also, if it is not hit, then if is safe, it will enter the combineInit();
    // this fuc will init a askPushLine(), and combine all the possible cacheline
    // from the upper level FMVCache.
    // add by hr
    combinePushLine(pushReq);
#ifdef DEBUG
    if (isCombining(paddr) ){
      ulong index = calcIndex4PAddr(paddr);
      for(ulong i=0; i < cache->getAssoc(); i++) {
	CacheLine *cl = cache->getPLine(index+i);
	if (cl->isInvalid())
	  continue;

	// All the cache lines for this addre must be gone
	I(!(cl->isHit(paddr) && cl->isSafe()));
      }
    }
#endif
    //if the pushReq is in combing, and it is safe, so just return the req to FMVCache,
    //the FMVCache do nothing but destroy thr req. add by hr
    if (pushReq->getVersionRef()->isSafe()) {
      ackPushLine(pushReq);
      return;
    }
  }else{
    // We can not loose messages
    I(pushReq->getAskPushReq() == 0);
  }

  if (pushReq->getVersionRef()->isKilled()) {
    ackPushLine(pushReq);
    return;
  }

  // when the pushReq is not safe and it is NOT getAskPushReq(), 
  // then put it in the victim. add by hr
  //
  // first, try to transform the version in the pushReq to lvid. add by hr
  LVID *lvid = findCreateLVID(pushReq);
  
  if (lvid==0) {
    wrRevLVIDEnergy->inc();
    if (pushReq->getVersionRef()->isSafe()) {
      I(!isCombining(pushReq->getPAddr()));
      combineInit(pushReq); // This func deal with the safe and not hit combine pushReq. It will 
      			    // combine all the possible safe cacheline. and just put the safe line 
			    // in the combine, that is cMap. and init the askReq, which will later 
			    // hit the combine. add by hr
      ackPushLine(pushReq);
    }else{
      recycleAllCache(pushReq->getVersionRef());// bao li!!!! add by hr
      forwardPushLine(pushReq);
      pushMiss.inc();
    }
    return;
  }
  I(!lvid->isKilled());

  LPAddr addr = lvid->calcLPAddr(paddr);

  CacheLine *cl = cache->findLine2Replace(addr);
  if (cl == 0) {
    cleanupSet(paddr);
    I(!lvid->isGarbageCollected());
    cl = allocateLine(lvid, addr);
    GI(cl, cl->isInvalid() || (cl->getLVID() == lvid && cl->isHit(paddr)) );

    if (cl == 0) {
      if (pushReq->getVersionRef()->isSafe()) {
	// initiate combine
	I(!isCombining(paddr));
	combineInit(pushReq);
	ackPushLine(pushReq);
      }else{
	I(!pushReq->getVersionRef()->isSafe());
	forwardPushLine(pushReq);
	pushHalfMiss.inc();
      }
      lvid->garbageCollect();
      return;
    }
  }else{
    cl->accessLine();
    if (!cl->isInvalid()) {
      if(cl->isHit(addr)) {
	// The line was already here??? Race detected
	ackPushLine(pushReq);
	return;
      }
      I(!cl->isLocked());

      if(!cl->hasState())
	cl->invalidate();
      else if(cl->isSafe()) {
	I(cl->isLeastSpecLine()); // It can be displaced at will (no combine)
#ifdef TS_VICTIM_DISPLACE_CLEAN
	writeMemory(paddr);
#else
	if (cl->isDirty())
	  writeMemory(paddr);
#endif
	cl->invalidate();
      }else{
	I(0);
      }
    }
    I(cl->isInvalid());
  }

#ifdef DEBUG
  if (isCombining(paddr) ){
    ulong index = calcIndex4PAddr(paddr);
    for(ulong i=0; i < cache->getAssoc(); i++) {
      CacheLine *cl = cache->getPLine(index+i);
      if (cl->isInvalid())
	continue;

      // All the cache lines for this addre must be gone
      I(!(cl->isHit(paddr) && cl->isSafe()));
    }
  }
#endif

  if (!pushReq->getVersionRef()->isKilled()) {
    if (!cl->isInvalid()) {
      GI(cl->hasState(), cl->getLVID() == lvid && cl->isHit(paddr));
      cl->invalidate(); 
    }
    if (lvid->getVersionRef()==0) {
      lvid = findCreateLVID(pushReq);
      I(lvid);
    }
    cl->resetState(addr, lvid, pushReq->getStateRef());
    cl->setMsgSerialNumber(pushReq->getSerialNumber()); 
  }

  ackPushLine(pushReq);
}

void LMVCache::pushLineAck(VMemPushLineReq *pushReq)
{
  I(0);
}

void LMVCache::askPushLine(VMemPushLineReq *askPushReq)
{
  I(0); // Not valid request for the last versioned cache
}


