
/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Karin Strauss
   Modified by Aniket Naik

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
#include <math.h>
#include "TMCache.h"
#include "TMCacheState.h"
#include "Cache.h"

#include "MESIProtocol.h"
#include "Epoch.h"

//Transactions virtualization available
#define TRANS_VERT 1 
//Stall when insufficient space to buffer transaction
//#define TRANS_STALL 0 





// This cache works under the assumption that caches above it in the memory
// hierarchy are write-through caches

// There is a major simplifying assumption for this cache subsystem:
// if there are two requests to the same cache line at the same time,
// only one of them is propagated to the lower levels. Once this
// request is serviced, the other one will be allowed to proceed. This
// is implemented by the mutExclBuffer.

// Another important assumption is that every cache line in the memory
// subsystem is the same size. This is not hard to fix, though. Any
// candidates?

MSHR<PAddr, SMPCache> *SMPCache::mutExclBuffer = NULL;

SMPCache::SMPCache(SMemorySystem *dms,const char *section, const char *name)
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
  #ifdef TLS
  ,epochMissW("%s:Write Miss due to Epoch Missmatch",name)
  ,epochMissR("%s:Read Miss due to Epoch Missmatch",name)
  ,sharedWrites("%s:L2 Shared Writes",name)
  ,writeHitProp("%s:Write hits sent down to system bus",name)
  ,readHitProp("%s:Read hits sent down to system bus",name)
  ,squashes("%s:Backend squashes",name)
  #endif
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
    protocol = new MESIProtocol(this);
  } else {
    MSG("unknown protocol, using MESI");
    protocol = new MESIProtocol(this);    
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
  
  I(addr > 1024); 

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
  
//Line in Cache
  if (l && l->canBeRead()) {
  	#ifdef TLS
  	if (mreq->isDataReq()==true){ 
	  	//Check for Epoch match
	 	//If Epoch mismatch -read miss, invalidate line
	 	if (l->getEpoch()!=mreq->getEpoch())
	 	{
	 		//printf("Line Epoch %d:%d\n",l->getEpoch()->getClock(),l->getEpoch()->getTid());
	 		if (!mutExclBuffer->issue(addr)) {
			   mutExclBuffer->addEntry(addr, sendReadCB::create(this, mreq),
						          sendReadCB::create(this, mreq));
			return;
	 		}
	 		invalidateLineTLS(mreq);
	 		l=0;
	 		mutExclBuffer->retire(addr);
	 		epochMissR.inc();
	 	} 
	  	//READ HIT-----------------------
	  	else
	  	{
		  	//Dont broadcast- if Any word is read exposed or write modify flag for word is set
		  	 if (isLineExposedRead(l) || isWordWriteModified(l,addr))
		   	 {
				
				readHit.inc();
			    outsReq->retire(addr);
			  	(l->getCacheFlags())->setWordER(getOffsetInLine(addr));//WM automatically checked		  	
			  	mreq->goUp(hitDelay);
			  	return;
	  	     }
			//Broadcast read hit on bus
		  	else
		  	{
				  readHitProp.inc();
	  			  readHit.inc();//Propagated hits still counted
				  (l->getCacheFlags())->setWordER(getOffsetInLine(addr));//WM automatically checked		  	
				  if (!mutExclBuffer->issue(addr)) {
				    mutExclBuffer->addEntry(addr, sendReadCB::create(this, mreq),
				      			          sendReadCB::create(this, mreq));
				    return;
				  }
				  sendRead(mreq);
				  return;
		  	}
		}
  	}
  	else
  	{
  	 	readHit.inc();
	    outsReq->retire(addr);
	    mreq->goUp(hitDelay);
	    return;
  	}
	#else 
	    readHit.inc();
	    outsReq->retire(addr);
	    mreq->goUp(hitDelay);
	    return;
  	#endif
  }
//Read Miss------------------

  if (l && l->isLocked()) {
  	//printf("Being Invalidated\n");
    readRetry.inc();
    doReadCB::scheduleAbs(nextSlot(), this, mreq);
    return;
  }
  GI(l, !l->isLocked());

  readMiss.inc(); 

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

//LINE exists in cache
 #ifdef TLS
  	if (l && !l->isLocked()) {
	  	if (mreq->isDataReq()==true){
		  	//Check for Epoch match
		 	//If Epoch mismatch -read miss, invalidate line
		 	if (l->getEpoch()!=mreq->getEpoch())
		 	{
		 		l=0;
		 		 if(!l && mreq->getMemOperation() == MemWrite) {
					    mreq->mutateWriteToRead();
  				}
		 		if (!mutExclBuffer->issue(addr)) {
				   mutExclBuffer->addEntry(addr, sendWriteCB::create(this, mreq),
							          sendReadCB::create(this, mreq));
				return;
		 		}
		 		invalidateLineTLS(mreq);
		 		mutExclBuffer->retire(addr);
		 		epochMissW.inc();
		 	}
		 	//WRITE HIT-----------------------
		  	else
		  	{
			  	//Broadcast write hit on bus- if If others exposed is set
			  	 if (l->getCacheFlags()->getOE())
			  	 {
					  (l->getCacheFlags())->setWordWM(getOffsetInLine(addr));
					    if(mreq->getMemOperation() == MemWrite) {
							    mreq->mutateWriteToRead();
	  					}
					  writeHitProp.inc();//write hit but broadcast
					  writeHit.inc();//Propagated hits still counted
					  if (!mutExclBuffer->issue(addr)) {
					    mutExclBuffer->addEntry(addr, sendWriteCB::create(this, mreq),
					      			          sendWriteCB::create(this, mreq));
					    return;
					  }
					 sendWrite(mreq);
					 return;
		  	     }
			  	//Dont broadcast
			  	else
			  	{
			  	    writeHit.inc();
				    outsReq->retire(addr);
				  	(l->getCacheFlags())->setWordWM(getOffsetInLine(addr));
				  	mreq->goUp(hitDelay);
				  	return;
			  	}
			} 
	  	} 
	  	else{
	  		writeHit.inc();
		    outsReq->retire(addr);
		    mreq->goUp(hitDelay);  
		    return;
	  	}
  	}	
	#else
     if (l && l->canBeWritten()) {
	    writeHit.inc();
	    outsReq->retire(addr);
	    mreq->goUp(hitDelay);  
	    return;
     }
   #endif

//LINE LOCKED RETRY
  if (l && l->isLocked()) {
	    writeRetry.inc();
	    doWriteCB::scheduleAbs(nextSlot(), this, mreq);
	    return;
  }
  GI(l, !l->isLocked());

  // this should never happen unless this is highest level because L2
  // is inclusive; there is only one case in which this could happen when 
  // SMPCache is used as an L2:
  // 1) line is written in L1 and scheduled to go down to L2
  // 2) line is invalidated in both L1 and L2
  // 3) doWrite in L2 is executed after line is invalidated

  if(!l && mreq->getMemOperation() == MemWrite) {
    mreq->mutateWriteToRead();
  }

  writeMiss.inc();

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
  I(oc);
  I(pendInvTable.find(addr) == pendInvTable.end());
  pendInvTable[addr].outsResps = getNumCachesInUpperLevels();
  pendInvTable[addr].cb = doInvalidateCB::create(oc, addr, size);
  pendInvTable[addr].invalidate = true;

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

  PendInvTable::iterator it = pendInvTable.find(addr);
  Entry *record = &(it->second);
  if(--(record->outsResps) <= 0) {
    cb = record->cb;
    invalidate = record->invalidate;
    pendInvTable.erase(addr);
  }

  if(invalidate)
    realInvalidate(addr, size);

  if(cb)
    EventScheduler::schedule((TimeDelta_t) 2,cb);
    
}

