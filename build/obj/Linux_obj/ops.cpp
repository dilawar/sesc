/*
 * Macros for generating multiple versions of functions to simulate an
 * instruction.
 *
 * Copyright (C) 1993 by Jack E. Veenstra (veenstra@cs.rochester.edu)
 * 
 * This file is part of MINT, a MIPS code interpreter and event generator
 * for parallel programs.
 * 
 * MINT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * MINT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MINT; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* If you think m4 source is hard to read, you should try *writing* it! */

/* If this is C code you are reading, this file was machine generated
 * using m4 macros. See the corresponding file.m4 for the source.
 */

/* Do not use a comma within a C comment inside an m4 macro!!! */

/* Change the quote character so the input file looks more like C code. */


/* This macro is used repeatedly to generate two versions of a function.
 * The first version is called for an instruction that is not in the branch
 * delay slot of the previously executed instruction. The second version
 * is called for an instruction that is in the branch delay slot of the
 * previously executed instruction.
 */


/* Byte READ */


/* Word READ */




/* floating point read */


/* double floating point read */


/* The following macro creates code to check that an address does not
 * have an active ll (load-linked) operation pending on it. It generates
 * this check on EVERY WRITE instruction.
 */


/* The following macro creates code to check that an address does not
 * have an active ll (load-linked) operation pending on it. It generates
 * this check only for sc (store-conditional) instructions.
 */


/* arg 5 is the value to write on successful sc; arg 6 is the value to
 * write on failed sc. When Verify_protocol is set we need to use the value
 * setup by the back-end.
 */




/* Define normal version only; no branch delay slot version */




/* Local Variables: */
/* mode: c */
/* End: */
/*
 * Routines for implementing the MIPS instruction set.
 *
 * Copyright (C) 1993 by Jack E. Veenstra (veenstra@cs.rochester.edu)
 * 
 * This file is part of MINT, a MIPS code interpreter and event generator
 * for parallel programs.
 * 
 * MINT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * MINT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MINT; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#include "icode.h"
#include "ThreadContext.h"
#include "globals.h"
#include "opcodes.h"
#include "mendian.h"

#if ( defined(TASKSCALAR))
int rsesc_exception(int pid);
int rsesc_is_safe(int pid);
int rsesc_become_safe(int pid);
#endif

/* opcode functions in alphabetical order (more or less) */

/* normal version */
OP(add_op_0)
{

    int val;

    val = pthread->getREG(picode, RS) + pthread->getREG(picode, RT);
#ifdef OVERFLOW_CHK
    /* need to check for overflow here */
#endif

    pthread->setREG(picode, RD, val);

    return picode->next;
}

/* branch delay slot version */
OP(add_op_1)
{

    int val;

    val = pthread->getREG(picode, RS) + pthread->getREG(picode, RT);
#ifdef OVERFLOW_CHK
    /* need to check for overflow here */
#endif

    pthread->setREG(picode, RD, val);

    return pthread->getTarget();
}

PFPI add_op[] = { add_op_0, add_op_1 };


/* normal version */
OP(addi_op_0)
{

    int val;

    val = pthread->getREG(picode, RS) + picode->immed;
#ifdef OVERFLOW_CHK
    /* need to check for overflow here */
#endif

    pthread->setREG(picode, RT, val);

    return picode->next;
}

/* branch delay slot version */
OP(addi_op_1)
{

    int val;

    val = pthread->getREG(picode, RS) + picode->immed;
#ifdef OVERFLOW_CHK
    /* need to check for overflow here */
#endif

    pthread->setREG(picode, RT, val);

    return pthread->getTarget();
}

PFPI addi_op[] = { addi_op_0, addi_op_1 };


/* normal version */
OP(addiu_op_0)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) + picode->immed);

    return picode->next;
}

/* branch delay slot version */
OP(addiu_op_1)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) + picode->immed);

    return pthread->getTarget();
}

PFPI addiu_op[] = { addiu_op_0, addiu_op_1 };


/* normal version */
OP(sp_over_op_0)
{

  pthread->setREGNUM(29, pthread->getREG(picode, RS) + picode->immed);
  if(pthread->getStkPtr()<pthread->getStackTop()){
#ifdef TASKSCALAR
    if(!rsesc_is_safe(pthread->getPid())) {
      rsesc_exception(pthread->getPid());
#ifdef TS_CAVA
      rsesc_become_safe(pthread->getPid());
#endif
    } else {
#endif
      fprintf(stderr, "stack overflow at instruction 0x%x sp v=0x%x p=0x%x stack top=0x%x\n", picode->addr
	      ,pthread->virt2real(pthread->getStkPtr())
                      ,pthread->getREGNUM(29)
                      ,pthread->getStackTop());
      fprintf(stderr, "Increase the stack size with the `-k' option.\n");
      exit(1);
#ifdef TASKSCALAR
    }
#endif
  }

    return picode->next;
}

/* branch delay slot version */
OP(sp_over_op_1)
{

  pthread->setREGNUM(29, pthread->getREG(picode, RS) + picode->immed);
  if(pthread->getStkPtr()<pthread->getStackTop()){
#ifdef TASKSCALAR
    if(!rsesc_is_safe(pthread->getPid())) {
      rsesc_exception(pthread->getPid());
#ifdef TS_CAVA
      rsesc_become_safe(pthread->getPid());
#endif
    } else {
#endif
      fprintf(stderr, "stack overflow at instruction 0x%x sp v=0x%x p=0x%x stack top=0x%x\n", picode->addr
	      ,pthread->virt2real(pthread->getStkPtr())
                      ,pthread->getREGNUM(29)
                      ,pthread->getStackTop());
      fprintf(stderr, "Increase the stack size with the `-k' option.\n");
      exit(1);
#ifdef TASKSCALAR
    }
#endif
  }

    return pthread->getTarget();
}

PFPI sp_over_op[] = { sp_over_op_0, sp_over_op_1 };


/* normal version */
OP(li_op_0)
{

  pthread->setREG(picode, RT, picode->immed);

    return picode->next;
}

/* branch delay slot version */
OP(li_op_1)
{

  pthread->setREG(picode, RT, picode->immed);

    return pthread->getTarget();
}

PFPI li_op[] = { li_op_0, li_op_1 };


/* normal version */
OP(addiu_xx_op_0)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RT) + picode->immed);

    return picode->next;
}

/* branch delay slot version */
OP(addiu_xx_op_1)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RT) + picode->immed);

    return pthread->getTarget();
}

PFPI addiu_xx_op[] = { addiu_xx_op_0, addiu_xx_op_1 };


/* normal version */
OP(addu_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) + pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(addu_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) + pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI addu_op[] = { addu_op_0, addu_op_1 };


/* normal version */
OP(move_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS));

    return picode->next;
}

/* branch delay slot version */
OP(move_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS));

    return pthread->getTarget();
}

PFPI move_op[] = { move_op_0, move_op_1 };


/* normal version */
OP(move0_op_0)
{

  pthread->setREG(picode, RD, 0);

    return picode->next;
}

/* branch delay slot version */
OP(move0_op_1)
{

  pthread->setREG(picode, RD, 0);

    return pthread->getTarget();
}

PFPI move0_op[] = { move0_op_0, move0_op_1 };


/* normal version */
OP(and_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) & pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(and_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) & pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI and_op[] = { and_op_0, and_op_1 };


