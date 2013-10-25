/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau
                  Luis Ceze   

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

#ifndef INSTTYPE_H
#define INSTTYPE_H

//! InstType Valid types of instructions inside sesc
/*! All the native instructions are converted to one of these
 *  types. The functional unit is choosed based on the
 *  InstructionType.
 */
enum InstType {
  iOpInvalid = 0,     //!< Never used.
  iALU,               //!< Simple int32_t ALU operations (+,-,>>)
  iMult,              //!< Integer Multiplication
  iDiv,               //!< Integer Division
  iBJ,                //!< branches, calls, and jumps
  iLoad,              //!< Load
  iStore,             //!< Store
  fpALU,              //!< coprocessor 1 floating point adds, converts, etc
  fpMult,             //!< floating point multiplies
  fpDiv,              //!< floating point division and sqrt
  iFence,             //!< Fetch&Op, iMemFence, iAcquire, iRelease (Not iLoad or iStore)
  iEvent,             //!< Fake user event instruction
  MaxInstType
};

enum InstSubType {
  iSubInvalid = 0,
  iNop,          // Nop
  iMemory,       // Load & Store
  iAtomicMemory, // Atomic Read or Write
  iFetchOp,      // Fetch&Op IMem opcode
  iMemFence,     // Release Consistency iMem opcode
  iAcquire,      // Release Consistency iMem opcode
  iRelease,      // Release Consistency iMem opcode
  BJUncond,      // iBJ opcode
  BJCall,        // iBJ opcode
  BJRet,         // iBJ opcode
  BJCond,        // iBJ opcode
  iFake,       // Internally generated instruction (store-address...)
  InstSubTypeMax
};

#endif
