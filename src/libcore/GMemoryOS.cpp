/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

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

#include "GMemoryOS.h"


DummyMemoryOS::DummyMemoryOS(int32_t i)
  : GMemoryOS(i)
{
  itlb = DTLBCache::create(SescConf->getCharPtr("cpucore","itlb",i)
			  , "", "P(%d)_ITLB",i);
  dtlb = DTLBCache::create(SescConf->getCharPtr("cpucore","dtlb",i)
			  , "", "P(%d)_DTLB",i);
}

DummyMemoryOS::~DummyMemoryOS() 
{
}

int32_t DummyMemoryOS::ITLBTranslate(VAddr iAddr)
{
  return iAddr;
}

int32_t DummyMemoryOS::TLBTranslate(VAddr vAddr)
{ 
  return vAddr;
}

void DummyMemoryOS::solveRequest(MemRequest *r)
{
  MSG("The DummyMemoryOS should not have failures in translation...");
  exit(-1);
}


void DummyMemoryOS::boot()
{
  // Nothing
}

void DummyMemoryOS::report(const char *str) 
{
  // Nothing
}
