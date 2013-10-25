#ifndef ICODE_H
#define ICODE_H
/*
 * Definitions of icodes, threads, queues, and locks. Macros for
 * address mapping, and register dereferencing.
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

#include "event.h"
#include "export.h"

#define _BSD_SIGNALS
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>

#include "mendian.h"

#if defined(DARWIN) || (defined(sparc) && !defined(__svr4__))
typedef uint32_t uint;
#endif

typedef struct icode *icode_ptr;

typedef class ThreadContext *thread_ptr;

typedef icode_ptr (*PFPI)(icode_ptr, thread_ptr);
typedef icode_ptr (*PPFPI[])(icode_ptr, thread_ptr);
typedef icode_ptr (*PPFPI6[6])(icode_ptr, thread_ptr);
typedef icode_ptr (*PPFPI12[12])(icode_ptr, thread_ptr);

#include "globals.h"
#include "event.h"
#include "export.h"
#include "common.h"
#include "non_mips.h"
#include "Snippets.h"
#include "nanassert.h"

/* mnemonics for the register indices into the args[] array */
#define RS 0
#define RT 1
#define RD 2
#define SA 3

/* mnemonics for the coprocessor register indices into the args[] array */
#define FMT 0
#define ICODEFT 1
#define ICODEFS 2
#define ICODEFD 3

class icode {
 private:
#if (defined TLS)
  OpClass opClass;
#endif
public:
  uint32_t instID;
  PFPI func;			/* function that simulates this instruction */
  int32_t addr;			/* text address of this instruction */
  short args[4];		/* the non-shifted register args */
  short immed;		/* bits 0 - 15 */
  icode_ptr next;		/* pointer to next icode to execute */
  icode_ptr target;		/* for branches and jumps */
  icode_ptr not_taken;	/* target when branch not taken */
  uint32_t is_target : 1;	/* =1 if this instruction is a branch target */
  uint32_t opnum : 15;	/* arbitrary index for an instruction */
  uint32_t opflags : 16;	/* tells what kind of instruction */
  int32_t instr;			/* undecoded instruction */

  int32_t getFPN(int32_t R) const { return args[R]; }
  int32_t getRN(int32_t R)  const { return args[R]; }
  int32_t getDPN(int32_t R) const { return args[R]; }
  const char *getFPFMT(int32_t F) {
    return ((F) ==  16 ? "s" : (F) == 17 ? "d" : (F) == 20 ? "w" : "?");
  }

  void dump();
  const char *dis_instr();
#if (defined TLS)
  void setClass(OpClass newClass){
    opClass=newClass;
  }
  OpClass getClass(void) const{
    return opClass;
  }
#endif
};

typedef struct icode  icode_t;

extern icode_ptr icodeArray;
extern size_t    icodeArraySize;

//extern signed int32_t Text_start;
//extern signed int32_t Text_end;
extern struct icode **Itext;

// Takes the logical address of an instruction, returns pointer to its icode
inline icode_ptr addr2icode(Address addr) {
  I(addr>=Text_start);
  icode_ptr picode=icodeArray+(addr-Text_start)/sizeof(icodeArray->instr);
  I((size_t)(picode-icodeArray)<icodeArraySize);
  return picode;
}

// Takes a pointer to an icode, returns the logical instruction address
inline Address icode2addr(icode_ptr picode) {
  I((size_t)(picode-icodeArray)<icodeArraySize);
  Address addr=Text_start+sizeof(icodeArray->instr)*(picode-icodeArray);
  I(addr>=Text_start);
  return addr;
}

// An icode that should not be executed. Unused, free, and inactive
// thread contexts can point to this icode to catch buggy execution
extern icode_t invalidIcode;

#endif /* ICODE_H */

