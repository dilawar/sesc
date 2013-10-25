/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau

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

#ifndef LDSTBUFFER_H
#define LDSTBUFFER_H

#include "estl.h"
#include "nanassert.h"

#include "Snippets.h"
#include "DInst.h"
#include "callback.h"
#include "ThreadContext.h"

/** Load Store Buffer class
 *
 * This class enforces dependences between load and store, by tracking the store
 * addresses.
 *
 * Acquire, Release, and MemFence dependences are also enforced in this class.
 */

class LDSTBuffer {
private:
  
  typedef HASH_MAP<VAddr,DInst *,VAddr_hash> EntryType;
  typedef HASH_MAP<int32_t, DInst*>   FenceEntryType;
  static EntryType stores;
  static FenceEntryType fences;

  // pendingBarrier can be an Acquire or a MemFence, NOT a Release. Releases are
  // like a store.
  static DInst *pendingBarrier;

  static VAddr calcWord(const DInst *dinst) {
    // Just remove the two lower bits
    return (dinst->getVaddr()) >> 2;
  }
public:

  /** Get an entry from the LDSTQueue
   *
   * Store, Acquire, MemFence would occupy an entry in this
   * structure.
   */

  /* getEntry called at rename time for FetchOp, memFence, Acquire, and
   * Release 
   */
  static void getFenceEntry(DInst *dinst);
  static void fenceLocallyPerformed(DInst *dinst);

  /** Store gets an entry in the LDSTBuffer
   *
   * The request is in the same order than the instructions are fetched.
   *
   */
  static void getStoreEntry(DInst *dinst);

  /** Loads check for an entry in the LDSTBuffer
   *
   * The request is in the same order than the instructions are feched.
   *
   */
  static void getLoadEntry(DInst *dinst);

  static void storeLocallyPerformed(DInst *dinst);

  static void dump(const char *str);
};

#endif // LDSTBUFFER_H
