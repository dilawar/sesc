/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Karin Strauss
		  Luis Ceze

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

#ifndef DIRSTATE_H
#define DIRSTATE_H

#include "CacheCore.h"
#include "ThreadContext.h"
#include <vector>

class DirState {
private:

  typedef CacheGeneric<DirState, PAddr, true> CacheType;

  CacheType *cache;

  typedef std::vector<uint32_t> dirEntryType;

  dirEntryType presenceBits;

  static uint32_t linesPerEntry; // number of lines for each we are keeping presence info in a directory line
  
  PAddr tag;
  bool invalid;

protected:

public:

  DirState();
  virtual ~DirState();

  void initialize(CacheType *c);
  bool isLocked();

  PAddr getTag() const;
  void setTag(PAddr a);

  bool isInvalid() const { return invalid; }
  bool isValid() const { return !invalid; }

  void invalidate();

  void setBit(const uint32_t cacheId, const PAddr addr) {
    I(cacheId < 32);
    uint32_t index = addr & (linesPerEntry - 1);
    presenceBits[index] |= (1 << cacheId);
  }

  void resetBit(const uint32_t cacheId, const PAddr addr) {
    I(cacheId < 32);
    uint32_t index = addr & (linesPerEntry - 1);
    presenceBits[index] &= ~(1 << cacheId);
  }

  void setAllEntryBits() {
    for(uint32_t i = 0; i < linesPerEntry; i++)
      presenceBits[i] = (uint32_t) -1;
  }

  void setAllLineBits(const PAddr addr) {
    uint32_t index = addr & (linesPerEntry - 1);
    presenceBits[index] = (uint32_t) -1;
  }

  void resetAllEntryBits() {
    for(uint32_t i = 0; i < linesPerEntry; i++)
      presenceBits[i] = 0;
  }
  
  void resetAllLineBits(const PAddr addr) {
    uint32_t index = addr & (linesPerEntry - 1);
    presenceBits[index] = 0;
  }

  bool isPresent(const uint32_t cacheId, const PAddr addr) {
    I(cacheId < 32);
    uint32_t index = addr & (linesPerEntry - 1);
    return (presenceBits[index] & (1 << cacheId));
  }

  uint32_t whereIsPresent(const PAddr addr) {
    uint32_t index = addr & (linesPerEntry - 1);
    return presenceBits[index];
  }

  void dump(const char *str) {
    // Nothing
  }

};

#endif /* DIRSTATE_H */
