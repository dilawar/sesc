/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Radu Teodorescu
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

#ifndef MEMBUFFER_H
#define MEMBUFFER_H

#include <map>
#include <set>

#include "estl.h"
#include "nanassert.h"
#include "pool.h"

#include "HVersion.h"
#include "ThreadContext.h"

class TaskContext;
class MemBufferEntry;
class MemBuffer;

struct MemBufferEntryLessThan {
  bool operator()(const MemBufferEntry *v1, const MemBufferEntry *v2) const;
};

typedef uint8_t BitMaskType;
enum {
  logChunkSize=3,              // == log2(sizeof(long long))
  chunkSize=(1<<logChunkSize), // == sizeof(long long) == sizeof(BitMaskType)*8
  chunkAddrMask=(chunkSize-1),
  chunkDataMask=0xFF
};

typedef std::set<MemBufferEntry *, MemBufferEntryLessThan> MemOpsType;
typedef HASH_MAP<uint32_t, MemOpsType::iterator> MapMemOpsType;

class MemBufferEntry {
private:
  static const BitMaskType accessBitMask[16][chunkSize];

  static pool<MemBufferEntry, true> mePool;
  friend class pool<MemBufferEntry, true>;
protected:

  unsigned long long data; // First variable of the class to be 8 bytes align
  RAddr realAddr;          // For stores only

  BitMaskType        xrdmask; // Exposed read mask (ONLY exposed reads)
  BitMaskType        wrmask;

  // TaskControl is responsible for the allocation/deallocation of HVersion
  const HVersion *ver; // Version that the rd/wr belongs

  uint32_t  cIndex; // chunkIndex (addr>>logChunkSize)

  uint32_t  instAddr; // the ST/LD instruction

  void initializeData(const MemBufferEntry *srcEntry) {
    data = srcEntry->data;
  }


public:
  static MemBufferEntry *create(const HVersion *v, uint32_t ci, RAddr raddr, uint32_t iaddr=0);
  void initializeData(MemOpsType::iterator mit);

  static bool chunkCopy(RAddr dstAddr, RAddr srcAddr, BitMaskType rbitmask);

  MemBufferEntry();

  RAddr getRealAddr() const { return realAddr; }

  static BitMaskType calcAccessMask(short opflags, uint32_t cOffset) {
    I(cOffset < chunkSize);
    BitMaskType bm = accessBitMask[opflags & (E_RIGHT|E_LEFT|E_SIZE)][cOffset];
    // bm can be zero because an illegal access can have invalid mask
    return bm;
  }

  void updateXRDMask(BitMaskType accMask) {
    xrdmask = xrdmask | calcMissingWRMask(accMask);
  }
  
  BitMaskType calcXRDMask(BitMaskType accMask) const {
    return accMask & xrdmask;
  }

  BitMaskType calcMissingWRMask(BitMaskType accMask) const {
    // remove protected writes
    return accMask & (~wrmask);
  }

  void addWRMask(BitMaskType accMask) {
    wrmask = wrmask | accMask;
  }

  static uint32_t calcChunkIndex(RAddr addr) {
	 I(sizeof(RAddr) == sizeof(uint));
	 return static_cast<uint>(addr) >> logChunkSize;
  }

  static uint32_t calcChunkOffset(RAddr addr) {
	 return static_cast<uint>(addr) & chunkAddrMask;
  }

  static uint32_t calcAlignChunk(RAddr addr) {
	 return static_cast<uint>(addr) & (~0UL ^chunkAddrMask);
  }

  RAddr getAddr(RAddr addr)       const { return (RAddr)(&data)+calcChunkOffset(addr);  }
  RAddr getAddr()                 const { return (RAddr)(&data);  }
  uint32_t getChunkIndex()           const { return cIndex;          }
  uint32_t getInstAddr()             const { return instAddr;        }
  const HVersion *getVersionRef() const { return ver;             }

  bool getDataIfNeeded(const MemBufferEntry *stEntry, BitMaskType cpmask);
  bool getDataFromEntry(BitMaskType rbitmask, const MemBufferEntry *stEntry, RAddr addr);
  BitMaskType getDataFromMemory(BitMaskType rbitmask, RAddr srcAddr);

  void mergeDestroy();
  void justDestroy();

  void dump(const char *str) const;

};

// to support partial ordering, we need different comparison domains,
// this is done by having one memOps set per domain
class MemBufferDomain {
 public:
  MemOpsType memOps;
};

class MemBuffer {
private:
  static pool<MemBuffer, true> mPool;
  friend class pool<MemBuffer, true>;

  //static MemOpsType memOps; // all the read/writes performed
  MemBufferDomain *mbd;

  MapMemOpsType mapMemOps;

  const HVersion *memVer; // version



protected:
public:

  static MemBufferDomain* createMemBufferDomain();

  const HVersion *getVersionRef() const { return memVer; }

  // Create a new clean MemBuffer using newMemVer as the current version
  static MemBuffer *create(const HVersion *ver);

  void mergeOps();
  void mergeDestroy();
  void justDestroy();

  // Prepare to read from this version. Returns the address to read from.
  RAddr read(uint32_t iaddr, short iFlags, RAddr addr);

  void silentReadChunk(MemOpsType::iterator it, MemBufferEntry *e, RAddr addr);

  const HVersion *postWrite(const unsigned long long *data, uint32_t iaddr,
			    short iFlags, RAddr addr);


};

#endif // MEMBUFFER_H
