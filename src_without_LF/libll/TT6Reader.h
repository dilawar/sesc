/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by Paul Sack
                  Luis Ceze
		  Pablo Montesinos Ortego

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

/* This reads TT6 traces, like the ones produced by Apple amber */

#ifndef TT6READER_H
#define TT6READER_H

#include "ThreadContext.h"
#include "minidecoder.h"
#include "TraceEntry.h"
#include "TraceReader.h"

class TT6Reader : public TraceReader {
 private:
  FILE* trace;
  VAddr PC;
  uint32_t inst;
  VAddr address;
  int32_t   count;
  
  bool tracEof;

  void readPC();
  void readInst();
  void readAddress();
  void readCount();

  VAddr getCurrentPC()    const { return PC; }
  int32_t   getCurrentInst()  const { return inst; }
  int32_t   getCurrentCount() const { return count;}
  VAddr getCurrentDataAddress() const { return address; }
  
  bool  isBranch()  { return tt6_isFlowAltering(inst>>26, (inst>>1) & 0x3FF);};
  
  bool  isMemory() { return (tt6_isMemory(inst>>26, (inst>>1) & 0x3FF) 
			     || tt6_isMemoryExtended(inst>>26, (inst>>1) & 0x3FF));}

  bool  isMemoryExtended() {return tt6_isMemoryExtended(inst>>26, (inst>>1) & 0x3FF);}
  
  void advancePC();

 public:
  TT6Reader();

  void openTrace(const char* basename);
  void closeTrace();

  void fillTraceEntry(TraceEntry *te, int32_t id);
};

#endif
