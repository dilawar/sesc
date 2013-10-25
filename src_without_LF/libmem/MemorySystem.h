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

#ifndef MEMORYSYSTEM_H
#define MEMORYSYSTEM_H

#include <string.h>

#include "GMemorySystem.h"

class MemObj;

class MemorySystem : public GMemorySystem {
private:
#ifdef TASKSCALAR
  GLVIDDummy glvid;
#endif
  void calcGlobalParameters();
protected:
  virtual MemObj *buildMemoryObj(const char *type, const char *section, const char *name);
  virtual GMemoryOS *buildMemoryOS(const char *descr_section);

  uint32_t procsPerNode;
  uint32_t pID;

public:
  // old Intf. MemorySystem(const char *descr_section = "memHierarchy");
  MemorySystem(int32_t processorId);

#ifdef TASKSCALAR
  GLVID *findCreateLVID(HVersion *ver);
#endif
};

#define k_lowerLevel "lowerLevel"
#define k_void       "void"
#define k_deviceType "deviceType"
#define k_OSType     "OSType"

#endif
