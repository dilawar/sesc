/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  James Tuck
                  Liu Wei
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

#include "ThreadContext.h"
#include "MemBuffer.h"

#include "ExecutionFlow.h"

#include "TaskHandler.h"
#include "GMemorySystem.h"
#include "TraceGen.h"

#include "TaskContext.h"
#include "mintapi.h"


#ifdef OOO_PAPER_STATS
TaskContext::TaskEntriesType TaskContext::taskEntries;
#endif


StaticCallbackFunction0<TaskContext::collect> TaskContext::collectCB;

pool<TaskContext, true> TaskContext::tcPool(32, "TaskContext");

TaskContext::Tid2TaskContextType TaskContext::tid2TaskContext;

unsigned long long TaskContext::writeData=0;

int32_t TaskContext::SyncOnRestart;

size_t TaskContext::MLThreshold;
size_t TaskContext::MFThreshold;

GStatsCntr *TaskContext::nRestartInvMem=0;
GStatsCntr *TaskContext::nRestartException=0;
GStatsCntr *TaskContext::nRestartBubble=0;


GStatsCntr *TaskContext::nMergeNext=0;
GStatsCntr *TaskContext::nMergeLast=0;
GStatsCntr *TaskContext::nMergeSuccessors=0;
GStatsCntr *TaskContext::nInOrderSpawn=0;
GStatsCntr *TaskContext::nOutOrderSpawn=0;
GStatsCntr *TaskContext::nCorrectOutOrderSpawn=0;
GStatsCntr *TaskContext::nCorrectInOrderSpawn=0;

#ifdef OOO_PAPER_STATS
// Statistics about the avg number of children per task
GStatsCntr *TaskContext::nRestart[TaskContext::nRestartMax];

GStatsCntr *TaskContext::nOOSpawn=0;
GStatsCntr *TaskContext::nOOInst=0;
GStatsCntr *TaskContext::nIOSpawn=0;
GStatsCntr *TaskContext::nIOInst=0;
GStatsCntr *TaskContext::nOOSpawn2=0;
GStatsCntr *TaskContext::nOOInst2=0;
GStatsCntr *TaskContext::nIOSpawn2=0;
GStatsCntr *TaskContext::nIOInst2=0;
#ifdef TS_COUNT_TASKS_AHEAD
GStatsAvg  *TaskContext::nTasksAhead=0;
#endif 
#endif


GStatsCntr *TaskContext::thReadEnergy=0;   // FIXME: Energy
GStatsCntr *TaskContext::thWriteEnergy=0;  // FIXME: Energy
GStatsCntr *TaskContext::thFillEnergy=0;   // FIXME: Energy


GStatsCntr *TaskContext::nDataDepViolation[DataDepViolationAtMax];

// Histogram statistics
GStatsCntr *TaskContext::numThreads[TaskContext::MaxThreadsHist];
GStatsCntr *TaskContext::numRunningThreads[TaskContext::MaxThreadsHist];

HASH_MAP<int32_t,int> TaskContext::compressedStaticId;

TaskContext::TaskContext() 
{
  tid  = -1;
}

void TaskContext::setFields(Pid_t i, HVersion *v) 
{
  thFillEnergy->inc();  // Fill task holder

  // Only an executed task can be removed from the tid2TaskContext
  GI(tid2TaskContext[i], tid2TaskContext[i]->memVer->isFinished());

  I(osSim->getPriority(i) == 0);

  startBPHistory = 0;
  newBPHistory   = 0;

  tid       = i;
  memVer    = v;
  memBuffer = MemBuffer::create(memVer);

  nLMergeNext = 0;

  I(usedThreads.empty());
  usedThreads.insert(i);

  I(tid2TaskContext.size() > (size_t)tid);
  I(tid2TaskContext[tid] == 0);
  tid2TaskContext[tid] = this;

  // Just spawn, it can be safe already
  I(!memVer->isSafe());
  I(!memVer->isFinished());

  nLocalRestarts  = 0;
  dataDepViolationID = 0;
  dataDepViolation   = false;


  sContext.copy(osSim->getContext(tid));

  ProcessId *proc = ProcessId::getProcessId(tid);
  I(proc);
  proc->clearExecuted();
  proc->clearWaiting();

  canEarlyAwake = true;
}

