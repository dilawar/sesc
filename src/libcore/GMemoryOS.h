/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Basilio Fraguela
                  Jose Renau
                  Smruti Sarangi

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

#ifndef GMEMORYOS_H
#define GMEMORYOS_H

#include "nanassert.h"

#include "SescConf.h"

#include "ThreadContext.h"
#include "CacheCore.h"

class MemRequest;

class GMemoryOS {
private:
  const int32_t Id; // One OS instance per processor (1 per SMT too)
protected:
public:
  GMemoryOS(int32_t i) 
    : Id(i) {
  }
  virtual ~GMemoryOS() {
  }
  virtual int32_t TLBTranslate(VAddr vAddr) = 0;
  virtual int32_t ITLBTranslate(VAddr iAddr) = 0 ;
  virtual void solveRequest(MemRequest *r) = 0;
  virtual void boot() = 0;
  virtual void report(const char *str) = 0;
};

class DummyMemoryOS : public GMemoryOS {
private:
  class DTLBState : public StateGeneric<> {
  public:
	 int32_t  physicalPage;

	 DTLBState(int32_t iphysicalPage = -1) {  physicalPage = iphysicalPage; }

    bool operator==(DTLBState s) const {
      return physicalPage == s.physicalPage;
    }
  };

  typedef CacheGeneric<DTLBState> DTLBCache;
  DTLBCache *itlb;
  DTLBCache *dtlb;
public:
  
  DummyMemoryOS(int32_t i);
  virtual ~DummyMemoryOS();

  int32_t ITLBTranslate(VAddr iAddr);
  int32_t TLBTranslate(VAddr vAddr);

  void solveRequest(MemRequest *r);

  void boot();
  void report(const char *str);
};

#endif /* GMEMORYOS_H */
