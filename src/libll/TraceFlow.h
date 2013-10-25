/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze
                  Jose Renau

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

#ifndef TRACEFLOW_H
#define TRACEFLOW_H

#include "GFlow.h"
#include "TraceReader.h"

class GMemoryOS;
class GMemorySystem;
class MemObj;


enum TraceMode {
  PPCTT6Trace = 0,
  SimicsTrace,
  QemuSpTrace
};

class TraceFlow : public GFlow {
 private:
  bool hasTrace;
  InstID nextPC;

  // Delay slot handling
  bool swappingDelaySlot;
  DInst *delayDInst;

  static TraceReader *trace;
  static char *traceFile;

  TraceMode mode;
  
 protected:
 public:
  TraceFlow(int32_t cId, int32_t i, GMemorySystem *gms);

  static void setTraceFile(const char* tf) { traceFile = strdup(tf); }
  static const char* getTraceFile() { return traceFile; }
  
  InstID getNextID() const {
    return nextPC;
  }

  void addEvent(EventType e, CallbackBase *cb, int32_t addr) {
    I(0);
  }
        
  // context-switch not supported in TraceFlow
  ThreadContext *getThreadContext(void) { I(0); return 0; }
  void saveThreadContext(int32_t pid) { I(0); }
  void loadThreadContext(int32_t pid) { I(0); }
  icode_ptr getInstructionPointer(void) { I(0); return 0; }
  void setInstructionPointer(icode_ptr picode) { I(0); }
  void switchIn(int32_t i)  { I(0); }
  void switchOut(int32_t i) { I(0); }

  // lets make the pid the same as the processor id
  // ideally we shold decouple Pid from the flow and sesc from ossim... but fine.
  int32_t currentPid(void) { return cpuId; }

  DInst *executePC();

  void goRabbitMode(long long n2skip=0) {
    GMSG(!n2skip, "TraceFlow::indefinite rabbit mode not supported yet. ;-P");
    while(n2skip > 0) 
      executePC();
  }

  bool hasWork() const { 
    return (hasTrace || 
            (trace->hasBufferedEntries() &&  // this is weird, but it saves one  
             trace->hasBufferedEntries(cpuId))); // call to an STL method and makes it faster ;-)
  }

  void dump(const char *str) const;
};

#endif
