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

#include "Port.h"
#include "TaskHandler.h"
#include "TaskContext.h"
#include "VBus.h"
#include "VMemorySystem.h"

ushort VBus::delay=0;

VBus::VBus(MemorySystem *gms, const char *section, const char *name)
  : VMemObj(gms, section, name)
  ,nPushLineForward("%s:nPushLineForward", name)
  ,victimCache(gms, SescConf->getCharPtr(section, "victimCache", 0), "MVVCache", this)
#ifdef DIRECTORY
  ,lineDirectory(SescConf->getCharPtr(section, "directory", 0), "VBusDirectory")
#endif
{ 
  SescConf->isInt(section, "numPorts");
  SescConf->isInt(section, "portOccp");

  NumUnits_t  num = SescConf->getInt(section, "numPorts");
  TimeDelta_t occ = SescConf->getInt(section, "portOccp");
  
  char cadena[100];
  sprintf(cadena,"Data%s", name);
  port = PortGeneric::create(cadena, num, occ);
  I(port);

  busEnergy = new GStatsEnergy("busEnergy", cadena, 0, 
			       MemPower, 
			       EnergyMgr::get(section,"BusEnergy",0));

  // Static variables only
  SescConf->isInt(section, "delay");
  delay= SescConf->getInt(section, "delay");
}

VBus::~VBus()
{
}

bool VBus::isInUpperLevel(MemObj *obj) const
{
  I(obj);
  
  size_t nL1s = upperLevel.size();
  for(size_t i=0;i<nL1s;i++) {
    if (upperLevel[i] == obj)
      return true;
  }
  return false;
}

void VBus::returnAccess(MemRequest *mreq)
{
  if(mreq->getMemOperation() == MemRead) {
    VMemReadReq *vreq = static_cast<VMemReadReq *>(mreq->getVMemReq());

    I(vreq);
    I(vreq->getType() == VRead);
    I(!vreq->hasCacheSentData());

    VMemReadReq *remReadAck = VMemReadReq::createReadAck(vreq->getVMem(), vreq
							 ,vreq->getVersionDuplicate());
    // Do not incPendingMsg because already increased when nonVersionRead was called
    // vreq->incPendingMsg();
    vreq->setMemSentData();
    sendReadAck(remReadAck);
    return;
  }

#if TS_WRITE
  I((mreq->getMemOperation() == MemWrite)|| ((mreq->getMemOperation() == MemReadW)));
#endif
  
  VMemWriteReq *vreq = static_cast<VMemWriteReq *>(mreq->getVMemReq());
  I(vreq);

  I(vreq->getType() == VWriteCheck);
  I(!vreq->hasCacheSentData());

  sendWriteCheckAck(vreq);
}


void VBus::localRead(MemRequest *mreq)
{
  I(0); // Not allowed in VBUS
}

//it seem sthe interface between FMVCache and VBus . add by hr
void VBus::read(VMemReadReq *vreq)
{
  I(vreq->getType() == VRead);

  TimeDelta_t lat = vreq->getLatency();
  vreq->clearLatency();
  doReadCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq);
  busEnergy->inc();
}

void VBus::doRead(VMemReadReq *readReq)
{
  I(readReq->getType() == VRead);  
  VMemObj    *obj  = 0;  

  // ask data to all caches except requestor, including lower level
  size_t nL1s = upperLevel.size();

  I(readReq->hasPendingMsg());

#ifdef DIRECTORY
  PAddr pos = calcLine(readReq->getPAddr());
  uint32_t lineInfo = (uint32_t) -1;
  lineDirectory.getInfoForLine(pos, &lineInfo);

  for(size_t i=0;i<nL1s;i++) {
    obj = static_cast<VMemObj *>(upperLevel[i]);
    if(obj == readReq->getVMem())
      continue;

    if (lineDirectory.isPresent(obj->getId(), lineInfo)) { 
      readReq->incPendingMsg();
      sendRead(readReq, obj);
    }
  }
#else
  for(size_t i=0;i<nL1s;i++) {
    obj = static_cast<VMemObj *>(upperLevel[i]);
    if(obj == readReq->getVMem())
      continue;

    readReq->incPendingMsg();

    sendRead(readReq, obj);
  }
#endif /* DIRECTORY */

#ifdef DIRECTORY
  if (lineDirectory.isPresent(obj->getId(), lineInfo)) { 
    readReq->incPendingMsg();
    victimCache.read(readReq); 
  }
#else
  readReq->incPendingMsg();
  victimCache.read(readReq); 
#endif /* DIRECTORY */
  
  readReq->decPendingMsg(); // current msg
}

