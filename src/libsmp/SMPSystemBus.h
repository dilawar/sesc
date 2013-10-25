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

#ifndef SMPSYSTEMBUS_H
#define SMPSYSTEMBUS_H

#include "SMemorySystem.h"
#include "MemObj.h"
#include "Port.h"
#include "estl.h"

class SMPSystemBus : public MemObj {
private:

protected:
  PortGeneric *busPort;
  PortGeneric *memNetPort;

  TimeDelta_t delay;

#ifdef SESC_ENERGY
  GStatsEnergy *busEnergy;
#endif

  typedef HASH_MAP<MemRequest *, int32_t, SMPMemReqHashFunc> PendReqsTable;

  PendReqsTable pendReqsTable;

  // interface with upper level
  void read(MemRequest *mreq);
  void write(MemRequest *mreq);
  void push(MemRequest *mreq);
  void specialOp(MemRequest *mreq);

  Time_t nextSlot(MemRequest *mreq);

  virtual void doRead(MemRequest *mreq);
  virtual void doWrite(MemRequest *mreq);
  virtual void doPush(MemRequest *mreq);

  typedef CallbackMember1<SMPSystemBus, MemRequest *, &SMPSystemBus::doRead> 
    doReadCB;
  typedef CallbackMember1<SMPSystemBus, MemRequest *, &SMPSystemBus::doWrite> 
    doWriteCB;
  typedef CallbackMember1<SMPSystemBus, MemRequest *, &SMPSystemBus::doPush> 
    doPushCB;

  virtual void finalizeRead(MemRequest *mreq);
  virtual void finalizeWrite(MemRequest *mreq);
  void finalizeAccess(MemRequest *mreq);
  virtual void goToMem(MemRequest *mreq);
  virtual unsigned getNumSnoopCaches(SMPMemRequest *sreq) {
    return upperLevel.size() - 1;
  }

public:
  SMPSystemBus(SMemorySystem *gms, const char *section, const char *name);
  ~SMPSystemBus();

  // BEGIN MemObj interface
  
  // port usage accounting
  Time_t getNextFreeCycle() const;

  // interface with upper level
  bool canAcceptStore(PAddr addr) const;
  void access(MemRequest *mreq);
  
  // interface with lower level
  virtual void returnAccess(MemRequest *mreq);

  void invalidate(PAddr addr, ushort size, MemObj *oc);
  void doInvalidate(PAddr addr, ushort size);

  bool canAcceptStore(PAddr addr) { return true; }

  // END MemObj interface

};

#endif // SMPSYSTEMBUS_H
