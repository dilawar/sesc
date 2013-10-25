/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Milos Prvulovic
                  James Tuck
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

#include "nanassert.h"

#include "GProcessor.h"
#include "OSSim.h"
#include "SescConf.h"
#include "BPred.h"
#include "ExecutionFlow.h"

#include "mintapi.h"

#ifdef TLS
#include "Epoch.h"
#endif

#ifdef TASKSCALAR
#include "TaskContext.h"
#endif

#ifdef VALUEPRED
#include "ValueTable.h"
#endif

static long long NES=0; // Number of exits pending to scape

int32_t rsesc_usecs(void)
{
  // as in 1000000 clocks per second report (Linux is 100)

  double usecs = 1000000*globalClock;
  usecs /= osSim->getFrequency();
  
  return (int)usecs;
}

/*
 * SESC support severals types of events. To all the events it is
 * possible to pass a pointer to some data in the code. In the
 * original code it's possible to insert the following
 * instrumentation.
 *
 * sesc_preevent(long vaddr, long type, void *sptr);
 *
 * Remember that if you pass an address * through val, it needs to be
 * converted to physical with virt2real. Much better if you use the
 * *sptr.
 */


/**************************************************
 * As soon as the instructions is fetched this event is called. Its
 * called before anything is done inside the sesc simulator.
 */
void rsesc_preevent(int32_t pid, int32_t vaddr, int32_t type, void *sptr)
{
  osSim->preEvent(pid, vaddr, type, sptr);
}

/**************************************************
 * Called when the instruction is dispatched. The instruction has no
 * dependence with any previous instructions. Internally in the sesc
 * the event takes resources in the same way that a NOP (consumes
 * fetch, issue, dispatch width, and occupy an instruction window
 * slot). It also enforces the dependences for the registers R4, R6.
 */
void rsesc_postevent(int32_t pid, int32_t vaddr, int32_t type, void *sptr)
{
  postEventCB *cb = postEventCB::create(osSim, pid, vaddr, type, sptr);
    
  osSim->pid2GProcessor(pid)->addEvent(PostEvent, cb, 0);
}

/**************************************************
 * Called when the instruction is dispatched. It's the same that
 * sesc_postevent, with the difference that it is not dispatched until
 * all the previous memory operations have been performed.
 */
void rsesc_memfence(int32_t pid, int32_t vaddr)
{
  osSim->pid2GProcessor(pid)->addEvent(MemFenceEvent, 0, vaddr);
}


/**************************************************
 * Release consistency acquire. The vaddr is the address inside the
 * simulator. If you want to access the data you must call
 * virt2real(vaddr). Accessing the data, you must be careful, so that
 * the ENDIAN is respected (this is a problem in little endian
 * machines like x86)
 */
void rsesc_acquire(int32_t pid, int32_t vaddr)
{
#ifndef JOE_MUST_DO_IT_OR_ELSE
  return;
#endif
  osSim->pid2GProcessor(pid)->addEvent(AcquireEvent, 0, vaddr);
}

/**************************************************
 * Release consistency release. The vaddr is the address inside the
 * simulator. If you want to access the data you must call
 * virt2real(vaddr). Accessing the data, you must be careful, so that
 * the ENDIAN is respected (this is a problem in little endian
 * machines like x86)
 */
void rsesc_release(int32_t pid, int32_t vaddr)
{
#ifndef JOE_MUST_DO_IT_OR_ELSE
  return;
#endif
  osSim->pid2GProcessor(pid)->addEvent(ReleaseEvent, 0, vaddr);
}

/**************************************************
 * Called each time that mint has created a new thread. Currently only
 * spawn is supported.
 *
 * pid is the Thread[pid] where the new context have been created.ExecutionFlow
 * constructor must use this parameter. ppid is the parent thread
 */
#if (!defined TASKSCALAR)
void rsesc_spawn(int32_t ppid, int32_t cpid, int32_t flags)
{
  osSim->eventSpawn(ppid, cpid, flags);
}

int32_t rsesc_exit(int32_t cpid, int32_t err)
{
  osSim->eventExit(cpid, err);
  return 1;
}
#endif

