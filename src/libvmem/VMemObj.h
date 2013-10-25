/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

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

#ifndef VMEMOBJ_H
#define VMEMOBJ_H

#include "Snippets.h"

#include "MemObj.h"
#include "VMemReq.h"

class MemorySystem;

// Common object for all Version Memory Objects. Either Directories or Caches
class VMemObj : public MemObj {
private:
protected:
  const unsigned  int32_t id;
  static uint32_t counter;
  static ushort  lineShift;

public:
  VMemObj(MemorySystem *gms, const char *section, const char *name);

  PAddr calcLine(PAddr addr) const { return addr >> lineShift; }
  
  uint32_t getId() const { return id; }

  // MemObj Interface (only for highest level cache)
  void access(MemRequest *mreq);

  //++++++++++++++++++++++++++
  // Read API
  // Local processor issues a read (only for highest level cache)
  virtual void localRead(MemRequest *mreq) = 0;

  // There is a local miss, the cache issues a read to another cache
  virtual void read(   VMemReadReq *mreq) = 0;
  virtual void readAck(VMemReadReq *mreq) = 0;

  //++++++++++++++++++++++++++
  // Write API

  // Local processor issues a write (only for highest level cache)
  virtual void localWrite(MemRequest *mreq) = 0;

  // Notification of write to other caches. Necessary to detect possible
  // restarts
  virtual void writeCheck(VMemWriteReq *mreq) = 0;

  // Notification of write to other caches. Necessary to detect possible
  // restarts
  virtual void writeCheckAck(VMemWriteReq *mreq) = 0;

  //++++++++++++++++++++++++++
  // Cache Line movement between caches API

  // A Cache decides to displace a cache line
  // 
  // pushLine can be initiated by itself or as a response to a askPushLine
  virtual void pushLine(VMemPushLineReq *mreq) = 0;

  virtual void pushLineAck(VMemPushLineReq *mreq) = 0;

  // A lower level cache ask to displace cache lines of addr X with a version
  // safer than Y (it may trigger multiple pushLine, 1 per version)
  virtual void askPushLine(VMemPushLineReq *mreq) = 0;
};

#endif // VMEMOBJ_H
