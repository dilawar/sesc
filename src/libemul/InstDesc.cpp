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

// We need to emulate all sorts of floating point operations
#include <math.h>
// We need to control the rounding mode for floating point operations
#if !(defined NO_FENV_H) 
#include <fenv.h>
#endif
// We use functionals in our implementation
#include <functional>
// We need std::vector
#include <vector>

#include "InstDesc.h"
#include "ThreadContext.h"
#include "LinuxSys.h"
//#include "MipsRegs.h"
#include "ArchDefs.h"
// Get endian conversion definitions
#include "EndianDefs.h"
// To get definition of fail()
#include "EmulInit.h"
// This is the SESC Instruction class to let us create a DInst
#include "Instruction.h"
// To get getContext(pid), needed to implement LL/SC
// (Remove this when ThreadContext stays in one place on thread switching)
#include "OSSim.h"

template<size_t siz>
struct TypesBySize{
};
template<>
class TypesBySize<sizeof(uint8_t)>{
 public:
  typedef int8_t  sig;
  typedef uint8_t uns;
};
template<>
class TypesBySize<sizeof(uint16_t)>{
 public:
  typedef int16_t  sig;
  typedef uint16_t uns;
};
template<>
class TypesBySize<sizeof(uint32_t)>{
 public:
  typedef int32_t  sig;
  typedef uint32_t uns;
};
template<>
class TypesBySize<sizeof(uint64_t)>{
 public:
  typedef int64_t  sig;
  typedef uint64_t uns;
};
template<typename T>
class TypeInfo{
public:
  static const bool isSig=(static_cast<T>(-1)<static_cast<T>(1));
  typedef typename TypesBySize<sizeof(T)>::uns uns;
  typedef typename TypesBySize<sizeof(T)>::sig sig;
};

typedef enum{
  ValNo,
  ValD1,
  ValD2,
  ValR1,
  ValR2,
  ValR3,
  ValI1,
  ValI2,
  ValAC,
  ValAN,
} ValDsc;
template <ValDsc _S1, ValDsc _S2, ValDsc _S3, ValDsc _R>
struct vals{
  static const ValDsc SVal1=_S1;
  static const ValDsc SVal2=_S2;
  static const ValDsc SVal3=_S3;
  static const ValDsc RVal =_R;
};
template <ValDsc _S1, ValDsc _S2, ValDsc _S3, ValDsc _R=ValNo>
struct vals3 : vals<_S1,_S2,_S3,_R>{
};
template <ValDsc _S1, ValDsc _S2, ValDsc _R=ValNo>
struct vals2 : vals<_S1,_S2,ValNo,_R>{
};
template <ValDsc _S1, ValDsc _R=ValNo>
struct vals1 : vals<_S1,ValNo,ValNo,_R>{
};
template <ValDsc _R=ValNo>
struct vals0 : vals<ValNo,ValNo,ValNo,_R>{
};

namespace fns{
  template <class _V, class _TR, class _T1, class _T2, class _T3>
  struct function : public _V{
    typedef _V  Vals;
    typedef _TR TRes;
    typedef _T1 TArg1;
    typedef _T2 TArg2;
    typedef _T3 TArg3;
  };
  template <class _V, class _TR, class _T1, class _T2, class _T3>
  struct function3 : public function<_V,_TR,_T1,_T2,_T3>{
  };
  template <class _V, class _TR, class _T1, class _T2>
  struct function2 : public function<_V,_TR,_T1,_T2,int>{
  };
  template <class _V, class _TR, class _T1>
  struct function1 : public function<_V,_TR,_T1,int,int>{
  };
  template <class _V, class _TR>
  struct function0 : public function<_V,_TR,int,int,int>{
  };
  
  template <class _V, class _T>
  struct ident : public function1<_V,_T,_T>{
    static inline _T eval(const _T &arg1, int arg2=0, int arg3=0){ return arg1; }
  };
  template <class _V, class _T>
  struct bwnot : public function1<_V,_T,_T>{
    static inline _T eval(const _T &arg1, int arg2=0, int arg3=0){ return ~arg1; }
  };
  template <class _V, class _T>
  struct bwand : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1, const _T &arg2, int arg3=0){ return (arg1&arg2); }
  };
  template <class _V, class _T>
  struct bwor : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1, const _T &arg2, int arg3=0){ return (arg1|arg2); }
  };
  template <class _V, class _T>
  struct bwnor : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1, const _T &arg2, int arg3=0){ return ~(arg1|arg2); }
  };
  template <class _V, class _T>
  struct bwxor : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1, const _T &arg2, int arg3=0){ return (arg1^arg2); }
  };
  template <class _V, class _T>
  struct neg : public function1<_V,_T,_T>{
    static inline _T eval(const _T &arg1, int arg2=0, int arg3=0){ return -arg1; }
  };
  template <class _F>
  struct fneg : public _F{
  private:
    typedef typename _F::TArg1 TArg1;
    typedef typename _F::TArg2 TArg2;
    typedef typename _F::TArg3 TArg3;
    typedef typename _F::TRes  TRes;
    static inline TRes eval(const TArg1 &arg1=0, const TArg2 &arg2=0, const TArg3 &arg3=0){ return -_F::eval(arg1,arg2,arg3); }
  };
  template <class _V, class _T>
  struct abs : public function1<_V,_T,_T>{
    static inline _T eval(const _T &arg1, int arg2=0, int arg3=0){ return (arg1>=0)?arg1:-arg1; }
  };
  template <class _V, class _T>
  struct sqroot : public function1<_V,_T,_T>{
    static inline _T eval(const _T &arg1, int arg2=0, int arg3=0){ return sqrt(arg1); }
  };
  template <class _V, class _T>
  struct recip : public function1<_V,_T,_T>{
    static inline _T eval(const _T &arg1, int arg2=0, int arg3=0){ return _T(1)/arg1; }
  };
  template <class _V, class _T>
  struct rsqroot : public function1<_V,_T,_T>{
    static inline _T eval(const _T &arg1, int arg2=0, int arg3=0){ return _T(1)/sqrt(arg1); }
  };
  template <class _V, class _T>
  struct add : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){ return (arg1+arg2); }
  };
  template <class _V, class _T>
  struct sub : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){ return (arg1-arg2); }
  };
  template <class _V, class _T>
  struct nadd : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){ return -(arg1+arg2); }
  };
  template <class _V, class _T>
  struct nsub : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){ return -(arg1-arg2); }
  };
  template <class _V, class _T>
  struct mul : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){ return arg1*arg2; }
  };
  template <class _V, class _T> 
  struct mulhi : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){
      static const size_t hshift=sizeof(_T)*4;
      static const size_t hmask =(_T(1)<<hshift)-1;
      _T arg1lo  =arg1&hmask;
      _T arg1hi  =(arg1>>hshift);
      _T arg2lo  =arg2&hmask;
      _T arg2hi  =(arg2>>hshift);
      _T res1lo2lo=arg1lo*arg2lo;
      _T res1hi2lo=arg1hi*arg2lo;
      _T res1lo2hi=arg1lo*arg2hi;
      _T res1hi2hi=arg1hi*arg2hi;
      _T reslo=res1lo2lo+(res1hi2lo<<hshift)+(res1lo2hi<<hshift);
      _T carry=(((res1lo2lo>>hshift)&hmask)+(res1hi2lo&hmask)+(res1lo2hi&hmask))>>hshift;
      _T reshi=res1hi2hi+(res1hi2lo>>hshift)+(res1lo2hi>>hshift)+carry;
      return reshi;
    }
  };
//   template <class _V>
//   struct mulhi<_V,int32_t> : public function2<_V,int32_t,int32_t,int32_t>{
//     static inline int32_t eval(const int32_t &arg1,const int32_t &arg2, int arg3=0){
//       typedef int32_t _T;
//       static const size_t hshift=sizeof(_T)*4;
//       static const size_t hmask =(_T(1)<<hshift)-1;
//       _T arg1lo  =arg1&hmask;
//       _T arg1hi  =(arg1>>hshift);
//       _T arg2lo  =arg2&hmask;
//       _T arg2hi  =(arg2>>hshift);
//       _T res1lo2lo=arg1lo*arg2lo;
//       _T res1hi2lo=arg1hi*arg2lo;
//       _T res1lo2hi=arg1lo*arg2hi;
//       _T res1hi2hi=arg1hi*arg2hi;
//       _T reslo1=res1lo2lo;
//       _T reslo2=(res1hi2lo<<hshift);
//       _T reslo3=(res1lo2hi<<hshift);
//       _T reslohi=((reslo1>>hshift)&hmask)+((reslo2>>hshift)&hmask)+((reslo3>>hshift)&hmask);
//       _T reslo=reslo1+reslo2+reslo3;
//       _T reshi=res1hi2hi+(res1hi2lo>>hshift)+(res1lo2hi>>hshift)+(reslohi>>hshift);
//       if(reslo!=arg1*arg2)
// 	fail("Low mismatch\n");
//       int64_t res=int64_t(arg1)*int64_t(arg2);
//       int32_t *ptr=(int32_t *)(&res);
//       if(reslo!=*(ptr+(__BYTE_ORDER==__BIG_ENDIAN)))
// 	fail("Low mismatch 2\n");
//       if(reshi!=*(ptr+(__BYTE_ORDER!=__BIG_ENDIAN)))
// 	fail("High mismatch\n");
//       return *(ptr+(__BYTE_ORDER!=__BIG_ENDIAN));
//     }
//   };
//   template <class _V>
//   struct mulhi<_V,uint32_t> : public function2<_V,uint32_t,uint32_t,uint32_t>{
//     static inline uint32_t eval(const uint32_t &arg1,const uint32_t &arg2, int arg3=0){
//       typedef uint32_t _T;
//       static const size_t hshift=sizeof(_T)*4;
//       static const size_t hmask =(_T(1)<<hshift)-1;
//       _T arg1lo  =arg1&hmask;
//       _T arg1hi  =(arg1>>hshift);
//       _T arg2lo  =arg2&hmask;
//       _T arg2hi  =(arg2>>hshift);
//       _T res1lo2lo=arg1lo*arg2lo;
//       _T res1hi2lo=arg1hi*arg2lo;
//       _T res1lo2hi=arg1lo*arg2hi;
//       _T res1hi2hi=arg1hi*arg2hi;
//       _T reslo=res1lo2lo+(res1hi2lo<<hshift)+(res1lo2hi<<hshift);
//       _T carry=(((res1lo2lo>>hshift)&hmask)+(res1hi2lo&hmask)+(res1lo2hi&hmask))>>hshift;
//       _T reshi=res1hi2hi+(res1hi2lo>>hshift)+(res1lo2hi>>hshift)+carry;
//       return reshi;
//       if(reslo!=arg1*arg2)
// 	fail("Low mismatch\n");
//       uint64_t res=uint64_t(arg1)*uint64_t(arg2);
//       uint32_t *ptr=(uint32_t *)(&res);
//       if(reslo!=*(ptr+(__BYTE_ORDER==__BIG_ENDIAN)))
// 	fail("Low mismatch 2\n");
//       if(reshi!=*(ptr+(__BYTE_ORDER!=__BIG_ENDIAN)))
// 	fail("High mismatch\n");
//       return *(ptr+(__BYTE_ORDER!=__BIG_ENDIAN));
//     }
//   };
//   template <class _V>
//   struct mulhi<_V,int64_t> : public function2<_V,int64_t,int64_t,int64_t>{
//     static inline int64_t eval(const int64_t &arg1,const int64_t &arg2, int arg3=0){
//       int64_t res=(arg1>>32)*(arg2>>32)+(((arg1>>32)*arg2)>>32)+((arg1*(arg2>>32))>>32);
//       if(res<0)
// 	fail("hi(%ld * %ld)=%ld\n",arg1,arg2,res);
//       return res;
//     }
//   };
//   template <class _V>
//   struct mulhi<_V,uint64_t> : public function2<_V,uint64_t,uint64_t,uint64_t>{
//     static inline uint64_t eval(const uint64_t &arg1,const uint64_t &arg2, int arg3=0){
//       uint64_t res=(arg1>>32)*(arg2>>32)+(((arg1>>32)*arg2)>>32)+((arg1*(arg2>>32))>>32);
//       return res;
//     }
//   };
  template <class _V, class _T>
  struct div : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){ return (arg1/arg2); }
  };
  template <class _V, class _T>
  struct mod : public function2<_V,_T,_T,_T>{
    static inline _T eval(const _T &arg1,const _T &arg2, int arg3=0){ return (arg1%arg2); }
  };
  template <class _V, class _T, class _S, _S mask>
  struct shl : public function2<_V,_T,_T,_S>{
    static inline _T eval(const _T &arg1, const _S &arg2, int arg3=0){ return (arg1<<(arg2&mask)); }
  };
  template <class _V, class _T, class _S, _S mask>
  struct shr : public function2<_V,_T,_T,_S>{
    static inline _T eval(const _T &arg1, const _S &arg2, int arg3=0){ return (arg1>>(arg2&mask)); }
  };
  template<class _V, class _Ts, class _Td>
  struct fround : public function1<_V,_Td,_Ts>{
    static inline _Td eval(const _Ts& arg1, int arg2=0, int arg3=0){ return _Td(round(arg1)); }
  };
  template<class _V, class _Ts, class _Td>
  struct ftrunc : public function1<_V,_Td,_Ts>{
    static inline _Td eval(const _Ts& arg1, int arg2=0, int arg3=0){ return _Td(trunc(arg1)); }
  };
  template<class _V, class _Ts, class _Td>
  struct fceil : public function1<_V,_Td,_Ts>{
    static inline _Td eval(const _Ts& arg1, int arg2=0, int arg3=0){ return _Td(ceil(arg1)); }
  };
  template<class _V, class _Ts, class _Td>
  struct ffloor : public function1<_V,_Td,_Ts>{
    static inline _Td eval(const _Ts& arg1, int arg2=0, int arg3=0){ return _Td(floor(arg1)); }
  };
  template<class _V, class _Tp, bool ExcUn, bool UseLt, bool UseEq, bool UseUn>
  struct fcmp : public function2<_V,bool,_Tp,_Tp>{
    static inline bool eval(const _Tp& __x, const _Tp& __y, int arg3=0){
      return ((UseLt&&(__x<__y))||(UseEq&&(__x==__y))||(UseUn&&isunordered(__x,__y)));
    }
  };

  template <class _V, class _R=bool>
  struct tt : public function0<_V,_R>{
    static inline _R eval(int arg1=0, int arg2=0, int arg3=0){ return true; }
  };
  template <class _V, class _T, class _R=bool>
  struct eq : public function2<_V,_R,_T,_T>{
    static inline _R eval(const _T &arg1, const _T &arg2, int arg3=0){ return _R(arg1==arg2); }
  };
  template <class _V, class _T, class _R=bool>
  struct ne : public function2<_V,_R,_T,_T>{
    static inline _R eval(const _T &arg1, const _T &arg2, int arg3=0){ return _R(arg1!=arg2); }
  };
  template <class _V, class _T, class _R=bool>
  struct lt : public function2<_V,_R,_T,_T>{
    static inline _R eval(const _T &arg1, const _T &arg2, int arg3=0){ return _R(arg1<arg2); }
  };
  template <class _V, class _T, class _R=bool>
  struct gt : public function2<_V,_R,_T,_T>{
    static inline _R eval(const _T &arg1, const _T &arg2, int arg3=0){ return _R(arg1>arg2); }
  };
  template <class _V, class _T, class _R=bool>
  struct le : public function2<_V,_R,_T,_T>{
    static inline _R eval(const _T &arg1, const _T &arg2, int arg3=0){ return _R(arg1<=arg2); }
  };
  template <class _V, class _T, class _R=bool>
  struct ge : public function2<_V,_R,_T,_T>{
    static inline _R eval(const _T &arg1, const _T &arg2, int arg3=0){ return _R(arg1>=arg2); }
  };
  template <class _V, class _T, class _R=bool>
  struct eqz : public function1<_V,_R,_T>{
    static inline _R eval(const _T &arg1, int arg2=0, int arg3=0){ return _R(arg1==_T(0)); }
  };
  template <class _V, class _T, class _R=bool>
  struct nez : public function1<_V,_R,_T>{
    static inline _R eval(const _T &arg1, int arg2=0, int arg3=0){ return _R(arg1!=_T(0)); }
  };
  template <class _V, class _T, class _R=bool>
  struct ltz : public function1<_V,_R,_T>{
    static inline _R eval(const _T &arg1, int arg2=0, int arg3=0){ return _R(arg1<_T(0)); }
  };
  template <class _V, class _T, class _R=bool>
  struct gtz : public function1<_V,_R,_T>{
    static inline _R eval(const _T &arg1, int arg2=0, int arg3=0){ return _R(arg1>_T(0)); }
  };
  template <class _V, class _T, class _R=bool>
  struct lez : public function1<_V,_R,_T>{
    static inline _R eval(const _T &arg1, int arg2=0, int arg3=0){ return _R(arg1<=_T(0)); }
  };
  template <class _V, class _T, class _R=bool>
  struct gez : public function1<_V,_R,_T>{
    static inline _R eval(const _T &arg1, int arg2=0, int arg3=0){ return _R(arg1>=_T(0)); }
  };
  template <class _F, class _TR>
  struct sxt : public function<typename _F::Vals,_TR, typename _F::TArg1, typename _F::TArg2, typename _F::TArg3>{
    typedef typename _F::TArg1 TArg1;
    typedef typename _F::TArg2 TArg2;
    typedef typename _F::TArg3 TArg3;    
    static inline _TR eval(const TArg1 &arg1=0, const TArg2 &arg2=0, const TArg3 &arg3=0){
      typename _F::TRes tmp=_F::eval(arg1,arg2,arg3);
      return static_cast<_TR>(static_cast<typename TypeInfo<_TR>::sig>(static_cast<typename TypeInfo<typename _F::TRes>::sig>(tmp)));
    }
  };
  template <class _F, class _TR>
  struct zxt : public function<typename _F::Vals,_TR, typename _F::TArg1, typename _F::TArg2, typename _F::TArg3>{
    typedef typename _F::TArg1 TArg1;
    typedef typename _F::TArg2 TArg2;
    typedef typename _F::TArg3 TArg3;    
    static inline _TR eval(const TArg1 &arg1=0, const TArg2 &arg2=0, const TArg3 &arg3=0){
      typename _F::TRes tmp=_F::eval(arg1,arg2,arg3);
      return static_cast<_TR>(static_cast<typename TypeInfo<_TR>::uns>(static_cast<typename TypeInfo<typename _F::TRes>::uns>(tmp)));
    }
  };
  template <class _F, class _TR>
  struct ext : public function<typename _F::Vals,_TR, typename _F::TArg1, typename _F::TArg2, typename _F::TArg3>{
    typedef typename _F::TArg1 TArg1;
    typedef typename _F::TArg2 TArg2;
    typedef typename _F::TArg3 TArg3;    
    static inline _TR eval(const TArg1 &arg1=0, const TArg2 &arg2=0, const TArg3 &arg3=0){
      if(TypeInfo<typename _F::TRes>::isSig)
	return sxt<_F,_TR>::eval(arg1,arg2,arg3);
      else
	return zxt<_F,_TR>::eval(arg1,arg2,arg3);
    }
  };
}