void TaskContext::freeThread(Pid_t pid)
{
  I(pid>=0);

  // Remove from usedThreads
  TaskContext *tc = tid2TaskContext[pid];
  I(tc);

  Pid_tSet::iterator uit = tc->usedThreads.find(pid);
  I(uit != tc->usedThreads.end());
  tc->usedThreads.erase(uit);

  LOG("TaskContext::freeThread(%d) 0x%x", pid, (int)tc->spawnAddr);

  I(tid2TaskContext[pid]);
  tid2TaskContext[pid]=0;

  // Recycled Thread directly
  osSim->eventExit(pid, 0);
}

void TaskContext::destroy()
{
  I(memBuffer == 0);
  I(memVer);



  memVer->garbageCollect(true);
  memVer = 0;

  IS(memVer = 0);
  IS(NES  = -1);  // Uggly value so that it crashes

  I(usedThreads.empty());
  tcPool.in(this);
}

void TaskContext::mergeDestroy()
{
  I(memVer);
  I(memVer->isSafe());

  LOG("TaskContext(%d)::mergeDestroy", tid);

  nCorrectOutOrderSpawn->cinc(memVer->getTaskContext()->wasSpawnedOO);
  nCorrectInOrderSpawn->cinc(!memVer->getTaskContext()->wasSpawnedOO);

  int32_t nInst = ProcessId::getProcessId(tid)->getNGradInsts();

#ifdef OOO_PAPER_STATS
  TaskEntriesType::iterator it = taskEntries.find(spawnAddr);
  TaskEntry *te;
  if(taskEntries.find(spawnAddr) == taskEntries.end()) {
    te = new TaskEntry();
    taskEntries[spawnAddr] = te;
  } else {
    te = (*it).second;
  }
  te->nSpawns++;
  te->nInst += nInst;
  if (memVer->getTaskContext()->isOOtask()) {
    nOOSpawn->inc();
    nOOInst->add(nInst);
    te->outOrder = true;
  } else {
    nIOSpawn->inc();
    nIOInst->add(nInst);
  }

  nRestart[nLocalRestarts<(int)nRestartMax?nLocalRestarts:nRestartMax-1]->inc();
#endif


  I(memBuffer);
  memBuffer->mergeDestroy();
  memBuffer = 0;

  clearStateSafe();
  freeThread(tid);

  I(usedThreads.empty());

  I(memVer->isOldestTaskContext());

  I(usedThreads.empty());

  destroy();
}

// Terminate all threads but the main
void TaskContext::clearState()
{
  I(memVer);
  I(!memVer->isSafe());
  I(memBuffer == 0);
  
  // Terminate on-the-fly threads
  if (usedThreads.size() >1 ) {
    for(Pid_tSet::iterator it = usedThreads.begin(); it != usedThreads.end(); it++) {
      if (tid == *it)
	continue;

      freeThread(*it);
    }
  }

  // Just one tid is kept
  I(usedThreads.size() == 1);
  I(usedThreads.find(tid) != usedThreads.end());
}

void TaskContext::clearStateSafe()
{
  I(memVer->isSafe());
  I(memBuffer == 0);
  
  if (usedThreads.size()  == 1 )
    return;
  
  clearState();
}

PAddr TaskContext::adjustChildSpawnAddr(PAddr childAddr)
{
  // There may be some nops. Skip them
  const Instruction *inst = Instruction::getInst4Addr(childAddr);
  // warning! this is assuming there is a nop at the begininf of every task
  while(!inst->isNOP()) { // skipping the if(commit) boundary instructions
    inst = Instruction::getInst(inst->calcNextInstID());
  }
  while(inst->isNOP()) {
    inst = Instruction::getInst(inst->calcNextInstID());
  }

  return inst->getAddr();
}


//XXX liuwei: actually it gets the jump address following 
//            a spawn point which is not necessary a new task
PAddr TaskContext::calcChildSpawnAddr(PAddr childAddr)
{
  ID(int32_t conta=0);
  const Instruction *inst = Instruction::getInst4Addr(childAddr);

  while(!inst->isBJCond()) {
    inst = Instruction::getInst(inst->calcNextInstID());
    GLOG(conta>20, "pc=0x%x should be a branch (failed)", (int)Instruction::getInst4Addr(childAddr)->getAddr());
    IS(conta++);
  }

  inst =  Instruction::getInst(inst->getTarget());

  // skip nops
  while(inst->isNOP()) {
    inst = Instruction::getInst(inst->calcNextInstID());
  }

  return inst->getAddr();
}

