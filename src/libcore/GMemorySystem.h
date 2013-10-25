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
#ifndef GMEMORYSYSTEM_H
#define GMEMORYSYSTEM_H

#include "estl.h"

#include "GMemoryOS.h"

#include "MemObj.h"
#include "EnergyMgr.h"

#ifdef TASKSCALAR
#include "GLVID.h"
#endif

class HVersion;

//Class for comparison to be used in hashes of char * where the
//content is to be compared
class MemObjCaseeqstr {
public:
  inline bool operator()(const char* s1, const char* s2) const {
    return strcasecmp(s1, s2) == 0;
  }
};  

class MemoryObjContainer {
private:
  typedef HASH_MAP<const char *, MemObj *, HASH<const char*>, MemObjCaseeqstr> StrToMemoryObjMapper;
  StrToMemoryObjMapper intlMemoryObjContainer;

public:
  void addMemoryObj(const char *device_name, MemObj *obj);

  MemObj *searchMemoryObj(const char *section, const char *name) const;
  MemObj *searchMemoryObj(const char *name) const;

  void clear();
};

class GMemorySystem {
private:
  static ushort Log2PageSize;
  static uint32_t PageMask;

  typedef HASH_MAP<const char*, uint32_t, HASH<const char*>, MemObjCaseeqstr > StrCounterType;
  static StrCounterType usedNames;

  static MemoryObjContainer sharedMemoryObjContainer;
  MemoryObjContainer *localMemoryObjContainer;

  const MemoryObjContainer *getMemoryObjContainer(bool shared) const {
    MemoryObjContainer *mo = shared ? &sharedMemoryObjContainer : localMemoryObjContainer;
    I(mo);
    return mo;
  }

  MemoryObjContainer *getMemoryObjContainer(bool shared) {
    MemoryObjContainer *mo = shared ? &sharedMemoryObjContainer : localMemoryObjContainer;
    I(mo);
    return mo;
  }

  MemObj *instrSource;
  MemObj *dataSource;
  GMemoryOS *memoryOS;

protected:
  const int32_t Id;

  char *buildUniqueName(const char *device_type);

  static char *privatizeDeviceName(char *given_name, int32_t num);

  virtual GMemoryOS *buildMemoryOS(const char *section);
  virtual MemObj *buildMemoryObj(const char *type, const char *section, const char *name);

public:
  GMemorySystem(int32_t processorId);
  virtual ~GMemorySystem();

  // The code can not be located in constructor because it is nor possible to
  // operate with virtual functions at construction time
  virtual void buildMemorySystem();

  static int32_t  getPageSize()  { return 1<<Log2PageSize; }
  static int32_t  calcPage(PAddr paddr)  { return paddr >> Log2PageSize; }
  static PAddr calcPAddr4Page(int32_t p) { return p << Log2PageSize; }
  static int32_t  calcFullPage(int32_t p) { return p << Log2PageSize; }
  static int32_t  calcPageMask(int32_t p) { return p & PageMask; }
  static int32_t  calcPageAddr(VAddr va) { return va & ~PageMask; }
  static PAddr calcPAddr(int32_t p, VAddr a) {
    I((p & PageMask) == 0);
    return p | (a & PageMask); 
  }

  MemObj *searchMemoryObj(bool shared, const char *section, const char *name) const;
  MemObj *searchMemoryObj(bool shared, const char *name) const;

  MemObj *declareMemoryObj(const char *block, const char *field);
  
  int32_t getId() const;
  
  MemObj *getDataSource()  const;
  MemObj *getInstrSource() const;
  GMemoryOS *getMemoryOS() const;
#ifdef TASKSCALAR
  virtual GLVID *findCreateLVID(HVersion *ver) = 0;
#endif

};

class DummyMemorySystem : public GMemorySystem {
private:
#ifdef TASKSCALAR
  GLVIDDummy glvid;
#endif
protected:
public:
  DummyMemorySystem(int32_t id);
  ~DummyMemorySystem();

#ifdef TASKSCALAR
  GLVID *findCreateLVID(HVersion *ver);
#endif
};

#endif /* GMEMORYSYSTEM_H */
