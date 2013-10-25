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

#include "QEMUFlow.h"
#include "OSSim.h"
#include "SescConf.h"

#include "qemu_sesc.h"

QEMUFlow::QEMUFlow(int32_t cId, int32_t i, GMemorySystem *gms) 
  : GFlow(i, cId, gms)
{
#if (defined(TASKSCALAR) || defined(SESC_MISPATH))
  MSG("QEMUFlow::TASKSCALAR or SESC_MISPATH not supported yet");
  exit(-5);
#endif

  delayDInst        = 0;
  swappingDelaySlot = false;

  n_te     = 0;
  index_te = 0;
}

DInst *QEMUFlow::executePC() 
{ 
  if (delayDInst) {
    DInst *dinst = delayDInst;
    delayDInst = 0;
    return dinst;
  }

  static struct qemu_sesc_te te[QEMU_SESC_INST_SIZE+1];

  int32_t cpuid = 0; // FIXME:

  if (index_te >= n_te) {
    n_te = sesc_cmd_advance_pc(te, cpuid); //cpuid =0
    index_te =0;
  }
  
  Instruction *instr = new Instruction(); // FIXME: it would be good to use a pool
  if(instr == 0)
    return 0;

  if (te[index_te].finalize)
    sesc_client_finish();
  
  Instruction::QemuSparcDecodeInstruction(instr, &(te[index_te].predec));
  DInst *dinst=DInst::createDInst(instr, te[index_te].dAddr ,0); // FIXME: cpuId=0
  index_te++;
  if(dinst == 0) { // end of trace
    MSG("This was the end of the trace!!!!!!");
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

InstID QEMUFlow::getNextID() const {
  return 0; // FIXME: pass next instID
}

void QEMUFlow::dump(const char *str) const {
  MSG("QEMUFlow::dump() not implemented. stop being lazy and do it!");
}