void TaskContext::localRestart()
{
  // NOTE: DO NOT CALL THIS METHOD DIRECTLY. Use taskHandler (there may be inheritance)
  I(!memVer->isSafe());

  dataDepViolation = false;

  thWriteEnergy->inc();  // mark entry

  for(Pid_tSet::iterator it = usedThreads.begin(); it != usedThreads.end(); it++) {
    ProcessId *proc = ProcessId::getProcessId(*it);
    proc->restart();
  }

  memVer->restarted();

  LOG("TaskContext(%d)::restart", tid);


  // justDestroy must be called before mergeSuccessors and
  // terminateAllThreadsButOriginal because it can generate a version
  // unclaim/reclaim, and the order of memOps can be invalid. ooopss!!!
  I(memBuffer);
  memBuffer->justDestroy();
  memBuffer = 0;

  mergeSuccessors(true);
  I(memVer->isNewest());
  NES = 0;

  clearState();

  I(tid2TaskContext[tid] == this);

  memBuffer = MemBuffer::create(memVer);

  newBPHistory = startBPHistory;

  ProcessId *proc = ProcessId::getProcessId(tid);
  I(proc);

  if (proc->isExecuted())
    osSim->resume(tid);
  proc->clearExecuted();

  bool wasSyncBecomeSafe;
  if (proc->isWaiting()) {
    osSim->resume(tid);
    wasSyncBecomeSafe=true;
  }else{
    wasSyncBecomeSafe=false;
  }
  proc->clearWaiting();

  if (osSim->getState(tid) == RunningState) {
    // TODO: do not stop and awake the process. slow and not necessary
    osSim->stop(tid);
    ThreadContext *context=osSim->getContext(tid);
    
    context->copy(&sContext); // Recover state
    
    osSim->unstop(tid);
  }else{
    ThreadContext *context=osSim->getContext(tid);

    context->copy(&sContext); // Recover state
    
    I(osSim->getState(tid) == ReadyState);
  }

#ifdef TS_PARANOID
  for(int32_t i=0; i<68; i++)
    bad_reg[i] = true;

  bad_reg[28]=false;
  bad_reg[29]=false;
  bad_reg[30]=false;
  bad_reg[31]=false;
#endif

  if (nLocalRestarts > SyncOnRestart || wasSyncBecomeSafe)
    syncBecomeSafe();
  else if (nLocalRestarts == SyncOnRestart)
    syncBecomeSafe(true); // Can early awake, one extra restart available

  nLocalRestarts++;
  canEarlyAwake = true;

}

// kill can be called by someone else than the parent, but it always
// would be a predecessor. Otherwise there could be a livelock.

void TaskContext::localKill(bool inv)
{
  // NOTE: DO NOT CALL THIS METHOD DIRECTLY. Use taskHandler (there may be inheritance)


  I(!memVer->isSafe());
  LOG("TaskContext(%d)::localKill", tid);

  for(Pid_tSet::iterator it = usedThreads.begin(); it != usedThreads.end(); it++) {
    ProcessId *proc = ProcessId::getProcessId(*it);
    proc->kill(inv);
  }

  thReadEnergy->inc();   // Read local info
  thWriteEnergy->inc();  // mark entry

  I(memBuffer);
  memBuffer->justDestroy();
  memBuffer = 0;

  mergeSuccessors(inv);

  clearState();

  freeThread(tid);

  I(usedThreads.empty());

  I(!memVer->isSafe());

  destroy();
}

void TaskContext::mergeLast(bool inv)
{

  I(!memVer->isNewest());

  nMergeLast->inc();

  const HVersion *last = HVersion::getNewestRef(memVer->getVersionDomain());
  I(last);
  I(last->getNextRef() == 0);
  const HVersion *prev = last->getPrevRef();
  I(prev);
  TaskContext *lastTC  = last->getTaskContext();
  TaskContext *prevTC  = prev->getTaskContext();
  I(lastTC);
  I(prevTC); // Both should exists, otherwise even the safe task got killed
  I(lastTC != this); // Can't kill itself

  LOG("mergeLast num=%d, vPid %d (0x%x) prevPid %d (0x%x)"
      , ProcessId::getNumThreads(), lastTC->tid, (int)lastTC->spawnAddr, prevTC->tid, (int)prevTC->spawnAddr);

  I(!lastTC->memVer->isSafe());

  I(!prevTC->memVer->isNewest());

  taskHandler->kill(lastTC->memVer, inv);

  I(prevTC->memVer->isNewest());
  prevTC->NES =0;
  if (prevTC->memVer->isFinished()) {
    // The oldest must be re-awake so that it can do mergeLast as needed
    ProcessId *proc = ProcessId::getProcessId(prevTC->tid);
    I(proc->isExecuted());
    I(!proc->isWaiting());
    osSim->resume(prevTC->tid);
    proc->clearExecuted();
    prevTC->memVer->clearFinished();
  }
}

