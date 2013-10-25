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
#ifndef DINST_H
#define DINST_H

#include "pool.h"
#include "nanassert.h"

#include "ThreadContext.h"
#include "Instruction.h"
#include "callback.h"

#include "Snippets.h"
#include "GStats.h"

#ifdef TASKSCALAR
#include "GLVID.h"
enum DataDepViolationAt { DataDepViolationAtExe=0, DataDepViolationAtFetch, 
                          DataDepViolationAtRetire, DataDepViolationAtMax };
#endif

#if (defined TLS)
namespace tls{
  class Epoch;
}
#endif


class Resource;
class FetchEngine;
class FReg;
class BPredictor;

//#define DINST_TRACK_PHYS 1

// FIXME: do a nice class. Not so public
class DInstNext {
 private:
  DInst *dinst;
#ifdef DINST_PARENT
  DInst *parentDInst;
#endif
 public:
  DInstNext() {
    dinst = 0;
  }

  DInstNext *nextDep;
  bool       isUsed; // true while non-satisfied RAW dependence
#ifdef DINST_TRACK_PHYS
  ushort preg;
#endif

  const DInstNext *getNext() const { return nextDep; }
  DInstNext *getNext() { return nextDep; }
  void setNextDep(DInstNext *n) {
    nextDep = n;
  }

  void init(DInst *d) {
    I(dinst==0);
    dinst = d;
  }

  DInst *getDInst() const { return dinst; }

#ifdef DINST_PARENT
  DInst *getParentDInst() const { return parentDInst; }
  void setParentDInst(DInst *d) {
    GI(d,isUsed);
    parentDInst = d;
  }
#else
  void setParentDInst(DInst *d) { }
#endif
};

class DInst {
 public:
  // In a typical RISC processor MAX_PENDING_SOURCES should be 2
  static const int32_t MAX_PENDING_SOURCES=2;

private:

  static pool<DInst> dInstPool;

  DInstNext pend[MAX_PENDING_SOURCES];
  DInstNext *last;
  DInstNext *first;
  
  int32_t cId;

  // BEGIN Boolean flags
  bool loadForwarded;
  bool issued;
  bool executed;
  bool depsAtRetire;
  bool deadStore;
  bool deadInst;
  bool waitOnMemory;

  bool resolved; // For load/stores when the address is computer, for
		 // the rest of instructions when it is executed


#ifdef SESC_MISPATH
  bool fake;
#endif
  // END Boolean flags

  // BEGIN Time counters
  Time_t wakeUpTime;
  // END Time counters

#ifdef DINST_TRACK_PHYS
  ushort preg;
#endif
#ifdef SESC_BAAD
  static int32_t fetch1QSize;
  static int32_t fetch2QSize;
  static int32_t issueQSize;
  static int32_t schedQSize;
  static int32_t exeQSize;
  static int32_t retireQSize;

  static GStatsTimingHist *fetch1QHist1;
  static GStatsTimingHist *fetch2QHist1;
  static GStatsTimingHist *issueQHist1;
  static GStatsTimingHist *schedQHist1;
  static GStatsTimingHist *exeQHist1;
  static GStatsTimingHist *retireQHist1;
  
  static GStatsHist *fetch1QHist2;
  static GStatsHist *fetch2QHist2;
  static GStatsHist *issueQHist2;
  static GStatsHist *schedQHist2;
  static GStatsHist *exeQHist2;
  static GStatsHist *retireQHist2;

  static GStatsHist *fetch1QHistUp;
  static GStatsHist *fetch2QHistUp;
  static GStatsHist *issueQHistUp;
  static GStatsHist *schedQHistUp;
  static GStatsHist *exeQHistUp;
  static GStatsHist *retireQHistUp;

  static GStatsHist *fetch1QHistDown;
  static GStatsHist *fetch2QHistDown;
  static GStatsHist *issueQHistDown;
  static GStatsHist *schedQHistDown;
  static GStatsHist *exeQHistDown;
  static GStatsHist *retireQHistDown;

  static GStatsHist **avgFetch1QTime;
  static GStatsHist **avgFetch2QTime;
  static GStatsHist **avgIssueQTime;
  static GStatsHist **avgSchedQTime;
  static GStatsHist **avgExeQTime;
  static GStatsHist **avgRetireQTime;

  static GStatsHist *brdistHist1;

  Time_t fetch1Time;
  Time_t fetch2Time;
  Time_t renameTime;
  Time_t issueTime;
  Time_t schedTime;
  Time_t exeTime;
#endif

#ifdef BPRED_UPDATE_RETIRE
  BPredictor *bpred;
  InstID oracleID;
#endif

