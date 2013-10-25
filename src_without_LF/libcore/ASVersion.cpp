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

#include "ASVersion.h"
#include "TaskContext.h"
#include "OSSim.h"

HVersion::poolType HVersion::vPool(1024, "HVersion");

GStatsCntr *HVersion::nCreate=0;
GStatsCntr *HVersion::nShift=0;
GStatsCntr *HVersion::nClaim=0;
GStatsCntr *HVersion::nRelease=0;

HVersion *HVersion::oldestTC=0;
HVersion *HVersion::oldest=0;
HVersion *HVersion::newest=0;

GStatsCntr *HVersion::nChildren[HVersion::nChildrenStatsMax];

HVersion::VType HVersion::lastver = 0;

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

  HVersion *vc = vPool.out();
   
  vc->nChild = 0;
  vc->nUsers = 1;
  vc->base = lastver = 1;
  vc->tc   = t;
  vc->dequeued = false;
  vc->killed   = false;
  vc->nOutsReqs = 0;

  oldestTC = vc;
  
  //  vc->atomic = true;

  oldest = vc;
  newest = vc;

  return vc;
}

void HVersion::garbageCollect(bool noTC)
{
  nUsers--;
  I(nUsers>=0);
  if (nUsers == 0) {
    vPool.in(this);
  }
  return;
}

HVersion *HVersion::create(TaskContext *t)
{
  HVersion *cont  = vPool.out();
  I(t);
  cont->tc = t;

  nCreate->inc();

  cont->nChild = 0;
  newest = cont;

  cont->nUsers = 1;
  cont->killed = false;
  cont->nOutsReqs = 0;
  
  return cont;
}

void HVersion::decOutsReqs() 
{ 
  nOutsReqs--; 
  I(nOutsReqs>=0);
}

HVersion *HVersion::createSuccessor(TaskContext *t, bool atomic)
{
  HVersion *cont = create(t);
  if (atomic)
    cont->base = ++lastver;
  else
    cont->base = 0;
  nChild++;

  return cont;
}


void HVersion::dump(const char *str, bool shortVersion) const
{
  fprintf(stderr,"%s[%6lld,%s]",str?str:"ver", base, base?"atomic":"non-atomic");
}

void HVersion::dumpAll() const
{
}

void HVersion::report()
{
  // The last one includes all the others
  Report::field("HVersion:nChildrenStatsMax=%d", nChildrenStatsMax); 
}