void TaskContext::mergeSuccessors(bool inv)
{

  if (memVer->isNewest())
    return;
  
  nMergeSuccessors->inc();
  do {
    mergeLast(inv);
  }while (!memVer->isNewest());
}

void TaskContext::awakeIfWaiting()
{
  if (!memVer->isSafe() && !canEarlyAwake)
    return; // Needs to be safe to be awaken

  for(Pid_tSet::const_iterator it = usedThreads.begin(); it != usedThreads.end();  it++) {
    ProcessId *proc = ProcessId::getProcessId(*it);
    I(!(proc->isWaiting() && proc->isExecuted()));

    if (proc->isWaiting()) {
      LOG("+++++++++++++awakeBecomeSafe(%d)",*it);
      proc->clearWaiting();
      osSim->resume(*it);
    }
  }
}

void TaskContext::syncBecomeSafe(bool earlyAwake)
{
  canEarlyAwake = canEarlyAwake && earlyAwake;

  if (!canSyncBecomeSafe())
    return;

#ifdef TS_TIMELINE
  TraceGen::add(memVer->getId(),"syncSafe=%lld",globalClock);
#endif

  I(!memVer->isSafe());

  // TODO: In the future it must be all the usedThreads
  
  LOG("+++++++++++++syncBecomeSafe (%d)", tid);

  ProcessId *proc = ProcessId::getProcessId(tid);

  I(!proc->isWaiting());
  proc->setWaiting();
  osSim->suspend(tid);
}

void TaskContext::tryPropagateSafeToken(const HVersionDomain *vd)
{
  bool doMergeOps = false;

  size_t nCommited=0;

  //LOG("TC::tryPropagateSafeToken()"); // way too often to be useful
  // Advance for all the continuously already finished TaskContext
  const HVersion *v = HVersion::getOldestTaskContextRef(vd);
  while(v) {
    TaskContext *tc = v->getTaskContext();
    I(tc);
    I(tc->memVer == v);

    //I(!tc->hasDataDepViolation());
    if (tc->hasDataDepViolation()) 
      break;

    if (!v->isSafe()) {
      osSim->setPriority(tc->getPid(), -1);
      LOG("TaskContext(%d)::setSafe", tc->getPid());
      taskHandler->setSafe(tc->memVer);
      doMergeOps = true;
    }

    I(v->isSafe());

    if (!v->isFinished() || v->hasOutsReqs()) {
      // Still not possible to propagate the commit token, because the
      // successor task can still get a restart from a in-flight
      // instruction (even if all the instructions finished, all the
      // memory operations have to finish too)

      const HVersion *vNext = v->getNextRef();
      if (vNext) {
	TaskContext *tc2 = vNext->getTaskContext();
	I(tc2);
	// Priority change only can happen if the task
	// finished. Otherwise, there can be priority inversion
	if (v->isFinished() )
	  osSim->setPriority(tc2->getPid(), -1);
	//#define TS_EARLY_RESTART
#ifdef TS_EARLY_RESTART
	if (ProcessId::getNumRunningThreads() < MFThreshold ) {
	  // Only try early restart if there are no work
	  tc2->awakeIfWaiting();
	}
#endif
      }
      break;
    }
    I(v->isFinished() && !v->hasOutsReqs());

    nCommited++;

    v = v->getNextRef();
  }

  while (nCommited) {
    v = HVersion::getOldestTaskContextRef(vd);
    I(v);
    TaskContext *tc = v->getTaskContext();
    I(tc);
    I(tc->memVer->isSafe());
    I(tc->memVer->isFinished());

#ifdef TS_TIMELINE
    TraceGen::add(tc->memVer->getId(),"executed=%lld",globalClock);
#endif

    tc->mergeDestroy();

    
    // mergeDestroy advanced the oldest TaskContext
    I(HVersion::getOldestTaskContextRef(vd) != v);
    nCommited--;
  }

#ifdef TC_PARTIALORDER
  if(doMergeOps) {
    v = HVersion::getOldestTaskContextRef(vd);
    I(v);
    TaskContext *tc = v->getTaskContext();
    //LOG("merging data to memory tc=%d", tc->getPid());
    I(tc);
    I(tc->memVer->isSafe());
    tc->memBuffer->mergeOps();
  }
#endif
}

