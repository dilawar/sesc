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

#ifndef GENERGYD_H
#define GENERGYD_H

#include <stdio.h>
#include <ctype.h>

#include "estl.h"
#include "GStats.h"
#include "Config.h"
#include "SescConf.h"

enum PowerGroup {
  Not_Valid_Power,
  FetchPower, 
  IssuePower,
  MemPower,
  ExecPower,
  ClockPower,
  MaxPowerGroup
};

/*****************************************************
  * designed to manage energy functions
  ****************************************************/

/*
 * Right now this class is designed as entry point to get the energy
 * for processor 0. The idea is also to use it for implementing
 * dynamic voltage scaling.
 */

/***********************************************
  *            Definitions of stat classes
  ***********************************************/
#ifndef SESC_ENERGY

class EnergyStore {
private:
public:
  EnergyStore() {
  }
  double get(const char *block, const char *name, int32_t procId=0) {
    return -1;
  }
  double get(const char *name, int32_t procId=0) {
    return -1;
  }
  void dump(){}
};

class GStatsEnergyBase {
 public:
  double getDouble(){
    return 0.0;
  }
  void inc() {}
  void add(int32_t v){}
};

class GStatsEnergy : public GStatsEnergyBase {
public:
  GStatsEnergy(const char *name, const char *block
               ,int32_t procId, PowerGroup grp
               ,double energy) {
  }
  ~GStatsEnergy() {
  }

  static double getTotalProc(int32_t procId) {
    return 0;
  }
  
  double getDouble(){
    return 0.0;
  }
  void inc() {}
  void add(int32_t v){}
};

class GStatsEnergyNull : public GStatsEnergyBase {
 public:
};

class GStatsEnergyCGBase {
public:
  GStatsEnergyCGBase(const char* str,int32_t id) {}
  void subscribe(double v) {
  }
  void calcClockGate() {
  }
  double getDouble(){
    return 0.0;
  }
};

class GStatsEnergyCG {
public:
  GStatsEnergyCG(const char *name, const char* block
                 ,int32_t procId, PowerGroup grp
                 ,double energy, GStatsEnergyCGBase *b) {
  }
  void inc() {}
  void add(int32_t v){}
  double getDouble() {
    return 0.0;
  }
};
#else // SESC_ENERGY

class EnergyStore {
private:
  const char *proc;

public:   
  static const char *getStr(PowerGroup d);
  static const char *getEnergyStr(PowerGroup d);

  EnergyStore();
  double get(const char *block, const char *name, int32_t procId=0);
  double get(const char *name, int32_t procId=0);
};

class GStatsEnergyBase : public GStats {
 public:
  virtual double getDouble() const = 0;
  virtual void inc() = 0;
  virtual void add(int32_t v) = 0;
};

class GStatsEnergyNull : public GStatsEnergyBase {
 public:
  double getDouble() const;
  void inc();
  void add(int32_t v);

  void reportValue() const {};
};

class GStatsEnergy : public GStatsEnergyBase {
protected:
  typedef std::vector< std::vector<GStatsEnergy *> > EProcStoreType;
  typedef std::vector< std::vector<GStatsEnergy *> > EGroupStoreType;

  static EProcStoreType  eProcStore;  // Energy store per processor
  static EGroupStoreType eGroupStore; // Energy store per group

  const double  StepEnergy;
  long long steps;
  
  PowerGroup gid;

public:
  GStatsEnergy(const char *name, const char* block
               ,int32_t procId, PowerGroup grp
               ,double energy);
  ~GStatsEnergy() {};
  static double getTotalProc(int32_t procId);
  static double getTotalGroup(PowerGroup grp);
  
  static void dump(int32_t procId);
  static void dump();
  static double getTotalEnergy();

#ifdef SESC_THERM
  static void setupDump(int32_t procId);
  static void printDump(int32_t procId);

  void reportValueDump() const;
  void reportValueDumpSetup() const;
#endif

  void reportValue() const;
  
  PowerGroup getGid() const { return gid; }

  virtual double getDouble() const;
  virtual void inc();
  virtual void add(int32_t v);
};

class GStatsEnergyCGBase {
protected:
  Time_t lastCycleUsed;
  int32_t  numCycles;
  char *name;
  int32_t   id;

public:
  int32_t getNumCycles(){ return numCycles; }
  GStatsEnergyCGBase(const char* str,int32_t id);
  
  void use();
};

class GStatsEnergyCG : public GStatsEnergy {
protected:  
  GStatsEnergyCGBase *eb;
  double localE;
  double clockE;

public:
  GStatsEnergyCG(const char *name, const char* block
                 ,int32_t procId, PowerGroup grp
                 ,double energy, GStatsEnergyCGBase *b);

  double getDouble() const;
  void inc();
  void add(int32_t v);
};
#endif

#endif
