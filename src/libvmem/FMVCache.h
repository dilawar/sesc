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

#ifndef FMVCACHE_H
#define FMVCACHE_H

#include "GMVCache.h"

// Multi Version Cache (not lowest level: LMVCache)
class FMVCache : public GMVCache {
private:
  GStatsCntr nRestartPush;

  typedef std::vector<CacheLine *> SafeLines;
  
  SafeLines safeLines;

  // a MSHR for external request. It can not use the local mshr
  // because it may generate deadlocks.
  MSHR<PAddr,FMVCache> *extMSHR;
  static char *getExtMSHRName(const char *name);
protected:

  void findSafeLines(PAddr paddr);
  bool displaceSafeLines(PAddr paddr, VMemPushLineReq *askReq=0);

  // BEGIN GMVCache pure virtual
  void cleanupSet(PAddr paddr);
  void displaceLine(CacheLine *cl);
  CacheLine *allocateLine(LVID *lvid, LPAddr addr);
  // END GMVCache pure virtual

  void createNewLine(const VRWReq *vreq, const VMemState *state);

  void handleMiss(MemRequest *mreq);
  void initWriteCheck(LVID *lvid, MemRequest *mreq, bool writeHit);

  void doAccess(MemRequest *mreq);
  void doAccessQueued(MemRequest *mreq);
  bool performAccess(MemRequest *mreq); // true = hit

  void sendPushLine(HVersion *verDup, PAddr paddr, const VMemState *state, bool noMoreSharers, VMemPushLineReq *askReq);
  void doSendPushLine(VMemPushLineReq *vreq);

  void doRead(VMemReadReq *vreq, bool wait4PushLine);

  typedef CallbackMember1<FMVCache, MemRequest *, &FMVCache::doAccess> 
    doAccessCB;

  typedef CallbackMember1<FMVCache, MemRequest *, &FMVCache::doAccessQueued> 
    doAccessQueuedCB;

  typedef CallbackMember1<FMVCache, VMemPushLineReq *, &FMVCache::doSendPushLine> 
    doSendPushLineCB;

  typedef CallbackMember2<FMVCache, VMemReadReq *, bool, &FMVCache::doRead> 
    doReadCB;

public:
  FMVCache(MemorySystem *gms, const char *section, const char *name);
  virtual ~FMVCache();

  void returnAccess(MemRequest *mreq);

  void localRead(MemRequest *mreq);
  void localWrite(MemRequest *mreq);

  void read(   VMemReadReq *mreq);
  void readAck(VMemReadReq *mreq);

  void writeCheck(VMemWriteReq *mreq);
  void writeCheckAck(VMemWriteReq *vreq);

  void pushLine(   VMemPushLineReq *vreq);
  void pushLineAck(VMemPushLineReq *vreq);
  void askPushLine(VMemPushLineReq *vreq);
};

#endif // FMVCACHE_H
