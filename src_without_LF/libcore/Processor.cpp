/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  James Tuck
                  Milos Prvulovic
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

#include "SescConf.h"

#include "Processor.h"

#include "Cache.h"
#include "FetchEngine.h"
#include "GMemorySystem.h"
#include "ExecutionFlow.h"
#include "OSSim.h"

Processor::Processor(GMemorySystem *gm, CPU_t i)
  :GProcessor(gm, i, 1)
  ,IFID(i, i, gm, this)
  ,pipeQ(i)
{
  spaceInInstQueue = InstQueueSize;

  bzero(RAT,sizeof(DInst*)*NumArchRegs);

  l1Cache = gm->getDataSource();

}

Processor::~Processor()
{
  // Nothing to do
}

DInst **Processor::getRAT(const int32_t contextId)
{
  I(contextId == Id);
  return RAT;
}

FetchEngine *Processor::currentFlow()
{
  return &IFID;
}

#if !(defined MIPS_EMUL)
void Processor::saveThreadContext(Pid_t pid)
{
  I(IFID.getPid()==pid);
  IFID.saveThreadContext();
}

void Processor::loadThreadContext(Pid_t pid)
{
  I(IFID.getPid()==pid);
  IFID.loadThreadContext();
}

ThreadContext *Processor::getThreadContext(Pid_t pid)
{
  I(IFID.getPid()==pid);
  return IFID.getThreadContext();
}

void Processor::setInstructionPointer(Pid_t pid, icode_ptr picode)
{
  I(IFID.getPid()==pid);
  IFID.setInstructionPointer(picode);
}

icode_ptr Processor::getInstructionPointer(Pid_t pid)
{
  I(IFID.getPid()==pid);
  return IFID.getInstructionPointer();  
}
#endif

void Processor::switchIn(Pid_t pid)
{
  IFID.switchIn(pid);
}

void Processor::switchOut(Pid_t pid)
{
  IFID.switchOut(pid);
}

size_t Processor::availableFlows() const 
{
  return IFID.getPid() < 0 ? 1 : 0;
}

long long Processor::getAndClearnGradInsts(Pid_t pid)
{
  I(IFID.getPid() == pid);
  
  return IFID.getAndClearnGradInsts();
}

long long Processor::getAndClearnWPathInsts(Pid_t pid)
{
  I(IFID.getPid() == pid);
  
  return IFID.getAndClearnWPathInsts();
}

Pid_t Processor::findVictimPid() const
{
  return IFID.getPid();
}

void Processor::goRabbitMode(long long n2Skip)
{
  IFID.goRabbitMode(n2Skip);
}

void Processor::advanceClock()
{
#ifdef TS_STALL
  if (isStall()) return;
#endif  

  clockTicks++;

  //  GMSG(!ROB.empty(),"robTop %d Ul %d Us %d Ub %d",ROB.getIdFromTop(0)
  //       ,unresolvedLoad, unresolvedStore, unresolvedBranch);

  // Fetch Stage
  if (IFID.hasWork() ) {
    IBucket *bucket = pipeQ.pipeLine.newItem();
    if( bucket ) {
      IFID.fetch(bucket);
    }
  }
  
  // ID Stage (insert to instQueue)
  if (spaceInInstQueue >= FetchWidth) {
    IBucket *bucket = pipeQ.pipeLine.nextItem();
    if( bucket ) {
      I(!bucket->empty());
      //      I(bucket->top()->getInst()->getAddr());
      
      spaceInInstQueue -= bucket->size();
      pipeQ.instQueue.push(bucket);
    }else{
      noFetch2.inc();
    }
  }else{
    noFetch.inc();
  }

  // RENAME Stage
  if ( !pipeQ.instQueue.empty() ) {
    spaceInInstQueue += issue(pipeQ);
    //    spaceInInstQueue += issue(pipeQ);
  }
  
  retire();
}


StallCause Processor::addInst(DInst *dinst) 
{
  const Instruction *inst = dinst->getInst();

  if (InOrderCore) {
    if(RAT[inst->getSrc1()] != 0 || RAT[inst->getSrc2()] != 0 
       || RAT[inst->getDest()] != 0
       ) {
      return SmallWinStall;
    }
  }

  StallCause sc = sharedAddInst(dinst);
  if (sc != NoStall)
    return sc;


  I(dinst->getResource() != 0); // Resource::schedule must set the resource field

  if(!dinst->isSrc2Ready()) {
    // It already has a src2 dep. It means that it is solved at
    // retirement (Memory consistency. coherence issues)
    if( RAT[inst->getSrc1()] )
      RAT[inst->getSrc1()]->addSrc1(dinst);
  }else{
    if( RAT[inst->getSrc1()] )
      RAT[inst->getSrc1()]->addSrc1(dinst);

    if( RAT[inst->getSrc2()] )
      RAT[inst->getSrc2()]->addSrc2(dinst);
  }

  dinst->setRATEntry(&RAT[inst->getDest()]);
  RAT[inst->getDest()] = dinst;

  I(dinst->getResource());
  dinst->getResource()->getCluster()->addInst(dinst);

  return NoStall;
}

bool Processor::hasWork() const 
{
  if (IFID.hasWork())
    return true;

  return !ROB.empty() || pipeQ.hasWork();
}

#ifdef SESC_MISPATH
void Processor::misBranchRestore(DInst *dinst)
{
  clusterManager.misBranchRestore();

  for (unsigned i = 0 ; i < INSTRUCTION_MAX_DESTPOOL; i++)
    misRegPool[i] = 0;

  I(dinst->getFetch() == &IFID);

  pipeQ.pipeLine.cleanMark();

  // TODO: try to remove from ROB
}
#endif
