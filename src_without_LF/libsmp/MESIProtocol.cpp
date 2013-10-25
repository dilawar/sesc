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

#include "MESIProtocol.h"

MESIProtocol::MESIProtocol(SMPCache *cache, const char *name)
  : SMPProtocol(cache)
{
  protocolType = MESI_Protocol;
}

MESIProtocol::~MESIProtocol()
{
  // nothing to do
}

void MESIProtocol::changeState(Line *l, unsigned newstate)
{
  // should use invalidate interface 
  I(newstate != MESI_INVALID);
  
  ID(unsigned currentstate);
  IS(currentstate = l->getState());
  GI(currentstate == MESI_INVALID,      newstate == MESI_TRANS_RSV);
  GI(currentstate == MESI_TRANS_RSV,    newstate == MESI_TRANS_RD ||
                                        newstate == MESI_TRANS_WR);
  GI(currentstate == MESI_TRANS_RD,     newstate == MESI_SHARED ||
                                        newstate == MESI_TRANS_RD_MEM);
  GI(currentstate == MESI_TRANS_RD_MEM, newstate == MESI_EXCLUSIVE);
  GI(currentstate == MESI_TRANS_WR,     newstate == MESI_MODIFIED ||
                                        newstate == MESI_TRANS_WR_MEM);
  GI(currentstate == MESI_TRANS_WR_MEM, newstate == MESI_MODIFIED);
  GI(currentstate == MESI_EXCLUSIVE,    newstate == MESI_SHARED ||
                                        newstate == MESI_MODIFIED ||
                                        newstate == MESI_TRANS_INV);
  GI(currentstate == MESI_SHARED,       newstate == MESI_SHARED ||
                                        newstate == MESI_MODIFIED ||
                                        newstate == MESI_TRANS_INV ||
                                        newstate == MESI_TRANS_WR);
  GI(currentstate == MESI_MODIFIED,     newstate == MESI_MODIFIED ||
                                        newstate == MESI_SHARED ||
                                        newstate == MESI_TRANS_INV_D);
  
  l->changeStateTo(newstate);
}

void MESIProtocol::makeDirty(Line *l)
{
  I(l);
  I(l->isValid());
  
  changeState(l, MESI_MODIFIED);
}

// preserves the dirty state while the cache 
// is being invalidated in upper levels
void MESIProtocol::preInvalidate(Line *l)
{
  I(l);
  I(l->isValid());
  
  if(l->isDirty()) 
    changeState(l, MESI_TRANS_INV_D);
  else
    changeState(l, MESI_TRANS_INV);
}

void MESIProtocol::read(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();

  // time for this has already been accounted in SMPCache::read
  Line *l = pCache->getLine(addr);

  // if line is in transient state, read should not have been called
  GI(l, !l->isLocked()); 

  if(!l)
    l = pCache->allocateLine(addr, doReadCB::create(this, mreq));

  if(!l) {
    // not possible to allocate a line, will be called back later
    return;
  }

  changeState(l, MESI_TRANS_RSV);
  doRead(mreq);
}

void MESIProtocol::doRead(MemRequest *mreq)
{
  Line *l = pCache->getLine(mreq->getPAddr());
  I(l);

  GI(l->isLocked(), l->getState() == MESI_TRANS_RSV);

  // go into transient state and send request out
  changeState(l, MESI_TRANS_RD);

  sendReadMiss(mreq);
}

void MESIProtocol::write(MemRequest *mreq)
{
  Line *l = pCache->getLine(mreq->getPAddr());

  // if line is in transient state, write should not have been called
  GI(l, !l->isLocked() && l->getState() == MESI_SHARED); 

  // hit in shared state
  if (l && !l->canBeWritten()) {
    changeState(l, MESI_TRANS_WR);
    sendInvalidate(mreq);
    return;
  }

  // miss - check other caches
  GI(l, !l->isValid());
  if (mreq->getMemOperation() != MemReadW)
    mreq->mutateWriteToRead();

  l = pCache->allocateLine(mreq->getPAddr(), doWriteCB::create(this, mreq));


  if(!l) {
    // not possible to allocate a line, will be called back later
    return;
  }

  changeState(l, MESI_TRANS_RSV);
  doWrite(mreq);
}

void MESIProtocol::doWrite(MemRequest *mreq)
{
  Line *l = pCache->getLine(mreq->getPAddr());

  GI(l->isLocked(), l->getState() == MESI_TRANS_RSV);

  changeState(l, MESI_TRANS_WR);

  sendWriteMiss(mreq);
}

void MESIProtocol::sendReadMiss(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();
  doSendReadMiss(mreq);
}

void MESIProtocol::doSendReadMiss(MemRequest *mreq)
{
  I(pCache->getLine(mreq->getPAddr()));
  SMPMemRequest *sreq = SMPMemRequest::create(mreq, pCache, true);

  pCache->sendBelow(sreq); 
}

void MESIProtocol::sendWriteMiss(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();
  doSendWriteMiss(mreq);
}

void MESIProtocol::doSendWriteMiss(MemRequest *mreq)
{
  I(pCache->getLine(mreq->getPAddr()));
  SMPMemRequest *sreq = SMPMemRequest::create(mreq, pCache, true);
    
  pCache->sendBelow(sreq); 
}

void MESIProtocol::sendInvalidate(MemRequest *mreq)
{
  PAddr addr = mreq->getPAddr();
  doSendInvalidate(mreq);
}

void MESIProtocol::doSendInvalidate(MemRequest *mreq)
{
  I(pCache->getLine(mreq->getPAddr()));
  SMPMemRequest *sreq = SMPMemRequest::create(mreq, pCache, false);
    
  pCache->sendBelow(sreq);
}

