/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Milos Prvulovic

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

#include "RunningProcs.h"
#include "GProcessor.h"

#ifdef SESC_THERM
#include "ReportTherm.h"
#endif

#ifdef TASKSCALAR
#include "TaskContext.h"
#endif

RunningProcs::RunningProcs()
{
  stayInLoop=true;
}

void RunningProcs::finishWorkNow()
{
  stayInLoop=false;
#ifdef SESC_THERM
  ReportTherm::stopCB();
#endif
  workingList.clear();
  startProc =0;
}

void RunningProcs::workingListRemove(GProcessor *core)
{
  ProcessorMultiSet::iterator availIt=availableProcessors.find(core);
  if (availIt==availableProcessors.end())
    availableProcessors.insert(core);

  if (core->hasWork())
    return;

  bool found=false;
  for(size_t i=0;i<workingList.size();i++) {
    if (workingList[i] == core) {
      found = true;
      continue;
    }else if (found) {
      I(i>0);
      workingList[i-1] = workingList[i];
    }
  }

  workingList.pop_back();
  startProc =0;
  stayInLoop=!workingList.empty();
}

void RunningProcs::workingListAdd(GProcessor *core)
{
  ProcessorMultiSet::iterator availIt=availableProcessors.find(core);
  if (availIt!=availableProcessors.end())
    availableProcessors.erase(availIt);

// TODO: Ein?  I(core->hasWork());

  for(size_t i=0;i<workingList.size();i++) {
    if (workingList[i] == core)
      return;
  }

  workingList.push_back(core);
}

void RunningProcs::run()
{
  I(cpuVector.size() > 0 );

  IS(currentCPU = 0);

  do{
    if ( workingList.empty() ) {
      EventScheduler::advanceClock();
    }

#ifdef TASKSCALAR
    HVersionDomain::tryPropagateSafeTokenAll();
#endif
    
    while (hasWork()) {
      stayInLoop=true;

      startProc = 0;
      printf("Working List Size:%d \n",workingList.size());
      do{
        // Loop duplicated so round-robin fetch starts on different
        // processor each cycle <><> i.e everytime when the fetch is done with all procs, next time priority is given to second in the line processor and fetching starts from there i.e 1-2-3-4 then 2-3-4-1 then 3-4-1-2 and so on

        for(size_t i=startProc ; i < workingList.size() ; i++) {
          if (workingList[i]->hasWork()) {
			#ifdef LF_ENABLED
			  //if ((ProcessId::getProcessId(0)->getNGradInsts())<2) {
			  if (1==1) {

				  currentCPU = workingList[1];   //Selecting the in-order simple processor
				  currentCPU->advanceClock();
			  }
			  else{
				  currentCPU = workingList[0];   //Selecting the SMT processor since SMT gets defined first in mtst1->Gprocessor constructor->osSim->registerProc(this)
				  currentCPU->advanceClock();
			  }
			#else
            currentCPU = workingList[i];
            currentCPU->advanceClock();
			#endif
          }else{
            workingListRemove(workingList[i]);
          }
        }
        for(size_t i=0 ; i < startProc ; i++) {
          if (workingList[i]->hasWork()) {
            #ifdef LF_ENABLED
			  //if ((ProcessId::getProcessId(0)->getNGradInsts())<2) {
			  if (1==1) {

				  currentCPU = workingList[1];   //Selecting the in-order simple processor
				  currentCPU->advanceClock();
			  }
			  else{
				  currentCPU = workingList[0];   //Selecting the SMT processor through workingList[0] since SMT gets defined first by mtst1->Gprocessor constructor->osSim->registerProc(this)
				  currentCPU->advanceClock();
			  }
			#else
            currentCPU = workingList[i];
            currentCPU->advanceClock();
			#endif
          }else{
            workingListRemove(workingList[i]);
          }
        }

        startProc++;
        if (startProc >= workingList.size())
          startProc = 0;

        IS(currentCPU = 0);
        EventScheduler::advanceClock();
      }while(stayInLoop);

#ifdef SESC_THERM
      ReportTherm::stopCB();
#endif
    }
  }while(!EventScheduler::empty());
  
  
}