  const Instruction *inst;
  VAddr vaddr;
  Resource    *resource;
  DInst      **RATEntry;
  FetchEngine *fetch;

#ifdef TASKSCALAR
  int32_t         dataDepViolationID;
  HVersion   *restartVer;
  HVersion   *lvidVersion;
  GLVID      *lvid;
  SubLVIDType subLVID;


#endif

  CallbackBase *pendEvent;

#if (defined TLS)
  tls::Epoch *myEpoch;
#endif // (defined TLS)


  char nDeps;              // 0, 1 or 2 for RISC processors

#ifdef DEBUG
 public:
  static int32_t currentID;
  int32_t ID; // static ID, increased every create (currentID). pointer to the
  // DInst may not be a valid ID because the instruction gets recycled
#endif
#if (defined MIPS_EMUL)
 public:
  ThreadContext::pointer context;
#endif
 protected:
 public:
  DInst();

  void doAtSimTime();
  StaticCallbackMember0<DInst,&DInst::doAtSimTime>  doAtSimTimeCB;

  void doAtSelect();
  StaticCallbackMember0<DInst,&DInst::doAtSelect>  doAtSelectCB;

  DInst *clone();

  void doAtExecuted();
  StaticCallbackMember0<DInst,&DInst::doAtExecuted> doAtExecutedCB;

#if (defined MIPS_EMUL)
  static DInst *createInst(InstID pc, VAddr va, int32_t cId, ThreadContext *context);
  static DInst *createDInst(const Instruction *inst, VAddr va, int32_t cId, ThreadContext *context);
#else
#if (defined TLS)
  static DInst *createInst(InstID pc, VAddr va, int32_t cId, tls::Epoch *epoch);
  static DInst *createDInst(const Instruction *inst, VAddr va, int32_t cId, tls::Epoch *epoch);
#else
  static DInst *createInst(InstID pc, VAddr va, int32_t cId);
  static DInst *createDInst(const Instruction *inst, VAddr va, int32_t cId);
#endif // Else of (defined TLS)
#endif // Else of (defined MIPS_EMUL)
  void killSilently();
  void scrap(); // Destroys the instruction without any other effects
  void destroy();

  void setResource(Resource *res) {
    I(!resource);
    resource = res;
  }
  Resource *getResource() const { return resource; }

  void setRATEntry(DInst **rentry) {
    I(!RATEntry);
    RATEntry = rentry;
  }

#ifdef BPRED_UPDATE_RETIRE
  void setBPred(BPredictor *bp, InstID oid) {
    I(oracleID==0);
    I(bpred == 0);
    bpred     = bp;
    oracleID = oid;
  }
#endif

#ifdef TASKSCALAR
  void addDataDepViolation(const HVersion *ver);
  // API for libvmem to notify the detection of the invalid memory
  // access
  void notifyDataDepViolation(DataDepViolationAt dAt=DataDepViolationAtExe, bool val=false);
  bool hasDataDepViolation() const {
    return restartVer != 0;
  }
  const HVersion *getRestartVerRef() const { return restartVer; }
  void setLVID(GLVID *b, HVersion *lvidV) {
    I(lvid==0);
    lvid        = b;
    subLVID     = lvid->getSubLVID();

    I(lvidVersion==0);
    lvidVersion = lvidV->duplicate();
    lvidVersion->incOutsReqs();
  }
  GLVID   *getLVID() const { return lvid; }
  SubLVIDType getSubLVID() const { return subLVID; }
  const HVersion *getVersionRef() const { return lvidVersion; }


#endif //TASKSCALAR

#if (defined TLS)
  void setEpoch(tls::Epoch *epoch){
    I(myEpoch);
    I(epoch==myEpoch);
    myEpoch=epoch;
  }
  tls::Epoch *getEpoch(void) const{
    I(myEpoch);
    return myEpoch;
  }
#endif

#ifdef DINST_PARENT
  DInst *getParentSrc1() const { 
    if (pend[0].isUsed)
      return pend[0].getParentDInst(); 
    return 0;
  }
  DInst *getParentSrc2() const { 
    if (pend[1].isUsed)
      return pend[1].getParentDInst();
    return 0;
  }
#endif

  void setFetch(FetchEngine *fe) {
    I(!isFake());

    fetch = fe;
  }

  FetchEngine *getFetch() const {
    return fetch;
  }

  void addEvent(CallbackBase *cb) {
    I(inst->isEvent());
    I(pendEvent == 0);
    pendEvent = cb;
  }

  CallbackBase *getPendEvent() {
    return pendEvent;
  }

  DInst *getNextPending() {
    I(first);
    DInst *n = first->getDInst();

    I(n);

    I(n->nDeps > 0);
    n->nDeps--;

    first->isUsed = false;
    first->setParentDInst(0);
    first = first->getNext();

    return n;
  }

