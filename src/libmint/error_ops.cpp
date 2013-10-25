/*
 * Routines for printing errors caused by simulated instruction execution.
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
#include "opcodes.h"
#include "globals.h"

OP(reserved_op1)
{
  fprintf(stderr, "0x%x: reserved instruction (0x%x)\n",
	  picode->addr, picode->instr);
  pthread->dump();
  picode->dump();
  exit(1);
  return NULL;
}

OP(address_exception)
{
  fprintf(stderr, "0x%x: address exception\n", picode->addr);
  pthread->dump();
  picode->dump();
  exit(1);
  return NULL;
}

OP(cop_reserved)
{
  fprintf(stderr, "0x%x: reserved coprocessor instruction (0x%x)\n",
	  picode->addr, picode->instr);
  pthread->dump();
  picode->dump();
  exit(1);
  return NULL;
}

OP(unimplemented_op)
{
  fprintf(stderr, "0x%x: unimplemented operation (0x%x)\n",
	  picode->addr, picode->instr);
  pthread->dump();
  picode->dump();

  if (picode->opnum) {
    char *str;
    str = (char *) picode->dis_instr();
    fprintf(stderr,"Unimplemented instruction:%s\n",str);
  }
	
  exit(1);
  return NULL;
}