/* normal version */
OP(andi_op_0)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) & (unsigned short) picode->immed);

    return picode->next;
}

/* branch delay slot version */
OP(andi_op_1)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) & (unsigned short) picode->immed);

    return pthread->getTarget();
}

PFPI andi_op[] = { andi_op_0, andi_op_1 };


/* normal version */
OP(beq_op_0)
{

  if (pthread->getREG(picode, RS) == pthread->getREG(picode, RT))
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(beq_op_1)
{

  if (pthread->getREG(picode, RS) == pthread->getREG(picode, RT))
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI beq_op[] = { beq_op_0, beq_op_1 };


/* normal version */
OP(b_op_0)
{

    pthread->setTarget(picode->target);

    return picode->next;
}

/* branch delay slot version */
OP(b_op_1)
{

    pthread->setTarget(picode->target);

    return pthread->getTarget();
}

PFPI b_op[] = { b_op_0, b_op_1 };


/* normal version */
OP(beq0_op_0)
{

    if (pthread->getREG(picode, RS) == 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(beq0_op_1)
{

    if (pthread->getREG(picode, RS) == 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI beq0_op[] = { beq0_op_0, beq0_op_1 };


/* normal version */
OP(beql_op_0)
{

    /* R4000 instruction */
    if (pthread->getREG(picode, RS) == pthread->getREG(picode, RT))
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return picode->next;
}

/* branch delay slot version */
OP(beql_op_1)
{

    /* R4000 instruction */
    if (pthread->getREG(picode, RS) == pthread->getREG(picode, RT))
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return pthread->getTarget();
}

PFPI beql_op[] = { beql_op_0, beql_op_1 };


/* normal version */
OP(bgez_op_0)
{

    if (pthread->getREG(picode, RS) >= 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bgez_op_1)
{

    if (pthread->getREG(picode, RS) >= 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bgez_op[] = { bgez_op_0, bgez_op_1 };


/* normal version */
OP(bgezal_op_0)
{

    /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) >= 0)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bgezal_op_1)
{

    /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) >= 0)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bgezal_op[] = { bgezal_op_0, bgezal_op_1 };


/* normal version */
OP(bgezall_op_0)
{

  /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) >= 0)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return picode->next;
}

/* branch delay slot version */
OP(bgezall_op_1)
{

  /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) >= 0)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return pthread->getTarget();
}

PFPI bgezall_op[] = { bgezall_op_0, bgezall_op_1 };


/* normal version */
OP(bgezl_op_0)
{

    if (pthread->getREG(picode, RS) >= 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return picode->next;
}

/* branch delay slot version */
OP(bgezl_op_1)
{

    if (pthread->getREG(picode, RS) >= 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return pthread->getTarget();
}

PFPI bgezl_op[] = { bgezl_op_0, bgezl_op_1 };


/* normal version */
OP(bgtz_op_0)
{

    if (pthread->getREG(picode, RS) > 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bgtz_op_1)
{

    if (pthread->getREG(picode, RS) > 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bgtz_op[] = { bgtz_op_0, bgtz_op_1 };


/* normal version */
OP(bgtzl_op_0)
{

    if (pthread->getREG(picode, RS) > 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return picode->next;
}

/* branch delay slot version */
OP(bgtzl_op_1)
{

    if (pthread->getREG(picode, RS) > 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return pthread->getTarget();
}

PFPI bgtzl_op[] = { bgtzl_op_0, bgtzl_op_1 };


/* normal version */
OP(blez_op_0)
{

    if (pthread->getREG(picode, RS) <= 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(blez_op_1)
{

    if (pthread->getREG(picode, RS) <= 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI blez_op[] = { blez_op_0, blez_op_1 };


/* normal version */
OP(blezl_op_0)
{

    if (pthread->getREG(picode, RS) <= 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return picode->next;
}

/* branch delay slot version */
OP(blezl_op_1)
{

    if (pthread->getREG(picode, RS) <= 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return pthread->getTarget();
}

PFPI blezl_op[] = { blezl_op_0, blezl_op_1 };


/* normal version */
OP(bltz_op_0)
{

    if (pthread->getREG(picode, RS) < 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bltz_op_1)
{

    if (pthread->getREG(picode, RS) < 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bltz_op[] = { bltz_op_0, bltz_op_1 };


/* normal version */
OP(bltzal_op_0)
{

  /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) < 0)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bltzal_op_1)
{

  /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) < 0)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bltzal_op[] = { bltzal_op_0, bltzal_op_1 };


/* normal version */
OP(bltzall_op_0)
{

  /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) < 0)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return picode->next;
}

/* branch delay slot version */
OP(bltzall_op_1)
{

  /* The return location is unconditionally copied to r31 */
  pthread->setREGNUM(31, icode2addr(picode->not_taken));
  if (pthread->getREG(picode, RS) < 0)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return pthread->getTarget();
}

PFPI bltzall_op[] = { bltzall_op_0, bltzall_op_1 };


/* normal version */
OP(bltzl_op_0)
{

    if (pthread->getREG(picode, RS) < 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return picode->next;
}

/* branch delay slot version */
OP(bltzl_op_1)
{

    if (pthread->getREG(picode, RS) < 0)
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return pthread->getTarget();
}

PFPI bltzl_op[] = { bltzl_op_0, bltzl_op_1 };


/* normal version */
OP(bne_op_0)
{

    if (pthread->getREG(picode, RS) != pthread->getREG(picode, RT))
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bne_op_1)
{

    if (pthread->getREG(picode, RS) != pthread->getREG(picode, RT))
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bne_op[] = { bne_op_0, bne_op_1 };


/* normal version */
OP(bne0_op_0)
{

    if (pthread->getREG(picode, RS) != 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bne0_op_1)
{

    if (pthread->getREG(picode, RS) != 0)
        pthread->setTarget(picode->target);
    else
        pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bne0_op[] = { bne0_op_0, bne0_op_1 };


/* normal version */
OP(bnel_op_0)
{

    /* R4000 instruction */
    if (pthread->getREG(picode, RS) != pthread->getREG(picode, RT))
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return picode->next;
}

/* branch delay slot version */
OP(bnel_op_1)
{

    /* R4000 instruction */
    if (pthread->getREG(picode, RS) != pthread->getREG(picode, RT))
        pthread->setTarget(picode->target);
    else {
        /* The conditional branch is not taken, so nullify the branch delay
         * slot instruction, but still count one cycle for the cost.
         */
        return picode->not_taken;
    }

    return pthread->getTarget();
}

PFPI bnel_op[] = { bnel_op_0, bnel_op_1 };


/* normal version */
OP(break_op_0)
{


#ifdef TASKSCALAR
    if(!rsesc_is_safe(pthread->getPid())) {
        rsesc_exception(pthread->getPid());
    } else
#endif
    fatal("break instruction at 0x%x (division by zero?)\n", picode->addr);


    return picode->next;
}

/* branch delay slot version */
OP(break_op_1)
{


#ifdef TASKSCALAR
    if(!rsesc_is_safe(pthread->getPid())) {
        rsesc_exception(pthread->getPid());
    } else
#endif
    fatal("break instruction at 0x%x (division by zero?)\n", picode->addr);


    return pthread->getTarget();
}

PFPI break_op[] = { break_op_0, break_op_1 };


/* normal version */
OP(cache_op_0)
{

    fatal("cache: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cache_op_1)
{

    fatal("cache: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cache_op[] = { cache_op_0, cache_op_1 };


/* normal version */
OP(div_op_0)
{

#ifdef TASKSCALAR
  if(pthread->getREG(picode, RT) == 0) {
      rsesc_exception(pthread->getPid());
      pthread->setREG(picode, RT, 0);
  } else 
#endif
  mips_div(pthread->getREG(picode, RS), pthread->getREG(picode, RT), &(pthread->lo), &(pthread->hi));

    return picode->next;
}

/* branch delay slot version */
OP(div_op_1)
{

#ifdef TASKSCALAR
  if(pthread->getREG(picode, RT) == 0) {
      rsesc_exception(pthread->getPid());
      pthread->setREG(picode, RT, 0);
  } else 
#endif
  mips_div(pthread->getREG(picode, RS), pthread->getREG(picode, RT), &(pthread->lo), &(pthread->hi));

    return pthread->getTarget();
}

PFPI div_op[] = { div_op_0, div_op_1 };


/* normal version */
OP(divu_op_0)
{

#ifdef TASKSCALAR
        if(((unsigned int) pthread->getREG(picode, RT)) == 0) {
      rsesc_exception(pthread->getPid());
      pthread->setREG(picode, RT, 0);
   } else 
#endif
         mips_divu((unsigned int) pthread->getREG(picode, RS),
                (unsigned int) pthread->getREG(picode, RT),
      &(pthread->lo), &(pthread->hi));

    return picode->next;
}

/* branch delay slot version */
OP(divu_op_1)
{

#ifdef TASKSCALAR
        if(((unsigned int) pthread->getREG(picode, RT)) == 0) {
      rsesc_exception(pthread->getPid());
      pthread->setREG(picode, RT, 0);
   } else 
#endif
         mips_divu((unsigned int) pthread->getREG(picode, RS),
                (unsigned int) pthread->getREG(picode, RT),
      &(pthread->lo), &(pthread->hi));

    return pthread->getTarget();
}

PFPI divu_op[] = { divu_op_0, divu_op_1 };


/* normal version */
OP(j_op_0)
{

  pthread->setTarget(picode->target);

    return picode->next;
}

/* branch delay slot version */
OP(j_op_1)
{

  pthread->setTarget(picode->target);

    return pthread->getTarget();
}

PFPI j_op[] = { j_op_0, j_op_1 };


/* normal version */
OP(jal_op_0)
{

  pthread->setREGNUM(31, picode->addr + 8);
          
  pthread->setTarget(picode->target);

    return picode->next;
}

/* branch delay slot version */
OP(jal_op_1)
{

  pthread->setREGNUM(31, picode->addr + 8);
          
  pthread->setTarget(picode->target);

    return pthread->getTarget();
}

PFPI jal_op[] = { jal_op_0, jal_op_1 };


/* normal version */
OP(jalr_op_0)
{

    icode_ptr iaddr;

    pthread->setREG(picode, RD, picode->addr + 8);

#if (defined(TASKSCALAR))
    if (pthread->getREG(picode, RS) & 0x3
        ||
         pthread->getREG(picode, RS) < Text_start 
        || pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
#ifdef DEBUG
      fprintf(stderr,"Jump to hell 0x%x target 0x%x\n",picode->addr, pthread->getREG(picode, RS));
#endif
      rsesc_exception(pthread->getPid());
#ifdef TS_CAVA
      rsesc_become_safe(pthread->getPid());
#endif
      iaddr = picode;
    }else{
      iaddr = addr2icode(pthread->getREG(picode, RS));
    }
#else
#ifdef ADDRESS_CHK
    if (pthread->getREG(picode, RS) & 0x3)
        address_exception_op(picode, pthread);
#endif
#ifdef DEBUG
         if ((signed int) pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
        fatal("target address (0x%x) of jalr instruction at addr 0x%x is past end of text.\n",
              pthread->getREG(picode, RS), picode->addr);
    }
#endif

    iaddr = addr2icode(pthread->getREG(picode, RS));
#endif

    pthread->setTarget(iaddr);

    return picode->next;
}

/* branch delay slot version */
OP(jalr_op_1)
{

    icode_ptr iaddr;

    pthread->setREG(picode, RD, picode->addr + 8);

#if (defined(TASKSCALAR))
    if (pthread->getREG(picode, RS) & 0x3
        ||
         pthread->getREG(picode, RS) < Text_start 
        || pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
#ifdef DEBUG
      fprintf(stderr,"Jump to hell 0x%x target 0x%x\n",picode->addr, pthread->getREG(picode, RS));
#endif
      rsesc_exception(pthread->getPid());
#ifdef TS_CAVA
      rsesc_become_safe(pthread->getPid());
#endif
      iaddr = picode;
    }else{
      iaddr = addr2icode(pthread->getREG(picode, RS));
    }
#else
#ifdef ADDRESS_CHK
    if (pthread->getREG(picode, RS) & 0x3)
        address_exception_op(picode, pthread);
#endif
#ifdef DEBUG
         if ((signed int) pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
        fatal("target address (0x%x) of jalr instruction at addr 0x%x is past end of text.\n",
              pthread->getREG(picode, RS), picode->addr);
    }
#endif

    iaddr = addr2icode(pthread->getREG(picode, RS));
#endif

    pthread->setTarget(iaddr);

    return pthread->getTarget();
}

PFPI jalr_op[] = { jalr_op_0, jalr_op_1 };



/* normal version */
OP(jr_op_0)
{

    icode_ptr iaddr;

#if (defined(TASKSCALAR))
    if (pthread->getREG(picode, RS) & 0x3
        ||
         pthread->getREG(picode, RS) < Text_start 
        || pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
#ifdef DEBUG
      fprintf(stderr,"Jump to hell 0x%x target 0x%x\n",picode->addr, pthread->getREG(picode, RS));
#endif
      rsesc_exception(pthread->getPid());
#ifdef TS_CAVA
      rsesc_become_safe(pthread->getPid());
#endif
      iaddr = picode;
    }else{
      iaddr = addr2icode(pthread->getREG(picode, RS));
    }
#else
#ifdef ADDRESS_CHK
    if (pthread->getREG(picode, RS) & 0x3)
        address_exception_op(picode, pthread);
#endif
#ifdef DEBUG
         if ((signed int) pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
        fatal("target address (0x%x) of jr instruction at addr 0x%x is past end of text.\n",
              pthread->getREG(picode, RS), picode->addr);
    }
#endif
    iaddr = addr2icode(pthread->getREG(picode, RS));
#endif

    pthread->setTarget(iaddr);

    return picode->next;
}

/* branch delay slot version */
OP(jr_op_1)
{

    icode_ptr iaddr;

#if (defined(TASKSCALAR))
    if (pthread->getREG(picode, RS) & 0x3
        ||
         pthread->getREG(picode, RS) < Text_start 
        || pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
#ifdef DEBUG
      fprintf(stderr,"Jump to hell 0x%x target 0x%x\n",picode->addr, pthread->getREG(picode, RS));
#endif
      rsesc_exception(pthread->getPid());
#ifdef TS_CAVA
      rsesc_become_safe(pthread->getPid());
#endif
      iaddr = picode;
    }else{
      iaddr = addr2icode(pthread->getREG(picode, RS));
    }
#else
#ifdef ADDRESS_CHK
    if (pthread->getREG(picode, RS) & 0x3)
        address_exception_op(picode, pthread);
#endif
#ifdef DEBUG
         if ((signed int) pthread->getREG(picode, RS) >= (Text_end + 4 * EXTRA_ICODES)) {
        fatal("target address (0x%x) of jr instruction at addr 0x%x is past end of text.\n",
              pthread->getREG(picode, RS), picode->addr);
    }
#endif
    iaddr = addr2icode(pthread->getREG(picode, RS));
#endif

    pthread->setTarget(iaddr);

    return pthread->getTarget();
}

PFPI jr_op[] = { jr_op_0, jr_op_1 };


/* Pre-computed address */
/* Normal version */
OP(lb_op_2)
{

  RAddr raddr = pthread->getRAddr();

    /* read value from memory */
  pthread->setREG(picode, RT, (int) *(signed char *) raddr);


    return picode->next;
}


/* This version is for branch delay slots */
OP(lb_op_3)
{

  RAddr raddr = pthread->getRAddr();

    /* read value from memory */
  pthread->setREG(picode, RT, (int) *(signed char *) raddr);


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lb_op[] = {
lb_op_2, lb_op_3, lb_op_2, lb_op_3, lb_op_2, lb_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lbu_op_2)
{

  RAddr raddr = pthread->getRAddr();

    /* read value from memory */
  pthread->setREG(picode, RT, (int) *(unsigned char *) raddr);


    return picode->next;
}


/* This version is for branch delay slots */
OP(lbu_op_3)
{

  RAddr raddr = pthread->getRAddr();

    /* read value from memory */
  pthread->setREG(picode, RT, (int) *(unsigned char *) raddr);


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lbu_op[] = {
lbu_op_2, lbu_op_3, lbu_op_2, lbu_op_3, lbu_op_2, lbu_op_3
};


/* Pre-computed address */
/* Normal version */
OP(ldc1_op_2)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
    if (raddr & 7)
        address_exception_op(picode, pthread);
#endif

    /* read value from memory */
    pthread->setDPFromMem(picode, ICODEFT, (double *) raddr);


    return picode->next;
}


/* This version is for branch delay slots */
OP(ldc1_op_3)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
    if (raddr & 7)
        address_exception_op(picode, pthread);
#endif

    /* read value from memory */
    pthread->setDPFromMem(picode, ICODEFT, (double *) raddr);


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI ldc1_op[] = {
ldc1_op_2, ldc1_op_3, ldc1_op_2, ldc1_op_3, ldc1_op_2, ldc1_op_3
};


/* Pre-computed address */
/* Normal version */
OP(ldc2_op_2)
{

  RAddr raddr = pthread->getRAddr();

    fatal("ldc2: not yet implemented\n");


    return picode->next;
}


/* This version is for branch delay slots */
OP(ldc2_op_3)
{

  RAddr raddr = pthread->getRAddr();

    fatal("ldc2: not yet implemented\n");


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI ldc2_op[] = {
ldc2_op_2, ldc2_op_3, ldc2_op_2, ldc2_op_3, ldc2_op_2, ldc2_op_3
};


/* Pre-computed address */
/* Normal version */
OP(ldc3_op_2)
{

  RAddr raddr = pthread->getRAddr();

    fatal("ldc3: not yet implemented\n");


    return picode->next;
}


/* This version is for branch delay slots */
OP(ldc3_op_3)
{

  RAddr raddr = pthread->getRAddr();

    fatal("ldc3: not yet implemented\n");


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI ldc3_op[] = {
ldc3_op_2, ldc3_op_3, ldc3_op_2, ldc3_op_3, ldc3_op_2, ldc3_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lh_op_2)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 1)
    address_exception_op(picode, pthread);
#endif
#ifdef LENDIAN
{
  unsigned short val = *(unsigned short *) raddr;
  val = SWAP_SHORT(val);
  pthread->setREG(picode, RT, *(signed short *)&val);
}
#else
 pthread->setREG(picode, RT, (int ) *(signed short *) raddr);
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(lh_op_3)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 1)
    address_exception_op(picode, pthread);
#endif
#ifdef LENDIAN
{
  unsigned short val = *(unsigned short *) raddr;
  val = SWAP_SHORT(val);
  pthread->setREG(picode, RT, *(signed short *)&val);
}
#else
 pthread->setREG(picode, RT, (int ) *(signed short *) raddr);
#endif


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lh_op[] = {
lh_op_2, lh_op_3, lh_op_2, lh_op_3, lh_op_2, lh_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lhu_op_2)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 1)
    address_exception_op(picode, pthread);
#endif
  pthread->setREG(picode, RT, (int ) *(unsigned short *) raddr);
#ifdef LENDIAN
  pthread->setREG(picode, RT, SWAP_SHORT(pthread->getREG(picode, RT)));
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(lhu_op_3)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 1)
    address_exception_op(picode, pthread);
#endif
  pthread->setREG(picode, RT, (int ) *(unsigned short *) raddr);
#ifdef LENDIAN
  pthread->setREG(picode, RT, SWAP_SHORT(pthread->getREG(picode, RT)));
#endif


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lhu_op[] = {
lhu_op_2, lhu_op_3, lhu_op_2, lhu_op_3, lhu_op_2, lhu_op_3
};


/* Pre-computed address */
/* Normal version */
OP(ll_op_2)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* read value from memory */
  pthread->setREGFromMem(picode, RT, (int *) raddr);


    return picode->next;
}

/* This version is for branch delay slots */
OP(ll_op_3)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* read value from memory */
  pthread->setREGFromMem(picode, RT, (int *) raddr);


    return pthread->getTarget();
}

/* FIXME: replicated methods. Do not index */
PFPI ll_op[] = {
ll_op_2, ll_op_3, ll_op_2, ll_op_3, ll_op_2, ll_op_3
};


/* normal version */
OP(lui_op_0)
{

  pthread->setREG(picode,RT,((IntRegValue)picode->immed)<<16);

    return picode->next;
}

/* branch delay slot version */
OP(lui_op_1)
{

  pthread->setREG(picode,RT,((IntRegValue)picode->immed)<<16);

    return pthread->getTarget();
}

PFPI lui_op[] = { lui_op_0, lui_op_1 };


/* Pre-computed address */
/* Normal version */
OP(lw_op_2)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* read value from memory */
  pthread->setREGFromMem(picode, RT, (int *) raddr);


    return picode->next;
}


/* This version is for branch delay slots */
OP(lw_op_3)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* read value from memory */
  pthread->setREGFromMem(picode, RT, (int *) raddr);


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lw_op[] = {
lw_op_2, lw_op_3, lw_op_2, lw_op_3, lw_op_2, lw_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lwc1_op_2)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* read value from memory */
  pthread->setFPFromMem(picode, ICODEFT, (float *) raddr);


    return picode->next;
}


/* This version is for branch delay slots */
OP(lwc1_op_3)
{

  RAddr raddr = pthread->getRAddr();

#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* read value from memory */
  pthread->setFPFromMem(picode, ICODEFT, (float *) raddr);


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lwc1_op[] = {
lwc1_op_2, lwc1_op_3, lwc1_op_2, lwc1_op_3, lwc1_op_2, lwc1_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lwc2_op_2)
{

  RAddr raddr = pthread->getRAddr();

    fatal("lwc2: not yet implemented\n");


    return picode->next;
}


/* This version is for branch delay slots */
OP(lwc2_op_3)
{

  RAddr raddr = pthread->getRAddr();

    fatal("lwc2: not yet implemented\n");


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lwc2_op[] = {
lwc2_op_2, lwc2_op_3, lwc2_op_2, lwc2_op_3, lwc2_op_2, lwc2_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lwc3_op_2)
{

  RAddr raddr = pthread->getRAddr();

    fatal("lwc3: not yet implemented\n");


    return picode->next;
}


/* This version is for branch delay slots */
OP(lwc3_op_3)
{

  RAddr raddr = pthread->getRAddr();

    fatal("lwc3: not yet implemented\n");


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lwc3_op[] = {
lwc3_op_2, lwc3_op_3, lwc3_op_2, lwc3_op_3, lwc3_op_2, lwc3_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lwl_op_2)
{

  RAddr raddr = pthread->getRAddr();

  /* read value from memory */
#ifdef LENDIAN
  pthread->setREG(picode, RT, mips_lwlLE(pthread->getREG(picode, RT), (char *)raddr));
#else
  pthread->setREG(picode, RT, mips_lwlBE(pthread->getREG(picode, RT), (char *)raddr));
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(lwl_op_3)
{

  RAddr raddr = pthread->getRAddr();

  /* read value from memory */
#ifdef LENDIAN
  pthread->setREG(picode, RT, mips_lwlLE(pthread->getREG(picode, RT), (char *)raddr));
#else
  pthread->setREG(picode, RT, mips_lwlBE(pthread->getREG(picode, RT), (char *)raddr));
#endif


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lwl_op[] = {
lwl_op_2, lwl_op_3, lwl_op_2, lwl_op_3, lwl_op_2, lwl_op_3
};


/* Pre-computed address */
/* Normal version */
OP(lwr_op_2)
{

  RAddr raddr = pthread->getRAddr();

  /* read value from memory */
#ifdef LENDIAN
  pthread->setREG(picode, RT, mips_lwrLE(pthread->getREG(picode, RT), (char *)raddr));
#else
  pthread->setREG(picode, RT, mips_lwrBE(pthread->getREG(picode, RT), (char *)raddr));
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(lwr_op_3)
{

  RAddr raddr = pthread->getRAddr();

  /* read value from memory */
#ifdef LENDIAN
  pthread->setREG(picode, RT, mips_lwrLE(pthread->getREG(picode, RT), (char *)raddr));
#else
  pthread->setREG(picode, RT, mips_lwrBE(pthread->getREG(picode, RT), (char *)raddr));
#endif


    return pthread->getTarget();
}


/* FIXME: replicated methods. Do not index */
PFPI lwr_op[] = {
lwr_op_2, lwr_op_3, lwr_op_2, lwr_op_3, lwr_op_2, lwr_op_3
};


/* normal version */
OP(mfhi_op_0)
{

  pthread->setREG(picode, RD, pthread->hi);

    return picode->next;
}

/* branch delay slot version */
OP(mfhi_op_1)
{

  pthread->setREG(picode, RD, pthread->hi);

    return pthread->getTarget();
}

PFPI mfhi_op[] = { mfhi_op_0, mfhi_op_1 };


/* normal version */
OP(mflo_op_0)
{

  pthread->setREG(picode, RD, pthread->lo);

    return picode->next;
}

/* branch delay slot version */
OP(mflo_op_1)
{

  pthread->setREG(picode, RD, pthread->lo);

    return pthread->getTarget();
}

PFPI mflo_op[] = { mflo_op_0, mflo_op_1 };


/* normal version */
OP(mthi_op_0)
{

    pthread->hi = pthread->getREG(picode, RS);

    return picode->next;
}

/* branch delay slot version */
OP(mthi_op_1)
{

    pthread->hi = pthread->getREG(picode, RS);

    return pthread->getTarget();
}

PFPI mthi_op[] = { mthi_op_0, mthi_op_1 };


/* normal version */
OP(mtlo_op_0)
{

    pthread->lo = pthread->getREG(picode, RS);

    return picode->next;
}

/* branch delay slot version */
OP(mtlo_op_1)
{

    pthread->lo = pthread->getREG(picode, RS);

    return pthread->getTarget();
}

PFPI mtlo_op[] = { mtlo_op_0, mtlo_op_1 };


/* normal version */
OP(mult_op_0)
{

    mips_mult(pthread->getREG(picode, RS), pthread->getREG(picode, RT), &(pthread->lo), &(pthread->hi));

    return picode->next;
}

/* branch delay slot version */
OP(mult_op_1)
{

    mips_mult(pthread->getREG(picode, RS), pthread->getREG(picode, RT), &(pthread->lo), &(pthread->hi));

    return pthread->getTarget();
}

PFPI mult_op[] = { mult_op_0, mult_op_1 };


/* normal version */
OP(multu_op_0)
{

         mips_multu((unsigned int) pthread->getREG(picode, RS), (unsigned int) pthread->getREG(picode, RT),
                &(pthread->lo), &(pthread->hi));

    return picode->next;
}

/* branch delay slot version */
OP(multu_op_1)
{

         mips_multu((unsigned int) pthread->getREG(picode, RS), (unsigned int) pthread->getREG(picode, RT),
                &(pthread->lo), &(pthread->hi));

    return pthread->getTarget();
}

PFPI multu_op[] = { multu_op_0, multu_op_1 };


/* normal version */
OP(nop_op_0)
{


    return picode->next;
}

/* branch delay slot version */
OP(nop_op_1)
{


    return pthread->getTarget();
}

PFPI nop_op[] = { nop_op_0, nop_op_1 };


/* normal version */
OP(nor_op_0)
{

  pthread->setREG(picode, RD, ~(pthread->getREG(picode, RS) | pthread->getREG(picode, RT)));

    return picode->next;
}

/* branch delay slot version */
OP(nor_op_1)
{

  pthread->setREG(picode, RD, ~(pthread->getREG(picode, RS) | pthread->getREG(picode, RT)));

    return pthread->getTarget();
}

PFPI nor_op[] = { nor_op_0, nor_op_1 };


/* normal version */
OP(or_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) | pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(or_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) | pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI or_op[] = { or_op_0, or_op_1 };


/* normal version */
OP(ori_op_0)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) | (unsigned short) picode->immed);

    return picode->next;
}

/* branch delay slot version */
OP(ori_op_1)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) | (unsigned short) picode->immed);

    return pthread->getTarget();
}

PFPI ori_op[] = { ori_op_0, ori_op_1 };


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(sb_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  *(char *) raddr = pthread->getREG(picode, RT);


    return picode->next;
}


/* This version is for branch delay slots */
OP(sb_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  *(char *) raddr = pthread->getREG(picode, RT);


    return pthread->getTarget();
}


PFPI sb_op[] = {
sb_op_2, sb_op_3, sb_op_2, sb_op_3, sb_op_2, sb_op_3,
sb_op_2, sb_op_3, sb_op_2, sb_op_3, sb_op_2, sb_op_3
};


/* this needs to check if any other processor has written this address */
/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(sc_op_2)
{
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
    if (raddr & 3)
        address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
         *(unsigned int *) raddr = SWAP_WORD(pthread->getREG(picode, RT));
#else
         *(int *) raddr = pthread->getREG(picode, RT);
#endif


  pthread->setREG(picode, RT, 1);
    return picode->next;
}

/* This version is for branch delay slots */
OP(sc_op_3)
{
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
    if (raddr & 3)
        address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
         *(unsigned int *) raddr = SWAP_WORD(pthread->getREG(picode, RT));
#else
         *(int *) raddr = pthread->getREG(picode, RT);
#endif


  pthread->setREG(picode, RT, 1);
    /* Set up the next field of the next picode so that the jump occurs
     * after the event is generated.
     */
    picode->next->next = pthread->getTarget();
    return picode->next;
}

PFPI sc_op[] = {
sc_op_2, sc_op_3, sc_op_2, sc_op_3, sc_op_2, sc_op_3,
sc_op_2, sc_op_3, sc_op_2, sc_op_3, sc_op_2, sc_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(sdc1_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
    if (raddr & 7)
        address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
{
  I(sizeof(double)==sizeof(unsigned long long));
  unsigned long long v1;
  *((double *)&v1)=pthread->getDP(picode,ICODEFT);
  v1 = SWAP_LONG(v1);
  *((double *)raddr)=*((double *)&v1);
} 
#else
 *((double *)raddr)=pthread->getDP(picode, ICODEFT);
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(sdc1_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
    if (raddr & 7)
        address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
{
  I(sizeof(double)==sizeof(unsigned long long));
  unsigned long long v1;
  *((double *)&v1)=pthread->getDP(picode,ICODEFT);
  v1 = SWAP_LONG(v1);
  *((double *)raddr)=*((double *)&v1);
} 
#else
 *((double *)raddr)=pthread->getDP(picode, ICODEFT);
#endif


    return pthread->getTarget();
}


PFPI sdc1_op[] = {
sdc1_op_2, sdc1_op_3, sdc1_op_2, sdc1_op_3, sdc1_op_2, sdc1_op_3,
sdc1_op_2, sdc1_op_3, sdc1_op_2, sdc1_op_3, sdc1_op_2, sdc1_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(sdc2_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


    fatal("sdc2: not yet implemented\n");


    return picode->next;
}


/* This version is for branch delay slots */
OP(sdc2_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


    fatal("sdc2: not yet implemented\n");


    return pthread->getTarget();
}


PFPI sdc2_op[] = {
sdc2_op_2, sdc2_op_3, sdc2_op_2, sdc2_op_3, sdc2_op_2, sdc2_op_3,
sdc2_op_2, sdc2_op_3, sdc2_op_2, sdc2_op_3, sdc2_op_2, sdc2_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(sdc3_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  /* fatal("sdc3: not yet implemented\n"); DO NOTHING*/


    return picode->next;
}


/* This version is for branch delay slots */
OP(sdc3_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  /* fatal("sdc3: not yet implemented\n"); DO NOTHING*/


    return pthread->getTarget();
}


PFPI sdc3_op[] = {
sdc3_op_2, sdc3_op_3, sdc3_op_2, sdc3_op_3, sdc3_op_2, sdc3_op_3,
sdc3_op_2, sdc3_op_3, sdc3_op_2, sdc3_op_3, sdc3_op_2, sdc3_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(sh_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
    if (raddr & 1)
        address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
    *(unsigned short *) raddr = SWAP_SHORT(pthread->getREG(picode, RT));
#else
    *(short *) raddr = pthread->getREG(picode, RT);
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(sh_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
    if (raddr & 1)
        address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
    *(unsigned short *) raddr = SWAP_SHORT(pthread->getREG(picode, RT));
#else
    *(short *) raddr = pthread->getREG(picode, RT);
#endif


    return pthread->getTarget();
}


PFPI sh_op[] = {
sh_op_2, sh_op_3, sh_op_2, sh_op_3, sh_op_2, sh_op_3,
sh_op_2, sh_op_3, sh_op_2, sh_op_3, sh_op_2, sh_op_3
};


/* normal version */
OP(sll_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) << picode->args[SA]);

    return picode->next;
}

/* branch delay slot version */
OP(sll_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) << picode->args[SA]);

    return pthread->getTarget();
}

PFPI sll_op[] = { sll_op_0, sll_op_1 };


/* normal version */
OP(sllv_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) << (pthread->getREG(picode, RS) & 0x1f));

    return picode->next;
}

/* branch delay slot version */
OP(sllv_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) << (pthread->getREG(picode, RS) & 0x1f));

    return pthread->getTarget();
}

PFPI sllv_op[] = { sllv_op_0, sllv_op_1 };


/* normal version */
OP(slt_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) < pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(slt_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) < pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI slt_op[] = { slt_op_0, slt_op_1 };


/* normal version */
OP(slti_op_0)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) < picode->immed);

    return picode->next;
}

/* branch delay slot version */
OP(slti_op_1)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) < picode->immed);

    return pthread->getTarget();
}

PFPI slti_op[] = { slti_op_0, slti_op_1 };


/* normal version */
OP(sltiu_op_0)
{

  pthread->setREG(picode, RT, (unsigned) pthread->getREG(picode, RS) < (unsigned) ((int) picode->immed));

    return picode->next;
}

/* branch delay slot version */
OP(sltiu_op_1)
{

  pthread->setREG(picode, RT, (unsigned) pthread->getREG(picode, RS) < (unsigned) ((int) picode->immed));

    return pthread->getTarget();
}

PFPI sltiu_op[] = { sltiu_op_0, sltiu_op_1 };


/* normal version */
OP(sltu_op_0)
{

  pthread->setREG(picode, RD, (unsigned) pthread->getREG(picode, RS) < (unsigned) pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(sltu_op_1)
{

  pthread->setREG(picode, RD, (unsigned) pthread->getREG(picode, RS) < (unsigned) pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI sltu_op[] = { sltu_op_0, sltu_op_1 };


/* normal version */
OP(sra_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) >> picode->args[SA]);

    return picode->next;
}

/* branch delay slot version */
OP(sra_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) >> picode->args[SA]);

    return pthread->getTarget();
}

PFPI sra_op[] = { sra_op_0, sra_op_1 };


/* normal version */
OP(srav_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) >> (pthread->getREG(picode, RS) & 0x1f));

    return picode->next;
}

/* branch delay slot version */
OP(srav_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RT) >> (pthread->getREG(picode, RS) & 0x1f));

    return pthread->getTarget();
}

PFPI srav_op[] = { srav_op_0, srav_op_1 };


/* normal version */
OP(srl_op_0)
{

  pthread->setREG(picode, RD, (unsigned) pthread->getREG(picode, RT) >> picode->args[SA]);

    return picode->next;
}

/* branch delay slot version */
OP(srl_op_1)
{

  pthread->setREG(picode, RD, (unsigned) pthread->getREG(picode, RT) >> picode->args[SA]);

    return pthread->getTarget();
}

PFPI srl_op[] = { srl_op_0, srl_op_1 };


/* normal version */
OP(srlv_op_0)
{

  pthread->setREG(picode, RD, (unsigned) pthread->getREG(picode, RT) >> (pthread->getREG(picode, RS) & 0x1f));

    return picode->next;
}

/* branch delay slot version */
OP(srlv_op_1)
{

  pthread->setREG(picode, RD, (unsigned) pthread->getREG(picode, RT) >> (pthread->getREG(picode, RS) & 0x1f));

    return pthread->getTarget();
}

PFPI srlv_op[] = { srlv_op_0, srlv_op_1 };


/* normal version */
OP(sub_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) - pthread->getREG(picode, RT));

#ifdef OVERFLOW_CHK
  /* need to check for overflow here */
#endif

    return picode->next;
}

/* branch delay slot version */
OP(sub_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) - pthread->getREG(picode, RT));

#ifdef OVERFLOW_CHK
  /* need to check for overflow here */
#endif

    return pthread->getTarget();
}

