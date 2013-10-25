#if !(defined ArchDefs_H)
#define ArchDefs_H

#include "ExecMode.h"

template<ExecMode mode>
class ArchRegSizes : public ArchRegSizes<ExecMode(mode&ExecModeArchMask)>{
};
#if (defined SUPPORT_MIPS32) || (defined SUPPORT_MIPSEL32)
template<>
class ArchRegSizes<ExecModeArchMips>{
 public:
  typedef uint32_t RawInst;
  typedef uint32_t Taddr_t;
  typedef int32_t  Tsaddr_t;
  typedef uint32_t Tregv_t;
  typedef uint32_t Turegv_t;
  typedef int32_t  Tsregv_t;
};
#endif
#if (defined SUPPORT_MIPS64) || (defined SUPPORT_MIPSEL64)
template<>
class ArchRegSizes<ExecModeArchMips64>{
 public:
  typedef uint32_t RawInst;
  typedef uint64_t Taddr_t;
  typedef int64_t  Tsaddr_t;
  typedef uint64_t Tregv_t;
  typedef uint64_t Turegv_t;
  typedef int64_t  Tsregv_t;
};
#endif
template<ExecMode mode>
class ArchRegNames : public ArchRegNames<ExecMode(mode&ExecModeArchMask)>{
};

class ArchMipsRegNames{
 public:
  enum{
    // General-purpose registers
    GprNameLb = RegTypeGpr,
    RegZero   = GprNameLb,
    RegAT,
    RegV0, // 2
    RegV1,
    RegA0, // 4
    RegA1,
    RegA2,
    RegA3,
    RegA4,
    RegA5,
    RegA6,
    RegA7,
    RegT4, // 12
    RegT5,
    RegT6,
    RegT7,
    RegS0, // 16
    RegS1,
    RegS2,
    RegS3,
    RegS4,
    RegS5,
    RegS6,
    RegS7,
    RegT8,
    RegT9, // 25
    RegKT0,
    RegKT1,
    RegGP,
    RegSP, // 29
    RegS8, // 30
    RegFP     = RegS8,
    RegRA, // 31
    RegJunk, // Junk register, stores to RegZero go here
    RegTmp,  // Temporary GPR for implementing multi-uop non-branches
    RegBTmp, // Temporary GPR for implementing multi-uop branches
    GprNameUb,
    // Floating-point registers
    FprNameLb = RegTypeFpr,
    RegF0     = FprNameLb,
    RegF31    = RegF0+31,
    RegFTmp, // Temporary FPR for implementing micro-ops
    FprNameUb,
    // Control registers
    FcrNameLb = RegTypeCtl,
    RegFC0    = FcrNameLb,
    RegFC31   = FcrNameLb+31,
    RegFCSR   = RegFC31, // FPU (COP1) control register 31 is the "FPU Control and Status Register" (FCSR)
    FcrNameUb,
    FccNameLb = FcrNameUb,
    FccNameUb = FccNameLb+8,
    FpRMode,            // RM bits fo the FCSR (rounding mode for floating point instructions)
    // Special registers
    SpcNameLb = RegTypeSpc,
    RegHi     = SpcNameLb,
    RegLo,
    RegCond, // Condition register for implementing micro-ops
    RegLink, // Link register for implementing LL/SC
    RegSys,  // Register used in system implementation
    SpcNameUb,
    HwrNameLb = SpcNameUb,
    RegTPtr   = HwrNameLb+29,
    HwrNameUb = HwrNameLb+32,
  };
};

#if (defined SUPPORT_MIPS32) || (defined SUPPORT_MIPSEL32)
template<>
class ArchRegNames<ExecModeArchMips> : public ArchMipsRegNames{
};
#endif
#if (defined SUPPORT_MIPS64) || (defined SUPPORT_MIPSEL64)
template<>
class ArchRegNames<ExecModeArchMips64> : public ArchMipsRegNames{
};
#endif
template<ExecMode mode>
class ArchDefs{
};

template<ExecMode mode, typename T, RegName RTyp>
class MipsRegAccess{
 public:
  static inline T getReg(const ThreadContext *context, RegName name){
    if(((mode&ExecModeArchMask)==ExecModeArchMips)&&isFprName(RTyp))
      return MipsRegAccess<mode,T,RegTypeFpr>::getReg(context,name);
    if(RTyp==RegName(ArchMipsRegNames::RegFCSR))
      return MipsRegAccess<mode,T,RegName(ArchMipsRegNames::RegFCSR)>::getReg(context,name);
    return MipsRegAccess<mode,T,RegTypeGpr>::getReg(context,name);
  }
  static inline void setReg(ThreadContext *context, RegName name, T val){
    if(((mode&ExecModeArchMask)==ExecModeArchMips)&&isFprName(RTyp))
      return MipsRegAccess<mode,T,RegTypeFpr>::setReg(context,name,val);
    if(RTyp==RegName(ArchMipsRegNames::RegFCSR))
      return MipsRegAccess<mode,T,RegName(ArchMipsRegNames::RegFCSR)>::setReg(context,name,val);
    return MipsRegAccess<mode,T,RegTypeGpr>::setReg(context,name,val);
  }
};

