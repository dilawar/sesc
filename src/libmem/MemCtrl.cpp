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
#include "SescConf.h"
#include "Snippets.h"
#include "MemCtrl.h"

const char *k_rowDelay="rowDelay", *k_dataXfer="dataXfer",
  *k_pagesPerBank="pagesPerBank", *k_numBanks="numBanks",
  *k_numChips="numChips";

MemCtrl::MemCtrl(MemorySystem* current, const char *device_descr_section,
		 const char *device_name)
  : MemObj(device_descr_section, device_name),
    busyUntil(0),
    chWait("%s:chWait", device_name)
{
  SescConf->isInt(device_descr_section, k_rowDelay);
  SescConf->isInt(device_descr_section, k_dataXfer);
  SescConf->isInt(device_descr_section, k_pagesPerBank);
  SescConf->isInt(device_descr_section, k_numBanks);
  SescConf->isInt(device_descr_section, k_numChips);

  SescConf->isGT(device_descr_section, k_pagesPerBank, 0);
  SescConf->isGT(device_descr_section, k_numBanks, 0);
  SescConf->isGT(device_descr_section, k_numChips, 0);

  rowAccessDelay = SescConf->getInt(device_descr_section, k_rowDelay);
  hitDelay       = SescConf->getInt(device_descr_section, k_dataXfer);
  pagesPerBank   = SescConf->getInt(device_descr_section, k_pagesPerBank);
  numBanksPerChip= SescConf->getInt(device_descr_section, k_numBanks);
  numChips       = SescConf->getInt(device_descr_section, k_numChips);

  if (!numChips)
    numChips = 1;

  numBanks = numChips * numBanksPerChip;
  I(numBanks);

  I(current);
  for (uint16_t i = 0; i < numBanks; i++)
    addLowerLevel(current->declareMemoryObj(device_descr_section, "bankType"));
}

void MemCtrl::read(MemRequest *mreq)
{
  mreq->goDown(rowAccessDelay, lowerLevel[calcBank(mreq->getPAddr())]);
}

void MemCtrl::returnAccess(MemRequest *mreq)
{
  if (busyUntil <= globalClock)
    busyUntil = globalClock;
  else /* if (((MCStack*) ptask->uptr)->IsNormalRead())  BBF By now they all will be NormalRead... */
    chWait.add(busyUntil - globalClock);

  busyUntil += hitDelay;
  mreq->goUpAbs(busyUntil);
}

void MemCtrl::write(MemRequest *mreq)
{
  mreq->goDown(rowAccessDelay, lowerLevel[calcBank(mreq->getPAddr())]);
} 

void MemCtrl::specialOp(MemRequest *mreq)
{
  MSG("MemCtrl::specialOp called, and not instrumented");
  exit(-1);
}

Bank *MemCtrl::getBank(uint16_t i) const 
{
  return static_cast < Bank * >(lowerLevel[i]);
}

Time_t MemCtrl::getNextFreeCycle() const
{ 
  return busyUntil; 
}

void MemCtrl::access(MemRequest *mreq)
{
  if(mreq->getMemOperation()==MemWrite)
    write(mreq);
  else
    read(mreq);
}

void MemCtrl::invalidate(PAddr addr,ushort size,MemObj *oc)
{ 
  invUpperLevel(addr,size,oc); 
}

bool MemCtrl::canAcceptStore(PAddr addr)
{
  return true;
}
