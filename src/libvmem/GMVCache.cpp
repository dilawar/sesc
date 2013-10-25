/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss

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

#include "GMVCache.h"
#include "HVersion.h"
#include "MemRequest.h"
#include "TaskHandler.h"
#include "Port.h"
#include "EnergyMgr.h"

GStatsCntr *GMVCache::recycleClk=0;

/**********************************
 * GMVCache
 **********************************/

GMVCache::GMVCache(MemorySystem *gms, const char *section, const char *name) 
  : VCache(gms, section, name)
    ,nRecycleAllCache("%s:nRecycleAllCache"  ,name)
    ,nReadHit("%s:nReadHit"    ,name)
    ,nReadHalfHit("%s:nReadHalfHit",name)
    ,nReadMiss("%s:nReadMiss"   ,name)
    ,nServicedFromFuture("%s:nServicedFromFuture",name)
{
  cache = CacheType::create(section,"", name);
  I(cache);

  if (recycleClk ==0)
    recycleClk = new GStatsCntr("VMEM:recycleClk");
}

GMVCache::~GMVCache()
{
  // Do nothing
}

VMemReadReq *GMVCache::read(VMemReadReq *readReq, TimeDelta_t latency)
{
  I(readReq->getType() == VRead);

  rdRevLVIDEnergy->inc();

  PAddr        paddr = readReq->getPAddr();

  //this case is what??? the version is killed, 
  //then why need to set the setCacheSentData()??? add by hr
  if (readReq->getVersionRef()->isKilled()) {
    VMemReadReq *readAckReq = VMemReadReq::createReadAck(this, readReq
							 ,readReq->getVersionDuplicate());

    readReq->incPendingMsg();
    readReq->setCacheSentData();
    readAckReq->setLatency(latency);
    return readAckReq;
  }

  CacheLine *cl = locallySatisfyLine(readReq->getVersionRef(), paddr);
  if (cl) {
    // Same version or immediatly predecessor (if mostSpecBit works,
    // can be any predecessor with the mostSpecBit set)
    I(!cl->isInvalid());
    I(!cl->isKilled());
    
    VMemReadReq *readAckReq = VMemReadReq::createReadAck(this, readReq
							 ,cl->getVersionDuplicate());
    
    readReq->incPendingMsg();

    // No clear or forward when comes from clean future immediate successor
    if (cl->getVersionRef() == readReq->getVersionRef()) {
      I(*(cl->getVersionRef()) == *(readReq->getVersionRef()));
      wrLVIDEnergy->inc();
      readAckReq->getState()->copyStateFrom(cl);
      cl->invalidate();
    }else if (*(cl->getVersionRef()) < *(readReq->getVersionRef())){
      cl->forwardStateTo(readAckReq->getState());
    }
    
    hitInc(MemRead);
    nReadHit.inc();
    
    readReq->setCacheSentData();
    readAckReq->setLatency(latency+hitDelay);
    return readAckReq;
  }
  
  cl = locallyLessSpecLine(readReq->getVersionRef(), paddr);
  if (cl) {
    // Less Spec (not immediatly predecessor)
    I(cl->getVersionRef());
    I(readReq->getVersionRef());
    I(!(*(cl->getVersionRef()) > *(readReq->getVersionRef())));

    halfHitInc(MemRead);
    nReadHalfHit.inc();

    readReq->setCacheSentData();
    VMemReadReq *readAckReq = VMemReadReq::createReadAck(this, readReq
							 ,cl->getVersionDuplicate());
    readReq->incPendingMsg();
    readAckReq->setLatency(latency + hitDelay);
    return readAckReq;
  }

  // FIXME: if immediat succesor of cache line has clean state, it can
  // also sent the cache line data (another opt)

  return 0;
}

