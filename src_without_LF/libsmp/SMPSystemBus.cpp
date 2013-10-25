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

#include "SMPSystemBus.h"
#include "SMemorySystem.h"
#include "SMPCache.h"
#include "SMPDebug.h"

SMPSystemBus::SMPSystemBus(SMemorySystem *dms, const char *section, const char *name)
  : MemObj(section, name)
{
  MemObj *ll = NULL;

  I(dms);
  ll = dms->declareMemoryObj(section, "lowerLevel");

  if (ll != NULL)
    addLowerLevel(ll);

  SescConf->isInt(section, "numPorts");
  SescConf->isInt(section, "portOccp");
  SescConf->isInt(section, "delay");
  
  delay = SescConf->getInt(section, "delay");

  char portName[100];
  sprintf(portName, "%s_bus", name);

  busPort = PortGeneric::create(portName, 
                                SescConf->getInt(section, "numPorts"), 
                                SescConf->getInt(section, "portOccp"));

#ifdef SESC_ENERGY
  busEnergy = new GStatsEnergy("busEnergy", "SMPSystemBus", 0,
                               MemPower,
                               EnergyMgr::get(section,"BusEnergy",0));
#endif
}

SMPSystemBus::~SMPSystemBus() 
{
  // do nothing
}

Time_t SMPSystemBus::getNextFreeCycle() const
{
  return busPort->nextSlot();
}

Time_t SMPSystemBus::nextSlot(MemRequest *mreq)
{
  return getNextFreeCycle();
}

bool SMPSystemBus::canAcceptStore(PAddr addr) const
{
  return true;
}

void SMPSystemBus::access(MemRequest *mreq)
{
  GMSG(mreq->getPAddr() < 1024,
       "mreq dinst=0x%p paddr=0x%x vaddr=0x%x memOp=%d",
       mreq->getDInst(),
       (uint32_t) mreq->getPAddr(),
       (uint32_t) mreq->getVaddr(),
       mreq->getMemOperation());
  
  I(mreq->getPAddr() > 1024); 

#ifdef SESC_ENERGY
  busEnergy->inc();
#endif

  switch(mreq->getMemOperation()){
  case MemRead:     read(mreq);      break;
  case MemReadW:    
  case MemWrite:    write(mreq);     break;
  case MemPush:     push(mreq);      break;
  default:          specialOp(mreq); break;
  }

  // for reqs coming from upper level:
  // MemRead means I need to read the data, but I don't have it
  // MemReadW means I need to write the data, but I don't have it
  // MemWrite means I need to write the data, but I don't have permission
  // MemPush means I don't have space to keep the data, send it to memory
}

void SMPSystemBus::read(MemRequest *mreq)
{
  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {
    doReadCB::scheduleAbs(nextSlot(mreq)+delay, this, mreq);
  } else {
    doRead(mreq);
  }
}

void SMPSystemBus::write(MemRequest *mreq)
{
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);

  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {
    doWriteCB::scheduleAbs(nextSlot(mreq)+delay, this, mreq);
  } else {
    doWrite(mreq);
  }
}

void SMPSystemBus::push(MemRequest *mreq)
{
  doPushCB::scheduleAbs(nextSlot(mreq)+delay, this, mreq);  
}

void SMPSystemBus::specialOp(MemRequest *mreq)
{
  I(0);
}

void SMPSystemBus::doRead(MemRequest *mreq)
{
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);

  // no need to snoop, go straight to memory
  if(!sreq->needsSnoop()) {
    goToMem(mreq);
    return;
  }

  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {

    unsigned numSnoops = getNumSnoopCaches(sreq);

    // operation is starting now, add it to the pending requests buffer
    pendReqsTable[mreq] = getNumSnoopCaches(sreq);

    if(!numSnoops) { 
      // nothing to snoop on this chip
      finalizeRead(mreq);
      return;
      // TODO: even if there is only one processor on each chip, 
      // request is doing two rounds: snoop and memory
    }

    // distribute requests to other caches, wait for responses
         for(uint32_t i = 0; i < upperLevel.size(); i++) {
      if(upperLevel[i] != static_cast<SMPMemRequest *>(mreq)->getRequestor()) {
        upperLevel[i]->returnAccess(mreq);
      }
    }
  } 
  else {
    // operation has already been sent to other caches, receive responses

    I(pendReqsTable[mreq] > 0);
    I(pendReqsTable[mreq] <= (int) upperLevel.size());

    pendReqsTable[mreq]--;
    if(pendReqsTable[mreq] != 0) {
      // this is an intermediate response, request is not serviced yet
      return;
    }

    // this is the final response, request can go up now
    finalizeRead(mreq);
  }
}

void SMPSystemBus::finalizeRead(MemRequest *mreq)
{
  finalizeAccess(mreq);
}

void SMPSystemBus::doWrite(MemRequest *mreq)
{
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);

  // no need to snoop, go straight to memory
  if(!sreq->needsSnoop()) {
    goToMem(mreq);
    return;
  }

  if(pendReqsTable.find(mreq) == pendReqsTable.end()) {

    unsigned numSnoops = getNumSnoopCaches(sreq);

    // operation is starting now, add it to the pending requests buffer
    pendReqsTable[mreq] = getNumSnoopCaches(sreq);

    if(!numSnoops) { 
      // nothing to snoop on this chip
      finalizeWrite(mreq);
      return;
      // TODO: even if there is only one processor on each chip, 
      // request is doing two rounds: snoop and memory
    }

    // distribute requests to other caches, wait for responses
         for(uint32_t i = 0; i < upperLevel.size(); i++) {
      if(upperLevel[i] != static_cast<SMPMemRequest *>(mreq)->getRequestor()) {
        upperLevel[i]->returnAccess(mreq);
      }
    }
  } 
  else {
    // operation has already been sent to other caches, receive responses

    I(pendReqsTable[mreq] > 0);
    I(pendReqsTable[mreq] <= (int) upperLevel.size());

    pendReqsTable[mreq]--;
    if(pendReqsTable[mreq] != 0) {
      // this is an intermediate response, request is not serviced yet
      return;
    }

    // this is the final response, request can go up now
    finalizeWrite(mreq);
  }
}

void SMPSystemBus::finalizeWrite(MemRequest *mreq)
{
  finalizeAccess(mreq);
}

void SMPSystemBus::finalizeAccess(MemRequest *mreq)
{
  PAddr addr  = mreq->getPAddr();
  SMPMemRequest *sreq = static_cast<SMPMemRequest *>(mreq);
  
  pendReqsTable.erase(mreq);
 
  // request completed, respond to requestor 
  // (may have to come back later to go to memory)
  sreq->goUpAbs(nextSlot(mreq)+delay);  
}

void SMPSystemBus::goToMem(MemRequest *mreq)
{
  mreq->goDown(delay, lowerLevel[0]);
}

void SMPSystemBus::doPush(MemRequest *mreq)
{
  mreq->goDown(delay, lowerLevel[0]);
}

void SMPSystemBus::invalidate(PAddr addr, ushort size, MemObj *oc)
{
  invUpperLevel(addr, size, oc);
}

void SMPSystemBus::doInvalidate(PAddr addr, ushort size)
{
  I(0);
}

void SMPSystemBus::returnAccess(MemRequest *mreq)
{
  mreq->goUpAbs(nextSlot(mreq)+delay);
}
