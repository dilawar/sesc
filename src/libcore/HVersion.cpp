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

#include "HVersion.h"
#include "TaskContext.h"
#include "OSSim.h"
#include "TraceGen.h"

#ifdef TS_TIMELINE
int32_t HVersion::gID=0;
#endif

HVersion::poolType HVersion::vPool(1024, "HVersion");

GStatsCntr **HVersion::IDP::correct;
GStatsCntr **HVersion::IDP::incorrect;

GStatsCntr *HVersion::nCreate=0;
GStatsCntr *HVersion::nShift=0;
GStatsCntr *HVersion::nClaim=0;
GStatsCntr *HVersion::nRelease=0;

HVersionDomain::VDomainVectorType HVersionDomain::vdVec;


GStatsCntr *HVersion::nChildren[HVersion::nChildrenStatsMax];

/***************************** IDP *******/

size_t HVersion::IDP::nChildMax;
HVersion::IDP::PredType **HVersion::IDP::predCache=0;

void HVersion::IDP::boot()
{
  const char *section = SescConf->getCharPtr("TaskScalar","idp");
  if (section==0)
    return;

  nChildMax = SescConf->getInt(section,"IDPnChildMax");

  SescConf->isBetween(section,"IDPnChildMax",0,32);
  // 0 means no predictor

  size_t nCPUs= osSim->getNumCPUs();

  predCache = (PredType **)malloc(sizeof(PredType *)*nCPUs);
  for(size_t i=0; i<nCPUs ; i++) {
    predCache[i] = PredType::create(section,"","HVersion_IDP(%d)",i);
  }

  correct   = (GStatsCntr **)malloc(sizeof(GStatsCntr *)*nCPUs);
  incorrect = (GStatsCntr **)malloc(sizeof(GStatsCntr *)*nCPUs);
  for(size_t i=0; i<nCPUs ; i++) {
    correct[i]   = new GStatsCntr("HVersion(%d)_IDP:correct"  , i);
    incorrect[i] = new GStatsCntr("HVersion(%d)_IDP:incorrect", i);
  }
}

void HVersion::IDP::executed(Pid_t pid, PAddr addr, size_t nChild)
{
  I(predCache); // boot called?
  I(nChildMax !=0);

  ProcessId *proc=ProcessId::getProcessId(pid);
  I(proc);

  CPU_t cpuID = proc->getCPU();
  if (cpuID <0)
    return;

  PredType::CacheLine *cl = predCache[cpuID]->fillLine(addr);
  if (nChild > nChildMax)
    nChild = nChildMax;

  if (cl->getnChild() == nChild)
    correct[cpuID]->inc();
  else
    incorrect[cpuID]->inc();
  
  cl->setnChild(nChild);
}

int32_t HVersion::IDP::predict(Pid_t pid, PAddr addr)
{
  I(nChildMax !=0);

  ProcessId *proc=ProcessId::getProcessId(pid);
  I(proc);

  I(predCache); // boot called?

  CPU_t cpuID = proc->getCPU();

  // Default Prediction. 1 is more common, but it implies give all the
  // bubble which is too much
  if (cpuID <0)
    return -1;

  PredType::CacheLine *cl = predCache[cpuID]->readLine(addr);
  if (cl == 0)
    return -1; 
  
  return cl->getnChild();
}

void HVersion::shiftAllVersions()
{
  // Shift all the versions (avoid VType overflow)

  I(vDomain->oldest);
  HVersion *t   = vDomain->oldest;
  VType minBase = vDomain->oldest->base;

  while(t) {
    t->base -= minBase;
    t->maxi -= minBase;
    t = t->next;
  }

  verify(vDomain->oldest);
}

