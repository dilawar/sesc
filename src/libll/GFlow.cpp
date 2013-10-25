/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by Luis Ceze
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

#include "GFlow.h"
#include "MemObj.h"
#include "GMemoryOS.h"
#include "GMemorySystem.h"

long long GFlow::nExec  = 0;
bool GFlow::goingRabbit = true; // Until everything boots, it is in running mode
MemObj *GFlow::trainCache = 0;

GFlow::GFlow(int32_t i, int32_t cId, GMemorySystem *gmem) 
  : fid(i), 
    cpuId(cId), 
    gms(gmem), 
    gmos(gmem->getMemoryOS())
{

  //gproc = osSim->id2GProcessor(cpuId);

  if (trainCache == 0) {
    if (SescConf->checkCharPtr("cpucore","trainCache", cId)) {
      const char *cpuSection = SescConf->getCharPtr("", "cpucore", cId);
      std::vector<char *> vPars = SescConf->getSplitCharPtr(cpuSection, "trainCache");
      
      if (vPars.size() != 2) {
	MSG("Required format: trainCache = \"descriptionSection name\"\n");
	return;
      }
    
      const char *cacheSection = vPars[0];
      const char *cacheName = vPars[1];

      trainCache = gms->searchMemoryObj(true, cacheSection, cacheName);
      if(trainCache == 0) {
	// Maybe it got privatized
	char *ret=(char*)malloc(strlen(cacheName) + 20);
	sprintf(ret,"P(%i)_%s", cId, cacheName);

	trainCache = gms->searchMemoryObj(false, cacheSection, ret);
      }
      GMSG(trainCache==0,"Unknown cache to train [%s:%s]",cacheName, cacheSection);
    }
  }
}

void GFlow::dump()
{
  if (trainCache == 0)
    return;

  // Finish the in-fligh operations
  while(!EventScheduler::empty())
    EventScheduler::advanceClock();

  MemObj *mobj = trainCache;
  do {

    const MemObj::LevelType *ll = mobj->getLowerLevel();

    mobj->dump();
    if (ll->empty())
      break;
    mobj = (*ll)[0];
  }while(1);
}
