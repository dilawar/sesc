/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss
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

#include "VCR.h"
#include "VMemReq.h"
#include "MemRequest.h"

pool<VCREntry>      VCREntry::rPool;
pool<VCR::VCRType>  VCR::vcrPool;

/****************************
 *       VCR Entry          *
 ****************************/

VCREntry *VCREntry::create(const VMemState *state, HVersion *version) 
{
  VCREntry *newEntry = rPool.out();

  I(state);
  I(version);
  newEntry->state.copyStateFrom(state);
  newEntry->version = version;
    
  return newEntry;
}

void VCREntry::destroy()
{ 
  version->garbageCollect();
  rPool.in(this); 
}

/****************************
 *           VCR            *
 ****************************/

void VCR::clearVCRMap(VCRType *versionList)
{
  // clean up the version list
  VCRType::iterator sit = versionList->begin();
  for(; sit != versionList->end(); sit++)
    (*sit)->destroy();

  // destroy the version list
  versionList->clear();
  vcrPool.in(versionList);
}

void VCR::addCheck(const VMemWriteReq *oreq, const VMemWriteReq *vreq)
{
  I(oreq->getType() == VWriteCheck);
  I(vreq->getType() == VWriteCheckAck);

  if(vreq->getVersionRef()==0)
    return;
  if (vreq->getVersionRef()->isKilled())
    return;

  // FIXME: add VCR energy
  //
  // VCR should be a small SRAM (direct map) structure with as many
  // entries as caches on the system multiplied by the associativity
  // (note that LMVCache may have different associativity)
  //
  // Additionaly, it also has version comparators (one per word).
  //
  // Caches send request in order, so that they can be locally combine
  // as soon as they arrive

  VCRMap::iterator it = vcrMap.find(oreq);
  VCRType *versionList;
  if(it == vcrMap.end()) {
    versionList = vcrPool.out();
    vcrMap[oreq] = versionList;
  }else{
    versionList = it->second;
  }

#ifdef DEBUG
  // Check that the same versions is not ack twice
  VCRType::iterator sit = versionList->begin();
  for(; sit != versionList->end(); sit++) {
    GI(!(*sit)->getVersionRef()->isKilled()
       ,(*sit)->getVersionRef() != vreq->getVersionRef());
  }
#endif

  VCREntry *entry = VCREntry::create(vreq->getStateRef()
				     ,vreq->getVersionDuplicate());
  versionList->insert(entry);
}

bool VCR::performCheck(const VMemWriteReq *oreq)
{
  VCRMap::iterator vit = vcrMap.find(oreq);
  if(vit == vcrMap.end())
    return true;
  
  VCRType *versionList = vit->second;

  if(versionList->empty()) {
    clearVCRMap(versionList);
    vcrMap.erase(vit);
    return true;
  }

#ifndef TS_IMMEDIAT_RESTART
  // NOTE: Silent stores do not generate restarts, but this check is
  // not included here. It is OK because we use
  // notifyDataDepViolation, if we were calling restart directly it
  // would generate more restarts than necessary.
  PAddr paddr = oreq->getPAddr();

  VMemState::WordMask wrmask = VMemState::calcWordMask(paddr);

  for(VCRType::iterator it = versionList->begin(); it != versionList->end(); it++) {
   const VMemState  *state   = (*it)->getState();

    if(state->hasExposedRead(wrmask)) {
      const HVersion  *version = (*it)->getVersionRef();
      // must be latter version. Otherwise, we shouldn't be here
      I(*(oreq->getVersionRef()) < *version); 
      if(!version->isKilled()) {
	// The store may have an ack, but it still has a pending request
	I(oreq->hasMemRequestPending());
	oreq->getMemRequest()->notifyDataDepViolation();
	break;
      }
    }

    if(state->hasProtectingWrite(wrmask))
      break; // stop looking for more versions
  }
#endif

  // do some housekeeping
  clearVCRMap(versionList);
  vcrMap.erase(vit);

  return false;
}