void rsesc_finish_err(int32_t pid, int err)
{
  ProcessId::destroyAll();

  osSim->stopSimulation();
  osSim->simFinish();

  exit(err);
}

void rsesc_finish(int32_t pid) {
  rsesc_finish_err(pid,0);
}

/* Same as rsesc_spawn, except that the new thread does not become ready */
void rsesc_spawn_stopped(int32_t cpid, int32_t pid, int32_t flags){
  osSim->eventSpawn(cpid, pid, flags, true);
}

/* Returns the number of CPUs (cores) the system has */ 
int32_t rsesc_get_num_cpus(void){
  return osSim->getNumCPUs();
}

void rsesc_sysconf(int32_t cpid, int32_t pid, int32_t flags)
{
  osSim->eventSysconf(cpid, pid, flags);
}

void rsesc_wait(int32_t cpid)
{
  osSim->eventWait(cpid);
}

int32_t rsesc_pseudoreset_cnt = 0;

void rsesc_pseudoreset(int32_t pid)
{ 
  if (pid==0 && rsesc_pseudoreset_cnt==0)
    {
      rsesc_pseudoreset_cnt++;
      
      osSim->pseudoReset();
    }
  else if (pid==0 && rsesc_pseudoreset_cnt > 0) {
    GLOG(1,"rsesc_pseudoreset called multiple times.");
  }
}

int32_t rsesc_suspend(int32_t cpid, int32_t pid)
{
  return osSim->eventSuspend(cpid,pid);
}

int32_t rsesc_resume(int32_t cpid, int32_t pid)
{
  return osSim->eventResume(cpid, pid);
}

int32_t rsesc_yield(int32_t cpid, int32_t pid)
{
  return osSim->eventYield(cpid,pid);
}

void rsesc_fast_sim_begin(int32_t pid)
{
  LOG("Begin Rabbit mode (embeded)");
  osSim->pid2GProcessor(pid)->addEvent(FastSimBeginEvent, 0, 0);
}

void mint_termination_err(int32_t pid, int err)
{
  LOG("mint_termination(%d) received (NES=%lld)\n", pid, NES);

  if (GFlow::isGoingRabbit()) {
#ifdef TS_PROFILING  
    osSim->getProfiler()->recTermination(pid);
#endif    

    Report::field("OSSim:rabbit2=%lld", ExecutionFlow::getnExecRabbit());
    osSim->report("Rabbit--Final");
  }else
    rsesc_finish_err(pid, err);

  exit(0);
}

void mint_termination(int32_t pid) {
  mint_termination_err(pid,0);
}

void rsesc_simulation_mark(int32_t pid)
{
  if (GFlow::isGoingRabbit()) {
    MSG("sesc_simulation_mark %d (rabbit) inst=%lld", osSim->getSimulationMark(), GFlow::getnExecRabbit());
    GFlow::dump();
  }else
    MSG("sesc_simulation_mark %d (simulated) @%lld", osSim->getSimulationMark(), globalClock);

  osSim->eventSimulationMark();

#ifdef TS_PROFILING
  if (ExecutionFlow::isGoingRabbit()) {
    if (osSim->enoughMarks1() && osSim->getProfiler()->notStart()) {
      osSim->getProfiler()->recStartInst();
      osSim->getProfiler()->recInitial(pid);
    }  
  }
#endif  

  if (osSim->enoughMarks2()) {
    mint_termination(pid);
  }
}

void rsesc_simulation_mark_id(int32_t pid, int32_t id)
{
  if (GFlow::isGoingRabbit()) {
    MSG("sesc_simulation_mark(%d) %d (rabbit) inst=%lld", id, osSim->getSimulationMark(id), GFlow::getnExecRabbit());
    GFlow::dump();
  }else
    MSG("sesc_simulation_mark(%d) %d (simulated) @%lld", id, osSim->getSimulationMark(id), globalClock);

  osSim->eventSimulationMark(id,pid);

#ifdef TS_PROFILING
  if (ExecutionFlow::isGoingRabbit()) {
    if (osSim->enoughMarks1() && osSim->getProfiler()->notStart()) {
      osSim->getProfiler()->recStartInst();
      osSim->getProfiler()->recInitial(pid);
    }  
  }
#endif  

  if (osSim->enoughMarks2(id)) {
    mint_termination(pid);
  }
}