InstDesc::~InstDesc(void){
  if(sescInst)
    delete sescInst;
  ID(sescInst=0);
}

namespace Mips {
  template<typename DstT, typename SrcT>
  class Extend{
    typedef TypeInfo<DstT> DstInfo;
    typedef TypeInfo<SrcT> SrcInfo;
  public:
    static inline DstT sxt(SrcT val){
      return static_cast<DstT>(static_cast<typename TypeInfo<DstT>::sig>(static_cast<typename TypeInfo<SrcT>::sig>(val)));
    }
    static inline DstT zxt(SrcT val){
      return static_cast<DstT>(static_cast<typename DstInfo::sig>(static_cast<typename SrcInfo::sig>(val)));
    }
    static inline DstT ext(SrcT val){
      if(SrcInfo::isSig)
	return sxt(val);
      else
	return zxt(val);
    }
  };
  typedef std::set<Pid_t> PidSet;
  PidSet linkset;

  static inline void preExec(InstDesc *inst, ThreadContext *context){
#if (defined DEBUG_BENCH)
    //    context->execInst(inst->addr,getRegAny<mode,uint32_t,RegTypeGpr>(context,static_cast<RegName>(RegSP)));
#endif
  }

  InstDesc *emulCut(InstDesc *inst, ThreadContext *context){
    context->setIAddr(context->getIAddr());
    I(context->getIDesc()!=inst);
    return (*(context->getIDesc()))(context);
  }

  InstDesc *emulNop(InstDesc *inst, ThreadContext *context){
    context->updIAddr(inst->aupdate,1);
    return (*(inst+1))(context);
  }

  template<class _V, class _Ts, class _Td>
  struct mips_fcvt : public fns::function2<_V,_Td,_Ts,uint8_t>{
    static inline _Td eval(const _Ts& __x, uint8_t __y, int arg3=0){
#if !(defined NO_FENV_H)
      static int Mips32_RoundMode[] = {
        FE_TONEAREST,  /* 00 nearest   */
        FE_TOWARDZERO, /* 01 zero      */
        FE_UPWARD,     /* 10 plus inf  */
        FE_DOWNWARD    /* 11 minus inf */
      };
      int rm=Mips32_RoundMode[__y];
      int saverm=fegetround();
      fesetround(rm);
      _Td retVal=_Td(__x);
      fesetround(saverm);
      return retVal;
#else
      return _Td(__x);
#endif
    }
  };

  InstDesc *emulBreak(InstDesc *inst, ThreadContext *context){
    fail("emulBreak: BREAK instruction not supported yet at 0x%08x\n",context->getIAddr());
    return inst;
  }

  InstDesc *emulSyscl(InstDesc *inst, ThreadContext *context){
    return context->getSystem()->sysCall(context,inst);
  }

  enum InstCtlInfoEnum{
    CtlInv  = 0x0000,   // No instruction should have CtlInfo of zero (should at least have length)
    
    CtlNorm = 0x0001,   // Regular (non-branch) instruction
    CtlBran = 0x0002,   // Branch instruction
    CtlLkly = 0x0004,   // ISA indicates that branch/jump is likely
    CtlTarg = 0x0008,   // Branch/jump has a fixed target
    CtlAdDS = 0x0010,   // Decode the delay slot after this but don't map it
    CtlMpDS = 0x0020,   // Can map the delay slot after this (decode only if mapping needed)
    CtlNoDS = 0x0040,   // This is a decoding for a branch instruction without a delay slot, skip to next if there is a dependent delay slot
    CtlMore = 0x0080,   // There are more instructions in this decoding

    CtlNMor = CtlNorm + CtlMore,
    CtlBr    = CtlBran + CtlMpDS,
    CtlBrL   = CtlBran + CtlMpDS + CtlLkly,
    CtlBrT   = CtlBran + CtlMpDS + CtlTarg,
    CtlBrTL  = CtlBran + CtlMpDS + CtlLkly + CtlTarg,
    CtlPrBr  = CtlNorm + CtlMore + CtlAdDS,
    CtlOBr   = CtlNoDS + CtlBran + CtlMpDS,
    CtlOBrL  = CtlNoDS + CtlBran + CtlMpDS + CtlLkly,
    CtlOBrT  = CtlNoDS + CtlBran + CtlMpDS + CtlTarg,
    CtlOBrTL = CtlNoDS + CtlBran + CtlMpDS + CtlLkly + CtlTarg,
    CtlOPrBr = CtlNoDS + CtlNorm + CtlAdDS
  };
  typedef InstCtlInfoEnum InstCtlInfo;
  
  template<ExecMode mode>
  class DecodeInst{
    typedef ArchDefs<mode> ArchBase;
    typedef typename ArchBase::RawInst  RawInst;
    typedef typename ArchDefs<mode>::Taddr_t  Taddr_t;
    typedef typename ArchDefs<mode>::Tsaddr_t Tsaddr_t;
    typedef typename ArchDefs<mode>::Tregv_t  Tregv_t;
    typedef typename ArchDefs<mode>::Turegv_t Turegv_t;
    typedef typename ArchDefs<mode>::Tsregv_t Tsregv_t;
    typedef bool Tcond_t;
    template<typename T, RegName RTyp>
    static inline T getReg(const ThreadContext *context, RegName name){
      return ArchDefs<mode>::template getReg<T,RTyp>(context,name);
    }
    template<typename T, RegName RTyp>
    static inline void setReg(ThreadContext *context, RegName name, T val){
      return ArchDefs<mode>::template setReg<T,RTyp>(context,name,val);
    }
    template<typename T>
    inline static T fixEndian(T val){
      return EndianDefs<mode>::template fixEndian<T>(val);
    }
    static const RegName GprNameLb=ArchDefs<mode>::GprNameLb;
    static const RegName FprNameLb=ArchDefs<mode>::FprNameLb;
    static const RegName FcrNameLb=ArchDefs<mode>::FcrNameLb;
    static const RegName FccNameLb=ArchDefs<mode>::FccNameLb;
    static const RegName HwrNameLb=ArchDefs<mode>::HwrNameLb;
    static const RegName RegLink=ArchDefs<mode>::RegLink;
    static const RegName RegTmp =ArchDefs<mode>::RegTmp;
    static const RegName RegBTmp=ArchDefs<mode>::RegBTmp;
    static const RegName RegFTmp=ArchDefs<mode>::RegFTmp;
    static const RegName RegFCSR=ArchDefs<mode>::RegFCSR;
    static const RegName FpRMode=ArchDefs<mode>::FpRMode;
    static const RegName RegAT  =ArchDefs<mode>::RegAT;
    static const RegName RegRA  =ArchDefs<mode>::RegRA;
    static const RegName RegSP  =ArchDefs<mode>::RegSP;
    static const RegName RegHi  =ArchDefs<mode>::RegHi;
    static const RegName RegLo  =ArchDefs<mode>::RegLo;
    static const RegName RegF0  =ArchDefs<mode>::RegF0;
    static const RegName RegF31 =ArchDefs<mode>::RegF31;
    static const RegName RegCond=ArchDefs<mode>::RegCond;
    static const RegName RegZero=ArchDefs<mode>::RegZero;
    static const RegName RegJunk=ArchDefs<mode>::RegJunk;
    static const RegName RegTPtr=ArchDefs<mode>::RegTPtr;

    class OpKey{
      RawInst mask;
      RawInst val;
    public:
      OpKey(RawInst mask, RawInst val)
	: mask(mask), val(val){
      }
      bool operator<(const OpKey &other) const{
	if(mask==other.mask)
	  return val<other.val;
	return mask<other.mask;
      }
      bool operator==(const OpKey &other) const{
	return (mask==other.mask)&&(val==other.val);
      }
      RawInst getMask(void)const { return mask; }
      RawInst getVal(void)const { return val; }
      bool contains(const OpKey &other) const{
	return ((other.mask&mask)==mask)&&(((other.val&mask)==val));
      }
    };

    // What is the encoding for the instruction's register argument
    enum InstArgInfo  { ArgNo, ArgRd, ArgRt, ArgRs, ArgFd, ArgFt, ArgFs, ArgFr, ArgHs,
			ArgFCs, ArgFCSR, ArgRM, ArgFccc, ArgFbcc,
			ArgTmp, ArgBTmp, ArgFTmp, ArgCond, ArgRa, ArgHi, ArgLo, ArgZero };
    // What is the format of the immediate (if any)
    enum InstImmInfo  { ImmNo, ImmJpTg, ImmBrOf, ImmSExt, ImmZExt, ImmLui, ImmSh, ImmSh32, ImmExCd, ImmTrCd};

    class OpData{
    public:
      RawInst     mask;
      RawInst     val;
      const char *name;
      EmulFunc   *emul;
      InstCtlInfo ctl;
      InstTypInfo typ;
      InstArgInfo dst;
      InstArgInfo src1;
      InstArgInfo src2;
      InstImmInfo imm;
      Opcode      next;
      OpData(void){
      }
      OpData(const char *name, InstCtlInfo ctl, InstTypInfo typ,
	     InstArgInfo dst, InstArgInfo src1, InstArgInfo src2,
	     InstImmInfo imm, EmulFunc emul)
	:  mask(0), val(0), name(name), emul(emul), ctl(ctl), typ(typ), dst(dst), src1(src1), src2(src2), imm(imm)
      {
      }
      bool addDSlot(void) const{
	return (ctl&CtlAdDS);
      }
      bool mapDSlot(void) const{
	return (ctl&CtlMpDS);
      }
    };

    class OpEntry : public std::vector<OpData>{
    public:
      RawInst   mask;
      OpEntry(void) : std::vector<OpData>(), mask(0){
      }
      OpEntry(RawInst mask, const OpEntry &src)
	: std::vector<OpData>(src), mask(mask){
      }
      OpEntry &operator<<(const OpData &datum){
        push_back(datum);
        return *this;
      }
    };

    typedef std::map<OpKey,OpEntry> OpMapBase;
    class OpMap : public OpMapBase{
      void build(const OpMapBase &src, OpKey key);
    public:
      OpMap(void);
      const OpEntry &operator[](RawInst raw) const{
	typename OpMapBase::const_iterator it=find(OpKey(0,0));
	while(true){
	  const OpEntry &curEntry=it->second;
	  if(!curEntry.mask)
	    return curEntry;
	  it=opMap.find(OpKey(curEntry.mask,raw&curEntry.mask));
	  if(it==opMap.end())
	    return curEntry;
	}
      }
    };
    static OpMap opMap;
    
