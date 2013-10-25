/*
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  James Tuck
                  Wei Liu
                  Milos Prvulovic
                  Luis Ceze
                  Smruti Sarangi
                  Paul Sack


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

#ifndef _OSSim_H_
#define _OSSim_H_

#include <sys/time.h>

#include <list>
#include <vector>
#include <map>
#include <queue>

#include "nanassert.h"
#include "Snippets.h"

#include "callback.h"

#include "ProcessId.h"
#include "RunningProcs.h"

#ifdef TS_PROFILING
#include "Profile.h"
#endif


#ifdef TS_RISKLOADPROF
#include "RiskLoadProf.h"
#endif

#include "Events.h"

// Interface that the backend should extend for intercepting the user events. It
// also must be extended for changing the mapping policies for the threads. For
// example, SMT and CMP may have different mapping policies.

class GProcessor;

class OSSim {
private:
  static char *benchName;
  char *reportFile;
  char *traceFile;

  char *thermFile;

  timeval stTime;
  double  frequency;

  char *benchRunning;
  char *benchSection;
  bool justTest;

  bool NoMigration; // Configuration option that dissables migration (optional)
  // Number of instructions to skip passed as parameter when the
  // simulation is invoked. -w10000 would skip the first 10000
  // instructions. A cheap way to implement FastSimBegin
  long long nInst2Skip;
  long long nInst2Sim;
  long long nInstCommited2Sim;

  unsigned long long snapshotGlobalClock;

  typedef struct {
    Pid_t pid;
    unsigned long total;
    unsigned long begin;
    unsigned long end;
    bool  mtMarks;
  } SimulationMark_t;

  SimulationMark_t simMarks;
  std::map<int32_t,SimulationMark_t> idSimMarks;

  int32_t numIdSimMarks;
  int32_t waitBeginIdSimMarks;
  int32_t waitEndIdSimMarks;

#ifdef TS_PROFILING
  Profile *profiler;
  int32_t profPhase;
  const char *profSectionName;
#endif


#ifdef TS_RISKLOADPROF
  RiskLoadProf *riskLoadProf;
#endif

protected:
  StaticCallbackMember0<RunningProcs, &RunningProcs::finishWorkNow> finishWorkNowCB;

  void processParams(int32_t argc, char **argv, char **envp);

public:

  RunningProcs cpus;
  OSSim(int32_t argc, char **argv, char **envp);
  virtual ~OSSim();

  void report(const char *str);

  GProcessor *pid2GProcessor(Pid_t pid);
  ProcessIdState getState(Pid_t pid);
  GProcessor *id2GProcessor(CPU_t cpu);

  bool trace() const { return (traceFile != 0); }




  virtual void preEvent(Pid_t pid, int32_t vaddr, int32_t type, void *sptr) {
    MSG("preevent(%d, %d, %p) pid %d", vaddr, type, sptr, pid);
  }

  virtual void postEvent(Pid_t pid, int32_t vaddr, int32_t type, const void *sptr) {
    // Notice that the sptr is a const. This is because the postEvent
    // is notified much latter than it was really executed. If the
    // backend tryies to modify (give data back) to the application
    // through the postEvent, it would NOT WORK. Instead use preEvent
    // to pass data from the backend to the application.
    MSG("postevent(%d, %d, %p) pid %d @%lld", vaddr, type, sptr, pid, (long long)globalClock);
  }

  virtual void memBarrierEvent(Pid_t pid, int32_t vaddr, int32_t type, const void *sptr) {
    MSG("membarrier(%d, %d, %p) pid %d @%lld", vaddr, type, sptr, pid, (long long)globalClock);
  }

  // Those functions are only callable through the events. Not
  // directly inside the simulator.

  virtual void eventSpawnOpcode(int32_t pid, const int32_t *params, int32_t nParams) {
    MSG("spawnOpcode(%p, %d) pid %d", params, nParams, pid);
  }

  // Spawns a new process newPid with given flags
  // If stopped is true, the new process will not be made runnable
  void eventSpawn(Pid_t curPid, Pid_t newPid, int32_t flags, bool stopped=false);
  void eventSysconf(Pid_t curPid, Pid_t targPid, int32_t flags);
  int32_t eventGetconf(Pid_t curPid, Pid_t targPid);
  void eventExit(Pid_t cpid, int32_t err);
  void tryWakeupParent(Pid_t cpid);
  void eventWait(Pid_t cpid);
  int32_t eventSuspend(Pid_t cpid, Pid_t tid);
  int32_t eventResume(Pid_t cpid, Pid_t tid);
  int32_t eventYield(Pid_t cpid, Pid_t yieldID);
  void eventSaveContext(Pid_t pid);
  void eventLoadContext(Pid_t pid);
  void eventSetInstructionPointer(Pid_t pid, icode_ptr picode);
  icode_ptr eventGetInstructionPointer(Pid_t pid);
  void eventSetPPid(Pid_t pid, Pid_t ppid);
  Pid_t eventGetPPid(Pid_t pid);

  void eventSimulationMark() {
    simMarks.total++;
  }
  void eventSimulationMark(int32_t id,Pid_t pid) {
    if(idSimMarks.find(id)==idSimMarks.end()) {
      idSimMarks[id].total = 0;
      idSimMarks[id].begin = 0;
      idSimMarks[id].end = (uint)((~0UL)-1);
    }

    idSimMarks[id].total++;
    idSimMarks[id].pid=pid;
    idSimMarks[id].mtMarks=true;
  }
  uint32_t getSimulationMark() const { return simMarks.total; }
  uint32_t getSimulationMark1() const { return simMarks.begin; }
  uint32_t getSimulationMark2() const { return simMarks.end; }
  // If marks are not used but -w option is, total and begin are both zero
  // and there are never enough marks to exit simulation. I changed the comparison
  // to >= from >. If this does not work, please fix it in a way that does not break
  // goRabbitMode when marks are not used. Thanks, Milos. 
  bool enoughMarks1() const { return simMarks.total >= simMarks.begin; }
  bool enoughMarks2() const { return simMarks.total > simMarks.end; }

  uint32_t getSimulationMark(int32_t id) const {
    std::map<int32_t,SimulationMark_t>::const_iterator it = idSimMarks.find(id);
    return (*it).second.total;
  }
  uint32_t getSimulationMark1(int32_t id) const {
    std::map<int32_t,SimulationMark_t>::const_iterator it = idSimMarks.find(id);
    return (*it).second.begin;
  }
  uint32_t getSimulationMark2(int32_t id) const {
    std::map<int32_t,SimulationMark_t>::const_iterator it = idSimMarks.find(id);
    return (*it).second.end;
  }
  bool enoughMarks1(int32_t id) const {
    std::map<int32_t,SimulationMark_t>::const_iterator it = idSimMarks.find(id);
    return (*it).second.total > (*it).second.begin;
  }
  bool enoughMarks2(int32_t id) const {
    std::map<int32_t,SimulationMark_t>::const_iterator it = idSimMarks.find(id);
    return (*it).second.total > (*it).second.end;
  }
  bool enoughMTMarks1() const {
    std::map<int32_t,SimulationMark_t>::const_iterator it = idSimMarks.begin();

    bool ret=true;
    for(it=idSimMarks.begin(); it!=idSimMarks.end(); it++) {
      ret = (ret && enoughMarks1( (*it).first ));
    }

    return ret;
  }
  bool enoughMTMarks1(int32_t pid,bool justMe) const {
    std::map<int32_t,SimulationMark_t>::const_iterator it = idSimMarks.begin();
    bool me=false;
    bool ret=true;
    for(it=idSimMarks.begin(); it!=idSimMarks.end(); it++) {
      if( (*it).second.pid != pid )
        ret = (ret && enoughMarks1( (*it).first ));
      else if( (*it).second.mtMarks )
        me = enoughMarks1( (*it).first );
    }
    if(justMe)
      ret=me;

    return ret;
  }

#ifdef TS_PROFILING
  Profile *getProfiler() const {
    return profiler;
  }
  int32_t getProfPhase() const {
    return profPhase;
  }
  const char *getProfSectionName() const {
    return profSectionName;
  }
#endif


#ifdef TS_RISKLOADPROF
  RiskLoadProf *getRiskLoadProf() const {
    return riskLoadProf;
  }
#endif

  ThreadContext *getContext(Pid_t pid);

  int32_t getContextRegister(Pid_t pid, int32_t regnum);

  void suspend(Pid_t pid) {
    eventSuspend(-1,pid);
  }

  void resume(Pid_t pid) {
    eventResume(-1,pid);
  }

  // Makes a runnable process stopped
  void stop(Pid_t pid);
  // Makes a stopped process runnable
  void unstop(Pid_t pid);

  // Sets the priority of a process
  void setPriority(Pid_t pid, int32_t newPrio);

  // Returns the current priority of a process
  int32_t getPriority(Pid_t pid);

  // Removes from cpu a running thread (only if necessary), and
  // activates the pid thread.
  Pid_t contextSwitch(CPU_t cpu, Pid_t nPid);

  // Currently, a CPU can never stop to exist (unregister). Maybe,
  // some fault tolerance freak needs this feature. In that case, he/she
  // should implemented it.
  void registerProc(GProcessor *core);
  void unRegisterProc(GProcessor *core);

  static const char *getBenchName(){
    return benchName;
  }

  const char *getReportFileName(){
    return reportFile;
  }

  const char *getBenchSectionName() const {
    return benchSection;
  }

  void initBoot();
  void preBoot();
  void postBoot();

  void simFinish();

  // Boot the whole simulator. Restart is set to true by the exception
  // handler. This may happen in a misspeculation, the simulator would be
  // restarted.
  virtual void boot() {
    initBoot();
    preBoot();
    if(!justTest)
      postBoot();
  }

  void reportOnTheFly(const char *file=0);

  double getFrequency() const { return frequency; }

  size_t getNumCPUs() const { return cpus.size(); }

  void stopSimulation() { cpus.finishWorkNow(); }
  void switchOut(CPU_t id, ProcessId *procId) { cpus.switchOut(id, procId); }

  long long getnInst2Sim() const { return nInst2Sim;  }
  long long getnInst2Skip() const { return nInst2Skip;  }
  long long getnInstCommited2Sim() const { return nInstCommited2Sim; }

  bool hasWork() const { return cpus.hasWork(); }

  void pseudoReset() {snapshotGlobalClock = globalClock;}

  // ugly, but need for TRACE_DRIVEN for now.
  void stopProcessor(CPU_t id) {
    cpus.stopProcessor(id);
  }
  void restartProcessor(CPU_t id) {
    cpus.restartProcessor(id);
  }

};

typedef CallbackMember4<OSSim, Pid_t, int32_t, int32_t, const void *, &OSSim::postEvent> postEventCB;

extern OSSim *osSim;
extern double etop(double energy);
#endif   // OSSim_H
