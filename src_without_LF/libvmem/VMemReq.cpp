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

#include "VMemReq.h"
#include "MemRequest.h"
#include "LVIDTable.h"
#include "VMemObj.h"

pool<VMemReadReq>     VMemReadReq::rPool(256, "VMemReadReq");
pool<VMemWriteReq>    VMemWriteReq::rPool(256, "VMemWriteReq");
pool<VMemPushLineReq> VMemPushLineReq::rPool(256, "VMemPushLineReq");

#ifdef DEBUG
uint32_t VMemReq::snCount = 0;
#endif

/**********************************
 * VMemReq
 **********************************/

void VMemReq::init(VMemReqType t, VMemObj *m, HVersion *v, PAddr p) 
{
  type  = t;
  paddr = p;
  vmem  = m;
  ver   = v;
  nReq  = 0;

  I(m);

  lat = 0;

  nPendingMsg = 0;
  state.clearState();

  IS(serialNumber = snCount++;);
}

void VMemReq::changeVersion(HVersion *v)
{
  if (ver)
    ver->garbageCollect();
  
  ver = v;
}

/**********************************
 * VMemReadReq
 **********************************/

VMemReadReq *VMemReadReq::createRead(VMemObj *c, HVersion *ver, MemRequest *mreq)
{
  VMemReadReq *req   = rPool.out();
  
  req->init(VRead, c, ver, mreq->getPAddr());
  req->mreq  = mreq;
  req->cacheSentData = false;
  req->memSentData   = false;
  req->wait4PushLine = false;
  mreq->setVMemReq(req);

  I(mreq->getMemOperation() == MemRead) ;
  if(mreq->getMemOperation() != MemRead){
    int32_t j = rand();
  }
  
  I(req->getType() == VRead);

  req->state.clearState();

  return req;
}

VMemReadReq *VMemReadReq::createReadAck(VMemObj *origCache, VMemReadReq *origReq, HVersion *verAck)
{
  VMemReadReq *req   = rPool.out();

  GI(origReq->hasMemRequestPending(), origReq->getMemRequest()->getMemOperation() == MemRead);

  I(origReq->type == VRead);

  req->init(VReadAck, origReq->vmem, verAck, origReq->paddr);
  IS(req->mreq = 0);
  req->cacheSentData = false;
  req->memSentData   = false;
  req->wait4PushLine = false;

  req->origReq   = origReq;
  req->origCache = origCache;

  origReq++;

  req->state.clearState();
  
  return req;
}

void VMemReadReq::destroy()
{
  I(mreq==0);
  I(nPendingMsg==0);
  if(ver)
    ver->garbageCollect();
  IS(ver=0);
  rPool.in(this);
}

/**********************************
 * VMemWriteReq
 **********************************/

VMemWriteReq *VMemWriteReq::createWriteCheck(VMemObj *c, HVersion *v
					     ,PAddr paddr, MemRequest *mreq
					     ,bool wrHit)
{
  VMemWriteReq *req = rPool.out();
//  printf("createWriteCheck: req is %x\n", req);

  req->init(VWriteCheck, c, v, paddr);

  req->mreq    = mreq;
  req->origReq = 0; //right???? add by hr
  req->state.clearState();
  req->cacheSentData = false;
  req->memSentData   = false;
  req->writeHit = wrHit;
  mreq->setVMemReq(req);

  return req;
}

VMemWriteReq *VMemWriteReq::createWriteCheckAck(VMemObj *c, HVersion *v
						,VMemState *state
						,VMemWriteReq *vreq)
{
  VMemWriteReq *req = rPool.out();

  req->init(VWriteCheckAck, c, v, vreq->paddr);
  req->mreq     = 0;
  req->origReq = vreq;

  // this can be an empty message (when the cache does not have any version)
  if(state) {
    req->state.copyStateFrom(state);
  } else {
    req->state.clearState();
  }

  // if this message is being created through this interface, it will be sent to
  // the Vbus - the original request is needed there
  req->cacheSentData = false;
  req->memSentData   = false;

  // FIXME: join cacheSentData and memSentData (no need to both flags)
  req->lastMsg = false;

  return req;
}

void VMemWriteReq::destroy()
{
  I(nPendingMsg==0);
  if(ver)
    ver->garbageCollect();
  IS(ver=0);
  
  rPool.in(this);
}

/**********************************
 * VMemPushLineReq
 **********************************/

VMemPushLineReq *VMemPushLineReq::createPushLine(VMemObj *c
						 ,HVersion *v
						 ,PAddr paddr
						 ,const VMemState *state
						 ,bool noMoreLocalSharers
						 ,VMemPushLineReq *askPushReq
						 )
{
  VMemPushLineReq *req = rPool.out();
  
  req->init(VPushLine, c, v, paddr);

  req->noMoreSharers = noMoreLocalSharers;

  I(state);
  req->state.copyStateFrom(state);

  req->askPushReq = askPushReq;
  GI(askPushReq, askPushReq->type == VAskPushLine);

  return req;
}

VMemPushLineReq *VMemPushLineReq::createAskPushLine(VMemObj *c
						    ,HVersion *v
						    ,PAddr paddr
						    ,const VMemState *state)
{
  VMemPushLineReq *req = rPool.out();
  
  req->init(VAskPushLine, c, v, paddr);

  I(state);
  req->state.copyStateFrom(state);

  req->askPushReq = 0;

  return req;
}

void VMemPushLineReq::convert2Ack()
{
  GI(askPushReq, ver == 0);
  I(type == VPushLine);
  type = VPushLineAck;
}

void VMemPushLineReq::destroy()
{
  I(nPendingMsg==0);
  if (ver) {
    ver->garbageCollect();
    IS(ver=0);
  }
  rPool.in(this);
}

