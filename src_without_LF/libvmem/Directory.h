/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss

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

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "DirState.h"
#include "nanassert.h"

class Directory {
private:
  typedef CacheGeneric<DirState, PAddr, true>            CacheType;
  typedef CacheGeneric<DirState, PAddr, true>::CacheLine Line;

  CacheType *cache;

  uint32_t corrShift; // if info can be kept in less than 1 byte 
                          // (only 2 and 4 bits allowed), there needs 
                          // to be corrections in the addressing 
                          // (cache granularity is 1 byte)

protected:
public:

  Directory(const char *section, const char *name);
  ~Directory();
  
  // marks addr as present in cacheId
  void setPresentIn(const uint32_t cacheId, PAddr addr);   

  // marks addr as not present in cacheId
  void resetPresentIn(const uint32_t cacheId, PAddr addr); 

  void setAllPresent(PAddr addr);   // marks addr as present in all caches
  void resetAllPresent(PAddr addr); // marks addr as not present in any cache

  // retrieves information for a certain address from directory
  bool getInfoForLine(PAddr addr, uint32_t *info) const;

  // checks if line may be present in cache 
  bool isPresent(const uint32_t cacheId, uint32_t info); 

  // returns a mask with caches in which line is present
  uint32_t whereIsPresent(PAddr addr); 
  
};

#endif /* DIRECTORY_H */
