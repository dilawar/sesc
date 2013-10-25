/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
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

#ifndef LVIDTABLE_H
#define LVIDTABLE_H

#include <vector>
#include <queue>

#include "estl.h"
#include "GLVID.h"
#include "HVersion.h"
#include "nanassert.h"
#include "Snippets.h"
#include "ThreadContext.h"

class LVIDTable;
class VMemReq;

typedef long long LPAddr;

class LVID : public GLVID {
private:
protected:
  // If the task EVER got migrated it should check in all the caches that may
  // have a SubLVID for ways to clean it
  static SubLVIDType nSubLVID;

  // Cache where the LVIDTable is associated. There is a relation of 1 to 1
  // between cache and LVIDTable
  LVIDTable *lvidc;
  bool lvidcRecycled;
  const ushort Id;
  const LPAddr shiftId;
  const PAddr  randSetId;

  HVersion *clver; // clver is valid even if the task got killed. The
                   // Version will be recycled when garbageCollect is
                   // called

  // killed   : clver->isKilled()
  // finished : clver->isFinished()
  // safe     : clver->isSafe()

  // # Lines that got pinned (includes also invalid lines for restart or kill)
  long nLinesPinned;

  SubLVIDType subLVID; // iterations from 0..subLVID are invalid

public:
  LVID(const char *section, LVIDTable *m, ushort i);
  virtual ~LVID() {
    // Nothing
  }

  long getnLinesPinned() const { return nLinesPinned; }

  void init(HVersion *v);

  GLVID  *getLVID()   { return this; }
  SubLVIDType getSubLVID() const { return subLVID; }

  virtual const HVersion *getVersionRef() const;
  virtual HVersion *getVersionDuplicate() const;

  bool isRecycled() const { return lvidcRecycled; }
  bool canRecycleLVID() const {
    I(!lvidcRecycled);
    // An entry can be recycled when there are no pinned lines and the task is
    // either finished or killed
    if (nLinesPinned > 0 || clver->hasOutsReqs())
      return false;

    // A LVID only can be recycled when there are not outstanding requests, and
    // the task has finished (fetch marks the finished, not retirement
    return clver->isFinished() || clver->isKilled();
  }
  virtual void decLinesUsed();
  virtual void incLinesUsed();

  // CL is safe when the CL would be commited to memory sometime (there may be
  // safer lines so it may not be possible to commit now)
  virtual bool isSafe() const { return clver->isSafe(); }
  virtual bool isKilled() const { return clver->isKilled(); }
  virtual bool isFinished() const {
    // TODO? Do we really need to check that it is not killed?
    return clver->isFinished() && !clver->isKilled(); 
  }

  LPAddr calcLPAddr(PAddr paddr) const {
#ifdef VMEM_LVID_IN_ADDR
    I(0); // TODO2: there should be a way to find all the version for an address
    return static_cast<LPAddr>((paddr^randSetId)) | shiftId;
#else
    return static_cast<LPAddr>(paddr) | shiftId;
#endif
  }

  PAddr calcPAddr(LPAddr addr) const {
#ifdef VMEM_LVID_IN_ADDR
    I(0); // TODO2: there should be a way to find all the version for an address
#else
    return static_cast<PAddr>(addr);
#endif
  }

  LVIDTable *getLVIDTable() { return lvidc; }

  virtual void garbageCollect();
  virtual void garbageCollectVersion();

  virtual void localRestart();

  bool isGarbageCollected() const { return clver==0; }

  virtual bool isSafestEntry() const;
};

// Fake LVID Entry for the safest version. Caches can point to it, and no version
// gets recycled.
class LVIDSafest : public LVID {
private:
protected:
public:
  LVIDSafest(const char *section, LVIDTable *m, ushort i);
  virtual ~LVIDSafest() {
    // Nothing
  }
 
  const HVersion *getVersionRef() const;
  HVersion *getVersionDuplicate() const;

  void decLinesUsed();
  void incLinesUsed();

  bool isSafe() const;
  bool isKilled() const;
  bool isFinished() const;
  
  void garbageCollect();
  void garbageCollectVersion();

  void localRestart();

  bool isSafestEntry() const;
};

// Class holding all the LVIDs for a cache. A single task can
// simultaneously use multiple LVIDs
class LVIDTable {
private:
  // used to spread kill and restarts to the caches
  typedef std::vector<LVIDTable *> SystemLVIDs;
  static SystemLVIDs systemLVIDs;

  const char *name;

  GStatsAvg nLinesOnKill;
  GStatsAvg nLinesOnRestart;
  GStatsAvg nLinesOnSetSafe;
  GStatsAvg nLinesOnSetFinished;

  // Cache where the LVIDTable is associated. There is a relation of 1 to 1
  // between cache and LVIDTable

  typedef HASH_MAP<const HVersion *, LVID *, HVersionHashFunc> Ver2LVIDMap;
  Ver2LVIDMap ver2lvid;

  typedef std::deque<LVID *> LVIDList;
  LVIDList freeLVIDs;
  LVIDList origLVIDs;

  LVIDSafest *lvidSafest;
protected:
  void localKill(const HVersion *v);
  void localRestart(const HVersion *v);
  void localSetSafe(const HVersion *v);
  void localSetFinished(const HVersion *v);

public:
  LVIDTable(const char *section, const char *n);

  LVID *findCreateLVID(HVersion *ver);
  LVID *findLVID(const HVersion *ver);
  void killSpecLVIDs(const HVersion *exeVer);
  void recycleLVID(LVID *lvid); // guarantee garbage collect (if called collects)

  LVID *getLVID2Recycle();

  LVIDSafest *getSafestEntry() const { return lvidSafest; }

  bool hasFreeLVIDs() const { return !freeLVIDs.empty(); }

  static void kill(const HVersion *v);
  static void restart(const HVersion *v);
  static void setSafe(const HVersion *v);
  static void setFinished(const HVersion *v);

  static void dump(const HVersion *v);
  void dump();
  
};


#endif // LVIDTABLE_H
