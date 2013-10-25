#include "nanassert.h"

#include "MemorySystem.h"

#include "TLB.h"
#include <iostream>
using namespace std;

TLB::TLB(const char *section, bool dataTLB, int32_t i)
  : id(i)
{
  if (dataTLB)
    cache = TLBCache::create(section, "", "P(%d)_DTLB", i);
  else
    cache = TLBCache::create(section, "", "P(%d)_ITLB", i);

  I(cache);
}

TLB::~TLB()
{
  if( cache )
    cache->destroy();
}

int32_t TLB::translate(VAddr vAddr)
{
  if(GMemorySystem::calcPage(vAddr) == 0)
    return 0;

  TLBCache::CacheLine *cl = cache->readLine(GMemorySystem::calcPageAddr(vAddr));

  //GMSG(cl==0 && id==0, "[%llu] TLB MISS %lx", globalClock, vAddr>>Log2PageSize);
  if (cl == 0) 
    return -1;

  return cl->physicalPage;
}

void TLB::insert(VAddr vAddr, int32_t  phPage)
{
  if(GMemorySystem::calcPage(vAddr) == 0)
    return;

  //GMSG(id==0, "[%llu] TLB INS %lx", globalClock, vAddr>>Log2PageSize);
  TLBCache::CacheLine *cl = cache->fillLine(GMemorySystem::calcPageAddr(vAddr));
  cl->physicalPage = phPage;
}