void VBus::readAck(VMemReadReq *readAckReq)
{
  I(readAckReq->getType() == VReadAck);
  I(isInUpperLevel(readAckReq->getVMem()));
  I((readAckReq->getOrigCache() != this) || readAckReq->getOrigRequest()->hasMemSentData());

#ifdef DIRECTORY
  PAddr pos = calcLine(readAckReq->getPAddr());
  uint32_t lineInfo = (uint32_t) -1;
  lineDirectory.getInfoForLine(pos, &lineInfo); 

  // the directory has to agree that the cache sending the data 
  // actually has the data 
  GI((readAckReq->getVersionRef() && (readAckReq->getOrigCache() != this)), 
     (lineDirectory.isPresent(readAckReq->getOrigCache()->getId(), lineInfo) == true));

  // vbus was source for data, forwarding it from memory or from
  // in-flight push line
  GI((readAckReq->getVersionRef() && (readAckReq->getOrigCache() == this)),
     readAckReq->getOrigRequest()->hasMemSentData() ||
     readAckReq->getOrigRequest()->hasCacheSentData());

#endif /* DIRECTORY */

  if (readAckReq->hasWait4PushLine()) {
    readAckReq->clearWait4PushLine();
    nPushLineForward.inc();

    VMemReadReq *readReq = readAckReq->getOrigRequest();
    readReq->incPendingMsg();
    victimCache.read(readReq);
    // I think a return is needed here! add by hr
  }

  sendReadAck(readAckReq);
}

void VBus::sendReadAck(VMemReadReq *vreq)
{
  TimeDelta_t lat = vreq->getLatency();
  vreq->clearLatency();

  doSendReadAckCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq);
  busEnergy->inc();
}

void VBus::sendRead(VMemReadReq *vreq, VMemObj *obj)
{
  obj->read(vreq);
}

void VBus::doSendReadAck(VMemReadReq *readAckReq)
{
  I(readAckReq->getType() == VReadAck);
  I(isInUpperLevel(readAckReq->getVMem()));

#ifdef DIRECTORY 
  uint32_t id = readAckReq->getVMem()->getId();
  PAddr pos = calcLine(readAckReq->getPAddr()); 

  // vbus was source for data, forwarding it from memory or from
  // in-flight push line
  GI((readAckReq->getVersionRef() && (readAckReq->getOrigCache() == this)),
     readAckReq->getOrigRequest()->hasMemSentData() || 
     readAckReq->getOrigRequest()->hasCacheSentData());

  if(readAckReq->getVersionRef())
    lineDirectory.setPresentIn(id, pos);

#endif /* DIRECTORY */

  VMemReadReq *readReq = readAckReq->getOrigRequest();
  I(readReq);
  readReq->decPendingMsg();

  if (!readReq->hasPendingMsg()) {
    if (!readReq->hasCacheSentData() && !readReq->hasMemSentData()) {
      // Need to bring data from memory

      MemRequest *mreq = readReq->getMemRequest();
      I(mreq);
      readReq->incPendingMsg();
      nonVersionRead(mreq);
    }
  }

  readReq->incPendingMsg();// why ????

  readAckReq->getVMem()->readAck(readAckReq); 
}

