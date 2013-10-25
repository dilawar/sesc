/*
 * Definitions of exported global variables.
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

#ifndef __export_h
#define __export_h

#include "event.h"

#include <stdio.h>
#include <stdlib.h>

/* The maximum number of processes that can be simulated. This is used
 * to allocate space. This value can be changed with the "-p" command
 * line option up to a maximum of MAXPROC.
 */
extern int32_t Max_nprocs;

/* The absolute maximum number of processes that can be simulated. */
#define MAXPROC 256

/* the maximum process id ever used (process ids start at zero) */
extern int32_t Maxpid;

/* The file descriptor used by MINT for its output. Can be assigned by
 * the user program. Defaults to stderr. If NULL, no output from MINT
 * is generated.
 */
extern FILE *Mint_output;

#endif /* !__export_h */
