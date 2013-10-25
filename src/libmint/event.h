/*
 * Definition of the event structure and bit field mnemonics.
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

#ifndef __event_h
#define __event_h

#include <stdlib.h>

/* define a type that can hold all machine types */
typedef union numeric {
    int32_t lval;
    float fval;
    double dval;
} numeric_t, *numeric_ptr;

/*
 * On an instruction reference (not a load or store) the type field
 * equals zero, and the only valid fields are pid, iaddr, time, and type.
 * In addition, the vaddr field for instructions contains the undecoded
 * instruction.
 *
 * Note that the value and time fields are pointers.
 *
 * The bits in the "type" field:
 *
 *   12 11 10  9  8  7  6  5  4  3  2  1  0
 *  ----------------------------------------
 *  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 *  ----------------------------------------
 *   Fl Uf Ut Sh Wr Rd Sp Sy Si R  L   size
 *
 *  size: 0 = byte
 *        1 = halfword
 *        2 = word
 *	  3 = double (not supported yet)
 *
 *  RL:   0 = normal
 *        1 = lwl (load word left) or swl (store word left)
 *        2 = lwr (load word right) or swr (store word right)
 *
 *  Si:   0 = unsigned
 *        1 = signed (only set for read-byte, or read-halfword)
 *
 *  Sy:   0 = normal
 *        1 = is a sync instruction (sync, ll, or sc)
 *            if WrRd = 0, then sync,
 *            if WrRd = 1, then ll,
 *            if WrRd = 2, then sc
 *
 *  Sp:	  0 = normal
 *        1 = special
 *            low-order bits encode which special event (see below)
 *
 *  WrRd: 0 = instruction
 *        1 = read
 *        2 = write
 *
 *  Sh:   0 = private
 *        1 = shared
 *
 *  Ut:   0 = normal
 *        1 = user-defined type, set by generate_event()
 *
 *  Uf:   0 = normal
 *        1 = call user-defined function, set by sched_task()
 * 
 *  Fl:   0 = integer type
 *        1 = float or double type
 */

/* define the event types */
#define E_INSTR		0x0

/* the low 2 bits encode the size */
#define E_SIZE		0x3	/* mask for size bits */
#define E_BYTE		0x0
#define E_HALF		0x1
#define E_WORD		0x2
#define E_DWORD		0x3	/* for 64-bit refs, not yet implemented */

/* lwl (load word left) or swl (store word left) operations */
#define E_LEFT		0x4

/* lwr (load word right) or swr (store word right) operations */
#define E_RIGHT		0x8

#define E_SIGNED	0x10
#define E_SYNC		0x20

/* special events for change in process state */
#define E_SPECIAL	0x40

/* read and write operations */
#define E_READ		0x80
#define E_WRITE		0x100
#define E_MEM_REF	(E_READ | E_WRITE)

/* shared references */
#define E_SHARED	0x200

/* user-defined event type */
#define E_USER		0x400

/* user-defined function call */
#define E_UFUNC		0x800

/* float type */
#define E_FLOAT		0x1000

#define E_LWL		(E_LEFT | E_READ)
#define E_LWR		(E_RIGHT | E_READ)
#define E_SWL		(E_LEFT | E_WRITE)
#define E_SWR		(E_RIGHT | E_WRITE)

/* bit mask for LWL or LWR */
#define E_RL		(E_LEFT | E_RIGHT)

#define E_READB		(E_READ | E_BYTE)
#define E_READH		(E_READ | E_HALF)
#define E_READW		(E_READ | E_WORD)
#define E_READD		(E_READ | E_DWORD)
#define E_READBS	(E_SIGNED | E_READB)
#define E_READHS	(E_SIGNED | E_READH)
#define E_WRITEB	(E_WRITE | E_BYTE)
#define E_WRITEH	(E_WRITE | E_HALF)
#define E_WRITEW	(E_WRITE | E_WORD)
#define E_WRITED	(E_WRITE | E_DWORD)

#define E_FREADW	(E_FLOAT | E_READW)
#define E_FREADD	(E_FLOAT | E_READD)
#define E_FWRITEW	(E_FLOAT | E_WRITEW)
#define E_FWRITED	(E_FLOAT | E_WRITED)

#define E_SHR_READB	(E_SHARED | E_READB)
#define E_SHR_READH	(E_SHARED | E_READH)
#define E_SHR_READW	(E_SHARED | E_READW)
#define E_SHR_READD	(E_SHARED | E_READD)
#define E_SHR_READBS	(E_SHARED | E_READBS)
#define E_SHR_READHS	(E_SHARED | E_READHS)
#define E_SHR_WRITEB	(E_SHARED | E_WRITEB)
#define E_SHR_WRITEH	(E_SHARED | E_WRITEH)
#define E_SHR_WRITEW	(E_SHARED | E_WRITEW)
#define E_SHR_WRITED	(E_SHARED | E_WRITED)

#define E_SHR_FREADW	(E_SHARED | E_FREADW)
#define E_SHR_FREADD	(E_SHARED | E_FREADD)
#define E_SHR_FWRITEW	(E_SHARED | E_FWRITEW)
#define E_SHR_FWRITED	(E_SHARED | E_FWRITED)

#define E_SHR_LWL	(E_SHARED | E_LWL)
#define E_SHR_LWR	(E_SHARED | E_LWR)
#define E_SHR_SWL	(E_SHARED | E_SWL)
#define E_SHR_SWR	(E_SHARED | E_SWR)

/* room for 31 special types */
#define E_NO_SPEC		(E_SPECIAL | 1)
#define E_LIMIT_SPEC		(E_SPECIAL | 2)

#endif /* !__event_h */