void HVersion::claim()
{

  VType skip = HVersionReclaim - (maxi - base);
  maxi += skip; // == maxi = base + HVersionReclaim

  I((maxi - base) <= HVersionReclaim);
  I(skip < HVersionReclaim);

  HVersion *n = next;

  nClaim->inc();

  LOG("Begin HVersion::claim %lld @%lld",skip, globalClock);

  while(n && skip) {
   
#ifdef FULL_CLAIM
    VType available = n->maxi - n->base;
#else
    VType available = (n->maxi - n->base)/2;
#endif

    if (n->next==0) {
      n->base += skip;
      n->maxi = n->base + HVersionReclaim;
    }else if (available > skip) {
      n->base += skip;
      skip = 0;
    }else{
      n->base += skip;
      skip    -= available;
      n->maxi += skip;
    }

    I(n->base <= n->maxi);

    I(n->tc);
    
    n = n->next;
  }

//  verify(next);

  LOG("End HVersion::claim @%lld", globalClock);
}

void HVersion::release()
{
  HVersion *n;

  VType overflow = maxi - base - HVersionReclaim;
  
  I(overflow>0);
  
  maxi = base + HVersionReclaim;
  
  n = next;

  nRelease->inc();

  LOG("Begin HVersion::release %lld @%lld",overflow, globalClock);

  while(n && overflow > 0) {
    VType space = HVersionReclaim - (n->maxi - n->base);

    n->base -= overflow;
    if ( space < overflow )
      overflow-= space;
    else
      overflow = 0;
    
    n->maxi -= overflow;

    I(n->base <= n->maxi);

    n = n->next;
  }

  LOG("End HVersion::release @%lld", globalClock);

//  verify(next);
}

HVersion *HVersion::create(TaskContext *t)
{
  HVersion *cont  = vPool.out();
  I(t);
  cont->tc = t;

  nCreate->inc();

  if (next == 0) {
    // Automatic reclaim for most speculative task
    maxi = base + HVersionReclaim;
  }

  if ((maxi - base) < 2)
    claim();

  I((maxi - base) >= 2); // reclaim always works

  cont->nChild = 0;
  // insert in list (cont is in front of this)
  cont->prev   = this;
  cont->next   = next;

  if (cont->next)
    cont->next->prev = cont;
  else {
    I(vDomain->newest == this);
    vDomain->newest = cont;
  }
  
  next = cont;

  cont->nUsers    = 1;
  cont->safe      = false;
  cont->finished  = false;
  cont->dequeued  = false;
  cont->killed    = false;
  cont->nOutsReqs = 0;

  return cont;
}

#ifdef DEBUG
void HVersion::verify(HVersion *orig)
{
  // Verify that the versions are correct
  HVersion *n = orig;

  while(n) {
    if((n->maxi - n->base) > HVersionReclaim){
      n->dump("fail range");
      MSG(".");
    }
    
    I(!n->visited);
    IS(n->visited = true);
    
    if (n->next) {
      if (n->next->base != n->maxi) {
	n->dump("1.fail");
	n->next->dump("2.fail");
	MSG(".");
      }
    }

    n = n->next;
  }

  n = orig;

  while(n) {
    IS(n->visited = false);
    n = n->next;
  }
}
#endif

HVersion *HVersion::boot(TaskContext *t)
{
  I(nClaim  ==0);
  I(nRelease==0);

  for(size_t i=0;i<nChildrenStatsMax;i++) {
    nChildren[i] = new GStatsCntr("HVersion(%d):nChildren",i);
  }

  nCreate  = new GStatsCntr("HVersion:nCreate");
  nShift   = new GStatsCntr("HVersion:nShift");
  nClaim   = new GStatsCntr("HVersion:nClaim");
  nRelease = new GStatsCntr("HVersion:nRelease");

  IDP::boot();

  return newFirstVersion(t);
}

