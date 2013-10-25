/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss

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

#ifndef VCACHE_H
#define VCACHE_H

#include "nanassert.h"

#include "MemRequest.h"
#include "GStats.h"
#include "GEnergy.h"
#include "EnergyMgr.h"

#include "VMemObj.h"
#include "LVIDTable.h"
#include "MSHR.h"
#include "Port.h"

// Generic functionality shared by all types of Version Caches

class VCache : public VMemObj {
private:
protected:
  PortGeneric *cachePort;

  LVIDTable lvidTable;

  MSHR<PAddr,VCache> *mshr;

  TimeDelta_t hitDelay;
  TimeDelta_t missDelay;

  // BEGIN Statistics
  GStatsEnergy lvidTableRdEnergy;

  GStatsCntr rdHit;      // Line Hit, Word Miss
  GStatsCntr rdMiss;     // Line Hit, Word Hit
  GStatsCntr rdHalfMiss; // Line Miss, pending request same line
  GStatsCntr rdHalfHit;  // Partial line hit, satisfied request with
			 // previous version

  GStatsCntr wrHit;       // Line Hit
  GStatsCntr wrMiss;      // Line Miss
  GStatsCntr wrHalfMiss;  // Line Miss, pending request same line
  GStatsCntr wrHalfHit;   // Partial line hit, satisfied request with
			  // previous version

  GStatsCntr pushHit;      // PushLine Hit (allocated)
  GStatsCntr pushMiss;     // PushLine Miss (not allocated due to
			   // space)
  GStatsCntr pushHalfMiss; // PushLine allocated, but displaced
			   // another instead (would get a restart
			   // very likely)

  // Reverse LVID 
  GStatsEnergy *rdRevLVIDEnergy;
  GStatsEnergy *wrRevLVIDEnergy;

  // LVID 
  GStatsEnergy *rdLVIDEnergy;   
  GStatsEnergy *wrLVIDEnergy;   

  // Cache Access
  GStatsEnergy *rdHitEnergy;    
  GStatsEnergy *rdMissEnergy;   
  GStatsEnergy *wrHitEnergy;    
  GStatsEnergy *wrMissEnergy;   
  // END Statistics

  void hitInc(MemOperation mop) {
    if (mop == MemRead) {
      rdHitEnergy->inc();
      rdHit.inc();
      return;
    }
    I(mop == MemWrite);
    wrHitEnergy->inc();
    wrHit.inc();
  }

  void halfHitInc(MemOperation mop) {
    if (mop == MemRead) {
      rdMissEnergy->inc();
      rdHalfHit.inc();
      return;
    }
    I(mop == MemWrite);
    wrMissEnergy->inc();
    wrHalfHit.inc();
  }

  void halfMissInc(MemOperation mop) {
    if (mop == MemRead) {
      rdMissEnergy->inc();
      rdHalfMiss.inc();
      return;
    }
    I(mop == MemWrite);
    wrMissEnergy->inc();
    wrHalfMiss.inc();
  }

  void missInc(MemOperation mop) {
    if (mop == MemRead) {
      rdMissEnergy->inc();
      rdMiss.inc();
      return;
    }
    I(mop == MemWrite);
    wrMissEnergy->inc();
    wrMiss.inc();
  }

public:
  VCache(MemorySystem *gms, const char *section, const char *name);
  virtual ~VCache();

  Time_t getNextFreeCycle() const;

  void invalidate(PAddr addr, ushort size, MemObj *oc);
  bool canAcceptStore(PAddr addr);

  LVIDTable *getLVIDTable() { return &lvidTable; }
};

#endif // VCACHE_H
