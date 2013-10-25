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

#ifndef SMEMORY_SYSTEM_H
#define SMEMORY_SYSTEM_H

#include "estl.h"
#include "MemorySystem.h"
#include "SMPProtocol.h"

class SMemorySystem : public MemorySystem {
private:
protected:

  MemObj *buildMemoryObj(const char *type, 
			 const char *section, 
			 const char *name);  

public:
  SMemorySystem(int32_t processorId);
  virtual ~SMemorySystem() {
  }
};

#endif // SMEMORY_SYSTEM_H

