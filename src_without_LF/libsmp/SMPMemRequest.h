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

#ifndef SMPMEMREQUEST_H
#define SMPMEMREQUEST_H

#include "pool.h"
#include "MemRequest.h"
#include "SMPDebug.h"

class SMPMemRequest : public MemRequest {
private:

  static pool<SMPMemRequest> rPool;
  friend class pool<SMPMemRequest>;

protected:

  MemRequest *oreq;
  uint32_t state;
  MemObj *requestor;
  MemObj *supplier;
  bool found;
  bool needSnoop; 
  bool needData;
  bool writeDown;

  // recycling counter
  unsigned nUses;

  // callback, if needed; should be only used when there is no oreq
  CallbackBase *cb;

public:
  SMPMemRequest(); 
  ~SMPMemRequest(){
  }

  // BEGIN: MemRequest interface

  static SMPMemRequest *create(MemRequest *mreq, 
			       MemObj *reqCache, 
			       bool sendData);

  static SMPMemRequest *create(MemObj *reqCache, 
			       PAddr addr, 
			       MemOperation mOp,
			       bool needsWriteDown,
			       CallbackBase *cb);

  void incUses();
  void destroy();

  VAddr getVaddr() const;
  PAddr getPAddr() const;
  void  ack(TimeDelta_t lat);

  // END: MemRequest interface

  void setOriginalRequest(MemRequest *mreq);
  void setState(uint32_t st);
  void setRequestor(MemObj *reqCache);
  void setSupplier(MemObj *supCache);

  MemRequest  *getOriginalRequest();
  MemOperation getMemOperation();
  uint32_t getState();
  MemObj      *getRequestor();
  MemObj      *getSupplier();

  bool         needsData()  { return needData; }
  bool         needsSnoop() { return needSnoop; }
  void         noSnoop()    { needSnoop = false; }

  bool         isFound()  { return found; }
  void         setFound() { found = true; }

  bool         needsWriteDown() { return writeDown; }
  void         setWriteDown()   { writeDown = true; }
};

class SMPMemReqHashFunc {
public: 
  size_t operator()(const MemRequest *mreq) const {
    HASH<const char *> H;
    return H((const char *)mreq);
  }
};


#endif // SMPMEMREQUEST_H