void rsesc_fast_sim_end(int32_t pid)
{
  osSim->pid2GProcessor(pid)->addEvent(FastSimEndEvent, 0, 0);
  LOG("End Rabbit mode (embeded)");
}

int32_t rsesc_fetch_op(int32_t pid, enum FetchOpType op, int32_t vaddr, int32_t *data, int32_t val)
{
  I(vaddr);

  osSim->pid2GProcessor(pid)->addEvent(FetchOpEvent, 0, vaddr);
  int32_t odata = SWAP_WORD(*data);

  if (odata)
    osSim->pid2GProcessor(pid)->nLockContCycles.inc();

  switch(op){
  case FetchIncOp:
    *data = SWAP_WORD(odata+1);
    break;
  case FetchDecOp:
    *data = SWAP_WORD(odata-1);
    break;
  case FetchSwapOp:
    *data = SWAP_WORD(val);
    break;
  default:
    MSG("sesc_fetch_op(%d,0x%p,%d) has invalid Op",(int)op,data,val);
    exit(-1);
  }

/*  MSG("%d. sesc_fetch_op: %d@0x%lx [%d]->[%d] ",pid, (int)op,addr, odata,SWAP_WORD(*data)); */
 
  return odata;
}

void rsesc_do_unlock(int* data, int32_t val)
{
  I(data);
  *data = SWAP_WORD(val);
}

typedef CallbackFunction2<int*, int32_t, &rsesc_do_unlock> do_unlockCB;

void rsesc_unlock_op(int32_t pid, int32_t vaddr, int32_t *data, int32_t val)
{
  I(vaddr);
  osSim->pid2GProcessor(pid)->addEvent(UnlockEvent, 
                                       do_unlockCB::create(data, val), vaddr);
  osSim->pid2GProcessor(pid)->nLocks.inc();
}

ThreadContext *rsesc_get_thread_context(int32_t pid)
{
  return osSim->getContext(pid);
}

#if !(defined MIPS_EMUL)
void rsesc_set_instruction_pointer(int32_t pid, icode_ptr picode)
{
  osSim->eventSetInstructionPointer(pid,picode);
}

icode_ptr rsesc_get_instruction_pointer(int32_t pid)
{
  return osSim->eventGetInstructionPointer(pid);
}
#endif // For !(defined MIPS_EMUL)

void rsesc_spawn_opcode(int32_t pid, const int32_t *params, int32_t nParams)
{
  osSim->eventSpawnOpcode(pid,params,nParams);
}

void rsesc_fatal(void)
{
  MSG("Dumping Partial Statistics from rsesc_fatal!");
  osSim->reportOnTheFly();
}

#if (defined TLS)
  
  // Transitions from normal execution to epoch-based execution
//   void rsesc_begin_epochs(Pid_t pid){
//     tls::Epoch *iniEpoch=tls::Epoch::initialEpoch(static_cast<tls::ThreadID>(pid));
//     iniEpoch->run();
//   }
  
  // Creates a sequential successor epoch by cloning current epoch
  // Return value in simulated execution is similar to fork:
  // Successor epoch returns with return value 0
  // Calling epoch returns with return value that is context ID of successor
  void rsesc_future_epoch(Pid_t oldPid){
    tls::Epoch *oldEpoch=tls::Epoch::getEpoch(oldPid);
    I(oldEpoch);
    // Set the return value to 0, then create the new epoch with that
    ThreadContext *oldContext=osSim->getContext(oldPid);
    oldContext->reg[2]=0;
    tls::Epoch *newEpoch=oldEpoch->spawnEpoch();
    Pid_t newPid=newEpoch->getPid();
    // Set the return value of the old epoch to the SESC pid of the new epoch
    oldContext=osSim->getContext(oldPid);
    oldContext->reg[2]=newPid;
    // Run the new epoch
    newEpoch->run();
  }

  void rsesc_future_epoch_jump(Pid_t oldPid, icode_ptr jump_icode){
    // Save the actual instruction pointer of the old epoch
    icode_ptr currInstrPtr=osSim->eventGetInstructionPointer(oldPid);
    // Create the new epoch with the wanted instruction pointer
    osSim->eventSetInstructionPointer(oldPid,jump_icode);
    tls::Epoch *oldEpoch=tls::Epoch::getEpoch(oldPid);
    I(oldEpoch);
    tls::Epoch *newEpoch=oldEpoch->spawnEpoch();
    // Restore the old epoch's instruction pointer
    osSim->eventSetInstructionPointer(oldPid,currInstrPtr);
    // Run the new epoch
    newEpoch->run();
  }
  
  void rsesc_nonspec_epoch(Pid_t pid){
    tls::Epoch *epoch=tls::Epoch::getEpoch(pid);
    if(epoch){
      I(0);
    }
  }

  // Ends an epoch for which a future has already been created