VMemWriteReq *GMVCache::writeCheck(VMemWriteReq *vreq, VMemObj *mobj)
{
  // find all the lines that hold versions of the address in the
  // request and send them to the VBus

  CacheLine *cl      = 0;
  PAddr paddr        = vreq->getPAddr();
  VMemWriteReq *nreq = 0;
  
  rdLVIDEnergy->add(cache->getAssoc());

  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i<cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;
    if(cl->isKilled()) {
      bool inv = cl->accessLine();
      I(inv);
      wrLVIDEnergy->inc();
      continue;
    }
    if (!cl->isHit(paddr) || cl->isRestarted())
      continue;
    
    rdHitEnergy->inc();
   
    I(cl->getVersionRef());
    I(vreq->getVersionRef());

    if(*(cl->getVersionRef()) <= *(vreq->getVersionRef())) {
      vreq->setCacheSentData();
    }

    if(cl->getVersionRef() == vreq->getVersionRef()) {
      if(this != vreq->getVMem()) {
	// invalidate line - data has just been written in another processor
	cl->invalidate();
      }
      continue;
    }

    if(*(cl->getVersionRef()) > *(vreq->getVersionRef())) { 
      // version is higher than the one in request
      
      if(nreq) {
	// send previous message
	nreq->setLatency(cachePort->nextSlotDelta()+hitDelay);
	mobj->writeCheckAck(nreq);
      }

      vreq->incnRequests();

      nreq = VMemWriteReq::createWriteCheckAck(this, cl->getVersionDuplicate(), cl, vreq); 
    }
  }

  if(!nreq) {
    vreq->incnRequests();
    nreq = VMemWriteReq::createWriteCheckAck(this, 0, 0, vreq);
  }

  nreq->setLastMessage(true);

  return nreq;
}


void GMVCache::recycleAllCache(const HVersion *exeVer)
{
  // Traverse the whole cache and free LVIDs when possible

  nRecycleAllCache.inc();
  cachePort->occupySlots(cache->getNumSets());
  recycleClk->add(cache->getNumSets());

  I(!lvidTable.hasFreeLVIDs());

  //  Recycle all the LVIDs that got a kill or a restart dirty
  //
  //  Displace all the safe and finished lines
  for (ulong i=0; i < cache->getNumLines(); i++) {
    CacheLine *cl = cache->getPLine(i);
    if (cl->isInvalid())
      continue;
    if (cl->accessLine()) {
      wrLVIDEnergy->inc();
      continue;
    }
    if (cl->isSafe() && cl->isFinished()) {
      displaceLine(cl);
      continue;
    }
  }

  if(lvidTable.hasFreeLVIDs())
    return;

  // Big measures. Kill all the spec tasks. Send them to hell

  lvidTable.killSpecLVIDs(exeVer);
  for (ulong i=0; i < cache->getNumLines(); i++) {
    CacheLine *cl = cache->getPLine(i);
    if (cl->isInvalid())
      continue;
    if (cl->accessLine()) {
      wrLVIDEnergy->inc();
      continue;
    }
  }
  GI(exeVer->isSafe(), lvidTable.hasFreeLVIDs());
#ifdef DEBUG
  if (exeVer->isSafe() && !lvidTable.hasFreeLVIDs()) {
    lvidTable.dump();
    for (ulong i=0; i < cache->getNumLines(); i++) {
      CacheLine *cl = cache->getPLine(i);
      if (cl->isInvalid())
	continue;
      if (cl->accessLine()) {
	wrLVIDEnergy->inc();
	continue;
      }
      if (cl->isSafe() && cl->isFinished()) {
	displaceLine(cl);
	continue;
      }
    }
    lvidTable.dump();
  }
  // The only cache data should be safe & unfinished (1 LVID)
  for (ulong i=0; i < cache->getNumLines(); i++) {
    CacheLine *cl = cache->getPLine(i);
    if (cl->isInvalid())
      continue;
    I(*cl->getVersionRef() <= *exeVer);
  }
#endif

}

int32_t GMVCache::countLinesInSet(PAddr paddr)
{
#ifdef VMEM_CVBASE_IN_ADDR
  I(0);
  // NOTE: there should be an additional structure to track this. Traversing the
  // set would not be enough to find all the address
#endif
  int32_t nLines =0;
  
  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;
    if (!cl->isHit(paddr))
      continue;
    
    nLines++;
  }

  return nLines;
}

GMVCache::CacheLine *GMVCache::getSafeLine(PAddr paddr)
{
#ifdef VMEM_CVBASE_IN_ADDR
  I(0);
  // NOTE: there should be an additional structure to track this. Traversing the
  // set would not be enough to find all the address
#endif
  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;
    if (cl->isHit(paddr) && cl->isSafe())
      return cl;
  }

  return 0;
}

GMVCache::CacheLine *GMVCache::getLineMoreSpecThan(const HVersion *ver, PAddr paddr)
{
#ifdef VMEM_CVBASE_IN_ADDR
  I(0);
  // NOTE: there should be an additional structure to track this. Traversing the
  // set would not be enough to find all the address
#endif

  CacheLine *mostSpecLine     = 0;
  const HVersion *mostSpecVer = ver;

  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;

    if (cl->isKilled()) {
      wrLVIDEnergy->inc();
      cl->invalidate();
      continue;
    }
    

    if ( *(cl->getVersionRef()) > *mostSpecVer) {
      mostSpecVer = cl->getVersionRef();
      mostSpecLine= cl;
    }
  }

  return mostSpecLine;
}