  public:
    template<typename T>
    static inline T readMem(ThreadContext *context, Taddr_t addr){
      return fixEndian(context->readMemRaw<T>(addr));
    }
    template<typename T>
    static inline void writeMem(ThreadContext *context, Taddr_t addr, const T &val){
      context->writeMemRaw(addr,fixEndian(val));
      if(!linkset.empty()){
	PidSet::iterator pidIt=linkset.begin();
	while(pidIt!=linkset.end()){
	  if((*pidIt==context->getPid())||
	     (getReg<Taddr_t,RegTypeSpc>(osSim->getContext(*pidIt),static_cast<RegName>(RegLink))==(addr-(addr&0x7)))){
	    setReg<Taddr_t,RegTypeSpc>(osSim->getContext(*pidIt),static_cast<RegName>(RegLink),0);
	    linkset.erase(pidIt);
	    pidIt=linkset.begin();
	  }else{
	    pidIt++;
	  }
	}
      }
    }
    static RegName decodeArg(InstArgInfo arg, RawInst inst){
      switch(arg){
      case ArgNo:   return RegNone;
      case ArgRd:   return static_cast<RegName>(((inst>>11)&0x1F)+GprNameLb);
      case ArgRt:   return static_cast<RegName>(((inst>>16)&0x1F)+GprNameLb);
      case ArgRs:   return static_cast<RegName>(((inst>>21)&0x1F)+GprNameLb);
      case ArgTmp:  return static_cast<RegName>(RegTmp);
      case ArgBTmp: return static_cast<RegName>(RegBTmp);
      case ArgFd:   return static_cast<RegName>(((inst>> 6)&0x1F)+FprNameLb);
      case ArgFt:   return static_cast<RegName>(((inst>>16)&0x1F)+FprNameLb);
      case ArgFs:   return static_cast<RegName>(((inst>>11)&0x1F)+FprNameLb);
      case ArgFr:   return static_cast<RegName>(((inst>>21)&0x1F)+FprNameLb);
      case ArgFCs:  return static_cast<RegName>(((inst>>11)&0x1F)+FcrNameLb);
      case ArgHs:   return static_cast<RegName>(((inst>>11)&0x1F)+HwrNameLb);
      case ArgFTmp: return static_cast<RegName>(RegFTmp);
      case ArgFCSR: return static_cast<RegName>(RegFCSR);
      case ArgRM:   return static_cast<RegName>(FpRMode);
      case ArgFccc: return static_cast<RegName>(FccNameLb+((inst>>8)&0x7));
      case ArgFbcc: return static_cast<RegName>(FccNameLb+((inst>>18)&0x7));
      case ArgRa:   return static_cast<RegName>(RegRA);
      case ArgHi:   return static_cast<RegName>(RegHi);
      case ArgLo:   return static_cast<RegName>(RegLo);
      case ArgCond: return static_cast<RegName>(RegCond);
      default:
        fail("decodeArg called for invalid register arg %d in raw inst 0x%08x\n",arg,inst);
      }
      return RegNone;
    }
    static InstImm decodeImm(InstImmInfo imm, RawInst inst, VAddr addr){
      switch(imm){
      case ImmNo:   return uint32_t(0);
      case ImmJpTg: return static_cast<Taddr_t>(((addr+sizeof(inst))&(~0x0fffffff))|((inst&0x03ffffff)*sizeof(inst)));
      case ImmBrOf: return static_cast<Taddr_t>((addr+sizeof(inst))+((static_cast<int16_t>(inst&0xffff))*sizeof(inst)));
      case ImmSExt: return static_cast<int16_t>(inst&0xffff);
      case ImmZExt: return static_cast<uint16_t>(inst&0xffff);
      case ImmLui:  return static_cast<int32_t>((inst<<16)&0xffff0000);
      case ImmSh:   return static_cast<uint8_t>((inst>>6)&0x1F);
      case ImmSh32: return static_cast<uint8_t>(32+((inst>>6)&0x1F));
      case ImmExCd: return static_cast<uint32_t>((inst>>6)&0xFFFFF);
      case ImmTrCd: return static_cast<uint16_t>((inst>>6)&0x3FF);
      default:
        fail("decodeImm called for invalid imm %d in raw inst 0x%08x\n",imm,inst);
      }
      return uint32_t(0);
    }
    static bool isNop(ThreadContext *context, VAddr addr){
      RawInst raw=readMem<RawInst>(context,addr);
      const OpEntry &entry=opMap[raw];
      return (entry.front().typ==TypNop)&&!(entry.front().ctl&CtlMore);
    }
    static bool decodeInstSize(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, size_t &tsize, bool domap){
      // This is a handler set for calls/returns 
      AddressSpace::HandlerSet hset;
      // Function entry point may need to call a handler
      if((curAddr==funcAddr)&&context->getAddressSpace()->getCallHandlers(funcAddr,hset)){
        I(domap);
        tsize+=hset.size();
	hset.clear();
      }
      RawInst raw=readMem<RawInst>(context,curAddr);
      curAddr+=sizeof(RawInst);
      const OpEntry &entry=opMap[raw];
      typename OpEntry::const_iterator opIt(entry.begin());
      I(opIt!=entry.end());
      // If this is an optimized decoding for no-delay-slot branches and there is a delay slot, skip to non-optimized decoding
      if((opIt->ctl&CtlNoDS)&&!isNop(context,curAddr)){
	while(opIt->ctl&CtlMore){
	  opIt++;
	  I(opIt!=entry.end());
	}
	opIt++;
	I(opIt!=entry.end());
      }
      while(true){
	const OpData &data=*opIt;
	InstTypInfo typ=static_cast<InstTypInfo>(data.typ&TypSubMask);
        // Function return may need to call a handler
        if((typ==BrOpRet)&&context->getAddressSpace()->getRetHandlers(funcAddr,hset)){
          I(domap);
          tsize+=hset.size();
	  hset.clear();
        }
	tsize++;
	// Decode delay slots
	if(data.addDSlot()){
	  I(domap);
	  VAddr dsaddr=curAddr;
	  decodeInstSize(context,funcAddr,dsaddr,endAddr,tsize,false);
	}else if(data.mapDSlot()){
	  I(domap);
	  decodeInstSize(context,funcAddr,curAddr,endAddr,tsize,true);
	}
	if(!(data.ctl&CtlMore))
	  break;
	opIt++;
      }
      // Is this the end of this trace?
      if(domap&&(curAddr>=endAddr)){
	InstTypInfo typ=static_cast<InstTypInfo>(entry.back().typ&TypSubMask);
	// Unconditional jumps, calls, and returns allow clean trace breaks
	if((typ==BrOpJump)||(typ==BrOpCall)||(typ==BrOpRet))
	  return false;
	// Everything else needs an OpCut to link to the continuation in another trace
	if(endAddr)
	  tsize++;
      }
      // Exit the decoding loop
      return true;
    }
    static void decodeInst(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, InstDesc *&trace, bool domap){
      // This is a handler set for calls/returns 
      AddressSpace::HandlerSet hset;
      if(domap)
	context->getAddressSpace()->mapInst(curAddr,trace);
      // Add function entry handlers if this is a function entry point
      if((curAddr==funcAddr)&&context->getAddressSpace()->getCallHandlers(funcAddr,hset)){
        I(domap);
        while(!hset.empty()){
          AddressSpace::HandlerSet::iterator it=hset.begin();
          trace->emul=*it;
#if (defined DEBUG)
	  trace->addr=curAddr;
#endif
          trace++;
          hset.erase(it);
        }
      }
      VAddr origiaddr=curAddr;
      RawInst raw=readMem<RawInst>(context,curAddr);
      curAddr+=sizeof(RawInst);
      const OpEntry &entry=opMap[raw];
      typename OpEntry::const_iterator opIt(entry.begin());
      I(opIt!=entry.end());
      // If this is an optimized decoding for no-delay-slot branches and there is a delay slot, skip to non-optimized decoding
      if((opIt->ctl&CtlNoDS)&&!isNop(context,curAddr)){
	while(opIt->ctl&CtlMore){
	  opIt++;
	  I(opIt!=entry.end());
	}
	opIt++;
	I(opIt!=entry.end());
      }
      InstDesc *myinst=0;
      while(opIt!=entry.end()){
	const OpData &data=*opIt;
	InstTypInfo typ=static_cast<InstTypInfo>(data.typ&TypSubMask);
        // Function return may need to call a handler
        if((typ==BrOpRet)&&context->getAddressSpace()->getRetHandlers(funcAddr,hset)){
          I(domap);
          while(!hset.empty()){
            AddressSpace::HandlerSet::iterator it=hset.begin();
            trace->emul=*it;
#if (defined DEBUG)
	    trace->addr=curAddr;
#endif
            trace++;
            hset.erase(it);
          }
        }
	myinst=trace++;
	myinst->emul=data.emul;
#if (defined DEBUG)
	if(domap)
	  myinst->addr=origiaddr;
	else
	  myinst->addr=origiaddr-sizeof(RawInst);
	myinst->typ=data.typ;
	myinst->name=data.name;
#endif
	myinst->regDst=decodeArg(data.dst,raw);
	if(myinst->regDst==static_cast<RegName>(RegZero))
	  myinst->regDst=static_cast<RegName>(RegJunk);
	myinst->regSrc1=decodeArg(data.src1,raw);
	myinst->regSrc2=decodeArg(data.src2,raw);
	myinst->imm=decodeImm(data.imm,raw,origiaddr);
	// Decode delay slots
	if(data.addDSlot()){
	  I(domap);
	  VAddr dsaddr=curAddr;
	  decodeInst(context,funcAddr,dsaddr,endAddr,trace,false);
	  myinst->iupdate=trace-myinst;
	}else if(data.mapDSlot()){
 	  I(domap);
	  decodeInst(context,funcAddr,curAddr,endAddr,trace,true);
	  myinst->iupdate=trace-myinst;
	}else{
          myinst->iupdate=1;
        }
	myinst->sescInst=createSescInst(myinst,origiaddr,curAddr-origiaddr,data.typ,data.ctl);
	myinst->aupdate=0;
	if(!(data.ctl&CtlMore))
	  break;
	opIt++;
      }
      // the last uop in the instruction updates the PC (but only if instruction can be mapped)
      if(domap)
	myinst->aupdate=curAddr-origiaddr;
      // Is this the end of this trace?
      if(domap&&(curAddr==endAddr)){
	InstTypInfo typ=static_cast<InstTypInfo>(entry.back().typ&TypSubMask);
	// Unconditional jumps, calls, and returns don't continue directly to next instruction
	// Everything else needs an OpCut to link to the continuation in another trace
	if((typ!=BrOpJump)&&(typ!=BrOpCall)&&(typ!=BrOpRet)){
	  InstDesc *ctinst=trace++;
	  ctinst->emul=emulCut;
	  ctinst->regDst=ctinst->regSrc1=ctinst->regSrc2=RegNone;
	  ctinst->imm=0;
	  ctinst->iupdate=0;
	  ctinst->aupdate=0;
#if (defined DEBUG)
	  ctinst->addr=curAddr;
	  ctinst->typ=TypNop;
	  ctinst->name="_cut";
#endif
	  ctinst->sescInst=0;
	}
      }
    }
    // Create a SESC Instruction for this static instruction
    static Instruction *createSescInst(const InstDesc *inst, VAddr iaddr, size_t deltaAddr, InstTypInfo typ, InstCtlInfo ctl){
      Instruction *sescInst=new Instruction();
      sescInst->addr=iaddr;
      InstType    iType    =iOpInvalid;
      InstSubType iSubType =iSubInvalid;
      MemDataSize iDataSize=0;
      switch(typ&TypOpMask){
      case TypNop:
	sescInst->opcode=iALU;
	sescInst->subCode=iNop;
	break;
      case TypIntOp:
	switch(typ&TypSubMask){
	case IntOpALU:
	  sescInst->opcode=iALU;
	  break;
	case IntOpMul:
	  sescInst->opcode=iMult;
	  break;
	case IntOpDiv:
	  sescInst->opcode=iDiv;
	  break;
	default:
	  fail("createSescInst: Unknown subtype for TypIntOp\n");
	}
	sescInst->subCode=iSubInvalid;
	break;
      case TypFpOp:
	switch(typ&TypSubMask){
	case FpOpALU:
	  sescInst->opcode=fpALU;
	  break;
	case FpOpMul:
	  sescInst->opcode=fpMult;
	  break;
	case FpOpDiv:
	  sescInst->opcode=fpDiv;
	  break;
	default:
	  fail("createSescInst: Unknown subtype for TypFpOp\n");
	}
	sescInst->subCode=iSubInvalid;    
	break;
      case TypBrOp:
	sescInst->opcode=iBJ;
	switch(typ&TypSubMask){
	case BrOpJump:
	  sescInst->subCode=BJUncond;
	  break;
	case BrOpCond:
	  sescInst->subCode=BJCond;
	  break;
	case BrOpCall: case BrOpCCall:
	  sescInst->subCode=BJCall;
	  break;
	case BrOpRet: case BrOpCRet:
	  sescInst->subCode=BJRet;
	  break;
	case BrOpTrap: case BrOpCTrap:
	  sescInst->opcode=iALU;
	  sescInst->subCode=iNop;
	  break;
	default:
	  fail("createSescInst: Unknown subtype for TypBrOp\n");
	}
	break;
      case TypMemOp:
	switch(typ&TypSubMask){
	case TypMemLd: case TypSynLd:
	  sescInst->opcode=iLoad;
	  break;
	case TypMemSt: case TypSynSt:
	  sescInst->opcode=iStore;
	  break;
	default:
	  fail("createSescInst: Unknown subtype for TypMemOp\n");
	}
	sescInst->dataSize=(typ&MemSizeMask);
	sescInst->subCode=iMemory;
	break;
      default:
	fail("createSescInst: Unknown instruction type\n");
      }
      sescInst->skipDelay=deltaAddr;
      sescInst->uEvent=NoEvent;
      sescInst->guessTaken=false;
      sescInst->condLikely=false;
      sescInst->jumpLabel =false;
      if(sescInst->opcode==iBJ){
	if(ctl&CtlTarg){
	  sescInst->jumpLabel=true;
	  if(static_cast<Taddr_t>(inst->imm)<static_cast<Taddr_t>(iaddr))
	    sescInst->guessTaken=true;
	}
	if(ctl&CtlLkly){
	  sescInst->condLikely=true;
	  sescInst->guessTaken=true;
	}
	if(sescInst->subCode!=BJCond)
	  sescInst->guessTaken=true;
      }
      sescInst->src1=getSescRegType(inst->regSrc1,true);
      sescInst->src2=getSescRegType(inst->regSrc2,true);
      sescInst->dest=getSescRegType(inst->regDst,false);
      sescInst->src1Pool=Instruction::whichPool(sescInst->src1);
      sescInst->src2Pool=Instruction::whichPool(sescInst->src2);
      sescInst->dstPool =Instruction::whichPool(sescInst->dest);
      return sescInst;
    }
    static inline RegType getSescRegType(RegName reg, bool src){
      if(src){
	if((reg==RegNone)||(reg==RegZero))
	  return NoDependence;
      }else{
	if((reg==RegNone)||(reg==RegJunk))
	  return InvalidOutput;
      }
      if((reg==RegTmp)||(reg==RegBTmp))
	return InternalReg;
      if(isGprName(reg)){
	I(reg>=RegAT);
	I(reg<=RegRA);
	return static_cast<RegType>(getRegNum(reg));
      }
      if(reg==RegFTmp)
	return IntFPBoundary;
      if(isFprName(reg)){
	I(reg>=RegF0);
	I(reg<=RegF31);
	return static_cast<RegType>(IntFPBoundary+getRegNum(reg));
      }
      if(getRegType(reg)==RegTypeCtl)
	return CoprocStatReg;
      if((reg==RegHi)||(reg==RegLo))
	return HiReg;
      if(reg==RegCond)
	return CondReg;
      if(reg==RegTPtr)
	return InternalReg;
      I(0);
      return NoDependence;
    }
    
