/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by  Liu Wei
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

#include "OSSim.h"
#include "ParProf.h"

enum EntryType {
  Load = 0,
  Store,
  FCall,
  Ret,
  Other
};

ParProf::ParProf() 
{
  char fname[1024];
  
  sprintf(fname, "%s.pptrace", osSim->getBenchName());
  fout = fopen(fname, "w");
  I(fout);

  totInsts = 0;
}

void ParProf::processInst(const Instruction *inst, unsigned vAddr)
{
  EntryType e = Other;
  bool writeAddr = false;

  if(!osSim->enoughMarks1()) 
    return;

  totInsts++;

  if(totInsts > 100000000)
    return;

  if(inst->isLoad()) {
    e = Load;
    writeAddr = true;
  }

  if(inst->isStore()) {
    e = Store;
    writeAddr = true;
  }

  if(inst->isFuncCall()) {
    e = FCall;
    vAddr = inst->getAddr();
    writeAddr = true;
  }

  if(inst->isFuncRet()) {
    e = Ret;
  }
  
  uint8_t tag = e;
  int32_t wc = fwrite(&tag, sizeof(tag), 1, fout);
  I(wc);

  if(writeAddr) {
    wc = fwrite(&vAddr, sizeof(vAddr), 1, fout);
    I(wc);
  }
  
}

void ParProf::terminate()
{
  Report::field("ParProf:instCount=%d", totInsts);

  fclose(fout);
}

