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


#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "estl.h"

#include "nanassert.h"

#include "icode.h"
#include "ThreadContext.h"

#include "Events.h"
#include "Snippets.h"
#include "callback.h"
#include "QemuSescReader.h"
#include "QemuSescTrace.h"
#ifdef SESC_SIMICS
#include "SimicsTraceFormat.h"
#endif
#ifdef QEMU_DRIVEN
#include "qemu_sesc.h"
#endif

// 0 int32_t, 1 FP, 2 none
#define INSTRUCTION_MAX_DESTPOOL 3

#include "InstType.h"

typedef uint32_t InstID;

enum RegType {
  NoDependence = 0, //fix this, 0 is used by PPC
    ReturnReg = 31,
    InternalReg,
    IntFPBoundary = 32+1,
    HiReg = 65+1,  //Y register
    CoprocStatReg,
    CondReg, 
    InvalidOutput,
    NumArchRegs // = BaseInstTypeReg +MaxInstType old times
};

typedef uint8_t MemDataSize;

//! Static Instruction type
/*! For each assembly instruction in the binary there is an
 *  Instruction associated. It is an expanded version of the MIPS
 *  instruction.
 *
 */
class Instruction {
public:
  static icode_ptr LowerLimit;
  static icode_ptr UpperLimit;

private:
  static size_t InstTableSize;
  static int32_t    maxFuncID;
  static Instruction *InstTable;
  
  typedef HASH_MAP<int32_t, Instruction*> InstHash;
  static InstHash instHash;

  static Instruction *simicsInstTable;

  static bool inLimits(icode_ptr next) {
    return !(next < LowerLimit || next > UpperLimit);
  }

  static void initializeMINT(int32_t argc,
                             char **argv,
                             char **envp);
  
  static void initializePPCTrace(int32_t argc,
                                 char **argv,
                                 char **envp);
#ifdef SESC_SIMICS
  static void initializeSimicsTrace(int32_t argc,
                                    char **argv,
                                    char **envp);
#endif

  static void MIPSDecodeInstruction(size_t index
                                    ,icode_ptr &picode
                                    ,InstType &opcode
                                    ,InstSubType &subCode
                                    ,MemDataSize &dataSize
                                    ,RegType &src1
                                    ,RegType &dest
                                    ,RegType &src2
                                    ,EventType &uEvent
                                    ,bool &BJCondLikely
                                    ,bool &guessTaken
                                    ,bool &jumpLabel);

  static void PPCDecodeInstruction(Instruction *inst, uint32_t rawInst);
#ifdef QEMU_DRIVEN
  static void QemuSparcDecodeInstruction(Instruction *inst, QemuSescTrace *qst); 
#endif

#ifdef SESC_SIMICS
  static const Instruction *SimicsDecodeInstruction(TraceSimicsOpc_t op);
#endif

#if (defined MIPS_EMUL)
 public:
#else
 protected:
#endif
  InstType opcode;
  RegType src1;
  RegType src2;
  RegType dest;
  // MINT notifies the call and the return of the instrumented
  // events. To avoid the overhead of the call and the return, I
  // mark the first instruction in the event function with the event
  // type (uEvent). Note that this is different from the iEvent
  // instruction type.

  EventType uEvent;
  InstSubType subCode;
  MemDataSize dataSize;

  char src1Pool;  // src1 register is in the FP pool?
  char src2Pool;  // src2 register is in the FP pool?
  char dstPool;   // Destination register is in the FP pool?
  char skipDelay; // 1 when the instruction has delay slot (iBJ ^ !BJCondLikely only)

  bool guessTaken;
  bool condLikely;
  bool jumpLabel; // If iBJ jumps to offset (not register)

  uint32_t addr;
  
#ifdef AIX
  const char *funcName;
#endif

public:
#ifdef QEMU_DRIVEN
  static void QemuSparcDecodeInstruction(Instruction *inst, Sparc_Inst_Predecode *predec);
#endif

  static void initialize(int32_t argc, char **argv, char **envp);  
  
  static void finalize();

  static const Instruction *getInst(InstID id) {
    // I(id <InstTableSize);
    return &InstTable[id];
  }

  static const Instruction *getSharedInstByPC(int32_t addr) {
    InstHash::iterator it = instHash.find(addr);

    if(it != instHash.end())
      return it->second;

    return 0;
  }

  // this is what should be called by TraceFlow in TT6PPC mode
  static const Instruction *getPPCInstByPC(int32_t addr, uint32_t rawInst) {

    InstHash::iterator it = instHash.find(addr);
    I(it == instHash.end());

    // horrible!! we need to fix this. --luis
    Instruction *inst = new Instruction(); // maybe we should have a pool for 
    // Instruction objects
    PPCDecodeInstruction(inst, rawInst);  //calling PPCDecodeInstruction....go to PPCInsruction.cpp 
    inst->addr = addr;  //
    //
    instHash[addr] = inst;
    return inst;   // to TraceFlow.cpp
  }
  
  static const Instruction *getQemuInstByPC(uint32_t addr, QemuSescTrace *qst) {            
    InstHash::iterator it = instHash.find(addr); 
    I(it == instHash.end());

    Instruction *inst = new Instruction();
      
#ifdef QEMU_DRIVEN
    QemuSparcDecodeInstruction(inst, qst);
#endif
    inst->addr = addr;  
    instHash[addr] = inst; 
    return inst;
  }

#ifdef SESC_RSTTRACE
  static const Instruction *getRSTInstByPC(uint32_t addr, uint32_t rawInst);
#endif

#ifdef SESC_SIMICS
  // this is what should be called by TraceFlow in simics mode
  static const Instruction *getSimicsInst(TraceSimicsOpc_t op) {
    return SimicsDecodeInstruction(op);
  }
#endif

