
/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by  Smruti Sarangi

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

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "EnergyMgr.h"
#include "ReportGen.h"
#ifdef SESC_THERM
#include "ReportTherm.h"
#endif

GStatsEnergy::EProcStoreType  GStatsEnergy::eProcStore;
GStatsEnergy::EGroupStoreType GStatsEnergy::eGroupStore; 

double GStatsEnergyNull::getDouble() const
{
  return 0;
}

void GStatsEnergyNull::inc()
{
}

void GStatsEnergyNull::add(int32_t v)
{
}

GStatsEnergy::GStatsEnergy(const char *field, const char *blk
			   ,int32_t procId, PowerGroup grp
			   ,double energy)
  :StepEnergy(energy)
  ,steps(0)
  ,gid(grp)
{
  if( eProcStore.size() <= static_cast<size_t>(procId) ) {
    eProcStore.resize(procId+1);
  }
  eProcStore[procId].push_back(this);

  if(eGroupStore.size() <= static_cast<size_t>(grp) )
    eGroupStore.resize(grp + 1);
  eGroupStore[grp].push_back(this);
    
  char cadena[1024];
  sprintf(cadena,"%s:%s", blk, field) ;
  name = strdup(cadena);
  subscribe();
}


#ifdef SESC_THERM
void GStatsEnergy::setupDump(int32_t procId) 
{
  I((uint32_t)procId < eProcStore.size());
  
  for (size_t i = 0; i < eProcStore[procId].size(); i++) {
    GStatsEnergy *e = eProcStore[procId][i];
    
    ReportTherm::field("%s\t", e->name);
  }
  ReportTherm::field("\n");
#if 0
  for (size_t i = 0; i < eProcStore[procId].size(); i++) {
    GStatsEnergy *e = eProcStore[procId][i];

    e->inc(); // charge unit energy to compute power densities
    double pwr = EnergyMgr::etop(e->getDouble());
    ReportTherm::field("%g\t", pwr);
  }
  ReportTherm::field("\n");
#endif
}

void GStatsEnergy::printDump(int32_t procId) 
{
  I((uint32_t)procId < eProcStore.size());

  // dump values
  for(size_t i=0;i<eProcStore[procId].size();i++) {
    GStatsEnergy *e = eProcStore[procId][i];

    double pwr = EnergyMgr::etop(e->getDouble());

    ReportTherm::fieldRaw(pwr);
  }
}

void GStatsEnergy::reportValueDumpSetup() const
{
}

void GStatsEnergy::reportValueDump() const
{
}
#endif

void GStatsEnergy::dump(int32_t procId)
{
  double pVals[MaxPowerGroup];
  for(int32_t c=0; c < MaxPowerGroup; c++) 
    pVals[c] = 0.0;
   
  I((uint32_t)procId < eProcStore.size());

  // calculate the values
  for(size_t i=0;i<eProcStore[procId].size();i++) {
    GStatsEnergy *e = eProcStore[procId][i];

    pVals[e->getGid()] += EnergyMgr::etop(e->getDouble());
  }

  // dump the values
  for(int32_t j=1; j < ClockPower;j++)
    Report::field("Proc(%d):%s=%g",procId, EnergyStore::getStr(static_cast<PowerGroup>(j)), pVals[j]);
}

void GStatsEnergy::dump()
{
  double pVals[MaxPowerGroup];
  for(int32_t i=0; i < MaxPowerGroup; i++) 
    pVals[i] = 0.0;

  // calculate the values
  for(size_t i=1;i< MaxPowerGroup ;i++) {
    PowerGroup pg = static_cast<PowerGroup>(i);
    double    pwr = EnergyMgr::etop(GStatsEnergy::getTotalGroup(pg));

    pVals[pg] += pwr;
  }

  // dump the values
  for(int32_t j=1; j < MaxPowerGroup;j++)
    Report::field("PowerMgr:%s=%g",EnergyStore::getStr(static_cast<PowerGroup>(j)),pVals[j]);
  for(int32_t j=1; j < MaxPowerGroup;j++)
    Report::field("EnergyMgr:%s=%g",EnergyStore::getEnergyStr(static_cast<PowerGroup>(j)),EnergyMgr::ptoe(pVals[j]));

}

