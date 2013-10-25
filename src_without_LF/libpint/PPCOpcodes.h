/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze
                  Brian Greskamp
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

#ifndef PPCOPCODES_H
#define PPCOPCODES_H

#include "InstType.h"

#define PPC_OP_BITS     6                        // # of bits for opcode
#define PPC_EXTOP_BITS  10                       // # of bits for ext. opcode
#define PPC_OPS         (1 << PPC_OP_BITS)       // # of possible opcodes
#define PPC_EXTOPS      (1 << PPC_EXTOP_BITS)    // # of possible ext. opcodes
#define PPC_OP_MASK     (PPC_OPS - 1)            // mask for an opcode
#define PPC_EXTOP_MASK  (PPC_EXTOPS - 1)         // mask for an ext. opcode
#define PPC_OP_SHIFT    26                       // # of bits to shift opcode
#define PPC_EXTOP_SHIFT 1                        // same for ext. opcode

#define PPC_OPC_MAIN(opc)      (((opc) >> PPC_OP_SHIFT) & PPC_OP_MASK)
#define PPC_OPC_EXT(opc)       (((opc) >> PPC_EXTOP_SHIFT) & PPC_EXTOP_MASK)
#define PPC_OPC_Rc             1
#define PPC_OPC_OE             (1<<10)
#define PPC_OPC_LK             1
#define PPC_OPC_AA             (1<<1)


#define PPC_DEC_I(rawinst, LI)			\
  {						\
    LI = (rawinst) & 0x3fffffc;			\
    if (LI & 0x02000000) LI |= 0xfc000000;	\
  }

#define PPC_DEC_B(rawinst, BO, BI, BD)				\
  {								\
    BO = ((rawinst) >> 21) & 0x1f;				\
    BI = ((rawinst) >> 16) & 0x1f; BD = (rawinst) & 0xfffc;	\
    if (BD & 0x8000) BD |= 0xffff0000;				\
  }

#define PPC_DEC_D_SImm(rawinst, rD, rA, imm)	\
  {						\
    rD = ((rawinst) >> 21) & 0x1f;		\
    rA = ((rawinst) >> 16) & 0x1f;		\
    imm = (rawinst) & 0xffff;			\
    if (imm & 0x8000) imm |= 0xffff0000;	\
  }

#define PPC_DEC_D_UImm(rawinst, rD, rA, imm)	\
  {						\
    rD = ((rawinst) >> 21) & 0x1f;		\
    rA = ((rawinst) >> 16) & 0x1f;		\
    imm = (rawinst) & 0xffff;			\
  }

#define PPC_DEC_D_Shift16(rawinst, rD, rA, imm)	  \
  {						  \
    rD = ((rawinst) >> 21) & 0x1f;		  \
    rA = ((rawinst) >> 16) & 0x1f;		  \
    imm = (rawinst) << 16;			  \
  }


#define PPC_DEC_M(rawinst, rS, rA, SH, MB, ME)	 \
  {						 \
    rS = ((rawinst) >> 21) & 0x1f;		 \
    rA = ((rawinst) >> 16) & 0x1f;		 \
    SH = ((rawinst) >> 11) & 0x1f;		 \
    MB = ((rawinst) >> 6)  & 0x1f;		 \
    ME = ((rawinst) >> 1)  & 0x1f;		 \
  }

#define PPC_DEC_X(rawinst, rS, rA, rB, XO)	\
  {						\
    rS = ((rawinst) >> 21) & 0x1f;		\
    rA = ((rawinst) >> 16) & 0x1f;		\
    rB = ((rawinst) >> 11) & 0x1f;		\
    XO = ((rawinst) >> 1)  & 0x3ff;		\
  }


#define PPC_DEC_XL(rawinst, BO, BI, BD, XO)	\
  {						\
    BO = ((rawinst) >> 21) & 0x1f;		\
    BI = ((rawinst) >> 16) & 0x1f;		\
    BD = ((rawinst) >> 11) & 0x1f;		\
    XO = ((rawinst) >> 1)  & 0x3ff;		\
  }

#define PPC_DEC_XFX(rawinst, rS, CRM, XO)	\
  {						\
    rS =  ((rawinst) >> 21) & 0x1f;		\
    CRM = ((rawinst) >> 12) & 0xff;		\
    XO = ((rawinst) >> 1)  & 0x3ff;		\
  }

#define PPC_DEC_XFL(rawinst, rB, FM, XO)	\
  {						\
    rB=((rawinst) >> 11) & 0x1f;		\
    FM=((rawinst) >> 17) & 0xff;		\
    XO = ((rawinst) >> 1)  & 0x3ff;		\
  }

#define PPC_DEC_XO(rawinst, rS, rA, rB, XO)   \
  {					      \
    rS = ((rawinst) >> 21) & 0x1f;	      \
    rA = ((rawinst) >> 16) & 0x1f;	      \
    rB = ((rawinst) >> 11) & 0x1f;	      \
    XO = ((rawinst) >> 1)  & 0x1ff;	      \
  }

#define PPC_DEC_A(rawinst, rD, rA, rB, rC, XO)	\
  {						\
    rD = ((rawinst) >> 21) & 0x1f;		\
    rA = ((rawinst) >> 16) & 0x1f;		\
    rB = ((rawinst) >> 11) & 0x1f;		\
    rC = ((rawinst) >>  6) & 0x1f;		\
    XO = ((rawinst) >>  1) & 0x1f;		\
  }

typedef enum PPCInstFormEnum {
  I_form = 0, 
  B_form, 
  SC_form, 
  D_form, 
  DS_form,
  X_form,
  XL_form,
  XFX_form,
  XFL_form,
  XS_form,
  XO_form,
  A_form,
  M_form
} PPCInstForm;

//enumeration with instruction names
typedef enum PPCInstEnum {
#undef PPCINST
#define PPCINST(i, mop, eop, f, t, st) i ## _inst
#include "PPCInsts.def"
} PPCInst;

typedef struct _PPCInstDef {
  const char *name;
  PPCInst inst;
  int32_t majorOpcode;
  int32_t extOpcode;
  PPCInstForm form;
  InstType type;
  InstSubType subType;
} PPCInstDef;


#endif

