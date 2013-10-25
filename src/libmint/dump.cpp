/*
 * Routines for printing the contents of various structures. Useful for
 * debugging. Also includes a custom disassembler for MINT picodes.
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
#include <string.h>

#include "icode.h"
#include "ThreadContext.h"
#include "globals.h"
#include "opcodes.h"
#include "coff.h"

#include "mendian.h"

void dump_filehdr(struct filehdr *pfhdr)
{
  printf("file header:\n");
  printf("  f_magic: 0x%x\n", pfhdr->f_magic);
  printf("  f_nscns: %d\n", (int) pfhdr->f_nscns);
  printf("  f_timdat: 0x%x\n", pfhdr->f_timdat);
  printf("  f_symptr: 0x%x\n", pfhdr->f_symptr);
  printf("  f_nsyms: %d\n", pfhdr->f_nsyms);
  printf("  f_opthdr: %d\n", (int) pfhdr->f_opthdr);
  printf("  f_flags: 0x%x\n", pfhdr->f_flags);
}

void dump_aouthdr(struct aouthdr *pahdr)
{
  printf("aout header:\n");
  printf("  magic: 0%o (0x%x)\n", pahdr->magic, pahdr->magic);
  printf("  vstamp: %d (0x%x)\n", pahdr->vstamp, pahdr->vstamp);
  printf("  tsize: %d (0x%x)\n",
	 pahdr->tsize, pahdr->tsize);
  printf("  dsize: %d (0x%x)\n",
	 pahdr->dsize, pahdr->dsize);
  printf("  bsize: %d (0x%x)\n",
	 pahdr->bsize, pahdr->bsize);
  printf("  entry: 0x%x\n", pahdr->entry);
  printf("  text_start: 0x%x\n", pahdr->text_start);
  printf("  data_start: 0x%x\n", pahdr->data_start);
  printf("  bss_start: 0x%x\n", pahdr->bss_start);
  printf("  gprmask: 0x%x\n", pahdr->gprmask);
  printf("  cprmask[]: 0x%x 0x%x 0x%x 0x%x\n",
	 pahdr->cprmask[0], pahdr->cprmask[1],
	 pahdr->cprmask[2], pahdr->cprmask[3]);
  printf("  gp_value: 0x%x\n", pahdr->gp_value);
}

void
dump_scnhdr(struct scnhdr *pshdr)
{
    printf("section header:\n");
    printf("  s_name: \"%s\"\n", pshdr->s_name);
    printf("  s_paddr: 0x%x\n", pshdr->s_paddr);
    printf("  s_vaddr: 0x%x\n", pshdr->s_vaddr);
    printf("  s_size: %d (0x%x)\n",
           pshdr->s_size, pshdr->s_size);
    printf("  s_scnptr: 0x%x\n", pshdr->s_scnptr);
    printf("  s_relptr: 0x%x\n", pshdr->s_relptr);
    printf("  s_lnnoptr: 0x%x\n", pshdr->s_lnnoptr);
    printf("  s_nreloc: %d\n", (int) pshdr->s_nreloc);
    printf("  s_nlnno: %d\n", (int) pshdr->s_nlnno);
    printf("  s_flags: 0x%x\n", pshdr->s_flags);
}

void
dump_symhdr(HDRR *psymhdr)
{
    printf("symbolic header:\n");
    printf("  magic: 0x%x\n", psymhdr->magic);
    printf("  vstamp: 0x%x\n", psymhdr->vstamp);
    printf("  ilineMax: 0x%x\n", psymhdr->ilineMax);
    printf("  cbLine: 0x%x\n", psymhdr->cbLine);
    printf("  cbLineOffset: 0x%x\n", psymhdr->cbLineOffset);
    printf("  idnMax: 0x%x\n", psymhdr->idnMax);
    printf("  cbDnOffset: 0x%x\n", psymhdr->cbDnOffset);
    printf("  ipdMax: 0x%x\n", psymhdr->ipdMax);
    printf("  cbPdOffset: 0x%x\n", psymhdr->cbPdOffset);
    printf("  isymMax: 0x%x\n", psymhdr->isymMax);
    printf("  cbSymOffset: 0x%x\n", psymhdr->cbSymOffset);
    printf("  ioptMax: 0x%x\n", psymhdr->ioptMax);
    printf("  cbOptOffset: 0x%x\n", psymhdr->cbOptOffset);
    printf("  iauxMax: 0x%x\n", psymhdr->iauxMax);
    printf("  cbAuxOffset: 0x%x\n", psymhdr->cbAuxOffset);
    printf("  issMax: 0x%x\n", psymhdr->issMax);
    printf("  cbSsOffset: 0x%x\n", psymhdr->cbSsOffset);
    printf("  issExtMax: 0x%x\n", psymhdr->issExtMax);
    printf("  cbSsExtOffset: 0x%x\n", psymhdr->cbSsExtOffset);
    printf("  ifdMax: 0x%x\n", psymhdr->ifdMax);
    printf("  cbFdOffset: 0x%x\n", psymhdr->cbFdOffset);
    printf("  crfd: 0x%x\n", psymhdr->crfd);
    printf("  cbRfdOffset: 0x%x\n", psymhdr->cbRfdOffset);
    printf("  iextMax: 0x%x\n", psymhdr->iextMax);
    printf("  cbExtOffset: 0x%x\n", psymhdr->cbExtOffset);
}



