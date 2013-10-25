/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by Jose Renau

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

#include "SescConf.h"
#include "MemorySystem.h"
#include "AlwaysPrefetch.h"
#include "ThreadContext.h"

static pool < std::queue<MemRequest *> > activeMemReqPool(32,"AlwaysPrefetcher");

AlwaysPrefetch::AlwaysPrefetch(MemorySystem* current
				     ,const char *section
				     ,const char *name)
  : MemObj(section, name)
  ,gms(current)
  ,halfMiss("%s:halfMiss", name)
  ,miss("%s:miss", name)
  ,hit("%s:hits", name)
  ,predictions("%s:predictions", name)
  ,accesses("%s:accesses", name)

{
  MemObj *lower_level = NULL;
 
  const char *Section = SescConf->getCharPtr(section, "nextLevel");
  if (Section) {
    lineSize = SescConf->getInt(Section, "bsize");
  }

  I(current);

  lower_level = current->declareMemoryObj(section, k_lowerLevel);   

  const char *buffSection = SescConf->getCharPtr(section, "buffCache");
  if (buffSection) {
    buff = BuffType::create(buffSection, "", name);
    
    SescConf->isInt(buffSection, "numPorts");
    numBuffPorts = SescConf->getInt(buffSection, "numPorts");
    
    SescConf->isInt(buffSection, "portOccp");
    buffPortOccp = SescConf->getInt(buffSection, "portOccp");
  }

  defaultMask  = ~(buff->getLineSize()-1);

  char portName[128];
  sprintf(portName, "%s_buff", name);
  buffPort  = PortGeneric::create(portName, numBuffPorts, buffPortOccp);

  sprintf(portName, "%s_table", name);
  tablePort = PortGeneric::create(portName, numTablePorts, tablePortOccp);

  if (lower_level != NULL)
    addLowerLevel(lower_level);
}

void AlwaysPrefetch::access(MemRequest *mreq)
{

  // TODO: should i really consider all these read types? 
  if (mreq->getMemOperation() == MemRead
      || mreq->getMemOperation() == MemReadW) {
    read(mreq);
  } else {
    nextBuffSlot();
    mreq->goDown(0, lowerLevel[0]);
  }
  accesses.inc();
}

void AlwaysPrefetch::read(MemRequest *mreq)
{
  uint32_t paddr = mreq->getPAddr() & defaultMask;
  bLine *l = buff->readLine(paddr);

  if(l) { //hit
    LOG("NLAP: hit on [%08lx]", paddr);
    hit.inc();    
    mreq->goUpAbs(nextBuffSlot());
    return;
  }

  penFetchSet::iterator it = pendingFetches.find(paddr);
  if(it != pendingFetches.end()) { // half-miss
    LOG("NLAP: half-miss on %08lx", paddr);
    penReqMapper::iterator itR = pendingRequests.find(paddr);
    halfMiss.inc();
    if (itR == pendingRequests.end()) {
      pendingRequests[paddr] = activeMemReqPool.out();
      itR = pendingRequests.find(paddr);
    }

    I(itR != pendingRequests.end());
    
    (*itR).second->push(mreq);
    //prefetch(paddr+lineSize, 0); 
    //prefetch( paddr + buff->getLineSize(), 0 ); 
    return;
  }

  LOG("NLAP: miss on [%08lx]", paddr);
  miss.inc();

  Time_t lat = nextTableSlot() - globalClock;    

  prefetch(paddr+(buff->getLineSize()), lat);
  lat = nextTableSlot() - globalClock;    
  prefetch(paddr+(2*buff->getLineSize()), lat); 
  mreq->goDown(0, lowerLevel[0]);
}

void AlwaysPrefetch::prefetch(PAddr prefAddr, Time_t lat)
{
  uint32_t paddr = prefAddr & defaultMask;
    if(!buff->readLine(paddr)) { // it is not in the buff
      penFetchSet::iterator it = pendingFetches.find(prefAddr);
      if(it == pendingFetches.end()) {
	CBMemRequest *r;
	
	r = CBMemRequest::create(lat, lowerLevel[0], MemRead, prefAddr, 
				 processAckCB::create(this, prefAddr));
	r->markPrefetch();
	LOG("NLAP: prefetch [0x%08lx]", prefAddr);
	predictions.inc();
	pendingFetches.insert(prefAddr);
      }
  }
}

void AlwaysPrefetch::returnAccess(MemRequest *mreq)
{
  uint32_t paddr = mreq->getPAddr();
  LOG("NLAP: returnAccess [%08lx]", paddr);
  mreq->goUp(0);
}

bool AlwaysPrefetch::canAcceptStore(PAddr addr)
{
  return true;
}

void AlwaysPrefetch::invalidate(PAddr addr,ushort size,MemObj *oc)
{ 
}

Time_t AlwaysPrefetch::getNextFreeCycle() const
{ 
  return cachePort->calcNextSlot();
}

void AlwaysPrefetch::processAck(PAddr addr)
{
  uint32_t paddr = addr & defaultMask;

  penFetchSet::iterator itF = pendingFetches.find(paddr);
  if(itF == pendingFetches.end()) 
    return;

  bLine *l = buff->fillLine(paddr);

  penReqMapper::iterator it = pendingRequests.find(paddr);

  if(it != pendingRequests.end()) {
    LOG("NLAP: returnAccess [%08lx]", paddr);
    std::queue<MemRequest *> *tmpReqQueue;
    tmpReqQueue = (*it).second;
    while (tmpReqQueue->size()) {
      tmpReqQueue->front()->goUpAbs(nextBuffSlot());
      tmpReqQueue->pop();
    }
    pendingRequests.erase(paddr);
    activeMemReqPool.in(tmpReqQueue);
  }
  pendingFetches.erase(paddr);
}