void VBus::localWrite(MemRequest *mreq)
{
  I(0); // Not allowed in VBUS
}

// the interface between FMVCache and VBus. add by hr
void VBus::writeCheck(VMemWriteReq *vreq)
{
  I(vreq->getType() == VWriteCheck);

  TimeDelta_t lat = vreq->getLatency();
  vreq->clearLatency();
  doWriteCheckCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq);

  busEnergy->inc();
}

void VBus::doWriteCheck(VMemWriteReq *vreq)
{
  I(vreq->getType() == VWriteCheck);

  // 1- vbus gets a write check init from an L1 
  // 2- vbus sends requests to all other caches 
  // then vbus will receive other data from requestor
  // cache and from other caches

  I(vreq);  

  // visit all caches
  // visit L1s
  // also vist viticmcache //add by hr
  size_t nL1s = upperLevel.size();
  
  for(size_t i=0;i<nL1s;i++) 
     vreq->incnRequests();
  
  for(size_t i=0;i<nL1s;i++) {
    VMemObj *obj = static_cast<VMemObj *>(upperLevel[i]);
    if(obj == vreq->getVMem())
      continue;

    //vreq->incnRequests();

    sendWriteCheck(vreq, obj);
  }

  //vreq->incnRequests();

  sendWriteCheck(vreq, &victimCache);
}

void VBus::writeCheckAck(VMemWriteReq *vreq)
{
  PAddr paddr = vreq->getPAddr();
  VMemWriteReq *oreq = vreq->getOrigRequest();

  I(vreq->getType() == VWriteCheckAck);
  I(oreq->getType() == VWriteCheck);
  I(oreq->getnRequests() > 0);
  I(vreq->getnRequests() == 0);

  oreq->decnRequests(); // FIXME: move
  vcr.addCheck(oreq, vreq);

  if (oreq->getnRequests() == 0) { // FIXME

    TimeDelta_t lat = vreq->getLatency();
    vreq->clearLatency();
    doWriteCheckAckCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq);
  }else{
    vreq->destroy();
  }

  busEnergy->inc();
}

void VBus::doWriteCheckAck(VMemWriteReq *vreq)
{
  VMemWriteReq *oreq = vreq->getOrigRequest();
  I(oreq);
  I(vreq->getType() == VWriteCheckAck);
  I(oreq->getType() == VWriteCheck);

  // MSG("2.vreq 0x%x oreq 0x%x", vreq, oreq);

  I(oreq->getnRequests() == 0);

  GI(!oreq->hasCacheSentData() && oreq->getnRequests() == 0
     ,!(victimCache.isCombining(vreq->getPAddr())));
 
  if (oreq->getnRequests() == 0) {
    // no more caches to ask for data, start sending invalidates if needed
    bool canSetMostSpec = vcr.performCheck(oreq);
    if (canSetMostSpec)
      oreq->getState()->setMostSpecLine();
    
    I(!oreq->hasMemSentData());

    // call upper level ack
    if(oreq->hasCacheSentData()) {  
      //the condition should be oreq->hasCacheSentData() || weitrhit. add by hr 
      sendWriteCheckAck(oreq);
    }else{
      I(!oreq->isWriteHit());
      nonVersionRead(oreq->getMemRequest());
    }
  }

  vreq->destroy();
}

void VBus::sendWriteCheckAck(VMemWriteReq *vreq)
{
  I(vreq->getType() == VWriteCheck);

  I(isInUpperLevel(vreq->getVMem()));

  TimeDelta_t lat = vreq->getLatency();
  vreq->clearLatency();
  doSendWriteCheckAckCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq);
  busEnergy->inc();
}

void VBus::doSendWriteCheckAck(VMemWriteReq *vreq)
{
  vreq->getVMem()->writeCheckAck(vreq);
}

