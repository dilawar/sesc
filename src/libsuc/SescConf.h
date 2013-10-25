/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Luis Ceze
                  Smruti Sarangi
                  Paul Sack

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

#ifndef SESCCONF_H
#define SESCCONF_H

#include <vector>

#include "Config.h"

class SConfig:public Config {
private:
protected:

  // Redefine the following two methods so that the class works a
  // little bit different.
  // 
  // Instead of the original Config interface I have something a
  // little bit more specific for the sesc configuration file.  The
  // first section of the configuration file has variables that
  // point to sections. Example:
  // 
  // bpred = 'myBPredSection'
  // a = 1
  // b = 1 
  // [myBPredSection]
  // a = 2
  // c = 2
  // 
  // Results for getInt:
  // getInt("bpred","a") = 1 // main section overides private section
  // getInt("bpred","b") = 1 // only defined in main
  // getInt("bpred","c") = 2 // only defined in section
  // 
  // Remeber that the environment variable ALWAYS overides local variables:
  // SESC_a = 7 // highest priority overide (a=7)
  // SESC_bpred_a = 8 // whould use a = 7 instead. If SESC_a is not defined a = 8
  // SESC_bpred_c = 7 // highest
  // SESC_b = 7 
  // SESC_bpred_b = 7 // unless getInt("bpred","b") it is ignored
  // 

  virtual const char *getEnvVar(const char *block,
				const char *name);

  virtual const Record *getRecord(const char *block,
				  const char *name,
				  int32_t vectorPos);

public:
  SConfig(const char *name);

  std::vector<char *> getSplitCharPtr(const char *block,
				      const char *name,
				      int32_t vectorPos=0);
};

extern SConfig *SescConf;       // declared in SescConf.cpp

#endif   // SESCCONF_H