HVersion *HVersion::newFirstVersion(TaskContext *t)
{
  HVersion *vc = vPool.out();
  vc->vDomain = HVersionDomain::create();
   

  vc->nChild = 0;
  vc->nUsers = 1;

  vc->next = 0;
  vc->prev = 0;

  vc->base = 1;
  vc->maxi = vc->base + HVersionReclaim;
  vc->tc   = t;
  vc->safe     = false;
  vc->finished = false;
  vc->dequeued = false;
  vc->killed   = false;
  vc->nOutsReqs = 0;

  vc->vDomain->oldestTC = vc;
  vc->vDomain->oldest = vc;
  vc->vDomain->newest = vc;

  return vc;
}

void HVersion::garbageCollect(bool noTC)
{
  I(base < maxi); // Version format is [base:maxi)

  nUsers--;
  I(nUsers>=0);

  // Quick exit for the most common case (performance freak)
  if(tc && nUsers && !noTC)
    return;

  if(nUsers) {
    if (noTC) {
      // TaskContext recycled the version, but it still can be mapped in a
      // LVIDTable (nUsers>1)
      I(tc);
      tc = 0;

      if (vDomain->oldestTC != this && vDomain->oldestTC->base < base) {
	// There are still in-flight instructions or structures pointing to that
	// version. To avoid to have holes of TC, the HVersion got dequeued

	if (prev) {
	  prev->next = next;
	}else{
	  I(vDomain->oldest == this);
	  vDomain->oldest = next;
	}
	
	if (next) {
	  next->prev = prev;
	}else{
	  I(vDomain->newest == this);
	  vDomain->newest = prev;
	}

	next = 0;
	prev = 0;
	dequeued = true;
	return;
      }
    }

    if (tc == 0 && vDomain->oldestTC == this) {
      vDomain->oldestTC = next;
      I(vDomain->oldestTC);
      I(vDomain->oldestTC->tc); // There can be no holes in commited TCs because
		       // garbageCollect(true) is called in order
    }
    return;
  }else if (dequeued) {
#ifdef TS_TIMELINE
    TraceGen::add(getId(),"recycled=%lld",globalClock);
    TraceGen::dump(getId());
#endif
    vPool.in(this);
    return;
  }

  if (noTC) {
    // TaskContext recycled the version, but it still can be mapped in a
    // LVIDTable (nUsers>1)
    I(tc);
    tc = 0;
  }
  if (vDomain->oldestTC == this && tc == 0)
    vDomain->oldestTC = next;

  I(vDomain->oldestTC!= this);

  if (next) {
    next->prev = prev;
  }else{
    vDomain->newest = prev;
  }
  
  // Remove from link list
  if (prev) {
    prev->next = next;

    // NOTE: If task is safe, versions should not be
    // readjusted. Safe/commited task can finish out of order. So
    // version should not be changed

    // Readjust versions
    if (prev->next) {
      prev->maxi = prev->next->base;
    
      if ((prev->maxi - prev->base) > HVersionReclaim)
	prev->release();

      I(prev->maxi == prev->next->base);
    }else{
      // Previous becomes the most speculative task, it can increase
      // the bubble to the maximum size
      prev->maxi = prev->base + HVersionReclaim;
    }
  }else{
    vDomain->oldest = next;
  }

  GI(vDomain->newest == 0 || vDomain->oldest == 0, 
     vDomain->oldest == vDomain->newest && next == 0 && prev == 0);

//  verify(prev);

  IS(next = 0);
  IS(prev = 0);

  nChildren[nChild<nChildrenStatsMax?nChild:nChildrenStatsMax-1]->inc();

#ifdef TS_TIMELINE
  TraceGen::add(getId(),"recycled=%lld",globalClock);
  TraceGen::dump(getId());
#endif
  vPool.in(this);
}

void HVersion::decOutsReqs() 
{ 
  nOutsReqs--; 
  I(nOutsReqs>=0);
  if (nOutsReqs == 0) 
    TaskContext::tryPropagateSafeToken(vDomain);
}