    template<RegName DTyp, RegName S1Typ, RegName S2Typ, ValDsc V, class T>
    static inline T getSrc(InstDesc *inst, ThreadContext *context){
      switch(V){
      case ValR1: return getReg<T,S1Typ>(context,inst->regSrc1);
      case ValR2: return getReg<T,S2Typ>(context,inst->regSrc2);
      case ValI1: return T(inst->imm);
      case ValNo: return 0;
      default: fail("Unsupported V in getSrc\n");
      }
    }
    
    template<RegName DTyp, RegName S1Typ, RegName S2Typ, ValDsc V, class T>
    static inline void setDst(InstDesc *inst, ThreadContext *context, const T &val){
      switch(V){
      case ValD1: return setReg<T,DTyp>(context,inst->regDst,val);
      case ValNo: return ;
      default: fail("Unsupported V in setDst\n");
      }
    }
    typedef enum{
      NextCont, // Continue to the next InstDesc (uop) within the same architectural instruction
      NextInst, // Continue to the next InstDesc (uop) and update the PC (move to the next architectural instruction)
      NextNext, // Continue to the next InstDesc (uop), check if PC update is needed
      NextBReg, // Branch to a register address (if cond) otherwise skip DS
      NextBImm, // Branch to a constant address (if cond) otherwise skip DS
      NextAnDS  // Skip DS if cond, otherwise continue to next InstDesc
    } NextTyp;
    template<NextTyp NTyp>
    static inline InstDesc *nextInst(InstDesc *inst, ThreadContext *context){
      if(NTyp==NextCont){
	context->updIDesc(1);
      }else if(NTyp==NextInst){
	context->updIAddr(inst->aupdate,1);
	if(context->hasReadySignal())
	  context->getSystem()->handleSignals(context);
      }else{
	context->updIAddr(inst->aupdate,1);
	if((inst->aupdate)&&context->hasReadySignal())
	  context->getSystem()->handleSignals(context);
      }
      return inst;
    }

#define DecodeRegs(fname)						\
    template<RegName DT,RegName S1T,RegName S2T,NextTyp NT>		\
    static InstDesc *dcdS1T(InstDesc *inst, ThreadContext *context){	\
      if(S1T==RegDyn){							\
	if(((mode&ExecModeArchMask)==ExecModeArchMips)&&		\
	   isFprName(inst->regSrc1))					\
	  return dcdS1T<DT,RegTypeFpr,S2T,NT>(inst,context);		\
	if(inst->regSrc1==RegFCSR)					\
	  return dcdS1T<DT,RegFCSR,S2T,NT>(inst,context);		\
	return dcdS1T<DT,RegTypeGpr,S2T,NT>(inst,context);		\
      }									\
      inst->emul=emul<DT,S1T,S2T,NT>;					\
      return inst->emul(inst,context);					\
      return dcdS2T<DT,S1T,S2T,NT>(inst,context);			\
    }									\
    template<RegName DT,RegName S1T,RegName S2T,NextTyp NT>		\
    static InstDesc *dcdS2T(InstDesc *inst, ThreadContext *context){	\
      if(S2T==RegDyn){							\
	if(((mode&ExecModeArchMask)==ExecModeArchMips)&&		\
	   isFprName(inst->regSrc2))					\
	  return dcdS2T<DT,S1T,RegTypeFpr,NT>(inst,context);		\
	if(inst->regSrc1==RegFCSR)					\
	  return dcdS2T<DT,S1T,RegFCSR,NT>(inst,context);		\
	return dcdS2T<DT,S1T,RegTypeGpr,NT>(inst,context);		\
      }									\
      inst->emul=emul<DT,S1T,S2T,NT>;					\
      return inst->emul(inst,context);					\
      return dcdDT<DT,S1T,S2T,NT>(inst,context);			\
    }									\
    template<RegName DT,RegName S1T,RegName S2T,NextTyp NT>		\
    static InstDesc *dcdDT(InstDesc *inst, ThreadContext *context){	\
      if(DT==RegDyn){							\
	if(((mode&ExecModeArchMask)==ExecModeArchMips)&&		\
	   isFprName(inst->regDst))					\
	  return dcdDT<RegTypeFpr,S1T,S2T,NT>(inst,context);		\
	if(inst->regDst==RegFCSR)					\
	  return dcdDT<RegFCSR,S1T,S2T,NT>(inst,context);		\
	if(inst->regDst==RegSP)						\
	  return dcdDT<RegSP,S1T,S2T,NT>(inst,context);			\
	return dcdDT<RegTypeGpr,S1T,S2T,NT>(inst,context);		\
      }									\
      return dcdNT<DT,S1T,S2T,NT>(inst,context);			\
    }									\
    template<RegName DT,RegName S1T,RegName S2T,NextTyp NT>		\
    static InstDesc *dcdNT(InstDesc *inst, ThreadContext *context){	\
      if(NT==NextNext){							\
	if(inst->aupdate)						\
	  return dcdNT<DT,S1T,S2T,NextInst>(inst,context);		\
	else								\
	  return dcdNT<DT,S1T,S2T,NextCont>(inst,context);		\
      }									\
      inst->emul=emul<DT,S1T,S2T,NT>;					\
      return inst->emul(inst,context);					\
    }									\
   public:								\
    operator EmulFunc *() const{					\
      return emul<RegDyn,RegDyn,RegDyn,NextNext>;			\
    }
  
    template<typename Func>
    class emulAlu{
      template<RegName DTyp,RegName S1Typ,RegName S2Typ,NextTyp NTyp>
      static InstDesc *emul(InstDesc *inst, ThreadContext *context){
	preExec(inst,context);
#if (defined EMUL_VALGRIND)
	typename Func::TArg1 src1=getSrc<DTyp,S1Typ,S2Typ,Func::SVal1,typename Func::TArg1>(inst,context);
	if(src1==0)
	  if(inst==0)
	    fail("Never\n");
	typename Func::TArg2 src2=getSrc<DTyp,S1Typ,S2Typ,Func::SVal2,typename Func::TArg2>(inst,context);
	if(src2==0)
	  if(inst==0)
	    fail("Never\n");
#endif
	typename Func::TRes dst=Func::eval(getSrc<DTyp,S1Typ,S2Typ,Func::SVal1,typename Func::TArg1>(inst,context),
					   getSrc<DTyp,S1Typ,S2Typ,Func::SVal2,typename Func::TArg2>(inst,context));
#if (defined EMUL_VALGRIND)
	if(dst==0)
	  if(inst==0)
	    fail("Never\n");
#endif
	// Catch stack pointer updates to track alloc/dealloc and stack growth
	if(DTyp==RegSP){
	  VAddr newsp=VAddr(dst);
	  VAddr oldsp=getReg<Taddr_t,RegSP>(context,RegSP);
	  if(newsp>oldsp){
	    // Stack deallocation
	    
	  }else if(newsp<oldsp){
	    // Stack allocation
	    
	  }
	  // Grow stack if needed
	  VAddr  sa=context->getStackAddr();
	  if(newsp<sa){
	    fail("Growning the stack in emulAlu\n");
	    size_t sl=context->getStackSize();
	    I(context->getAddressSpace()->isSegment(sa,sl));
	    I(newsp<sa);
	    I(context->getAddressSpace()->isNoSegment(newsp,sa-newsp));
	    // If there is room to grow stack, do it
	    if((newsp<sa)&&(context->getAddressSpace()->isNoSegment(newsp,sa-newsp))){
	      // Try to grow it to a 16KB boundary, but if we can't then grow only to cover sp
	      VAddr newSa=newsp-(newsp&0x3FFF);
	      if(!context->getAddressSpace()->isNoSegment(newSa,sa-newSa))
		newSa=newsp;
	      context->getAddressSpace()->growSegmentDown(sa,newSa);
	      context->setStack(newSa,sa+sl);
	  }
	  }
	}
	// Update destination register
	//	setDst<DTyp,S1Typ,S2Typ,Func::RVal,typename Func::TRes>(inst,context,dst);
	setDst<DTyp,S1Typ,S2Typ,ValD1,typename Func::TRes>(inst,context,dst);
	return nextInst<NTyp>(inst,context);
      }
      DecodeRegs(emulAlu);
    };
    typedef enum{ DestNone, DestTarg, DestRetA, DestCond } DestTyp;
    
    template<typename CFunc, DestTyp DstTyp, NextTyp NxtTyp>
    class emulJump{
      template<RegName DTyp,RegName S1Typ,RegName S2Typ,NextTyp NTyp>
      static InstDesc *emul(InstDesc *inst, ThreadContext *context){
	I(inst->addr==context->getIAddr());
	preExec(inst,context);
#if (defined EMUL_VALGRIND)
	typename CFunc::TArg1 src1=getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal1,typename CFunc::TArg1>(inst,context);
	if(src1==0)
	  if(inst==0)
	    fail("Never\n");
	typename CFunc::TArg2 src2=getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal2,typename CFunc::TArg2>(inst,context);
	if(src2==0)
	  if(inst==0)
	    fail("Never\n");
#endif
	bool cond=CFunc::eval(getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal1,typename CFunc::TArg1>(inst,context),
			      getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal2,typename CFunc::TArg2>(inst,context));
#if (defined EMUL_VALGRIND)
	if(cond==0)
	  if(inst==0)
	    fail("Never\n");
#endif
	// Set the destination register (if any)
	if(DstTyp==DestTarg)
	  setReg<Taddr_t,RegTypeGpr>(context,inst->regDst,getReg<Taddr_t,RegTypeGpr>(context,inst->regSrc1));
	else if(DstTyp==DestRetA)
	  setReg<Taddr_t,RegTypeGpr>(context,inst->regDst,context->getIAddr()+2*sizeof(RawInst));
	else if(DstTyp==DestCond)
	  setReg<Tcond_t,RegTypeSpc>(context,inst->regDst,cond?1:0);
	// Update the PC and next instruction
	if(NxtTyp==NextBReg){
        context->setIAddr(getReg<Taddr_t,RegTypeGpr>(context,inst->regSrc1));
	}else if(NxtTyp==NextCont){
	  context->updIDesc(1);
	}else if(NxtTyp==NextBImm){
	  if(cond) {
	    context->setIAddr(Taddr_t(inst->imm));
	  }else{
	    context->updIAddr(inst->aupdate,inst->iupdate);
	  }
	}else if(NxtTyp==NextAnDS){
	  context->updIDesc(cond?1:inst->iupdate);
	}
	if(inst->aupdate)
	  context->getSystem()->handleSignals(context);
	return inst;
      }
      DecodeRegs(emulJump);
    };
    template<typename CFunc>
    class emulTcnd{
      template<RegName DTyp,RegName S1Typ,RegName S2Typ,NextTyp NTyp>
      static InstDesc *emul(InstDesc *inst, ThreadContext *context){
	preExec(inst,context);
	if(CFunc::eval(getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal1,typename CFunc::TArg1>(inst,context),
		       getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal2,typename CFunc::TArg2>(inst,context))){
	  fail("emulTcnd: trap caused at 0x%08x, not supported yet\n",context->getIAddr());
	}
	return nextInst<NTyp>(inst,context);
      }
      DecodeRegs(emulTcnd);
    };
    template<class CFunc, typename RegT>
    class emulMovc{
      template<RegName DTyp,RegName S1Typ,RegName S2Typ,NextTyp NTyp>
      static InstDesc *emul(InstDesc *inst, ThreadContext *context){
	preExec(inst,context);
	if(CFunc::eval(getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal1,typename CFunc::TArg1>(inst,context),
		       getSrc<DTyp,S1Typ,S2Typ,CFunc::SVal2,typename CFunc::TArg2>(inst,context))){
	  setReg<RegT,DTyp>(context,inst->regDst,getReg<RegT,S1Typ>(context,inst->regSrc1));
	}
	return nextInst<NTyp>(inst,context);
      }
      DecodeRegs(emulMovc);
    };
    typedef enum{
      LdStNormal = 0,
      LdStLlSc   = 1,
      LdStLR     = 2,
      LdStLeft   = 2,
      LdStRight  = LdStLR+4,
      LdStNoExt  = 8
    } LdStKind;
    
    template<class AFunc, LdStKind kind, typename MemT>
    class emulLd{
      template<RegName DTyp,RegName S1Typ,RegName S2Typ,NextTyp NTyp>
      static InstDesc *emul(InstDesc *inst, ThreadContext *context){
	preExec(inst,context);
#if (defined EMUL_VALGRIND)
	typename AFunc::TArg1 addr1=getSrc<DTyp,S1Typ,S2Typ,AFunc::SVal1,typename AFunc::TArg1>(inst,context);
	if(addr1==0)
	  if(inst==0)
	    fail("Never\n");
	typename AFunc::TArg2 addr2=getSrc<DTyp,S1Typ,S2Typ,AFunc::SVal2,typename AFunc::TArg2>(inst,context);
	if(addr2==0)
	  if(inst==0)
	    fail("Never\n");
#endif
	Taddr_t addr=AFunc::eval(getSrc<DTyp,S1Typ,S2Typ,AFunc::SVal1,typename AFunc::TArg1>(inst,context),
				 getSrc<DTyp,S1Typ,S2Typ,AFunc::SVal2,typename AFunc::TArg2>(inst,context));
// 	if((addr<0x7fffdbe4+4)&&(addr+sizeof(MemT)>0x7fffdbe4))
// 	  printf("Ld %d bytes 0x%016llx from 0x%08x (instr 0x%08x %s)\n",
// 		 sizeof(MemT),(unsigned long long)(readMem<MemT>(context,addr)),addr,inst->addr,inst->name);
#if (defined EMUL_VALGRIND)
	if(addr==0)
	  if(inst==0)
	    fail("Never\n");
#endif
	context->setDAddr(addr);
	if(kind==LdStLlSc){
	  setReg<Taddr_t,RegTypeSpc>(context,static_cast<RegName>(RegLink),addr-(addr&0x7));
	  linkset.insert(context->getPid());
	}
	if(kind&LdStLR){
	  size_t offs=(addr%sizeof(MemT));
	  MemT   mval=readMem<MemT>(context,addr-offs);
	  MemT   rval=getReg<MemT,DTyp>(context,inst->regDst);
          if((mode&ExecModeEndianMask)==ExecModeEndianLittle)
            offs=sizeof(MemT)-offs-1;
	  if(kind==LdStLeft){
            rval&=~(MemT(-1)<<(8*offs));
	    rval|=(mval<<(8*offs));
	  }else{
	    rval&=(MemT(-256)<<(8*offs));
	    rval|=((mval>>(8*(sizeof(MemT)-offs-1)))&(~(MemT(-256)<<(8*offs))));
	  }
#if (defined EMUL_VALGRIND)
	  if(rval==0)
	    if(inst==0)
	      fail("Never\n");
#endif
	  setReg<Tregv_t,DTyp>(context,inst->regDst,Extend<MemT,Tregv_t>::ext(rval));
	}else{
	  MemT  val=readMem<MemT>(context,addr);
#if (defined EMUL_VALGRIND)
	  if(val==0)
	    if(inst==0)
	      fail("Never\n");
#endif
	  if(kind&LdStNoExt)
	    setReg<MemT,DTyp>(context,inst->regDst,val);
	  else
	    setReg<Tregv_t,DTyp>(context,inst->regDst,Extend<MemT,Tregv_t>::ext(val));
	}
	return nextInst<NTyp>(inst,context);
      }
      DecodeRegs(emulLd);
    };
    