template<ExecMode mode,typename T>
class MipsRegAccess<mode,T,RegDyn>{
 public:
  static inline T getReg(const ThreadContext *context, RegName name){
    if(((mode&ExecModeArchMask)==ExecModeArchMips)&&isFprName(name))
      return MipsRegAccess<mode,T,RegTypeFpr>::getReg(context,name);
    if(name==RegName(ArchMipsRegNames::RegFCSR))
      return MipsRegAccess<mode,T,RegName(ArchMipsRegNames::RegFCSR)>::getReg(context,name);
    return MipsRegAccess<mode,T,RegTypeGpr>::getReg(context,name);
  }
  static inline void setReg(ThreadContext *context, RegName name, T val){
    if(((mode&ExecModeArchMask)==ExecModeArchMips)&&isFprName(name))
      return MipsRegAccess<mode,T,RegTypeFpr>::setReg(context,name,val);
    if(name==RegName(ArchMipsRegNames::RegFCSR))
      return MipsRegAccess<mode,T,RegName(ArchMipsRegNames::RegFCSR)>::setReg(context,name,val);
    return MipsRegAccess<mode,T,RegTypeGpr>::setReg(context,name,val);
  }
};

template<ExecMode mode,typename T>
class MipsRegAccess<mode,T,RegTypeGpr>{
 public:
  static inline T getReg(const ThreadContext *context, RegName name){
    const T *ptr=static_cast<const T *>(context->getReg(name));
    return *(ptr+(sizeof(RegVal)/sizeof(T)-1)*(__BYTE_ORDER==__BIG_ENDIAN));
  }
  static inline void setReg(ThreadContext *context, RegName name, T val){
    T *ptr=static_cast<T *>(context->getReg(name));
    *(ptr+(sizeof(RegVal)/sizeof(T)-1)*(__BYTE_ORDER==__BIG_ENDIAN))=val;
  }
};

