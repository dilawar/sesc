/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of California Santa Cruz

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

#ifndef DEPWINDOW_H
#define DEPWINDOW_H

#include "Instruction.h"
#include "EnergyMgr.h"
#include "nanassert.h"
#include "Port.h"
#include "Resource.h"
#include "Snippets.h"
#include "SCTable.h"

class DInst;
class GProcessor;

class DepWindow {
private:
  GProcessor *gproc;

  const int32_t Id;

  const TimeDelta_t InterClusterLat;
  const TimeDelta_t WakeUpDelay;
  const TimeDelta_t SchedDelay;
  const TimeDelta_t RegFileDelay;


  GStatsCntr nReplay;

  GStatsEnergy *resultBusEnergy;
  GStatsEnergy *forwardBusEnergy;

  GStatsEnergyBase *windowRdWrEnergy;  // read/write on an individual window entry
  GStatsEnergyBase *windowCheckEnergy; // Check for dependences on the window
  GStatsEnergyBase *windowSelEnergy;   // instruction selection

  
  PortGeneric *wakeUpPort;
  PortGeneric *schedPort;


protected:
  void preSelect(DInst *dinst);

public:
  ~DepWindow();
  DepWindow(GProcessor *gp, const char *clusterName);

  void wakeUpDeps(DInst *dinst);

  void select(DInst *dinst);

  StallCause canIssue(DInst *dinst) const;
  void addInst(DInst *dinst);
  void executed(DInst *dinst);


};

#endif // DEPWINDOW_H