    template<class AFunc, LdStKind kind, typename MemT>
    class emulSt{
      template<RegName DTyp,RegName S1Typ,RegName S2Typ,NextTyp NTyp>
      static InstDesc *emul(InstDesc *inst, ThreadContext *context){
	preExec(inst,context);
	Taddr_t addr=AFunc::eval(getSrc<DTyp,S1Typ,S2Typ,AFunc::SVal1,typename AFunc::TArg1>(inst,context),
				 getSrc<DTyp,S1Typ,S2Typ,AFunc::SVal2,typename AFunc::TArg2>(inst,context));
// 	if((addr<0x7fffdbe4+4)&&(addr+sizeof(MemT)>0x7fffdbe4))
// 	  printf("St %d bytes 0x%016llx from 0x%08x (instr 0x%08x %s)\n",
// 		 sizeof(MemT),(unsigned long long)(getReg<MemT,S2Typ>(context,inst->regSrc2)),addr,inst->addr,inst->name);
	if((kind==LdStLlSc)&&(getReg<Taddr_t,RegTypeSpc>(context,RegLink)!=(addr-(addr&0x7)))){
	  setReg<Tregv_t,DTyp>(context,inst->regDst,0);
	}else{
	  context->setDAddr(addr);
	  MemT val=getReg<MemT,S2Typ>(context,inst->regSrc2);
	  if(kind&LdStLR){
            size_t tsiz=sizeof(MemT);
            size_t offs=(addr%tsiz);
	    EndianDefs<mode>::cvtEndian(val);
	    if((kind==LdStLeft)^((mode&ExecModeEndianMask)==ExecModeEndianLittle)){
	      context->writeMemFromBuf(addr,tsiz-offs,&val);
	    }else{
	      context->writeMemFromBuf(addr-offs,offs+1,((uint8_t *)(&val))+tsiz-1-offs);
	    }
	  }else{
	    writeMem<MemT>(context,addr,val);
	  }
	  if(kind==LdStLlSc)
	    setReg<Tregv_t,DTyp>(context,inst->regDst,1);
	}
	return nextInst<NTyp>(inst,context);
      }
      DecodeRegs(emulSt);
    };
#undef DecodeRegs
  };

  template<ExecMode mode>
  typename DecodeInst<mode>::OpMap DecodeInst<mode>::opMap;
  
  template<ExecMode mode>
  void DecodeInst<mode>::OpMap::build(const OpMapBase &src, OpKey key){
    RawInst newMask=0xFFFFFFFF;
    const OpEntry *entry(0);
    bool changeMask=false;
    for(typename OpMapBase::const_iterator it=src.begin();it!=src.end();it++){
      if(!it->first.getMask())
	fail("buildOpMap: mask is zero\n");
      OpKey itKey(it->first);
      if(!key.contains(itKey))
	continue;
      if(itKey==key){
        entry=&(it->second);
      }else{
	changeMask=true;
	newMask&=itKey.getMask();
      }
    }
    OpMapBase::operator[](key)=OpEntry(changeMask?newMask:0,entry?(*entry):OpEntry());
    if(!changeMask)
      return;
    for(typename OpMapBase::const_iterator it=src.begin();it!=src.end();it++){
      OpKey itKey(it->first);
      if(!key.contains(itKey))
	continue;
      OpKey newKey(newMask,itKey.getVal()&newMask);
      if(OpMapBase::count(newKey))
	continue;
      build(src,newKey);
    }
  }

