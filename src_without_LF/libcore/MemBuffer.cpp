/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Radu Teodorescu

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

#include "ThreadContext.h"
#include "MemBuffer.h"

#include "TaskContext.h"

// MemBufferEntry (Read & Write)

//MemOpsType MemBuffer::memOps;

pool<MemBuffer,true>      MemBuffer::mPool(32768, "MemBuffer");
pool<MemBufferEntry,true> MemBufferEntry::mePool(32768, "MemBufferEntry");

#if ((E_RIGHT != 0x8) || (E_LEFT !=0x4) || (E_SIZE != 0x3))
#error "OpFlags does not have the proper structure!"
#endif

// accessBitMask[OpFlags][Offset]
//   Where OpFlags is 0..16 and Offset is 0..7
//   The OpFlags represents for bits, from MSB (bit 3) to LSB (bit 0):
//     E_RIGHT, E_LEFT, and 2 bits for E_SIZE
//   The bits of BitMask represent which bytes the memory operation accesses
//     (MSB is lowest byte, LSB is highest byte)

const BitMaskType MemBufferEntry::accessBitMask[16][chunkSize]={
  // E_RIGHT is 0, E_LEFT is 0, E_SIZE is 0..3
  {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01},
  {0xC0,0x00,0x30,0x00,0x0C,0x00,0x03,0x00},
  {0xF0,0x00,0x00,0x00,0x0F,0x00,0x00,0x00},
  {0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  // E_RIGHT is 0, E_LEFT is 1, E_SIZE should be 0
  {0xF0,0x70,0x30,0x10,0x0F,0x07,0x03,0x01},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  // E_RIGHT is 1, E_LEFT is 0, E_SIZE should be 0
  {0x80,0xC0,0xE0,0xF0,0x08,0x0C,0x0E,0x0F},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  // E_RIGHT is 1, E_LEFT is 1, this should not happen
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

MemBufferEntry::MemBufferEntry()
  : ver(0)
  ,cIndex(0)
{
  data = 0;
}

MemBufferEntry *MemBufferEntry::create(const HVersion *v, uint32_t ci, RAddr addr, uint32_t iaddr)
{
  MemBufferEntry *e = mePool.out();

//  e->data    = 0;
  e->xrdmask = 0; // No valid data
  e->wrmask  = 0; // No valid data
  e->ver     = v;
  e->cIndex  = ci;

  e->realAddr = calcAlignChunk(addr);
  e->instAddr = iaddr;


  return e;
}

void MemBufferEntry::initializeData(MemOpsType::iterator mit)
{
  mit--; // skip itself

  MemBufferEntry *prevEntry = *mit;
  if (prevEntry->getChunkIndex() == getChunkIndex()) {
    // Copy from previous version
    I(prevEntry!=this);
    I(*(prevEntry->getVersionRef()) < *ver);
    initializeData(prevEntry);
  }else{
    // Copy from memory
    chunkCopy(getAddr(), getRealAddr(), chunkDataMask);
  }
}

bool MemBufferEntry::chunkCopy(RAddr dstAddr, RAddr srcAddr, BitMaskType rbitmask)
{
  bool copied = false;
  
  I(rbitmask); // works with zero, but it is a waste
  
  I(calcChunkOffset(srcAddr) == 0); // addr Chunk aligned
  I(calcChunkOffset(dstAddr) == 0); // addr Chunk aligned

  I(sizeof(uint32_t) > sizeof(BitMaskType));

  const char *src = (const char *)srcAddr;
  char *dst = (char *)dstAddr;
    
  while(rbitmask) {
    if ((rbitmask & 0xF0) == 0xF0) {
		const uint32_t *wsrc = (const uint32_t *)src;
		uint32_t *wdst = (uint32_t *)dst;
      
      copied = copied || (*wdst != *wsrc);
      *wdst = *wsrc;

      src+=4;
      dst+=4;
      rbitmask = rbitmask<<4;
    }else if ((rbitmask & 0xF0) == 0x00) {
      src+=4;
      dst+=4;
      rbitmask = rbitmask<<4;
    }else if (rbitmask & 0x80) {
      copied = copied || (*dst != *src);
      *dst = *src;

      src++;
      dst++;
      rbitmask = rbitmask<<1;
    }else{
      src++;
      dst++;
      rbitmask = rbitmask<<1;
    }
  }

  return copied;
}

void MemBufferEntry::justDestroy()
{
  mePool.in(this);
}

void MemBufferEntry::mergeDestroy()
{
  if (wrmask)
    chunkCopy(realAddr, getAddr(), wrmask);

  mePool.in(this);
}

bool MemBufferEntry::getDataIfNeeded(const MemBufferEntry *stEntry, BitMaskType cpmask)
{
  if (calcXRDMask(cpmask))
    return true;


  return false;
}

void MemBufferEntry::dump(const char *str) const
{
  fprintf(stderr,"%s:cIndex=0x%lx:xrdmask=0x%2x:wrmask=0x%2x", str, cIndex, xrdmask, wrmask);
  I(ver);
  ver->dump(":ver",true);
  fprintf(stderr," value=0x%llx (addr=0x%x)\n",data, (int)&data);
}

// Less Than comparators

// Order:
// By address
// Same address ordered by version

bool MemBufferEntryLessThan::operator()(const MemBufferEntry *v1, const MemBufferEntry *v2) const 
{
  return ((v1->getChunkIndex() < v2->getChunkIndex())
	  ||
	  ((v1->getChunkIndex() == v2->getChunkIndex())
	   &&
	   (*(v1->getVersionRef()) < *(v2->getVersionRef()))));
}

MemBufferDomain* MemBuffer::createMemBufferDomain()
{
  MemBufferDomain *mbd = new MemBufferDomain();

  // JUNK entry at the beginning (address 0 so that it is the first)
  mbd->memOps.insert(MemBufferEntry::create(0,0,0));

  // JUNK entry at the end (address 0xFFFFFFFF so that it is the first)
  mbd->memOps.insert(MemBufferEntry::create(0,0xFFFFFFFF,0));

  return mbd;
}


MemBuffer *MemBuffer::create(const HVersion *ver)
{
  MemBuffer *mb = mPool.out();
  
  mb->memVer = ver;
  mb->mbd = ver->getMemBufferDomain();
  I(mb->mapMemOps.empty());


  return mb;
}

void MemBuffer::justDestroy()
{
  MapMemOpsType::iterator it = mapMemOps.begin();
  
  while(it != mapMemOps.end()) {
    MemOpsType::iterator mit = it->second;
    mbd->memOps.erase(mit);
    (*mit)->justDestroy();
    
    MapMemOpsType::iterator dit = it;
    it++;
    mapMemOps.erase(dit);
  }
  // mapMemOps.clear();
  
  mPool.in(this);
}

void MemBuffer::mergeDestroy()
{
  MapMemOpsType::iterator it = mapMemOps.begin();
  
  while(it != mapMemOps.end()) {
    MemOpsType::iterator mit = it->second;
    mbd->memOps.erase(mit);
    (*mit)->mergeDestroy();

    MapMemOpsType::iterator dit = it;
    it++;
    mapMemOps.erase(dit);
  }
  // mapMemOps.clear();
  
  mPool.in(this);
}

void MemBuffer::mergeOps()
{
  I(memVer->isSafe());
  MapMemOpsType::iterator it = mapMemOps.begin();
  
  while(it != mapMemOps.end()) {
    MemOpsType::iterator mit = it->second;
    mbd->memOps.erase(mit);
    (*mit)->mergeDestroy();
    
    MapMemOpsType::iterator dit = it;
    it++;
    mapMemOps.erase(dit);
  }
}


RAddr MemBuffer::read(uint32_t iaddr, short opflags, RAddr addr)
{
  I(addr); // load to address 0 not allowed
  I(addr!=0xFFFFFFFF); // load to address 0xFFFFFFFF not allowed

  MemBufferEntry *e;
  MemOpsType::iterator mit;
  uint32_t cIndex  = MemBufferEntry::calcChunkIndex(addr);
  MapMemOpsType::iterator hit = mapMemOps.find(cIndex);
  if (hit == mapMemOps.end()) {
    e = MemBufferEntry::create(memVer, cIndex, addr, iaddr);

    mit = mbd->memOps.insert(e).first;
    mapMemOps[cIndex] = mit;

    e->initializeData(mit);
  }else{
    mit = hit->second;
    e = *mit;
    I(e->getRealAddr() == MemBufferEntry::calcAlignChunk(addr));
  }

  uint32_t cOffset = MemBufferEntry::calcChunkOffset(addr);
  BitMaskType accMask=MemBufferEntry::calcAccessMask(opflags, cOffset);


  e->updateXRDMask(accMask);
  return e->getAddr()+cOffset;
}

const HVersion *MemBuffer::postWrite(const unsigned long long *writeData, 
					  uint32_t iaddr, short opflags, RAddr addr)
{
  I(addr); // load to address 0 not allowed

  MemBufferEntry *e;
  MemOpsType::iterator mit;
  uint32_t cIndex  = MemBufferEntry::calcChunkIndex(addr);
  MapMemOpsType::iterator hit = mapMemOps.find(cIndex);
  if (hit == mapMemOps.end()) {
    e = MemBufferEntry::create(memVer, cIndex, addr);
      
    mit = mbd->memOps.insert(e).first;
    mapMemOps[cIndex] = mit;
    
    e->initializeData(mit);
  }else{
    mit = hit->second;
    e = *mit;
    I(e->getRealAddr() == MemBufferEntry::calcAlignChunk(addr));
  }

  uint32_t cOffset = MemBufferEntry::calcChunkOffset(addr);
  BitMaskType accMask=MemBufferEntry::calcAccessMask(opflags, cOffset);

  e->addWRMask(accMask);

  bool copied = MemBufferEntry::chunkCopy(e->getAddr(), (RAddr)writeData, accMask);


#ifdef SILENT_STORE
  if (!copied)
    return 0;
#endif

  mit++; // skip itself

  I(cIndex!=0xFFFFFFFF); // store to address 0xFFFFFFFF not allowed (JUNK entry)

  while((*mit)->getChunkIndex() == cIndex) {
    MemBufferEntry *succEntry = *mit;
    mit++;
    
    I(*(succEntry->getVersionRef()) > *memVer);
    I(succEntry->getVersionRef() != memVer);
    
    if (succEntry->calcXRDMask(accMask)) {
      // Since all the successors would be killed or restarted. It is
      // not necessary to keep traversing the list. Just return the
      // first task that would get a restart
      LOG("********dataDep violation succ=%d", 
	  succEntry->getVersionRef()->getTaskContext()->getPid());
#ifdef TS_RISKLOADPROF
      LOG("*************** write @ 0x%08lx ", iaddr);
      LOG("*************** read  @ 0x%08lx ", succEntry->getInstAddr());
      osSim->getRiskLoadProf()->insert(succEntry->getInstAddr());
#endif
      return succEntry->getVersionRef();
    }
    
    // Remove protected store bytes (not before check because there can be
    // expossed reads)
    accMask = succEntry->calcMissingWRMask(accMask);
    if (accMask == 0)
      return 0;
    MemBufferEntry::chunkCopy(succEntry->getAddr(), (RAddr)writeData, accMask);
  }
  return 0;
}


