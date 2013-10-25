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

#ifndef GENERGYMGR_H
#define GENERGYMGR_H

#include "nanassert.h"

#include "GEnergy.h"
#ifdef SESC_ENERGY

class EnergyMgr {
private:
  static EnergyStore *enStore;
  
  EnergyMgr() {
    I(0); // No instance allowed
  }
public:
  static void init();

  static double etop(double energy); 
  
  static double ptoe(double power);
  
  static double cycletons(double clk);

  static double get(const char *block, const char *name, int32_t procId=0);
  static double get(const char* name, int32_t procId=0);
};

#else // !SESC_ENERGY

class EnergyMgr {
private:
  EnergyMgr() {
    I(0); // No instance allowed
  }
public:
  static void init(){
  }
  static double etop(double energy) {
    I(0);
    return 0;
  }
  
  static double ptoe(double power) {
    I(0);
    return 0;
  }

  static double get(const char *block, const char *name, int32_t procId=0) {
    return -1.0;
  }
  static double get(const char* name, int32_t procId=0) {
    return -1.0;
  }
};
#endif // !SESC_ENERGY

#endif // !GENERGYMGR_H
