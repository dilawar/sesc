/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Luis Ceze
		  Milos Prvulovic

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

#include "SCTable.h"

SCTable::SCTable(int32_t id, const char *str, size_t size, uchar bits)
  : sizeMask(size - 1)
  ,Saturate(bits > 1 ? (1<<(bits-1)) : 1)
  ,MaxValue((1<<bits)-1) 
{
  if((size & (size - 1)) != 0) {
    MSG("SCTable (%s) size [%d] a power of two", str, (int)size);
    return;
  }
  if( bits > 7 || bits < 1 ) {
    MSG("SCTable (%s) bits [%d] should be between 1 and 7", str, bits);
    return;
  }

  table = new uchar[size];
  I(table);

  uchar flipflop = 1;

  for(size_t cnt = 0; cnt < size; cnt++) {
    table[cnt] = flipflop;
    flipflop = Saturate - flipflop;
  }
}

SCTable::~SCTable(void){
  delete [] table;
}

void SCTable::reset(ulong cid, bool taken)
{
  table[cid & sizeMask] = taken ? Saturate : Saturate-1;
}

void SCTable::clear(ulong cid)
{
  // Bias to not-taken
  table[cid & sizeMask] = 0;
}

bool SCTable::predict(ulong cid, bool taken)
{
  uchar *entry = &table[cid & sizeMask];

  bool ptaken=((*entry) >= Saturate);

  if(taken) {
    if(*entry < MaxValue)
      *entry = (*entry) + 1;
  } else {
    if(*entry > 0)
      *entry = (*entry) - 1;
  }

  return ptaken;
}

void SCTable::update(ulong cid, bool taken)
{
  uchar *entry = &table[cid & sizeMask];

  if(taken) {
    if(*entry < MaxValue)
      *entry = (*entry) + 1;
  } else {
    if(*entry > 0)
      *entry = (*entry) - 1;
  }
}

bool SCTable::predict(ulong cid) const
{
  uchar *entry = &table[cid & sizeMask];

  return (*entry >= Saturate);
}

bool SCTable::isLowest(ulong cid) const 
{
  uchar *entry = &table[cid & sizeMask];

  return (*entry == 0);
}

bool SCTable::isHighest(ulong cid) const
{
  uchar *entry = &table[cid & sizeMask];

  return (*entry == MaxValue);
}