HVersion *HVersion::createSuccessor(TaskContext *t)
{
  HVersion *cont = create(t);
  cont->vDomain = vDomain; // successor has the same vDomain

  nChild++;

  if (cont->next == 0) {
    // The most speculative version
    I(next == cont);
    // Assign versions
    cont->base = maxi;
    cont->maxi = maxi+HVersionReclaim;
    if (maxi > HVersionMax)
      shiftAllVersions();
  }else{
    VType prange =maxi-base; // Parent range
    I(256 < (HVersionReclaim/16));
    if (prange <= 4)
      prange = prange/2;
    else if (prange <= 64 || IDP::deactivated())
      prange = prange/4;
    else { 
		int32_t c = IDP::predict(tc->getPid(), t->getSpawnAddr());
		int32_t p = IDP::predict(tc->getPid(), tc->getSpawnAddr());
		if ((int)nChild >= p && c>0) {
	// last spawn, child spawns
	prange = prange/64;
      }else if (t->getSpawnAddr() == tc->getSpawnAddr()) {
	// Freaking loop
	prange = prange/64;
		}else if ((int)nChild >= p && p == 1) {
	// last spawn
	prange = prange/32;
		}else if ((int)nChild < p && c == 0) {
	// Oscilates betwen c==0 & c==1
	prange = prange/4;
      }else{
        prange = prange/16;
      }
    }

    // Assign versions
    cont->base = base+prange;
    cont->maxi = maxi;
    maxi = cont->base;
  }

  I(base <= maxi);
  I(cont->base <= cont->maxi);
  I(base < cont->base);
  I(maxi == cont->base);

  I(cont->nUsers = 1);

#ifdef TS_TIMELINE
  cont->id = gID++;
#endif
  return cont;
}


void HVersion::dump(const char *str, bool shortVersion) const
{
  if (shortVersion)
    fprintf(stderr,"%s[0x%6llx]",str?str:"ver", base);
  else
    fprintf(stderr,"%s[0x%6llx:0x%6llx]",str?str:"ver", base,maxi);

  if (finished)
    fprintf(stderr,"F");
  if (safe)
    fprintf(stderr,"S");
  if (killed)
    fprintf(stderr,"K");
  if (dequeued)
    fprintf(stderr,"D");
}

void HVersion::dumpAll() const
{
  const HVersion *n;
  char cadena[100];
  int32_t i;

  n = this;
  i = 0;
  while(n) {
    sprintf(cadena,"%4d",i);
    
    n->dump(cadena);
    fprintf(stderr," nUsers[%d]\n"
	    ,nUsers
      );
    
    n = n->next;
    i++;
  }
}

void HVersion::setSafe() 
{
  safe = true;

#ifdef TS_TIMELINE
  TraceGen::add(getId(),"safe=%lld",globalClock);
#endif
}

void HVersion::setKilled() 
{
  I(!safe);
  killed   = true;
  finished = true;

#ifdef TS_TIMELINE
  TraceGen::add(getId(),"kill=%lld:finish=%lld",globalClock,globalClock);
#endif
}

void HVersion::setFinished()
{
  Pid_t tid       = tc->getPid();
  PAddr spawnAddr = tc->getSpawnAddr();

  IDP::executed(tid, spawnAddr, nChild);

  I(!finished);
  finished = true;

#ifdef TS_TIMELINE
  TraceGen::add(getId(),"finish=%lld",globalClock);
#endif
}

void HVersion::report()
{
  // The last one includes all the others
  Report::field("HVersion:nChildrenStatsMax=%d", nChildrenStatsMax); 
}

HVersionDomain *HVersionDomain::create() 
{
    HVersionDomain *nvd = new HVersionDomain();
    vdVec.push_back(nvd);
    return nvd;
}

void HVersionDomain::tryPropagateSafeTokenAll()
{
for (uint32_t i = 0; i < vdVec.size(); i++)
  TaskContext::tryPropagateSafeToken(vdVec[i]);
}