  template<ExecMode mode>
  DecodeInst<mode>::OpMap::OpMap(void){

    OpMapBase ops;

    typedef vals0<ValNo>       VNone;
    typedef vals2<ValR1,ValR2> VR1R2;
    typedef vals2<ValR1,ValI1> VR1I1;
    typedef vals1<ValR1>       VR1;
    typedef vals1<ValR2>       VR2;
    typedef vals1<ValI1>       VI1;
    
    typedef fns::add<VR1I1,Tsaddr_t> AddrRegImm;
    typedef fns::add<VR1R2,Tsaddr_t> AddrRegReg;
    typedef fns::ident<VR1,Tsaddr_t> AddrReg;

    ops[OpKey(0xFC000000,0x0C000000)]<<OpData("jal(O)"    , CtlOBrT , BrOpCall , ArgRa  , ArgNo  , ArgNo  , ImmJpTg, emulJump<fns::tt <VNone> , DestRetA, NextBImm>())
				     <<OpData("jal(1)"    , CtlPrBr , IntOpALU , ArgRa  , ArgNo  , ArgNo  , ImmJpTg, emulJump<fns::tt <VNone> , DestRetA, NextCont>())
				     <<OpData("jal(2)"    , CtlBrT  , BrOpCall , ArgRa  , ArgNo  , ArgNo  , ImmJpTg, emulJump<fns::tt <VNone> , DestRetA, NextBImm>());
    
//    OpDataZ(0xFC00003F,0x00000009,"jalr"    ,CtlBr, BrOpJump);

    // Branches and jumps
    ops[OpKey(0xFFFF0000,0x10000000)]<<OpData("b(O)"      , CtlOBrT , BrOpJump , ArgNo  , ArgNo  , ArgNo  , ImmBrOf, emulJump<fns::tt <VNone> , DestNone, NextBImm>())
				     <<OpData("b(1)"      , CtlPrBr , IntOpALU , ArgNo  , ArgNo  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestNone, NextCont>())
				     <<OpData("b(2)"      , CtlBrT  , BrOpJump , ArgNo  , ArgNo  , ArgNo  , ImmBrOf, emulJump<fns::tt <VNone> , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x08000000)]<<OpData("j(O)"      , CtlOBrT , BrOpJump , ArgNo  , ArgNo  , ArgNo  , ImmJpTg, emulJump<fns::tt <VNone> , DestNone, NextBImm>())
				     <<OpData("j(1)"      , CtlPrBr , IntOpALU , ArgNo  , ArgNo  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestNone, NextCont>())
				     <<OpData("j(2)"      , CtlBrT  , BrOpJump , ArgNo  , ArgNo  , ArgNo  , ImmJpTg, emulJump<fns::tt <VNone> , DestNone, NextBImm>());
    ops[OpKey(0xFC00003F,0x00000008)]<<OpData("jr(O)"     , CtlOBr  , BrOpJump , ArgNo  , ArgRs  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestNone, NextBReg>())
				     <<OpData("jr(1)"     , CtlPrBr , IntOpALU , ArgBTmp, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestTarg, NextCont>())
				     <<OpData("jr(2)"     , CtlBr   , BrOpJump , ArgNo  , ArgBTmp, ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestNone, NextBReg>());
    ops[OpKey(0xFC00F83F,0x0000f809)]<<OpData("callr(O)"  , CtlOBr  , BrOpCall , ArgRa  , ArgRs  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestRetA, NextBReg>())
				     <<OpData("callr(1)"  , CtlNMor , IntOpALU , ArgBTmp, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestTarg, NextCont>())
				     <<OpData("callr(2)"  , CtlPrBr , IntOpALU , ArgRa  , ArgNo  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestRetA, NextCont>())
				     <<OpData("callr(3)"  , CtlBr   , BrOpCall , ArgNo  , ArgBTmp, ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestNone, NextBReg>());
    ops[OpKey(0xFFFF0000,0x04110000)]<<OpData("bal(O)"    , CtlOBrT , BrOpCall , ArgRa  , ArgNo  , ArgNo  , ImmBrOf, emulJump<fns::tt <VNone> , DestRetA, NextBImm>())
				     <<OpData("bal(1)"    , CtlPrBr , IntOpALU , ArgRa  , ArgNo  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestRetA, NextCont>())
				     <<OpData("bal(2)"    , CtlBrT  , BrOpCall , ArgNo  , ArgNo  , ArgNo  , ImmBrOf, emulJump<fns::tt <VNone> , DestNone, NextBImm>());
    ops[OpKey(0xFFE0003F,0x03e00008)]<<OpData("ret(O)"    , CtlOBr  , BrOpRet  , ArgNo  , ArgRa  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestNone, NextBReg>())
				     <<OpData("ret(1)"    , CtlPrBr , IntOpALU , ArgBTmp, ArgRa  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestTarg, NextCont>())
				     <<OpData("ret(2)"    , CtlBr   , BrOpRet  , ArgNo  , ArgBTmp, ArgNo  , ImmNo  , emulJump<fns::tt <VNone> , DestNone, NextBReg>());
    ops[OpKey(0xFC000000,0x10000000)]<<OpData("beq(O)"    , CtlOBrT , BrOpCond , ArgNo  , ArgRs  , ArgRt  , ImmBrOf, emulJump<fns::eq <VR1R2,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("beq(1)"    , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgRt  , ImmNo  , emulJump<fns::eq <VR1R2,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("beq(2)"    , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x50000000)]<<OpData("beql(O)"   , CtlOBrTL, BrOpCond , ArgNo  , ArgRs  , ArgRt  , ImmBrOf, emulJump<fns::eq <VR1R2,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("beql(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgRt  , ImmNo  , emulJump<fns::eq <VR1R2,Tsregv_t> , DestCond, NextAnDS>())
				     <<OpData("beql(2)"   , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x14000000)]<<OpData("bne(O)"    , CtlOBrT , BrOpCond , ArgNo  , ArgRs  , ArgRt  , ImmBrOf, emulJump<fns::ne <VR1R2,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bne(1)"    , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgRt  , ImmNo  , emulJump<fns::ne <VR1R2,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bne(2)"    , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x54000000)]<<OpData("bnel(O)"   , CtlOBrTL, BrOpCond , ArgNo  , ArgRs  , ArgRt  , ImmBrOf, emulJump<fns::ne <VR1R2,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bnel(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgRt  , ImmNo  , emulJump<fns::ne <VR1R2,Tsregv_t> , DestCond, NextAnDS>())
				     <<OpData("bnel(2)"   , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x18000000)]<<OpData("blez(O)"   , CtlOBrT , BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::lez<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("blez(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::lez<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("blez(2)"   , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x58000000)]<<OpData("blezl(O)"  , CtlOBrTL, BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::lez<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("blezl(1)"  , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::lez<VR1  ,Tsregv_t> , DestCond, NextAnDS>())
				     <<OpData("blezl(2)"  , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x1C000000)]<<OpData("bgtz(O)"   , CtlOBrT , BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::gtz<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bgtz(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::gtz<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bgtz(2)"   , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC000000,0x5C000000)]<<OpData("bgtzl(O)"  , CtlOBrTL, BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::gtz<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bgtzl(1)"  , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::gtz<VR1  ,Tsregv_t> , DestCond, NextAnDS>())
				     <<OpData("bgtzl(2)"  , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04000000)]<<OpData("bltz(O)"   , CtlOBrT , BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::ltz<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bltz(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::ltz<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bltz(2)"   , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04020000)]<<OpData("bltzl(O)"  , CtlOBrTL, BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::ltz<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bltzl(1)"  , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::ltz<VR1  ,Tsregv_t> , DestCond, NextAnDS>())
				     <<OpData("bltzl(2)"  , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04010000)]<<OpData("bgez(O)"   , CtlOBrT , BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::gez<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bgez(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::gez<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bgez(2)"   , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04030000)]<<OpData("bgezl(O)"  , CtlOBrTL, BrOpCond , ArgNo  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::gez<VR1  ,Tsregv_t> , DestNone, NextBImm>())
				     <<OpData("bgezl(1)"  , CtlPrBr , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::gez<VR1  ,Tsregv_t> , DestCond, NextAnDS>())
				     <<OpData("bgezl(2)"  , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFFE30000,0x45000000)]<<OpData("bc1f(O)"   , CtlOBrT , IntOpALU , ArgNo  , ArgFbcc, ArgNo  , ImmBrOf, emulJump<fns::eqz<VR1  ,Tcond_t >, DestNone, NextBImm>())
				     <<OpData("bc1f(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgFbcc, ArgNo  , ImmNo  , emulJump<fns::eqz<VR1  ,Tcond_t >, DestCond, NextCont>())
				     <<OpData("bc1f(2)"   , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t >, DestNone, NextBImm>());
    ops[OpKey(0xFFE30000,0x45020000)]<<OpData("bc1fl(O)"  , CtlOBrTL, IntOpALU , ArgNo  , ArgFbcc, ArgNo  , ImmBrOf, emulJump<fns::eqz<VR1  ,Tcond_t >, DestNone, NextBImm>())
				     <<OpData("bc1fl(1)"  , CtlPrBr , IntOpALU , ArgCond, ArgFbcc, ArgNo  , ImmNo  , emulJump<fns::eqz<VR1  ,Tcond_t >, DestCond, NextAnDS>())
				     <<OpData("bc1fl(2)"  , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFFE30000,0x45010000)]<<OpData("bc1t(O)"   , CtlOBrT , IntOpALU , ArgNo  , ArgFbcc, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t >, DestNone, NextBImm>())
				     <<OpData("bc1t(1)"   , CtlPrBr , IntOpALU , ArgCond, ArgFbcc, ArgNo  , ImmNo  , emulJump<fns::nez<VR1  ,Tcond_t >, DestCond, NextCont>())
				     <<OpData("bc1t(2)"   , CtlBrT  , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFFE30000,0x45030000)]<<OpData("bc1tl(O)"  , CtlOBrTL, IntOpALU , ArgNo  , ArgFbcc, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t >, DestNone, NextBImm>())
				     <<OpData("bc1tl(1)"  , CtlPrBr , IntOpALU , ArgCond, ArgFbcc, ArgNo  , ImmNo  , emulJump<fns::nez<VR1  ,Tcond_t >, DestCond, NextAnDS>())
				     <<OpData("bc1tl(2)"  , CtlBrTL , BrOpCond , ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04100000)]<<OpData("bltzal(O)" , CtlOBrT , BrOpCCall, ArgRa  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::ltz<VR1  ,Tsregv_t> , DestRetA, NextBImm>())
				     <<OpData("bltzal(1)" , CtlNMor , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::ltz<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bltzal(2)" , CtlPrBr , IntOpALU , ArgRa  , ArgNo  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone         > , DestRetA, NextCont>())
				     <<OpData("bltzal(3)" , CtlBrT  , BrOpCCall, ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04120000)]<<OpData("bltzall(O)", CtlOBrTL, BrOpCCall, ArgRa  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::ltz<VR1  ,Tsregv_t> , DestRetA, NextBImm>())
				     <<OpData("bltzall(1)", CtlNMor , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::ltz<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bltzall(2)", CtlPrBr , IntOpALU , ArgRa  , ArgCond, ArgNo  , ImmNo  , emulJump<fns::nez<VR1  ,Tcond_t > , DestRetA, NextAnDS>())
				     <<OpData("bltzall(3)", CtlBrTL , BrOpCCall, ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04110000)]<<OpData("bgezal(O)" , CtlOBrT , BrOpCCall, ArgRa  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::gez<VR1  ,Tsregv_t> , DestRetA, NextBImm>())
				     <<OpData("bgezal(1)" , CtlNMor , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::gez<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bgezal(2)" , CtlPrBr , IntOpALU , ArgRa  , ArgNo  , ArgNo  , ImmNo  , emulJump<fns::tt <VNone         > , DestRetA, NextCont>())
				     <<OpData("bgezal(3)" , CtlBrT  , BrOpCCall, ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    ops[OpKey(0xFC1F0000,0x04130000)]<<OpData("bgezall(O)", CtlOBrTL, BrOpCCall, ArgRa  , ArgRs  , ArgNo  , ImmBrOf, emulJump<fns::gez<VR1  ,Tsregv_t> , DestRetA, NextBImm>())
				     <<OpData("bgezall(1)", CtlNMor , IntOpALU , ArgCond, ArgRs  , ArgNo  , ImmNo  , emulJump<fns::gez<VR1  ,Tsregv_t> , DestCond, NextCont>())
				     <<OpData("bgezall(2)", CtlPrBr , IntOpALU , ArgRa  , ArgCond, ArgNo  , ImmNo  , emulJump<fns::nez<VR1  ,Tcond_t > , DestRetA, NextAnDS>())
				     <<OpData("bgezall(3)", CtlBrTL , BrOpCCall, ArgNo  , ArgCond, ArgNo  , ImmBrOf, emulJump<fns::nez<VR1  ,Tcond_t > , DestNone, NextBImm>());
    // Syscalls and traps
    ops[OpKey(0xFC00003F,0x0000000C)]<<OpData("syscall"   , CtlNorm , BrOpTrap , ArgNo  , ArgNo  , ArgNo  , ImmExCd, emulSyscl);
    ops[OpKey(0xFC00003F,0x0000000D)]<<OpData("break"     , CtlNorm , BrOpTrap , ArgNo  , ArgNo  , ArgNo  , ImmExCd, emulBreak);
    // Conditional traps
    ops[OpKey(0xFC00003F,0x00000030)]<<OpData( "tge"      , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgRt  , ImmTrCd, emulTcnd<fns::ge<VR1R2,Tsregv_t> >());
    ops[OpKey(0xFC00003F,0x00000031)]<<OpData( "tgeu"     , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgRt  , ImmTrCd, emulTcnd<fns::ge<VR1R2,Turegv_t> >());
    ops[OpKey(0xFC00003F,0x00000032)]<<OpData( "tlt"      , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgRt  , ImmTrCd, emulTcnd<fns::lt<VR1R2,Tsregv_t> >());
    ops[OpKey(0xFC00003F,0x00000033)]<<OpData( "tltu"     , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgRt  , ImmTrCd, emulTcnd<fns::lt<VR1R2,Turegv_t> >());
    ops[OpKey(0xFC00003F,0x00000034)]<<OpData( "teq"      , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgRt  , ImmTrCd, emulTcnd<fns::eq<VR1R2,Tsregv_t> >());
    ops[OpKey(0xFC00003F,0x00000036)]<<OpData( "tne"      , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgRt  , ImmTrCd, emulTcnd<fns::ne<VR1R2,Tsregv_t> >());
    ops[OpKey(0xFC1F0000,0x04080000)]<<OpData( "tgei"     , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgNo  , ImmSExt, emulTcnd<fns::ge<VR1I1,Tsregv_t> >());
    ops[OpKey(0xFC1F0000,0x04090000)]<<OpData( "tgeiu"    , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgNo  , ImmSExt, emulTcnd<fns::ge<VR1I1,Turegv_t> >());
    ops[OpKey(0xFC1F0000,0x040A0000)]<<OpData( "tlti"     , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgNo  , ImmSExt, emulTcnd<fns::lt<VR1I1,Tsregv_t> >());
    ops[OpKey(0xFC1F0000,0x040B0000)]<<OpData( "tltiu"    , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgNo  , ImmSExt, emulTcnd<fns::lt<VR1I1,Turegv_t> >());
    ops[OpKey(0xFC1F0000,0x040C0000)]<<OpData( "teqi"     , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgNo  , ImmSExt, emulTcnd<fns::eq<VR1I1,Tsregv_t> >());
    ops[OpKey(0xFC1F0000,0x040E0000)]<<OpData( "tnei"     , CtlNorm , BrOpCTrap, ArgNo  , ArgRs  , ArgNo  , ImmSExt, emulTcnd<fns::ne<VR1I1,Tsregv_t> >());
    // Conditional moves
    ops[OpKey(0xFC00003F,0x0000000A)]<<OpData( "movz"     , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulMovc<fns::eqz<VR2  ,Tregv_t>, Tregv_t  >());
    ops[OpKey(0xFC00003F,0x0000000B)]<<OpData( "movn"     , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulMovc<fns::nez<VR2  ,Tregv_t>, Tregv_t  >());
    ops[OpKey(0xFC01003F,0x00000001)]<<OpData( "movf"     , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgFbcc, ImmNo  , emulMovc<fns::eqz<VR2  ,Tcond_t>, Tregv_t  >());
    ops[OpKey(0xFC01003F,0x00010001)]<<OpData( "movt"     , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgFbcc, ImmNo  , emulMovc<fns::nez<VR2  ,Tcond_t>, Tregv_t  >());
    ops[OpKey(0xFFE0003F,0x46000012)]<<OpData( "movz.s"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRt  , ImmNo  , emulMovc<fns::eqz<VR2  ,Tregv_t>, float32_t>());
    ops[OpKey(0xFFE0003F,0x46000013)]<<OpData( "movn.s"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRt  , ImmNo  , emulMovc<fns::nez<VR2  ,Tregv_t>, float32_t>());
    ops[OpKey(0xFFE1003F,0x46000011)]<<OpData( "movf.s"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFbcc, ImmNo  , emulMovc<fns::eqz<VR2  ,Tcond_t>, float32_t>());
    ops[OpKey(0xFFE1003F,0x46010011)]<<OpData( "movt.s"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFbcc, ImmNo  , emulMovc<fns::nez<VR2  ,Tcond_t>, float32_t>());
    ops[OpKey(0xFFE0003F,0x46200012)]<<OpData( "movz.d"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRt  , ImmNo  , emulMovc<fns::eqz<VR2  ,Tregv_t>, float64_t>());
    ops[OpKey(0xFFE0003F,0x46200013)]<<OpData( "movn.d"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRt  , ImmNo  , emulMovc<fns::nez<VR2  ,Tregv_t>, float64_t>());
    ops[OpKey(0xFFE1003F,0x46200011)]<<OpData( "movf.d"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFbcc, ImmNo  , emulMovc<fns::eqz<VR2  ,Tcond_t>, float64_t>());
    ops[OpKey(0xFFE1003F,0x46210011)]<<OpData( "movt.d"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFbcc, ImmNo  , emulMovc<fns::nez<VR2  ,Tcond_t>, float64_t>());
    // Load/Store
    ops[OpKey(0xFC000000,0x80000000)]<<OpData( "lb"       , CtlNorm , MemOpLd1 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNormal, int8_t  >());
    ops[OpKey(0xFC000000,0x90000000)]<<OpData( "lbu"      , CtlNorm , MemOpLd1 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNormal, uint8_t >());
    ops[OpKey(0xFC000000,0x84000000)]<<OpData( "lh"       , CtlNorm , MemOpLd2 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNormal, int16_t >());
    ops[OpKey(0xFC000000,0x94000000)]<<OpData( "lhu"      , CtlNorm , MemOpLd2 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNormal, uint16_t>());
    ops[OpKey(0xFC000000,0x8C000000)]<<OpData( "lw"       , CtlNorm , MemOpLd4 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNormal, int32_t >());
    ops[OpKey(0xFC000000,0x9C000000)]<<OpData( "lwu"      , CtlNorm , MemOpLd4 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNormal, uint32_t>());
    ops[OpKey(0xFC000000,0x88000000)]<<OpData( "lwl"      , CtlNorm , MemOpLd4 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStLeft  , int32_t >());
    ops[OpKey(0xFC000000,0x98000000)]<<OpData( "lwr"      , CtlNorm , MemOpLd4 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStRight , int32_t >());
    ops[OpKey(0xFC000000,0xDC000000)]<<OpData( "ld"       , CtlNorm , MemOpLd8 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNormal, int64_t >());
    ops[OpKey(0xFC000000,0x68000000)]<<OpData( "ldl"      , CtlNorm , MemOpLd8 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStLeft  , int64_t >());
    ops[OpKey(0xFC000000,0x6C000000)]<<OpData( "ldr"      , CtlNorm , MemOpLd8 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStRight , int64_t >());
    ops[OpKey(0xFC000000,0xA0000000)]<<OpData( "sb"       , CtlNorm , MemOpSt1 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStNormal, uint8_t >());
    ops[OpKey(0xFC000000,0xA4000000)]<<OpData( "sh"       , CtlNorm , MemOpSt2 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStNormal, uint16_t>());
    ops[OpKey(0xFC000000,0xAC000000)]<<OpData( "sw"       , CtlNorm , MemOpSt4 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStNormal, uint32_t>());
    ops[OpKey(0xFC000000,0xA8000000)]<<OpData( "swl"      , CtlNorm , MemOpSt4 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStLeft  , uint32_t>());
    ops[OpKey(0xFC000000,0xB8000000)]<<OpData( "swr"      , CtlNorm , MemOpSt4 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStRight , uint32_t>());
    ops[OpKey(0xFC000000,0xFC000000)]<<OpData( "sd"       , CtlNorm , MemOpSt8 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStNormal, uint64_t>());
    ops[OpKey(0xFC000000,0xB0000000)]<<OpData( "sdl"      , CtlNorm , MemOpSt8 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStLeft  , uint64_t>());
    ops[OpKey(0xFC000000,0xB4000000)]<<OpData( "sdr"      , CtlNorm , MemOpSt8 , ArgNo  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStRight , uint64_t>());
    
    ops[OpKey(0xFC000000,0xC4000000)]<<OpData( "lwc1"     , CtlNorm , MemOpLd4 , ArgFt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNoExt , uint32_t>());
    ops[OpKey(0xFC000000,0xD4000000)]<<OpData( "ldc1"     , CtlNorm , MemOpLd8 , ArgFt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStNoExt , uint64_t>());
    //     ops[OpKey(0xFC000000,0xC8000000)]<<OpData("lwc2", CtlNorm , MemOpLd4>());
//     ops[OpKey(0xFC000000,0xD8000000)]<<OpData("ldc2", CtlNorm , MemOpLd8>());
    ops[OpKey(0xFC00003F,0x4C000000)]<<OpData( "lwxc1"    , CtlNorm , MemOpLd4 , ArgFd  , ArgRs  , ArgRt  , ImmNo  , emulLd<AddrRegReg, LdStNoExt , uint32_t>());
    ops[OpKey(0xFC00003F,0x4C000001)]<<OpData( "ldxc1"    , CtlNorm , MemOpLd8 , ArgFd  , ArgRs  , ArgRt  , ImmNo  , emulLd<AddrRegReg, LdStNoExt , uint64_t>());
    ops[OpKey(0xFC000000,0xE4000000)]<<OpData( "swc1"     , CtlNorm , MemOpSt4 , ArgNo  , ArgRs  , ArgFt  , ImmSExt, emulSt<AddrRegImm, LdStNormal, uint32_t>());
    ops[OpKey(0xFC000000,0xF4000000)]<<OpData( "sdc1"     , CtlNorm , MemOpSt8 , ArgNo  , ArgRs  , ArgFt  , ImmSExt, emulSt<AddrRegImm, LdStNormal, uint64_t>());
//     ops[OpKey(0xFC000000,0xE8000000)]<<OpData("swc2", CtlNorm , MemOpSt4>());
//     ops[OpKey(0xFC000000,0xF8000000)]<<OpData("sdc2", CtlNorm , MemOpSt8>());
//     ops[OpKey(0xFC000000,0xCC000000)]<<OpData("pref", CtlNorm , TypNop>());
//     ops[OpKey(0xFC00003F,0x4C00000F)]<<OpData( "prefx",CtlNorm , TypNop>());
    // These are two-part ops: calc addr, then store
    ops[OpKey(0xFC00003F,0x4C000008)]<<OpData("swxc1(1)"  , CtlNMor , IntOpALU , ArgTmp , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::add<VR1R2,Taddr_t> >())
				     <<OpData("swxc1(2)"  , CtlNorm , MemOpSt4 , ArgNo  , ArgTmp , ArgFs  , ImmNo  , emulSt<AddrReg   , LdStNoExt , uint32_t>());
    ops[OpKey(0xFC00003F,0x4C000009)]<<OpData("sdxc1(1)"  , CtlNMor , IntOpALU , ArgTmp , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::add<VR1R2,Taddr_t> >())
				     <<OpData("sdxc1(2)"  , CtlNorm , MemOpSt8 , ArgNo  , ArgTmp , ArgFs  , ImmNo  , emulSt<AddrReg   , LdStNoExt , uint64_t>());
    // Load-linked and store-conditional sync ops
    ops[OpKey(0xFC000000,0xC0000000)]<<OpData( "ll"       , CtlNorm , MemOpLl4 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStLlSc  , int32_t >());
    ops[OpKey(0xFC000000,0xE0000000)]<<OpData( "sc"       , CtlNorm , MemOpSc4 , ArgRt  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStLlSc  , int32_t >());
    ops[OpKey(0xFC000000,0xD0000000)]<<OpData( "lld"      , CtlNorm , MemOpLl8 , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulLd<AddrRegImm, LdStLlSc  , int64_t >());
    ops[OpKey(0xFC000000,0xF0000000)]<<OpData( "scd"      , CtlNorm , MemOpSc8 , ArgRt  , ArgRs  , ArgRt  , ImmSExt, emulSt<AddrRegImm, LdStLlSc  , int64_t >());
    ops[OpKey(0xFC00003F,0x0000000F)]<<OpData( "sync"     , CtlNorm , TypNop   , ArgNo  , ArgNo  , ArgNo  , ImmNo  , emulNop);
    
    // Constant transfers
    ops[OpKey(0xFC000000,0x3C000000)]<<OpData("lui"       , CtlNorm , IntOpALU , ArgRt  , ArgNo  , ArgNo  , ImmLui , emulAlu<fns::ident<VI1,Tsregv_t> >());
    ops[OpKey(0xFFE00000,0x24000000)]<<OpData("li"        , CtlNorm , IntOpALU , ArgRt  , ArgNo  , ArgNo  , ImmSExt, emulAlu<fns::ident<VI1,Tsregv_t> >());
    ops[OpKey(0xFFE00000,0x34000000)]<<OpData("liu"       , CtlNorm , IntOpALU , ArgRt  , ArgNo  , ArgNo  , ImmZExt, emulAlu<fns::ident<VI1,Turegv_t> >());
    // Shifts
    ops[OpKey(0xFC00003F,0x00000000)]<<OpData("sll"       , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh  , emulAlu<fns::sxt<fns::shl<VR1I1,uint32_t,uint8_t,0x1F>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000002)]<<OpData("srl"       , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh  , emulAlu<fns::sxt<fns::shr<VR1I1,uint32_t,uint8_t,0x1F>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000003)]<<OpData("sra"       , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh  , emulAlu<fns::sxt<fns::shr<VR1I1, int32_t, int8_t,0x1F>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000004)]<<OpData("sllv"      , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgRs  , ImmNo  , emulAlu<fns::sxt<fns::shl<VR1R2,uint32_t,uint8_t,0x1F>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000006)]<<OpData("srlv"      , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgRs  , ImmNo  , emulAlu<fns::sxt<fns::shr<VR1R2,uint32_t,uint8_t,0x1F>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000007)]<<OpData("srav"      , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgRs  , ImmNo  , emulAlu<fns::sxt<fns::shr<VR1R2, int32_t, int8_t,0x1F>,Tregv_t> >());
    if(sizeof(Tregv_t)>=sizeof(int64_t)){
    ops[OpKey(0xFC00003F,0x00000038)]<<OpData("dsll"      , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh  , emulAlu<fns::shl<VR1I1,uint64_t,uint8_t,0x1F> >());
    ops[OpKey(0xFC00003F,0x0000003A)]<<OpData("dsrl"      , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh  , emulAlu<fns::shr<VR1I1,uint64_t,uint8_t,0x1F> >());
    ops[OpKey(0xFC00003F,0x0000003B)]<<OpData("dsra"      , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh  , emulAlu<fns::shr<VR1I1, int64_t, int8_t,0x1F> >());
    ops[OpKey(0xFC00003F,0x0000003C)]<<OpData("dsll32"    , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh32, emulAlu<fns::shl<VR1I1,uint64_t,uint8_t,0x3F> >());
    ops[OpKey(0xFC00003F,0x0000003E)]<<OpData("dsrl32"    , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh32, emulAlu<fns::shr<VR1I1,uint64_t,uint8_t,0x3F> >());
    ops[OpKey(0xFC00003F,0x0000003F)]<<OpData("dsra32"    , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmSh32, emulAlu<fns::shr<VR1I1, int64_t, int8_t,0x3F> >());
    ops[OpKey(0xFC00003F,0x00000014)]<<OpData("dsllv"     , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgRs  , ImmNo  , emulAlu<fns::shl<VR1R2,uint64_t,uint8_t,0x3F> >());
    ops[OpKey(0xFC00003F,0x00000016)]<<OpData("dsrlv"     , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgRs  , ImmNo  , emulAlu<fns::shr<VR1R2,uint64_t,uint8_t,0x3F> >());
    ops[OpKey(0xFC00003F,0x00000017)]<<OpData("dsrav"     , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgRs  , ImmNo  , emulAlu<fns::shr<VR1R2, int64_t, int8_t,0x3F> >());
    }
    // Logical
    ops[OpKey(0xFC00003F,0x00000024)]<<OpData("and"       , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::bwand<VR1R2,Turegv_t> >());
    ops[OpKey(0xFC00003F,0x00000025)]<<OpData("or"        , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::bwor <VR1R2,Turegv_t> >());
    ops[OpKey(0xFC00003F,0x00000026)]<<OpData("xor"       , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::bwxor<VR1R2,Turegv_t> >());
    ops[OpKey(0xFC00003F,0x00000027)]<<OpData("nor"       , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::bwnor<VR1R2,Turegv_t> >());
    ops[OpKey(0xFC000000,0x30000000)]<<OpData("andi"      , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmZExt, emulAlu<fns::bwand<VR1I1,Turegv_t> >());
    ops[OpKey(0xFC000000,0x34000000)]<<OpData("ori"       , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmZExt, emulAlu<fns::bwor <VR1I1,Turegv_t> >());
    ops[OpKey(0xFC000000,0x38000000)]<<OpData("xori"      , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmZExt, emulAlu<fns::bwxor<VR1I1,Turegv_t> >());
    ops[OpKey(0xFFE0003F,0x00000027)]<<OpData("not"       , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmNo  , emulAlu<fns::bwnot<VR1  ,Turegv_t> >());
    // Arithmetic
    ops[OpKey(0xFC00003F,0x00000020)]<<OpData("add"       , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::add<VR1R2,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000021)]<<OpData("addu"      , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::add<VR1R2,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000022)]<<OpData("sub"       , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::sub<VR1R2,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000023)]<<OpData("subu"      , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::sub<VR1R2,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC000000,0x20000000)]<<OpData("addi"      , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulAlu<fns::sxt<fns::add<VR1I1,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC000000,0x24000000)]<<OpData("addiu"     , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulAlu<fns::sxt<fns::add<VR1I1,int32_t>,Tregv_t> >());
    ops[OpKey(0xFFE0003F,0x00000023)]<<OpData("negu"      , CtlNorm , IntOpALU , ArgRd  , ArgRt  , ArgNo  , ImmNo  , emulAlu<fns::sxt<fns::neg<VR1  ,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000018)]<<OpData("mult(1)"   , CtlNMor , IntOpMul , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mul<VR1R2,int32_t>,Tregv_t> >())
				     <<OpData("mult(2)"   , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mulhi<VR1R2,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000019)]<<OpData("multu(1)"  , CtlNMor , IntOpMul , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mul<VR1R2,uint32_t>,Tregv_t> >())
				     <<OpData("multu(2)"  , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mulhi<VR1R2,uint32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x0000001A)]<<OpData("div(1)"    , CtlNMor , IntOpDiv , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::div<VR1R2,int32_t>,Tregv_t> >())
				     <<OpData("div(2)"    , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mod<VR1R2,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x0000001B)]<<OpData("divu(1)"   , CtlNMor , IntOpDiv , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::div<VR1R2,uint32_t>,Tregv_t> >())
				     <<OpData("divu(2)"   , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mod<VR1R2,uint32_t>,Tregv_t> >());
    if(sizeof(Tregv_t)>=sizeof(int64_t)){
    ops[OpKey(0xFC00003F,0x0000002C)]<<OpData("dadd"      , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::add<VR1R2,int64_t> >());
    ops[OpKey(0xFC00003F,0x0000002D)]<<OpData("daddu"     , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::add<VR1R2,int64_t> >());
    ops[OpKey(0xFC00003F,0x0000002E)]<<OpData("dsub"      , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sub<VR1R2,int64_t> >());
    ops[OpKey(0xFC00003F,0x0000002F)]<<OpData("dsubu"     , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sub<VR1R2,int64_t> >());
    ops[OpKey(0xFC000000,0x60000000)]<<OpData("daddi"     , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulAlu<fns::add<VR1I1,int64_t> >());
    ops[OpKey(0xFC000000,0x64000000)]<<OpData("daddiu"    , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulAlu<fns::add<VR1I1,int64_t> >());
    ops[OpKey(0xFC00003F,0x0000001C)]<<OpData("dmult(1)"  , CtlNMor , IntOpMul , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mul<VR1R2,int64_t>,Tregv_t> >())
				     <<OpData("dmult(2)"  , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mulhi<VR1R2,int64_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x0000001D)]<<OpData("dmultu(1)" , CtlNMor , IntOpMul , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mul<VR1R2,uint64_t>,Tregv_t> >())
				     <<OpData("dmultu(2)" , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mulhi<VR1R2,uint64_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x0000001E)]<<OpData("ddiv(1)"   , CtlNMor , IntOpDiv , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::div<VR1R2,int64_t>,Tregv_t> >())
				     <<OpData("ddiv(2)"   , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mod<VR1R2,int64_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x0000001F)]<<OpData("ddivu(1)"  , CtlNMor , IntOpDiv , ArgLo  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::div<VR1R2,uint64_t>,Tregv_t> >())
				     <<OpData("ddivu(2)"  , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::sxt<fns::mod<VR1R2,uint64_t>,Tregv_t> >());
    }
    // Moves
    ops[OpKey(0xFC1F003F,0x00000021)]<<OpData("movw"      , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgNo  , ImmNo  , emulAlu<fns::sxt<fns::ident<VR1  ,int32_t>,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000010)]<<OpData("mfhi"      , CtlNorm , IntOpALU , ArgRd  , ArgHi  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000012)]<<OpData("mflo"      , CtlNorm , IntOpALU , ArgRd  , ArgLo  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000011)]<<OpData("mthi"      , CtlNorm , IntOpALU , ArgHi  , ArgRs  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x00000013)]<<OpData("mtlo"      , CtlNorm , IntOpALU , ArgLo  , ArgRs  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,Tregv_t> >());
    ops[OpKey(0xFFE00000,0x44000000)]<<OpData("mfc1"      , CtlNorm , IntOpALU , ArgRt  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::sxt<fns::ident<VR1  ,int32_t>,Tregv_t> >());
    ops[OpKey(0xFFE00000,0x44200000)]<<OpData("dmfc1"     , CtlNorm , IntOpALU , ArgRt  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,int64_t> >());
    ops[OpKey(0xFFE00000,0x44400000)]<<OpData("cfc1"      , CtlNorm , IntOpALU , ArgRt  , ArgFCs , ArgNo  , ImmNo  , emulAlu<fns::sxt<fns::ident<VR1  ,int32_t>,Tregv_t> >());
    ops[OpKey(0xFFE00000,0x44800000)]<<OpData("mtc1"      , CtlNorm , IntOpALU , ArgFs  , ArgRt  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,int32_t> >());
    ops[OpKey(0xFFE00000,0x44A00000)]<<OpData("dmtc1"     , CtlNorm , IntOpALU , ArgFs  , ArgRt  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,int64_t> >());
    ops[OpKey(0xFFE00000,0x44C00000)]<<OpData("ctc1"      , CtlNorm , IntOpALU , ArgFCs , ArgRt  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,int32_t> >());
    ops[OpKey(0xFC00003F,0x7C00003B)]<<OpData("rdhwr"     , CtlNorm , IntOpALU , ArgRt  , ArgHs  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,Tregv_t> >());
    // Integer comparisons
    ops[OpKey(0xFC000000,0x28000000)]<<OpData("slti"      , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulAlu<fns::lt<VR1I1,Tsregv_t,Tregv_t> >());
    ops[OpKey(0xFC000000,0x2C000000)]<<OpData("sltiu"     , CtlNorm , IntOpALU , ArgRt  , ArgRs  , ArgNo  , ImmSExt, emulAlu<fns::lt<VR1I1,Turegv_t,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x0000002A)]<<OpData("slt"       , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::lt<VR1R2,Tsregv_t,Tregv_t> >());
    ops[OpKey(0xFC00003F,0x0000002B)]<<OpData("sltu"      , CtlNorm , IntOpALU , ArgRd  , ArgRs  , ArgRt  , ImmNo  , emulAlu<fns::lt<VR1R2,Turegv_t,Tregv_t> >());
    // Floating-point comparisons
    ops[OpKey(0xFFE0003F,0x46000030)]<<OpData("c.f.s"     , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, false, false, false> >());
    ops[OpKey(0xFFE0003F,0x46000031)]<<OpData("c.un.s"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, false, false, true > >());
    ops[OpKey(0xFFE0003F,0x46000032)]<<OpData("c.eq.s"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, false, true , false> >());
    ops[OpKey(0xFFE0003F,0x46000033)]<<OpData("c.ueq.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, false, true , true > >());
    ops[OpKey(0xFFE0003F,0x46000034)]<<OpData("c.olt.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, true , false, false> >());
    ops[OpKey(0xFFE0003F,0x46000035)]<<OpData("c.ult.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, true , false, true > >());
    ops[OpKey(0xFFE0003F,0x46000036)]<<OpData("c.ole.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, true , true , false> >());
    ops[OpKey(0xFFE0003F,0x46000037)]<<OpData("c.ule.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, false, true , true , true > >());
    ops[OpKey(0xFFE0003F,0x46000038)]<<OpData("c.sf.s"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , false, false, false> >());
    ops[OpKey(0xFFE0003F,0x46000039)]<<OpData("c.ngle.s"  , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , false, false, true > >());
    ops[OpKey(0xFFE0003F,0x4600003A)]<<OpData("c.seq.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , false, true , false> >());
    ops[OpKey(0xFFE0003F,0x4600003B)]<<OpData("c.ngl.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , false, true , true > >());
    ops[OpKey(0xFFE0003F,0x4600003C)]<<OpData("c.lt.s"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , true , false, false> >());
    ops[OpKey(0xFFE0003F,0x4600003D)]<<OpData("c.nge.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , true , false, true > >());
    ops[OpKey(0xFFE0003F,0x4600003E)]<<OpData("c.le.s"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , true , true , false> >());
    ops[OpKey(0xFFE0003F,0x4600003F)]<<OpData("c.ngt.s"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float32_t, true , true , true , true > >());
    ops[OpKey(0xFFE0003F,0x46200030)]<<OpData("c.f.d"     , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, false, false, false> >());
    ops[OpKey(0xFFE0003F,0x46200031)]<<OpData("c.un.d"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, false, false, true > >());
    ops[OpKey(0xFFE0003F,0x46200032)]<<OpData("c.eq.d"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, false, true , false> >());
    ops[OpKey(0xFFE0003F,0x46200033)]<<OpData("c.ueq.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, false, true , true > >());
    ops[OpKey(0xFFE0003F,0x46200034)]<<OpData("c.olt.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, true , false, false> >());
    ops[OpKey(0xFFE0003F,0x46200035)]<<OpData("c.ult.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, true , false, true > >());
    ops[OpKey(0xFFE0003F,0x46200036)]<<OpData("c.ole.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, true , true , false> >());
    ops[OpKey(0xFFE0003F,0x46200037)]<<OpData("c.ule.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, false, true , true , true > >());
    ops[OpKey(0xFFE0003F,0x46200038)]<<OpData("c.sf.d"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , false, false, false> >());
    ops[OpKey(0xFFE0003F,0x46200039)]<<OpData("c.ngle.d"  , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , false, false, true > >());
    ops[OpKey(0xFFE0003F,0x4620003A)]<<OpData("c.seq.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , false, true , false> >());
    ops[OpKey(0xFFE0003F,0x4620003B)]<<OpData("c.ngl.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , false, true , true > >());
    ops[OpKey(0xFFE0003F,0x4620003C)]<<OpData("c.lt.d"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , true , false, false> >());
    ops[OpKey(0xFFE0003F,0x4620003D)]<<OpData("c.nge.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , true , false, true > >());
    ops[OpKey(0xFFE0003F,0x4620003E)]<<OpData("c.le.d"    , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , true , true , false> >());
    ops[OpKey(0xFFE0003F,0x4620003F)]<<OpData("c.ngt.d"   , CtlNorm , FpOpALU  , ArgFccc, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::fcmp<VR1R2,float64_t, true , true , true , true > >());
    // Floating-point arithmetic
    ops[OpKey(0xFFE0003F,0x46000006)]<<OpData("mov.s"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000004)]<<OpData("sqrt.s"    , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::sqroot<VR1  ,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000005)]<<OpData("abs.s"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::abs<VR1  ,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000007)]<<OpData("neg.s"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::neg<VR1  ,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000015)]<<OpData("recip.s"   , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::recip<VR1  ,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000016)]<<OpData("rsqrt.s"   , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::rsqroot<VR1  ,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46200006)]<<OpData("mov.d"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ident<VR1  ,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200004)]<<OpData("sqrt.d"    , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::sqroot<VR1  ,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200005)]<<OpData("abs.d"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::abs<VR1  ,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200007)]<<OpData("neg.d"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::neg<VR1  ,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200015)]<<OpData("recip.d"   , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::recip<VR1  ,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200016)]<<OpData("rsqrt.d"   , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::rsqroot<VR1  ,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46000000)]<<OpData("add.s"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::add<VR1R2,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000001)]<<OpData("sub.s"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::sub<VR1R2,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000002)]<<OpData("mul.s"     , CtlNorm , FpOpMul  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46000003)]<<OpData("div.s"     , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::div<VR1R2,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46200000)]<<OpData("add.d"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::add<VR1R2,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200001)]<<OpData("sub.d"     , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::sub<VR1R2,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200002)]<<OpData("mul.d"     , CtlNorm , FpOpMul  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46200003)]<<OpData("div.d"     , CtlNorm , FpOpDiv  , ArgFd  , ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::div<VR1R2,float64_t> >());
    // These are two-part FP arithmetic ops: multiply, then add/subtract
    ops[OpKey(0xFC00003F,0x4C000020)]<<OpData("madd.s(1)" , CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float32_t> >())
				     <<OpData("madd.s(2)" , CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::add<VR1R2,float32_t> >());
    ops[OpKey(0xFC00003F,0x4C000021)]<<OpData("madd.d(1)" , CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float64_t> >())
				     <<OpData("madd.d(2)" , CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::add<VR1R2,float64_t> >());
    ops[OpKey(0xFC00003F,0x4C000028)]<<OpData("msub.s(1)" , CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float32_t> >())
				     <<OpData("msub.s(2)" , CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::sub<VR1R2,float32_t> >());
    ops[OpKey(0xFC00003F,0x4C000029)]<<OpData("msub.d(1)" , CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float64_t> >())
				     <<OpData("msub.d(2)" , CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::sub<VR1R2,float64_t> >());
    ops[OpKey(0xFC00003F,0x4C000030)]<<OpData("nmadd.s(1)", CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float32_t> >())
				     <<OpData("nmadd.s(2)", CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::nadd<VR1R2,float32_t> >());
    ops[OpKey(0xFC00003F,0x4C000031)]<<OpData("nmadd.d(1)", CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float64_t> >())
				     <<OpData("nmadd.d(2)", CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::nadd<VR1R2,float64_t> >());
    ops[OpKey(0xFC00003F,0x4C000038)]<<OpData("nmsub.s(1)", CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float32_t> >())
				     <<OpData("nmsub.s(2)", CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::nsub<VR1R2,float32_t> >());
    ops[OpKey(0xFC00003F,0x4C000039)]<<OpData("nmsub.d(1)", CtlNMor , FpOpMul  , ArgFTmp, ArgFs  , ArgFt  , ImmNo  , emulAlu<fns::mul<VR1R2,float64_t> >())
				     <<OpData("nmsub.d(2)", CtlNorm , FpOpALU  , ArgFd  , ArgFTmp, ArgFr  , ImmNo  , emulAlu<fns::nsub<VR1R2,float64_t> >());
    // Floating-point format conversion
    ops[OpKey(0xFFE0003F,0x46000008)]<<OpData("round.l.s" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fround<VR1,float32_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x46000009)]<<OpData("trunc.l.s" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ftrunc<VR1,float32_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x4600000A)]<<OpData("ceil.l.s"  , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fceil <VR1,float32_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x4600000B)]<<OpData("floor.l.s" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ffloor<VR1,float32_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x4600000C)]<<OpData("round.w.s" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fround<VR1,float32_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x4600000D)]<<OpData("trunc.w.s" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ftrunc<VR1,float32_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x4600000E)]<<OpData("ceil.w.s"  , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fceil <VR1,float32_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x4600000F)]<<OpData("floor.w.s" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ffloor<VR1,float32_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x46200008)]<<OpData("round.l.d" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fround<VR1,float64_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x46200009)]<<OpData("trunc.l.d" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ftrunc<VR1,float64_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x4620000A)]<<OpData("ceil.l.d"  , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fceil <VR1,float64_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x4620000B)]<<OpData("floor.l.d" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ffloor<VR1,float64_t,int64_t> >());
    ops[OpKey(0xFFE0003F,0x4620000C)]<<OpData("round.w.d" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fround<VR1,float64_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x4620000D)]<<OpData("trunc.w.d" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ftrunc<VR1,float64_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x4620000E)]<<OpData("ceil.w.d"  , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::fceil <VR1,float64_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x4620000F)]<<OpData("floor.w.d" , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgNo  , ImmNo  , emulAlu<fns::ffloor<VR1,float64_t,int32_t> >());
    ops[OpKey(0xFFE0003F,0x46000021)]<<OpData("cvt.d.s"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,float32_t,float64_t> >());
    ops[OpKey(0xFFE0003F,0x46000024)]<<OpData("cvt.w.s"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,float32_t,int32_t  > >());
    ops[OpKey(0xFFE0003F,0x46000025)]<<OpData("cvt.l.s"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,float32_t,int64_t  > >());
    ops[OpKey(0xFFE0003F,0x46200020)]<<OpData("cvt.s.d"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,float64_t,float32_t> >());
    ops[OpKey(0xFFE0003F,0x46200024)]<<OpData("cvt.w.d"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,float64_t,int32_t  > >());
    ops[OpKey(0xFFE0003F,0x46200025)]<<OpData("cvt.l.d"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,float64_t,int64_t  > >());
    ops[OpKey(0xFFE0003F,0x46800020)]<<OpData("cvt.s.w"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,int32_t,  float32_t> >());
    ops[OpKey(0xFFE0003F,0x46800021)]<<OpData("cvt.d.w"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,int32_t,  float64_t> >());
    ops[OpKey(0xFFE0003F,0x46A00020)]<<OpData("cvt.s.l"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,int64_t,  float32_t> >());
    ops[OpKey(0xFFE0003F,0x46A00021)]<<OpData("cvt.d.l"   , CtlNorm , FpOpALU  , ArgFd  , ArgFs  , ArgRM  , ImmNo  , emulAlu<mips_fcvt<VR1R2,int64_t,  float64_t> >());

    ops[OpKey(0xFC00F83F,0x00000000)]<<OpData("nop"       , CtlNorm , TypNop   , ArgNo  , ArgNo  , ArgNo  , ImmNo  , emulNop);

    build(ops,OpKey(0,0));
  }

} // End of namespace Mips

