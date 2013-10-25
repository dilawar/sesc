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

#ifndef PRIORITY_BUS_H
#define PRIORITY_BUS_H

#include "GStats.h"
#include "Port.h"
#include "MemRequest.h"
#include "MemorySystem.h"
#include "MemObj.h"

class PriorityBus: public MemObj {
 protected:
  const bool enablePrio;
  TimeDelta_t delay;
  TimeDelta_t dataOcc;
  TimeDelta_t controlOcc;

  PortNPipe *busPort;

  typedef std::deque<MemRequest *> RequestQueue;
  typedef std::deque<Time_t> TimestampQueue;

  enum QueueId {
    upHighP = 0,  // reqs going up
    downHighP,
    upLowP,   // reqs going down
    downLowP,
    LastQ // getting the number of Qs.
  };

  // queues of reqs coming from the higher levels 
  RequestQueue reqQs[LastQ];
  TimestampQueue timeQs[LastQ];

  GStatsAvg  *avgTime[LastQ];
  GStatsTimingHist *tHist[LastQ];

  GStatsCntr nBypassDown;
  GStatsCntr nBypassUp;
  GStatsCntr nBypassDirection;

  void processQ();
  StaticCallbackMember0<PriorityBus, &PriorityBus::processQ> processQCB;

  bool allQueuesEmpty() {
    bool ret = true;;
    
    for(int32_t q = 0; q < LastQ; q++)
      ret &= reqQs[q].empty();

    return ret;
  }

  void pushBack(int32_t q, MemRequest *mreq) {
    I(q >= 0 && q < LastQ);
    reqQs[q].push_back(mreq);
    timeQs[q].push_back(globalClock);
    tHist[q]->sample(reqQs[q].size());
  }
  

 public:

  PriorityBus(MemorySystem* current, const char *device_descr_section,
	      const char *device_name = NULL);
  ~PriorityBus() {}

  void access(MemRequest *mreq);
  void returnAccess(MemRequest *mreq);

  bool canAcceptStore(PAddr addr);
  void invalidate(PAddr addr,ushort size,MemObj *oc);
  Time_t getNextFreeCycle() const;
};
#endif
