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

#include "LVIDTable.h"
#include "TaskContext.h"
#include "VMemObj.h"
#include "TaskHandler.h"

/**********************************
 * LVID Entry
 **********************************/

SubLVIDType LVID::nSubLVID=0;

LVID::LVID(const char *section, LVIDTable *m, ushort i)
  : lvidc(m)
    ,Id(i)
    ,shiftId((static_cast<LPAddr>(i))<<32)
    ,randSetId(((static_cast<PAddr>(i))<<8) | ((static_cast<PAddr>(i))<<16))
{
  if (nSubLVID == 0) {
    SescConf->isInt(section    , "nSubLVID");
    SescConf->isBetween(section , "nSubLVID", 1, 32);
    nSubLVID = SescConf->getInt(section, "nSubLVID");
  }

  subLVID  = 0;
  clver    = 0;
}

void LVID::init(HVersion *v)
{
  I(v);
  I(clver == 0);
  
  I(!v->isKilled());
  
  clver = v->duplicate();
  I(clver->getVersionDomain()); //add by hr
  
  subLVID       = 0;
  lvidcRecycled = false;
  nLinesPinned   = 0;
}

const HVersion *LVID::getVersionRef() const 
{
  // invalid has no version associated
  return clver;
}

HVersion *LVID::getVersionDuplicate() const
{
  I(clver);

  return clver->duplicate();
}

void LVID::decLinesUsed() 
{
  I(nLinesPinned>0);
  nLinesPinned--;

  if (!canRecycleLVID())
    return;
  
  I(!lvidcRecycled);
  lvidc->recycleLVID(this);
  lvidcRecycled = true;
  I(clver==0);
}

void LVID::incLinesUsed()
{
  nLinesPinned++;
}

void LVID::localRestart()
{
  if (canRecycleLVID()) {
    // No more cache lines, and no more requests. Just recycle it now
    lvidc->recycleLVID(this);
    lvidcRecycled = true;
    I(clver==0);
    return;
  }
  
  I(clver);
  I(clver->getTaskContext()); // A task can only receive a restart if TaskContext is alive

  if (nLinesPinned == 0)
    return;
  
  subLVID++;

  // nLinesPinned does not change
}

bool LVID::isSafestEntry() const
{
  I(lvidc->getSafestEntry() != this);
  return false;
}

void LVID::garbageCollectVersion()
{
  // Only called from LVIDTable::recycleLVID
  I(nLinesPinned == 0 && !clver->hasOutsReqs());

  I(clver);
  clver->garbageCollect();
  clver = 0;
}

void LVID::garbageCollect()
{
  if (lvidcRecycled)
    return; // already Recycled

  if (nLinesPinned || clver->hasOutsReqs())
    return; // Outstanding requests or cacheLines pinned :(

  lvidc->recycleLVID(this);
  lvidcRecycled = true;
  I(clver==0);
}

/**********************************
 * LVIDSafest
 **********************************/

LVIDSafest::LVIDSafest(const char *section, LVIDTable *m, ushort i)
  : LVID(section, m, i)
{
  nLinesPinned = 1; // So that never would be recycled
}

const HVersion *LVIDSafest::getVersionRef() const 
{
  return clver;
}

HVersion *LVIDSafest::getVersionDuplicate() const 
{
  return clver->duplicate();
}

void LVIDSafest::decLinesUsed()
{
  // TODO?: keep statistics of % of lines in the cache that are Safest
  // Do nothing
}

void LVIDSafest::incLinesUsed()
{
  // TODO?: keep statistics of % of lines in the cache that are Safest
}

bool LVIDSafest::isSafe() const
{
  return true;
}

bool LVIDSafest::isKilled() const
{
  return false;
}

bool LVIDSafest::isFinished() const
{
  return true;
}

void LVIDSafest::garbageCollect()
{
  // Do nothing
}

void LVIDSafest::garbageCollectVersion()
{
  // Do nothing
}


void LVIDSafest::localRestart()
{
  I(0); // Should not happen
}

bool LVIDSafest::isSafestEntry() const
{
  return true;
}


/**********************************
 * LVID Table
 **********************************/

LVIDTable::SystemLVIDs LVIDTable::systemLVIDs;

