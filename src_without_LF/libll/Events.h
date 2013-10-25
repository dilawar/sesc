/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Basilio Fraguela
                  Milos Prvulovic

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
#ifndef EVENTS_H
#define EVENTS_H

#include "icode.h"
#include "ThreadContext.h"

enum EventType {
  NoEvent = 0,
#ifdef TASKSCALAR
  SpawnEvent,
#endif
  PreEvent,
  PostEvent,
  FetchOpEvent,    // Memory rd/wr performed atomicaly. Notified as MemFence op
  MemFenceEvent,   // Release Consistency Barrier or memory fence
  AcquireEvent,    // Release Consistency Release
  ReleaseEvent,    // Release Consistency Ackquire
  UnlockEvent,     // Like above, but implemented
  FastSimBeginEvent,
  FastSimEndEvent,
  LibCallEvent,
  FakeInst,
  FakeInstMax = FakeInst+32,
  MaxEvent
};
// I(MaxEvent < SESC_MAXEVENT);

/* WARNING: If you add more events (MaxEvent) and MIPSInstruction.cpp
 * adds them, be sure that the Itext size also grows. (mint:globals.h)
 */

/* System events supported by sesc_postevent(....)
 */
static const int32_t EvSimStop  = 1;
static const int32_t EvSimStart = 2;

/*
 * The first 10 types are reserved for communicate with the
 * system. This means that you CAN NOT use sesc_preevent(...,3,...);
 */
static const int32_t EvMinType  = 10;

void mint_termination_err(int32_t pid, int err);
void mint_termination(int32_t pid);

#include "mintapi.h"

#endif   // EVENTS_H
