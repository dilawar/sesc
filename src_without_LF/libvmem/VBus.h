/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss
		  Luis Ceze

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

#ifndef VBUS_H
#define VBUS_H

#include <set>

#include "callback.h"
#include "nanassert.h"
#include "pool.h"
#include "GStats.h"

#include "VMemObj.h"
#include "LMVCache.h"
#include "GEnergy.h"

#include "VCR.h"

#ifdef DIRECTORY
#include "Directory.h"
#endif /* DIRECTORY */

class PortGeneric;
class LMVCache;


class VBus : public VMemObj {
private:
  static TimeDelta_t delay;

  // Methods

  void doRead(VMemReadReq *vreq);
  typedef CallbackMember1<VBus, VMemReadReq * , &VBus::doRead>  doReadCB;

  void doSendReadAck(VMemReadReq *vreq);
  typedef CallbackMember1<VBus, VMemReadReq * , &VBus::doSendReadAck>  doSendReadAckCB;

  void doWriteCheck(VMemWriteReq *vreq);
  typedef CallbackMember1<VBus, VMemWriteReq * , &VBus::doWriteCheck> doWriteCheckCB;

  void doWriteCheckAck(VMemWriteReq *vreq);
  typedef CallbackMember1<VBus, VMemWriteReq * , &VBus::doWriteCheckAck> doWriteCheckAckCB;

  void doSendWriteCheckAck(VMemWriteReq *vreq);
  typedef CallbackMember1<VBus, VMemWriteReq * , &VBus::doSendWriteCheckAck> doSendWriteCheckAckCB;

  void doPushLine(VMemPushLineReq *vreq);
  typedef CallbackMember1<VBus, VMemPushLineReq * , &VBus::doPushLine>    doPushLineCB;

  void doPushLineAck(VMemPushLineReq *vreq);
  typedef CallbackMember1<VBus, VMemPushLineReq * , &VBus::doPushLineAck>    doPushLineAckCB;

  void doSendAskPushLine(VMemPushLineReq *vreq, VMemObj *obj);
  typedef CallbackMember2<VBus, VMemPushLineReq *, VMemObj * , &VBus::doSendAskPushLine> doSendAskPushLineCB;

  PortGeneric  *port;
  GStatsEnergy *busEnergy;
  GStatsCntr    nPushLineForward;

  LMVCache victimCache; // Victim Multi-version cache

  VCR vcr; // Version Combine Register (dependece enforcement)

#ifdef DIRECTORY
  // directory to maintain "may be present" information
  Directory lineDirectory; 
#endif

  bool isInUpperLevel(MemObj *obj) const;

  void nonVersionRead(MemRequest *mreq);

  void sendReadAck(VMemReadReq *vreq);
  void sendRead(VMemReadReq *vreq, VMemObj *obj);

  void sendWriteCheckAck(VMemWriteReq *vreq);
  void sendWriteCheck(VMemWriteReq *vreq, VMemObj *obj);

public:
  VBus(MemorySystem *gms, const char *section, const char *name);
  virtual ~VBus();

  void returnAccess(MemRequest *mreq);

  void localRead(MemRequest *mreq);
  void localWrite(MemRequest *mreq);

  void read(   VMemReadReq *vreq);
  void readAck(VMemReadReq *vreq);

  void writeCheck(VMemWriteReq *vreq);
  void writeCheckAck(VMemWriteReq *vreq);

  void pushLine(VMemPushLineReq *vreq);
  void pushLineAck(VMemPushLineReq *vreq);
  void askPushLine(VMemPushLineReq *vreq);

  Time_t getNextFreeCycle() const;

  void invalidate(PAddr addr, ushort size, MemObj *oc);
  bool canAcceptStore(PAddr addr);

  MemObj *getLowerLevel() const { return lowerLevel[0]; }
};

#endif // VBUS_H
