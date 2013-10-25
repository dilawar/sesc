/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2006 University of Illinois.

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

#ifndef RSTFLOW_H
#define RSTFLOW_H

#include "GFlow.h"
#include "RSTReader.h"

class GMemoryOS;
class GMemorySystem;
class MemObj;

class RSTFlow : public GFlow {
 private:

  // Delay slot handling
  bool swappingDelaySlot;
  DInst *delayDInst;

  static RSTReader *trace;
  static char *traceFile;

 protected:
 public:
  RSTFlow(int32_t cId, int32_t i, GMemorySystem *gms);

  static void setTraceFile(const char *tf) { traceFile = strdup(tf); }
  static const char *getTraceFile() { return traceFile; }
  
  InstID getNextID() const;

  void addEvent(EventType e, CallbackBase *cb, int32_t addr) {
    I(0);
  }
        
  // context-switch not supported in RSTFlow
  ThreadContext *getThreadContext(void) { I(0); return 0; }
  void saveThreadContext(int32_t pid) { I(0); }
  void loadThreadContext(int32_t pid) { I(0); }
  icode_ptr getInstructionPointer(void) { I(0); return 0; }
  void setInstructionPointer(icode_ptr picode) { I(0); }
  void switchIn(int32_t i)  { I(0); }
  void switchOut(int32_t i) { I(0); }

  // lets make the pid the same as the processor id
  // ideally we shold decouple Pid from the flow and sesc from ossim... but fine.
  int32_t currentPid(void) { return fid; }

  DInst *executePC();

  void goRabbitMode(long long n2skip=0) {
    GMSG(!n2skip, "RSTFlow::indefinite rabbit mode not supported yet. ;-P");
    while(n2skip > 0) 
      executePC();
  }

  bool hasWork() const { 
    return trace->hasWork(fid);
  }

  void dump(const char *str) const;
};

#endif