  void addSrc1(DInst * d) {
    I(d->nDeps < MAX_PENDING_SOURCES);
    d->nDeps++;
    
    DInstNext *n = &d->pend[0];
    I(!n->isUsed);
    n->isUsed = true;
#ifdef DINST_TRACK_PHYS
    n->preg = preg;
#endif
    n->setParentDInst(this);

    I(n->getDInst() == d);
    if (first == 0) {
      first = n;
    } else {
      last->nextDep = n;
    }
    n->nextDep = 0;
    last = n;
  }

  void addSrc2(DInst * d) {
    I(d->nDeps < MAX_PENDING_SOURCES);
    d->nDeps++;
    #ifndef TLS
    I(!d->waitOnMemory); // pend[1] reused on memory ops. Not both! 
    #endif
    DInstNext *n = &d->pend[1];
    I(!n->isUsed);
    n->isUsed = true;
#ifdef DINST_TRACK_PHYS
    n->preg = preg;
#endif
    n->setParentDInst(this);

    I(n->getDInst() == d);
    if (first == 0) {
      first = n;
    } else {
      last->nextDep = n;
    }
    n->nextDep = 0;
    last = n;
  }

  void addFakeSrc(DInst * d, bool requeue = false) {
    I(d->nDeps < MAX_PENDING_SOURCES);
    d->nDeps++;
    #ifndef TLS
    GI(requeue == false, !d->waitOnMemory);
    d->waitOnMemory = true;
    #endif
    DInstNext *n = &d->pend[1];
    I(!n->isUsed);
    n->isUsed = true;
#ifdef DINST_TRACK_PHYS
    n->preg = preg;
#endif
    n->setParentDInst(this);

    I(n->getDInst() == d);
    if (first == 0) {
      first = n;
    } else {
      last->nextDep = n;
    }
    n->nextDep = 0;
    last = n;
  }

  char getnDeps() const { return nDeps; }

  bool isStallOnLoad() const {  return false; }

  bool isSrc1Ready() const { return !pend[0].isUsed; }
  bool isSrc2Ready() const { return !pend[1].isUsed; }
  bool isJustWaitingOnMemory() const { return !pend[0].isUsed && waitOnMemory; }
  bool hasDeps()     const { 
    GI(!pend[0].isUsed && !pend[1].isUsed, nDeps==0);
    return nDeps!=0;
  }
  bool hasPending()  const { return first != 0;  }
  const DInst *getFirstPending() const { return first->getDInst(); }
  const DInstNext *getFirst() const { return first; }

  const Instruction *getInst() const { return inst; }

  VAddr getVaddr() const { return vaddr;  }

  int32_t getContextId() const { return cId; }

  void dump(const char *id);

  // methods required for LDSTBuffer
  bool isLoadForwarded() const { return loadForwarded; }
  void setLoadForwarded() {
    I(!loadForwarded);
    loadForwarded=true;
  }

  bool isIssued() const { return issued; }
  void markIssued() {
    I(!issued);
    I(!executed);
    issued = true;
  }

  bool isExecuted() const { return executed; }
  void markExecuted() {
    I(issued);
    I(!executed);
    executed = true;
  }

  bool isDeadStore() const { return deadStore; }
  void setDeadStore() { 
    I(!deadStore);
    I(!hasPending());
    deadStore = true; 
  }

  void setDeadInst() { deadInst = true; }
  bool isDeadInst() { return deadInst; }
  
  bool hasDepsAtRetire() const { return depsAtRetire; }
  void setDepsAtRetire() { 
    I(!depsAtRetire);
    depsAtRetire = true;
  }
  void clearDepsAtRetire() { 
    I(depsAtRetire);
    depsAtRetire = false;
  }

  bool isResolved() const { return resolved; }
  void markResolved() { 
    resolved = true; 
  }

  bool isEarlyRecycled() const { return false; }

#ifdef SESC_MISPATH
  void setFake() { 
    I(!fake);
    fake = true; 
  }
  bool isFake() const  { return fake; }
#else
  bool isFake() const  { return false; }
#endif


  void awakeRemoteInstructions();

  void setWakeUpTime(Time_t t)  { 
    // ??? FIXME: Why fails?I(wakeUpTime <= t); // Never go back in time
    //I(wakeUpTime <= t);
    wakeUpTime = t;
  }

  Time_t getWakeUpTime() const { return wakeUpTime; }

#ifdef SESC_BAAD
  void setFetch1Time();
  void setFetch2Time();
  void setRenameTime();
  void setIssueTime();
  void setSchedTime();
  void setExeTime();
  void setRetireTime();
#endif

#ifdef DEBUG
  int32_t getID() const { return ID; }
#endif
};

class Hash4DInst {
 public: 
  size_t operator()(const DInst *dinst) const {
    return (size_t)(dinst);
  }
};

#endif   // DINST_H
