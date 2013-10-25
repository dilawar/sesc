#if !(defined MIPS_REGS_H)
#define MIPS_REGS_H

#include "Regs.h"
#include "ThreadContext.h"
#error Obsolete!
namespace Mips {
  
  enum MipsRegName{
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
    RegF1,
    RegF2,
    RegF3,
    RegF4,
    RegF5,
    RegF6,
    RegF7,
    RegF8,
    RegF9,
    RegF10,
    RegF11,
    RegF12,
    RegF13,
    RegF14,
    RegF15,
    RegF16,
    RegF17,
    RegF18,
    RegF19,
    RegF20,
    RegF21,
    RegF22,
    RegF23,
    RegF24,
    RegF25,
    RegF26,
    RegF27,
    RegF28,
    RegF29,
    RegF30,
    RegF31,
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

  template<ExecMode mode,typename T>
  inline T getRegCtl(const ThreadContext *context, RegName name){
    I(isCtlName(name));
    if(name==static_cast<RegName>(RegFCSR)){
      I(sizeof(T)==sizeof(uint32_t));
      const uint32_t *ptr=static_cast<const uint32_t *>(context->getReg(static_cast<RegName>(RegFCSR)));
      uint32_t retVal=(*ptr)&0x17FFFFFF;
      for(int32_t fcc=FccNameLb;fcc!=FccNameUb;fcc++)
        if(*static_cast<const bool *>(context->getReg(static_cast<RegName>(fcc))))
          retVal|=(1<<(23+(fcc-FccNameLb)+(fcc!=FccNameLb)));
      if(retVal!=*ptr)
        fail("FCSR lookup not matching fcc lookups\n");
      return T(retVal);
    }
    I(name>=static_cast<RegName>(FccNameLb));
    I(name<static_cast<RegName>(FccNameUb));
    const T *ptr=static_cast<const T *>(context->getReg(name));
    I((*ptr==0)||(*ptr==1));
//#if (defined DEBUG)
      uint32_t mask=(1<<(23+(name-FccNameLb)+(name!=static_cast<RegName>(FccNameLb))));
      const uint32_t *tst=static_cast<const uint32_t *>(context->getReg(static_cast<RegName>(RegFCSR)));
      if(bool(*ptr)!=bool((*tst)&mask))
        fail("Fcc lookup not matching FCSR lookup\n");
//#endif
    return *ptr;
  }

  template<ExecMode mode,typename T>
  inline void setRegCtl(ThreadContext *context, RegName name, T val){
    I(isCtlName(name));
    if(name==static_cast<RegName>(RegFCSR)){
      I(sizeof(T)==sizeof(uint32_t));
      uint32_t *ptr=static_cast<uint32_t *>(context->getReg(name));
      *ptr=uint32_t(val);
      for(int32_t fcc=FccNameLb;fcc!=FccNameUb;fcc++){
        bool *fccptr=static_cast<bool *>(context->getReg(static_cast<RegName>(fcc)));
        *fccptr=(uint32_t(val)&(1<<(23+(fcc-FccNameLb)+(fcc!=FccNameLb))));
      }
    }else{
      I(name>=static_cast<RegName>(FccNameLb));
      I(name<static_cast<RegName>(FccNameUb));
      I((val==0)||(val==1));
      *(static_cast<T *>(context->getReg(name)))=val;
//#if (defined DEBUG)
      uint32_t mask=(1<<(23+(name-FccNameLb)+(name!=static_cast<RegName>(FccNameLb))));
      uint32_t *ptr=static_cast<uint32_t *>(context->getReg(static_cast<RegName>(RegFCSR)));
      if(val)
        *ptr|=mask;
      else
        *ptr&=~mask;
//#endif
    }
  }
  
  template<ExecMode mode,typename T>
  inline T getRegGpr(const ThreadContext *context, RegName name){
    const T *ptr=static_cast<const T *>(context->getReg(name));
    return *(ptr+(sizeof(RegVal)/sizeof(T)-1)*(__BYTE_ORDER==__BIG_ENDIAN));
  }
  
  template<ExecMode mode,typename T>
  inline void setRegGpr(ThreadContext *context, RegName name, T val){
    I(!isFprName(name));
    I(static_cast<MipsRegName>(name)!=RegZero);
    T *ptr=static_cast<T *>(context->getReg(name));
    switch(sizeof(T)){
    case 8:
      I(context->getMode()==ExecModeMips64);
      *ptr=val;
      break;
    case 4: {
      *(ptr+(__BYTE_ORDER==__BIG_ENDIAN))=val;
#if (defined DEBUG)
      T chk=getRegGpr<mode,T>(context,name);
      I(chk==val);
#endif
      I(static_cast<uint32_t>(*static_cast<uint64_t *>(context->getReg(name)))==static_cast<uint32_t>(val));
    } break;
    case 1: {
      setRegGpr<mode,int32_t>(context,name,int8_t(val));
#if (defined DEBUG)
      T chk=getRegGpr<mode,T>(context,name);
      I(chk==val);
#endif
    } break;
    }
  }
  