void VBus::sendWriteCheck(VMemWriteReq *vreq, VMemObj *obj)
{
  obj->writeCheck(vreq);
}

void VBus::pushLine(VMemPushLineReq *vreq)
{
  // Just forward the packet to the victim cache

#ifdef DIRECTORY
  PAddr pos = calcLine(vreq->getPAddr());

  // no version of the line with this address is in cache that
  // originated the pushline
  if(vreq->isLastSharer() && vreq->getnForwards()>0)
    lineDirectory.resetPresentIn(vreq->getVMem()->getId(), pos);
#endif

  I(vreq->getVersionRef());
  I(vreq->getType() == VPushLine);
  TimeDelta_t lat = vreq->getLatency();
  vreq->clearLatency();

  doPushLineCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq);
  busEnergy->inc();
}

void VBus::doPushLine(VMemPushLineReq *vreq)
{
  I(lowerLevel.size()==1);
  I(vreq->getVersionRef());
  I(vreq->getType() == VPushLine);

#ifdef DIRECTORY
  PAddr pos = calcLine(vreq->getPAddr());
  lineDirectory.setPresentIn(victimCache.getId(), pos);
#endif /* DIRECTORY */

  victimCache.pushLine(vreq);
}

void VBus::pushLineAck(VMemPushLineReq *vreq)
{
  I(vreq->getType() == VPushLineAck);
  I(vreq->getVMem());

  TimeDelta_t lat = vreq->getLatency();
  vreq->clearLatency();

  doPushLineAckCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq);
  busEnergy->inc();
}

void VBus::doPushLineAck(VMemPushLineReq *vreq)
{
  I(vreq->getType() == VPushLineAck);
  
  vreq->getVMem()->pushLineAck(vreq);
}

void VBus::askPushLine(VMemPushLineReq *vreq)
{
  I(vreq->getType() == VAskPushLine);

  TimeDelta_t lat = vreq->getLatency();
  vreq->clearLatency();

  // Request can only be sent by the LMVCache
  I(&victimCache == vreq->getVMem());
  
  size_t nL1s = upperLevel.size();
#ifdef DIRECTORY
  PAddr pos = calcLine(vreq->getPAddr());
  uint32_t lineInfo = (uint32_t) -1;
  lineDirectory.getInfoForLine(pos, &lineInfo);

  for(size_t i=0;i<nL1s;i++) {
    // Send request to all L1 caches
    VMemObj *obj = static_cast<VMemObj *>(upperLevel[i]);
    
    if (lineDirectory.isPresent(obj->getId(), lineInfo)) { 
      vreq->incPendingMsg();
      doSendAskPushLineCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq, obj);
    } 
  }
#else
  for(size_t i=0;i<nL1s;i++) {
    // Send request to all L1 caches
    VMemObj *obj = static_cast<VMemObj *>(upperLevel[i]);

    vreq->incPendingMsg();

    I(vreq->getVersion());

    doSendAskPushLineCB::scheduleAbs(port->nextSlot()+delay+lat, this, vreq, obj);
  }
#endif /* DIRECTORY */

  vreq->decPendingMsg();

  busEnergy->inc();
}

void VBus::doSendAskPushLine(VMemPushLineReq *vreq, VMemObj *obj)
{
  I(vreq->getVersion());
		   
  obj->askPushLine(vreq);
}

Time_t VBus::getNextFreeCycle() const
{
  return port->calcNextSlot();
}

void VBus::invalidate(PAddr addr, ushort size, MemObj *oc)
{
  return;
}

bool VBus::canAcceptStore(PAddr addr)
{
  return true;
}

void VBus::nonVersionRead(MemRequest *mreq)
{
  I(!(victimCache.isCombining(mreq->getPAddr())));
  I(mreq->isTopLevel());
  
  mreq->mutateWriteToRead();
  mreq->setCurrentMemObj(this);
  mreq->goDown(0, lowerLevel[0]);
}


