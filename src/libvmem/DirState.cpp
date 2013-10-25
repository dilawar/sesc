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

#include "DirState.h"

uint32_t DirState::linesPerEntry = 0;

DirState::DirState()
{
  tag = 0;
  invalid = true;
}

DirState::~DirState()
{
  // Nothing as usual
}

void DirState::initialize(CacheType *c) 
{
  // size (in bits) of unit of information is # of processors (== # of L1
  // caches) + 1 (victim cache), rounded to the next power of 2
  uint32_t unitSize = SescConf->getRecordSize("", "cpucore") + 1;
  unitSize = roundUpPower2(unitSize); 

  cache = c;

  if (unitSize < 8) {
    if (unitSize == 2)
      linesPerEntry = (cache->getLineSize()) << 2;
    else if (unitSize == 4)
      linesPerEntry = (cache->getLineSize()) << 1;
    else I(0); // no other number is legal (either not power of 2 
               // or not greater than 1 - at least 1 L1 and 1 VC)
  } else {
    linesPerEntry = cache->getLineSize()*8 / unitSize;
  } 
  
  I(linesPerEntry != 0);

  // initialize directory entries
  for(uint32_t i = 0; i < linesPerEntry; i++)
    presenceBits.push_back((uint32_t) -1);
}

bool DirState::isLocked() {
  return false;
}

PAddr DirState::getTag() const 
{ 
  return tag; 
}

void DirState::setTag(PAddr a) {
  tag = a; 
  invalid = false;
}

void DirState::invalidate() 
{ 
  invalid = true; 
  for(uint32_t i = 0; i < linesPerEntry; i++)
    presenceBits[i] = (uint32_t) -1;
}

