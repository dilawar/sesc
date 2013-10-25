/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Karin Strauss

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

// TODO: timing: should the protocol decision take any time?

#ifndef SMPPROTOCOL_H
#define SMPPROTOCOL_H

#include "SMPMemRequest.h"
#include "MSHR.h"
#include "estl.h"

#include "SMPDebug.h"
#include "SMPCacheState.h"

class SMPCache;

enum Protocol_t {
  MESI_Protocol     = 10,
  MESITO_Protocol   = 11,
  MESITOGL_Protocol = 12
}; 

class SMPProtocol {
private:
protected:
  
  Protocol_t protocolType;
  
  SMPCache *pCache;

public:
  typedef CacheGeneric<SMPCacheState, PAddr, false>            CacheType;
  typedef CacheGeneric<SMPCacheState, PAddr, false>::CacheLine Line;

  Protocol_t getProtocolType() {
    return protocolType;
  }

  SMPProtocol(SMPCache *cache);
  virtual ~SMPProtocol();

  // BEGIN interface with cache
  virtual void changeState(Line *, unsigned);
  virtual void makeDirty(Line *l);
  virtual void preInvalidate(Line *l);

  virtual void read(MemRequest *mreq);
  virtual void write(MemRequest *mreq);
  virtual void writeBack(MemRequest *mreq);
  virtual void returnAccess(MemRequest *mreq);
  // END interface with cache

  // BEGIN interface of Protocol

  // coherent cache access interface
  virtual void sendReadMiss(MemRequest *mreq);
  virtual void sendWriteMiss(MemRequest *mreq);
  virtual void sendInvalidate(MemRequest *mreq);
  virtual void sendReadMissAck(SMPMemRequest *sreq);
  virtual void sendWriteMissAck(SMPMemRequest *sreq);
  virtual void sendInvalidateAck(SMPMemRequest *sreq);

  virtual void readMissHandler(SMPMemRequest *sreq) { I(0); }
  virtual void writeMissHandler(SMPMemRequest *sreq) { I(0); }
  virtual void invalidateHandler(SMPMemRequest *sreq) { I(0); }
  virtual void readMissAckHandler(SMPMemRequest *sreq) { I(0); }
  virtual void writeMissAckHandler(SMPMemRequest *sreq) { I(0); }
  virtual void invalidateAckHandler(SMPMemRequest *sreq) { I(0); }

  virtual void sendWriteBack(PAddr addr, CallbackBase *cb);
  virtual void writeBackAckHandler(SMPMemRequest *sreq);

  virtual void sendDisplaceNotify(PAddr addr, CallbackBase *cb);
  virtual void displaceNotifyAckHandler(SMPMemRequest *sreq);

  virtual void sendData(SMPMemRequest *sreq);
  virtual void dataHandler(SMPMemRequest *sreq) { I(0); }

  // END interface of Protocol
};

#endif // SMPPROTOCOL_H
