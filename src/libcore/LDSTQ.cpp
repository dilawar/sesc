/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze

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

#include "LDSTQ.h"
#include "GProcessor.h"

LDSTQ::LDSTQ(GProcessor *gp, const int32_t id) 
  :ldldViolations("LDSTQ(%d)_ldldViolations", id)
  ,stldViolations("LDSTQ(%d)_stldViolations", id)
  ,ststViolations("LDSTQ(%d)_ststViolations", id)
  ,stldForwarding("LDSTQ(%d)_stldForwarding", id)
  ,gproc(gp)
{
}

void LDSTQ::insert(DInst *dinst)
{
  I(inflightInsts.find(dinst) == inflightInsts.end());
  instMap[calcWord(dinst)].push_back(dinst);
  
  inflightInsts.insert(dinst);
}

bool LDSTQ::executed(DInst *dinst)
{
  if(dinst->isDeadInst())
    return false;

  bool doReplay = false;
  I(inflightInsts.find(dinst) != inflightInsts.end());

  const Instruction *inst = dinst->getInst();
  AddrDInstQMap::iterator addrIt = instMap.find(calcWord(dinst));
  
  I(addrIt != instMap.end());

  dinst->markResolved();

  bool beforeInst = true;

  DInstQueue::iterator instIt = addrIt->second.end();
  instIt--;
  while(instIt != addrIt->second.begin()) {
    DInst *qdinst = *instIt;
    if(qdinst == dinst) 
      beforeInst = false;

    const Instruction *qinst = qdinst->getInst();

    if(beforeInst && qdinst->isResolved()) {
      if(inst->isLoad() && qinst->isLoad()) {
	ldldViolations.inc();
	doReplay = true;
	if(!dinst->isDeadInst())
	  gproc->replay(qdinst);
      } else if(inst->isStore() && qinst->isStore()) {
	ststViolations.inc();
      } else if(inst->isStore() && qinst->isLoad()) {
	stldViolations.inc();
	doReplay = true;
	if(!dinst->isDeadInst())
	  gproc->replay(qdinst);
      }
    }


    if(!beforeInst && inst->isLoad() 
       && qinst->isStore() && qdinst->isResolved()) {
#ifdef LDSTQ_FWD
      dinst->setLoadForwarded();
#endif
	stldForwarding.inc();
	break; // found if forwarded no need to check the rest of the entries
    }
    
    instIt--;
  }

  return doReplay;
}

void LDSTQ::remove(DInst *dinst)
{
  I(inflightInsts.find(dinst) != inflightInsts.end());
  AddrDInstQMap::iterator addrIt = instMap.find(calcWord(dinst));
  
  I(addrIt != instMap.end());

  DInstQueue::iterator instIt = addrIt->second.begin();
  while(instIt != addrIt->second.end()) {
    if(*instIt == dinst)
      break;
    instIt++;
  }

  I(instIt != addrIt->second.end());
  addrIt->second.erase(instIt);

  if(addrIt->second.size() == 0)
    instMap.erase(addrIt);
  
  inflightInsts.erase(dinst);
}