#if (defined SUPPORT_MIPS32)
template<typename T>
class MipsRegAccess<ExecModeMips32,T,RegTypeFpr>{
 public:
  static inline T getReg(const ThreadContext *context, RegName name){
    switch(sizeof(T)){
    case sizeof(uint64_t): {
      const T *ptr=static_cast<const T *>(context->getReg(name));
      return *ptr;
    } break;
    case sizeof(uint32_t): {
      bool isOdd=static_cast<bool>(name&1);
      const T *ptr=static_cast<const T *>(context->getReg(static_cast<RegName>(name^isOdd)));
      return *(ptr+(isOdd^(__BYTE_ORDER==__BIG_ENDIAN)));
    } break;
    default:
      fail("MipsRegAccess::getReg for RegTypeFpr with unsupported size %d\n",sizeof(T));
    }
  }
  static inline void setReg(ThreadContext *context, RegName name, T val){
    switch(sizeof(T)){
    case sizeof(uint64_t): {
      T *ptr=static_cast<T *>(context->getReg(name));
      *ptr=val;
    } break;
    case sizeof(uint32_t): {
      bool isOdd=static_cast<bool>(name&1);
      T *ptr=static_cast<T *>(context->getReg(static_cast<RegName>(name^isOdd)));
      *(ptr+(isOdd^(__BYTE_ORDER==__BIG_ENDIAN)))=val;
    } break;
    }
  }
};
#endif
#if (defined SUPPORT_MIPSEL32)
template<typename T>
class MipsRegAccess<ExecModeMipsel32,T,RegTypeFpr>{
 public:
  static inline T getReg(const ThreadContext *context, RegName name){
    return MipsRegAccess<ExecModeMips32,T,RegTypeFpr>::getReg(context,name);
  }
  static inline void setReg(ThreadContext *context, RegName name, T val){
    return MipsRegAccess<ExecModeMips32,T,RegTypeFpr>::setReg(context,name,val);
  }
};
#endif
template<ExecMode mode,typename T>
class MipsRegAccess<mode,T,RegName(ArchMipsRegNames::RegFCSR)>{
 public:
  static inline T getReg(const ThreadContext *context, RegName name){
    I(sizeof(T)==sizeof(uint32_t));
    uint32_t retVal=MipsRegAccess<mode,uint32_t,RegTypeGpr>::getReg(context,RegName(ArchMipsRegNames::RegFCSR));
    I(retVal==(retVal&0x17FFFFFC));
    for(int32_t fcc=ArchMipsRegNames::FccNameLb;fcc!=ArchMipsRegNames::FccNameUb;fcc++)
      if(MipsRegAccess<mode,bool,RegTypeGpr>::getReg(context,RegName(fcc)))
	retVal|=(uint32_t(1)<<(23+(fcc-ArchMipsRegNames::FccNameLb)+(fcc!=ArchMipsRegNames::FccNameLb)));
    return T(retVal|uint32_t(MipsRegAccess<mode,uint8_t,RegTypeGpr>::getReg(context,RegName(ArchMipsRegNames::FpRMode))));
  }
  static inline void setReg(ThreadContext *context, RegName name, T val){
    I(sizeof(T)==sizeof(uint32_t));
    MipsRegAccess<mode,uint32_t,RegTypeGpr>::setReg(context,RegName(ArchMipsRegNames::RegFCSR),uint32_t(val)&0x17FFFFFC);
    for(int32_t fcc=ArchMipsRegNames::FccNameLb;fcc!=ArchMipsRegNames::FccNameUb;fcc++){
      bool fccval=(uint32_t(val)&(uint32_t(1)<<(23+(fcc-ArchMipsRegNames::FccNameLb)+(fcc!=ArchMipsRegNames::FccNameLb))));
      MipsRegAccess<mode,bool,RegTypeGpr>::setReg(context,RegName(fcc),fccval);
    }
    MipsRegAccess<mode,uint8_t,RegTypeGpr>::setReg(context,RegName(ArchMipsRegNames::FpRMode),uint8_t(val)&3);
  }
};
#if (defined SUPPORT_MIPS32)
template<>
class ArchDefs<ExecModeMips32> : public ArchRegNames<ExecModeMips32>, public ArchRegSizes<ExecModeMips32>{
 public:
  template<typename T, RegName RTyp>
  struct RegAccess : public MipsRegAccess<ExecModeMips32,T,RTyp>{
  };
  template<typename T, RegName RTyp>
  static inline T getReg(const ThreadContext *context, RegName name){
    return MipsRegAccess<ExecModeMips32,T,RTyp>::getReg(context,name);
  }
  template<typename T, RegName RTyp>
  static inline void setReg(ThreadContext *context, RegName name, T val){
    return MipsRegAccess<ExecModeMips32,T,RTyp>::setReg(context,name,val);
  }
};
#endif
#if (defined SUPPORT_MIPSEL32)
template<>
class ArchDefs<ExecModeMipsel32> : public ArchRegNames<ExecModeMipsel32>, public ArchRegSizes<ExecModeMipsel32>{
 public:
  template<typename T, RegName RTyp>
  struct RegAccess : public MipsRegAccess<ExecModeMipsel32,T,RTyp>{
  };
  template<typename T, RegName RTyp>
  static inline T getReg(const ThreadContext *context, RegName name){
    return MipsRegAccess<ExecModeMipsel32,T,RTyp>::getReg(context,name);
  }
  template<typename T, RegName RTyp>
  static inline void setReg(ThreadContext *context, RegName name, T val){
    return MipsRegAccess<ExecModeMipsel32,T,RTyp>::setReg(context,name,val);
  }
};
#endif
#if (defined SUPPORT_MIPS64)
template<>
class ArchDefs<ExecModeMips64> : public ArchRegNames<ExecModeMips64>, public ArchRegSizes<ExecModeMips64>{
 public:
  template<typename T, RegName RTyp>
  static inline T getReg(const ThreadContext *context, RegName name){
    return MipsRegAccess<ExecModeMips64,T,RTyp>::getReg(context,name);
  }
  template<typename T, RegName RTyp>
  static inline void setReg(ThreadContext *context, RegName name, T val){
    return MipsRegAccess<ExecModeMips64,T,RTyp>::setReg(context,name,val);
  }
};
#endif
#if (defined SUPPORT_MIPSEL64)
template<>
class ArchDefs<ExecModeMipsel64> : public ArchRegNames<ExecModeMipsel64>, public ArchRegSizes<ExecModeMipsel64>{
 public:
  template<typename T, RegName RTyp>
  static inline T getReg(const ThreadContext *context, RegName name){
    return MipsRegAccess<ExecModeMipsel64,T,RTyp>::getReg(context,name);
  }
  template<typename T, RegName RTyp>
  static inline void setReg(ThreadContext *context, RegName name, T val){
    return MipsRegAccess<ExecModeMipsel64,T,RTyp>::setReg(context,name,val);
  }
};
#endif

#endif // !(defined ArchDefs_H)