//   void rsesc_commit_epoch(Pid_t oldPid){
//     tls::Epoch *epoch=tls::Epoch::getEpoch(oldPid);
//     if(epoch)
//       epoch->complete();
//   }
  
  // Transitions from one epoch into its (newly created) sequential successor
//   void rsesc_change_epoch(Pid_t pid){
//     tls::Epoch *oldEpoch=tls::Epoch::getEpoch(pid);
//     if(oldEpoch)
//       oldEpoch->changeEpoch();
//   }
  
  // Transitions from epoch-based into normal execution
//   void rsesc_end_epochs(Pid_t pid){
//     tls::Epoch *epoch=tls::Epoch::getEpoch(pid);
//     if(epoch)
//       epoch->complete();
//   }
  
void *rsesc_OS_read(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags) {

  ThreadContext *pthread = ThreadContext::getContext(pid);
  I((flags & E_WRITE) == 0);
  flags = (E_READ | flags); 
  tls::Epoch *epoch=tls::Epoch::getEpoch(pid);
  if(epoch)
    return (void *)(epoch->read(iAddr, flags, vaddr, pthread->virt2real(vaddr, flags)));

  return (void *)(pthread->virt2real(vaddr, flags));
}

void *rsesc_OS_prewrite(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags) {
  ThreadContext *pthread = ThreadContext::getContext(pid);
  I((flags & E_READ) == 0);
  flags = (E_WRITE | flags); 
  tls::Epoch *epoch=tls::Epoch::getEpoch(pid);
  if(epoch)
    return (void *)(epoch->write(iAddr, flags, vaddr, pthread->virt2real(vaddr, flags)));

  return (void *)(pthread->virt2real(vaddr, flags));
}

void rsesc_OS_postwrite(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags) {
  // Do nothing
}
  
#endif
  
#ifdef VALUEPRED
int32_t rsesc_get_last_value(int32_t pid, int32_t index)
{
  if (!osSim->enoughMarks1())
    return 0;

  return ValueTable::readLVPredictor(index);
}

void rsesc_put_last_value(int32_t pid, int32_t index, int32_t val)
{
  if (!osSim->enoughMarks1())
    return;

  ValueTable::updateLVPredictor(index, val);
}

int32_t rsesc_get_stride_value(int32_t pid, int32_t index)
{
  if (!osSim->enoughMarks1())
    return 0;

  return ValueTable::readSVPredictor(index);
}

void rsesc_put_stride_value(int32_t pid, int32_t index, int32_t val)
{
  if (!osSim->enoughMarks1())
    return;

  ValueTable::updateSVPredictor(index, val);
}

int32_t rsesc_get_incr_value(int32_t pid, int32_t index, int32_t lval)
{
  if (!osSim->enoughMarks1())
    return 0;

  return ValueTable::readIncrPredictor(index, lval);
}

void  rsesc_put_incr_value(int32_t pid, int32_t index, int32_t incr)
{
  if (!osSim->enoughMarks1())
    return;

  ValueTable::updateIncrPredictor(index, incr);
}

