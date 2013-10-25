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

#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>

#include "nanassert.h"
#include "TT6Reader.h"

TT6Reader::TT6Reader() {
  trace = 0;
  PC = 0;
  inst = 0;
  address = 0;
  count = 0;
  tracEof = true;
}

void TT6Reader::openTrace(const char* basename) {
  char filename[80];

  strcpy(filename, basename);
  strcat(filename, "/thread_001.tt6");
  trace = fopen(filename, "r");

  if (trace == NULL){
    MSG("TT6Reader::Can't open the trace file."); 
    exit(-1);
  }

  tracEof = false;

  readPC();
  if(tracEof) {
    MSG("TT6Reader::Bad trace file."); 
    exit(-1);
  }
  readInst();
}

void TT6Reader::closeTrace(){
  fclose(trace);
}

void TT6Reader::advancePC() {
  if (tracEof) {
    PC =  0xffffffff;
    return;
  }

  readInst();
  if (isBranch()) {
    readPC();
    I(!isMemory());
    IS(address = count = 0);
  }
  else {
    PC += 4;

    if (isMemory()) {
      readAddress();
      I(!isBranch());
      IS(count = 0);
    }
    else if (isMemoryExtended()) {
      readCount();
    }
    else { /* arithmetic/regular inst */
      IS(address = count = 0);
    }

  }
}

void TT6Reader::readPC() { 
  if (fread(&PC, sizeof(PC), 1, trace) != 1)
    if (feof(trace))
      tracEof=true;
    else{
      MSG("Error while reading the trace file.");
      exit(1);
    }
  //MSG("TT6Reader: readPC() currentPC = %08x", (uint32_t) PC);
}

void TT6Reader::readInst() { 
        
  if (fread(&inst, sizeof(inst), 1,trace)!=1)
    if (feof(trace))
      tracEof=true;             
    else{
      MSG("Error while reading the trace file.");
      exit(1);
    }
        
}

void TT6Reader::readAddress() { 
        
  if (fread(&address, sizeof(address), 1,trace)!=1)             
    if (feof(trace))
      tracEof=true;             
    else{
      MSG("Error while reading the trace file.");
      exit(1);
    }
}

void TT6Reader::readCount() { 
  if (fread(&count, sizeof(count), 1,trace)!=1) 
    if (feof(trace))
      tracEof=true;             
    else{
      MSG("Error while reading the trace file.");
      exit(1);
    }
}

void TT6Reader::fillTraceEntry(TraceEntry *te, int32_t id) {
  I(id == 0); // multi-threaded TT6 not supported yet;
  
  if(getCurrentPC() == 0xffffffff) { // end of trace
    te->eot = true;
    return;
  }
  
  te->rawInst = getCurrentInst();
  te->iAddr   = getCurrentPC();
  
  if(isMemory()) {
    te->dAddr = getCurrentDataAddress();
  }
  
  advancePC();

  te->nextIAddr = getCurrentPC();
}
