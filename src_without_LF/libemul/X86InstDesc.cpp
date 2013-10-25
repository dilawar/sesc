/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Milos Prvulovic

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

#include "X86InstDesc.h"
#include "ThreadContext.h"

namespace X86{
  typedef enum{ SegDf, SegES, SegCS, SegSS, SegDS, SegFS, SegGS } SegReg;   

  #define I1(Op,Func,Src1,Src2,Dst)

  I1(0x0000,funcADD  ,ArgEb  ,ArgGb  ,ArgEb  )
  I1(0x0001,funcADD  ,ArgEv  ,ArgGv  ,ArgEv  )
  I1(0x0002,funcADD  ,ArgGb  ,ArgEb  ,ArgGb  )
  I1(0x0003,funcADD  ,ArgGv  ,ArgEv  ,ArgGv  )
  I1(0x0004,funcADD  ,ArgAL  ,ArgIb  ,ArgAL  )
  I1(0x0005,funcADD  ,ArgrAX ,ArgIz  ,ArgrAX )
  I1(0x0006,funcPUSH,ArgES  ,ArgNone,ArgNone)
  I1(0x0007,funcPOP  ,ArgNone,ArgNone,ArgES  )
  I1(0x0010,funcADDC ,ArgEb  ,ArgGb  ,ArgEb  )
  I1(0x0011,funcADDC ,ArgEv  ,ArgGv  ,ArgEv  )
  I1(0x0012,funcADDC ,ArgGb  ,ArgEb  ,ArgGb  )
  I1(0x0013,funcADDC ,ArgGv  ,ArgEv  ,ArgGv  )
  I1(0x0014,funcADDC ,ArgAL  ,ArgIb  ,ArgAL  )
  I1(0x0015,funcADDC ,ArgrAX ,ArgIz  ,ArgrAX )
  I1(0x0016,funcPUSH ,ArgSS  ,ArgNone,ArgNone)
  I1(0x0017,funcPOP  ,ArgNone,ArgNone,ArgSS  )
  I1(0x0020,funcAND  ,ArgEb  ,ArgGb  ,ArgEb  )
  I1(0x0021,funcAND  ,ArgEv  ,ArgGv  ,ArgEv  )
  I1(0x0022,funcAND  ,ArgGb  ,ArgEb  ,ArgGb  )
  I1(0x0023,funcAND  ,ArgGv  ,ArgEv  ,ArgGv  )
  I1(0x0024,funcAND  ,ArgAL  ,ArgIb  ,ArgAL  )
  I1(0x0025,funcAND  ,ArgrAX ,ArgIz  ,ArgrAX )
  // 0x0026 is a prefix (SEG=ES)
  I1(0x0027,funcDAA  ,ArgNone,ArgNone,ArgNone)
  I1(0x0030,funcXOR  ,ArgEb  ,ArgGb  ,ArgEb  )
  I1(0x0031,funcXOR  ,ArgEv  ,ArgGv  ,ArgEv  )
  I1(0x0032,funcXOR  ,ArgGb  ,ArgEb  ,ArgGb  )
  I1(0x0033,funcXOR  ,ArgGv  ,ArgEv  ,ArgGv  )
  I1(0x0034,funcXOR  ,ArgAL  ,ArgIb  ,ArgAL  )
  I1(0x0035,funcXOR  ,ArgrAX ,ArgIz  ,ArgrAX )
  // 0x0036 is a prefix (SEG=SS)
  I1(0x0037,funcAAA  ,ArgNone,ArgNone,ArgNone)
  I1(0x0040,funcINC  ,ArgeAX ,ArgNone,ArgeAX )
  I1(0x0041,funcINC  ,ArgeCX ,ArgNone,ArgeCX )
  I1(0x0042,funcINC  ,ArgeDX ,ArgNone,ArgeDX )
  I1(0x0043,funcINC  ,ArgeBX ,ArgNone,ArgeBX )
  I1(0x0044,funcINC  ,ArgeSP ,ArgNone,ArgeSP )
  I1(0x0045,funcINC  ,ArgeBP ,ArgNone,ArgeBP )
  I1(0x0046,funcINC  ,ArgeSI ,ArgNone,ArgeSI )
  I1(0x0047,funcINC  ,ArgeDI ,ArgNone,ArgeDI )
  I1(0x0050,funcPUSH ,ArgrAX ,ArgNone,ArgNone)
  I1(0x0051,funcPUSH ,ArgrCX ,ArgNone,ArgNone)
  I1(0x0052,funcPUSH ,ArgrDX ,ArgNone,ArgNone)
  I1(0x0053,funcPUSH ,ArgrBX ,ArgNone,ArgNone)
  I1(0x0054,funcPUSH ,ArgrSP ,ArgNone,ArgNone)
  I1(0x0055,funcPUSH ,ArgrBP ,ArgNone,ArgNone)
  I1(0x0056,funcPUSH ,ArgrSI ,ArgNone,ArgNone)
  I1(0x0057,funcPUSH ,ArgrDI ,ArgNone,ArgNone)
  I1(0x0060,funcPUSHA,ArgNone,ArgNone,ArgNone)
  I1(0x0061,funcPOPA ,ArgNone,ArgNone,ArgNone)
  I1(0x0062,funcPUSH ,ArgrDX ,ArgNone,ArgNone)
  I1(0x0063,funcARPL ,ArgEw  ,ArgGw  ,ArgNone)
  // 0x0064 is a prefix (SEG=FS)
  // 0x0065 is a prefix (SEG=GS)
  // 0x0066 is a prefix (Operand Size)
  // 0x0076 is a prefix (Address Size)
  I1(0x0070,funcJO   ,ArgNone,ArgNone,ArgNone)
  I1(0x0071,funcJNO  ,ArgNone,ArgNone,ArgNone)
  I1(0x0072,funcJC   ,ArgNone,ArgNone,ArgNone)
  I1(0x0073,funcJNC  ,ArgNone,ArgNone,ArgNone)
  I1(0x0074,funcJZ   ,ArgNone,ArgNone,ArgNone)
  I1(0x0075,funcJNZ  ,ArgNone,ArgNone,ArgNone)
  I1(0x0076,funcJNA  ,ArgNone,ArgNone,ArgNone)
  I1(0x0077,funcJA   ,ArgNone,ArgNone,ArgNone)
  I1(0x0080,funcImmG1,ArgEb  ,ArgIb  ,ArgNone)
  I1(0x0081,funcImmG1,ArgEv  ,ArgIz  ,ArgNone)
  I1(0x0082,funcImmG1,ArgEb  ,ArgIb  ,ArgNone)
  I1(0x0083,funcImmG1,ArgEv  ,ArgIb  ,ArgNone)
  I1(0x0084,funcTEST ,ArgEb  ,ArgGb  ,ArgNone)
  I1(0x0085,funcTEST ,ArgEv  ,ArgGv  ,ArgNone)
  I1(0x0086,funcXCHG ,ArgEb  ,ArgGb  ,ArgNone)
  I1(0x0087,funcXCHG ,ArgEv  ,ArgGv  ,ArgNone)
  I1(0x0090,funcNOP  ,ArgNone,ArgNone,ArgNone)
  I1(0x0091,funcXCHG ,ArgrCX ,ArgrAX ,ArgNone)
  I1(0x0092,funcXCHG ,ArgrDX ,ArgrAX ,ArgNone)
  I1(0x0093,funcXCHG ,ArgrBX ,ArgrAX ,ArgNone)
  I1(0x0094,funcXCHG ,ArgrSP ,ArgrAX ,ArgNone)
  I1(0x0095,funcXCHG ,ArgrBP ,ArgrAX ,ArgNone)
  I1(0x0096,funcXCHG ,ArgrSI ,ArgrAX ,ArgNone)
  I1(0x0097,funcXCHG ,ArgrDI ,ArgrAX ,ArgNone)
  I1(0x00A0,funcMOV  ,ArgAL  ,ArgOb  ,ArgNone)
  I1(0x00A1,funcMOV  ,ArgrAX ,ArgOv  ,ArgNone)
  I1(0x00A2,funcMOV  ,ArgOb  ,ArgAL  ,ArgNone)
  I1(0x00A3,funcMOV  ,ArgOv  ,ArgrAX ,ArgNone)
  I1(0x00A4,funcMOVS ,ArgXb  ,ArgrYb ,ArgNone)
  I1(0x00A5,funcMOVS ,ArgXv  ,ArgrYv ,ArgNone)
  I1(0x00A6,funcCMPS ,ArgXb  ,ArgrYb ,ArgNone)
  I1(0x00A7,funcCMPS ,ArgXv  ,ArgrYv ,ArgNone)
  I1(0x00B0,funcMOV  ,ArgAL  ,ArgIb  ,ArgNone)
  I1(0x00B1,funcMOV  ,ArgCL  ,ArgIb  ,ArgNone)
  I1(0x00B2,funcMOV  ,ArgDL  ,ArgIb  ,ArgNone)
  I1(0x00B3,funcMOV  ,ArgBL  ,ArgIb  ,ArgNone)
  I1(0x00B4,funcMOV  ,ArgAH  ,ArgIb  ,ArgNone)
  I1(0x00B5,funcMOV  ,ArgCH  ,ArgIb  ,ArgNone)
  I1(0x00B6,funcMOV  ,ArgDH  ,ArgIb  ,ArgNone)
  I1(0x00B7,funcMOV  ,ArgBH  ,ArgIb  ,ArgNone)
  I1(0x00C0,funcShG2 ,ArgEb  ,ArgIb  ,ArgNone)
  I1(0x00C1,funcShG2 ,ArgEv  ,ArgIb  ,ArgNone)
  I1(0x00C2,funcRETN ,ArgIw  ,ArgNone,ArgNone)
  I1(0x00C3,funcRETN ,ArgNone,ArgNone,ArgNone)
  I1(0x00C4,funcLES  ,ArgGz  ,ArgMp  ,ArgNone)
  I1(0x00C5,funcLDS  ,ArgGz  ,ArgMp  ,ArgNone)
  I1(0x00C6,funcG11  ,ArgEb  ,ArgIb  ,ArgNone)
  I1(0x00C7,funcG11  ,ArgEv  ,ArgIz  ,ArgNone)
  #undef I1

