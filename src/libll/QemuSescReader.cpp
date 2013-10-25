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

#include <sys/types.h>
#include <dirent.h>

#include "nanassert.h"
#include "QemuSescReader.h"
#include "QemuSescTrace.h"

QemuSescReader::QemuSescReader() {
  trace = 0;
  PC = 0;

  tracEof = true;
  qst = (QemuSescTrace*)malloc(sizeof(QemuSescTrace));  
}

void QemuSescReader::openTrace(const char *filename) {

  trace = fopen(filename, "rb");

  if (trace == NULL){
    MSG("QemuSescReader::Can't open the trace file [%s].", filename); 
    exit(-1);
  }

  tracEof = false;

  readInst();   //for first instruction

  if(tracEof) {
    MSG("QemuSescReader::Bad trace file."); 
    exit(-1);
  }
}

void QemuSescReader::closeTrace() {
  fclose(trace);
}

void QemuSescReader::advancePC() {

  if (tracEof) {
    PC =  0xffffffff;
    return;
  }

  readInst();
}

void QemuSescReader::readInst() { 

  if (fread(qst, sizeof(QemuSescTrace), 1, trace) !=1){
    MSG("Size of each structure is %lu", sizeof(qst));
    if (feof(trace))
      tracEof = true;
    else {
      MSG("QemuSescReader: Error while reading TraceFile");
      exit(1);
    }
  } 

  PC = qst->pc;

  MSG("PC 0x%x r%d <- r%d %u r%d",PC, qst->dest, qst->src1, qst->opc, qst->src2);

  if (feof(trace)) {
    tracEof = true; 
    closeTrace();
  }
      
}

void QemuSescReader::fillTraceEntry(TraceEntry *te, int32_t id) {
  I(id == 0); // multi-threaded TT6 not supported yet;

  if(getCurrentPC() == 0xffffffff) {
    te->eot = true;
  }
  
  te->rawInst = 0;
  te->iAddr     = getCurrentPC();  

  te->nextIAddr = qst->npc;
  
  advancePC();   
}
