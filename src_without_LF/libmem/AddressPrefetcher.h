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

#ifndef ADDRESSPREFETCHER_H
#define ADDRESSPREFETCHER_H

#include "Port.h"
#include "MemRequest.h"
#include "CacheCore.h"
#include "MemObj.h"

class AddressPrefetcher : public MemObj {
protected:
  class BState : public StateGeneric<> {
  };

  typedef CacheGeneric<BState,PAddr> CacheType;
  typedef CacheGeneric<BState,PAddr>::CacheLine Line;

  const int32_t bsize;
  GMemorySystem *gms;
  PortGeneric *cachePort;

  TimeDelta_t hitDelay;
  TimeDelta_t missDelay;

  CacheType *cache;

  void tryPrefetch(MemRequest *mreq);

public:
  AddressPrefetcher(MemorySystem* current, const char *device_descr_section,
      const char *device_name = NULL);
  ~AddressPrefetcher() {}
  void access(MemRequest *mreq);
  void returnAccess(MemRequest *mreq);
  bool canAcceptStore(PAddr addr);
  virtual void invalidate(PAddr addr,ushort size,MemObj *oc);
  Time_t getNextFreeCycle() const;
};

#endif // ADDRESSPREFETCHER_H
