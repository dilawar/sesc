/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

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

#ifndef ASVERSION_H
#define ASVERSION_H

#include <limits.h>

#include "nanassert.h"
#include "GStats.h"
#include "pool.h"
#include "CacheCore.h"


// A HVersion points to the task context that it is responsible for the output
// of that version. A unique TaskContext is responsible for updating a single
// version. A TaskContext can read multiple versions (but only update one).
class TaskContext;

class HVersion {
 private:
  typedef long long VType;

  // HVersionReclaim (262143)
  // Max L is 28 bits (256 tasks) = 268435456
  static const VType HVersionReclaim = 268435456;

  typedef pool<HVersion, true> poolType;
  static poolType vPool;
  friend class poolType;

  // Statistics about the avg number of children per task
  static const size_t nChildrenStatsMax=10;
  static GStatsCntr *nChildren[];

  /************* Instance Data ***************/

  size_t nChild;

  // A node can be duplicated.
  int32_t nUsers;
#if 0
  HVersion *next;
  HVersion *prev;
 #endif
  static GStatsCntr *nCreate;
  static GStatsCntr *nShift;
  static GStatsCntr *nClaim;
  static GStatsCntr *nRelease;

  static HVersion *oldestTC; 
  static HVersion *oldest;
  static HVersion *newest;
  
  VType base;
  static VType lastver;
  //  bool atomic;
  bool dequeued;
  bool killed;

  TaskContext *tc;

  int32_t nOutsReqs;  // both outs instructions and mem requests                                                                  
  void spawn(HVersion *child, VType skip, bool atomic);
  void claim();
  void release();

  // Return the original version. Only when the system is initialized
  void setInitValues();

  // Set all fields but the versions
  HVersion *create(TaskContext *t);

#ifdef DEBUG
  bool visited;
  static void verify(HVersion *orig);
 public:
  VType getBase() { return base; }
 private:
#endif

  static void propagateOldestTaskContext(HVersion *v);

 protected:
 public:
  static HVersion *boot(TaskContext *t);

  HVersion *duplicate() {
    nUsers++;
  
    return this;
  }

  bool isAtomic() const { return base; }
  /*  void becomeAtomic()   { base = ++lastver; } */

  HVersion *createSuccessor(TaskContext *t, bool atomic = false);

  bool needReclaim() const { I(0); return true; } /* just for interface compatibility */

  void garbageCollect(bool noTC=false);

  bool isOldestTaskContext() const { return ! base; }
  bool isOldest() const { return ! base; }
  bool isNewest() const { return base; }

  // Safe token can not be propagate while previous instructions have
  // hasOutsReqs()
  bool hasOutsReqs()  const { return (nOutsReqs > 0); }
  void incOutsReqs() { nOutsReqs++; }
  void decOutsReqs();

  const HVersion *getNextRef() const { I(0); return NULL; }
  const HVersion *getPrevRef() const { I(0); return NULL; }

  static const HVersion *getOldestTaskContextRef() { I(0); return NULL; }
  static HVersion *getOldestDuplicate() { I(0); return NULL; }
  static const HVersion *getOldestRef() { I(0); return NULL; }
  static const HVersion *getNewestRef() { I(0); return NULL; }

  // If the task already commited, there may be no TC associated
  TaskContext *getTaskContext() const { return tc; }

  bool operator< (const HVersion &v) const { return base <  v.base; }
  bool operator<=(const HVersion &v) const { return base <= v.base; }
  bool operator> (const HVersion &v) const { return base >  v.base; }
  bool operator>=(const HVersion &v) const { return base >= v.base; }
  bool operator==(const HVersion &v) const { return base == v.base; }
  bool operator!=(const HVersion &v) const { return base != v.base; }
    
  void dump(const char *str, bool shortVersion = false) const;
  void dumpAll() const;

  int32_t  getPrio() const { 
    I(oldest);
    return (int)(base - oldest->base);
  }

  void restarted() {
    nChild   = 0;
  }

  // Is it the oldest task in the system that has a TaskContext (finish & safe)?

  
  bool isSafe() const { return !base; }
  
  void setSafe() {
    base = 0;
  }

  bool isKilled() const { return killed; }
  void setKilled() {
    killed   = true;
  }
  bool isFinished() const { return false; }
  
  // Interface compatibility:
  void setFinished() { I(0); }
  void clearFinished() { I(0); }
  

  static void report();
};

class HVersionLessThan {
public:
  bool operator()(const HVersion *v1, const HVersion *v2) const {
    return *v1 < *v2;
  }
};
class HVersionHashFunc {
public: 
  // DO NOT USE base because it can change with time
  size_t operator()(const HVersion *v) const {
    size_t val = (size_t)v;
    return val>>2;
  }
};

#endif // ASVERSION_H
