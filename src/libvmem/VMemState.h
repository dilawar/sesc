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

#ifndef VMEMSTATE_H
#define VMEMSTATE_H

#include "Snippets.h"
#include "nanassert.h"
#include "pool.h"
#include "LVIDTable.h"

class VMemState {
public:
  typedef unsigned long WordMask;
  
private:
protected:
  // 32 bits means 4 * 32 (128 bytes is the max cache line)

  WordMask xrdmask; // eXpossed ReaDs mask
  WordMask wrmask;  // WRite mask

  bool leastSpecLine;
  bool mostSpecLine; // If there is not successor that needs to be restarted for
		     // that write

  static ulong  lineMask; // Cache Line Mask (unique for the whole system)

#ifdef DEBUG
  uint32_t msgSerialNumber;
#endif

  virtual void promote();

  void initialize();

  typedef ushort WordType;
  void writeWord(PAddr paddr) {
    // NOTE: a writeWord (protecting write) only can set if the store
    // is for the whole word. if the store is for a byte, it CAN NOT
    // set the wrmask. Otherwise, a restart may be hidden. Then a
    // dirty bit would be required because the line would be modified,
    // and the wrmask could not be set.
    wrmask |= calcWordMask(paddr);
  }

  void readWord(PAddr paddr) {
    // read is expossed, iff no previous protecting write was performed
    
    xrdmask |= ((~wrmask) & calcWordMask(paddr));
  }

public:

#ifdef TS_USE_SPECBITS
  bool isMostSpecLine() const {  return mostSpecLine;  }
  void setMostSpecLine() { 
    mostSpecLine = true; 
  }
  bool isLeastSpecLine() const { return leastSpecLine; }
  void setLeastSpecLine() { 
    leastSpecLine = true;
  }
#else
  bool isMostSpecLine() const {  return false;  }
  void setMostSpecLine() { 
  }
  bool isLeastSpecLine() const { return false; }
  void setLeastSpecLine() { 
  }
#endif

  void clearState();
  void copyStateFrom(const VMemState *st);
  void combineStateFrom(const VMemState *st);

  static WordMask calcWordMask(PAddr paddr) { return 1<<((paddr>>2) & lineMask); }

  bool hasProtectingWrite(WordMask mask) const { return wrmask  & mask;  }
  bool hasExposedRead(WordMask mask)    const { return xrdmask & mask;  }

  bool hasState() const { 
    return xrdmask || wrmask; 
  }

  void clearMasks() {
    xrdmask = 0;
    wrmask  = 0;
  }

  void forwardStateTo(VMemState *state);

  bool isDirty() const { return wrmask != 0; }

#ifdef DEBUG
  void setMsgSerialNumber(uint32_t sn) {
    msgSerialNumber = sn;
  }

  uint32_t getMsgSerialNumber() const {
    return msgSerialNumber;
  }
#else
  void setMsgSerialNumber(uint32_t sn) {  }
#endif

};

#endif // VMEMSTATE_H