  template<SegReg segReg>
  void decodeInstSize2(uint32_t rawInst, ThreadContext *context, VAddr &curAddr, VAddr endAddr, size_t &tsize, bool domap){
    
  }
  template<SegReg segReg>
  void decodeInstSize1(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, size_t &tsize, bool domap){
    uint8_t opbyte=context->readMemRaw<uint8_t>(curAddr);
    curAddr+=sizeof(uint8_t);
    switch(opbyte){
    case 0x26: // Prefix SEG=ES
      I(segReg==SegDf); return decodeInstSize1<SegES>(context,funcAddr,curAddr,endAddr,tsize,false);
    case 0x2E: // Prefix SEG=CS
      I(segReg==SegDf); return decodeInstSize1<SegCS>(context,funcAddr,curAddr,endAddr,tsize,false);
    case 0x36: // Prefix SEG=SS
      I(segReg==SegDf); return decodeInstSize1<SegSS>(context,funcAddr,curAddr,endAddr,tsize,false);
    case 0x3E: // Prefix SEG=DS
      I(segReg==SegDf); return decodeInstSize1<SegDS>(context,funcAddr,curAddr,endAddr,tsize,false);
    case 0x64: // Prefix SEG=FS
      I(segReg==SegDf); return decodeInstSize1<SegFS>(context,funcAddr,curAddr,endAddr,tsize,false);
    case 0x65: // Prefix SEG=GS
      I(segReg==SegDf); return decodeInstSize1<SegGS>(context,funcAddr,curAddr,endAddr,tsize,false);
    case 0x66: // Prefix OpSize
    case 0x67: // Prefix AddrSize
    case 0xF0: // Prefix LOCK
    case 0xF2: // Prefix REPNE
    case 0xF3: // Prefix REP/REPE
      printf("Prefix at 0x%08x\n",curAddr-1);
      return decodeInstSize1<segReg>(context,funcAddr,curAddr,endAddr,tsize,false);
    }
    uint8_t exbyte=0;
    // If 2-byte escape, read second byte
    if(opbyte==0x0F){
      opbyte=context->readMemRaw<uint8_t>(curAddr);
      curAddr+=sizeof(uint8_t);
      exbyte++;
      // If one of 3-byte escapes, read third byte
      if((opbyte==0x38)||(opbyte==0x3A)){
        opbyte=context->readMemRaw<uint8_t>(curAddr);
        curAddr+=sizeof(uint8_t);
        exbyte++;
        if(opbyte==0x3A)
          exbyte++;
      }
    }
    return decodeInstSize2<segReg>((uint32_t(exbyte)<<24)+(uint32_t(opbyte)<<16),context,curAddr,endAddr,tsize,domap);
  }

  void decodeInstSize(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, size_t &tsize, bool domap){
    return decodeInstSize1<SegDf>(context,funcAddr,curAddr,endAddr,tsize,domap);
  }
  void decodeInst(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, InstDesc *&trace, bool domap){
  }
}
