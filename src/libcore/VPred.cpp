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

#include "VPred.h"

GValuePredictor::GValuePredictor(const char *name) 
  : hit("VP_%s:hit", name)
  ,miss("VP_%s:miss", name)
  ,onlyHit("VP_%s:onlyHit", name) 
{ 
  brHist = 0;
}

ZeroPred::ZeroPred (const char *name) :
  GValuePredictor(name)
{
}
bool ZeroPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  if(value == 0) 
    return true;

  return false;
}

bool ZeroPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);
  if(countAcc) {
    if(vhit) {
      hit.inc();
    } else {
      miss.inc();
    }
  }
  return vhit;
}

GlobalLVPred::GlobalLVPred(const char *name) :
  GValuePredictor(name)
{
  lastValue = 0;
}

bool GlobalLVPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  if(value == lastValue) 
    return true;
  
  return false;
}

bool GlobalLVPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);

  if(countAcc) {
    if(vhit) 
      hit.inc();
    else 
      miss.inc();
  }
  lastValue = value;
  return vhit;
}

LVPred::LVPred(unsigned size, const char *name) :
  GValuePredictor(name)
{
  nEntries = size;
  LVPT = new ValueType[nEntries];
  for(unsigned i = 0; i < nEntries; i++)
    LVPT[i] = 0;
}

bool LVPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  unsigned index = (iPC >> 2) % nEntries;

  if(value == LVPT[index]) 
    return true;

  return false;
}

bool LVPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);
  unsigned index = (iPC >> 2) % nEntries;

  if(countAcc) {
    if(vhit) 
      hit.inc();
    else
      miss.inc();
  }

  LVPT[index] = value;
  return vhit;
}

ELVPred::ELVPred(unsigned size, const char *name) :
  GValuePredictor(name)
{
  nEntries = size;
  LVPT = new ValueType[nEntries];
  for(unsigned i = 0; i < nEntries; i++)
    LVPT[i] = 0;
}

bool ELVPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  unsigned index = ((iPC ^ addr) >> 2) % nEntries;

  if(value == LVPT[index]) 
    return true;
  
  return false;
}

bool ELVPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);
  unsigned index = ((iPC ^ addr) >> 2) % nEntries;

  if(countAcc) {
    if(vhit) 
      hit.inc();
    else
      miss.inc();
  }

  LVPT[index] = value;
  return vhit;
}

BHLVPred::BHLVPred(unsigned size, const char *name) :
  GValuePredictor(name)
{
  nEntries = size;
  LVPT = new ValueType[nEntries];
  for(unsigned i = 0; i < nEntries; i++)
    LVPT[i] = 0;
}

bool BHLVPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  unsigned index = ((iPC ^ brHist) >> 2) % nEntries;

  if(value == LVPT[index]) 
    return true;

  return false;
}

bool BHLVPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);
  unsigned index = ((iPC ^ brHist) >> 2) % nEntries;

  if(countAcc) {
    if(vhit) 
      hit.inc();
    else 
      miss.inc();
  }

  LVPT[index] = value;
  return vhit;
}

ValueType BHLVPred::getValue(PAddr addr, PAddr iPC)
{
  unsigned index = ((iPC ^ brHist) >> 2) % nEntries;
  return LVPT[index];
}

SPred::SPred(unsigned size, const char *name) :
  GValuePredictor(name)
{
  nEntries = size;
  LVPTa = new ValueType[nEntries];
  LVPTb = new ValueType[nEntries];
  
  for(unsigned i = 0; i < nEntries; i++) {
    LVPTa[i] = 0;
    LVPTb[i] = 0;
  }
}

bool SPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  unsigned index = (iPC >> 2) % nEntries;
  ValueType a = LVPTa[index];
  ValueType b = LVPTb[index];
  ValueType pred;
  pred = 2*b-a;

  if(value == pred) 
    return true;

  return false;
}

bool SPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);
  unsigned index = (iPC >> 2) % nEntries;

  if(countAcc) {
    if(vhit) 
      hit.inc();
    else 
      miss.inc();
  }

  LVPTa[index] = LVPTb[index];
  LVPTb[index] = value;
  return vhit;
}

FCMPred::FCMPred(unsigned size, const char *name) :
  GValuePredictor(name)
{
  nEntries = size;
  HT = new unsigned[nEntries];
  VT = new unsigned[nEntries];
  v1 = new ValueType[nEntries];
  v2 = new ValueType[nEntries];
  v3 = new ValueType[nEntries];
  v4 = new ValueType[nEntries];

  
  for(unsigned i = 0; i < nEntries; i++) {
    VT[i] = 0;
    HT[i] = 0;
    v1[i] = 0;
    v2[i] = 0;
    v3[i] = 0;
    v4[i] = 0;
  }
}

bool FCMPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  unsigned hIndex = (iPC >> 2) % nEntries;
  unsigned index = HT[hIndex] % nEntries;
  
  if(value == VT[index])
    return true;

  return false;
}