void SMPCache::realInvalidate(PAddr addr, ushort size)
{
  while(size) {

    Line *l = cache->findLine(addr);
    
    if (l) {
      nextSlot(); // counts for occupancy to invalidate line
      I(l->isValid());
      if (l->isDirty()) {
	invalDirty.inc();
	doWriteBack(addr);
      } 
      l->invalidate();
    }
    addr += cache->getLineSize();
    size -= cache->getLineSize();
  }
}
#ifdef TLS
void SMPCache::realInvalidateTLS(PAddr addr, ushort size)
{
  //Invalidate without writeback
  while(size) {

    Line *l = cache->findLine(addr);
    if (l) {
      nextSlot(); // counts for occupancy to invalidate line
      I(l->isValid());
      l->invalidate();
    }
    addr += cache->getLineSize();
    size -= cache->getLineSize();
  }
}
#endif

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
# ifndef TLS
  if(sreq->getSupplier() == this && sreq->needsWriteDown()) {
    doWriteBack(addr);
  }
#endif
  lowerLevel[0]->access(sreq);
}

// receives requests from other caches
void SMPCache::receiveFromBelow(SMPMemRequest *sreq) {
//TLS error- for writeback miss delay will incur needless delay- FIX
  doReceiveFromBelowCB::scheduleAbs(nextSlot() + missDelay, this, sreq);
}

