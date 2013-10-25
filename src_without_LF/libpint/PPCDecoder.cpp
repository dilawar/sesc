/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze
                  Karin Strauss

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


#include <strings.h>
#include <stdlib.h>

#include "nanassert.h"
#include "PPCDecoder.h"

PPCInstDef PPCDecoder::ppcInstTable[] = {
#undef PPCINST
#define PPCINST(i, mop, eop, f, t, st) { #i, i ## _inst, mop, eop, f ## _form, t, st}
#include "PPCInsts.def"
};

PPCInstDef** PPCDecoder::decodeTable = 0;

// Decode table:
// the number of entries in the decode table is (# major ops)*(size of XO)
// then we can do fast table-driver decoding
void PPCDecoder::Initialize() 
{
  I(decodeTable == 0);

  int32_t nInsts = sizeof(ppcInstTable) / sizeof(PPCInstDef);
  // 32 major opcodes
  // 1024 max number of extended opcodes
  int32_t nEntries = PPC_OPS * PPC_EXTOPS * sizeof(PPCInstDef *);
  decodeTable = (PPCInstDef **) malloc(nEntries);
  bzero(decodeTable, nEntries);

  // first entry in table is dummy
  for(int32_t i = 1; i < nInsts; i++) {
    PPCInstDef *instDef = &ppcInstTable[i];
    switch(instDef->form) {
    case I_form:
    case B_form: 
    case SC_form: 
    case D_form: 
    case M_form:      
      expandDecodeEntry(instDef, 0);
      break;
    case DS_form:
      expandDecodeEntry(instDef, 2);
      break;
    case X_form:
    case XL_form:
    case XFX_form:
    case XFL_form:
      expandDecodeEntry(instDef, 10);
      break;
    case XS_form:
    case XO_form:
      expandDecodeEntry(instDef, 9);
      break;
    case A_form:
      expandDecodeEntry(instDef, 5);
      break;
    default:
      MSG("fatal:unknown instruction form");
      exit(-1);
    }
  }

  fillDummyEntries();
}

void PPCDecoder::expandDecodeEntry(PPCInstDef *instDef, int32_t extOpSize) 
{
  I(extOpSize <= PPC_EXTOP_BITS);
  
  int32_t nOffset = 1 << extOpSize;

  int32_t constIndex = (instDef->majorOpcode << PPC_EXTOP_BITS) 
                            + instDef->extOpcode;

  for(int32_t i = 0; i < PPC_EXTOPS; i += nOffset) {
    I(decodeTable[(constIndex + i)] == 0);
    decodeTable[(constIndex + i)] = instDef;
  }
}

void PPCDecoder::fillDummyEntries() 
{
  PPCInstDef *dummyDef = &ppcInstTable[0];

  for(int32_t i = 0; i < PPC_OPS * PPC_EXTOPS; i++) {
    if(decodeTable[i] == NULL)
      decodeTable[i] = dummyDef;
  }
}

