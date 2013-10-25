/*
* ========== Copyright Header Begin ==========================================
* 
* OpenSPARC T1 Processor File: ValueCache.h
* Copyright (c) 2006 Sun Microsystems, Inc.  All Rights Reserved.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
* 
* The above named program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License version 2 as published by the Free Software Foundation.
* 
* The above named program is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
* 
* You should have received a copy of the GNU General Public
* License along with this work; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
* 
* ========== Copyright Header End ============================================
*/

#ifndef INCLUDED_VALUECACHE_H
#define INCLUDED_VALUECACHE_H
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

struct VCEntry {
  VCEntry() 
    : valid(false)
    , counter( 0 )
  {}

  void invalidate() { valid = false; }
  void incCounter() { ++counter; }
  bool hit( uint64_t v ) { ++counter; return valid && value == v; }
  void insert( uint64_t d ) { value = d; valid = true; counter = 1; }

  uint64_t value;
  bool valid; 
  uint32_t counter;
};

class InvalidIndexException {
 public:
  InvalidIndexException( int32_t i ) : idx( i )
  {}

  int32_t idx;
};

class ValueCache {
 public:
  typedef uint16_t IdxT;
  typedef int16_t DiffT;
  enum { VALUE_NOT_FOUND = -1 };

  ValueCache( unsigned size = 65535 ) 
    :vcacheSize(size)
    ,evictions(0)  {
    cache = new VCEntry[vcacheSize];
  }

  ~ValueCache() {
    delete [] cache;
  }

  void reset() {
    for( IdxT i = 0; i < vcacheSize; ++i ){
      cache[i].invalidate();
    }
  }

  bool hit( uint64_t d, IdxT &idx ) {
    idx = hash( d );
    
    return cache[idx].hit( d );
  }

  // compatability w/ existing code
  int32_t hit( uint64_t d ) {
    IdxT i;

    if( hit( d, i ) ){
      return i;
    } else {
      return -1;
    }
  }

  IdxT insert( uint64_t d ) {
    IdxT idx = hash( d );

    if( cache[idx].valid ){
      ++evictions;
    }

    cache[idx].insert( d );

    return idx;
  }


  IdxT hash( uint64_t d ) {
    //return (d ^ (d & 0x0FUL)) % vcacheSize;
    return (IdxT)(d % vcacheSize);
  }

  uint64_t operator[](IdxT idx) {
    assert( idx < vcacheSize );
            
    assert( !cache[idx].valid );
    return cache[idx].value;
  }

  int32_t conflicts() const { return evictions; }

 private:
  unsigned vcacheSize;
  VCEntry *cache;
  int32_t evictions;
};

#endif /* INCLUDED_VALUECACHE_H */

