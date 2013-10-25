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

#ifndef VMEMREQ_H
#define VMEMREQ_H

#include "pool.h"

#include "ThreadContext.h"
#include "HVersion.h"

#include "VMemState.h"

// This protocol is not fully race-free (ejem!!!). It would be nice if
// someone verifies it. To do it, it would be necesary to hace a
// PushLineAck type of message (the ack for AskPushLine is pushLine).
//
// Additionaly, it would be nice if it were fault tolerant. To do so,
// it would be necessary to include a NACK (retry again), and a
// timemout.
//

enum VMemReqType {
  VRead,
  VReadAck,
  VWriteCheck,
  VWriteCheckAck,
  VPushLine,
  VPushLineAck,
  VAskPushLine,
  VMaxRequest
};

class VMemObj;
class MemRequest;
class LVID;


class VMemReq {
protected:
  VMemReqType type;
  PAddr paddr; // Version caches have their own LPAddr (LPAddr may be different
	       // for each cache. It needs to be recalculated locally

  VMemObj  *vmem; // Originator
  HVersion *ver;

  TimeDelta_t lat;

  int32_t  nPendingMsg;

  VMemState state;

#ifdef DEBUG
  static uint32_t snCount; // serial number counter
  uint32_t serialNumber;   // serial number itself
#endif

  int32_t nReq;
  
public:
  TimeDelta_t getLatency() const { return lat; }
  void setLatency(TimeDelta_t l) { lat = l; }
  void clearLatency() { lat = 0; }

  void init(VMemReqType t, VMemObj *m, HVersion *v, PAddr p);

  VMemReqType getType() const { return type;  }
  PAddr getPAddr() const      { return paddr; }
  VMemObj *getVMem() const    { return vmem;  }

  void  changeVersion(HVersion *v);
  const HVersion *getVersionRef() const { return ver; }
  HVersion *getVersion() const { return ver; }
  HVersion *getVersionDuplicate() const { return ver->duplicate(); }
  const VMemState *getStateRef() const  { return &state; }
  VMemState *getState()       { return &state; }

  void incPendingMsg() {
    I(type == VRead || type == VWriteCheck || type == VAskPushLine);
    I(nPendingMsg>=0);
    nPendingMsg++;
  }
  void decPendingMsg() {
    I(type == VRead || type == VWriteCheck || type == VAskPushLine);
    I(nPendingMsg>0);
    nPendingMsg--;
  }
  bool hasPendingMsg() const { 
    I(type == VRead || type == VWriteCheck || type == VAskPushLine);
    I(nPendingMsg>=0);
    return nPendingMsg != 0;
  }

  int32_t getnRequests()  const {return nReq; }
  void incnRequests() { nReq++; }
  void decnRequests() { nReq--; }
  
#ifdef DEBUG
  uint32_t getSerialNumber() const { return serialNumber; }
#else
  uint32_t getSerialNumber() const { return 0; }
#endif
};

class VRWReq : public VMemReq {
 protected:
  MemRequest  *mreq;
  VRWReq      *origReq;  // original request
  bool     memSentData;  // data supplied by memory?
  
  bool  cacheSentData; // data found in caches? or should we use response from memory?
 public:
  MemRequest *getMemRequest() const {
    I(mreq);
    return mreq;
  }

  bool hasMemRequestPending() const { return mreq != 0; }
  void markMemRequestAck() {
    I(mreq);
    mreq = 0; // mreq is no longer valid
  }
  bool hasCacheSentData() const { return cacheSentData; }
  void setCacheSentData() {
    cacheSentData = true;
  }

  bool hasMemSentData() const { return memSentData; }
  void setMemSentData() {
    memSentData = true;
  }
};

class VMemReadReq : public VRWReq {
private:
  static pool<VMemReadReq> rPool;
  friend class pool<VMemReadReq>;
  
protected:
  VMemObj *origCache;    // requestor cache
  bool     wait4PushLine; // Did the read request wait for a pushLine to finish?
public:
  static VMemReadReq *createRead(VMemObj *origCache, HVersion *ver, MemRequest *mreq);
  static VMemReadReq *createReadAck(VMemObj *origCache, VMemReadReq *origReq, HVersion *verAck);

  void destroy();

  VMemReadReq *getOrigRequest() const { return static_cast<VMemReadReq *>(origReq); } 

  VMemObj *getOrigCache() const {
    I(getType() == VRead || getType() == VReadAck);
    I(origCache);
    return origCache;
  }

  bool hasWait4PushLine() const { 
    I(type == VReadAck);
    return wait4PushLine; 
  }
  void setWait4PushLine() {
    I(type == VReadAck);
    wait4PushLine = true;
  }
  void clearWait4PushLine() {
    I(type == VReadAck);
    I(wait4PushLine == true);
    wait4PushLine = false;
  }

};

class VMemWriteReq : public VRWReq {
private:
  static pool<VMemWriteReq> rPool;
  friend class pool<VMemWriteReq>;

protected:
  bool lastMsg;
  bool writeHit;

public:
  static VMemWriteReq *createWriteCheck(VMemObj *c, HVersion *v
					,PAddr paddr, MemRequest *mreq, bool wHit);
  
  static VMemWriteReq *createWriteCheckAck(VMemObj *c, HVersion *v
					   ,VMemState *state, VMemWriteReq *vreq);

  bool isWriteHit() const { return writeHit; }

  void destroy();

  VMemWriteReq *getOrigRequest() const { return static_cast<VMemWriteReq *>(origReq); } 
  
  bool isLastMessage() const     { return lastMsg; }
  void setLastMessage(bool flag) { lastMsg = flag; }
};

class VMemPushLineReq : public VMemReq {
private:
  static pool<VMemPushLineReq> rPool;
  friend class pool<VMemPushLineReq>;

  VMemPushLineReq *askPushReq;
protected:
  bool noMoreSharers;

public:
  static VMemPushLineReq *createPushLine(VMemObj *origCache
					 ,HVersion *v
					 ,PAddr paddr
					 ,const VMemState *state
					 ,bool noMoreLocalSharers
					 ,VMemPushLineReq *askPushReq
					 );

  static VMemPushLineReq *createAskPushLine(VMemObj *origCache
					    ,HVersion *v
					    ,PAddr paddr
					    ,const VMemState *state);

  bool isLastSharer() const {
    I(type == VPushLine);
    // If the pushLine got forwarded this question is no valid
    return noMoreSharers;
  }

  VMemPushLineReq *getAskPushReq() const {
    I(type == VPushLine);
    return askPushReq;
  }

  void convert2Ack();
  void destroy();
};

#endif // VMEMREQ_H