LVIDTable::LVIDTable(const char *section, const char *n) 
  : name(strdup(n))
  ,nLinesOnKill("%s_LVIDTable_nLinesOnKill", n)
  ,nLinesOnRestart("%s_LVIDTable_nLinesOnRestart", n)
  ,nLinesOnSetSafe("%s_LVIDTable_nLinesOnSetSafe", n)
  ,nLinesOnSetFinished("%s_LVIDTable_nLinesOnSetFinished", n)
{
  SescConf->isInt(section    , "nLVID");
  SescConf->isBetween(section , "nLVID",2 ,32768);
  SescConf->isPower2(section  , "nLVID");
  long nLVID = SescConf->getInt(section, "nLVID");

  lvidSafest = new LVIDSafest(section, this, 0);
    
  for(long i=0;i<nLVID;i++) {
    LVID *lvid = new LVID(section, this, i);
    freeLVIDs.push_front(lvid);
    origLVIDs.push_front(lvid);
  }

  systemLVIDs.push_back(this);
}

LVID *LVIDTable::findCreateLVID(HVersion *ver)
{
  LVID *lvid = findLVID(ver);
  if (lvid) {
    I(!lvid->isKilled());
    return lvid;
  }
  if (freeLVIDs.empty()) {
    // Lazy garbageCollect
    for(LVIDList::iterator it=origLVIDs.begin() ; it != origLVIDs.end() ; it++)
      (*it)->garbageCollect();
  }
  if (freeLVIDs.empty() || ver->isKilled()) {
    I(ver2lvid.find(ver) == ver2lvid.end());
    return 0;
  }
  
  lvid = freeLVIDs.front();
  freeLVIDs.pop_front();

  lvid->init(ver);

  I(ver2lvid.find(lvid->getVersionRef()) == ver2lvid.end());
  
  ver2lvid[lvid->getVersionRef()] = lvid;
  I(!lvid->isKilled());
  
  return lvid;
}

LVID *LVIDTable::findLVID(const HVersion *v)
{
  Ver2LVIDMap::iterator it = ver2lvid.find(v);
  if (it == ver2lvid.end())
    return 0;
  
  return it->second;
}

void LVIDTable::killSpecLVIDs(const HVersion *exeVer)
{
  LVIDList::iterator it;

  MSG("Killing all the cache Versions [%d] @%lld (not enough version)", freeLVIDs.size(), globalClock); 

  for(it=origLVIDs.begin() ; it != origLVIDs.end() ; it++) {
    const HVersion *ver = (*it)->getVersionRef();
    if (ver == 0)
      continue;
    if ( *ver > *exeVer) {
      I(!ver->isSafe());
      taskHandler->kill(ver, false);
      I(ver->isKilled());
    }
  }
}

void LVIDTable::recycleLVID(LVID *lvid)
{
  I(lvid);

#ifdef DEBUG
  {
    LVIDList::iterator it;
  
    // do not recycle twice
    for(it=freeLVIDs.begin() ; it != freeLVIDs.end() ; it++) {
      I(*it != lvid);
    }
    
    // recycle in the appropiate place
    bool found = false;
    for(it=origLVIDs.begin() ; it != origLVIDs.end() ; it++) {
      if (*it == lvid) {
	found = true;
      break;
      }
    }
    I(found);
  }
#endif  

  Ver2LVIDMap::iterator it = ver2lvid.find(lvid->getVersionRef());
  I(it != ver2lvid.end());

  I(it->second == lvid);
  ver2lvid.erase(it);

  lvid->garbageCollectVersion();

  freeLVIDs.push_front(lvid);
}

void LVIDTable::localKill(const HVersion *v)
{
  Ver2LVIDMap::iterator it = ver2lvid.find(v);
  if(it == ver2lvid.end())
    return;

  LVID *lvid = it->second;

  nLinesOnKill.sample(lvid->getnLinesPinned());
}

void LVIDTable::localRestart(const HVersion *v)
{
  Ver2LVIDMap::iterator it = ver2lvid.find(v);
  if(it == ver2lvid.end())
    return;
  
  LVID *lvid = it->second;

  nLinesOnRestart.sample(lvid->getnLinesPinned());

  lvid->localRestart();
}

void LVIDTable::localSetSafe(const HVersion *v)
{
  Ver2LVIDMap::iterator it = ver2lvid.find(v);
  if (it == ver2lvid.end())
    return;

  LVID *lvid = it->second;

  // FIXME: never called??????

  nLinesOnSetSafe.sample(lvid->getnLinesPinned());
}

