/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Milos Prvulovic
                  Smruti Sarangi

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

/*
 * This launches the SESC simulator environment with an ideal memory
 */

#include <stdlib.h>
#include <vector>

#include "nanassert.h"

#include "SMTProcessor.h"
#include "Processor.h"

#include "GMemorySystem.h"

#include "OSSim.h"
#include "SescConf.h"

#ifdef TASKSCALAR
#include "TaskHandler.h"
#endif

int32_t main(int32_t argc, char **argv, char **envp)
{ 
#ifdef TASKSCALAR
  taskHandler = new TaskHandler();
#endif
  
  osSim = new OSSim(argc, argv, envp); // You can extend the OS if desired

  int32_t nProcs = SescConf->getRecordSize("","cpucore");

  std::vector<GMemorySystem *> ms(nProcs);
  std::vector<GProcessor *>    pr(nProcs);
  
  for(Pid_t i = 0; i < nProcs; i ++) {
    GMemorySystem *gms= new DummyMemorySystem(i);
    gms->buildMemorySystem();
    ms[i] = gms;
    pr[i] = 0;
    if(SescConf->checkInt("cpucore","smtContexts",i)) {
      if( SescConf->getInt("cpucore","smtContexts",i) > 1 )
	pr[i] =new SMTProcessor(ms[i], i);
    }
    if (pr[i] == 0)
      pr[i] =new Processor(ms[i], i);
  }

  osSim->boot();

  // Reaches this point only when all the active threads have finished.

  for(int32_t i = 0; i < nProcs; i ++) { 
    delete pr[i];
    delete ms[i];
  }

  delete osSim;

  return 0;
}