void  rsesc_verify_value(int32_t pid, int32_t rval, int32_t pval)
{
  if (!osSim->enoughMarks1())
    return;

  ValueTable::verifyValue(rval, pval);
}
#endif

#if (defined TASKSCALAR) || (defined TLS)
// Memory protection subroutines. Those are invoqued by mint (mainly subst.c) to
// access/update data that has been versioned

// Copy data from srcStart (real) to version memory (logical can generate squash/restart)
void rsesc_OS_write_block(int32_t pid, int32_t iAddr, VAddr dstStart, RAddr rsrcStart, size_t size) {

  RAddr rsrc = rsrcStart;
  VAddr dst  = dstStart;
  VAddr end  = dstStart + size;
  
  // BYTE copy (8 bits)
  while(dst < end) {
    uint8_t *rsrc_addr = (uint8_t*)rsrc;
    uint8_t *dst_addr  = static_cast<uint8_t*>(rsesc_OS_prewrite(pid, iAddr, dst, E_BYTE));

    *dst_addr = *rsrc_addr;
    rsesc_OS_postwrite(pid, iAddr, dst, E_BYTE);

    rsrc++;
    dst++;
  }
}
void rsesc_OS_write_block(int32_t pid, int32_t iAddr, VAddr dstStart, const void *srcStart, size_t size) {
  return rsesc_OS_write_block(pid, iAddr, dstStart, (RAddr)srcStart, size);
}

// Copy data from srcStart (logical can generate squash/restart) to rdstStart (real)
bool rsesc_OS_read_string(int32_t pid, VAddr iAddr, RAddr rdstStart, VAddr srcStart, size_t size) {

  VAddr src  = srcStart;
  RAddr rdst = rdstStart;
  RAddr rend = rdstStart + size;

  while(rdst < rend) {
    uint8_t *src_addr  = static_cast<uint8_t*>(rsesc_OS_read(pid, iAddr, src, E_BYTE));
    uint8_t *rdst_addr = (uint8_t*)rdst;

    *rdst_addr = *src_addr;

    if(*src_addr==0)
      return true;

    src++;
    rdst++;
  }

  // String does not fit in dst buffer
  return false;
}
bool rsesc_OS_read_string(int32_t pid, VAddr iAddr, void *dstStart, VAddr srcStart, size_t size) {
  return rsesc_OS_read_string(pid, iAddr, (RAddr)dstStart, srcStart, size);
}

// Copy from srcStart (Logical) to dstStart (Real). dstStart should be some
// stack allocated memory that is discarded briefly after. The reason is that if
// a restart is generated, it can not recopy the data.
//
// TODO: Maybe it should not track the read (screw the restart!)
void rsesc_OS_read_block(int32_t pid, int32_t iAddr, RAddr rdstStart, VAddr srcStart, size_t size) {

  VAddr src  = srcStart;
  RAddr rdst = rdstStart;
  RAddr rend = rdstStart + size;

  while(rdst < rend) {
    uint8_t *src_addr  = static_cast<uint8_t*>(rsesc_OS_read(pid, iAddr, src, E_BYTE));
    uint8_t *rdst_addr = (uint8_t*)rdst;

    *rdst_addr = *src_addr;

    src++;
    rdst++;
  }
}
void rsesc_OS_read_block(int32_t pid, int32_t iAddr, void *dstStart, VAddr srcStart, size_t size) {
  return rsesc_OS_read_block(pid, iAddr, (RAddr)dstStart, srcStart, size);
}
#endif // END of either TaskScalar or VersionMem or TLS

/****************** TaskScalar *******************/


#ifdef TASKSCALAR

#if (defined TLS)
#error "Taskscalar is incompatible with TLS"
#endif

void rsesc_exception(int32_t pid)
{
  if (ExecutionFlow::isGoingRabbit()) {
    MSG("exception in rabbit mode pc=0x%x r31=0x%x"
        ,(int)osSim->eventGetInstructionPointer(pid)->addr, (int)osSim->getContextRegister(pid,31));
    exit(0);
  }

  I(pid>=0);
  TaskContext *tc =TaskContext::getTaskContext(pid);
  I(tc);
  I(!tc->getVersionRef()->isSafe());
  GMSG(tc->getVersionRef()->isSafe(),
       "(failed) safe thread exception. stopping thread. pid = %d. Iaddr=0x%08lx"
       , pid, osSim->eventGetInstructionPointer(pid)->addr);

  tc->exception();
}

