/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Basilio Fraguela
                  Jose Renau

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
#ifndef MEMCTRL_H
#define MEMCTRL_H

#include "nanassert.h"

#include <queue>

#include "GStats.h"
#include "MemRequest.h"
#include "MemObj.h"
#include "Bank.h"
#include "MemorySystem.h"

// Memory controler. Models contention at the memory chanel and delay to the
// memory chips, loosely according to RAMBus

class MemCtrl: public MemObj {
protected:
  // TODO: Change busyUntil for a GenericPort
  Time_t busyUntil;

  uint16_t numBanks;
  uint16_t numBanksPerChip;
  uint16_t pagesPerBank;
  uint16_t numChips;

  TimeDelta_t rowAccessDelay;
  TimeDelta_t hitDelay;

  GStatsCntr chWait;

  int32_t calcBank(PAddr paddr) {
    return GMemorySystem::calcPage(paddr) % numBanks;
  }

public:
  MemCtrl(MemorySystem* current, const char *device_descr_section,
	  const char *device_name=NULL);
  ~MemCtrl() {};
  
  void returnAccess(MemRequest *mreq);

  virtual void read(MemRequest *mreq);
  virtual void write(MemRequest *mreq);
  virtual void specialOp(MemRequest *mreq);

  Bank *getBank(uint16_t i) const;

  Time_t getNextFreeCycle() const;

  void access(MemRequest *mreq);

  virtual void invalidate(PAddr addr,ushort size,MemObj *oc);
  bool canAcceptStore(PAddr addr);
};


#endif
