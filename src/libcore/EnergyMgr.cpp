/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by  Smruti Sarangi
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

#include "EnergyMgr.h"
#include "SescConf.h"
#include "ProcessId.h"
#include "OSSim.h"

EnergyStore *EnergyMgr::enStore=0;

void EnergyMgr::init()
{
  enStore = new EnergyStore();

  int32_t nProcs = SescConf->getRecordSize("","cpucore");

  // Check area factor, so that it is dump in conf
  for(Pid_t i = 0; i < nProcs; i ++) {
    SescConf->getDouble("cpucore","areaFactor",i);
  }
}

double EnergyMgr::get(const char *block, const char *name, int32_t procId)
{
  I(enStore);
  return enStore->get(block, name, procId);
}

double EnergyMgr::get(const char* name, int32_t procId)
{
  I(enStore);
  return enStore->get(name, procId);
}

double EnergyMgr::etop(double energy) 
{     // Energy to Power
  return (energy/globalClock) * (osSim->getFrequency()/1e9);
}
  
double EnergyMgr::ptoe(double power) 
{      // Power to Energy
  double time = globalClock * (1e9/osSim->getFrequency());
  return power * time;
}
  
double EnergyMgr::cycletons(double clk) 
{ // cycles to ns
  double fac = osSim->getFrequency()/1e9;
  return clk / fac;
}

