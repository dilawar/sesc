/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

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


#ifndef TRACEENTRY_H
#define TRACEENTRY_H

#include "nanassert.h"
#include "Snippets.h"
#include "ThreadContext.h" // just for the types :-(

class TraceEntry {
 public:
  uint32_t  rawInst;
  VAddr iAddr;
  VAddr dAddr;

  VAddr nextIAddr;

  uint32_t  cid; // cpu number
  uint32_t  pid;
  uint32_t  ppid;
  int32_t   barrNProcs;
  
  int32_t dataSize;
  unsigned long long value;

  bool eot;
  
  bool contextSwitch;
  
  TraceEntry() {
    rawInst   = 0;
    iAddr     = 0;
    dAddr     = 0;
    nextIAddr = 0;
    pid       = 0;

    dataSize = 0;
    value    = 0;

    eot           = false;
    contextSwitch = false;
  }
};

#endif
