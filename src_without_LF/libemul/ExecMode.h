#if !(defined ExecMode_H)
#define ExecMode_H

#define SUPPORT_MIPS32
#define SUPPORT_MIPSEL32
//#define SUPPORT_MIPS64
//#define SUPPORT_MIPSEL64

typedef enum{
  ExecModeNone=0,
  ExecModeArchMask=0xf,
  ExecModeArchUnit=1,
  ExecModeArchMips    = 1*ExecModeArchUnit,
  ExecModeArchMips64  = 2*ExecModeArchUnit,
  ExecModeArchX86     = 3*ExecModeArchUnit,
  // We can have a little- or big-endian execution
  ExecModeEndianMask=16,
  ExecModeEndianUnit=16,
  ExecModeEndianBig   = 0*ExecModeEndianUnit,
  ExecModeEndianLittle= 1*ExecModeEndianUnit,                            
  // We can have a 32- or 64-bit execution
  ExecModeBitsMask=32,
  ExecModeBitsUnit=32,
  ExecModeBits32 = 0*ExecModeBitsUnit,
  ExecModeBits64 = 1*ExecModeBitsUnit,

  ExecModeMips32    =ExecModeArchMips  |ExecModeEndianBig   |ExecModeBits32,
  ExecModeMipsN32   =ExecModeArchMips64|ExecModeEndianBig   |ExecModeBits32,
  ExecModeMips64    =ExecModeArchMips64|ExecModeEndianBig   |ExecModeBits64,
  ExecModeMipsel32  =ExecModeArchMips  |ExecModeEndianLittle|ExecModeBits32,
  ExecModeMipselN32 =ExecModeArchMips64|ExecModeEndianLittle|ExecModeBits32,
  ExecModeMipsel64  =ExecModeArchMips64|ExecModeEndianLittle|ExecModeBits64,
  ExecModeX86_32    =ExecModeArchX86   |ExecModeEndianLittle|ExecModeBits32,
  ExecModeX86_64    =ExecModeArchX86   |ExecModeEndianLittle|ExecModeBits64,
} ExecMode;

#endif // !(defined ExecMode_H)