  static const Instruction *getInst4Addr(int32_t addr) {
    icode_ptr picode = addr2icode(addr);
    return getInst(picode->instID);
  }

  bool hasDestRegister() const {
    GI(dstPool==2, dest == InvalidOutput);
    return dest != InvalidOutput;
  }
  bool hasSrc1Register() const {
    GI(src1Pool==2, src1 == NoDependence);
    return src1 != NoDependence;
  }
  bool hasSrc2Register() const {
    GI(src2Pool==2, src2 == NoDependence);
    return src2 != NoDependence;
  }
  static uint8_t whichPool(RegType r) {
    uint8_t p;
    if( r < IntFPBoundary && r != NoDependence ) {
      p = 0;
    }else if(r == CondReg) {
      p = 0;  // assuming condition register uses the Int pool
    }else if( r >= IntFPBoundary && r <= CoprocStatReg ) {
      p  = 1;
    }else{
      I(r == InvalidOutput || r == NoDependence);
      p  = 2; // void null
    }
    I(p<INSTRUCTION_MAX_DESTPOOL);

    return p;
  }
  
  InstID currentID() const {  
#if ((defined TRACE_DRIVEN)||(defined MIPS_EMUL)||(defined QEMU_DRIVEN))
    return addr;
#else
    return (this - InstTable);  
#endif
  }

  int32_t getAddr() const {
#if ((defined TRACE_DRIVEN)||(defined MIPS_EMUL)||(defined QEMU_DRIVEN))
    return addr;
#else
    icode_ptr picode = Itext[currentID()];
    I(picode->instID == currentID());
    return picode->addr;
#endif
  }

  icode_ptr getICode() const { 
#if ((defined TRACE_DRIVEN)||(defined MIPS_EMUL)||(defined QEMU_DRIVEN))
    return 0;
#else
    icode_ptr picode = Itext[currentID()];
    I(picode->instID == currentID());
    return picode;  
#endif
  }
  
  bool guessAsTaken() const { return guessTaken; }
  bool isBJLikely() const   { return condLikely; }
  bool isBJCond() const     { return opcode == iBJ && subCode == BJCond; }
  bool doesJump2Label() const {
    I(opcode==iBJ);
    return jumpLabel;
  }

  int32_t getSrc1Pool() const { return src1Pool; } //  0 Int , 1 FP, 2 none
  int32_t getSrc2Pool() const { return src2Pool; } //  0 Int , 1 FP, 2 none
  int32_t getDstPool() const  { return dstPool;  } //  0 Int , 1 FP, 2 none

#if !(defined MIPS_EMUL)
  InstID getTarget() const {
    I(opcode == iBJ);
    icode_ptr picode = Itext[currentID()];
    if( picode->target )
      return picode->target->instID;
    return calcNextInstID();
  }
#endif

  InstID calcNextInstID() const { 
    return currentID()+skipDelay;  
  }

  RegType getSrc2() const { return src2;  }
  RegType getDest() const { return dest;  }
  RegType getSrc1() const { return src1;  }

  bool isBranchTaken() const { return subCode == BJUncond; } // subCode == iBJUncond -> opcode == iBJ
  bool isBranch() const      { return opcode == iBJ; }
  bool isFuncCall() const    { return subCode == BJCall; }   // subCode == iBJCall -> opcode == iBJ
  bool isFuncRet() const     { return subCode == BJRet;  }   // subCode == iBJRet -> opcode == iBJ

  bool isMemory() const {
    // Important: it is not possible to use (picode->opflags & E_MEM_REF)
    // because some memory instructions at the beginning of the libcall are
    // modeled as returns, not as memory
    return (subCode == iMemory || opcode == iFence
            || subCode == iFetchOp || subCode == iAtomicMemory);
  }

  bool isFence() const { return opcode == iFence; }
  bool isLoad() const  { return opcode == iLoad;  }

  // In the stores the src1 is the address where the data is going to
  // be stored.  src2 is the data to be stored
  bool isStore() const { return opcode == iStore; }
  bool isStoreAddr() const { return subCode == iFake; } // opcode == iALU 

  MemDataSize getDataSize() const { return dataSize; }

  bool isType(InstType t) const { return t == opcode;  }

  bool isEvent() const { return uEvent != NoEvent; }
  EventType getEvent() const { return uEvent;  }
  InstType getOpcode() const { return opcode;  }
  InstSubType getSubCode() const { return subCode;  }

  bool isNOP() const { return subCode == iNop;  }

  // Get the name of a given opcode
  static const char *opcode2Name(InstType op);
  
  static InstID getEventID(EventType ev) { return (InstTableSize - MaxEvent) + ev;  }

  static InstID getTableSize() { return InstTableSize;  }
  static int32_t getMaxFuncID()    { return maxFuncID;  }

  void dump(const char *str) const;

  // This class is useful for STL hash_maps. It is more efficient that
  // the default % size
  class HashAddress{
  public: 
         size_t operator()(const int32_t &addr) const {
      return ((addr>>2) ^ (addr>>18));
    }
  };
};

#endif   // INSTRUCTION_H