void RunningProcs::makeRunnable(ProcessId *proc)
{
  // The process must be in the InvalidState
  I(proc->getState()==InvalidState);
  // Now the process is runnable (but still not running)
  ProcessId *victimProc=proc->becomeReady();
  // Get the CPU where the process would like to run
  CPU_t cpu=proc->getCPU();
  // If there is a preferred CPU, try to schedule there
  if(cpu>=0){
    // Get the GProcessor of the CPU
    GProcessor *core=getProcessor(cpu);
    // Are there available flows on this CPU
    if(core->availableFlows()){
      // There is an available flow, grab it
      switchIn(cpu,proc);
      return;
    }
  }
  // Could not run the process on the cpu it wanted
  // If the process is not pinned to that processor, try to find another cpu
  if(!proc->isPinned()){
    // Find an available processor
    GProcessor *core=getAvailableProcessor();
    // If available processor found, run there
    if(core){
      switchIn(core->getId(),proc);
      return;
    }
  }
  // Could not run on an available processor
  // If there is a process to evict, switch it out and switch the new one in its place
  if(victimProc){
    I(victimProc->getState()==RunningState);
    // get the processor where victim process is running
    cpu=victimProc->getCPU();
    switchOut(cpu, victimProc);
    switchIn(cpu,proc);
    victimProc->becomeNonReady();
    makeRunnable(victimProc);
  }
  // No free processor, no process to evict
  // The new process has to wait its turn 
}

void RunningProcs::makeNonRunnable(ProcessId *proc)
{
  // It should still be running or ready
  I((proc->getState()==RunningState)||(proc->getState()==ReadyState));
  // If it is still running, remove it from the processor
  if(proc->getState()==RunningState){
    // Find the CPU where it is running
    CPU_t cpu=proc->getCPU();
    // Remove it from there
    switchOut(cpu, proc);
    // Set the state to InvalidState to make the process non-runnable
    proc->becomeNonReady();
    // Find another process to run on this cpu
    ProcessId *newProc=ProcessId::queueGet(cpu);
    // If a process has been found, switch it in
    if(newProc){
      switchIn(cpu,newProc);
    }
  }else{
    // Just set the state to InvalidState to make it non-runnable
    proc->becomeNonReady();
  }
}

void RunningProcs::setProcessor(CPU_t cpu, GProcessor *newCore)
{
  if(cpu >= (CPU_t)size())
    cpuVector.resize(cpu+1);

  GProcessor *oldCore=cpuVector[cpu];   //Assigning the oldcore pointer to the old cpu in the cpuvector
  cpuVector[cpu]=newCore;

  // Is there was an old core in this slot
  if(oldCore){
    // Erase all instances of the old core from the available multiset
    size_t erased=availableProcessors.erase(oldCore);
    // Check whether we erased the right number of entries
    I(oldCore->getMaxFlows()==erased);
  }

  // If there is a new core in this slot
  if(newCore){
    // Insert the new core into the available mulstiset
    // once for each of its available flows
    for(size_t i=0;i<newCore->getMaxFlows();i++)
      availableProcessors.insert(newCore);
  }

#ifdef TRACE_DRIVEN
  // in trace-driven mode, all processor must have work in the beggining
  workingListAdd(newCore);
#endif
}


void RunningProcs::switchIn(CPU_t id, ProcessId *proc)
{
  GProcessor *core=getProcessor(id);

  workingListAdd(core);

  proc->switchIn(id);
#ifdef TS_STALL  
  core->setStallUntil(globalClock+5);
#endif  
  core->switchIn(proc->getPid()); // Must be the last thing because it can generate a switch
}

void RunningProcs::switchOut(CPU_t id, ProcessId *proc) 
{
  GProcessor *core=getProcessor(id);
  Pid_t pid = proc->getPid();

  proc->switchOut(core->getAndClearnGradInsts(pid),
                  core->getAndClearnWPathInsts(pid));

  core->switchOut(pid);

  workingListRemove(core);
}

GProcessor *RunningProcs::getAvailableProcessor(void)
{
  // Get the first processor in the avaialable multiset
  ProcessorMultiSet::iterator availIt=availableProcessors.begin();

  if(availIt==availableProcessors.end()) {
    //UGLY UGLY fix for the bug, i'll fix it soon. --luis
    for(unsigned cpuId = 0; cpuId < size(); cpuId++) {
      if(getProcessor(cpuId)->availableFlows() > 0)
        return getProcessor(cpuId);
    }
    return 0;
  }

  // SMTs have multiple available processors. Give more priority to
  // empty cpus

  GProcessor *gproc = *availIt;
  
  while ((*availIt)->getMaxFlows() > (*availIt)->availableFlows()) {
    availIt++;
    if (availIt == availableProcessors.end())
      return gproc;
  }

  I(availIt != availableProcessors.end());

  return *availIt;
}

