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

#ifndef RUNNINGPROCS
#define RUNNINGPROCS

#include <set>

#include "estl.h"
#include "nanassert.h"
#include "ProcessId.h"

class GProcessor;

class RunningProcs {
private:
  bool stayInLoop;
  size_t startProc;

  typedef std::vector<GProcessor *> GProcCont;
  GProcCont  workingList;

  GProcessor *currentCPU;
  
  std::vector<GProcessor *> cpuVector;

  // Multiset of processors
  typedef std::multiset<GProcessor *> ProcessorMultiSet;
  // Each processor has one entry for each available flow
  ProcessorMultiSet availableProcessors;

  void workingListRemove(GProcessor *core);
  void workingListAdd(GProcessor *core);
public:
  void makeRunnable(ProcessId *proc);
  void makeNonRunnable(ProcessId *proc);
  void setProcessor(CPU_t cpu, GProcessor *newCore);

  void switchIn(CPU_t id, ProcessId *proc);
  void switchOut(CPU_t id, ProcessId *proc);
  
  GProcessor *getProcessor(CPU_t cpu) const {
    I((CPU_t)cpuVector.size() > cpu);
    return cpuVector[cpu];
  }

  // Returns a processor that has an avaialable flow
  GProcessor *getAvailableProcessor(void);

  size_t size() const { return cpuVector.size();  }
  
  RunningProcs();
  void run();
  void finishWorkNow();
  
  // ugly, but need for TRACE_DRIVEN for now.
  void stopProcessor(CPU_t cpu) {
    GProcessor *core = getProcessor(cpu);
    I(core);
    workingListRemove(core);
  }
  void restartProcessor(CPU_t cpu) {
    GProcessor *core = getProcessor(cpu);
    I(core);
    workingListAdd(core);
  }

  GProcessor *getCurrentCPU() const {
    return currentCPU;
  }

  bool hasWork() const {
    return !workingList.empty();
  }
};

#endif // RUNNINGPROCS