void TaskContext::collect()
{
  if (!osSim->hasWork() || ExecutionFlow::isGoingRabbit())
    return;
  
  dumpAll();

  // Histogram statistics
  size_t num = ProcessId::getNumThreads();
  if (num>=MaxThreadsHist)
    num = MaxThreadsHist-1;
  numThreads[num]->inc();

  num = ProcessId::getNumRunningThreads();
  if (num>=MaxThreadsHist)
    num = MaxThreadsHist-1;
  numRunningThreads[num]->inc();

  collectCB.schedule(63023);
}

void TaskContext::preBoot()
{
  nRestartInvMem         = new GStatsCntr("TC:nRestartInvMem");
  nRestartException      = new GStatsCntr("TC:nRestartException");
  nRestartBubble         = new GStatsCntr("TC:nRestartBubble");


  nMergeNext             = new GStatsCntr("TC:nMergeNext");
  nMergeLast             = new GStatsCntr("TC:nMergeLast");
  nMergeSuccessors       = new GStatsCntr("TC:nMergeSuccessors");
  nInOrderSpawn          = new GStatsCntr("TC:nInOrderSpawn");
  nOutOrderSpawn         = new GStatsCntr("TC:nOutOrderSpawn");
  nCorrectOutOrderSpawn  = new GStatsCntr("TC:nCorrectOutOrderSpawn");
  nCorrectInOrderSpawn   = new GStatsCntr("TC:nCorrectInOrderSpawn");
#ifdef OOO_PAPER_STATS
  for(size_t i=0;i<nRestartMax;i++) {
    nRestart[i] = new GStatsCntr("TC(%d):nRestart",i);
  }

  nOOSpawn               = new GStatsCntr("TC:nOOSpawn");
  nIOSpawn               = new GStatsCntr("TC:nIOSpawn");
  nOOInst                = new GStatsCntr("TC:nOOInst");
  nIOInst                = new GStatsCntr("TC:nIOInst");
  nOOSpawn2              = new GStatsCntr("TC:nOOSpawn2");
  nIOSpawn2              = new GStatsCntr("TC:nIOSpawn2");
  nOOInst2               = new GStatsCntr("TC:nOOInst2");
  nIOInst2               = new GStatsCntr("TC:nIOInst2");
#ifdef TS_COUNT_TASKS_AHEAD
  nTasksAhead            = new GStatsAvg("TC:nTasksAhead");
#endif
#endif


  thReadEnergy   = new GStatsCntr("TC:thReadEnergy");
  thWriteEnergy  = new GStatsCntr("TC:thWriteEnergy");
  thFillEnergy   = new GStatsCntr("TC:thFillEnergy");

  nDataDepViolation[DataDepViolationAtExe]   = new GStatsCntr("TC:nDataDepViolationAtExe");
  nDataDepViolation[DataDepViolationAtFetch] = new GStatsCntr("TC:nDataDepViolationAtFetch");
  nDataDepViolation[DataDepViolationAtRetire]= new GStatsCntr("TC:nDataDepViolationAtRetire");

  for(size_t i=0;i<MaxThreadsHist;i++) {
    numThreads[i]        = new GStatsCntr("TC:numThreads_%d",i);
    numRunningThreads[i] = new GStatsCntr("TC:numRunningThreads_%d",i);
  }

  collectCB.schedule(63023);

  MLThreshold = SescConf->getInt("TaskScalar","MLThreshold");
  MFThreshold = SescConf->getInt("TaskScalar","MFThreshold");

  SyncOnRestart = SescConf->getInt("TaskScalar","SyncOnRestart");

  // First must be lower than Last
  
  I(MAXPROC>30);
  SescConf->isBetween("TaskScalar" ,"MLThreshold",2,MAXPROC-20);
  SescConf->isBetween("TaskScalar" ,"MFThreshold",2,MAXPROC-20);

  for(int32_t i=0;i<MAXPROC;i++) {
    tid2TaskContext.push_back(0);
  }
  I(tid2TaskContext.size() == MAXPROC);


  MemBufferDomain *mbd = MemBuffer::createMemBufferDomain();

  TaskContext *tc = tcPool.out();

  tc->spawnAddr       = (uint) -1;
  tc->memVer          = HVersion::boot(tc);
  tc->memVer->getVersionDomain()->setMemBufferDomain(mbd);

  tid2TaskContext[0]  = tc;
  tc->wasSpawnedOO    = false;
  tc->ooTask          = false;

#ifdef TS_PARANOID
  for(int32_t i=0; i<68; i++)
    tc->bad_reg[i] = false;
#endif



}

