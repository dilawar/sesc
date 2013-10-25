/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Basilio Fraguela
                  Jose Renau
                  James Tuck
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

#include <math.h>
#include "GMemorySystem.h"

ushort GMemorySystem::Log2PageSize=0;
uint32_t GMemorySystem::PageMask;

MemoryObjContainer GMemorySystem::sharedMemoryObjContainer;
GMemorySystem::StrCounterType  GMemorySystem::usedNames;


//////////////////////////////////////////////
// MemoryObjContainer

void MemoryObjContainer::addMemoryObj(const char *device_name, MemObj *obj) 
{
  intlMemoryObjContainer[device_name] = obj;
}

MemObj *MemoryObjContainer::searchMemoryObj(const char *descr_section, 
					    const char *device_name) const
{
  I(descr_section);
  I(device_name);

  StrToMemoryObjMapper::const_iterator it = intlMemoryObjContainer.find(device_name);

  if (it != intlMemoryObjContainer.end()) {

    const char *descrSection=(*it).second->getDescrSection();

    if (strcasecmp(descrSection, descr_section)) {

      MSG("Two versions of MemoryObject [%s] with different definitions [%s] and [%s]", 
	  device_name, descrSection, descr_section);
      exit(-1);

    }

    return (*it).second;
  }

  return NULL;
}

/* Only returns a pointer if there is only one with that name */
MemObj *MemoryObjContainer::searchMemoryObj(const char *device_name) const
{
  I(device_name);

  if(intlMemoryObjContainer.count(device_name) != 1) 
    return NULL;

  return (*(intlMemoryObjContainer.find(device_name))).second;
}

void MemoryObjContainer::clear()
{ 
  intlMemoryObjContainer.clear(); 
}

//////////////////////////////////////////////
// GMemorySystem

GMemorySystem::GMemorySystem(int32_t processorId) 
  :Id(processorId) 
{
  localMemoryObjContainer = new MemoryObjContainer();

  if (!Log2PageSize) {
    SescConf->isPower2("", "pageSize");
    SescConf->isGT("", "pageSize", 2048);

	 uint32_t page_size = SescConf->getInt("", "pageSize");
    Log2PageSize = log2i(page_size);
    PageMask = (1 << Log2PageSize) - 1;
  }

  dataSource = 0;
  instrSource= 0;
}

GMemorySystem::~GMemorySystem() 
{
  if (dataSource)
    delete dataSource;
  if (instrSource && dataSource != instrSource)
    delete instrSource;
  delete localMemoryObjContainer;
}

GMemoryOS *GMemorySystem::buildMemoryOS(const char *section)
{
  const char *osType = SescConf->getCharPtr(section, "OSType");

  if (!(strcasecmp(osType, "dummy") == 0
	|| strcasecmp(osType, "std") == 0 
	)) {
    MSG("Invalid OStype [%s]", osType);
  }

  
  return new DummyMemoryOS(Id);
}

MemObj *GMemorySystem::buildMemoryObj(const char *type, const char *section, const char *name)
{
  if (!(strcasecmp(type, "dummy") == 0
	|| strcasecmp(type, "cache") == 0 
	|| strcasecmp(type, "mvcache") == 0 
	|| strcasecmp(type, "icache") == 0 
	|| strcasecmp(type, "smpcache") == 0 
	)) {
    MSG("Invalid memory type [%s]", type);
  }

  return new DummyMemObj(section, name);
}

void GMemorySystem::buildMemorySystem()
{
  SescConf->isCharPtr("", "cpucore", Id);

  const char *def_block = SescConf->getCharPtr("", "cpucore", Id);

  dataSource = declareMemoryObj(def_block, "dataSource");
  if (dataSource) {
    dataSource->setHighestLevel();
    dataSource->computenUpperCaches();
  }

  instrSource = declareMemoryObj(def_block, "instrSource");
  if (instrSource && dataSource != instrSource) {
    instrSource->setHighestLevel();
    instrSource->computenUpperCaches();
  }

  memoryOS = buildMemoryOS(def_block);
}