void MESIProtocol::sendReadMissAck(SMPMemRequest *sreq)
{
  pCache->respondBelow(sreq);
}

void MESIProtocol::sendWriteMissAck(SMPMemRequest *sreq)
{
  pCache->respondBelow(sreq);
}

void MESIProtocol::sendInvalidateAck(SMPMemRequest *sreq)
{
  pCache->respondBelow(sreq);
}

void MESIProtocol::readMissHandler(SMPMemRequest *sreq) 
{
  PAddr addr = sreq->getPAddr();
  Line *l = pCache->getLine(addr);
  
  if(l && !l->isLocked()) {
    combineResponses(sreq, (MESIState_t) l->getState());
    changeState(l, MESI_SHARED);
  } 

  sendReadMissAck(sreq);
  //sendData(sreq);
}

void MESIProtocol::writeMissHandler(SMPMemRequest *sreq) 
{
  PAddr addr = sreq->getPAddr();
  Line *l = pCache->getLine(addr);
  
  if(l && !l->isLocked()) {
    combineResponses(sreq, (MESIState_t) l->getState());
    pCache->invalidateLine(addr, sendWriteMissAckCB::create(this, sreq), false);
    return;
  } else {
    sendWriteMissAck(sreq);
  } 

  //sendData(sreq);
}

void MESIProtocol::invalidateHandler(SMPMemRequest *sreq) 
{
  PAddr addr = sreq->getPAddr();
  Line *l = pCache->getLine(addr);
  
  if(l && !l->isLocked()) {
    combineResponses(sreq, (MESIState_t) l->getState());
    pCache->invalidateLine(addr, sendInvalidateAckCB::create(this, sreq), false);
    return;
  } else {
    sendInvalidateAck(sreq);
  } 
 
}

void MESIProtocol::readMissAckHandler(SMPMemRequest *sreq) 
{
  PAddr addr = sreq->getPAddr();
  Line *l = pCache->getLine(addr);

  I(l);

  if(sreq->getState() == MESI_INVALID && l->getState() == MESI_TRANS_RD) {
    I(!sreq->isFound());
    changeState(l, MESI_TRANS_RD_MEM);
    sreq->noSnoop();
    pCache->sendBelow(sreq); // miss delay may be counted twice
    return;
  }
  
  if(sreq->getState() == MESI_INVALID) {
    I(l->getState() == MESI_TRANS_RD_MEM);
    changeState(l, MESI_EXCLUSIVE);
  } else {
    I(l->getState() == MESI_TRANS_RD);
    changeState(l, MESI_SHARED);
  }

  pCache->writeLine(addr);
  pCache->concludeAccess(sreq->getOriginalRequest());
  sreq->destroy();
}

void MESIProtocol::writeMissAckHandler(SMPMemRequest *sreq) 
{
  PAddr addr = sreq->getPAddr();
  Line *l = pCache->getLine(addr);

  I(l);
  I(sreq->needsData());

  if(sreq->getState() == MESI_INVALID && l->getState() == MESI_TRANS_WR) {
    I(!sreq->isFound());
    changeState(l, MESI_TRANS_WR_MEM);
    sreq->noSnoop();
    pCache->sendBelow(sreq);
    return;
  }

  I(l->getState() == MESI_TRANS_WR || l->getState() == MESI_TRANS_WR_MEM);
  changeState(l, MESI_MODIFIED);

  pCache->writeLine(addr);
  pCache->concludeAccess(sreq->getOriginalRequest());
  sreq->destroy();
}

void MESIProtocol::invalidateAckHandler(SMPMemRequest *sreq) 
{
  PAddr addr = sreq->getPAddr();
  Line *l = pCache->getLine(addr);

  I(l);
  I(l->getState() == MESI_TRANS_WR);
  changeState(l, MESI_MODIFIED);

  pCache->concludeAccess(sreq->getOriginalRequest());
  sreq->destroy();
}

void MESIProtocol::sendDisplaceNotify(PAddr addr, CallbackBase *cb)
{
  I(pCache->getLine(addr));
  SMPMemRequest *sreq = SMPMemRequest::create(pCache, addr, MemPush, false, cb);
  pCache->sendBelow(sreq);
}

void MESIProtocol::sendData(SMPMemRequest *sreq)
{
  I(0);
}

void MESIProtocol::dataHandler(SMPMemRequest *sreq) 
{
  // this should cause an access to the cache
  // for now, I am assuming data comes with response, 
  // so the access is done in the ack handlers
  I(0);
}

void MESIProtocol::combineResponses(SMPMemRequest *sreq, 
					   MESIState_t localState)
{
  MESIState_t currentResponse = (MESIState_t) sreq->getState();

  if((localState == MESI_INVALID) || (localState & MESI_TRANS)) {
    sreq->setState(currentResponse);
    return;
  }

  if(localState == MESI_SHARED) {
    I(currentResponse != MESI_EXCLUSIVE && currentResponse != MESI_MODIFIED);
    if(!sreq->isFound()) {
      sreq->setFound();
      sreq->setSupplier(pCache);
      sreq->setState(localState);
    }
    return;
  }

  if(localState == MESI_EXCLUSIVE) {
    I(currentResponse != MESI_EXCLUSIVE && currentResponse != MESI_MODIFIED);
    sreq->setFound();
    sreq->setSupplier(pCache);
    sreq->setState(localState);
    return;
  }

  I(localState == MESI_MODIFIED);
  I(currentResponse != MESI_EXCLUSIVE && currentResponse != MESI_MODIFIED);
  sreq->setFound();
  sreq->setSupplier(pCache);
  sreq->setWriteDown();
  sreq->setState(localState);
}