  template<ExecMode mode, typename T>
  inline T getRegFpr(const ThreadContext *context, RegName name){
    I(isFprName(name));
    switch(sizeof(T)){
    case 8: {
      const T *ptr=static_cast<const T *>(context->getReg(name));
      return *ptr;
    } break;
    case 4: {
      I(__FLOAT_WORD_ORDER==__BYTE_ORDER);
      switch(mode){
      case ExecModeMips64: {
	const T *ptr=static_cast<const T *>(context->getReg(name));
        return *(ptr+(__BYTE_ORDER==__BIG_ENDIAN));
      } break;
      case ExecModeMips32: {
        bool isOdd=static_cast<bool>(name&1);
	const T *ptr=static_cast<const T *>(context->getReg(static_cast<RegName>(name^isOdd)));
        return *(ptr+(isOdd^(__BYTE_ORDER==__BIG_ENDIAN)));
      } break;
      }
    } break;
    default: fail("getRegFpr with unsuppoerted operand size\n");
    }
  }
  
  template<ExecMode mode, typename T>
  inline void setRegFpr(ThreadContext *context, RegName name, T val){
    I(isFprName(name));
    switch(sizeof(T)){
    case 8: {
      T *ptr=static_cast<T *>(context->getReg(name));
      *ptr=val;
    } break;
    case 4:
      I(__FLOAT_WORD_ORDER==__BYTE_ORDER);
      switch(mode){
      case ExecModeMips64: {
	T *ptr=static_cast<T *>(context->getReg(name));
        *(ptr+(__BYTE_ORDER==__BIG_ENDIAN))=val;
      } break;
      case ExecModeMips32: {
        bool isOdd=static_cast<bool>(name&1);
	T *ptr=static_cast<T *>(context->getReg(static_cast<RegName>(name^isOdd)));
        *(ptr+(isOdd^(__BYTE_ORDER==__BIG_ENDIAN)))=val;
      } break;
      }
    }
  }
  
  template<ExecMode mode, typename T, RegName RTyp>
  inline void setRegAny(ThreadContext *context, RegName name, T val){
    if(isGprName(RTyp)||isSpcName(RTyp)){
      if((!isGprName(name))&&(!isSpcName(name)))
	fail("RTyp is Gpr or Spc, but name is 0x%x in setRegAny\n",name);
      return setRegGpr<mode,T>(context,name,val);
    }else if(isFprName(RTyp)){
      if(!isFprName(name))
	fail("RTyp is Fpr, but name is 0x%x in setRegAny\n",name);
      return setRegFpr<mode,T>(context,name,val);
    }else if(isCtlName(RTyp)){
      if(!isCtlName(name))
	fail("RTyp is Ctl, but name is 0x%x in setRegAny\n",name);
      return setRegCtl<mode,T>(context,name,val);
    }
    I(RTyp==RegDyn);
    if(isGprName(name)||isSpcName(name)){
      return setRegGpr<mode,T>(context,name,val);
    }else if(isFprName(name)){
      return setRegFpr<mode,T>(context,name,val);
    }else if(isCtlName(name)){
      return setRegCtl<mode,T>(context,name,val);
    }else{
      fail("RTyp is Dyn and name is unknown in setRegAny\n");
    }
  }
  template<ExecMode mode, typename T, RegName RTyp>
  inline T getRegAny(const ThreadContext *context, RegName name){
    if(isGprName(RTyp)||isSpcName(RTyp)){
      if((!isGprName(name))&&(!isSpcName(name)))
	fail("RTyp is Gpr or Spc, but name is 0x%x in getRegAny\n",name);
      return getRegGpr<mode,T>(context,name);
    }else if(isFprName(RTyp)){
      if(!isFprName(name))
	fail("RTyp is Fpr, but name is 0x%x in getRegAny\n",name);
      return getRegFpr<mode,T>(context,name);
    }else if(isCtlName(RTyp)){
      if(!isCtlName(name))
	fail("RTyp is Ctl, but name is 0x%x in getRegAny\n",name);
      return getRegCtl<mode,T>(context,name);
    }
    I(RTyp==RegDyn);
    if(isGprName(name)||isSpcName(name)){
      return getRegGpr<mode,T>(context,name);
    }else if(isFprName(name)){
      return getRegFpr<mode,T>(context,name);
    }else if(isCtlName(name)){
      return getRegCtl<mode,T>(context,name);
    }else{
      fail("RTyp is Dyn and name is unknown in getRegAny\n");
    }
    return 0;
  }

}

#endif // !(defined MIPS_REGS__H)
