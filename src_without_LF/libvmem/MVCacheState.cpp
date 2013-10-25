
#include "MVCacheState.h"

/**********************************
 * MVCacheState
 **********************************/

MVCacheState::MVCacheState()
{
  lvid = 0;
  tag  = 0;
  IS(_set = (ulong) -1;);
}

void MVCacheState::promote()
{
  I(lvid);

  subLVID = lvid->getSubLVID();

  VMemState::promote();
  // Too optimistic
  //  if(lvid->getVersionRef()->isNewest())
  //    setMostSpecLine();
}

void MVCacheState::promote2Safest()
{
  I(isSafe());
  I(lvid->getVersionRef());
  I(leastSpecLine);

  return;

  LVID *safestLVID = lvid->getLVIDTable()->getSafestEntry();

  if (lvid == safestLVID)
    return; // already promoted or not finished the task

#ifdef VMEM_CVBASE_IN_ADDR
  I(0);
  // A tag can go to different sets, so the cache line must be removed from one
  // set and inserted in another if such a thing happens. Of course, there
  // should be free entries in the other set too. (Tought process)
#endif
  PAddr paddr = getPAddr(); // use lvid to calc address

  I(safestLVID);
  
  lvid->decLinesUsed();
  safestLVID->incLinesUsed();

  // update tag
  LPAddr a=cache->calcTag(lvid->calcLPAddr(paddr));
  I(paddr);
  I(a);
  setTag(a);
}

void MVCacheState::initialize(CacheType *c)
{
  cache = c;

  VMemState::initialize();

  ulong l = c->getLog2AddrLs();
  l = (1<<l)-1;
  I(lineMask == l);

  lvid = 0;
  tag  = 0;
}

bool MVCacheState::isLocked() const 
{
  if (lvid == 0) {
    I(tag == 0);
    return false;
  }

  I(tag);

  if (isRestarted()) {
    I(!isSafe());
    return false;
  }
  if (lvid->isKilled())
    return false;
  
  // CL is pinned as long as it is not the safest version in the system (a
  // global view of the system can have more aggresive definition of pinned, but
  // it would be necessary to know if there are safer version for the same
  // address)
  if (!hasState())
    return false;

  return !isLeastSpecLine();
}

void MVCacheState::invalidate() 
{
  if (lvid == 0) {
    I(getTag() == 0);
    return;
  }

  lvid->decLinesUsed();
  lvid = 0;
  tag  = 0;
}

void MVCacheState::resetState(LPAddr addr, LVID *l, const VMemState *st)
{
  I(lvid==0);
  lvid = l; 
  
  lvid->incLinesUsed();
 
  I(!l->isKilled());
  
  subLVID = lvid->getSubLVID();

  LPAddr a = cache->calcTag(addr);
  I(l->calcPAddr(addr));
  I(a);
  setTag(a);
  
  if (st) {
    VMemState::copyStateFrom(st);
  }else
    VMemState::clearState();

//  I(l->getVersionRef());  //new delete by hr

  // Too optimistic
  //if (l->getVersionRef()->isOldest())
  //   setLeastSpecLine();

  //  if (l->getVersionRef()->isNewest())
  //    setMostSpecLine();
}

bool MVCacheState::accessLine()
{
  if (lvid == 0) {
    // Invalid line do nothing
    return false;
  }

  if (lvid->isKilled()) {
    // task got Killed, recycle LVID
    invalidate();
    return true;
  }

  if (subLVID == lvid->getSubLVID())
    return false;

  I(isRestarted());

  // task got restarted, upgrade flags accordingly
#ifdef TS_VMNOPROMOTE
  MSG("TS_VMNOPROMOTE!!!!!!!!!\n");
  invalidate(); 
  return true;
#endif
  
  if (isDirty()) {
    invalidate(); // Dirty lines can not be easily promoted
    return true;
  }
  
  promote();
  return false;
}

void MVCacheState::dump(const char *str)
{
  printf(" %s tag=0x%llx wrmask[0x%lx] rdmask[0x%lx]"
	 ,str, tag
	 ,wrmask, xrdmask
	 );

  if(isSafe())
    printf(":  safe");
  else
    printf(":NOsafe");

  if(isFinished())
    printf(":  finished");
  else
    printf(":NOfinished");

  if(isFinished())
    printf(":  finished");
  else
    printf(":NOfinished");

  if(isRestarted())
    printf(":  restarted");
  else
    printf(":NOrestarted");

  if(isKilled())
    printf(":  killed");
  else
    printf(":NOkilled");

  if(isInvalid())
    printf(":  invalid");
  else
    printf(":NOinvalid");

  if(isLocked())
    printf(":  locked");
  else
    printf(":NOlocked");

  if(isLeastSpecLine())
    printf(":  LeastSpec");
  else
    printf(":NOLeastSpec");

  if(isMostSpecLine())
    printf(":  MostSpec");
  else
    printf(":NOMostSpec");

  printf("\n");
}
