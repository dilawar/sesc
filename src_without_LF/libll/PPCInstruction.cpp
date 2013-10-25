/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Luis Ceze

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

#include "icode.h"
#include "opcodes.h"
#include "globals.h"

#include "Instruction.h"
#include "PPCDecoder.h"

void Instruction::initializePPCTrace(int32_t argc,
				     char **argv,
				     char **envp)
{
  PPCDecoder::Initialize();
}

void Instruction::PPCDecodeInstruction(Instruction *inst, uint32_t rawInst)
{
  PPCInstDef *ppcInst = PPCDecoder::getInstDef(rawInst);

  inst->src1 = NoDependence;
  inst->dest = InvalidOutput;
  inst->src2 = NoDependence;
  inst->uEvent = NoEvent;
  inst->condLikely = false;
  inst->guessTaken = false;
  inst->jumpLabel = false;
  inst->addr = 0;

  inst->opcode = ppcInst->type;
  inst->subCode = ppcInst->subType;

  //TODO: add support for 2nd output reg
  //TODO: fix writes to cond regs
  //TODO: fix 3rd source reg (stux, for instance)

  switch(ppcInst->form) {
  case I_form: 
    inst->guessTaken = true;
    inst->jumpLabel = true;
    break;
  case B_form: 
    inst->src1 = CondReg;
    inst->guessTaken = true;
    inst->jumpLabel  = true;
    break;

  case SC_form: 
    break; // syscall instruction only. nothing to do;

  case D_form: 
  case DS_form: 
    {
      int32_t rD, rA, imm;
      PPC_DEC_D_UImm(rawInst, rD, rA, imm);
      
      switch(ppcInst->inst) {
      case cmpi_inst:
      case cmpli_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->dest = CondReg;
	break;
	
      case lfd_inst:
      case lfs_inst:
      case lfdu_inst: 
      case lfsu_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->dest = static_cast < RegType > (rD + IntFPBoundary);
	break;
	
      case stfd_inst:
      case stfdu_inst:
      case stfs_inst:
      case stfsu_inst: // stores have no dest
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rD + IntFPBoundary);
	break;
	
      case stb_inst:
      case stbu_inst:
      case sth_inst:
      case sthu_inst:
      case stmw_inst:
      case stw_inst:
      case stwu_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rD);
	
      default:
	inst->src1 = static_cast < RegType > (rA);
	inst->dest = static_cast < RegType > (rD);
      }
    }

  case X_form:
    {
      int32_t rS, rA, rB, xo;
      PPC_DEC_X(rawInst, rS, rA, rB, xo);

      switch(ppcInst->inst) {
      case cmp_inst:
      case cmpl_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rB);
	inst->dest = CondReg;
	break;
      case cntlzw_inst:
      case extsb_inst:
      case extsh_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->dest = static_cast < RegType > (rS);
	break;
	
      case dcba_inst:
      case dcbf_inst:
      case dcbi_inst:
      case dcbst_inst:
      case dcbt_inst:
      case dcbtst_inst:
      case dcbz_inst:
      case icbi_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rB);
	break;
	
      case eieio_inst: // no src, no dests!
	break;
	
      case fabs_inst:
      case fnabs_inst:
      case fneg_inst:
      case frsp_inst:
	inst->src1 = static_cast < RegType > (rB + IntFPBoundary);
	inst->dest = static_cast < RegType > (rS + IntFPBoundary);
	break;
	
      case fcmpo_inst:
      case fcmpu_inst:
	inst->dest = CondReg;
	inst->src1 = static_cast < RegType > (rA + IntFPBoundary);
	break;
	
	
      case fctiw_inst:
      case fctiwz_inst:
	inst->dest = static_cast < RegType > (rS);
	inst->src1 = static_cast < RegType > (rB + IntFPBoundary);
	break;
	
      case fmr_inst:
	inst->src1 = static_cast < RegType > (rB);
	inst->dest = static_cast < RegType > (rS);
	break;
	
      case mcrfs_inst:
	inst->src1 = CondReg;
	inst->dest = CondReg;
	break;
	
      case mcrxr_inst:
      case mtfsb0_inst:
      case mtfsb1_inst:
      case mtfsfi_inst:
	inst->dest = CondReg;
	break;
	
      case mffs_inst:
	inst->dest = static_cast < RegType > (rS);
	inst->src1 = CondReg;
	break;
	
      case mfmsr_inst:
	inst->dest = static_cast < RegType > (rS);
	break;
	
      case mfsr_inst:
	inst->dest = static_cast < RegType > (rS);
	break;
	
      case mfsrin_inst:
	inst->dest = static_cast < RegType > (rS);
	inst->src1 = static_cast < RegType > (rB);
	break;
	
      case mtmsr_inst:
      case mtsr_inst:
	inst->src1 = static_cast < RegType > (rS);
	break;
	
      case mtsrin_inst:
	inst->src1 = static_cast < RegType > (rB);
	inst->src2 = static_cast < RegType > (rS);
	break;
	
	
      case lfdux_inst:
      case lfdx_inst:
      case lfsux_inst:
      case lfsx_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rB);
	inst->dest = static_cast < RegType > (rS + IntFPBoundary);
	break;
	
      case stbux_inst:
      case stbx_inst:
      case sthbrx_inst:
      case sthux_inst:
      case sthx_inst:
      case stswi_inst:
      case stswx_inst:
      case stwbrx_inst:
      case stwcx__inst:
      case stwux_inst:
      case stwx_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rS);
	//TODO: add src3 = rB;
	break;
	
      case stfdux_inst:
      case stfdx_inst:
      case stfiwx_inst:
      case stfsux_inst:
      case stfsx_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rS + IntFPBoundary);
	break;
	
      case sync_inst:
      case tlbia_inst:
      case tlbsync_inst:
	break;
	
      case tlbie_inst:
	inst->src1 = static_cast < RegType > (rB);
	break;
	
      case tw_inst:
	inst->src1 = static_cast < RegType > (rA);
	inst->src1 = static_cast < RegType > (rB);
	break;
	
      default:
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rB);
	inst->dest = static_cast < RegType > (rS);
      }
      break;
    }
    
  case XL_form:
    inst->src1 = CondReg;
    inst->dest = CondReg;
    break;

  case XFX_form:
    {
      int32_t rS, crm, xo;
      PPC_DEC_XFX(rawInst, rS, crm, xo);
      switch(ppcInst->inst) {
      case mfspr_inst:
      case mftb_inst:
	inst->dest = static_cast < RegType > (rS);
	break;
	
      case mtcrf_inst:
      case mtspr_inst:
	inst->src1 = static_cast < RegType > (rS);
	break;
      default:
	break;
      }
      break;
    }
  case XFL_form: 
    {
      int32_t rB, fm, xo;
      PPC_DEC_XFL(rawInst, rB, fm, xo);
      inst->src1 = static_cast < RegType > (rB);
      break;
    }

  case XS_form:
    break;

  case XO_form: 
    {
      int32_t rS, rA, rB, xo;
      PPC_DEC_XO(rawInst, rS, rA, rB, xo);
      
      switch(ppcInst->inst) {
      case neg_inst:
	inst->dest = static_cast < RegType > (rS);
	inst->src1 = static_cast < RegType > (rA);
	break;
	
      case subfme_inst:
      case subfze_inst:
	inst->dest = static_cast < RegType > (rS);
	inst->src1 = static_cast < RegType > (rA);
	break;
	
      default:
	inst->dest = static_cast < RegType > (rS);
	inst->src1 = static_cast < RegType > (rA);
	inst->src2 = static_cast < RegType > (rB);
      }
    }
    
  case A_form: 
    {
      int32_t rD, rA, rB, rC, xo;
      PPC_DEC_A(rawInst, rD, rA, rB, rC, xo);
      rD += IntFPBoundary;
      rA += IntFPBoundary;
      rB += IntFPBoundary;
      rC += IntFPBoundary;
      
      inst->dest = static_cast < RegType > (rD);
      inst->src1 = static_cast < RegType > (rA);
      inst->src2 = static_cast < RegType > (rB);
      
      break;
    
    }
    
  case M_form: 
    {
      int32_t rS, rA, sh, mb, me;
      PPC_DEC_M(rawInst, rS, rA, sh, mb, me); 
      
      inst->dest = static_cast < RegType > (rS);
      inst->src1 = static_cast < RegType > (rA);
      
      switch(ppcInst->inst) {
      case rlwnmx_inst:
	inst->src2 = static_cast < RegType > (sh);
	break;
      default:
	break;
      }
      break;
    }
  default:
   I(0); // the instruction must have a valid form.
  }
  
  inst->src1Pool = whichPool(inst->src1);
  inst->src2Pool = whichPool(inst->src2);
  inst->dstPool  = whichPool(inst->dest);
  
}
