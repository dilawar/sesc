/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
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

#include <stdio.h>

#include "LDSTBuffer.h"
#include "Instruction.h"
#include "Resource.h"

LDSTBuffer::EntryType      LDSTBuffer::stores;
LDSTBuffer::FenceEntryType LDSTBuffer::fences;

DInst *LDSTBuffer::pendingBarrier=0;

void LDSTBuffer::getFenceEntry(DInst *dinst) 
{
  /* TODO: My suggestion of implementation for whoever is the nice guy to do
   * it (James?) 
   *
   * dinst->getVaddr() has the address of the FetchOp
   *
   * src1 field has the register that holds the address of the lock
   * where the fence is acquired.
   *
   * memFence is a local operation. It is like the MFENCE of Pentium IV. It
   * forces the previous store and load to be performed, and does not let other
   * newer loads or store to be issued. This can be "easily" acomplish by
   * queuing all the loads to the memFence instruction, and delaying the
   * execution of the memFence instruction until retirement (do that by not
   * calling "PWin->simTimeAck(dinst)" until the instruction is retired. Since
   * the store are not "really performed" until retirement there is not need to
   * queue the store.
   *
   * Acquire/Release is a little bit more elaborated. My recomendation is as
   * follows:
   *
   * -When an Acquire is received: 
   *
   * if release pending
   *   Queue the execution of the Acquire to the latest Release.
   * else
   *   Ignore the acquire and assume it executed.
   *
   * If an Acquire is pending (only when there is a Relase pending too), all
   * the memory operations get queued in the Acquire (nothing goes until
   * acquire is executed).
   *
   * When an Acquire is frozen (pending a Release), it would unfreeze its
   * dependents when it is executed.
   *
   * -When a Release is received:
   *
   * A release only queues Releases and Acquires. The Release is executed at
   * retirement (similar way to memFence). Once it is executed (at retirement)
   * it unfreezes (do automatically by simTimeAck) the pending Release and
   * Acquires.
   * 
   */

  const Instruction *inst = dinst->getInst();
  int32_t cid = dinst->getContextId();  // FIXME: Is contextId correct?

  I(inst->isFence());

#ifdef LDSTBUFFER_IGNORE_DEPS
  return;
#endif

  fences[cid] = dinst;
}

void LDSTBuffer::fenceLocallyPerformed(DInst *dinst)
{
  int32_t cid = dinst->getContextId();
  FenceEntryType::iterator it = fences.find(cid);

  if (it->second == dinst)
    fences.erase(it);
}


void LDSTBuffer::getStoreEntry(DInst *dinst) 
{
  I( dinst->getInst()->isStore() );

#ifdef LDSTBUFFER_IGNORE_DEPS
  return;
#endif

  EntryType::iterator sit = stores.find(calcWord(dinst));
  if (sit != stores.end()) {
    DInst *pdinst = sit->second;
    I(pdinst->getInst()->isStore());
    if (!pdinst->hasPending() && dinst->getContextId() == pdinst->getContextId())
      pdinst->setDeadStore();
  }

  stores[calcWord(dinst)] = dinst;
}

void LDSTBuffer::getLoadEntry(DInst *dinst) 
{
  I(dinst->getInst()->isLoad());

#ifdef LDSTBUFFER_IGNORE_DEPS
  return;
#endif
    
  // LOAD
  EntryType::iterator sit = stores.find(calcWord(dinst));
  if (sit == stores.end())
    return;

  DInst *pdinst = sit->second;
  I(pdinst->getInst()->isStore());

#if defined(TASKSCALAR) && !defined(TS_CAVA)
  if (dinst->getVersionRef() != pdinst->getVersionRef())
    return;
#else
  if (dinst->getContextId() != pdinst->getContextId()) {
    // FIXME2: In a context switch the same processor may have two different
    // PIDs

    // Different processor or window. Queue the instruction even if executed
    dinst->setDepsAtRetire();

#ifndef TRACE_DRIVEN
    I(pdinst->getInst()->getAddr() != dinst->getInst()->getAddr()); 
#endif

    pdinst->addFakeSrc(dinst);

    I(calcWord(dinst) == calcWord(pdinst));

#ifdef LOG_FORWARD
    LOG( "FORWARD pc=0x%x [addr=0x%x] (%p)-> pc=0x%x [addr=0x%x] (%p)"
        ,(int)pdinst->getInst()->getAddr() , (int)pdinst->getVaddr(), pdinst
        ,(int)dinst->getInst()->getAddr()  , (int)dinst->getVaddr(), dinst);
#endif

    return;
  }
#endif

#ifndef LDSTQ_FWD
  dinst->setLoadForwarded();
#endif
  if (!pdinst->isExecuted()) {
#ifndef TRACE_DRIVEN
    I(pdinst->getInst()->getAddr() != dinst->getInst()->getAddr());
#endif
    pdinst->addFakeSrc(dinst);
  }
}

void LDSTBuffer::storeLocallyPerformed(DInst *dinst) 
{
  I(dinst->getInst()->isStore());

#ifdef LDSTBUFFER_IGNORE_DEPS
  return;
#endif

  EntryType::iterator sit = stores.find(calcWord(dinst));
  if (sit == stores.end()) 
    return; // accross processors stores can be removed out-of-order
 
  if (sit->second == dinst)
    stores.erase(sit);
}

void LDSTBuffer::dump(const char *str)
{
  EntryType::const_iterator sit;

  fprintf(stderr,"LDSTBuffer %s @%d:",str,(int)globalClock);

  fprintf(stderr,"pendingStores ");
  for( sit = stores.begin();
       sit != stores.end();
       sit++ ) {

    fprintf(stderr,": pc=0x%x, addr=0x%x %d"
            ,(int)(sit->second->getInst()->getAddr())
            ,(int)((sit->first)<<2)
            ,sit->second->getContextId()
      );
  }
  fprintf(stderr,"\n");

}

