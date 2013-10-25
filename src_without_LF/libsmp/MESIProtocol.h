/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Karin Strauss
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

#ifndef MESIPROTOCOL_H
#define MESIPROTOCOL_H

#include "SMPProtocol.h"
#include "SMPCache.h"

enum MESIState_t {
  MESI_INVALID          = 0x00000000, // data is invalid
  MESI_TRANS            = 0x10000000, // all transient states start w/ 0x1
  MESI_TRANS_RD         = 0x12000000, // just started rd
  MESI_TRANS_RD_MEM     = 0x16000000, // had to go to memory for rd 
  MESI_TRANS_WR         = 0x13000000, // just started wr
  MESI_TRANS_WR_MEM     = 0x17000000, // had to go to memory for wr 
  MESI_TRANS_RSV        = 0x14000000, // line has just been allocated (reserved)
  MESI_TRANS_INV        = 0x18000000, // used while invalidating upper levels
  MESI_TRANS_INV_D      = 0x18000001, // used while invalidating upper levels,
                                      // and data is dirty (needs writeback)

  MESI_VALID_BIT        = 0x00100000, // data is valid
  MESI_DIRTY_BIT        = 0x00000001, // data is dirty (different from memory)

  MESI_MODIFIED         = 0x00300001, // data is dirty and writable 
  MESI_EXCLUSIVE        = 0x00300010, // data is present only in local cache
  MESI_SHARED           = 0x00100100  // data is shared
};

class MESIProtocol : public SMPProtocol {

protected:

public:  

  MESIProtocol(SMPCache *cache, const char *name); 
  ~MESIProtocol();

  void changeState(Line *l, unsigned newstate);
  void makeDirty(Line *l);
  void preInvalidate(Line *l);

  void read(MemRequest *mreq);
  void doRead(MemRequest *mreq);
  typedef CallbackMember1<MESIProtocol, MemRequest *, 
                         &MESIProtocol::doRead> doReadCB;

  void write(MemRequest *mreq);
  void doWrite(MemRequest *mreq);
  typedef CallbackMember1<MESIProtocol, MemRequest *, 
                         &MESIProtocol::doWrite> doWriteCB;
    
  //  void returnAccess(SMPMemRequest *sreq);

  void sendReadMiss(MemRequest *mreq);
  void sendWriteMiss(MemRequest *mreq);
  void sendInvalidate(MemRequest *mreq);

  void doSendReadMiss(MemRequest *mreq);
  void doSendWriteMiss(MemRequest *mreq);
  void doSendInvalidate(MemRequest *mreq);

  typedef CallbackMember1<MESIProtocol, MemRequest *,
                         &MESIProtocol::doSendReadMiss> doSendReadMissCB;
  typedef CallbackMember1<MESIProtocol, MemRequest *,
                         &MESIProtocol::doSendWriteMiss> doSendWriteMissCB;
  typedef CallbackMember1<MESIProtocol, MemRequest *,
                         &MESIProtocol::doSendInvalidate> doSendInvalidateCB;

  void sendReadMissAck(SMPMemRequest *sreq);
  void sendWriteMissAck(SMPMemRequest *sreq);
  void sendInvalidateAck(SMPMemRequest *sreq);

  void sendDisplaceNotify(PAddr addr, CallbackBase *cb);

  void readMissHandler(SMPMemRequest *sreq);
  void writeMissHandler(SMPMemRequest *sreq); 
  void invalidateHandler(SMPMemRequest *sreq); 

  typedef CallbackMember1<MESIProtocol, SMPMemRequest *,
                   &MESIProtocol::sendWriteMissAck> sendWriteMissAckCB;
  typedef CallbackMember1<MESIProtocol, SMPMemRequest *,
                   &MESIProtocol::sendInvalidateAck> sendInvalidateAckCB;
  typedef CallbackMember1<MESIProtocol, SMPMemRequest *,
                   &MESIProtocol::writeMissHandler> writeMissHandlerCB;
  typedef CallbackMember1<MESIProtocol, SMPMemRequest *,
                   &MESIProtocol::invalidateHandler> invalidateHandlerCB;

  void readMissAckHandler(SMPMemRequest *sreq); 
  void writeMissAckHandler(SMPMemRequest *sreq); 
  void invalidateAckHandler(SMPMemRequest *sreq); 

  // data related
  void sendData(SMPMemRequest *sreq);
  void dataHandler(SMPMemRequest *sreq);

  void combineResponses(SMPMemRequest *sreq, MESIState_t localState);
};

#endif //MESIPROTOCOL_H
