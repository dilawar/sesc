/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2005 University California, Santa Cruz.

   Contributed by Saangetha
                  Keertika
		  Jose Renau

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


#ifndef QEMU_SESC_READER_H
#define QEMU_SESC_READER_H

#include "ThreadContext.h"
#include "minidecoder.h"
#include "TraceEntry.h"
#include "TraceReader.h"
#include "QemuSescTrace.h"

class QemuSescReader : public TraceReader {
 private:
  FILE* trace;
  uint32_t  PC;
  
  bool tracEof;
  void readPC();
  void readInst();

  uint32_t  getCurrentPC() { return PC; }
  void advancePC();

  QemuSescTrace *qst;
 public:
  QemuSescTrace *currentInst() { return qst; }
  QemuSescReader();

  void openTrace(const char* basename);
  void closeTrace();

  void fillTraceEntry(TraceEntry *te, int32_t id);

};

#endif
