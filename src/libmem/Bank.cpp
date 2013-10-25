/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Basilio Fraguela
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

#include "nanassert.h"

#include "SescConf.h"

#include "Bank.h"

void Bank::initialize_bank(int32_t rb_num, int32_t rb_width, const char *p, 
			   int32_t org, bool pipeline, bool segm)
{
  // the last two parameters are read timing which is not used 
  // the reuse of code is the book keeping for different policy
  // in class Cache. so only interface functions uses are
  // Fill, and HIT
  // org refers to the organization of the sub banks. eg. two segments
  // with four subbank. 0204 and for the number, shared rowbuffer between
  // neighboring banks is "-1"

  isPipe = pipeline;
  isSegm = segm;
  segment = org/100;
  subBank = org%100;

  // non segmented mode = (subBank = 1)
  if (!isSegm) subBank = 1;

  if (rb_num < 0) {
    isSharing = true;
    numRB = -rb_num;
  } else {
    numRB = rb_num;
    isSharing = false;
  }
  
  for (int32_t i = 0; i < segment; i++) {
    segBusy[i] = 0;
    for (int32_t j = 0; j < subBank; j++) {
      bankBusy[i][j] = 0;
      bankPC[i][j] = 0;
      nextRBN[i][j] = 0;
      for (int32_t k = 0; k < numRB; k ++)
	address[i][j][k] = -1;
    }
  }

  rowSize = rb_width/segment/subBank*2;
  active = false;
}

Bank::Bank(MemorySystem* current, const char *device_descr_section,
	   const char *device_name)
  : MemObj(device_descr_section, device_name)
  ,HitDelay(SescConf->getInt(device_descr_section,"RBHit"))
  ,MissDelay(SescConf->getInt(device_descr_section,"RBMiss"))
  ,nWrites("%s:nWrites", device_name)
  ,readHit("%s:readHit", device_name)
  ,readMiss("%s:readMiss", device_name)
  ,nWaits("%s:nWaits", device_name)
  ,bankWait("%s:bankWait", device_name)
  ,busyUntil(0)
{ 

  int32_t rb_num   = SescConf->getInt(device_descr_section, "numRBs");
  int32_t rb_width = SescConf->getInt(device_descr_section, "rowWidth");

  const char *p= SescConf->getCharPtr(device_descr_section, "RBRepl");
  int32_t org      = SescConf->getInt(device_descr_section,"RBOrg");
  bool pipe    = SescConf->getBool(device_descr_section,"RBPipelined");
  bool seg     = SescConf->getBool(device_descr_section,"RBSegmented");

  id=SescConf->getInt(device_descr_section,"RBBank");

  initialize_bank(rb_num, rb_width, p, org, pipe, seg);
}

bool Bank::RBHIT(int32_t addr) const
{ 
  for (int32_t i = 0; i < numRB; i++)
    if (address[SEG(addr)][BANK(addr)][i] == addr / rowSize)
      return true;

  return false;
}

void Bank::DESTROYNEIGHBOR(int32_t seg, int32_t bank, int32_t i)
{
  if (bank != 0)
    address[seg][bank-1][i] = -1;

  if (bank != subBank-1)
    address[seg][bank+1][i] = -1;
}

void Bank::DisableBankTill(Time_t time)
{
  if (busyUntil < time)
    busyUntil = time;
}

