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

#include "RSTFlow.h"
#include "OSSim.h"
#include "SescConf.h"
#include "RSTReader.h"


char *RSTFlow::traceFile = 0;
RSTReader *RSTFlow::trace = 0;

RSTFlow::RSTFlow(int32_t cId, int32_t i, GMemorySystem *gms) 
  : GFlow(i, cId, gms)
{
#if (defined(TASKSCALAR) || defined(SESC_MISPATH))
  MSG("RSTFlow::TASKSCALAR or SESC_MISPATH not supported yet");
  exit(-5);
#endif

  bool createReader = (!trace);
  // all traceflow instances share the same reader obj
  
  if(createReader) {
    MSG("Using RST trace format");
    trace = new RSTReader(); 

    I(traceFile);

    MSG("RSTFlow::RSTFlow() traceFile=%s", traceFile);
    trace->openTrace(traceFile);
  }
  
  delayDInst        = 0;
  swappingDelaySlot = false;
}

DInst *RSTFlow::executePC() 
{ 
  if (delayDInst) {
    DInst *dinst = delayDInst;
    delayDInst = 0;
    return dinst;
  }

  DInst *dinst = trace->executePC(fid);

  if(dinst == 0) { // end of trace
    return 0;
  }

  const Instruction *inst = dinst->getInst();
  
  // Handle delay slots (delay slot can be a branch!@#!)
  if (inst->isBranch() && !swappingDelaySlot) {
    I(!delayDInst);
    swappingDelaySlot = true;
    DInst *dinstNext = executePC();
    I(!delayDInst);
    delayDInst = dinst;
    swappingDelaySlot = false;
    return dinstNext;
  }

  return dinst;
}

InstID RSTFlow::getNextID() const {
  return trace->currentPC(fid);
}

void RSTFlow::dump(const char *str) const
{
  MSG("RSTFlow::dump() not implemented. stop being lazy and do it!");
}