double GStatsEnergy::getTotalEnergy()
{
  double totalEnergy = 0.0;

  double pVals[MaxPowerGroup];
  for(int32_t i=0; i < MaxPowerGroup; i++) 
    pVals[i] = 0.0;

  // calculate the values
  for(size_t i=1;i< MaxPowerGroup ;i++) {
    PowerGroup pg = static_cast<PowerGroup>(i);
    double    pwr = EnergyMgr::etop(GStatsEnergy::getTotalGroup(pg));

    pVals[pg] += pwr;
  }

  // dump the values
  for(int32_t j=1; j < MaxPowerGroup;j++)
    totalEnergy += EnergyMgr::ptoe(pVals[j]);

  // printf("E:%f\n", totalEnergy);
  return totalEnergy;
}

void GStatsEnergy::reportValue() const
{
  Report::field("%s=%g", name, getDouble());
}


double GStatsEnergy::getTotalProc(int32_t procId)
{
  double total=0;

  I((uint32_t)procId < eProcStore.size());

  for(size_t i=0;i<eProcStore[procId].size();i++) {
    GStatsEnergy *e = eProcStore[procId][i];
    total += e->getDouble();
  }

  return total;
}

double GStatsEnergy::getTotalGroup(PowerGroup grp)
{
  double total=0;

  if(eGroupStore.size() <= static_cast<size_t>(grp))
    return 0.0;

  for(size_t i=0;i<eGroupStore[grp].size();i++) {
    total += eGroupStore[grp][i]->getDouble();
  }

  return total;
}

double GStatsEnergy::getDouble() const
{
#ifndef ACCESS
  return StepEnergy*steps;
#else
  return static_cast<double>(steps) ;
#endif
}

void GStatsEnergy::inc() 
{
  steps++;
}

void GStatsEnergy::add(int32_t v)
{
  I(v >= 0);
  steps += v;
}

/*****************************************************
  *           GStatsEnergyCGBase
  ****************************************************/

GStatsEnergyCGBase::GStatsEnergyCGBase(const char* str,int32_t procId)
  : lastCycleUsed(0) 
  ,numCycles(0)
{
  id = procId;
  name = strdup(str);
}

void GStatsEnergyCGBase::use()
{
  if(lastCycleUsed != globalClock) {
    numCycles++;
    lastCycleUsed = globalClock;
  }
}

/*****************************************************
  *           GStatsEnergyCG
  ****************************************************/
GStatsEnergyCG::GStatsEnergyCG(const char *name, 
			       const char* block, 
			       int32_t procId, 
			       PowerGroup grp, 
			       double energy, 
			       GStatsEnergyCGBase *b)
  : GStatsEnergy(name, block, procId, grp, energy) 
{
  eb = b;
  localE = energy*0.95;
  clockE = energy*0.05;
}

double GStatsEnergyCG::getDouble() const
{
#ifndef ACCESS
  return (steps * localE + eb->getNumCycles()*clockE);
#else
  return static_cast<double>(steps);
#endif
}

void GStatsEnergyCG::inc()
{
  steps++;
  eb->use();
}

void GStatsEnergyCG::add(int32_t v)
{
  I(v >= 0);
  steps += v;
  eb->use();
}

// Energy Store functions
EnergyStore::EnergyStore() 
{
  proc = SescConf->getCharPtr("","cpucore",0) ;
}

double EnergyStore::get(const char *name, int32_t procId)
{
  return get(proc, name, procId);
}

double EnergyStore::get(const char *block, const char *name, int32_t procId)
{
  return SescConf->getDouble(block, name);
}

const char* EnergyStore::getStr(PowerGroup p)
{
  switch(p) {
  case FetchPower:
    return "fetchPower";
  case IssuePower:
    return "issuePower";
  case MemPower:
    return "memPower";
  case ExecPower:
    return "execPower";
  case ClockPower:
    return "clockPower";
  case  Not_Valid_Power:
  default:
    I(0);
    return "";
  }

  return 0;
}

const char *EnergyStore::getEnergyStr(PowerGroup p)
{
   switch(p) {
  case FetchPower:
    return "fetchEnergy";
  case IssuePower:
    return "issueEnergy";
  case MemPower:
    return "memEnergy";
  case ExecPower:
    return "execEnergy";
  case ClockPower:
    return "clockEnergy";
  case  Not_Valid_Power:
  default:
    I(0);
    return "";
  }

  return 0;

}