void rsesc_spawn(int32_t ppid, int32_t cpid, int32_t flags)
{
  //if (ExecutionFlow::isGoingRabbit()) {
  //  MSG("spawn not supported in rabbit mode. Sorry");
  //  exit(-1);
  //}

#ifdef TC_PARTIALORDER
  osSim->eventSpawn(ppid, cpid, flags);
  TaskContext::normalForkNewDomain(cpid);
#else
  // New thread must share the same TaskContext
  TaskContext::getTaskContext(ppid)->normalFork(cpid);
  osSim->eventSpawn(ppid, cpid, flags);
#endif
}

int32_t rsesc_exit(int32_t cpid, int32_t err)
{
  if (ExecutionFlow::isGoingRabbit()) {
    MSG("Real exit called (NES=%lld) pc=0x%x", NES, (int)osSim->getContextRegister(cpid,31));
    exit(0);
    return 1;
  }

  TaskContext *tc=TaskContext::getTaskContext(cpid);
  if (tc) {
    tc->endTaskExecuted(cpid);
    return 0; // Do not recycle the Thread[cpid]
  }else
    osSim->eventExit(cpid, err);
  return 1;
}

int32_t rsesc_version_can_exit(int32_t pid)
{
  if (ExecutionFlow::isGoingRabbit()) {
    GLOG(DEBUG2, "can_exit (NES=%lld) pc=0x%x", NES, (int)osSim->getContextRegister(pid,31));
    if (NES > 0) {
      NES--;
      return 1;
    }
    return 0;
  }
  // 0 means proceed normal exit code (call rsesc_exit)
  // 1 means ignore exit
  TaskContext *tc=TaskContext::getTaskContext(pid);
  if (tc)
    return tc->ignoreExit();

  return 0; // Proceed normal exit
}

void rsesc_prof_commit(int32_t pid, int32_t tid)
{
#ifdef TS_PROFILING
  if (ExecutionFlow::isGoingRabbit()) {
    osSim->getProfiler()->recCommit(pid, tid);
  }
#endif
}

void rsesc_fork_successor(int32_t ppid, int32_t where, int32_t tid)
{
  if (ExecutionFlow::isGoingRabbit()) {
#ifdef TS_PROFILING  
    if (osSim->enoughMarks1())
      osSim->getProfiler()->recSpawn(tid);
#endif      

    ThreadContext *pthread=osSim->getContext(ppid);
    GLOG(DEBUG2, "fork_successor (NES=%lld) pc=0x%x", NES, (int)pthread->reg[31]);
    NES++;
    pthread->reg[2]= -1;
    return;
  }

  TaskContext *pTC = TaskContext::getTaskContext(ppid);
  I(pTC);
  
  bool canNotSpawn = pTC->canMergeNext();
#ifdef TS_MERGENEXT
  if(canNotSpawn) {
    ThreadContext *pthread=osSim->getContext(ppid);
    pthread->reg[2]= -1;
    return;
  }
#endif

  ThreadContext *parentThread=osSim->getContext(ppid);

  int32_t childPid=mint_sesc_create_clone(parentThread); // Child

  ThreadContext *childThread=osSim->getContext(childPid);

  childThread->reg[2]  =0;
  parentThread->reg[2] =childPid;

  // Swallow the first two (should not exist) instructions from the thread (branch + nop)

  for(int32_t i=0; i<0; i++) {
    int32_t iAddr = childThread->getPicode()->addr;
    I( ((childThread->getPicode()->opflags)&E_MEM_REF) == 0);
    do{
      childThread->setPicode(childThread->getPicode()->func(childThread->getPicode(), childThread));
    }while(childThread->getPicode()->addr==iAddr);
  }

  LOG("@%lld fork_successor ppid %d child %d (r31=0x%08x)", globalClock, ppid, childPid, parentThread->reg[31]);

  pTC->spawnSuccessor(childPid, parentThread->reg[31], tid); //tid is the static task id

  // The child was forked stopped, it can resume now 
  osSim->unstop(childPid);

#ifdef TS_MERGENEXT
  I(!canNotSpawn);
#else
  if (canNotSpawn) {
    TaskContext *childTC = TaskContext::getTaskContext(childPid);
    I(childTC);
    childTC->syncBecomeSafe(true);
  }
#endif

#if 0 // def SPREAD_SPAWNING
  ProcessId *proc = ProcessId::getProcessId(childPid);
  
  uint16_t r = (uint16_t)pTC->getSpawnAddr();
  r = (r>>3) ^ (r>>5);
  r = r % (osSim->getNumCPUs()+1);
  proc->sysconf(SESC_FLAG_MAP| r);
  // proc->sysconf(SESC_FLAG_MAP| 0);
#endif
}

