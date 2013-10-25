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

#include "SMPDebug.h"
#include "SMPMemRequest.h"
#include "SMPCacheState.h"
#include "MemObj.h"

pool<SMPMemRequest> SMPMemRequest::rPool(256);

SMPMemRequest::SMPMemRequest()
  : MemRequest()
{
}

SMPMemRequest *SMPMemRequest::create(MemRequest *mreq, 
				     MemObj *reqCache,
				     bool sendData)
{
  SMPMemRequest *sreq = rPool.out();

  I(mreq);
  sreq->oreq = mreq;
  IS(sreq->acknowledged = false);
  I(sreq->memStack.empty());

  sreq->currentClockStamp = globalClock;

  sreq->pAddr = mreq->getPAddr();
  sreq->memOp = mreq->getMemOperation();
  sreq->dataReq = mreq->isDataReq();
  sreq->prefetch = mreq->isPrefetch();

  sreq->state = SMP_INVALID;

  I(reqCache);
  sreq->requestor = reqCache;
  sreq->supplier = 0;
  sreq->currentMemObj = reqCache;

  sreq->writeDown = false;
  sreq->needData = sendData;
 
  if(sreq->memOp == MemPush) 
    sreq->needSnoop = false;
  else
    sreq->needSnoop = true;

  sreq->found = false;
  sreq->nUses = 1;

  sreq->cb = 0;

  return sreq;
}

SMPMemRequest *SMPMemRequest::create(MemObj *reqCache, 
				     PAddr addr, 
				     MemOperation mOp,
				     bool needsWriteDown,
				     CallbackBase *cb)
{
  SMPMemRequest *sreq = rPool.out();

  sreq->oreq = 0;
  IS(sreq->acknowledged = false);
  I(sreq->memStack.empty());

  sreq->currentClockStamp = globalClock;

  sreq->pAddr = addr;
  sreq->memOp = mOp;
  sreq->cb = cb;

  sreq->dataReq = false;
  sreq->prefetch = false;

  sreq->state = SMP_INVALID;

  I(reqCache);
  sreq->requestor = reqCache;
  sreq->supplier = 0;
  sreq->currentMemObj = reqCache;

  sreq->writeDown = needsWriteDown;
  sreq->needData = false; // TODO: check this (should it be true?)
 
  if(sreq->memOp == MemPush) 
    sreq->needSnoop = false;
  else
    sreq->needSnoop = true;

  sreq->found = false;
  sreq->nUses = 1;

  return sreq;
}

void SMPMemRequest::incUses()
{
  nUses++;
}

void SMPMemRequest::destroy()
{
  nUses--;
  if(!nUses) {

    GLOG(SMPDBG_MSGS, "sreq %p real destroy", this);
    I(memStack.empty());
    rPool.in(this);
    return;
  } 

  GLOG(SMPDBG_MSGS, "sreq %p fake destroy", this);
}

VAddr SMPMemRequest::getVaddr() const
{
  I(0);

  if(oreq)
    return oreq->getVaddr();

  return 0;
}

PAddr SMPMemRequest::getPAddr() const
{
  return pAddr;
}

void SMPMemRequest::ack(TimeDelta_t lat)
{
  I(memStack.empty());
  I(acknowledged == false);
  IS(acknowledged = true);
  I(lat == 0);

  if (cb==0) {
    destroy();
    return; // avoid double ack
  }

  CallbackBase *ncb=cb;
  cb = 0;
  ncb->call();
  destroy();
}

void SMPMemRequest::setState(uint32_t st)
{
  state = st;
}

void SMPMemRequest::setSupplier(MemObj *supCache)
{
  supplier = supCache;
}

MemRequest *SMPMemRequest::getOriginalRequest()
{
  return oreq;
}

MemOperation SMPMemRequest::getMemOperation()
{
  return memOp;
}

uint32_t SMPMemRequest::getState()
{
  return state;
}

MemObj *SMPMemRequest::getRequestor()
{
  return requestor;
}

MemObj *SMPMemRequest::getSupplier()
{
  return supplier;
}


