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

#ifndef SCTABLE_H
#define SCTABLE_H

#include "Snippets.h"
#include "nanassert.h"

class SCTable {
private:
  const ulong  sizeMask;
  const uchar  Saturate;
  const uchar  MaxValue;
  
  uchar *table;
 protected:
 public:
  SCTable(int32_t id, const char *str, size_t size, uchar bits=2);
  ~SCTable(void);
  void clear(ulong cid); // Bias to not-taken
  void reset(ulong cid, bool taken);
  bool predict(ulong cid, bool taken); // predict and update
  void update(ulong cid, bool taken);

  bool predict(ulong cid)  const;
  bool isLowest(ulong cid) const;
  bool isHighest(ulong cid) const;
};

#endif