typedef void HandlerFunc(InstDesc *inst, ThreadContext *context);

template<HandlerFunc f>
InstDesc *WrapHandler(InstDesc *inst, ThreadContext *context){
  f(inst,context);
  context->updIDesc(1);
  return (*(inst+1))(context);
}


#if (defined INTERCEPT_HEAP_CALLS)
void handleMallocCall(InstDesc *inst, ThreadContext *context){
  uint32_t siz=Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegA0));
  printf("Malloc call with size=0x%08x\n",siz);
}
void handleMallocRet(InstDesc *inst, ThreadContext *context){
  uint32_t addr=Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegV0));
  printf("Malloc ret  with addr=0x%08x\n",addr);
}
void handleCallocCall(InstDesc *inst, ThreadContext *context){
  uint32_t nmemb=Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegA0));
  uint32_t siz  =Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegA1));
  printf("Calloc call with nmemb=0x%08x size=0x%08x\n",nmemb,siz);
}
void handleCallocRet(InstDesc *inst, ThreadContext *context){
  uint32_t addr=Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegV0));
  printf("Calloc ret  with addr=0x%08x\n",addr);
}
void handleReallocCall(InstDesc *inst, ThreadContext *context){
  uint32_t addr=Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegA0));
  uint32_t siz =Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegA1));
  printf("Realloc call with addr=0x%08x size=0x%08x\n",addr,siz);
}
void handleReallocRet(InstDesc *inst, ThreadContext *context){
  uint32_t addr=Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegV0));
  printf("Realloc ret  with addr=0x%08x\n",addr);
}
void handleFreeCall(InstDesc *inst, ThreadContext *context){
  uint32_t addr=Mips::getRegAny<ExecModeMips32,uint32_t,RegTypeGpr>(context,static_cast<RegName>(Mips::RegA0));
  printf("Free call with addr=0x%08x\n",addr);
}
#endif