void TaskContext::postBoot()
{
  TaskContext *tc = tid2TaskContext[0];
  I(tc);

  I(tid2TaskContext[0]);
  
  // Create original TaskContext
  tid2TaskContext[0]=0; // setFields would do it again (avoid warning)
  tc->setFields(0, tc->memVer);

  I(!tc->memVer->isSafe());
  taskHandler->setSafe(tc->memVer);
  I(tc->memVer->isOldest());
  I(tc->memVer->isOldestTaskContext());
}

void TaskContext::spawnSuccessor(Pid_t childPid, PAddr childAddr, int32_t sid)
{
  thReadEnergy->inc();  // Read local info
  thWriteEnergy->inc(); // update local info acordingly

#if defined(CLAIM_MERGE_SUCCESSOR) && defined(TS_MERGELAST)
  if (!memVer->isNewest()) {
    // If not the most speculative we can run out of bubble
    if (memVer->needReclaim()) {
      I(memVer->getNextRef());
#ifdef TS_TIMELINE
      TraceGen::add(memVer->getId(),"Bubble=%lld",globalClock);
#endif      
      taskHandler->kill(memVer->getNextRef(), false);
      nRestartBubble->inc();
    }
    I(!memVer->needReclaim());
  }
#endif

  TaskContext *tc    = tcPool.out();

  tc->spawnAddr = calcChildSpawnAddr(childAddr);
  tc->staticId  = sid;

  tc->ooTask         = false;
  if(memVer->isNewest()) {
    tc->wasSpawnedOO = false;
    nInOrderSpawn->inc();
  } else {
#ifdef TS_CHECK_INORDER
    // Just to check the in-order-compiler compiler
    MSG("Out of order spawn 0x%x -> 0x%x", spawnAddr, tc->spawnAddr);
#endif
    tc->wasSpawnedOO = true;
    nOutOrderSpawn->inc();
    tc->setOOtask();
  }

  // pass NES to the continuation
  tc->NES  = NES;
  NES      = 0;

#ifdef TS_PARANOID
  for(int32_t i=0; i<68; i++)
    tc->bad_reg[i] = true;

  tc->bad_reg[28]=false;
  tc->bad_reg[29]=false;
  tc->bad_reg[30]=false;
  tc->bad_reg[31]=false;
#endif

#if defined(TS_COUNT_TASKS_AHEAD) && defined(OOO_PAPER_STATS)
  int32_t ntAhead=0;
  for(const HVersion *hp=memVer->getNextRef(); hp != 0; hp = hp->getNextRef(), ntAhead++);
  nTasksAhead->sample(ntAhead);
#endif

  HVersion *childVer = memVer->createSuccessor(tc);


#ifdef TS_TIMELINE
  TraceGen::add(childVer->getId(),"base=%lld:spawn=%lld:addr=0x%x",memVer->getBase(), globalClock, spawnAddr);
  TraceGen::add(memVer->getId()  ,"childId=%ld:childT=%lld",childVer->getId(),globalClock);
#endif

  tc->setFields(childPid, childVer);

}


void TaskContext::normalFork(Pid_t cpid)
{
  // The same context is inside that thread. It is not necessary to remember
  // like in a normal spawn. Anyway, if the thread gets restarted the thread
  // must be killed.

  I(usedThreads.find(cpid) == usedThreads.end());
  usedThreads.insert(cpid);

  I(tid2TaskContext.size() > (size_t)cpid);
  tid2TaskContext[cpid] = this;
}

void TaskContext::normalForkNewDomain(Pid_t cpid)
{
  MSG("normal fork. new pid: %d", cpid);
  TaskContext *nTC = tcPool.out();

  nTC->spawnAddr       = (uint) -1;
  nTC->memVer          = HVersion::newFirstVersion(nTC);

  MemBufferDomain *mbd = MemBuffer::createMemBufferDomain();
  nTC->memVer->getVersionDomain()->setMemBufferDomain(mbd);

  nTC->wasSpawnedOO    = false;
  nTC->ooTask          = false;


  nTC->setFields(cpid, nTC->memVer);
  I(!nTC->memVer->isSafe());
  taskHandler->setSafe(nTC->memVer);
  I(nTC->memVer->isOldest());
  I(nTC->memVer->isOldestTaskContext());
}

