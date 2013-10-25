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

#include "SMPProtocol.h"
#include "SMPCache.h"

SMPProtocol::SMPProtocol(SMPCache *cache)
  : pCache(cache)
{
}

SMPProtocol::~SMPProtocol() 
{
  // nothing to do
}

void SMPProtocol::changeState(Line *l, unsigned)
{
  I(0);
}

void SMPProtocol::makeDirty(Line *l)
{
  I(0);
}

void SMPProtocol::preInvalidate(Line *l)
{
  I(0);
}

void SMPProtocol::read(MemRequest *mreq)
{
  I(0);
}

void SMPProtocol::write(MemRequest *mreq)
{
  I(0);
}

void SMPProtocol::writeBack(MemRequest *mreq)
{
  I(0);
}

void SMPProtocol::returnAccess(MemRequest *mreq)
{
  I(0);
}

void SMPProtocol::sendReadMiss(MemRequest *mreq)
{
  I(0);
}

void SMPProtocol::sendWriteMiss(MemRequest *mreq)
{
  I(0);
}

void SMPProtocol::sendInvalidate(MemRequest *mreq)
{
  I(0);
}

void SMPProtocol::sendReadMissAck(SMPMemRequest *sreq)
{
  I(0);
}

void SMPProtocol::sendWriteMissAck(SMPMemRequest *sreq)
{
  I(0);
}

void SMPProtocol::sendInvalidateAck(SMPMemRequest *sreq)
{
  I(0);
}

void SMPProtocol::sendWriteBack(PAddr addr, CallbackBase *cb) 
{
  SMPMemRequest *sreq = SMPMemRequest::create(pCache, addr, MemPush, true, cb);
  pCache->sendBelow(sreq);
}

void SMPProtocol::writeBackAckHandler(SMPMemRequest *sreq)
{
  sreq->ack(0);
}

void SMPProtocol::sendDisplaceNotify(PAddr addr, CallbackBase *cb)
{
  SMPMemRequest *sreq = SMPMemRequest::create(pCache, addr, MemPush, false, cb);
  pCache->sendBelow(sreq);
}

void SMPProtocol::displaceNotifyAckHandler(SMPMemRequest *sreq)
{
  sreq->ack(0);
}

void SMPProtocol::sendData(SMPMemRequest *sreq)
{
  I(0);
}

