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

#ifndef HVERSION_H
#define HVERSION_H


#include <vector>

#include <limits.h>

#include "nanassert.h"
#include "GStats.h"
#include "pool.h"
#include "CacheCore.h"
#include "ThreadContext.h"

// A HVersion points to the task context that it is responsible for
// the output of that version. A unique TaskContext is responsible for
// updating a single version. A TaskContext can read multiple versions
// (but only update one).
class TaskContext;

class HVersion;
class MemBufferDomain;

class HVersionDomain {
 private:
  typedef std::vector<HVersionDomain *> VDomainVectorType;
  static VDomainVectorType vdVec;

  MemBufferDomain *mbd;

  
 public:
  HVersion *oldestTC; 
  HVersion *oldest;
  HVersion *newest;

  HVersionDomain() {
    oldestTC = 0;
    oldest = 0;
    newest = 0;
    mbd = 0;
  }

  static HVersionDomain *create();
  static void tryPropagateSafeTokenAll();

  static int32_t getNDomains() { return vdVec.size(); }
  static HVersionDomain* getVDomain(int32_t id) { return vdVec[id]; }

  void setMemBufferDomain(MemBufferDomain *m) {
    I(m);
    mbd = m;
  }

  MemBufferDomain* getMemBufferDomain() {
    I(mbd);
    return mbd;
  }

};

class HVersion {
 private:
  typedef unsigned long long VType;

  // HVersionReclaim (262143) Max L is 28 bits (256 tasks) = 268435456
  static const VType HVersionReclaim = 268435456;

  // When a VType (base) reaches HVersionMax, all the versions on the
  // system should be shifted
  static const VType HVersionMax = 0xffffffffffffffffULL - 4*HVersionReclaim;

  class IDP { // Interval Distribution Predictor
  private:
    // Prediction accuracy statistics
    static GStatsCntr **correct;
    static GStatsCntr **incorrect;
    
	 typedef uint32_t PredEntryTag;
    
    class PredEntry {
    private:
      PredEntryTag tag;
      size_t nChild;

    public:
      void initialize(CacheGeneric<PredEntry, PredEntryTag> *c) {
	nChild = 0;
	tag = 0;
      }
      PredEntryTag getTag() const { return tag; }
      void setTag(PredEntryTag a) { 
	tag = a; 
      }
      
      bool isLocked () const {
	return false;
      }
      
      bool isValid() const { return tag; }
      void invalidate() { tag = 0; }

      size_t getnChild() const { return nChild; }
      void setnChild(size_t a) {
	nChild = a;
      }
      void dump(const char *str) {
      }
    };

    typedef CacheGeneric<PredEntry, PredEntryTag> PredType;


  protected:
    static size_t nChildMax;

    static PredType **predCache;
  public:
    static void boot();
    static void executed(Pid_t pid, PAddr addr, size_t nChild);
	 static int32_t predict(Pid_t pid, PAddr addr);
    static bool deactivated() { return nChildMax == 0; }
  };

  typedef pool<HVersion, true> poolType;
  static poolType vPool;
  friend class pool<HVersion, true>;

  // Statistics about the avg number of children per task
  static const size_t nChildrenStatsMax=10;
  static GStatsCntr *nChildren[];

  /************* Instance Data ***************/

#ifdef TS_TIMELINE
  static int32_t gID;
  int32_t id;
#endif

  size_t nChild;

  // A node can be duplicated.
  int32_t nUsers;

  HVersion *next;
  HVersion *prev;
  
  static GStatsCntr *nCreate;
  static GStatsCntr *nShift;
  static GStatsCntr *nClaim;
  static GStatsCntr *nRelease;

  HVersionDomain *vDomain;
  
  VType base;
  VType maxi;
  
  TaskContext *tc; // tc == 0 means, finished and (safe or killed)
  bool dequeued;
  bool finished; // Task finished execution (all threads)
  bool safe;     // Task is safe (may commit but order between safe version
		 // still needs to be enforced)
  bool killed;

  int32_t nOutsReqs;  // both outs instructions and mem requests