char *GMemorySystem::buildUniqueName(const char *device_type)
{ 
  int32_t num;

  StrCounterType::iterator it = usedNames.find(device_type);
  if (it == usedNames.end()) {

    usedNames[device_type] = 0;
    num = 0;

  } else
    num = ++(*it).second;

  size_t size = strlen(device_type);
  char *ret = (char*)malloc(size + 6 + (int)log10((float)num+10));
  sprintf(ret,"%s(%d)", device_type, num);

  return ret;
}

char *GMemorySystem::privatizeDeviceName(char *given_name, int32_t num)
{ 
  char *ret=new char[strlen(given_name) + 8 + (int)log10((float)num+10)];

  sprintf(ret,"P(%i)_%s", num, given_name);

  delete[] given_name;

  return ret;
}

MemObj *GMemorySystem::searchMemoryObj(bool shared, const char *section, const char *name) const
{
  return getMemoryObjContainer(shared)->searchMemoryObj(section, name);
}

MemObj *GMemorySystem::searchMemoryObj(bool shared, const char *name) const
{
  return getMemoryObjContainer(shared)->searchMemoryObj(name);
}

MemObj *GMemorySystem::declareMemoryObj(const char *block, const char *field)
{ 
  bool shared = false; // Private by default
  SescConf->isCharPtr(block, field);
  
  std::vector<char *> vPars = SescConf->getSplitCharPtr(block, field);

  if (!vPars.size()) {
    MSG("Section [%s] field [%s] does not describe a MemoryObj\n", 
	block, field);
    MSG("Required format: memoryDevice = descriptionSection [name] [shared|private]\n");
    SescConf->notCorrect();
    I(0);
    return 0; // Known-error mode
  }

  const char *device_descr_section = vPars[0];
  char *device_name = (vPars.size() > 1) ? vPars[1] : 0;

  if (vPars.size() > 2) {

    if (strcasecmp(vPars[2], "shared") == 0) {

      I(vPars.size() == 3);
      shared = true;

    } else if (strcasecmp(vPars[2], "sharedBy") == 0) {

      I(vPars.size() == 4);
      int32_t sharedBy = atoi(vPars[3]);
      delete[] vPars[3];
      GMSG(sharedBy <= 0,
	   "SharedBy should be bigger than zero (field %s)",
	   device_name);

      int32_t nId = Id / sharedBy;
      device_name = privatizeDeviceName(device_name, nId);
      shared = true;
    }

    delete[] vPars[2];

  } else if (device_name) {

    if (strcasecmp(device_name, "shared") == 0) {
      delete[] device_name;
      device_name = 0;
      shared = true;
    }

  }

  SescConf->isCharPtr(device_descr_section, "deviceType");
  const char *device_type = SescConf->getCharPtr(device_descr_section, 
						 "deviceType");

  /* If the device has been given a name, we may be refering to an
   * already existing device in the system, so let's search
   * it. Anonymous devices (no name given) are always unique, and only
   * one reference to them may exist in the system.
   */

  if (device_name) {

    if (!shared)
      device_name = privatizeDeviceName(device_name, Id);

    MemObj *memdev = searchMemoryObj(shared, device_descr_section, device_name);

    if (memdev) {
      delete[] device_name;
      delete[] device_descr_section;
      return memdev;
    }

  } else
    device_name = buildUniqueName(device_type);

  MemObj *newMem = buildMemoryObj(device_type, 
				  device_descr_section, 
				  device_name);

  if (newMem) // Would be 0 in known-error mode
    getMemoryObjContainer(shared)->addMemoryObj(device_name, newMem);

  return newMem;
}

int32_t GMemorySystem::getId() const
{ 
  return Id;
}

MemObj *GMemorySystem::getDataSource()  const 
{ 
  return dataSource;
}

MemObj *GMemorySystem::getInstrSource() const 
{ 
  return instrSource;
}

GMemoryOS *GMemorySystem::getMemoryOS() const 
{ 
  return memoryOS;
}

DummyMemorySystem::DummyMemorySystem(int32_t id) 
  : GMemorySystem(id) 
{
  // Do nothing
}

DummyMemorySystem::~DummyMemorySystem() 
{
  // Do nothing
}

#ifdef TASKSCALAR
GLVID *DummyMemorySystem::findCreateLVID(HVersion *ver) 
{
  return &glvid;
}
#endif