void rsesc_become_safe(int32_t pid)
{
  if (ExecutionFlow::isGoingRabbit())
    return;

  LOG("@%lld become_safe for thread %d",globalClock, pid);

  // Itself deciding to become safe sycall otherwise rsesc_exception
  TaskContext *tc = TaskContext::getTaskContext(pid);
  I(tc);
  
  tc->syncBecomeSafe();
}

bool rsesc_is_safe(int32_t pid)
{
  if (ExecutionFlow::isGoingRabbit())
    return true;

  return TaskContext::getTaskContext(pid)->getVersionRef()->isSafe();
}

ID(static bool preWriteCalled=false);

void *rsesc_OS_prewrite(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags)
{
  ThreadContext *pthread = ThreadContext::getContext(pid);

  I(!preWriteCalled);
  IS(preWriteCalled=true);
  I((flags & E_READ) == 0);
  flags = (E_WRITE | flags); 

  if (ExecutionFlow::isGoingRabbit())
    return (void *)pthread->virt2real(vaddr, flags);

  return (void *)TaskContext::getTaskContext(pid)->preWrite(pthread->virt2real(vaddr, flags));
}

void rsesc_OS_postwrite(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags)
{
  I(preWriteCalled);
  IS(preWriteCalled=false);

  I((flags & E_READ) == 0);
  flags = (E_WRITE | flags); 

  if (ExecutionFlow::isGoingRabbit())
    return;

  ThreadContext *pthread = ThreadContext::getContext(pid);

  TaskContext::getTaskContext(pid)->postWrite(iAddr, flags, pthread->virt2real(vaddr, flags));
}

void *rsesc_OS_read(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags)
{
  I((flags & E_WRITE) == 0);
  flags = (E_READ | flags); 

  ThreadContext *pthread = ThreadContext::getContext(pid);

  if (ExecutionFlow::isGoingRabbit())
    return (void *)pthread->virt2real(vaddr, flags);

  return (void *)TaskContext::getTaskContext(pid)->read(iAddr, flags, pthread->virt2real(vaddr, flags));
}

int32_t rsesc_is_versioned(int32_t pid)
{
  return ExecutionFlow::isGoingRabbit()? 0 : 1;
}


#endif // TASKSCALAR

#ifdef SESC_LOCKPROFILE
static GStatsCntr* lockTime = 0;
static GStatsCntr* lockCount = 0;
static GStatsCntr* lockOccTime = 0;

extern "C" void rsesc_startlock(int32_t pid)
{
  if (!lockTime) {
    lockTime    = new GStatsCntr("","LOCK:Time");
    lockOccTime = new GStatsCntr("","LOCK:OccTime");
    lockCount   = new GStatsCntr("","LOCK:Count");
  }
  lockTime->add(-globalClock);
  lockCount->inc();
}

extern "C" void rsesc_endlock(int32_t pid)
{
  lockTime->add(globalClock);
}

extern "C" void rsesc_startlock2(int32_t pid)
{
  lockOccTime->add(-globalClock);
}

extern "C" void rsesc_endlock2(int32_t pid)
{
  lockOccTime->add(globalClock);
}

#endif