// fpid is finishing pid
void TaskContext::endTaskExecuted(Pid_t fpid)
{
  if (dataDepViolation) {
    syncBecomeSafe();
    return;
  }

  LOG("TC(%d)::endTaskExecuted(%d)", tid, fpid);

  I(!memVer->isFinished());

  ProcessId *proc = ProcessId::getProcessId(fpid);

  I(!proc->isExecuted());
  I(!proc->isWaiting());
  proc->setExecuted();
  osSim->suspend(fpid);

  // Did all usedThreads finished?
  for(Pid_tSet::const_iterator it = usedThreads.begin(); it != usedThreads.end(); it++) {
    ProcessId *p = ProcessId::getProcessId(*it);
    if (!p->isExecuted()) {
      LOG("TC::setExecuted not all threads finished, awake parent of %d", fpid);
      osSim->tryWakeupParent(fpid);
      return;
    }
  }

  I(NES==0);

  taskHandler->setFinished(memVer);

  

  if (memVer->isOldestTaskContext()) {
    tryPropagateSafeToken(memVer->getVersionDomain());
  }else{
#ifdef DEBUG
    // Fail when all the previous versions have finished, they should be
    // commited by now
    for (const HVersion *v = HVersion::getOldestRef(memVer->getVersionDomain()) 
	   ; v!=memVer 
	   ; v = v->getNextRef()) {
      if (v->getTaskContext()) {
	I(v->isOldestTaskContext());
	
	break;
      }
      I(v->isFinished());
    }
#endif
  }
}

void TaskContext::invalidMemAccess(int32_t rID, DataDepViolationAt rAt)
{
  if (rID != dataDepViolationID || !dataDepViolation)
    return;

  
  nDataDepViolation[rAt]->inc();

  dataDepViolation = false;

  taskHandler->restart(memVer);
  nRestartInvMem->inc();
#ifdef TS_TIMELINE
  TraceGen::add(memVer->getId(),"InvMem=%lld",globalClock);
#endif      
}

void TaskContext::exception()
{
  taskHandler->restart(memVer);
  nRestartException->inc();
#ifdef TS_TIMELINE
  TraceGen::add(memVer->getId(),"Exception=%lld",globalClock);
#endif
}


bool TaskContext::canMergeNext()
{


#ifdef MERGE_FIRST_ALL
  nMergeNext->inc();
  nLMergeNext++;
  return true;
#endif

  size_t nR = ProcessId::getNumRunningThreads();
  size_t nX = ProcessId::getNumThreads();

  bool doMergeLast = nX >= MLThreshold;
  bool doMergeNext = nR >= MFThreshold;

#ifdef TS_INORDER
  if( !memVer->isNewest() ) {
    // Enforce in-order task spawn
    //doMergeNext = true; // merge
    doMergeLast = true; // kill
  }
#endif

  if (memVer->isNewest() && doMergeLast) {
    // The most speculative was going to do a mergeLast, just do a
    // mergeNext now because it is the same, and it is faster to execute
    LOG("1.mix-mergeNext-Last NES=%ld, vPid %d", NES, tid );

#ifdef TS_MERGENEXT
    nMergeNext->inc();
    nLMergeNext++;
#endif
    return true;
  }

#ifdef TS_MERGELAST
  if (doMergeLast)
    mergeLast(false);
#endif

#ifndef TS_INORDER
  if (doMergeNext && (nLMergeNext & 0x7 == 0x7) && !memVer->isSafe()) {
    // If the task got too big, do not do mergeNext (only if task can
    // get restarted)
    doMergeNext = false;
  }
#endif

#ifdef TS_MERGENEXT
  if (doMergeNext) {
    if (!memVer->isNewest())
      NES++;
    
    LOG("1.mergeNext NES=%ld, vPid %d", NES, tid );
    
    nMergeNext->inc();
    nLMergeNext++;
    return true;
  }
#endif

  return false;
}