bool FCMPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);
  unsigned hIndex = (iPC >> 2) % nEntries;
  unsigned index = HT[hIndex] % nEntries;
  
  vhit = (value == VT[index]);

  if(countAcc) {
    if(vhit) {
      hit.inc();
    } else {
      miss.inc();
    }
  }
  
  VT[index] = value;

  v1[hIndex] = v2[hIndex];
  v2[hIndex] = v2[hIndex];
  v3[hIndex] = v4[hIndex];
  v4[hIndex] = value;
  index = v1[hIndex] ^ v2[hIndex] ^ v3[hIndex] ^ v4[hIndex];
  
  HT[hIndex] ^= index;
  return vhit;
}


FWPred::FWPred(unsigned size, const char *name) :
  GValuePredictor(name),
  storeQueueSize(2048)
{
  nEntries = size;
  VPT = new ValueType[nEntries];
  DPT = new unsigned[nEntries];

  for(unsigned i = 0; i < nEntries; i++) {
    DPT[i] = 0;
    VPT[i] = 0;
  }
}

bool FWPred::isValueHit(PAddr addr, PAddr iPC, ValueType value)
{
  unsigned index = ((iPC ^ brHist) >> 2) % nEntries;

  if(value == VPT[index]) 
    return true;

  return false;
}

bool FWPred::putValue(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{
  bool vhit = isValueHit(addr, iPC, value);
  unsigned vptIndex = ((iPC ^ brHist) >> 2) % nEntries;

  StoreMap::iterator it = storeMap.find(addr);
  if(it != storeMap.end()) {
    unsigned dptIndex = ((it->second ^ addr)>> 2) % nEntries;
    DPT[dptIndex] = vptIndex;
  }

  if(countAcc) {
    if(vhit) 
      hit.inc();
    else
      miss.inc();
  }

  VPT[vptIndex] = value;

  return vhit;
}

void FWPred::putStore(PAddr addr, PAddr iPC, ValueType value)
{
  StoreMap::iterator it = storeMap.insert(std::make_pair(addr, iPC));
  storeQueue.push_back(it);

  if(storeQueue.size() > storeQueueSize) {
    StoreMap::iterator fit = storeQueue.front();
    storeMap.erase(fit);
    storeQueue.pop_front();
  }

  unsigned dptIndex = ((iPC ^ addr) >> 2) % nEntries;
  unsigned vptIndex = DPT[dptIndex];

  //MSG("FWPred: i %d v %08x", vptIndex, value);

  VPT[vptIndex] = value;
}

Selector::Selector(int32_t size, const char *name, GValuePredictor *p1, GValuePredictor *p2) 
  :  table(0, name, size)
    ,hit("VPSEL_%s:hit", name)
    ,miss("VPSEL_%s:miss", name)
    ,phit("VPSEL_%s:phit", name)
    ,pmiss("VPSEL_%s:pmiss", name)
{
  vp1 = p1;
  vp2 = p2;
}

unsigned Selector::getIndex(PAddr iPC)
{
  unsigned index;

  return (iPC >> 2);
}

bool Selector::predict(PAddr addr, PAddr iPC, ValueType value, bool countAcc)
{ 
  bool vhit1 = vp1->isValueHit(addr, iPC, value);
  bool vhit2 = vp2->isValueHit(addr, iPC, value);
  bool vhit;

  vhit = table.predict(getIndex(iPC)) ? vhit1 : vhit2;

  if(countAcc) {
    if(vhit)
      hit.inc();
    else
      miss.inc();

    if(vhit1 || vhit2)
      phit.inc();
    else
      pmiss.inc();
  }

  return vhit;
}

ValueType Selector::getValue(PAddr addr, PAddr iPC)
{
  ValueType value;
  if(table.predict(getIndex(iPC)))
    value = vp1->getValue(addr, iPC);
  else
    value = vp2->getValue(addr, iPC);

  return value;
}

void Selector::update(PAddr addr, PAddr iPC, ValueType value, bool updateVP)
{
  bool vhit1 = vp1->isValueHit(addr, iPC, value);
  bool vhit2 = vp2->isValueHit(addr, iPC, value);

  if(vhit1 && !vhit2)
    table.predict(getIndex(iPC), true);
  else if(!vhit1 && vhit2)
    table.predict(getIndex(iPC), false);

  if(updateVP) {
    vp1->putValue(addr, iPC, value, false);
    vp2->putValue(addr, iPC, value, false);
  }
}

ConfidenceEstimator::ConfidenceEstimator(int32_t size, const char *name, bool ubh) 
  : table(0, name, size)
    ,useBH(ubh)
    ,estHit("VPCE_%s:estHit", name)
    ,estMiss("VPCE_%s:estMiss", name)
{
  brHist = 0;
}
 
unsigned ConfidenceEstimator::getIndex(PAddr iPC)
{
  unsigned index;

  if(useBH)
    index = ((iPC ^ brHist) >> 2);
  else
    index = (iPC >> 2);
  
  return index;
}

bool ConfidenceEstimator::doPrediction(PAddr iPC, bool vhit)
{
  bool dp = table.predict(getIndex(iPC), vhit);

  if((dp && vhit) || (!dp &&!vhit))
    estHit.inc();
  else
    estMiss.inc();

  return dp;
}

