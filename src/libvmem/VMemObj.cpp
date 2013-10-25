/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

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

#include "VMemObj.h"
#include "MemRequest.h"
#include "MemorySystem.h"
#include "LVIDTable.h"
#include "TaskContext.h"

uint32_t VMemObj::counter = 0;
ushort VMemObj::lineShift=0;

VMemObj::VMemObj(MemorySystem *gms, const char *section, const char *name)
  : MemObj(section, name)
  , id(counter++) 
{
  MemObj *lower_level = NULL;
  
  I(gms);
  lower_level = gms->declareMemoryObj(section, "lowerLevel");

  if (lower_level != NULL)
    addLowerLevel(lower_level);

  if (lineShift==0) {
    SescConf->isPower2("TaskScalar", "bsize");
    SescConf->isGT("TaskScalar", "bsize", 1);
    int32_t lineSize = SescConf->getInt("TaskScalar", "bsize");
    lineShift = log2i(lineSize);
    I(lineShift < 8);
  }

  I(counter < 32);

  GMSG(!lower_level, "You CAN NOT use a void as lower level for VMemObj\n");
}

void VMemObj::access(MemRequest *mreq) 
{
  // Main entry point for the Version Memory System

  I(isHighestLevel());
  I(mreq->getLVID());

  // iCache requests can generate a non-valid data access (it should
  // be 100% of the time). The i-cache access should NOT go through
  // the version memory (il1 go directly to a non-version cache)
  I(ThreadContext::getMainThreadContext()->isValidDataVAddr(mreq->getVaddr()));

  LVID *lvid = static_cast<LVID *>(mreq->getLVID());

  if(mreq->getSubLVID() != mreq->getSubLVID()) {
    // The instruction is performed with an LVID that already received
    // a restart and it was process. We can just ignore this
    // instruction
    mreq->goUp(0); // Invalid address. Do not waste time
    return;
  }

  // Main API with Processor
  switch(mreq->getMemOperation()) {
  case MemRead:    localRead(mreq);       break;
  case MemWrite:   localWrite(mreq);      break;
  default:
    I(0);
  }
}