void SMPCache::doReceiveFromBelow(SMPMemRequest *sreq)
{
  MemOperation memOp = sreq->getMemOperation();
  //printf("Symbolic Name: %s\n",symbolicName);
  // request from other caches, need to process them accordingly
  	PAddr addr= sreq->getPAddr();
	Line *l = cache->findLine(addr);
  //READ HIT/MISS------------
  if(memOp == MemRead) {
#ifdef TLS
	//check if cache has line marked with epoch
	//Add the chache flags to the request
	if(sreq->isDataReq() && l)
	{
		if (sreq->needsData())	//READ MISS was broadcast
		{
			MemRequest *oreq=sreq->getOriginalRequest();
			//If Line epoch is < request epoch set Others exposed
			long int32_t tmpclk=((oreq->getEpoch())->getClock());
			if ((l->getEpoch())->getClock()<tmpclk)
			{
				l->getCacheFlags()->setOE();
			}
			//READ MISS--- then Add line cache flags to request
			if (sreq->needsData())
		 	{
				//HACK: some bug causes this value already set to disappear
				(l->getCacheFlags())->setEpoch(l->getEpoch());
				sreq->setCacheFlags(l->getCacheFlags());
			}
		}
		else //READ HIT was broadcast
		{
			MemRequest *oreq=sreq->getOriginalRequest();
			//If Line epoch is < request epoch set Others exposed
			long int32_t tmpclk=((oreq->getEpoch())->getClock());
			if ((l->getEpoch())->getClock()<tmpclk)
			{
				l->getCacheFlags()->setOE();
			}
		}

	}
#endif
  protocol->readMissHandler(sreq);
  return;
  }
  //WRITE HIT/MISS----------
  if(memOp == MemReadW) {
    //L1 Write Miss
    if(sreq->needsData()) 
    {
	    #ifdef TLS
			//Write miss -check if cache has line marked with epoch
			if (sreq->isDataReq() && l)
			{
				MemRequest *oreq=sreq->getOriginalRequest();
				//SQUASH EPOCH!!!
				if (((l->getEpoch())->getClock()>((oreq->getEpoch())->getClock())) && l->getCacheFlags()->getWordER(getOffsetInLine(addr)))
				{
	 				//realInvalidateTLS(addr,cache->getLineSize());
	 				int32_t sqhCount=squash(l->getEpoch());
					printf("Backend Squash: Invalidated lines- %ld\n",sqhCount);
					squashes.inc();
				}
				//WRITE MISS--- then Add line cache flags to request
				else if (sreq->needsData())
				{
					//HACK: some bug causes this value already set to disappear
					(l->getCacheFlags())->setEpoch(l->getEpoch());
					sreq->setCacheFlags(l->getCacheFlags());
				}
			}
		#endif
	    protocol->writeMissHandler(sreq);
	    return;
    }
    else{
     //WRITE HIT
	#ifdef TLS
		//Write Hit -check if cache has line marked with epoch
		if (sreq->isDataReq() && l)
		{
			MemRequest *oreq=sreq->getOriginalRequest();
			//SQUASH EPOCH!!!
			if (((l->getEpoch())->getClock()>((oreq->getEpoch())->getClock())) && l->getCacheFlags()->getWordER(getOffsetInLine(addr)))
			{
 				//realInvalidateTLS(addr,cache->getLineSize());
	 				int32_t sqhCount=squash(l->getEpoch());
					printf("Backend Squash: Invalidated lines- %ld\n",sqhCount);
					squashes.inc();
			}
			//WRITE Hit--- then Add line cache flags to request
			else if (sreq->needsData())
			{
				//HACK: some bug causes this value already set to disappear
				(l->getCacheFlags())->setEpoch(l->getEpoch());
				sreq->setCacheFlags(l->getCacheFlags());
			}
		}

	    protocol->writeMissHandler(sreq);
	    return;
	#else
	     protocol->invalidateHandler(sreq);
	#endif
    }
    return;
  }
#ifdef TLS 
//L1 HIT,L2 HIT 
//INVALIDATION MESSAGE FROM ANOTHER CACHE AS IT WANTS IT IN EXCLUSIVE STATE
if(memOp == MemWrite) {
    protocol->invalidateHandler(sreq);
    return;
}
#else
  if(memOp == MemWrite) {
    I(!sreq->needsData());
    protocol->invalidateHandler(sreq);
    return;
  }
#endif 
#ifdef TLS
	if (memOp==MemPush)
	{	
		if (l)
		{
			MemRequest *oreq=sreq->getOriginalRequest();
			//All epochs preceeding the one being written back MUST be commited
			if ((l->getEpoch())->getClock()<((oreq->getEpoch())->getClock()))
			{
				//HACK: some bug causes this value already set to disappear
				(l->getCacheFlags())->setEpoch(l->getEpoch());
				sreq->setCacheFlags(l->getCacheFlags());
	 			realInvalidateTLS(addr,cache->getLineSize());
			}
		}
  		sreq->goDown(0, lowerLevel[0]);
		return;
	}
#else
  I(0); // this routine should not be called for any other memory request
#endif
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
#ifdef TLS
	SMPCache::Line *SMPCache::allocateLine(PAddr addr, MemRequest *mreq, CallbackBase *cb)
	{
	  PAddr rpl_addr = 0;
	  I(cache->findLine(addr)==0);
	  //I(cache->findLineDebug(addr) == 0);
	  if (cache->findLineDebug(addr))
	  	printf("Error\n");
	  Line *l = cache->findLine2Replace(addr);
	
	  if(!l) {
	    // need to schedule allocate line for next cycle
	    doAllocateLineCB::scheduleAbs(globalClock+1, this, addr, 0, mreq,cb);
	    return 0;
	  }
	
	  rpl_addr = cache->calcAddr4Tag(l->getTag());
	  lineFill.inc();
	
	  nextSlot(); // have to do an access to check which line is free
	
	  if(!l->isValid()) {
	    cb->destroy();
	    l->setTag(cache->calcTag(addr));
	    return l;
	  }
	  
	  if(isHighestLevel()) {
	    if(l->isDirty()) {
	      allocDirty.inc();
	      //This prevents further acces to this line  	
		  //l->changeStateTo(SMP_TRANS_INV_D);
	      doWriteBack(mreq);
	    } 
	
	    cb->destroy();
	    l->invalidate();
	    l->setTag(cache->calcTag(addr));
	    return l;
	  }
	
	  I(pendInvTable.find(rpl_addr) == pendInvTable.end());
	  pendInvTable[rpl_addr].outsResps = getNumCachesInUpperLevels();
	  pendInvTable[rpl_addr].cb = doAllocateLineCB::create(this, addr, rpl_addr, mreq,cb);
	  pendInvTable[rpl_addr].invalidate = false;
	
	  protocol->preInvalidate(l);
	
	  invUpperLevel(rpl_addr, cache->getLineSize(), this);
	
	  return 0;
	}
	void SMPCache::doAllocateLine(PAddr addr, PAddr rpl_addr, MemRequest *mreq,CallbackBase *cb)
	{
	  Line *l = cache->findLine(rpl_addr); 
	  I(l && l->isLocked());
	
	  if(l->isDirty()) {
	    allocDirty.inc();
	    //This prevents further acces to this line  	
	    //l->changeStateTo(SMP_TRANS_INV_D);
	    doWriteBack(mreq);
	  }
	
	  I(cb);
	  l->setTag(cache->calcTag(addr));
	  l->changeStateTo(SMP_TRANS_RSV);
	  cb->call();
	}
	
#else
	SMPCache::Line *SMPCache::allocateLine(PAddr addr, CallbackBase *cb)
	{
	  PAddr rpl_addr = 0;
	  I(cache->findLine(addr)==0);
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
	    cb->destroy();
	    l->setTag(cache->calcTag(addr));
	    return l;
	  }
	  
	  if(isHighestLevel()) {
	    if(l->isDirty()) {
	      allocDirty.inc();
	      doWriteBack(rpl_addr);
	    } 
	
	    cb->destroy();
	    l->invalidate();
	    l->setTag(cache->calcTag(addr));
	    return l;
	  }
	
	  I(pendInvTable.find(rpl_addr) == pendInvTable.end());
	  pendInvTable[rpl_addr].outsResps = getNumCachesInUpperLevels();
	  pendInvTable[rpl_addr].cb = doAllocateLineCB::create(this, addr, rpl_addr, cb);
	  pendInvTable[rpl_addr].invalidate = false;
	
	  protocol->preInvalidate(l);
	
	  invUpperLevel(rpl_addr, cache->getLineSize(), this);
	
	  return 0;
	}
	
