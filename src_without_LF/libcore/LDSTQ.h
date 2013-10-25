/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze

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


#ifndef LDSTQ_H
#define LDSTQ_H

#include <vector>
#include <set>
#include "estl.h"
#include "GStats.h"

#include "DInst.h"

class GProcessor;

class LDSTQ {
 private:
  typedef std::set<DInst *> DInstSet;
  DInstSet inflightInsts;

  typedef std::vector<DInst *> DInstQueue;
  typedef HASH_MAP<PAddr, DInstQueue> AddrDInstQMap;
  AddrDInstQMap instMap;

  GStatsCntr ldldViolations;
  GStatsCntr stldViolations;
  GStatsCntr ststViolations;
  GStatsCntr stldForwarding;

  GProcessor *gproc;
  
 public:
  LDSTQ(GProcessor *gp, const int32_t id);
  ~LDSTQ() { }
  
  void insert(DInst *dinst);
  bool executed(DInst *dinst);
  void remove(DInst *dinst);

  static VAddr calcWord(const DInst *dinst) {
    return (dinst->getVaddr()) >> 2;
  }
};

#endif 
