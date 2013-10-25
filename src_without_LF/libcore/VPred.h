/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2005 University of Illinois.

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

#ifndef VPRED_H
#define VPRED_H

#include "GStats.h"
#include "SCTable.h"
#include "ThreadContext.h"

#include <deque> 

typedef unsigned long long ValueType;

class GValuePredictor {
 protected:
  GStatsCntr hit;
  GStatsCntr miss;
  GStatsCntr onlyHit;
  unsigned brHist;
 public:
  GValuePredictor(const char *name);
  virtual ~GValuePredictor() { }
  virtual bool isValueHit(PAddr addr, PAddr iPC, ValueType value) = 0;
  virtual bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true) = 0;
  virtual void putStore(PAddr addr, PAddr iPC, ValueType value) { }
  virtual ValueType getValue(PAddr addr, PAddr iPC) { return 0; }
  virtual void incOnlyHit() { onlyHit.inc(); }
  virtual void setBrHist(int32_t bo) {  brHist = ((brHist << 1) | !!bo); }
};

class Selector {
 private:
  SCTable table;
  GStatsCntr hit;
  GStatsCntr miss;
  GStatsCntr phit;
  GStatsCntr pmiss;
  GValuePredictor *vp1;
  GValuePredictor *vp2;

 public:
  Selector(int32_t size, const char *name, GValuePredictor *p1, GValuePredictor *p2);
  ~Selector() { }
  bool predict(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
  ValueType getValue(PAddr addr, PAddr iPC);
  void update(PAddr addr, PAddr iPC, ValueType value, bool updateVP=false);
  unsigned getIndex(PAddr iPC);
  void setBrHist(int32_t bo) { vp1->setBrHist(bo); vp2->setBrHist(bo); }
};

class ConfidenceEstimator {
 protected:
  SCTable table;
  const bool useBH;
  GStatsCntr estHit;
  GStatsCntr estMiss;
  unsigned brHist;

 public:
  ConfidenceEstimator(int32_t size, const char *name, bool ubh);
  ~ConfidenceEstimator() { }
  unsigned getIndex(PAddr iPC);
  bool doPrediction(PAddr iPC, bool vhit);
  void setBrHist(int32_t bo) {  brHist = ((brHist << 1) | !!bo); }
};


class ZeroPred : public GValuePredictor {
 public:
  ZeroPred(const char *name = "Z");
  virtual ~ZeroPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
};

class GlobalLVPred : public GValuePredictor {
  unsigned lastValue;
 public:
  GlobalLVPred(const char *name = "GLV");
  virtual ~GlobalLVPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
  ValueType getValue(PAddr addr, PAddr iPC) { return lastValue; }
};

class LVPred : public GValuePredictor {
  unsigned nEntries;
  ValueType *LVPT;
  
 public:
  LVPred(unsigned size = 16, const char *name = "LV");
  virtual ~LVPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
};

class ELVPred : public GValuePredictor {
  unsigned nEntries;
  ValueType *LVPT;
  
 public:
  ELVPred(unsigned size = 16, const char *name = "ELV");
  virtual ~ELVPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
};

class BHLVPred : public GValuePredictor {
  unsigned nEntries;
  ValueType *LVPT;
  
 public:
  BHLVPred(unsigned size = 16, const char *name = "BHLV");
  virtual ~BHLVPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
  ValueType getValue(PAddr addr, PAddr iPC);
};

class SPred : public GValuePredictor {
  unsigned nEntries;
  ValueType *LVPTa;
  ValueType *LVPTb;
  
 public:
  SPred(unsigned size = 16, const char *name = "S");
  virtual ~SPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
};

class FCMPred : public GValuePredictor {
  unsigned nEntries;
  unsigned *HT;
  unsigned *VT;
  ValueType *v1, *v2, *v3 ,*v4;
  
 public:
  FCMPred(unsigned size = 16, const char *name = "FCM");
  virtual ~FCMPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc=true);
};


class FWPred : public GValuePredictor {
  typedef HASH_MULTIMAP<PAddr, PAddr> StoreMap;
  typedef std::deque<StoreMap::iterator> StoreQueue;

  StoreMap storeMap;
  StoreQueue storeQueue;

  const unsigned storeQueueSize;

  unsigned nEntries;
  unsigned *DPT;
  ValueType *VPT;

 public:
  FWPred(unsigned size = 16, const char *name = "FW");
  virtual ~FWPred() { }
  bool isValueHit(PAddr addr, PAddr iPC, ValueType value);
  bool putValue(PAddr addr, PAddr iPC, ValueType value,  bool countAcc=true);
  void putStore(PAddr addr, PAddr iPC, ValueType value);
};

#endif //VPRED_H