int32_t TaskContext::ignoreExit()
{
  I(NES>=0); // Can't be negative

  if (memVer->isNewest()) {
    I(NES == 0); // Newest does not need to track NES

    LOG("1.mergeNext 0x%x -> 0x%x", (int)spawnAddr, (int)adjustChildSpawnAddr(osSim->getContextRegister(tid, 31)));

    spawnAddr = adjustChildSpawnAddr(osSim->getContextRegister(tid, 31));

    return 1; // ignore exit if newest task
  }

  if (NES>0) {
    NES--;
    
    PAddr mergeAddr = adjustChildSpawnAddr(osSim->getContextRegister(tid, 31));

    LOG("2.mergeNext 0x%x -> 0x%x @(%d:0x%x)", (int)spawnAddr, (int)mergeAddr, tid, (int)osSim->getContextRegister(tid, 31));

    spawnAddr = mergeAddr;

    return 1; // ignore exit
  }

  LOG("TaskContext::exit(%d) 0x%x -> 0x%x", tid, (int)spawnAddr, (int)osSim->getContextRegister(tid, 31));

  return 0; // do normal exit
}

void TaskContext::report()
{
#ifdef OOO_PAPER_STATS
  TaskEntriesType::iterator it = taskEntries.begin();
  for(; it != taskEntries.end(); it++) {
    TaskEntry *te = (*it).second;
    if(te->outOrder) {
      nOOSpawn2->add(te->nSpawns);
      nOOInst2->add(te->nInst);
    } else {
      nIOSpawn2->add(te->nSpawns);
      nIOInst2->add(te->nInst);
    }
  }
#endif

  HVersion::report();
}

void TaskContext::dumpAll()
{
  const HVersion *v;

  printf("TC[%3d:%3d]%10lld:", ProcessId::getNumThreads(), ProcessId::getNumRunningThreads(), globalClock);

  for(int32_t i = 0; i < HVersionDomain::getNDomains(); i++) {
    HVersionDomain *vd = HVersionDomain::getVDomain(i);
    v = HVersion::getOldestRef(vd);
    int32_t numL=0;
    while(v) {
      const TaskContext *tc = v->getTaskContext();
      if (tc==0) {
	numL++;
	v = v->getNextRef();
	continue;
      }
      if (numL) {
	printf("%3dxL",numL); // Lazy Commit
	numL=0;
      }

      I(tc->tid>=0);

      for(Pid_tSet::const_iterator it = tc->usedThreads.begin(); it != tc->usedThreads.end(); it++) {
	const ProcessId *proc = ProcessId::getProcessId(*it);

	if(proc->isExecuted())
	  printf("X"); // Executed, waiting for predecessors to finish
	else if(proc->isWaiting())
	  printf("W"); // Waiting to become safe
	else
	  printf("_"); // Executing or waiting to execute
      }
      v = v->getNextRef();
    }
    printf(" \t");
  }

  printf("\n");
}

BPred::HistoryType TaskContext::getBPHistory(BPred::HistoryType cHistory)
{
  // called at BPred::switchIn
  if (newBPHistory)
    return newBPHistory;

#ifdef TS_RESET_HISTORY
  startBPHistory = (spawnAddr>>3) ^ (spawnAddr>>5);
#else
  startBPHistory = cHistory; // Do not reset, use current
#endif

  newBPHistory = startBPHistory;

  //#define TS_FUTURE_HISTORY

#ifdef TS_FUTURE_HISTORY
  if (memVer->getNextRef()) {
    // Next task. Try to get future history
    TaskContext *nTC = memVer->getNextRef()->getTaskContext();
    if (nTC->newBPHistory && nTC->newBPHistory != nTC->startBPHistory) {
      MSG("future history passed [0x%llx] instead of [0x%llx] cHis[0x%llx]"
	  ,nTC->newBPHistory, newBPHistory, cHistory
	  );
      newBPHistory ^= nTC->newBPHistory;
    }
  }
#endif

  return newBPHistory;
}

void TaskContext::setBPHistory(BPred::HistoryType h) 
{ 
  // called at BPred::switchOut
  newBPHistory = h;
}

void TaskContext::setOOtask() 
{
  const HVersion *v = getVersionRef();

  while(v) {
    TaskContext *t = v->getTaskContext();
    if (t)
      t->ooTask=true;
    v = v->getNextRef();
  }
}


int32_t TaskContext::getCompressedStaticId(int32_t staticId)
{
  if (compressedStaticId.find(staticId) == compressedStaticId.end()) {
    compressedStaticId[staticId] = compressedStaticId.size();
  } 

  return compressedStaticId[staticId];
}



