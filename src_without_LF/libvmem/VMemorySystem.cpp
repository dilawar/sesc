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

#include "VMemorySystem.h"
#include "FMVCache.h"
#include "LMVCache.h"
#include "VBus.h"
#include "SescConf.h"

VMemorySystem::VMemorySystem(int32_t processorId)
  : MemorySystem(processorId)
{
  baseCache = 0;
}

MemObj *VMemorySystem::buildMemoryObj(const char *type, const char *section, const char *name)
{
  MemObj *obj;
  
  if (strcasecmp(type, "mvcache") == 0) {
    if(baseCache) {
      MSG("Only first level cache must be a mvcache");
      SescConf->notCorrect();
    }

    baseCache = (VMemObj *)1; // So that levels are correctly build
    baseCache = new FMVCache(this, section, name); // Only one level of mvcache tested
    obj = baseCache;

  }else{
    if (baseCache == 0 && strcasecmp(type, "icache")) {
      MSG("First level cache must be a mvcache [%s]", type);
      SescConf->notCorrect();
    }

    if (!strcasecmp(type, "dirmvcache")) {
      obj = new LMVCache(this, section, name);
    }else if (!strcasecmp(type, "vbus")) {
      obj = new VBus(this, section, name);
    }else{
      obj = MemorySystem::buildMemoryObj(type, section, name);
    }
  }
  
  return obj;
}

GLVID *VMemorySystem::findCreateLVID(HVersion *ver)
{
  // NOTE: NO versioned cache is used; you may be using the wrong
  // configuration file.
  I(baseCache);

  return static_cast<GMVCache *>(baseCache)->findCreateLVID(ver);
}