GMVCache::CacheLine *GMVCache::locallySatisfyLine(const HVersion *ver, PAddr paddr)
{
  // Look for a previous version or the same version

  // Conditions when data  can be locally satisfied:
  //
  // 1-Previous version has the mostSpecLine set
  //
  // 2-Immediatly predecessor version
  //
  // 3-Immediatly successor version, as long as it is clean
  //
  // 4-FIXME: If a line has mostSpecLine & leastSpecLine, and it is
  // clean. It is not necessary to go to memory (even if it belongs to
  // a restarted or a kill tasks).

  CacheLine *line=0;

  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl = cache->getPLine(index+i);
    if (cl->isInvalid())
      continue;

    if (cl->accessLine()) {
      wrLVIDEnergy->inc();
      continue;
    }

    GI(cl->getLVID(), !cl->getLVID()->isKilled());

    if (!cl->isHit(paddr)) //this isHit() ensure that the cacheline is same 
      			   //but has different version. add by hr
      continue;
  
#ifdef VMEM_PRED_FORWARD
    if (*(cl->getVersionRef()) < *ver ) {
      if (cl->isMostSpecLine()) {
	// Local predecessor with no successors. Yuppi!!!  No need to
	// keep searching. mostSpecLine set means that no future
	// exists (this includes *ver)
	return cl;
      }

      I(cl->getVersionRef());
      I(cl->getVersionRef()->getNextRef());
      if (cl->getVersionRef()->getNextRef() == ver) {
	I(*(cl->getVersionRef()->getNextRef()) == *ver);
	// Immediatly predecessor can also forward
	line = cl;
      }
    }else{
      I(*(cl->getVersionRef()) >= *ver );
      const HVersion *nextVer = ver->getNextRef();
      if (nextVer)
	if ((nextVer == cl->getVersionRef()) && !cl->isDirty()) {
	  // Immediatly successor can forward data as long as it is clean
	  I(*nextVer == *(cl->getVersionRef()));
	  line = cl;
	  nServicedFromFuture.inc();
	}
    }
#endif
    if (cl->getVersionRef() == ver)
      return cl;

    I(*(cl->getVersionRef()) != *ver);
  }
  
  GI(line, !line->isInvalid());
  GI(line, !line->isKilled());
  
  return line;
}

GMVCache::CacheLine *GMVCache::locallyLessSpecLine(const HVersion *ver, PAddr paddr)
{
  // returns the most speculative line with a version less or equal to ver

  rdLVIDEnergy->add(cache->getAssoc());
  
  CacheLine *cl = 0;

  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *ncl = cache->getPLine(index+i);
    if (ncl->isInvalid())
      continue;
    if (ncl->accessLine()) {
      wrLVIDEnergy->inc();
      continue;
    }
    if (!ncl->isHit(paddr))
      continue;
    if (*(ncl->getVersionRef()) > *ver) 
      continue;
    
    if (cl == 0)
      cl = ncl;
    else if (*(ncl->getVersionRef()) > *(cl->getVersionRef())) {
      // Between cl and version
      cl = ncl;
    }
  }

  return cl;
}

LVID *GMVCache::findCreateLVID(const VMemReq *vreq)
{
  return findCreateLVID(vreq->getVersion());
}

LVID *GMVCache::findCreateLVID(HVersion *ver)
{
  LVID *lvid  = lvidTable.findCreateLVID(ver);
  if (lvid) {
    I(!lvid->isKilled());
    return lvid;
  }
  if (!ver->isSafe())
    return 0;

  recycleAllCache(ver);
  return lvidTable.findCreateLVID(ver);
}

bool GMVCache::checkSet(PAddr paddr)
{
  ulong index = calcIndex4PAddr(paddr);
  for(ulong i=0; i < cache->getAssoc(); i++) {
    CacheLine *cl1 = cache->getPLine(index+i);

    if (cl1->isInvalid() || cl1->isKilled())
      continue;

    for(ulong j=0; j < cache->getAssoc(); j++) {
      CacheLine *cl2 = cache->getPLine(index+j);

      if (cl2->isKilled() || cl2->isInvalid() || cl2 == cl1)
	continue;

      if (!cl2->isHit(cl1->getPAddr()))
	continue;

      if (cl1->getVersionRef() == cl2->getVersionRef()) {
	return false;
      }
    }
  }
  return true;
}