  void shiftAllVersions();
  void claim();
  void release();

  // Set all fields but the versions
  HVersion *create(TaskContext *t);

#ifdef DEBUG
  bool visited;
  static void verify(HVersion *orig);
#else
  static void verify(HVersion *orig) {
  };
#endif

 protected:
 public:
  
  static HVersion *boot(TaskContext *t);
  static HVersion *newFirstVersion(TaskContext *t);

  void setVersionDomain(HVersionDomain *vd) {
    I(vd);
    vDomain = vd;
  }

  HVersionDomain* getVersionDomain() const {
    I(vDomain);
    return vDomain;
  }

  MemBufferDomain* getMemBufferDomain() const {
    I(vDomain->getMemBufferDomain());
    return vDomain->getMemBufferDomain();
  }

#ifdef TS_TIMELINE
  int32_t getId() const { return id; }
#else
  int32_t getId() const { return 0; }
#endif
  HVersion *duplicate() {
    nUsers++;
  
    return this;
  }

  HVersion *createSuccessor(TaskContext *t);

  bool needReclaim() const { return (maxi - base) < 1; }  // really needs reclaim

  void garbageCollect(bool noTC=false);

  bool isOldestTaskContext() const { return vDomain->oldestTC == this; }
  bool isOldest() const { return prev == 0; }
  bool isNewest() const { return next == 0; }
  bool isOnly() const { GI(isOldest() && isNewest(), isSafe()); return (isOldest() && isNewest()); }

  // Safe token can not be propagate while previous instructions have
  // hasOutsReqs()
  bool hasOutsReqs()  const { return (nOutsReqs > 0); }
  int32_t  getnOutsReqs() const { return nOutsReqs; }
  void incOutsReqs() { nOutsReqs++; }
  void decOutsReqs();

  const HVersion *getNextRef() const { return next; }
  const HVersion *getPrevRef() const { return prev; }

  static const HVersion *getOldestTaskContextRef(const HVersionDomain *vd) { 
    return vd->oldestTC; 
  }

  static HVersion *getOldestDuplicate(const HVersionDomain *vd) { 
    return vd->oldest->duplicate(); 
  }

  static const HVersion *getOldestRef(const HVersionDomain *vd) { 
    return vd->oldest; 
  }

  static const HVersion *getNewestRef(const HVersionDomain *vd) { 
    return vd->newest; 
  }

  // If the task already commited, there may be no TC associated
  TaskContext *getTaskContext() const { return tc; }


  bool operator< (const HVersion &v) const { 
    I(vDomain == v.vDomain);
    return base <  v.base;
  }

  bool operator<=(const HVersion &v) const { 
    I(vDomain == v.vDomain);
    return base <= v.base; 
  }

  bool operator> (const HVersion &v) const { 
    I(vDomain == v.vDomain);
    return base >  v.base; 
  }
  
  bool operator>=(const HVersion &v) const { 
    I(vDomain == v.vDomain);
    return base >= v.base; 
  }

  bool operator==(const HVersion &v) const { 
    I(vDomain == v.vDomain);
    return base == v.base; 
  }

  bool operator!=(const HVersion &v) const { 
    I(vDomain == v.vDomain);
    return base != v.base; 
  }

  VType getBase() const { return base; }
    
  void dump(const char *str, bool shortVersion = false) const;
  void dumpAll() const;

  int32_t  getPrio() const { 
    I(vDomain->oldest);
    return (int)(base - vDomain->oldest->base);
  }

  void restarted() {
    finished = false;
    nChild   = 0;
  }

  // Is it the oldest task in the system that has a TaskContext (finish & safe)?

  bool isSafe() const { 
    GI(!safe,tc || killed);
    return safe; 
  }
  void setSafe();

  bool isKilled() const { return killed; }
  void setKilled();

  bool isFinished() const { 
    GI(!finished,tc);
    return finished; 
  }

  void setFinished();
  void clearFinished() {
    I(finished); // a task may be re-awaked
    finished = false;
  }


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

#endif // HVERSION_H
