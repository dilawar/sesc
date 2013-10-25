/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2006 University California, Santa Cruz.

   Contributed by Jose Renau

This file is part of SESC.

 This code is based on QEMU sparc-dis.c. QEMU has a GPL2 license too.

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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include "SPARCInstruction.h"

// This function uses non-native bit order
#define GET_FIELD(X, FROM, TO) \
  ((X) >> (31 - (TO)) & ((1 << ((TO) - (FROM) + 1)) - 1))

// This function uses the order in the manuals, i.e. bit 0 is 2^0
#define GET_FIELD_SP(X, FROM, TO) \
    GET_FIELD(X, 31 - (TO), 31 - (FROM))

#define IS_IMM (insn & (1<<13))

void disas_sparc_insn(uint32_t insn,
		      InstType    &type, 
		      InstSubType &subType,
		      uint8_t &rd,
		      uint8_t &rs1,
		      uint8_t &rs2
		      )
{
  type    = iOpInvalid;
  subType = iSubInvalid;
  rs1 =0;
  rs2 =0;
  rd  = GET_FIELD(insn, 2, 6);

  uint32_t cc;
  uint32_t opc = GET_FIELD(insn, 0, 1);

  switch (opc) {
  case 0: {  /* branches/sethi */           
    uint32_t xop = GET_FIELD(insn, 7, 9);
    //nprintf(" xop in hex is %0x", xop);
    type = iBJ;
    switch (xop) {
    case 0x1: { /* V9 BPcc */
      int32_t cc = GET_FIELD_SP(insn, 20, 21);
      if (cc == 0)
	subType = BJUncond;
      else if (cc == 2)
	subType = BJCond;
      else
	return; // illegal_insn;
      return;
    }
    case 0x3: {  /* V9 BPr */
      rs1 = GET_FIELD(insn, 13, 17);

      subType = BJUncond;
      return;
    }
    case 0x5: { /* V9 FBPcc */
      subType = BJCond; // FP
      return;
    }
    case 0x2: {          /* BN+x */
      subType = BJUncond;
      return;
    }
    case 0x6: {          /* FBN+x */
      subType = BJCond; // FP
      return;
    }
    case 0x4:           /* SETHI */
      type    = iALU;
      if (rd) {
	subType = iSubInvalid;
      }else{
	subType = iNop;
      }
      break;
    case 0x0:           /* UNIMPL */
    default:
      return; // illegal_insn;
    }
    break;
  }
    break;
  case 1: { /*CALL*/
    type    = iBJ;
    subType = BJCall;
  }
    return; // jmp_insn;
  case 2: {                    /* FPU & Logical Operations */
    uint32_t xop = GET_FIELD(insn, 7, 12);
   
    if (xop == 0x3a) {  /* generate trap */
      rs1 = GET_FIELD(insn, 13, 17);
               
      if (IS_IMM) {
	rs2 = 0;
      } else {
	rs2 = GET_FIELD(insn, 27, 31);
      }
      
      type    = iBJ; // TRAP
      subType = BJUncond;
    } else if (xop == 0x28) {
      rs1 = GET_FIELD(insn, 13, 17);
            
      switch(rs1) {
      case 0: /* rdy */
	type    = iALU;
	subType = iSubInvalid;
	break;
      case 15: /* stbar / V9 membar */
	type    = iFence;
	subType = iMemFence;
	break;
      case 0x2: /* V9 rdccr */
      case 0x3: /* V9 rdasi */
      case 0x4: /* V9 rdtick */
      case 0x5: /* V9 rdpc */
      case 0x6: /* V9 rdfprs */
      case 0x17: /* Tick compare */
      case 0x18: /* System tick */
      case 0x19: /* System tick compare */
      case 0x10: /* Performance Control */
      case 0x11: /* Performance Instrumentation Counter */
      case 0x12: /* Dispatch Control */
      case 0x13: /* Graphics Status */
      case 0x14: /* Softint32_t set, WO */
      case 0x15: /* Softint32_t clear, WO */
      case 0x16: /* Softint32_t write */
	type    = iALU;
	subType = iSubInvalid;
	break;
      }
    } else if (xop == 0x2a) { /* RDPR */
      // Reads status register bits
      type    = iALU;
      subType = iSubInvalid;
    } else if (xop == 0x2b) { /* rdtbr / V9 flushw */
      type    = iFence;
      subType = iMemFence;
    } else if (xop == 0x34) {   /* FPU Operations */
      rs1 = GET_FIELD(insn, 13, 17);
      rs2 = GET_FIELD(insn, 27, 31);
      xop = GET_FIELD(insn, 18, 26);
      switch (xop) {
      case 0x1: /* fmovs */
      case 0x5: /* fnegs */
      case 0x9: /* fabss */
	type    = fpALU;
	subType = iSubInvalid;
	break;
      case 0x29: /* fsqrts */
      case 0x2a: /* fsqrtd */
      case 0x2b: /* fsqrtq */
	type    = fpDiv;
	subType = iSubInvalid;
	return; // nfpu_insn;
      case 0x41:
      case 0x42:
      case 0x43: /* faddq */
      case 0x45:
      case 0x46:
	type    = fpALU;
	subType = iSubInvalid;
	break;
      case 0x47: /* fsubq */
	type    = fpALU;
	subType = iSubInvalid;
	return; // nfpu_insn;
      case 0x49:
      case 0x4a:
      case 0x4b: /* fmulq */
	type    = fpMult;
	subType = iSubInvalid;
	return; // nfpu_insn;
      case 0x4d:
      case 0x4e:
      case 0x4f: /* fdivq */
	type    = fpDiv;
	subType = iSubInvalid;
	return; // nfpu_insn;
      case 0x69:
      case 0x6e: /* fdmulq */
	type    = fpMult;
	subType = iSubInvalid;
	return; // nfpu_insn;
      case 0xc4:
      case 0xc6:
      case 0xc7: /* fqtos */
      case 0xc8:
      case 0xc9:
      case 0xcb: /* fqtod */
      case 0xcc: /* fitoq */
      case 0xcd: /* fstoq */
      case 0xce: /* fdtoq */
      case 0xd1:
      case 0xd2:
      case 0xd3: /* fqtoi */
      case 0x2: /* V9 fmovd */
      case 0x6: /* V9 fnegd */
      case 0xa: /* V9 fabsd */
      case 0x81: /* V9 fstox */
      case 0x82: /* V9 fdtox */
      case 0x84: /* V9 fxtos */
      case 0x88: /* V9 fxtod */
      case 0x3: /* V9 fmovq */
      case 0x7: /* V9 fnegq */
      case 0xb: /* V9 fabsq */
      case 0x83: /* V9 fqtox */
      case 0x8c: /* V9 fxtoq */
	type    = fpALU;
	subType = iSubInvalid;
	return; // nfpu_insn;
      default:
	return; // illegal_insn;
      }
    } else if (xop == 0x35) {   /* FPU Operations */
      int32_t cond;
      rs1 = GET_FIELD(insn, 13, 17);
      rs2 = GET_FIELD(insn, 27, 31);
 
      xop = GET_FIELD(insn, 18, 26);

      type    = fpALU;
      subType = iSubInvalid;

    } else if (xop == 0x2) {
      // clr/mov shortcut
      rs1 = GET_FIELD(insn, 13, 17);
      if (IS_IMM) {       /* immediate */
	rs2 = 0;
      } else {            /* register */
	rs2 = GET_FIELD(insn, 27, 31);
      }

      type    = iALU;
      subType = iSubInvalid;
    } else if (xop == 0x25     /* sll, V9 sllx ( == sll) */
	       || xop == 0x26  /* srl, V9 srlx */
	       || xop == 0x27  /* sra, V9 srax */
	       ) { 

      rs1 = GET_FIELD(insn, 13, 17);
      if (IS_IMM) {       /* immediate */
	rs2 = 0;
      } else {            /* register */
	rs2 = GET_FIELD(insn, 27, 31);
      }

      type    = iALU;
      subType = iSubInvalid;

    } else if (xop < 0x38) {

      rs1 = GET_FIELD(insn, 13, 17);
      if (IS_IMM) {   /* immediate */
	rs2 = 0;
      } else {                /* register */
	rs2 = GET_FIELD(insn, 27, 31);
      }
      
      if (xop < 0x20) {
	type    = iALU;
	subType = iSubInvalid;
      } else {

	switch (xop) {
	case 0x9: /* V9 mulx */
	  type    = iMult;
	  subType = iSubInvalid;
	  break;
	case 0xd: /* V9 udivx */
	  type    = iDiv;
	  subType = iSubInvalid;
	  break;
	case 0x20: /* taddcc */
	case 0x21: /* tsubcc */
	case 0x22: /* taddcctv */
	case 0x23: /* tsubcctv */
	  type    = iALU;
	  subType = iSubInvalid;
	  break;
	case 0x24: /* mulscc */
	  type    = iMult;
	  subType = iSubInvalid;
	  break;
	case 0x25:  /* sll */
	case 0x26:  /* srl */
	case 0x27:  /* sra */
	case 0x30:
	case 0x31: /* wrpsr, V9 saved, restored */
	case 0x32: /* wrwim, V9 wrpr */
	case 0x33: /* wrtbr, V9 unimp */
	  type    = iALU;
	  subType = iSubInvalid;
	  break;
	case 0x2c: /* V9 movcc */
	  if (IS_IMM) {       /* immediate */
	    rs2 = 0;
	  } else {
	    rs2 = GET_FIELD_SP(insn, 0, 4);
	  }
	  type    = iALU;
	  subType = iSubInvalid;
	  break;
	case 0x2d: /* V9 sdivx */
	  type    = iDiv;
	  subType = iSubInvalid;
	  break;
	case 0x2e: { /* V9 popc */
	  if (IS_IMM) {       /* immediate */
	    rs2 =0; // XXX optimize: popc(constant)
	  } else {
	    rs2 = GET_FIELD_SP(insn, 0, 4);
	  }
	  type    = iALU;
	  subType = iSubInvalid;
	  break;
	}
	case 0x2f: { /* V9 movr */
	  int32_t cond = GET_FIELD_SP(insn, 10, 12);
	  rs1 = GET_FIELD(insn, 13, 17);
	  if (IS_IMM) {       /* immediate */
	    rs2 =0;
	  } else {
	    rs2 = GET_FIELD_SP(insn, 0, 4);
	  }
	  type    = iALU;
	  subType = iSubInvalid;
	  break;
	}
	case 0x36: { /* UltraSparc shutdown, VIS */
	  type    = iALU;
	  subType = iSubInvalid;
	  // XXX
	}
	default:
	  return; // illegal_insn;
	}
      }
    } else if (xop == 0x39) { /* V9 return */
      //printf("\t xop is return");
      rs1 = GET_FIELD(insn, 13, 17);
      type    = iBJ;
      subType = BJRet;
      if (IS_IMM) {   /* immediate */
	rs2 = 0;
      } else {                /* register */
	rs2 = GET_FIELD(insn, 27, 31);
      }
      return; // jmp_insn;
    } else {
      rs1 = GET_FIELD(insn, 13, 17);
      if (IS_IMM) {   /* immediate */
	rs2 = 0;
      } else {                /* register */
	rs2 = GET_FIELD(insn, 27, 31);
      }
      switch (xop) {
      case 0x38:       /* jmpl */
	type    = iBJ;
	subType = BJCall;
	return; // jmp_insn;
      case 0x39:       /* rett, V9 return */
	type    = iBJ;
	subType = BJRet;
	return; // jmp_insn;
      case 0x3b: /* flush */
	type    = iALU;
	subType = iSubInvalid;
	break;
      case 0x3c:      /* save */
	type    = iALU;
	subType = iSubInvalid;
	break;
      case 0x3d:      /* restore */
	type    = iALU;
	subType = iSubInvalid;
	break;
      case 0x3e:      /* V9 done/retry */
	type    = iBJ;
	subType = BJUncond;
	break;
      default:
	return; // illegal_insn;
      }
    }
    break;
  }
    break;
  case 3: {                     /* load/store instructions */
             
    uint32_t xop = GET_FIELD(insn, 7, 12);
    rs1 = GET_FIELD(insn, 13, 17);

    if (IS_IMM) {       /* immediate */
      rs2 = 0;
      type    = iALU;
      subType = iSubInvalid;
    } else {            /* register */
      rs2 = GET_FIELD(insn, 27, 31);
      type    = iLoad;
      subType = iMemory;
    }
    if (xop < 4 || (xop > 7 && xop < 0x14 && xop != 0x0e) ||
		    (xop > 0x17 && xop < 0x1d ) ||
		    (xop > 0x2c && xop < 0x33) || xop == 0x1f) {
      switch (xop) {
      case 0x0:       /* load word */
      case 0x1:       /* load unsigned byte */
      case 0x2:       /* load unsigned halfword */
      case 0x3:       /* load double word */
      case 0x9:       /* load signed byte */
      case 0xa:       /* load signed halfword */
      case 0xd:       /* ldstub -- XXX: should be atomically */
	type    = iLoad;
	subType = iMemory;
	break;
      case 0x0f:      /* swap register with memory. Also atomically */
	type    = iFence;
	subType = iFetchOp;
	break;
      case 0x10:      /* load word alternate */
      case 0x11:      /* load unsigned byte alternate */
      case 0x12:      /* load unsigned halfword alternate */
      case 0x13:      /* load double word alternate */
      case 0x19:      /* load signed byte alternate */
      case 0x1a:      /* load signed halfword alternate */
      case 0x1d:      /* ldstuba -- XXX: should be atomically */
	type    = iLoad;
	subType = iMemory;
	break;
      case 0x1f:      /* swap reg with alt. memory. Also atomically */
	type    = iFence;
	subType = iFetchOp;
	break;
      case 0x08: /* V9 ldsw */
      case 0x0b: /* V9 ldx */
      case 0x18: /* V9 ldswa */
      case 0x1b: /* V9 ldxa */
	type    = iLoad;
	subType = iMemory;
	break;
      case 0x2d: /* V9 prefetch, no effect */
	type    = iALU;
	subType = iNop;
	return; // skip_move;
      case 0x30: /* V9 ldfa */
      case 0x33: /* V9 lddfa */
	type    = iLoad;
	subType = iMemory;
	break;
      case 0x3d: /* V9 prefetcha, no effect */
	type    = iALU;
	subType = iNop;
	return; // skip_move;
      case 0x32: /* V9 ldqfa */
	type    = iLoad;
	subType = iMemory;
	return; // nfpu_insn;
      default:
	return; // illegal_insn;
      }
    skip_move: ;
    } else if (xop >= 0x20 && xop < 0x24) {
      type    = iLoad;
      subType = iMemory;
    } else {
      type    = iFence;
      subType = iFetchOp;
    }
  }
    break;
  }
}