#if (defined DEBUG_BENCH)
void handleEveryCall(InstDesc *inst, ThreadContext *context){
  I(context->getIAddr()==context->getAddressSpace()->getFuncAddr(context->getIAddr()));
  context->execCall(context->getIAddr(),
		    ArchDefs<ExecModeMips32>::getReg<uint32_t,RegTypeGpr>(context,ArchDefs<ExecModeMips32>::RegRA),
		    ArchDefs<ExecModeMips32>::getReg<uint32_t,RegTypeGpr>(context,ArchDefs<ExecModeMips32>::RegSP)
		    );
}
void handleEveryRet(InstDesc *inst, ThreadContext *context){
  context->execRet(context->getAddressSpace()->getFuncAddr(context->getIAddr()),
		    ArchDefs<ExecModeMips32>::getReg<uint32_t,RegTypeGpr>(context,ArchDefs<ExecModeMips32>::RegRA),
		    ArchDefs<ExecModeMips32>::getReg<uint32_t,RegTypeGpr>(context,ArchDefs<ExecModeMips32>::RegSP)
		   );
}
#endif

#include "X86InstDesc.h"

template<ExecMode mode>
bool decodeInstSize(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, size_t &tsize, bool domap){
  switch(mode&ExecModeArchMask){
  case ExecModeArchMips:
  case ExecModeArchMips64:
    return Mips::DecodeInst<mode>::decodeInstSize(context,funcAddr,curAddr,endAddr,tsize,domap);
  }
  fail("decodeInstSize<0x%x> called with unsupported mode\n");
}
template<ExecMode mode>
void decodeInst(ThreadContext *context, VAddr funcAddr, VAddr &curAddr, VAddr endAddr, InstDesc *&trace, bool domap){
  switch(mode&ExecModeArchMask){
  case ExecModeArchMips:
  case ExecModeArchMips64:
    return Mips::DecodeInst<mode>::decodeInst(context,funcAddr,curAddr,endAddr,trace,domap);
  }
  fail("decodeInst<0x%x> called with unsupported mode\n");
}

template<ExecMode mode>
void decodeTrace(ThreadContext *context, VAddr addr, size_t len){
  VAddr funcAddr=context->getAddressSpace()->getFuncAddr(addr);
  I((!funcAddr)||(addr+len<=funcAddr+context->getAddressSpace()->getFuncSize(funcAddr)));
  VAddr endAddr=len?(addr+len):0;
  size_t tsize=0;
  VAddr sizaddr=addr;
  if(!endAddr){
    I(0);
    while(decodeInstSize<mode>(context,funcAddr,sizaddr,endAddr,tsize,true));
    endAddr=sizaddr;
    sizaddr=addr;
    tsize=0;
  }
  while(sizaddr<endAddr)
    decodeInstSize<mode>(context,funcAddr,sizaddr,endAddr,tsize,true);
  context->getAddressSpace()->delInsts(addr,endAddr);
  InstDesc *trace=new InstDesc[tsize];
  InstDesc *curtrace=trace;
  VAddr trcaddr=addr;
  while(trcaddr<endAddr)
    decodeInst<mode>(context,funcAddr,trcaddr,endAddr,curtrace,true);
  I((ssize_t)tsize==(curtrace-trace));
  I(trcaddr==sizaddr);
  context->getAddressSpace()->mapTrace(trace,curtrace,addr,trcaddr);
}

void decodeTrace(ThreadContext *context, VAddr addr, size_t len){
  static bool didThis=false;
  if(!didThis){
#if (defined INTERCEPT_HEAP_CALLS)
    // This should not be here, but it's added for testing
    AddressSpace::addCallHandler("malloc",WrapHandler<handleMallocCall>);
    AddressSpace::addRetHandler("malloc",WrapHandler<handleMallocRet>);
    AddressSpace::addCallHandler("calloc",WrapHandler<handleCallocCall>);
    AddressSpace::addRetHandler("calloc",WrapHandler<handleCallocRet>);
    AddressSpace::addCallHandler("realloc",WrapHandler<handleReallocCall>);
    AddressSpace::addRetHandler("realloc",WrapHandler<handleReallocRet>);
    AddressSpace::addCallHandler("free",WrapHandler<handleFreeCall>);
#endif
#if (defined DEBUG_BENCH)
    AddressSpace::addCallHandler("",WrapHandler<handleEveryCall>);
    AddressSpace::addRetHandler("",WrapHandler<handleEveryRet>);
#endif
    didThis=true;
  }
  switch(context->getMode()){
#if (defined SUPPORT_MIPS32)
  case ExecModeMips32:   return decodeTrace<ExecModeMips32>(context,addr,len);
#endif
#if (defined SUPPORT_MIPSEL32)
  case ExecModeMipsel32: return decodeTrace<ExecModeMipsel32>(context,addr,len);
#endif
#if (defined SUPPORT_MIPS64)
  case ExecModeMips64:   return decodeTrace<ExecModeMips64>(context,addr,len);
#endif
#if (defined SUPPORT_MIPSEL64)
  case ExecModeMipsel64: return decodeTrace<ExecModeMipsel64>(context,addr,len);
#endif
  default:
    fail("decodeTrace called in unsupported CPU mode\n");
  }
}
