/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Smruti Sarangi
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

#ifndef RESOURCE_H
#define RESOURCE_H

#include "nanassert.h"

#include "callback.h"
#include "GStats.h"
#include "EnergyMgr.h"
#include "BloomFilter.h"

class PortGeneric;
class DInst;
class MemObj;
class Cluster;

enum StallCause {
  NoStall=0,
  SmallWinStall,
  SmallROBStall,
  SmallREGStall,
  OutsLoadsStall,
  OutsStoresStall,
  OutsBranchesStall,
  ReplayStall,
  PortConflictStall,
  SwitchStall,
  MaxStall
};

enum RetOutcome {
  Retired=0,
  NotExecuted,
  NotFinished,   // for loads only
  NoCacheSpace,  // for stores and ifetch ops only
  NoCachePorts,  // for stores only
  WaitForFence,  // for ifetch ops only
  MaxRetOutcome
};

enum NoRetResp { // instruction responsible for stall
  Self=0,
  Other,
  MaxNoRetResp
};

class Resource {
protected:
  Cluster *const cluster;
  PortGeneric *const gen;

  Resource(Cluster *cls, PortGeneric *gen);
public:
  virtual ~Resource();

  const Cluster *getCluster() const { return cluster; }
  Cluster *getCluster() { return cluster; }

  // Sequence:
  //
  // 1st) A canIssue check is done with "canIssue". This checks that the cluster
  // can accept another request (cluster window size), and that additional
  // structures (like the LD/ST queue entry) also have enough resources.
  //
  // 2nd) The timing to calculate when the inputs are ready is done at
  // simTime.
  //
  // 3rd) executed is called the instructions has been executed. It may be
  // called through DInst::doAtExecuted
  //
  // 4th) When the instruction is retired from the ROB retire is called

  virtual StallCause canIssue(DInst *dinst) = 0;
  virtual void simTime(DInst *dinst) = 0;
  virtual void executed(DInst *dinst);
  virtual RetOutcome retire(DInst *dinst);

};

class GMemorySystem;

class MemResource : public Resource {
private:
protected:
  MemObj  *L1DCache;
  GMemorySystem *memorySystem;

  
  GStatsEnergyBase *ldqCheckEnergy; // Check for data dependence
  GStatsEnergyBase *ldqRdWrEnergy;  // Read-write operations (insert, exec, retire)
  
  GStatsEnergyBase *stqCheckEnergy; // Check for data dependence
  GStatsEnergyBase *stqRdWrEnergy;  // Read-write operations (insert, exec, retire)
  GStatsEnergyBase *iAluEnergy;

  MemResource(Cluster *cls, PortGeneric *aGen, GMemorySystem *ms, int32_t id, const char *cad);
public:
};

class FUMemory : public MemResource {
private:
protected:
public:
  FUMemory(Cluster *cls, GMemorySystem *ms, int32_t id);

  StallCause canIssue(DInst *dinst);
  void simTime(DInst *dinst);
  RetOutcome retire(DInst *dinst);

};

class FULoad : public MemResource {
private:
  GStatsAvg   ldqNotUsed;
  GStatsCntr  nForwarded;

  const TimeDelta_t lat;
  const TimeDelta_t LSDelay;
  int32_t freeLoads;
  int32_t misLoads; // loads from wrong paths

protected:
  void cacheDispatched(DInst *dinst);
  typedef CallbackMember1<FULoad, DInst *, &FULoad::cacheDispatched> cacheDispatchedCB;

public:
  FULoad(Cluster *cls, PortGeneric *aGen
         ,TimeDelta_t l, TimeDelta_t lsdelay
         ,GMemorySystem *ms, size_t maxLoads
         ,int32_t id);

  StallCause canIssue(DInst *dinst);
  void simTime(DInst *dinst);
  RetOutcome retire(DInst *dinst);

  void executed(DInst *dinst);
  int32_t freeEntries() const { return freeLoads; }


#ifdef SESC_MISPATH
  void misBranchRestore();
#endif
};

class FUStore : public MemResource {
private:

  GStatsAvg   stqNotUsed;
  GStatsCntr  nDeadStore;

  const TimeDelta_t lat;
  int32_t               freeStores;
  int32_t               misStores;

  bool pendingFence;
  int32_t  nOutsStores;

  GStatsCntr   nFences;
  GStatsCntr   fenceStallCycles;
  
protected:
  void doRetire(DInst *dinst);
public:
  FUStore(Cluster *cls
          ,PortGeneric *aGen
          ,TimeDelta_t l
          ,GMemorySystem *ms
          ,size_t maxLoads
          ,int32_t id);

  StallCause canIssue(DInst *dinst);
  void simTime(DInst *dinst);
  void executed(DInst *dinst);
  RetOutcome retire(DInst *dinst);

  int32_t freeEntries() const { return freeStores; }


#ifdef SESC_MISPATH
  void misBranchRestore();
#endif

  bool waitingOnFence() {
    if (pendingFence)
      fenceStallCycles.inc();
    return pendingFence;
  }
  void storeCompleted();
  void storeSent() { nOutsStores++; }
  void doFence();
};

class FUGeneric : public Resource {
private:
  const TimeDelta_t lat;

protected:
public:
  GStatsEnergyCG *fuEnergy;

  FUGeneric(Cluster *cls, PortGeneric *aGen, TimeDelta_t l, GStatsEnergyCG *eb);

  StallCause canIssue(DInst *dinst);
  void simTime(DInst *dinst);
  void executed(DInst *dinst);


};

class SpawnRob;

class FUBranch : public Resource {
private:
  const TimeDelta_t lat;
  int32_t freeBranches;

protected:
public:
  FUBranch(Cluster *cls, PortGeneric *aGen, TimeDelta_t l, int32_t mb);

  StallCause canIssue(DInst *dinst);
  void simTime(DInst * dinst);
  void executed(DInst *dinst);

#ifdef SESC_BRANCH_AT_RETIRE
  RetOutcome retire(DInst *dinst);
#endif
};

class FUEvent : public Resource {
private:
protected:
public:
  FUEvent(Cluster *cls);

  StallCause canIssue(DInst *dinst);
  void simTime(DInst * dinst);


  
};

#endif   // RESOURCE_H