void LVIDTable::localSetFinished(const HVersion *v)
{
  Ver2LVIDMap::iterator it = ver2lvid.find(v);
  if (it == ver2lvid.end())
    return;

  LVID *lvid = it->second;

  // FIXME: never called??????
  
  nLinesOnSetFinished.sample(lvid->getnLinesPinned());
}

LVID *LVIDTable::getLVID2Recycle()
{
  LVIDList::iterator it=origLVIDs.begin();
  
  LVID *newestLVID = 0;
  for( ; it != origLVIDs.end() ; it++) {
    if ((*it)->getVersionRef()->hasOutsReqs())
      continue; // Zillions of dinst and outs request
    
    if ((*it)->isKilled())
      return (*it);

    I(!(*it)->getVersionRef()->isKilled());

    if (newestLVID==0) {
      newestLVID = *it;
      continue;
    }
    
    const HVersion *v = (*it)->getVersionRef();
    
    if (*v > *(newestLVID->getVersionRef()))
      newestLVID = *it;
  }
  
  return newestLVID;
}

void LVIDTable::kill(const HVersion *v)
{
  // Send a kill to all the caches in the system (so that local LVID entries
  // are marked accordingly)
  
  for(SystemLVIDs::iterator it = systemLVIDs.begin(); it != systemLVIDs.end(); it++) {
    // TODO: Add timing to propagate signal if non-local cache

    // WARNING: This is more difficult that it seems. If the versions do not get
    // recycled immediately, a mergeSuccessors would not free version. This has
    // many implications in the spawn and displacement. A realistic
    // implementation should not allow local versions to go out (other caches)
    // until previous mergeSuccessors have finished.
    //
    // SOLUTION: as long as there are outstanding mergeSuccessors (or kill
    // successors), non-safe cache lines can not be displaced from the L1 cache
    (*it)->localKill(v);
  }
}

void LVIDTable::restart(const HVersion *v)
{
  // Send a restart to all the caches in the system (silently mark all the
  // caches with a restart). It is not necessary to really send the data because
  // displacements would send the latest up to date state. ONLY kills need to be
  // propagated.

  for(SystemLVIDs::iterator it = systemLVIDs.begin(); it != systemLVIDs.end(); it++) {
    // TODO: Add timing to propagate signal if non-local cache
    (*it)->localRestart(v);
  }
}

void LVIDTable::setSafe(const HVersion *v)
{
  // Send a setSafe to all the caches in the system (silently mark all
  // the caches as safe).
  //
  // This is not strictly necessary in hardware (although it
  // simplifies the protocol). I implemented in software to have more
  // statistics

  for(SystemLVIDs::iterator it = systemLVIDs.begin(); it != systemLVIDs.end(); it++) {
    (*it)->localSetSafe(v);
  }
}

void LVIDTable::setFinished(const HVersion *v)
{
  // Send a setFinished to all the caches in the system (silently mark
  // all the caches as safe).
  //
  // This is not strictly necessary in hardware (although it
  // simplifies the protocol). I implemented in software to have more
  // statistics

  for(SystemLVIDs::iterator it = systemLVIDs.begin(); it != systemLVIDs.end(); it++) {
    (*it)->localSetFinished(v);
  }
}

void LVIDTable::dump(const HVersion *v)
{
  for(SystemLVIDs::iterator it = systemLVIDs.begin(); it != systemLVIDs.end(); it++) {
    // TODO: Add timing to propagate signal if non-local cache
    LVIDTable *lvidc = (*it);
    LVID *lvid = lvidc->findLVID(v);
    if (lvid) {
      MSG("LVIDTable[%s] entry", lvidc->name);
    }
  }
}

void LVIDTable::dump()
{
  LVIDList::iterator it;

  int32_t conta=0;
  for(it=origLVIDs.begin() ; it != origLVIDs.end() ; it++) {
    LVID *lvid = *it;
    const HVersion *ver = lvid->getVersionRef();
    fprintf(stderr,"LVIDTable::dump[%d]",conta++);
    if (ver == 0) {
      fprintf(stderr," K ");
    }else{
      if (ver->isSafe())
	fprintf(stderr," S");
      if (ver->isFinished())
	fprintf(stderr,"F ");
    }

    fprintf(stderr," L[%ld] R[%d] ", lvid->getnLinesPinned(), lvid->getVersionRef()->getnOutsReqs());

    if (lvid->isRecycled())
      fprintf(stderr," Recycled ");
    fprintf(stderr,"\n");
  }
  
}
