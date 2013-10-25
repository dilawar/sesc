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

uint32_t reads = 0;
uint32_t writes = 0;
uint32_t fills = 0;

#include "Directory.h"

Directory::Directory(const char *section, const char *name) 
{
  cache = CacheType::create(section, "", name);
  I(cache);

  // size (in bits) of unit of information is # of processors (== # of L1
  // caches) + 1 (victim cache), rounded to the next power of 2
  uint32_t unitSize = SescConf->getRecordSize("", "cpucore") + 1;
  unitSize = roundUpPower2(unitSize); 

  if (unitSize < 8) { // 8 = # of bits in a byte
    if (unitSize == 2)
      corrShift = 2;
    else if (unitSize == 4)
      corrShift = 1;
    else I(0); // no other number is legal (either not power of 2 
               // or not greater than 1 - at least 1 L1 and 1 VC) 
  }
}

Directory::~Directory() 
{
  // Do nothing
}

void Directory::setPresentIn(const uint32_t cacheId, PAddr addr)
{
  Line *l = cache->writeLine(addr >> corrShift);
  writes++;

  // line does not exist in the directory, create it
  if (!l) { 
    Line *ol = cache->fillLine(addr >> corrShift);
    fills++;
    
    // set state
    ol->setAllEntryBits();
    return;
  }

  if(!(writes & ((1 << 15) - 1)))
    LOG("directory r=%d, w=%d, f=%d", reads, writes, fills);

  // change line state
  l->setBit(cacheId, addr);

  I(l);
  I(l->isPresent(cacheId, addr));
}

void Directory::resetPresentIn(const uint32_t cacheId, PAddr addr)
{
  Line *l = cache->writeLine(addr >> corrShift);
  writes++;

  // line does not exist in the directory, create it
  if (!l) {
    Line *ol = cache->fillLine(addr >> corrShift);
    fills++;

    ol->setAllEntryBits();
    l = ol;
  }

  if(!(writes & ((1 << 15) - 1)))
    LOG("directory r=%d, w=%d, f=%d", reads, writes, fills);

  // change line state
  l->resetBit(cacheId, addr);

  I(l);
  I(!l->isPresent(cacheId, addr));
}

void Directory::setAllPresent(PAddr addr)
{  
  Line *l = cache->writeLine(addr >> corrShift);
  writes++;

  // line does not exist in the directory, create it
  if (!l) {
    Line *ol = cache->fillLine(addr >> corrShift);
    fills++;

    ol->setAllEntryBits();
    l = ol;
  }

  if(!(writes & ((1 << 15) - 1)))
    LOG("directory r=%d, w=%d, f=%d", reads, writes, fills);

  // change line state
  l->setAllLineBits(addr);
}

void Directory::resetAllPresent(PAddr addr)
{
  Line *l = cache->writeLine(addr >> corrShift);
  writes++;

  // line does not exist in the directory, create it
  if (!l) {
    Line *ol = cache->fillLine(addr >> corrShift);
    fills++;

    ol->setAllEntryBits();
    l = ol;
  }

  if(!(writes & ((1 << 15) - 1)))
    LOG("directory r=%d, w=%d, f=%d", reads, writes, fills);

  // change line state
  l->resetAllLineBits(addr);
}

bool Directory::getInfoForLine(PAddr addr, 
			       uint32_t *info) const
{
  Line *l = cache->readLine(addr >> corrShift);
  reads++;
 
  if (!l) {
    // if there is a miss, no info is returned
    return false;
  }

  if(!(reads & ((1 << 15) - 1)))
    LOG("directory r=%d, w=%d, f=%d", reads, writes, fills);  

  *info = l->whereIsPresent(addr);
  return true;
}

bool Directory::isPresent(const uint32_t cacheId, const uint32_t info)
{
  return ((1 << cacheId) & info);
}

uint32_t Directory::whereIsPresent(PAddr addr)
{
  Line *l = cache->readLine(addr >> corrShift);

  if (!l) {
    MSG("address 0x%08lx not present in directory", addr);
    return (uint32_t) -1;
  }

  return l->whereIsPresent(addr);
}
