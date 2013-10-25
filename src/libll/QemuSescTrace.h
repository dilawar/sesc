/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2005 University California, Santa Cruz.

   Contributed by Saangetha
                  Keertika
		  Jose Renau

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

#ifndef QEMU_SESC_TRACE
#define QEMU_SESC_TRACE
#include <stdio.h>

typedef struct {
  uint32_t pc:32;    
  uint32_t npc:32;
  uint8_t src1;
  uint8_t src2;
  uint8_t dest;
  int32_t opc;
  int32_t type;
  int32_t subtype;
} QemuSescTrace;

// defining the getter functions

void setpc(QemuSescTrace *qst, uint32_t pc);
void setopc(QemuSescTrace *qst, int32_t opc);
void setsrc1(QemuSescTrace *qst, uint8_t src1);
void setsrc2(QemuSescTrace *qst, uint8_t src2);
void setdest(QemuSescTrace *qst, uint8_t dest);

uint32_t getpc(QemuSescTrace *qst);
int32_t getopc(QemuSescTrace *qst);
uint32_t getsrc1(QemuSescTrace *qst);
uint32_t getsrc2(QemuSescTrace *qst);
uint32_t getdest(QemuSescTrace *qst);
uint32_t getNextPC(QemuSescTrace *qst,uint32_t);

//defining trace file functions

FILE *openFile(const char *logfilename );// getthe name of the file from the trace argument

void writeToFile(FILE* fd, QemuSescTrace *x);
QemuSescTrace* readFromFile();
void closeFile(FILE *fd);

#endif