void Bank::read(MemRequest *mreq)
{
  bool NR;
  Time_t finalTime;
  Time_t lateTime = 0;

  // address bus is not the bottleneck do not model it
  // but due to LIMIT_DM mode, we need a busyUntil to model this mechanism
  // make it like processor doesn't issue any request before the disabling finishes
  if (globalClock < busyUntil)
    finalTime = busyUntil;
  else
    finalTime = globalClock;

  int32_t addr= mreq->getPAddr();
  int32_t seg = SEG(addr);
  int32_t bank = BANK(addr);

  /* BBF By now they all will be NormalRead...
     NR = ((MCStack*) ptask->uptr)->IsNormalRead(); */
  NR= true; //BBF

  bool rbhit = RBHIT(addr);

  /* the meaning of segBusy is different between two mode: FULL and SEGM in
     SEGM, we assume the segment is fully serial so segBusy[seg] notes the
     earliest transaction you can initiate in segment "seg"; while in FULL mode,
     the segment only shares data buffer and data lines, so segBusy[seg] denotes
     the earliest hit you can have in this segment, thus a miss can not start
     early than segBusy[seg] - (MissDelay - HitDelay)
  */

  if (isPipe) {
    // pipeline
    lateTime = bankBusy[seg][bank];
    if (rbhit && lateTime < segBusy[seg]) lateTime = segBusy[seg];
    else if(!rbhit && lateTime < segBusy[seg] - (MissDelay - HitDelay))
      lateTime = segBusy[seg] - (MissDelay - HitDelay);
  }else 
    lateTime = segBusy[seg];

  if (!rbhit && lateTime < bankPC[seg][bank]) {
    lateTime = bankPC[seg][bank];
  }
	
  if (finalTime < lateTime) {
    if (NR) {
      bankWait.add(lateTime - finalTime);
      nWaits.inc();
    }
    finalTime = lateTime;
  }

  // for fully pipelined version address part (II cycles) are always
  if (rbhit) {
    if (isPipe) segBusy[seg] = bankBusy[seg][bank] = finalTime + II;
    else segBusy[seg] = finalTime + HitDelay;
    finalTime += HitDelay;
  } else {	// rb miss
    bankPC[seg][bank] = finalTime + MissDelay + PC_TIME;
    if (isPipe)
      // assume FIFO order within Segment, but the address part is
      // pipelined
      segBusy[seg] = bankBusy[seg][bank] = 
	finalTime + MissDelay - HitDelay + II;
    else segBusy[seg] = finalTime + MissDelay;
		
    // replacing new row, selecting which one to replace
    int32_t n = nextRBN[seg][bank];
    address[seg][bank][n] = addr / rowSize;
    nextRBN[seg][bank] = (n + 1) % numRB;
    if (isSharing) DESTROYNEIGHBOR(seg, bank, n);
    finalTime += MissDelay;
  }
			
  // only normal read (NR) matters as far as performance is concerned
  // prefetch is also counted for energy consideration
  if (NR) {
    //nReads.inc();
    if (rbhit)  {
      readHit.inc();
    } else {
      readMiss.inc();
    }
  }
  /* else  BBF : No, no: I have said they all are std. Reads by now!
     if(((MCStack*) ptask->uptr)->IsPrefetch()) 
     statistics->inc_bank_pf_access(getId(), rbhit);
     BBF END
     else statistics->inc_bank_or_access(getId(), rbhit);
  */
  mreq->goUpAbs(finalTime);
}

void Bank::write(MemRequest *mreq)
{
  /* We call MaxActiveBank(ptask) to halt any read access to the bank if
     power consumption requires us to do so. It's not clear how to stall
     a write. If we can do it in hardware, then add similar call as the 
     ::Read(..) function. Remember at the end of write and writeback we
     need to regain control to deal with bookkeeping of suspended tasks	*/

  Time_t finalTime, lateTime = 0;
  int32_t addr= mreq->getPAddr();
  int32_t seg = SEG(addr);
  int32_t  bank = BANK(addr);
  bool rbhit = RBHIT(addr);

  //BBF : ? statistics->inc_bank_wr_access(getId(), rbhit);
  if (isPipe) {
    // access within the segment is FIFO 
    lateTime = bankBusy[seg][bank];
    if(lateTime < segBusy[seg] - (MissDelay - HitDelay))
      lateTime = segBusy[seg] - (MissDelay - HitDelay);
  }else lateTime = segBusy[seg]; 	// isPipe == false

  if (lateTime < bankPC[seg][bank]) {
    lateTime = bankPC[seg][bank];
  }
	
  if (globalClock < lateTime)
    finalTime = lateTime;
  else
    finalTime = globalClock;

  nWrites.inc();

  // write always retires to memory cell, which is a full delay
  bankPC[seg][bank] = finalTime + MissDelay + PC_TIME;
  if (isPipe)
    // assume FIFO order within Segment, but the address part is
    // pipelined
    segBusy[seg] = bankBusy[seg][bank] = 
      finalTime + MissDelay - HitDelay + II;
  else segBusy[seg] = finalTime + MissDelay;
	
  // it's not clear if we can avoid distroying the content
  // of row buffer if we have a write buffer that can drive
  // the bit lines.  talking with seung suggests that we
  // have to go through row buffer replacing new row,
  // selecting which one to replace
  int32_t n = nextRBN[seg][bank];
  address[seg][bank][n] = addr;
  nextRBN[seg][bank] = (n + 1) % numRB;
  if (isSharing)
    DESTROYNEIGHBOR(seg, bank, n);
  finalTime += MissDelay;

  mreq->goUpAbs(finalTime);
  //mreq->terminateAbs(finalTime);
}

void Bank::access(MemRequest *mreq) 
{
  MemOperation memOp = mreq->getMemOperation();
  if( memOp == MemWrite )
    write(mreq);
  else
    read(mreq);
}

Time_t Bank::getNextFreeCycle() const
{ 
  return busyUntil; 
}

bool Bank::canAcceptStore(PAddr addr)
{
  return true;
}

void Bank::invalidate(PAddr addr,ushort size,MemObj *oc) 
{ 
  invUpperLevel(addr,size,oc); 
}