PFPI sub_op[] = { sub_op_0, sub_op_1 };


/* normal version */
OP(subu_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) - pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(subu_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) - pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI subu_op[] = { subu_op_0, subu_op_1 };


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(sw_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
  *(unsigned int *) raddr = SWAP_WORD(pthread->getREG(picode, RT));
#else
  *(int *) raddr = pthread->getREG(picode, RT);
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(sw_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
  *(unsigned int *) raddr = SWAP_WORD(pthread->getREG(picode, RT));
#else
  *(int *) raddr = pthread->getREG(picode, RT);
#endif


    return pthread->getTarget();
}


PFPI sw_op[] = {
sw_op_2, sw_op_3, sw_op_2, sw_op_3, sw_op_2, sw_op_3,
sw_op_2, sw_op_3, sw_op_2, sw_op_3, sw_op_2, sw_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(swc1_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
{
  I(sizeof(float)==sizeof(unsigned int));
  unsigned int v1;
  *((float *)&v1)=pthread->getFP(picode,ICODEFT);
  v1 = SWAP_WORD(v1);
  *((float *)raddr)=*((float *)&v1);
 } 
#else
 *(float *) raddr = pthread->getFP(picode, ICODEFT);
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(swc1_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
{
  I(sizeof(float)==sizeof(unsigned int));
  unsigned int v1;
  *((float *)&v1)=pthread->getFP(picode,ICODEFT);
  v1 = SWAP_WORD(v1);
  *((float *)raddr)=*((float *)&v1);
 } 
#else
 *(float *) raddr = pthread->getFP(picode, ICODEFT);
#endif


    return pthread->getTarget();
}


PFPI swc1_op[] = {
swc1_op_2, swc1_op_3, swc1_op_2, swc1_op_3, swc1_op_2, swc1_op_3,
swc1_op_2, swc1_op_3, swc1_op_2, swc1_op_3, swc1_op_2, swc1_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(swc2_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  fatal("swc2: not yet implemented\n");


    return picode->next;
}


/* This version is for branch delay slots */
OP(swc2_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  fatal("swc2: not yet implemented\n");


    return pthread->getTarget();
}


PFPI swc2_op[] = {
swc2_op_2, swc2_op_3, swc2_op_2, swc2_op_3, swc2_op_2, swc2_op_3,
swc2_op_2, swc2_op_3, swc2_op_2, swc2_op_3, swc2_op_2, swc2_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(swc3_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  fatal("swc3: not yet implemented\n");


    return picode->next;
}


/* This version is for branch delay slots */
OP(swc3_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


  fatal("swc3: not yet implemented\n");


    return pthread->getTarget();
}


PFPI swc3_op[] = {
swc3_op_2, swc3_op_3, swc3_op_2, swc3_op_3, swc3_op_2, swc3_op_3,
swc3_op_2, swc3_op_3, swc3_op_2, swc3_op_3, swc3_op_2, swc3_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(swl_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* write value to memory */
#ifdef LENDIAN
  mips_swlLE(pthread->getREG(picode, RT), (char *)raddr);
#else
  mips_swlBE(pthread->getREG(picode, RT), (char *)raddr);
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(swl_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
  /* write value to memory */
#ifdef LENDIAN
  mips_swlLE(pthread->getREG(picode, RT), (char *)raddr);
#else
  mips_swlBE(pthread->getREG(picode, RT), (char *)raddr);
#endif


    return pthread->getTarget();
}


PFPI swl_op[] = {
swl_op_2, swl_op_3, swl_op_2, swl_op_3, swl_op_2, swl_op_3,
swl_op_2, swl_op_3, swl_op_2, swl_op_3, swl_op_2, swl_op_3
};


/* Pre-computed address; no verification; test for ll on sc only */
/* Normal version */
OP(swr_op_2)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
  mips_swrLE(pthread->getREG(picode, RT), (char *)raddr);
#else
  mips_swrBE(pthread->getREG(picode, RT), (char *)raddr);
#endif


    return picode->next;
}


/* This version is for branch delay slots */
OP(swr_op_3)
{
    /* this is not an sc instruction */
         RAddr raddr;
raddr = pthread->getRAddr();


#ifdef ADDRESS_CHK
  if (raddr & 3)
    address_exception_op(picode, pthread);
#endif
    /* write value to memory */
#ifdef LENDIAN
  mips_swrLE(pthread->getREG(picode, RT), (char *)raddr);
#else
  mips_swrBE(pthread->getREG(picode, RT), (char *)raddr);
#endif


    return pthread->getTarget();
}


PFPI swr_op[] = {
swr_op_2, swr_op_3, swr_op_2, swr_op_3, swr_op_2, swr_op_3,
swr_op_2, swr_op_3, swr_op_2, swr_op_3, swr_op_2, swr_op_3
};


/* normal version */
OP(sync_op_0)
{

    /* fatal("sync: not yet implemented\n"); */

    return picode->next;
}

/* branch delay slot version */
OP(sync_op_1)
{

    /* fatal("sync: not yet implemented\n"); */

    return pthread->getTarget();
}

PFPI sync_op[] = { sync_op_0, sync_op_1 };


/* normal version */
OP(syscall_op_0)
{

  int sysnum, addr;
  
  sysnum = pthread->getREGNUM(2);
  addr = pthread->getREGNUM(31) - 8;
  
#ifdef TASKSCALAR
  fprintf(stderr,"syscall to hell 0x%x (sysnum %d) from 0x%x\n",picode->addr, sysnum, addr);
  rsesc_exception(pthread->getPid());
#else
  if (sysnum=0x4001) /* syscall_exit */
    mint_exit(picode, pthread);
  else
    fatal("syscall %d at 0x%x, called from 0x%x, not supported yet.\n", sysnum, picode->addr, addr);
#endif

    return picode->next;
}

PFPI syscall_op[] = { syscall_op_0, NULL };


/* normal version */
OP(teq_op_0)
{

  if (pthread->getREG(picode, RS) == pthread->getREG(picode, RT)) {
#ifdef TASKSCALAR
    if(!rsesc_is_safe(pthread->getPid())) {
        rsesc_exception(pthread->getPid());
    } else
#endif
    fatal("teq: TRAP. trap handling not implemented (division by zero problem) at 0x%x\n", picode->addr);
  }

    return picode->next;
}

/* branch delay slot version */
OP(teq_op_1)
{

  if (pthread->getREG(picode, RS) == pthread->getREG(picode, RT)) {
#ifdef TASKSCALAR
    if(!rsesc_is_safe(pthread->getPid())) {
        rsesc_exception(pthread->getPid());
    } else
#endif
    fatal("teq: TRAP. trap handling not implemented (division by zero problem) at 0x%x\n", picode->addr);
  }

    return pthread->getTarget();
}

PFPI teq_op[] = { teq_op_0, teq_op_1 };


/* normal version */
OP(teqi_op_0)
{

  fatal("teqi: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(teqi_op_1)
{

  fatal("teqi: not yet implemented\n");

    return pthread->getTarget();
}

PFPI teqi_op[] = { teqi_op_0, teqi_op_1 };


/* normal version */
OP(tge_op_0)
{

  fatal("tge: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tge_op_1)
{

  fatal("tge: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tge_op[] = { tge_op_0, tge_op_1 };


/* normal version */
OP(tgei_op_0)
{

  fatal("tgei: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tgei_op_1)
{

  fatal("tgei: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tgei_op[] = { tgei_op_0, tgei_op_1 };


/* normal version */
OP(tgeiu_op_0)
{

  fatal("tgeiu: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tgeiu_op_1)
{

  fatal("tgeiu: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tgeiu_op[] = { tgeiu_op_0, tgeiu_op_1 };


/* normal version */
OP(tgeu_op_0)
{

  fatal("tgeu: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tgeu_op_1)
{

  fatal("tgeu: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tgeu_op[] = { tgeu_op_0, tgeu_op_1 };


/* normal version */
OP(tlt_op_0)
{

  fatal("tlt: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tlt_op_1)
{

  fatal("tlt: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tlt_op[] = { tlt_op_0, tlt_op_1 };


/* normal version */
OP(tlti_op_0)
{

  fatal("tlti: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tlti_op_1)
{

  fatal("tlti: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tlti_op[] = { tlti_op_0, tlti_op_1 };


/* normal version */
OP(tltiu_op_0)
{

  fatal("tltiu: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tltiu_op_1)
{

  fatal("tltiu: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tltiu_op[] = { tltiu_op_0, tltiu_op_1 };


/* normal version */
OP(tltu_op_0)
{

  fatal("tltu: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tltu_op_1)
{

  fatal("tltu: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tltu_op[] = { tltu_op_0, tltu_op_1 };


/* normal version */
OP(tne_op_0)
{

  fatal("tne: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tne_op_1)
{

  fatal("tne: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tne_op[] = { tne_op_0, tne_op_1 };


/* normal version */
OP(tnei_op_0)
{

  fatal("tnei: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(tnei_op_1)
{

  fatal("tnei: not yet implemented\n");

    return pthread->getTarget();
}

PFPI tnei_op[] = { tnei_op_0, tnei_op_1 };


/* normal version */
OP(xor_op_0)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) ^ pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(xor_op_1)
{

  pthread->setREG(picode, RD, pthread->getREG(picode, RS) ^ pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI xor_op[] = { xor_op_0, xor_op_1 };


/* normal version */
OP(xori_op_0)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) ^ (unsigned short) picode->immed);

    return picode->next;
}

/* branch delay slot version */
OP(xori_op_1)
{

  pthread->setREG(picode, RT, pthread->getREG(picode, RS) ^ (unsigned short) picode->immed);

    return pthread->getTarget();
}

PFPI xori_op[] = { xori_op_0, xori_op_1 };


/* normal version */
OP(swallow_op_0)
{
   
  int i;
  icode_ptr ud_icode;
  printf("swallow_op: \n");
  ud_icode=picode+1;
  for(i=0; i<picode->args[RD]; i++,ud_icode++) {
    printf("parameter[%d]=%d",i,ud_icode->instr);
  }

    return picode->next;
}

/* branch delay slot version */
OP(swallow_op_1)
{
   
  int i;
  icode_ptr ud_icode;
  printf("swallow_op: \n");
  ud_icode=picode+1;
  for(i=0; i<picode->args[RD]; i++,ud_icode++) {
    printf("parameter[%d]=%d",i,ud_icode->instr);
  }

    return pthread->getTarget();
}

PFPI swallow_op[] = { swallow_op_0, swallow_op_1 };


/* normal version */
OP(call_op_0)
{

  int i;
  icode_ptr ud_icode;

  ud_icode=picode+2;
  for(i=1; i<picode->args[RD]; i++,ud_icode++) {
    //FIXME: This check against 200 is hack to catch addresses
    //that are passed in instead of registers.  So, we need 
    //some way to mark each parameter with a type, either
    //register or immediate or stack. Using a counter that increments
    //for each type, we know exactly where each item should be copied.
    
    if(ud_icode->instr > 200)
      pthread->setREGNUM(i+3, ud_icode->instr);
    else
      pthread->setREGNUM(i+3, pthread->getREGNUM(ud_icode->instr));
  }
  
  //Call ops have an extra nop that is not counted amoung the 
  //arguments that insures the delay slot has nothing harmful
  //in it, like restoration of the return register from the stack.
  pthread->setREGNUM(31, picode->addr + 4*(picode->args[RD]+1));
  
  pthread->setTarget(picode->target);

    return picode->next;
}

/* branch delay slot version */
OP(call_op_1)
{

  int i;
  icode_ptr ud_icode;

  ud_icode=picode+2;
  for(i=1; i<picode->args[RD]; i++,ud_icode++) {
    //FIXME: This check against 200 is hack to catch addresses
    //that are passed in instead of registers.  So, we need 
    //some way to mark each parameter with a type, either
    //register or immediate or stack. Using a counter that increments
    //for each type, we know exactly where each item should be copied.
    
    if(ud_icode->instr > 200)
      pthread->setREGNUM(i+3, ud_icode->instr);
    else
      pthread->setREGNUM(i+3, pthread->getREGNUM(ud_icode->instr));
  }
  
  //Call ops have an extra nop that is not counted amoung the 
  //arguments that insures the delay slot has nothing harmful
  //in it, like restoration of the return register from the stack.
  pthread->setREGNUM(31, picode->addr + 4*(picode->args[RD]+1));
  
  pthread->setTarget(picode->target);

    return pthread->getTarget();
}

PFPI call_op[] = { call_op_0, call_op_1 };


void rsesc_spawn_opcode(int, const int *, int);

/* normal version */
OP(spawn_op_0)
{

  icode_ptr ud_icode;
  int params[100];
  int i;

  printf("spawn op\n");

  ud_icode=picode+2;
  
  for(i=0; i<picode->args[RD]; i++,ud_icode++) {
    //FIXME: This check against 200 is hack to catch addresses
    //that are passed in instead of registers.  So, we need 
    //some way to mark each parameter with a type, either
    //register or immediate or stack. Using a counter that increments
    //for each type, we know exactly where each item should be copied.
    
    if(ud_icode->instr > 200)
      params[i] = ud_icode->instr;
    else
      params[i] = pthread->getREGNUM(ud_icode->instr);
  }
  
  rsesc_spawn_opcode(pthread->getPid(), params, i);
  
  pthread->setREGNUM(31, picode->addr + 8);
  
  pthread->setTarget(picode->target);

    return picode->next;
}

/* branch delay slot version */
OP(spawn_op_1)
{

  icode_ptr ud_icode;
  int params[100];
  int i;

  printf("spawn op\n");

  ud_icode=picode+2;
  
  for(i=0; i<picode->args[RD]; i++,ud_icode++) {
    //FIXME: This check against 200 is hack to catch addresses
    //that are passed in instead of registers.  So, we need 
    //some way to mark each parameter with a type, either
    //register or immediate or stack. Using a counter that increments
    //for each type, we know exactly where each item should be copied.
    
    if(ud_icode->instr > 200)
      params[i] = ud_icode->instr;
    else
      params[i] = pthread->getREGNUM(ud_icode->instr);
  }
  
  rsesc_spawn_opcode(pthread->getPid(), params, i);
  
  pthread->setREGNUM(31, picode->addr + 8);
  
  pthread->setTarget(picode->target);

    return pthread->getTarget();
}

PFPI spawn_op[] = { spawn_op_0, spawn_op_1 };


/* Local Variables: */
/* mode: c */
/* End: */
