/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze

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

#include "PriorityBus.h"

PriorityBus::PriorityBus(MemorySystem* current, const char *section,
			 const char *name) 
  : MemObj(section, name)
  ,enablePrio(SescConf->getBool(section, "enablePrio"))
  ,delay(SescConf->getInt(section, "delay"))
  ,nBypassDown("PBusPort%s_nBypassDown", name)
  ,nBypassUp("PBusPort%s_nBypassUp", name)
  ,nBypassDirection("PBusPort%s_nBypassDirection", name)
  ,processQCB(this)
{
  MemObj *lower_level = NULL;

  NumUnits_t  num = SescConf->getInt(section, "numPorts");
  dataOcc = SescConf->getInt(section, "dataOccp");
  controlOcc = SescConf->getInt(section, "controlOccp");

  char portName[256];
  sprintf(portName, "PBusPort%s", name);

  busPort = new PortNPipe(portName, num, dataOcc);
  
  avgTime[upHighP] = new GStatsAvg("PBus%s_upHighAvgTime", name);
  avgTime[downHighP] = new GStatsAvg("PBus%s_downHighAvgTime", name);
  avgTime[upLowP] = new GStatsAvg("PBus%s_upLowAvgTime", name);
  avgTime[downLowP] = new GStatsAvg("PBus%s_downLowAvgTime", name);

  tHist[upHighP] = new GStatsTimingHist("PBus%s_upHighOcc", name);
  tHist[downHighP] = new GStatsTimingHist("PBus%s_downHighOcc", name);
  tHist[upLowP] = new GStatsTimingHist("PBus%s_upLowOcc", name);
  tHist[downLowP] = new GStatsTimingHist("PBus%s_downLowOcc", name);

  for(int32_t q = 0; q < LastQ; q++) {
    reqQs[q].clear();
    timeQs[q].clear();
  }

  I(current);
  lower_level = current->declareMemoryObj(section, k_lowerLevel);   
  if (lower_level != NULL)
    addLowerLevel(lower_level);
}

void PriorityBus::access(MemRequest *mreq)
{
  // making writes low priority
  if(mreq->getMemOperation() != MemRead || mreq->isPrefetch()) 
    mreq->setPriority(1);
  
  bool schedCallback = allQueuesEmpty();
  int32_t q = -1;

  if(enablePrio && mreq->getPriority() != 0) 
    q = downLowP;
  else
    q = downHighP;

  I(q != -1);

  pushBack(q, mreq);
  
  if(schedCallback)
    processQCB.scheduleAbs(busPort->calcNextSlot());
}

void PriorityBus::returnAccess(MemRequest *mreq)
{

  bool schedCallback = allQueuesEmpty();
  int32_t q = -1;

  if(enablePrio && mreq->getPriority() != 0)
    q = upLowP;
  else
    q = upHighP;

  I(q != -1);

  pushBack(q, mreq);

  if(schedCallback)
    processQCB.scheduleAbs(busPort->calcNextSlot());
}

void PriorityBus::processQ()
{
  // if this was called, there *must* be a req waiting
  I(!allQueuesEmpty());

  int32_t selectedQ = -1; 
  for(int32_t q = 0; q < LastQ; q++) {
    if(!reqQs[q].empty()) {
      selectedQ = q;
      break;
    }
  }

  I(selectedQ != -1);

  MemRequest *mreq = reqQs[selectedQ].front();
  reqQs[selectedQ].pop_front();

  Time_t reqArrivalTime = timeQs[selectedQ].front();
  timeQs[selectedQ].pop_front();

  //determining when to dispatch the request
  bool goUp = ((selectedQ == upHighP) || (selectedQ == upLowP));
  Time_t when = 0;
  if(goUp) {
    if (mreq->getMemOperation() == MemPush || mreq->getMemOperation() == MemWrite) {
      when = busPort->nextSlot(controlOcc);
    }else{
      when = busPort->nextSlot(dataOcc)+delay;
    }
  } else { // req is going down
    if (mreq->getMemOperation() == MemPush || mreq->getMemOperation() == MemWrite) {
      when = busPort->nextSlot(dataOcc)+delay;
    }else{
      when = busPort->nextSlot(controlOcc);
    }
  }

  tHist[selectedQ]->sample(reqQs[selectedQ].size());
  avgTime[selectedQ]->sample(when - reqArrivalTime);

  //counting the different types of bypass
  if(selectedQ == upHighP && !timeQs[upLowP].empty()) {
    if(reqArrivalTime > timeQs[upLowP].front())
      nBypassUp.inc();
  }
  if(selectedQ == downHighP && !timeQs[downLowP].empty()) {
    if(reqArrivalTime > timeQs[downLowP].front())
      nBypassDown.inc();
  }
  if(selectedQ == upHighP && !timeQs[downLowP].empty()) {
    // a req with high prio going up bypassed a low prio req going down
    if(reqArrivalTime > timeQs[downLowP].front())
      nBypassDirection.inc();
  }

  // scheduling next callback *has* to be before sending reqs up or down.
  if(!allQueuesEmpty())
    processQCB.scheduleAbs(busPort->calcNextSlot());

  // dispatching the request
  if(goUp)
    mreq->goUpAbs(when);
  else
    mreq->goDownAbs(when, lowerLevel[0]);
}
 
bool PriorityBus::canAcceptStore(PAddr addr)
{
  return true;
}

void PriorityBus::invalidate(PAddr addr,ushort size,MemObj *oc)
{
  invUpperLevel(addr,size,oc); 
}

Time_t PriorityBus::getNextFreeCycle() const
{
  return busPort->calcNextSlot();
}