void SMPCache::doAllocateLine(PAddr addr, PAddr rpl_addr, CallbackBase *cb)
{
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


#endif


SMPCache::Line *SMPCache::getLine(PAddr addr)
{
  nextSlot(); 
  return cache->findLine(addr);
}

void SMPCache::writeLine(PAddr addr) {
  Line *l = cache->writeLine(addr);
  I(l);
}

void SMPCache::invalidateLine(PAddr addr, CallbackBase *cb) 
{
  Line *l = cache->findLine(addr);
  
  I(l);

  I(pendInvTable.find(addr) == pendInvTable.end());
  pendInvTable[addr].outsResps = getNumCachesInUpperLevels();
  pendInvTable[addr].cb = cb;
  pendInvTable[addr].invalidate = true;

  protocol->preInvalidate(l);

  if(!isHighestLevel()) {
    invUpperLevel(addr, cache->getLineSize(), this);
    return;
  }

  doInvalidate(addr, cache->getLineSize());
}

void SMPCache::inclusionCheck(PAddr addr) {
  const LevelType* la = getUpperLevel();
  MemObj* c  = (*la)[0];

  const LevelType* lb = c->getUpperLevel();
  MemObj*    cc = (*lb)[0];

  I(!((Cache*)cc)->isInCache(addr));
}

#ifdef TLS 
void SMPCache::sendBelowUp(SMPMemRequest *sreq, MemRequest *mreq)
{
  sreq->goDown(missDelay, lowerLevel[0]);
//Wont work just yet, prevent the above sreq from coming up.
//  readHit.inc();
//  outsReq->retire(mreq->getPaddr());
//  mreq->goUp(missDelay);
  
}
  
SMPCache::Line * SMPCache::getLineLE(PAddr addr, tls::Epoch * eph)
{
 nextSlot(); 
 Line *l= cache->findLine(addr);
 if ((l->getEpoch())->getClock() < eph->getClock())
 	return l;
 else return 0;
}

void SMPCache::sendUp(MemRequest *mreq)
{
	PAddr addr=mreq->getPAddr();
	writeHit.inc();
	sharedWrites.inc();
    outsReq->retire(addr);
	mreq->goUp(hitDelay);
}


uint32_t SMPCache::getOffsetInLine(PAddr addr)
{
	if (cache->getLineSize()!=32)
		printf("Check allocation for cacheFlags!!!!\n");
	uint32_t temp,shft;
	shft=(int)(log10((double)cache->getLineSize())/(double)log10(2.0));
	temp= addr >> shft;
	temp=addr-(temp<<shft);
	return (uint32_t)temp;
}
void SMPCache::doWriteBack(MemRequest *mreq)
{
  // FIXME: right now we are assuming cache line sizes are same in every cache

  bool commitResult;
  RAddr addr;  
  int32_t tid;
    
   Line *l = cache->findLine2Replace(mreq->getPAddr());
   PAddr rpl_addr = cache->calcAddr4Tag(l->getTag());
  

  	if (l->getEpoch())
  	{
  	tid=l->getEpoch()->getTid();
  	}	
  	else
  	{
  	 	writeBack.inc();
 	 	protocol->sendWriteBack(mreq);
 	 	return;
   	}
  	#ifdef TRANS_STALL
	  	//Check if line is marked with a tag later than mreq tag. In this case ignore
		if ((tid==mreq->getEpoch()->getTid()) &&(mreq->getEpoch()->getClock() < l->getEpoch()->getClock()))
		{	
			printf("Out of order Memory Tagging Request %d:%d, Line %d:%d\n",mreq->getEpoch()->getTid(),mreq->getEpoch()->getClock(),tid,l->getEpoch()->getClock());
	  		commitResult= true;
	  	}
	 	else if ((tid==mreq->getEpoch()->getTid()) &&(mreq->getEpoch()->getClock() < l->getEpoch()->getClock()))
	 	{
	  		#ifdef TRANS_VIRT //if memory virtualization assumed
	  			commitResult=true;
	  		#else
	  			commitResult=l->getEpoch()->requestBlockRemovalWB(rpl_addr,doBlockRemovedCB::create(this, mreq));
	  		#endif
	 	}
	 	else
		 	commitResult=l->getEpoch()->requestBlockRemovalWB(rpl_addr,doBlockRemovedCB::create(this, mreq));
	#else
	  commitResult=true;
	#endif
  if (commitResult)
  {
  	writeBack.inc();
  	protocol->sendWriteBack(mreq);
  }
  //else do block removed called
  else
  {
  	mreq->setStall();
  	printf("Request %u:%u stalled on line-Epoch %u:%u Stall address %u\n",mreq->getEpoch()->getClock(),mreq->getEpoch()->getTid(),l->getEpoch()->getClock(),l->getEpoch()->getTid(),rpl_addr);
 	writeBack.inc();
  	protocol->sendWriteBack(mreq);
  }
 }

void SMPCache::doBlockRemoved(MemRequest *mreq)
{
 	//writeBack.inc();
  	//protocol->sendWriteBack(mreq);
  	mreq->clearStall();
  	printf("Stall cleared %p\n",mreq);
  	//Send request up if a request is missed, else a go up is scheduled which has not stalled
  	if (mreq->isStalledOnce()==true) mreq->goUp(0);
}

void SMPCache:: invalidateLineTLS(MemRequest *mreq)
{
 PAddr addr= mreq->getPAddr();
   doInvalidateLineTLS(mreq);
}
void SMPCache::doInvalidateLineTLS(MemRequest *mreq)
{
  PAddr addr=mreq->getPAddr();
  Line *l = cache->findLine(addr);
  ushort size =cache->getLineSize();
  I(l);
  I(pendInvTable.find(addr) == pendInvTable.end());
  pendInvTable[addr].outsResps = getNumCachesInUpperLevels();
  pendInvTable[addr].cb = dummyCB::create(this,mreq);;
  pendInvTable[addr].invalidate = true;

  protocol->preInvalidate(l);

  if(!isHighestLevel()) {
    invUpperLevel(addr, cache->getLineSize(), this);
    return;
  }

  //doInvalidate(addr, cache->getLineSize());
///Do invalidate
  I(pendInvTable.find(addr) != pendInvTable.end());
  CallbackBase *cb = 0;
  bool invalidate = false;

  PendInvTable::iterator it = pendInvTable.find(addr);
  Entry *record = &(it->second);
  if(--(record->outsResps) <= 0) {
    cb = record->cb;
    invalidate = record->invalidate;
    pendInvTable.erase(addr);
  }

 
  if(cb)
    EventScheduler::schedule((TimeDelta_t) 2,cb);
  
   //if(invalidate)
    //realInvalidate(addr, size);
  if(invalidate)
  {
	    while(size) {
	
	    //Line *l = cache->findLine(addr);
	    
	    if (l) {
	      nextSlot(); // counts for occupancy to invalidate line
	      I(l->isValid());
	      if (l->isDirty()) {
		invalDirty.inc();
			//This prevents further acces to this line  	
	    	//l->changeStateTo(SMP_TRANS_INV_D);
			doWriteBack(mreq);
	      } 
	      l->invalidate();
	    }
	    addr += cache->getLineSize();
	    size -= cache->getLineSize();
	  }
  }
  //mutExclBuffer->retire(addr);
}

int32_t SMPCache::squash(tls::Epoch *epoch)
{
	uint32_t i,count=0;
	Line *l;
	for (i=0;i<cache->getNumLines();i++)
	{
		l=cache->getPLine(i);
		if (l->getEpoch()==epoch)
		{
			l->invalidate();
			count++;
		}
	}
	return count;
}
#endif

