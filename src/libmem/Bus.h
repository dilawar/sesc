/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela

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

#ifndef BUS_H
#define BUS_H

#include "GStats.h"
#include "Port.h"
#include "MemRequest.h"
#include "MemObj.h"

class Bus: public MemObj {
protected:
  TimeDelta_t delay;
  PortGeneric *dataPort;
  PortGeneric *cmdPort;

  GStatsAvg  *opAvgBusTime[MemLastOp];

public:
  Bus(MemorySystem* current, const char *device_descr_section,
      const char *device_name = NULL);
  ~Bus() {}
  void access(MemRequest *mreq);
  void returnAccess(MemRequest *mreq);
  bool canAcceptStore(PAddr addr);
  void invalidate(PAddr addr,ushort size,MemObj *oc);
  Time_t getNextFreeCycle() const;
};

#endif
