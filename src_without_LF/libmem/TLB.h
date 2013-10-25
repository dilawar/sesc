#ifndef TLB_H
#define TLB_H

#include "Snippets.h"

#include "ThreadContext.h"
#include "CacheCore.h"

class TLB {
private:
  class TLBState : public StateGeneric<> {
  public:
    int32_t  physicalPage;

    TLBState(int32_t iphysicalPage = -1) {  
      physicalPage = iphysicalPage; 
    }

    bool operator==(TLBState s) const {
      return physicalPage == s.physicalPage;
    }
  };

  typedef CacheGeneric<TLBState, VAddr> TLBCache;

  const ushort id;
  TLBCache *cache;

public:
  TLB(const char *section, bool dataTLB, int32_t i);
  ~TLB();

  int32_t translate(VAddr vAddr);
  void insert(VAddr vAddr, int32_t  phPage);
};

#endif

