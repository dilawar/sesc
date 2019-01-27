#include "LinuxSys.h"
#include "ThreadContext.h"
// Get declaration of fail()
#include "EmulInit.h"
#include "OSSim.h"
#include "ElfObject.h"

//#include "MipsRegs.h"

#include "ABIDefs.h"

#include "ArchDefs.h"

// Get definition of endian conversions
#include "EndianDefs.h"

#include <map>

// Get definitions of native file operations and flags
#include <fcntl.h>
// Get definition of native types
#include <linux/types.h>
// Get definition of native getdirentries(), dirent64, etc.
#include <dirent.h>
// Get native ioctl() and data type definitions for it
#include <sys/ioctl.h>
// Get error number and errno definitions
#include <errno.h>

//#define DEBUG_FILES
//#define DEBUG_VMEM
//#define DEBUG_SOCKET
//#define DEBUG_SYSCALLS

template<ExecMode mode>
class RealLinuxSys : public LinuxSys, public ArchDefs<mode>{
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
    return EndianDefs<mode>::fixEndian(val);
  }
  static const ExecMode execMode=mode;
  typedef ABIDefs<mode>    Base;
  typedef ElfDefs<mode>    ElfBase;
  // Basic types
  typedef typename Base::Tint       Tint;
  typedef typename Base::Tuint      Tuint;
  typedef typename Base::Tlong      Tlong;
  typedef typename Base::Tulong     Tulong;
  typedef typename Base::Tpointer_t Tpointer_t;
  typedef typename Base::Tsize_t    Tsize_t;
  typedef typename Base::Tssize_t   Tssize_t;
  typedef typename Base::Tpid_t     Tpid_t;
  class Tstr{
   private:
    char *s;
   public:
    Tstr(ThreadContext *context, Tpointer_t addr) : s(0){
      ssize_t len=context->readMemString(addr,0,0);
      if(len==-1){
        setSysErr(context,VEFAULT);
      }else{
        s=new char[len];
        ssize_t chklen=context->readMemString(addr,len,s);
        I(chklen==len);
      }
    }
    ~Tstr(void){
      if(s!=0)
        delete [] s;
    }
    operator const char *() const{
      return s;
    }
    operator bool() const{
      return (s!=0);
    }
  };
  // Error codes
  const static typeof(Base::VEPERM       ) VEPERM        = Base::VEPERM;
  const static typeof(Base::VENOENT      ) VENOENT       = Base::VENOENT;
  const static typeof(Base::VESRCH       ) VESRCH        = Base::VESRCH;
  const static typeof(Base::VEINTR       ) VEINTR        = Base::VEINTR;
  const static typeof(Base::VEIO         ) VEIO          = Base::VEIO;
  const static typeof(Base::VENXIO       ) VENXIO        = Base::VENXIO;
  const static typeof(Base::VE2BIG       ) VE2BIG        = Base::VE2BIG;
  const static typeof(Base::VENOEXEC     ) VENOEXEC      = Base::VENOEXEC;
  const static typeof(Base::VEBADF       ) VEBADF        = Base::VEBADF;
  const static typeof(Base::VECHILD      ) VECHILD       = Base::VECHILD;
  const static typeof(Base::VEAGAIN      ) VEAGAIN       = Base::VEAGAIN;
  const static typeof(Base::VENOMEM      ) VENOMEM       = Base::VENOMEM;
  const static typeof(Base::VEACCES      ) VEACCES       = Base::VEACCES;
  const static typeof(Base::VEFAULT      ) VEFAULT       = Base::VEFAULT;
  const static typeof(Base::VENOTBLK     ) VENOTBLK      = Base::VENOTBLK;
  const static typeof(Base::VEBUSY       ) VEBUSY        = Base::VEBUSY;
  const static typeof(Base::VEEXIST      ) VEEXIST       = Base::VEEXIST;
  const static typeof(Base::VEXDEV       ) VEXDEV        = Base::VEXDEV;
  const static typeof(Base::VENODEV      ) VENODEV       = Base::VENODEV;
  const static typeof(Base::VENOTDIR     ) VENOTDIR      = Base::VENOTDIR;
  const static typeof(Base::VEISDIR      ) VEISDIR       = Base::VEISDIR;
  const static typeof(Base::VEINVAL      ) VEINVAL       = Base::VEINVAL;
  const static typeof(Base::VENFILE      ) VENFILE       = Base::VENFILE;
  const static typeof(Base::VEMFILE      ) VEMFILE       = Base::VEMFILE;
  const static typeof(Base::VENOTTY      ) VENOTTY       = Base::VENOTTY;
  const static typeof(Base::VETXTBSY     ) VETXTBSY      = Base::VETXTBSY;
  const static typeof(Base::VEFBIG       ) VEFBIG        = Base::VEFBIG;
  const static typeof(Base::VENOSPC      ) VENOSPC       = Base::VENOSPC;
  const static typeof(Base::VESPIPE      ) VESPIPE       = Base::VESPIPE;
  const static typeof(Base::VEROFS       ) VEROFS        = Base::VEROFS;
  const static typeof(Base::VEMLINK      ) VEMLINK       = Base::VEMLINK;
  const static typeof(Base::VEPIPE       ) VEPIPE        = Base::VEPIPE;
  const static typeof(Base::VEDOM        ) VEDOM         = Base::VEDOM;
  const static typeof(Base::VERANGE      ) VERANGE       = Base::VERANGE;
  const static typeof(Base::VENOSYS      ) VENOSYS       = Base::VENOSYS;
  const static typeof(Base::VELOOP       ) VELOOP        = Base::VELOOP;
  const static typeof(Base::VEAFNOSUPPORT) VEAFNOSUPPORT = Base::VEAFNOSUPPORT;
  static Tint errorFromNative(int err=errno){
    switch(err){
    case EPERM:        return VEPERM;
    case ENOENT:       return VENOENT;
    case ESRCH:        return VESRCH;
    case EINTR:        return VEINTR;
    case EIO:          return VEIO;
    case ENXIO:        return VENXIO;
    case E2BIG:        return VE2BIG;
    case ENOEXEC:      return VENOEXEC;
    case EBADF:        return VEBADF;
    case ECHILD:       return VECHILD;
    case EAGAIN:       return VEAGAIN;
    case ENOMEM:       return VENOMEM;
    case EACCES:       return VEACCES;
    case EFAULT:       return VEFAULT;
    case ENOTBLK:      return VENOTBLK;
    case EBUSY:        return VEBUSY;
    case EEXIST:       return VEEXIST;
    case EXDEV:        return VEXDEV;
    case ENODEV:       return VENODEV;
    case ENOTDIR:      return VENOTDIR;
    case EISDIR:       return VEISDIR;
    case EINVAL:       return VEINVAL;
    case ENFILE:       return VENFILE;
    case EMFILE:       return VEMFILE;
    case ENOTTY:       return VENOTTY;
    case ETXTBSY:      return VETXTBSY;
    case EFBIG:        return VEFBIG;
    case ENOSPC:       return VENOSPC;
    case ESPIPE:       return VESPIPE;
    case EROFS:        return VEROFS;
    case EMLINK:       return VEMLINK;
    case EPIPE:        return VEPIPE;
    case EDOM :        return VEDOM;
    case ERANGE:       return VERANGE;
    case ENOSYS:       return VENOSYS;
    case EAFNOSUPPORT: return VEAFNOSUPPORT;
    default:
      fail("errorFromNative(%d) with unsupported native error code\n");
    }
  }
  // Signal numbers
  const static typeof(Base::VSIGHUP ) VSIGHUP  = Base::VSIGHUP;
  const static typeof(Base::VSIGINT ) VSIGINT  = Base::VSIGINT;
  const static typeof(Base::VSIGQUIT) VSIGQUIT = Base::VSIGQUIT;
  const static typeof(Base::VSIGILL ) VSIGILL  = Base::VSIGILL;
  const static typeof(Base::VSIGTRAP) VSIGTRAP = Base::VSIGTRAP;
  const static typeof(Base::VSIGABRT) VSIGABRT = Base::VSIGABRT;
  const static typeof(Base::VSIGFPE ) VSIGFPE  = Base::VSIGFPE;
  const static typeof(Base::VSIGKILL) VSIGKILL = Base::VSIGKILL;
  const static typeof(Base::VSIGBUS ) VSIGBUS  = Base::VSIGBUS;
  const static typeof(Base::VSIGSEGV) VSIGSEGV = Base::VSIGSEGV;
  const static typeof(Base::VSIGPIPE) VSIGPIPE = Base::VSIGPIPE;
  const static typeof(Base::VSIGALRM) VSIGALRM = Base::VSIGALRM;
  const static typeof(Base::VSIGTERM) VSIGTERM = Base::VSIGTERM;
  const static typeof(Base::VSIGUSR1) VSIGUSR1 = Base::VSIGUSR1;
  const static typeof(Base::VSIGUSR2) VSIGUSR2 = Base::VSIGUSR2;
  const static typeof(Base::VSIGCHLD) VSIGCHLD = Base::VSIGCHLD;
  const static typeof(Base::VSIGSTOP) VSIGSTOP = Base::VSIGSTOP;
  const static typeof(Base::VSIGTSTP) VSIGTSTP = Base::VSIGTSTP;
  const static typeof(Base::VSIGCONT) VSIGCONT = Base::VSIGCONT;
  static SignalID sigNumToLocal(Tint sig){
    switch(sig){
    case VSIGCHLD: return SigChld;
    default:
      if(sig<=SigNMax)
	return static_cast<SignalID>(sig);
    }
    fail("RealLinuxSys::sigNumToLocal(%d) not supported\n",sig);
    return SigNone;
  }
  static Tint localToSigNum(SignalID sig){
    switch(sig){
    case SigChld: return VSIGCHLD;
    default:
      return static_cast<Tint>(sig);
    }
  }
  // Thread/process creation and destruction
  class Tauxv_t{
    typedef typename ElfBase::Tauxv_t This;
  public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_a_type a_type;
    typename This::Type_a_val  a_val;
    Tauxv_t(typename This::Type_a_type a_type, typename This::Type_a_val a_val)
      : a_type(a_type), a_val(a_val)
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_a_type,fixEndian(a_type));
      context->writeMemRaw(addr+This::Offs_a_val,fixEndian(a_val));
    }
  };
  const static typeof(Base::VCSIGNAL) VCSIGNAL = Base::VCSIGNAL;
  const static typeof(Base::VCLONE_VM) VCLONE_VM = Base::VCLONE_VM;
  const static typeof(Base::VCLONE_FS) VCLONE_FS = Base::VCLONE_FS;
  const static typeof(Base::VCLONE_FILES) VCLONE_FILES = Base::VCLONE_FILES;
  const static typeof(Base::VCLONE_SIGHAND) VCLONE_SIGHAND = Base::VCLONE_SIGHAND;
  const static typeof(Base::VCLONE_VFORK) VCLONE_VFORK = Base::VCLONE_VFORK;
  const static typeof(Base::VCLONE_PARENT) VCLONE_PARENT = Base::VCLONE_PARENT;
  const static typeof(Base::VCLONE_THREAD) VCLONE_THREAD = Base::VCLONE_THREAD;
  const static typeof(Base::VCLONE_NEWNS) VCLONE_NEWNS = Base::VCLONE_NEWNS;
  const static typeof(Base::VCLONE_SYSVSEM) VCLONE_SYSVSEM = Base::VCLONE_SYSVSEM;
  const static typeof(Base::VCLONE_SETTLS) VCLONE_SETTLS = Base::VCLONE_SETTLS;
  const static typeof(Base::VCLONE_PARENT_SETTID) VCLONE_PARENT_SETTID = Base::VCLONE_PARENT_SETTID;
  const static typeof(Base::VCLONE_CHILD_CLEARTID) VCLONE_CHILD_CLEARTID = Base::VCLONE_CHILD_CLEARTID;
  const static typeof(Base::VCLONE_DETACHED) VCLONE_DETACHED = Base::VCLONE_DETACHED;
  const static typeof(Base::VCLONE_UNTRACED) VCLONE_UNTRACED = Base::VCLONE_UNTRACED;
  const static typeof(Base::VCLONE_CHILD_SETTID) VCLONE_CHILD_SETTID = Base::VCLONE_CHILD_SETTID;
  const static typeof(Base::VCLONE_STOPPED) VCLONE_STOPPED = Base::VCLONE_STOPPED;
  const static typeof(Base::V__NR_clone) V__NR_clone = Base::V__NR_clone;
  void sysClone(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_fork) V__NR_fork = Base::V__NR_fork;
  void sysFork(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_execve) V__NR_execve = Base::V__NR_execve;
  InstDesc *sysExecVe(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_exit) V__NR_exit = Base::V__NR_exit;
  InstDesc *sysExit(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_exit_group) V__NR_exit_group = Base::V__NR_exit_group;
  InstDesc *sysExitGroup(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::VWNOHANG  ) VWNOHANG   = Base::VWNOHANG;
  const static typeof(Base::VWUNTRACED) VWUNTRACED = Base::VWUNTRACED;
  void doWait4(ThreadContext *context, InstDesc *inst,
               Tpid_t cpid, Tpointer_t status,
               Tuint options, Tpointer_t rusage);
  const static typeof(Base::V__NR_waitpid) V__NR_waitpid = Base::V__NR_waitpid;
  void sysWaitpid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_wait4) V__NR_wait4 = Base::V__NR_wait4;
  void sysWait4(ThreadContext *context, InstDesc *inst);
  // Thread/Process relationships functionality
  const static typeof(Base::V__NR_getpid) V__NR_getpid = Base::V__NR_getpid;
  void sysGetPid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_gettid) V__NR_gettid = Base::V__NR_gettid;
  void sysGetTid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_getppid) V__NR_getppid = Base::V__NR_getppid;
  void sysGetPPid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_setpgid) V__NR_setpgid = Base::V__NR_setpgid;
  const static typeof(Base::V__NR_getpgid) V__NR_getpgid = Base::V__NR_getpgid;
  void sysGetPGid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_getpgrp) V__NR_getpgrp = Base::V__NR_getpgrp;
  void sysGetPGrp(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_setsid) V__NR_setsid = Base::V__NR_setsid;
  const static typeof(Base::V__NR_getsid) V__NR_getsid = Base::V__NR_getsid;
  // Futex functionality
  const static typeof(Base::VFUTEX_PRIVATE_FLAG) VFUTEX_PRIVATE_FLAG = Base::VFUTEX_PRIVATE_FLAG;
  const static typeof(Base::VFUTEX_CLOCK_REALTIME) VFUTEX_CLOCK_REALTIME = Base::VFUTEX_CLOCK_REALTIME;
  const static typeof(Base::VFUTEX_CMD_MASK) VFUTEX_CMD_MASK = Base::VFUTEX_CMD_MASK;
  const static typeof(Base::VFUTEX_WAIT) VFUTEX_WAIT = Base::VFUTEX_WAIT;
  const static typeof(Base::VFUTEX_WAKE) VFUTEX_WAKE = Base::VFUTEX_WAKE;
  const static typeof(Base::VFUTEX_FD) VFUTEX_FD = Base::VFUTEX_FD;
  const static typeof(Base::VFUTEX_REQUEUE) VFUTEX_REQUEUE = Base::VFUTEX_REQUEUE;
  const static typeof(Base::VFUTEX_CMP_REQUEUE) VFUTEX_CMP_REQUEUE = Base::VFUTEX_CMP_REQUEUE;
  const static typeof(Base::VFUTEX_WAKE_OP) VFUTEX_WAKE_OP = Base::VFUTEX_WAKE_OP;
  const static typeof(Base::VFUTEX_LOCK_PI) VFUTEX_LOCK_PI = Base::VFUTEX_LOCK_PI;
  const static typeof(Base::VFUTEX_UNLOCK_PI) VFUTEX_UNLOCK_PI = Base::VFUTEX_UNLOCK_PI;
  const static typeof(Base::VFUTEX_TRYLOCK_PI) VFUTEX_TRYLOCK_PI = Base::VFUTEX_TRYLOCK_PI;
  const static typeof(Base::VFUTEX_WAIT_BITSET) VFUTEX_WAIT_BITSET = Base::VFUTEX_WAIT_BITSET;
  const static typeof(Base::VFUTEX_WAKE_BITSET) VFUTEX_WAKE_BITSET = Base::VFUTEX_WAKE_BITSET;
  const static typeof(Base::VFUTEX_BITSET_MATCH_ANY) VFUTEX_BITSET_MATCH_ANY = Base::VFUTEX_BITSET_MATCH_ANY;
  const static typeof(Base::VFUTEX_OP_OPARG_SHIFT) VFUTEX_OP_OPARG_SHIFT = Base::VFUTEX_OP_OPARG_SHIFT;
  const static typeof(Base::VFUTEX_OP_SET) VFUTEX_OP_SET = Base::VFUTEX_OP_SET;
  const static typeof(Base::VFUTEX_OP_ADD) VFUTEX_OP_ADD = Base::VFUTEX_OP_ADD;
  const static typeof(Base::VFUTEX_OP_OR) VFUTEX_OP_OR = Base::VFUTEX_OP_OR;
  const static typeof(Base::VFUTEX_OP_ANDN) VFUTEX_OP_ANDN = Base::VFUTEX_OP_ANDN;
  const static typeof(Base::VFUTEX_OP_XOR) VFUTEX_OP_XOR = Base::VFUTEX_OP_XOR;
  const static typeof(Base::VFUTEX_OP_CMP_EQ) VFUTEX_OP_CMP_EQ = Base::VFUTEX_OP_CMP_EQ;
  const static typeof(Base::VFUTEX_OP_CMP_NE) VFUTEX_OP_CMP_NE = Base::VFUTEX_OP_CMP_NE;
  const static typeof(Base::VFUTEX_OP_CMP_LT) VFUTEX_OP_CMP_LT = Base::VFUTEX_OP_CMP_LT;
  const static typeof(Base::VFUTEX_OP_CMP_LE) VFUTEX_OP_CMP_LE = Base::VFUTEX_OP_CMP_LE;
  const static typeof(Base::VFUTEX_OP_CMP_GT) VFUTEX_OP_CMP_GT = Base::VFUTEX_OP_CMP_GT;
  const static typeof(Base::VFUTEX_OP_CMP_GE) VFUTEX_OP_CMP_GE = Base::VFUTEX_OP_CMP_GE;
  const static typeof(Base::V__NR_futex) V__NR_futex = Base::V__NR_futex;
  bool futexCheck(ThreadContext *context, Tpointer_t futex, Tint val);
  void futexWait(ThreadContext *context, Tpointer_t futex);
  int futexWake(ThreadContext *context, Tpointer_t futex, int nr_wake);
  int futexMove(ThreadContext *context, Tpointer_t srcFutex, Tpointer_t dstFutex, int nr_move);
  void sysFutex(ThreadContext *context, InstDesc *inst);
  // Robust list functionality
  class Trobust_list{
    typedef typename Base::Trobust_list This;
  public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_next next;
    Trobust_list(ThreadContext *context, VAddr addr)
      :
      next(fixEndian(context->readMemRaw<typeof(next)>(addr+This::Offs_next)))
    {
    }
  };
  class Trobust_list_head{
    typedef typename Base::Trobust_list_head This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    Trobust_list                        list;
    typename This::Type_futex_offset    futex_offset;
    typename This::Type_list_op_pending list_op_pending;
    Trobust_list_head(ThreadContext *context, VAddr addr)
      :
      list(context,addr+This::Offs_list),
      futex_offset(fixEndian(context->readMemRaw<typeof(futex_offset)>(addr+This::Offs_futex_offset))),
      list_op_pending(fixEndian(context->readMemRaw<typeof(list_op_pending)>(addr+This::Offs_list_op_pending)))
    {
    }
  };
  const static typeof(Base::V__NR_set_robust_list) V__NR_set_robust_list = Base::V__NR_set_robust_list;
  void sysSetRobustList(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_get_robust_list) V__NR_get_robust_list = Base::V__NR_get_robust_list;
  const static typeof(Base::V__NR_set_tid_address) V__NR_set_tid_address = Base::V__NR_set_tid_address;
  void sysSetTidAddress(ThreadContext *context, InstDesc *inst);
  // Signal-related functionality
  const static typeof(Base::V__NR_kill) V__NR_kill = Base::V__NR_kill;
  void sysKill(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_tkill) V__NR_tkill = Base::V__NR_tkill;
  void sysTKill(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_tgkill) V__NR_tgkill = Base::V__NR_tgkill;
  void sysTgKill(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_rt_sigqueueinfo) V__NR_rt_sigqueueinfo = Base::V__NR_rt_sigqueueinfo;
  class Tsigset_t{
    typedef typename Base::Tsigset_t This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_sig sig[This::Size_sig];
    Tsigset_t(ThreadContext *context, VAddr addr){
      for(size_t i=0;i<This::Size_sig;i++)
        sig[i]=fixEndian(context->readMemRaw<typename This::Type_sig>(addr+This::Offs_sig+i*This::Step_sig));
    }
    Tsigset_t(const SignalSet &sset){
      for(size_t i=0;i<This::Size_sig;i++){
        sig[i]=0;
        for(size_t b=0;b<8*sizeof(typename This::Type_sig);b++){
          SignalID lsig=sigNumToLocal(Tint(i*8*sizeof(typename This::Type_sig)+b+1));
          if((lsig!=SigNone)&&(sset.test(lsig)))
            sig[i]|=(1<<b);
        }
      }
    }
    void write(ThreadContext *context, VAddr addr) const{
      for(size_t i=0;i<This::Size_sig;i++)
        context->writeMemRaw(addr+This::Offs_sig+i*This::Step_sig,fixEndian(sig[i]));
    }
    operator SignalSet(){
      SignalSet rv(0);
      for(size_t i=0;i<This::Size_sig;i++){
        for(size_t b=0;b<8*sizeof(typename This::Type_sig);b++){
          if(sig[i]&(1<<b)){
	    SignalID lsig=sigNumToLocal(Tint(i*8*sizeof(typename This::Type_sig)+b+1));
            if(lsig!=SigNone)
              rv.set(lsig);
          }
        }
      }
      return rv;
    } 
  };
  class Tsigaction{
    typedef typename Base::Tsigaction This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    // Can't call this sa_handler because of conflict with a native define 
    typename This::Type_sa_handler sa_hand;
    typename This::Type_sa_flags   sa_flags;
    Tsigset_t                      sa_mask;
    Tsigaction(ThreadContext *context, VAddr addr)
      :
      sa_hand(fixEndian(context->readMemRaw<typeof(sa_hand)>(addr+This::Offs_sa_handler))),
      sa_flags(fixEndian(context->readMemRaw<typeof(sa_flags)>(addr+This::Offs_sa_flags))),
      sa_mask(context,addr+This::Offs_sa_mask)
    {
    }
    Tsigaction(SignalDesc &sdesc)
      :
      sa_hand(saHandlerFromLocal(sdesc.handler)),
      sa_flags(saFlagsFromLocal(sdesc.flags)),
      sa_mask(sdesc.mask)
    {
    }
    operator SignalDesc(){
      return SignalDesc(saHandlerToLocal(sa_hand),sa_mask,saFlagsToLocal(sa_flags));
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_sa_handler,fixEndian(sa_hand));
      context->writeMemRaw(addr+This::Offs_sa_flags,fixEndian(sa_flags));
      sa_mask.write(context,addr+This::Offs_sa_mask);
    }
  };
  const static typeof(Base::VSA_NOCLDSTOP) VSA_NOCLDSTOP = Base::VSA_NOCLDSTOP;
  const static typeof(Base::VSA_NOCLDWAIT) VSA_NOCLDWAIT = Base::VSA_NOCLDWAIT;
  const static typeof(Base::VSA_RESETHAND) VSA_RESETHAND = Base::VSA_RESETHAND;
  const static typeof(Base::VSA_ONSTACK) VSA_ONSTACK = Base::VSA_ONSTACK;
  const static typeof(Base::VSA_SIGINFO) VSA_SIGINFO = Base::VSA_SIGINFO;
  const static typeof(Base::VSA_RESTART) VSA_RESTART = Base::VSA_RESTART;
  const static typeof(Base::VSA_NODEFER) VSA_NODEFER = Base::VSA_NODEFER;
  const static typeof(Base::VSA_INTERRUPT) VSA_INTERRUPT = Base::VSA_INTERRUPT;
  static SaSigFlags saFlagsToLocal(Tuint flags){
    SaSigFlags rv(static_cast<SaSigFlags>(0));
    if(flags&VSA_NODEFER) rv=static_cast<SaSigFlags>(rv|SaNoDefer);
    if(flags&VSA_SIGINFO) rv=static_cast<SaSigFlags>(rv|SaSigInfo);
    if(flags&VSA_RESTART) rv=static_cast<SaSigFlags>(rv|SaRestart);
    if(flags&~(VSA_NODEFER|VSA_SIGINFO|VSA_RESTART))
      fail("saFlagsToLocal(%lx) with unsupported flags\n",(unsigned long)flags);
    return rv;
  }
  static Tuint saFlagsFromLocal(SaSigFlags flags){
    Tuint rv(0);
    if(flags&SaNoDefer) rv|=VSA_NODEFER;
    if(flags&SaSigInfo) rv|=VSA_SIGINFO;
    if(flags&SaRestart) rv|=VSA_RESTART;
    if(flags&~(SaNoDefer|SaSigInfo|SaRestart))
      fail("saFlagsFromLocal(%lx) with unsupported flags\n",(unsigned long)flags);
    return rv;
  }
  const static typeof(Base::VSIG_DFL) VSIG_DFL = Base::VSIG_DFL;
  const static typeof(Base::VSIG_IGN) VSIG_IGN = Base::VSIG_IGN;
  static VAddr saHandlerToLocal(Tpointer_t hnd){
    switch(hnd){
    case VSIG_DFL: return static_cast<VAddr>(SigActDefault);
    case VSIG_IGN: return static_cast<VAddr>(SigActIgnore);
    }
    return static_cast<VAddr>(hnd);
  }
  static Tpointer_t saHandlerFromLocal(VAddr hnd){
    switch(hnd){
    case static_cast<VAddr>(SigActDefault): return VSIG_DFL;
    case static_cast<VAddr>(SigActIgnore):  return VSIG_IGN;
    }
    return static_cast<Tpointer_t>(hnd);
  }
  const static typeof(Base::V__NR_sigaction) V__NR_sigaction = Base::V__NR_sigaction;
  const static typeof(Base::V__NR_rt_sigaction) V__NR_rt_sigaction = Base::V__NR_rt_sigaction;
  void sysRtSigAction(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::VSIG_BLOCK) VSIG_BLOCK = Base::VSIG_BLOCK;
  const static typeof(Base::VSIG_UNBLOCK) VSIG_UNBLOCK = Base::VSIG_UNBLOCK;
  const static typeof(Base::VSIG_SETMASK) VSIG_SETMASK = Base::VSIG_SETMASK;
  const static typeof(Base::V__NR_sigprocmask) V__NR_sigprocmask = Base::V__NR_sigprocmask;
  const static typeof(Base::V__NR_rt_sigprocmask) V__NR_rt_sigprocmask = Base::V__NR_rt_sigprocmask;
  void sysRtSigProcMask(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sigpending) V__NR_sigpending = Base::V__NR_sigpending;
  const static typeof(Base::V__NR_rt_sigpending) V__NR_rt_sigpending = Base::V__NR_rt_sigpending;
  const static typeof(Base::V__NR_sigsuspend) V__NR_sigsuspend = Base::V__NR_sigsuspend;
  const static typeof(Base::V__NR_rt_sigsuspend) V__NR_rt_sigsuspend = Base::V__NR_rt_sigsuspend;
  void sysRtSigSuspend(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_rt_sigtimedwait) V__NR_rt_sigtimedwait = Base::V__NR_rt_sigtimedwait;
  const static typeof(Base::V__NR_signal) V__NR_signal = Base::V__NR_signal;
  const static typeof(Base::V__NR_signalfd) V__NR_signalfd = Base::V__NR_signalfd;
  const static typeof(Base::V__NR_sigaltstack) V__NR_sigaltstack = Base::V__NR_sigaltstack;
  const static typeof(Base::V__NR_sigreturn) V__NR_sigreturn = Base::V__NR_sigreturn;
  const static typeof(Base::V__NR_rt_sigreturn) V__NR_rt_sigreturn = Base::V__NR_rt_sigreturn;
  void sysRtSigReturn(ThreadContext *context, InstDesc *inst);
  // Time-related functionality
  typedef typename Base::Tclock_t Tclock_t;
  class Ttms{
    typedef typename Base::Ttms This;
  public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_tms_utime tms_utime;
    typename This::Type_tms_cutime tms_cutime;
    typename This::Type_tms_cstime tms_cstime;
    typename This::Type_tms_stime tms_stime;
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_tms_utime,fixEndian(tms_utime));
      context->writeMemRaw(addr+This::Offs_tms_cutime,fixEndian(tms_cutime));
      context->writeMemRaw(addr+This::Offs_tms_cstime,fixEndian(tms_cstime));
      context->writeMemRaw(addr+This::Offs_tms_stime,fixEndian(tms_stime));
    }
  };
  const static typeof(Base::V__NR_times) V__NR_times = Base::V__NR_times;
  void sysTimes(ThreadContext *context, InstDesc *inst);
  typedef typename Base::Ttime_t Ttime_t;
  const static typeof(Base::V__NR_time) V__NR_time = Base::V__NR_time;
  void sysTime(ThreadContext *context, InstDesc *inst);
  typedef typename Base::Tsuseconds_t Tsuseconds_t;
  class Ttimeval{
    typedef typename Base::Ttimeval This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_tv_sec  tv_sec;
    typename This::Type_tv_usec tv_usec;
    Ttimeval(typename This::Type_tv_sec sec, typename This::Type_tv_usec usec)
      : tv_sec(sec),
	tv_usec(usec)
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_tv_usec,fixEndian(tv_usec));
      context->writeMemRaw(addr+This::Offs_tv_sec,fixEndian(tv_sec));
    }
  };
  class Ttimezone{
    typedef typename Base::Ttimezone This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_tz_minuteswest tz_minuteswest;
    typename This::Type_tz_dsttime     tz_dsttime;
    Ttimezone(int minwest, int dsttime)
      : tz_minuteswest(minwest),
	tz_dsttime(dsttime)
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_tz_minuteswest,fixEndian(tz_minuteswest));
      context->writeMemRaw(addr+This::Offs_tz_dsttime,fixEndian(tz_dsttime));
    }
  };
  const static typeof(Base::V__NR_settimeofday) V__NR_settimeofday = Base::V__NR_settimeofday;
  const static typeof(Base::V__NR_gettimeofday) V__NR_gettimeofday = Base::V__NR_gettimeofday;
  void sysGetTimeOfDay(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_setitimer) V__NR_setitimer = Base::V__NR_setitimer;
  void sysSetITimer(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_getitimer) V__NR_getitimer = Base::V__NR_getitimer;
  typedef typename Base::Tclockid_t Tclockid_t;
  const static typeof(Base::V__NR_clock_getres) V__NR_clock_getres = Base::V__NR_clock_getres;
  void sysClockGetRes(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_clock_settime) V__NR_clock_settime = Base::V__NR_clock_settime;
  const static typeof(Base::V__NR_clock_gettime) V__NR_clock_gettime = Base::V__NR_clock_gettime;
  void sysClockGetTime(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_alarm) V__NR_alarm = Base::V__NR_alarm;
  void sysAlarm(ThreadContext *context, InstDesc *inst);
  class Ttimespec{
    typedef typename Base::Ttimespec This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_tv_sec tv_sec;
    typename This::Type_tv_nsec tv_nsec;
    Ttimespec(ThreadContext *context, VAddr addr)
      :
      tv_sec(fixEndian(context->readMemRaw<typeof(tv_sec)>(addr+This::Offs_tv_sec))),
      tv_nsec(fixEndian(context->readMemRaw<typeof(tv_nsec)>(addr+This::Offs_tv_nsec)))
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_tv_sec,fixEndian(tv_sec));
      context->writeMemRaw(addr+This::Offs_tv_nsec,fixEndian(tv_nsec));
    }
  };
  const static typeof(Base::V__NR_nanosleep) V__NR_nanosleep = Base::V__NR_nanosleep;
  void sysNanoSleep(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_clock_nanosleep) V__NR_clock_nanosleep = Base::V__NR_clock_nanosleep;
  void sysClockNanoSleep(ThreadContext *context, InstDesc *inst);
  // Process/thread scheduling functionality
  const static typeof(Base::V__NR_sched_yield) V__NR_sched_yield = Base::V__NR_sched_yield;
  void sysSchedYield(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_setpriority) V__NR_setpriority = Base::V__NR_setpriority;
  const static typeof(Base::V__NR_getpriority) V__NR_getpriority = Base::V__NR_getpriority;
  void sysGetPriority(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_getparam) V__NR_sched_getparam = Base::V__NR_sched_getparam;
  void sysSchedGetParam(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_setparam) V__NR_sched_setparam = Base::V__NR_sched_setparam;
  const static typeof(Base::V__NR_sched_setscheduler) V__NR_sched_setscheduler = Base::V__NR_sched_setscheduler;
  void sysSchedSetScheduler(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_getscheduler) V__NR_sched_getscheduler = Base::V__NR_sched_getscheduler;
  void sysSchedGetScheduler(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_get_priority_max) V__NR_sched_get_priority_max = Base::V__NR_sched_get_priority_max;
  void sysSchedGetPriorityMax(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_get_priority_min) V__NR_sched_get_priority_min = Base::V__NR_sched_get_priority_min;
  void sysSchedGetPriorityMin(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_setaffinity) V__NR_sched_setaffinity = Base::V__NR_sched_setaffinity;
  void sysSchedSetAffinity(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_getaffinity) V__NR_sched_getaffinity = Base::V__NR_sched_getaffinity;
  void sysSchedGetAffinity(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_sched_rr_get_interval) V__NR_sched_rr_get_interval = Base::V__NR_sched_rr_get_interval;
  // User info functionality
  typedef typename Base::Tuid_t     Tuid_t;
  typedef typename Base::Tgid_t     Tgid_t;
  const static typeof(Base::V__NR_setuid) V__NR_setuid = Base::V__NR_setuid;
  const static typeof(Base::V__NR_getuid) V__NR_getuid = Base::V__NR_getuid;
  void sysGetUid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_setreuid) V__NR_setreuid = Base::V__NR_setreuid;
  const static typeof(Base::V__NR_setresuid) V__NR_setresuid = Base::V__NR_setresuid;
  const static typeof(Base::V__NR_setfsuid) V__NR_setfsuid = Base::V__NR_setfsuid;
  const static typeof(Base::V__NR_geteuid) V__NR_geteuid = Base::V__NR_geteuid;
  void sysGetEuid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_getresuid) V__NR_getresuid = Base::V__NR_getresuid;
  const static typeof(Base::V__NR_setgid) V__NR_setgid = Base::V__NR_setgid;
  const static typeof(Base::V__NR_getgid) V__NR_getgid = Base::V__NR_getgid;
  void sysGetGid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_setregid) V__NR_setregid = Base::V__NR_setregid;
  const static typeof(Base::V__NR_setresgid) V__NR_setresgid = Base::V__NR_setresgid;
  const static typeof(Base::V__NR_setfsgid) V__NR_setfsgid = Base::V__NR_setfsgid;
  const static typeof(Base::V__NR_getegid) V__NR_getegid = Base::V__NR_getegid;
  void sysGetEgid(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_getresgid) V__NR_getresgid = Base::V__NR_getresgid;
  const static typeof(Base::V__NR_setgroups) V__NR_setgroups = Base::V__NR_setgroups;
  const static typeof(Base::V__NR_getgroups) V__NR_getgroups = Base::V__NR_getgroups;
  void sysGetGroups(ThreadContext *context, InstDesc *inst);
  // Memory management functionality
  const static typeof(Base::V__NR_brk) V__NR_brk = Base::V__NR_brk;
  void sysBrk(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_set_thread_area) V__NR_set_thread_area = Base::V__NR_set_thread_area;
  void sysSetThreadArea(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::VMAP_SHARED) VMAP_SHARED = Base::VMAP_SHARED;
  const static typeof(Base::VMAP_PRIVATE) VMAP_PRIVATE = Base::VMAP_PRIVATE;
  const static typeof(Base::VMAP_FIXED) VMAP_FIXED = Base::VMAP_FIXED;
  const static typeof(Base::VMAP_ANONYMOUS) VMAP_ANONYMOUS = Base::VMAP_ANONYMOUS;
  const static typeof(Base::VPROT_NONE) VPROT_NONE = Base::VPROT_NONE;
  const static typeof(Base::VPROT_READ) VPROT_READ = Base::VPROT_READ;
  const static typeof(Base::VPROT_WRITE) VPROT_WRITE = Base::VPROT_WRITE;
  const static typeof(Base::VPROT_EXEC) VPROT_EXEC = Base::VPROT_EXEC;
  const static typeof(Base::V__NR_mmap) V__NR_mmap = Base::V__NR_mmap;
  const static typeof(Base::V__NR_mmap2) V__NR_mmap2 = Base::V__NR_mmap2;
  template<size_t offsmul>
  void sysMMap(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::VMREMAP_MAYMOVE) VMREMAP_MAYMOVE = Base::VMREMAP_MAYMOVE;
  const static typeof(Base::VMREMAP_FIXED) VMREMAP_FIXED = Base::VMREMAP_FIXED;
  const static typeof(Base::V__NR_mremap) V__NR_mremap = Base::V__NR_mremap;
  void sysMReMap(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_munmap) V__NR_munmap = Base::V__NR_munmap;
  void sysMUnMap(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_mprotect) V__NR_mprotect = Base::V__NR_mprotect;
  void sysMProtect(ThreadContext *context, InstDesc *inst);
  // File-related functionality
  const static typeof(Base::VO_ACCMODE  ) VO_ACCMODE   = Base::VO_ACCMODE;
  const static typeof(Base::VO_RDONLY   ) VO_RDONLY    = Base::VO_RDONLY;
  const static typeof(Base::VO_WRONLY   ) VO_WRONLY    = Base::VO_WRONLY;
  const static typeof(Base::VO_RDWR     ) VO_RDWR      = Base::VO_RDWR;
  const static typeof(Base::VO_APPEND   ) VO_APPEND    = Base::VO_APPEND;
  const static typeof(Base::VO_SYNC     ) VO_SYNC      = Base::VO_SYNC;
  const static typeof(Base::VO_NONBLOCK ) VO_NONBLOCK  = Base::VO_NONBLOCK;
  const static typeof(Base::VO_CREAT    ) VO_CREAT     = Base::VO_CREAT;
  const static typeof(Base::VO_TRUNC    ) VO_TRUNC     = Base::VO_TRUNC;
  const static typeof(Base::VO_EXCL     ) VO_EXCL      = Base::VO_EXCL;
  const static typeof(Base::VO_NOCTTY   ) VO_NOCTTY    = Base::VO_NOCTTY;
  const static typeof(Base::VO_ASYNC    ) VO_ASYNC     = Base::VO_ASYNC;
  const static typeof(Base::VO_LARGEFILE) VO_LARGEFILE = Base::VO_LARGEFILE;
  const static typeof(Base::VO_DIRECT   ) VO_DIRECT    = Base::VO_DIRECT;
  const static typeof(Base::VO_DIRECTORY) VO_DIRECTORY = Base::VO_DIRECTORY;
  const static typeof(Base::VO_NOFOLLOW ) VO_NOFOLLOW  = Base::VO_NOFOLLOW;
  static int openFlagsToNative(Tint flags){
    int retVal;
    switch(flags&VO_ACCMODE){
      case VO_RDONLY: retVal=O_RDONLY; break;
      case VO_WRONLY: retVal=O_WRONLY; break;
      case VO_RDWR:   retVal=O_RDWR;   break;
      default: fail("openFlagsToNative(): unknown O_ACCMODE in %0x08\n",flags);
    }
    if(flags&VO_APPEND   ) retVal|=O_APPEND;
    if(flags&VO_SYNC     ) retVal|=O_SYNC;
    if(flags&VO_NONBLOCK ) retVal|=O_NONBLOCK;
    if(flags&VO_CREAT    ) retVal|=O_CREAT;
    if(flags&VO_TRUNC    ) retVal|=O_TRUNC;
    if(flags&VO_EXCL     ) retVal|=O_EXCL;
    if(flags&VO_NOCTTY   ) retVal|=O_NOCTTY;
    if(flags&VO_ASYNC    ) retVal|=O_ASYNC;
    if(flags&VO_LARGEFILE) retVal|=O_LARGEFILE;
    if(flags&VO_DIRECT   ) retVal|=O_DIRECT;
    if(flags&VO_DIRECTORY) retVal|=O_DIRECTORY;
    if(flags&VO_NOFOLLOW ) retVal|=O_NOFOLLOW;
    return retVal;
  }
  static Tint openFlagsFromNative(int flags){
    Tint retVal;
    switch(flags&O_ACCMODE){
      case O_RDONLY: retVal=VO_RDONLY; break;
      case O_WRONLY: retVal=VO_WRONLY; break;
      case O_RDWR:   retVal=VO_RDWR;   break;
      default: fail("openFlagsFromNative(): unknown O_ACCMODE in %0x08\n",flags);
    }
    if(flags&O_APPEND   ) retVal|=VO_APPEND;
    if(flags&O_SYNC     ) retVal|=VO_SYNC;
    if(flags&O_NONBLOCK ) retVal|=VO_NONBLOCK;
    if(flags&O_CREAT    ) retVal|=VO_CREAT;
    if(flags&O_TRUNC    ) retVal|=VO_TRUNC;
    if(flags&O_EXCL     ) retVal|=VO_EXCL;
    if(flags&O_NOCTTY   ) retVal|=VO_NOCTTY;
    if(flags&O_ASYNC    ) retVal|=VO_ASYNC;
    if(flags&O_LARGEFILE) retVal|=VO_LARGEFILE;
    if(flags&O_DIRECT   ) retVal|=VO_DIRECT;
    if(flags&O_DIRECTORY) retVal|=VO_DIRECTORY;
    if(flags&O_NOFOLLOW ) retVal|=VO_NOFOLLOW;
    return retVal;
  }
  typedef typename Base::Tmode_t Tmode_t;
  const static typeof(Base::VS_IRUSR) VS_IRUSR = Base::VS_IRUSR;
  const static typeof(Base::VS_IWUSR) VS_IWUSR = Base::VS_IWUSR;
  const static typeof(Base::VS_IXUSR) VS_IXUSR = Base::VS_IXUSR;
  const static typeof(Base::VS_IRGRP) VS_IRGRP = Base::VS_IRGRP;
  const static typeof(Base::VS_IWGRP) VS_IWGRP = Base::VS_IWGRP;
  const static typeof(Base::VS_IXGRP) VS_IXGRP = Base::VS_IXGRP;
  const static typeof(Base::VS_IROTH) VS_IROTH = Base::VS_IROTH;
  const static typeof(Base::VS_IWOTH) VS_IWOTH = Base::VS_IWOTH;
  const static typeof(Base::VS_IXOTH) VS_IXOTH = Base::VS_IXOTH;
  static mode_t mode_tToNative(Tmode_t fmode){
    mode_t retVal=0;
    if(fmode&VS_IRUSR) retVal|=S_IRUSR;
    if(fmode&VS_IWUSR) retVal|=S_IWUSR;
    if(fmode&VS_IXUSR) retVal|=S_IXUSR;
    if(fmode&VS_IRGRP) retVal|=S_IRGRP;
    if(fmode&VS_IWGRP) retVal|=S_IWGRP;
    if(fmode&VS_IXGRP) retVal|=S_IXGRP;
    if(fmode&VS_IROTH) retVal|=S_IROTH;
    if(fmode&VS_IWOTH) retVal|=S_IWOTH;
    if(fmode&VS_IXOTH) retVal|=S_IXOTH;
    return retVal;
  }
  const static typeof(Base::V__NR_open) V__NR_open = Base::V__NR_open;
  void sysOpen(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_pipe) V__NR_pipe = Base::V__NR_pipe;
  void sysPipe(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_dup) V__NR_dup = Base::V__NR_dup;
  void sysDup(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_dup2) V__NR_dup2 = Base::V__NR_dup2;
  void sysDup2(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::VF_DUPFD) VF_DUPFD = Base::VF_DUPFD;
  const static typeof(Base::VF_GETFD) VF_GETFD = Base::VF_GETFD;
  const static typeof(Base::VF_SETFD) VF_SETFD = Base::VF_SETFD;
  const static typeof(Base::VF_GETFL) VF_GETFL = Base::VF_GETFL;
  const static typeof(Base::VF_SETFL) VF_SETFL = Base::VF_SETFL;
  const static typeof(Base::VFD_CLOEXEC) VFD_CLOEXEC = Base::VFD_CLOEXEC;
  const static typeof(Base::V__NR_fcntl) V__NR_fcntl = Base::V__NR_fcntl;
  const static typeof(Base::V__NR_fcntl64) V__NR_fcntl64 = Base::V__NR_fcntl64;
  void sysFCntl(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_read) V__NR_read = Base::V__NR_read;
  void sysRead(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_write) V__NR_write = Base::V__NR_write;
  void sysWrite(ThreadContext *context, InstDesc *inst);
  class Tiovec{
    typedef typename Base::Tiovec This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_iov_base iov_base;
    typename This::Type_iov_len  iov_len;
    Tiovec(ThreadContext *context, VAddr addr)
      :
      iov_base(fixEndian(context->readMemRaw<typeof(iov_base)>(addr+This::Offs_iov_base))),
      iov_len(fixEndian(context->readMemRaw<typeof(iov_len)>(addr+This::Offs_iov_len)))
    {
    }
  };
  const static typeof(Base::V__NR_writev) V__NR_writev = Base::V__NR_writev;
  void sysWriteV(ThreadContext *context, InstDesc *inst);
  typedef typename Base::Toff_t  Toff_t;
  typedef typename Base::Tloff_t Tloff_t;
  const static typeof(Base::VSEEK_SET) VSEEK_SET = Base::VSEEK_SET;
  const static typeof(Base::VSEEK_CUR) VSEEK_CUR = Base::VSEEK_CUR;
  const static typeof(Base::VSEEK_END) VSEEK_END = Base::VSEEK_END;
  static int whenceToNative(Tint whence){
    switch(whence){
      case VSEEK_SET: return SEEK_SET;
      case VSEEK_CUR: return SEEK_CUR;
      case VSEEK_END: return SEEK_END;
      default:
        fail("whenceToNative(): unknown whence value %d\n",(int)whence);
    }
  }
  const static typeof(Base::V__NR_lseek) V__NR_lseek = Base::V__NR_lseek;
  void sysLSeek(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR__llseek) V__NR__llseek = Base::V__NR__llseek;
  void sysLLSeek(ThreadContext *context, InstDesc *inst);
  class Tdirent{
    typedef typename Base::Tdirent This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_d_ino    d_ino;
    typename This::Type_d_off    d_off;
    typename This::Type_d_reclen d_reclen;
    typename This::Type_d_name   d_name[This::Size_d_name];
    Tdirent(struct dirent64 &src)
      : d_ino(src.d_ino),
        d_off(src.d_off)
    {
      size_t namelen=strlen(src.d_name)+1;
      if(namelen>This::Size_d_name)
        fail("Tdirent() d_name is too long\n");
      d_reclen=alignUp(This::Offs_d_name+This::Step_d_name*namelen,sizeof(uint64_t));
      for(size_t i=0;i<(d_reclen-This::Offs_d_name);i++)
        d_name[i]=(i<namelen)?src.d_name[i]:0;
    }
    Tdirent(const std::string &name, off_t off, ino_t ino)
      : d_ino(ino),
        d_off(off)
    {
      size_t namelen=name.length()+1;
      if(namelen>This::Size_d_name)
        fail("Tdirent() d_name is too long\n");
      d_reclen=alignUp(This::Offs_d_name+This::Step_d_name*namelen,sizeof(uint64_t));
      for(size_t i=0;i<(d_reclen-This::Offs_d_name);i++)
        d_name[i]=(i<namelen)?name[i]:0;
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_d_ino,fixEndian(d_ino));
      context->writeMemRaw(addr+This::Offs_d_off,fixEndian(d_off));
      context->writeMemRaw(addr+This::Offs_d_reclen,fixEndian(d_reclen));
      for(size_t i=0;i<(d_reclen-This::Offs_d_name);i++)
        context->writeMemRaw(addr+This::Offs_d_name+i*This::Step_d_name,fixEndian(d_name[i]));
    }
  };
  class Tdirent64{
    typedef typename Base::Tdirent64 This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_d_ino    d_ino;
    typename This::Type_d_off    d_off;
    typename This::Type_d_reclen d_reclen;
    typename This::Type_d_type   d_type;
    typename This::Type_d_name   d_name[This::Size_d_name];
    Tdirent64(struct dirent64 &src)
      : d_ino(src.d_ino),
        d_off(src.d_off),
        d_type(src.d_type)
    {
      size_t namelen=strlen(src.d_name)+1;
      if(namelen>This::Size_d_name)
        fail("Tdirent64() d_name is too long\n");
      d_reclen=alignUp(This::Offs_d_name+This::Step_d_name*namelen,sizeof(uint64_t));
      for(size_t i=0;i<((d_reclen-This::Offs_d_name)/This::Step_d_name);i++)
        d_name[i]=(i<namelen)?src.d_name[i]:0;
      I(src.d_reclen%8==0);
      I(size_t(src.d_reclen-(src.d_name-(char *)&src))==d_reclen-This::Offs_d_name);
    }
    Tdirent64(const std::string &name, off_t off, ino_t ino)
      : d_ino(ino),
        d_off(off),
	d_type(0)
    {
      size_t namelen=name.length()+1;
      if(namelen>This::Size_d_name)
        fail("Tdirent() d_name is too long\n");
      d_reclen=alignUp(This::Offs_d_name+This::Step_d_name*namelen,sizeof(uint64_t));
      for(size_t i=0;i<(d_reclen-This::Offs_d_name);i++)
        d_name[i]=(i<namelen)?name[i]:0;
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_d_ino,fixEndian(d_ino));
      context->writeMemRaw(addr+This::Offs_d_off,fixEndian(d_off));
      context->writeMemRaw(addr+This::Offs_d_reclen,fixEndian(d_reclen));
      context->writeMemRaw(addr+This::Offs_d_type,fixEndian(d_type));
      for(size_t i=0;i<((d_reclen-This::Offs_d_name)/This::Step_d_name);i++)
        context->writeMemRaw(addr+This::Offs_d_name+i*This::Step_d_name,fixEndian(d_name[i]));
    }
  };
  const static typeof(Base::V__NR_getdents) V__NR_getdents = Base::V__NR_getdents;
  const static typeof(Base::V__NR_getdents64) V__NR_getdents64 = Base::V__NR_getdents64;
  template<class Tdirent>
  void sysGetDEnts(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::VTCGETS    ) VTCGETS     = Base::VTCGETS;
  const static typeof(Base::VTCSETS    ) VTCSETS     = Base::VTCSETS;
  const static typeof(Base::VTCSETSW   ) VTCSETSW    = Base::VTCSETSW;
  const static typeof(Base::VTCSETSF   ) VTCSETSF    = Base::VTCSETSF;
  const static typeof(Base::VTCGETA    ) VTCGETA     = Base::VTCGETA;
  const static typeof(Base::VTCSETA    ) VTCSETA     = Base::VTCSETA;
  const static typeof(Base::VTCSETAW   ) VTCSETAW    = Base::VTCSETAW;
  const static typeof(Base::VTCSETAF   ) VTCSETAF    = Base::VTCSETAF;
  const static typeof(Base::VTIOCGWINSZ) VTIOCGWINSZ = Base::VTIOCGWINSZ;
  class Twinsize{
    typedef typename Base::Twinsize This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_ws_row    ws_row;
    typename This::Type_ws_col    ws_col;
    typename This::Type_ws_xpixel ws_xpixel;
    typename This::Type_ws_ypixel ws_ypixel;
    Twinsize(const struct winsize &src)
      : ws_row(src.ws_row),
        ws_col(src.ws_col),
        ws_xpixel(src.ws_xpixel),
        ws_ypixel(src.ws_ypixel)
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_ws_row,fixEndian(ws_row));
      context->writeMemRaw(addr+This::Offs_ws_col,fixEndian(ws_col));
      context->writeMemRaw(addr+This::Offs_ws_xpixel,fixEndian(ws_xpixel));
      context->writeMemRaw(addr+This::Offs_ws_ypixel,fixEndian(ws_ypixel));
    }
  };
  const static typeof(Base::V__NR_ioctl) V__NR_ioctl = Base::V__NR_ioctl;
  void sysIOCtl(ThreadContext *context, InstDesc *inst);
  class Tpollfd{
    typedef typename Base::Tpollfd This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_fd      fd;
    typename This::Type_events  events;
    typename This::Type_revents revents;
    Tpollfd(ThreadContext *context, VAddr addr)
      :
      fd(fixEndian(context->readMemRaw<typeof(fd)>(addr+This::Offs_fd))),
      events(fixEndian(context->readMemRaw<typeof(events) >(addr+This::Offs_events ))),
      revents(fixEndian(context->readMemRaw<typeof(revents)>(addr+This::Offs_revents)))
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_fd,fixEndian(fd));
      context->writeMemRaw(addr+This::Offs_events,fixEndian(events));
      context->writeMemRaw(addr+This::Offs_revents,fixEndian(revents));
    }
  };
  const static typeof(Base::VPOLLIN  ) VPOLLIN   = Base::VPOLLIN;
  const static typeof(Base::VPOLLPRI ) VPOLLPRI  = Base::VPOLLPRI;
  const static typeof(Base::VPOLLOUT ) VPOLLOUT  = Base::VPOLLOUT;
  const static typeof(Base::VPOLLERR ) VPOLLERR  = Base::VPOLLERR;
  const static typeof(Base::VPOLLHUP ) VPOLLHUP  = Base::VPOLLHUP;
  const static typeof(Base::VPOLLNVAL) VPOLLNVAL = Base::VPOLLNVAL;
  const static typeof(Base::V__NR_poll) V__NR_poll = Base::V__NR_poll;
  void sysPoll(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_close) V__NR_close = Base::V__NR_close;
  void sysClose(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_truncate) V__NR_truncate = Base::V__NR_truncate;
  const static typeof(Base::V__NR_truncate64) V__NR_truncate64 = Base::V__NR_truncate64;
  template<class Toffs>
  void sysTruncate(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_ftruncate) V__NR_ftruncate = Base::V__NR_ftruncate;
  const static typeof(Base::V__NR_ftruncate64) V__NR_ftruncate64 = Base::V__NR_ftruncate64;
  template<class Toffs>
  void sysFTruncate(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_chmod) V__NR_chmod = Base::V__NR_chmod;
  void sysChMod(ThreadContext *context, InstDesc *inst);
  typedef typename Base::Tino_t     Tino_t;
  typedef typename Base::Tnlink_t   Tnlink_t;
  typedef typename Base::Tblksize_t Tblksize_t;
  class Tstat{
    typedef typename Base::Tstat This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_st_blksize st_blksize;
    typename This::Type_st_blocks st_blocks;
    typename This::Type_st_atime_sec st_atime_sec;
    typename This::Type_st_gid st_gid;
    typename This::Type_st_uid st_uid;
    typename This::Type_st_mode st_mode;
    typename This::Type_st_ino st_ino;
    typename This::Type_st_mtime_sec st_mtime_sec;
    typename This::Type_st_ctime_sec st_ctime_sec;
    typename This::Type_st_dev st_dev;
    typename This::Type_st_rdev st_rdev;
    typename This::Type_st_size st_size;
    typename This::Type_st_nlink st_nlink;
    Tstat(void)
      :
      st_blksize(0),
      st_blocks(0),
      st_atime_sec(0),
      st_gid(0),
      st_uid(0),
      st_mode(0),
      st_ino(0),
      st_mtime_sec(0),
      st_ctime_sec(0),
      st_dev(0),
      st_rdev(0),
      st_size(0),
      st_nlink(0)
    {
    }
    Tstat(struct stat &st)
      :
      st_blksize(st.st_blksize),
      st_blocks(st.st_blocks),
      st_atime_sec(st.st_atime),
      st_gid(st.st_gid),
      st_uid(st.st_uid),
      st_mode(st.st_mode),
      st_ino(st.st_ino),
      st_mtime_sec(st.st_mtime),
      st_ctime_sec(st.st_ctime),
      st_dev(st.st_dev),
      st_rdev(st.st_rdev),
      st_size(st.st_size),
      st_nlink(st.st_nlink)
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_st_blksize,fixEndian(st_blksize));
      context->writeMemRaw(addr+This::Offs_st_blocks,fixEndian(st_blocks));
      context->writeMemRaw(addr+This::Offs_st_atime_sec,fixEndian(st_atime_sec));
      context->writeMemRaw(addr+This::Offs_st_gid,fixEndian(st_gid));
      context->writeMemRaw(addr+This::Offs_st_uid,fixEndian(st_uid));
      context->writeMemRaw(addr+This::Offs_st_mode,fixEndian(st_mode));
      context->writeMemRaw(addr+This::Offs_st_ino,fixEndian(st_ino));
      context->writeMemRaw(addr+This::Offs_st_mtime_sec,fixEndian(st_mtime_sec));
      context->writeMemRaw(addr+This::Offs_st_ctime_sec,fixEndian(st_ctime_sec));
      context->writeMemRaw(addr+This::Offs_st_dev,fixEndian(st_dev));
      context->writeMemRaw(addr+This::Offs_st_rdev,fixEndian(st_rdev));
      context->writeMemRaw(addr+This::Offs_st_size,fixEndian(st_size));
      context->writeMemRaw(addr+This::Offs_st_nlink,fixEndian(st_nlink));
    }
  };
  typedef typename Base::Tblkcnt64_t Tblkcnt64_t;
  class Tstat64{
    typedef typename Base::Tstat64 This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_st_blksize st_blksize;
    typename This::Type_st_blocks st_blocks;
    typename This::Type_st_atime st_atim;
    typename This::Type_st_gid st_gid;
    typename This::Type_st_uid st_uid;
    typename This::Type_st_mode st_mode;
    typename This::Type_st_ino st_ino;
    typename This::Type_st_mtime st_mtim;
    typename This::Type_st_ctime st_ctim;
    typename This::Type_st_dev st_dev;
    typename This::Type_st_rdev st_rdev;
    typename This::Type_st_size st_size;
    typename This::Type_st_nlink st_nlink;
    Tstat64(void)
      :
      st_blksize(0),
      st_blocks(0),
      st_atim(0),
      st_gid(0),
      st_uid(0),
      st_mode(0),
      st_ino(0),
      st_mtim(0),
      st_ctim(0),
      st_dev(0),
      st_rdev(0),
      st_size(0),
      st_nlink(0)
    {
    }
    Tstat64(struct stat &st)
      :
      st_blksize(st.st_blksize),
      st_blocks(st.st_blocks),
      st_atim(st.st_atime),
      st_gid(st.st_gid),
      st_uid(st.st_uid),
      st_mode(st.st_mode),
      st_ino(st.st_ino),
      st_mtim(st.st_mtime),
      st_ctim(st.st_ctime),
      st_dev(st.st_dev),
      st_rdev(st.st_rdev),
      st_size(st.st_size),
      st_nlink(st.st_nlink)
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_st_blksize,fixEndian(st_blksize));
      context->writeMemRaw(addr+This::Offs_st_blocks,fixEndian(st_blocks));
      context->writeMemRaw(addr+This::Offs_st_atime,fixEndian(st_atim));
      context->writeMemRaw(addr+This::Offs_st_gid,fixEndian(st_gid));
      context->writeMemRaw(addr+This::Offs_st_uid,fixEndian(st_uid));
      context->writeMemRaw(addr+This::Offs_st_mode,fixEndian(st_mode));
      context->writeMemRaw(addr+This::Offs_st_ino,fixEndian(st_ino));
      context->writeMemRaw(addr+This::Offs_st_mtime,fixEndian(st_mtim));
      context->writeMemRaw(addr+This::Offs_st_ctime,fixEndian(st_ctim));
      context->writeMemRaw(addr+This::Offs_st_dev,fixEndian(st_dev));
      context->writeMemRaw(addr+This::Offs_st_rdev,fixEndian(st_rdev));
      context->writeMemRaw(addr+This::Offs_st_size,fixEndian(st_size));
      context->writeMemRaw(addr+This::Offs_st_nlink,fixEndian(st_nlink));
    }
  };
  const static typeof(Base::V__NR_stat) V__NR_stat = Base::V__NR_stat;
  const static typeof(Base::V__NR_stat64) V__NR_stat64 = Base::V__NR_stat64;
  const static typeof(Base::V__NR_lstat) V__NR_lstat = Base::V__NR_lstat;
  const static typeof(Base::V__NR_lstat64) V__NR_lstat64 = Base::V__NR_lstat64;
  template<bool link, class Tstat_t>
  void sysStat(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_fstat) V__NR_fstat = Base::V__NR_fstat;
  const static typeof(Base::V__NR_fstat64) V__NR_fstat64 = Base::V__NR_fstat64;
  template<class Tstat_t>
  void sysFStat(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_fstatfs) V__NR_fstatfs = Base::V__NR_fstatfs;
  const static typeof(Base::V__NR_fstatfs64) V__NR_fstatfs64 = Base::V__NR_fstatfs64;
  void sysFStatFS(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_unlink) V__NR_unlink = Base::V__NR_unlink;
  void sysUnlink(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_symlink) V__NR_symlink = Base::V__NR_symlink;
  void sysSymLink(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_rename) V__NR_rename = Base::V__NR_rename;
  void sysRename(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_chdir) V__NR_chdir = Base::V__NR_chdir;
  void sysChdir(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::VF_OK) VF_OK = Base::VF_OK;
  const static typeof(Base::VR_OK) VR_OK = Base::VR_OK;
  const static typeof(Base::VW_OK) VW_OK = Base::VW_OK;
  const static typeof(Base::VX_OK) VX_OK = Base::VX_OK;
  static int accessModeToNative(int amode){
    int retVal=F_OK;
    if(amode&VR_OK) retVal|=R_OK;
    if(amode&VW_OK) retVal|=W_OK;
    if(amode&VX_OK) retVal|=X_OK;
    return retVal;
  }
  const static typeof(Base::V__NR_access) V__NR_access = Base::V__NR_access;
  void sysAccess(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_getcwd) V__NR_getcwd = Base::V__NR_getcwd;
  void sysGetCWD(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_mkdir) V__NR_mkdir = Base::V__NR_mkdir;
  void sysMkdir(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_rmdir) V__NR_rmdir = Base::V__NR_rmdir;
  void sysRmdir(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_umask) V__NR_umask = Base::V__NR_umask;
  void sysUmask(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_readlink) V__NR_readlink = Base::V__NR_readlink;
  void sysReadLink(ThreadContext *context, InstDesc *inst);
  // Network functionality
  const static typeof(Base::V__NR_socket) V__NR_socket = Base::V__NR_socket;
  void sysSocket(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_socketpair) V__NR_socketpair = Base::V__NR_socketpair;
  const static typeof(Base::V__NR_bind) V__NR_bind = Base::V__NR_bind;
  void sysBind(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_accept) V__NR_accept = Base::V__NR_accept;
  const static typeof(Base::V__NR_connect) V__NR_connect = Base::V__NR_connect;
  void sysConnect(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_send) V__NR_send = Base::V__NR_send;
  void sysSend(ThreadContext *context, InstDesc *inst);
  // Process info functionality
  typedef typename Base::Trlim_t Trlim_t;
  class Trlimit{
    typedef typename Base::Trlimit This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_rlim_cur rlim_cur;
    typename This::Type_rlim_max rlim_max;
    Trlimit(Trlim_t cur, Trlim_t max)
      :
      rlim_cur(cur),
      rlim_max(max)
    {
    }
    Trlimit(ThreadContext *context, VAddr addr)
      :
      rlim_cur(fixEndian(context->readMemRaw<typeof(rlim_cur)>(addr+This::Offs_rlim_cur))),
      rlim_max(fixEndian(context->readMemRaw<typeof(rlim_max)>(addr+This::Offs_rlim_max)))
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      context->writeMemRaw(addr+This::Offs_rlim_cur,fixEndian(rlim_cur));
      context->writeMemRaw(addr+This::Offs_rlim_max,fixEndian(rlim_max));
    }
  };
  const static typeof(Base::VRLIMIT_AS     ) VRLIMIT_AS      = Base::VRLIMIT_AS;
  const static typeof(Base::VRLIMIT_CORE   ) VRLIMIT_CORE    = Base::VRLIMIT_CORE;
  const static typeof(Base::VRLIMIT_CPU    ) VRLIMIT_CPU     = Base::VRLIMIT_CPU;
  const static typeof(Base::VRLIMIT_DATA   ) VRLIMIT_DATA    = Base::VRLIMIT_DATA;
  const static typeof(Base::VRLIMIT_FSIZE  ) VRLIMIT_FSIZE   = Base::VRLIMIT_FSIZE;
  const static typeof(Base::VRLIMIT_LOCKS  ) VRLIMIT_LOCKS   = Base::VRLIMIT_LOCKS;
  const static typeof(Base::VRLIMIT_MEMLOCK) VRLIMIT_MEMLOCK = Base::VRLIMIT_MEMLOCK;
  const static typeof(Base::VRLIMIT_NOFILE ) VRLIMIT_NOFILE  = Base::VRLIMIT_NOFILE;
  const static typeof(Base::VRLIMIT_NPROC  ) VRLIMIT_NPROC   = Base::VRLIMIT_NPROC;
  const static typeof(Base::VRLIMIT_RSS    ) VRLIMIT_RSS     = Base::VRLIMIT_RSS;
  const static typeof(Base::VRLIMIT_STACK  ) VRLIMIT_STACK   = Base::VRLIMIT_STACK;
  const static typeof(Base::VRLIM_INFINITY ) VRLIM_INFINITY  = Base::VRLIM_INFINITY;
  const static typeof(Base::V__NR_setrlimit) V__NR_setrlimit = Base::V__NR_setrlimit;
  void sysSetRLimit(ThreadContext *context, InstDesc *inst);
  const static typeof(Base::V__NR_getrlimit) V__NR_getrlimit = Base::V__NR_getrlimit;
  void sysGetRLimit(ThreadContext *context, InstDesc *inst);
  class Trusage{
    typedef typename Base::Trusage This;
   public:
    static const size_t getSize(void){ return This::Size_All; }
    Ttimeval ru_utime;
    Ttimeval ru_stime;
    typename This::Type_ru_maxrss ru_maxrss;
    typename This::Type_ru_ixrss ru_ixrss;
    typename This::Type_ru_idrss ru_idrss;
    typename This::Type_ru_isrss ru_isrss;
    typename This::Type_ru_minflt ru_minflt;
    typename This::Type_ru_majflt ru_majflt;
    typename This::Type_ru_nswap ru_nswap;
    typename This::Type_ru_inblock ru_inblock;
    typename This::Type_ru_oublock ru_oublock;
    typename This::Type_ru_msgsnd ru_msgsnd;
    typename This::Type_ru_msgrcv ru_msgrcv;
    typename This::Type_ru_nsignals ru_nsignals;
    typename This::Type_ru_nvcsw ru_nvcsw;
    typename This::Type_ru_nivcsw ru_nivcsw;
    Trusage(const Ttimeval &utime, const Ttimeval &stime)
      : ru_utime(utime),
	ru_stime(stime),
	ru_maxrss(0), ru_ixrss(0), ru_idrss(0), ru_isrss(0),
	ru_minflt(0), ru_majflt(0), ru_nswap(0),
	ru_inblock(0), ru_oublock(0), ru_msgsnd(0), ru_msgrcv(0),
	ru_nsignals(0), ru_nvcsw(0), ru_nivcsw(0)
    {
    }
    void write(ThreadContext *context, VAddr addr) const{
      ru_utime.write(context,addr+This::Offs_ru_utime);
      ru_stime.write(context,addr+This::Offs_ru_stime);
      context->writeMemRaw(addr+This::Offs_ru_maxrss,fixEndian(ru_maxrss));
      context->writeMemRaw(addr+This::Offs_ru_ixrss,fixEndian(ru_ixrss));
      context->writeMemRaw(addr+This::Offs_ru_idrss,fixEndian(ru_idrss));
      context->writeMemRaw(addr+This::Offs_ru_isrss,fixEndian(ru_isrss));
      context->writeMemRaw(addr+This::Offs_ru_minflt,fixEndian(ru_minflt));
      context->writeMemRaw(addr+This::Offs_ru_majflt,fixEndian(ru_majflt));
      context->writeMemRaw(addr+This::Offs_ru_nswap,fixEndian(ru_nswap));
      context->writeMemRaw(addr+This::Offs_ru_inblock,fixEndian(ru_inblock));
      context->writeMemRaw(addr+This::Offs_ru_oublock,fixEndian(ru_oublock));
      context->writeMemRaw(addr+This::Offs_ru_msgsnd,fixEndian(ru_msgsnd));
      context->writeMemRaw(addr+This::Offs_ru_msgrcv,fixEndian(ru_msgrcv));
      context->writeMemRaw(addr+This::Offs_ru_nsignals,fixEndian(ru_nsignals));
      context->writeMemRaw(addr+This::Offs_ru_nvcsw,fixEndian(ru_nvcsw));
      context->writeMemRaw(addr+This::Offs_ru_nivcsw,fixEndian(ru_nivcsw));
    }
  };
  const static typeof(Base::VRUSAGE_SELF) VRUSAGE_SELF = Base::VRUSAGE_SELF;
  const static typeof(Base::VRUSAGE_CHILDREN) VRUSAGE_CHILDREN = Base::VRUSAGE_CHILDREN;
  const static typeof(Base::V__NR_getrusage) V__NR_getrusage = Base::V__NR_getrusage;
  void sysGetRUsage(ThreadContext *context, InstDesc *inst);
  // System info functionality
  class Tutsname{
    typedef typename Base::Tutsname This;
  public:
    static const size_t getSize(void){ return This::Size_All; }
    typename This::Type_nodename nodename[This::Size_nodename];
    typename This::Type_sysname sysname[This::Size_sysname];
    typename This::Type_version version[This::Size_version];
    typename This::Type_release release[This::Size_release];
    typename This::Type_machine machine[This::Size_machine];
    Tutsname(void){
      memset(nodename,0,sizeof(nodename));
      memset(sysname,0,sizeof(sysname));
      memset(version,0,sizeof(version));
      memset(release,0,sizeof(release));
      memset(machine,0,sizeof(machine));
    }
    void write(ThreadContext *context, VAddr addr) const{
      char buf[This::Size_All];
      memset(buf,0,This::Size_All);
      context->writeMemFromBuf(addr,This::Size_All,buf);
      for(size_t i=0;i<This::Size_nodename;i++)
        context->writeMemRaw(addr+This::Offs_nodename+i*This::Step_nodename,fixEndian(nodename[i]));
      for(size_t i=0;i<This::Size_sysname;i++)
        context->writeMemRaw(addr+This::Offs_sysname+i*This::Step_sysname,fixEndian(sysname[i]));
      for(size_t i=0;i<This::Size_version;i++)
        context->writeMemRaw(addr+This::Offs_version+i*This::Step_version,fixEndian(version[i]));
      for(size_t i=0;i<This::Size_release;i++)
        context->writeMemRaw(addr+This::Offs_release+i*This::Step_release,fixEndian(release[i]));
      for(size_t i=0;i<This::Size_machine;i++)
        context->writeMemRaw(addr+This::Offs_machine+i*This::Step_machine,fixEndian(machine[i]));
    }
  };
  const static typeof(Base::V__NR_uname) V__NR_uname = Base::V__NR_uname;
  void sysUname(ThreadContext *context, InstDesc *inst);
  class T__sysctl_args{
    typedef typename Base::T__sysctl_args This;
   public:
    typename This::Type_newval newval;
    typename This::Type_oldval oldval;
    typename This::Type_name name;
    typename This::Type_newlen newlen;
    typename This::Type_nlen nlen;
    typename This::Type_oldlenp oldlenp;
    static const size_t getSize(void){ return This::Size_All; }
    T__sysctl_args(ThreadContext *context, VAddr addr)
      :
      newval(fixEndian(context->readMemRaw<typeof(newval)>(addr+This::Offs_newval))),
      oldval(fixEndian(context->readMemRaw<typeof(oldval)>(addr+This::Offs_oldval))),
      name(fixEndian(context->readMemRaw<typeof(name)>(addr+This::Offs_name))),
      newlen(fixEndian(context->readMemRaw<typeof(newlen)>(addr+This::Offs_newlen))),
      nlen(fixEndian(context->readMemRaw<typeof(nlen)>(addr+This::Offs_nlen))),
      oldlenp(fixEndian(context->readMemRaw<typeof(oldlenp)>(addr+This::Offs_oldlenp)))
    {
    }
  };
  const static typeof(Base::VCTL_KERN) VCTL_KERN = Base::VCTL_KERN;
  const static typeof(Base::VKERN_VERSION) VKERN_VERSION = Base::VKERN_VERSION;
  const static typeof(Base::V__NR__sysctl) V__NR__sysctl = Base::V__NR__sysctl;
  void sysSysCtl(ThreadContext *context, InstDesc *inst);
public:
  virtual InstDesc *sysCall(ThreadContext *context, InstDesc *inst);
protected:
  static int stackGrowthSign(void);
  static Tpointer_t getStackPointer(const ThreadContext *context);
  static void setStackPointer(ThreadContext *context, Tpointer_t addr);
  template<typename T>
  static void pushScalar(ThreadContext *context, const T &val);
  template<typename T>
  static T popScalar(ThreadContext *context);
  template<typename T>
  static void pushStruct(ThreadContext *context, const T &val);
  template<typename T>
  static T popStruct(ThreadContext *context);
  class CallArgs{
  protected:
    ThreadContext *context;
    size_t pos;
  public:
    CallArgs(ThreadContext *context) : context(context), pos(0){}
    template<typename T>
    CallArgs &operator>>(T &ref){ fill<T>(ref); return *this; }
    template<typename T>
    void fill(T &ref);
  };
  static void setThreadArea(ThreadContext *context, Tpointer_t addr);
  static void setSysErr(ThreadContext *context, Tint err=errorFromNative());
  template<typename T>
  static void setSysRet(ThreadContext *context, T val);
  static inline void setSysRet(ThreadContext *context){
    return setSysRet<Tint>(context,0);
  }
  static void setSysRet(ThreadContext *context, Tint val1, Tint val2);
  static Tint getSysCallNum(const ThreadContext *context);
 public:
  virtual SignalAction handleSignal(ThreadContext *context, SigInfo *sigInfo) const;
  virtual void createStack(ThreadContext *context) const;
  virtual void initSystem(ThreadContext *context) const;
  virtual void setProgArgs(ThreadContext *context, int argc, char **argv, int envc, char **envp) const;
  virtual void exitRobustList(ThreadContext *context, VAddr robust_list);
  virtual void clearChildTid(ThreadContext *context, VAddr &clear_child_tid);
};

#if (defined SUPPORT_MIPS32)
template<>
RealLinuxSys<ExecModeMips32>::Tpointer_t RealLinuxSys<ExecModeMips32>::getStackPointer(const ThreadContext *context){
  return getReg<Tpointer_t,RegSP>(context,RegSP);
}
template<>
void RealLinuxSys<ExecModeMips32>::setStackPointer(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegSP>(context,RegSP,addr);
}

template<>
template<typename T>
void RealLinuxSys<ExecModeMips32>::CallArgs::fill(T &ref){
  pos=(pos+sizeof(T)-1)&(~(sizeof(T)-1));
  I((pos%sizeof(T))==0);
  if(pos<16){
    uint32_t tmp1=getReg<uint32_t,RegTypeGpr>(context,RegA0+pos/sizeof(uint32_t));
    switch(sizeof(T)){
    case sizeof(uint32_t):
      ref=static_cast<T>(tmp1);
      break;
    case sizeof(uint64_t): {
      uint32_t tmp2=getReg<uint32_t,RegTypeGpr>(context,RegA0+pos/sizeof(uint32_t)+1);
      ref=static_cast<T>((uint64_t(tmp1)<<32)|uint64_t(tmp2));
    } break;
    default:
      fail("RealLinuxSys::Args::operator>>() for type of size %d unsupported\n",sizeof(T));
    }
  }else{
    ref=fixEndian(context->readMemRaw<T>(getStackPointer(context)+pos));
  }
  pos+=sizeof(T);
}

template<>
void RealLinuxSys<ExecModeMips32>::setSysErr(ThreadContext *context, Tint err){
  setReg<Tint,RegTypeGpr>(context,RegA3,Tint(1));
  setReg<Tint,RegTypeGpr>(context,RegV0,err);
}
template<>
template<typename T>
void RealLinuxSys<ExecModeMips32>::setSysRet(ThreadContext *context, T val){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val));
}
template<>
void RealLinuxSys<ExecModeMips32>::setSysRet(ThreadContext *context, Tint val1, Tint val2){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val1));
  setReg<Tregv_t,RegTypeGpr>(context,RegV1,Tregv_t(val2));
}
#endif
#if (defined SUPPORT_MIPSEL32)
template<>
RealLinuxSys<ExecModeMipsel32>::Tpointer_t RealLinuxSys<ExecModeMipsel32>::getStackPointer(const ThreadContext *context){
  return getReg<Tpointer_t,RegSP>(context,RegSP);
}
template<>
void RealLinuxSys<ExecModeMipsel32>::setStackPointer(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegSP>(context,RegSP,addr);
}

template<>
template<typename T>
void RealLinuxSys<ExecModeMipsel32>::CallArgs::fill(T &ref){
  pos=(pos+sizeof(T)-1)&(~(sizeof(T)-1));
  I((pos%sizeof(T))==0);
  if(pos<16){
    uint32_t tmp1=getReg<uint32_t,RegTypeGpr>(context,RegA0+pos/sizeof(uint32_t));
    switch(sizeof(T)){
    case sizeof(uint32_t):
      ref=static_cast<T>(tmp1);
      break;
    case sizeof(uint64_t): {
      uint32_t tmp2=getReg<uint32_t,RegTypeGpr>(context,RegA0+pos/sizeof(uint32_t)+1);
      ref=static_cast<T>((uint64_t(tmp1))|(uint64_t(tmp2)<<32));
    } break;
    default:
      fail("RealLinuxSys::Args::operator>>() for type of size %d unsupported\n",sizeof(T));
    }
  }else{
    ref=fixEndian(context->readMemRaw<T>(getStackPointer(context)+pos));
  }
  pos+=sizeof(T);
}

template<>
void RealLinuxSys<ExecModeMipsel32>::setSysErr(ThreadContext *context, Tint err){
  setReg<Tint,RegTypeGpr>(context,RegA3,Tint(1));
  setReg<Tint,RegTypeGpr>(context,RegV0,err);
}
template<>
template<typename T>
void RealLinuxSys<ExecModeMipsel32>::setSysRet(ThreadContext *context, T val){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val));
}
template<>
void RealLinuxSys<ExecModeMipsel32>::setSysRet(ThreadContext *context, Tint val1, Tint val2){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val1));
  setReg<Tregv_t,RegTypeGpr>(context,RegV1,Tregv_t(val2));
}
#endif
#if (defined SUPPORT_MIPS64)
template<>
RealLinuxSys<ExecModeMips64>::Tpointer_t RealLinuxSys<ExecModeMips64>::getStackPointer(const ThreadContext *context){
  return getReg<Tpointer_t,RegSP>(context,RegSP);
}
template<>
void RealLinuxSys<ExecModeMips64>::setStackPointer(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegSP>(context,RegSP,addr);
}

template<>
template<typename T>
void RealLinuxSys<ExecModeMips64>::CallArgs::fill(T &ref){
  if(pos<64){
    ref=getReg<T,RegTypeGpr>(context,RegA0+pos/sizeof(uint64_t));
  }else{
    ref=fixEndian(context->readMemRaw<T>(getStackPointer(context)+(pos-64)+sizeof(uint64_t)-sizeof(T)));
  }
  pos+=sizeof(uint64_t);
}

template<>
void RealLinuxSys<ExecModeMips64>::setSysErr(ThreadContext *context, Tint err){
  setReg<Tint,RegTypeGpr>(context,RegA3,Tint(1));
  setReg<Tint,RegTypeGpr>(context,RegV0,err);
}
template<>
template<typename T>
void RealLinuxSys<ExecModeMips64>::setSysRet(ThreadContext *context, T val){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val));
}
template<>
void RealLinuxSys<ExecModeMips64>::setSysRet(ThreadContext *context, Tint val1, Tint val2){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val1));
  setReg<Tregv_t,RegTypeGpr>(context,RegV1,Tregv_t(val2));
}
#endif
#if (defined SUPPORT_MIPSEL64)
template<>
RealLinuxSys<ExecModeMipsel64>::Tpointer_t RealLinuxSys<ExecModeMipsel64>::getStackPointer(const ThreadContext *context){
  return getReg<Tpointer_t,RegSP>(context,RegSP);
}
template<>
void RealLinuxSys<ExecModeMipsel64>::setStackPointer(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegSP>(context,RegSP,addr);
}

template<>
template<typename T>
void RealLinuxSys<ExecModeMipsel64>::CallArgs::fill(T &ref){
  if(pos<64){
    ref=getReg<T,RegTypeGpr>(context,RegA0+pos/sizeof(uint64_t));
  }else{
    ref=fixEndian(context->readMemRaw<T>(getStackPointer(context)+(pos-64)+sizeof(uint64_t)-sizeof(T)));
  }
  pos+=sizeof(uint64_t);
}

template<>
void RealLinuxSys<ExecModeMipsel64>::setSysErr(ThreadContext *context, Tint err){
  setReg<Tint,RegTypeGpr>(context,RegA3,Tint(1));
  setReg<Tint,RegTypeGpr>(context,RegV0,err);
}
template<>
template<typename T>
void RealLinuxSys<ExecModeMipsel64>::setSysRet(ThreadContext *context, T val){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val));
}
template<>
void RealLinuxSys<ExecModeMipsel64>::setSysRet(ThreadContext *context, Tint val1, Tint val2){
  setReg<Tregv_t,RegTypeGpr>(context,RegA3,Tregv_t(0));
  setReg<Tregv_t,RegTypeGpr>(context,RegV0,Tregv_t(val1));
  setReg<Tregv_t,RegTypeGpr>(context,RegV1,Tregv_t(val2));
}
#endif
enum{
  BaseSimTid = 1000
};

static VAddr  sysCodeAddr=AddrSpacPageSize;
static size_t sysCodeSize=6*sizeof(uint32_t);

LinuxSys *LinuxSys::create(ExecMode mode){
  switch(mode){
#if (defined SUPPORT_MIPS32)
  case ExecModeMips32:   return new RealLinuxSys<ExecModeMips32>();
#endif
#if (defined SUPPORT_MIPSEL32)
  case ExecModeMipsel32: return new RealLinuxSys<ExecModeMipsel32>();
#endif
#if (defined SUPPORT_MIPS64)
  case ExecModeMips64:   return new RealLinuxSys<ExecModeMips64>();
#endif
#if (defined SUPPORT_MIPSEL64)
  case ExecModeMipsel64: return new RealLinuxSys<ExecModeMipsel64>();
#endif
  default:
    fail("LinuxSys::create with unsupported ExecMode=%d\n",mode);
  }
  return 0;
}
template<ExecMode mode>
template<typename T>
void RealLinuxSys<mode>::pushScalar(ThreadContext *context, const T &val){
  Tpointer_t oldsp=getStackPointer(context);
  Tpointer_t newsp=oldsp+(stackGrowthSign()*sizeof(T));
  setStackPointer(context,newsp);
  context->writeMemRaw((stackGrowthSign()>0)?oldsp:newsp,fixEndian(val));
}
template<ExecMode mode>
template<typename T>
T RealLinuxSys<mode>::popScalar(ThreadContext *context){
  Tpointer_t oldsp=getStackPointer(context);
  Tpointer_t newsp=oldsp-(stackGrowthSign()*sizeof(T));
  setStackPointer(context,newsp);
  return fixEndian(context->readMemRaw<T>((stackGrowthSign()>0)?newsp:oldsp));
}
template<ExecMode mode>
template<typename T>
void RealLinuxSys<mode>::pushStruct(ThreadContext *context, const T &val){
  Tpointer_t oldsp=getStackPointer(context);
  Tpointer_t newsp=oldsp+(stackGrowthSign()*T::getSize());
  setStackPointer(context,newsp);
  val.write(context,(stackGrowthSign()>0)?oldsp:newsp);
}
template<ExecMode mode>
template<typename T>
T RealLinuxSys<mode>::popStruct(ThreadContext *context){
  Tpointer_t oldsp=getStackPointer(context);
  Tpointer_t newsp=oldsp-(stackGrowthSign()*T::getSize());
  setStackPointer(context,newsp);
  return T(context,(stackGrowthSign()>0)?newsp:oldsp);
}
typedef std::multimap<VAddr,ThreadContext *> ContextMultiMap;
ContextMultiMap futexContexts;

template<ExecMode mode>
bool RealLinuxSys<mode>::futexCheck(ThreadContext *context, Tpointer_t futex, Tint val){
  bool rv=(fixEndian(context->readMemRaw<Tint>(futex))==val);
  if(!rv)
    setSysErr(context,VEAGAIN);
  return rv;
}
template<ExecMode mode>
void RealLinuxSys<mode>::futexWait(ThreadContext *context, Tpointer_t futex){
  futexContexts.insert(ContextMultiMap::value_type(futex,context));                
#if (defined DEBUG_SIGNALS)     
  suspSet.insert(context->gettid());
#endif
  context->suspend();
}
template<ExecMode mode>
int RealLinuxSys<mode>::futexWake(ThreadContext *context, Tpointer_t futex, int nr_wake){
  int rv_wake=0;
  while((rv_wake<nr_wake)&&(futexContexts.lower_bound(futex)!=futexContexts.upper_bound(futex))){
    ThreadContext *wcontext=futexContexts.lower_bound(futex)->second;
    futexContexts.erase(futexContexts.lower_bound(futex));
    wcontext->resume();
    setSysRet(wcontext);
    rv_wake++;
  }
  return rv_wake;
}
template<ExecMode mode>
int RealLinuxSys<mode>::futexMove(ThreadContext *context, Tpointer_t srcFutex,
                                  Tpointer_t dstFutex, int nr_move){
  int rv_move=0;
  while((rv_move<nr_move)&&(futexContexts.lower_bound(srcFutex)!=futexContexts.upper_bound(srcFutex))){
    ThreadContext *wcontext=futexContexts.lower_bound(srcFutex)->second;
    futexContexts.erase(futexContexts.lower_bound(srcFutex));
    futexContexts.insert(ContextMultiMap::value_type(dstFutex,wcontext));
    rv_move++;
  }
  return rv_move;
}

bool LinuxSys::handleSignals(ThreadContext *context) const{
  while(context->hasReadySignal()){
    SigInfo *sigInfo=context->nextReadySignal();
    SignalAction sa=handleSignal(context,sigInfo);
    delete sigInfo;
    if(sa!=SigActIgnore)
      return true;
  }
  return false;
}
#if (defined SUPPORT_MIPS32)
template<>
SignalAction RealLinuxSys<ExecModeMips32>::handleSignal(ThreadContext *context, SigInfo *sigInfo) const{
  // Pop signal mask if it's been saved
  uint32_t dopop=getReg<uint32_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<uint32_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  SignalDesc &sigDesc=(*(context->getSignalTable()))[sigInfo->signo];
#if (defined DEBUG_SIGNALS)
  printf("handleSignal for pid=%d, signal=%d, action=0x%08x\n",context->gettid(),sigInfo->signo,sigDesc.handler);
#endif
  SignalAction sact((SignalAction)(sigDesc.handler));
  if(sact==SigActDefault)
    sact=getDflSigAction(sigInfo->signo);
  switch(sact){
  case SigActDefault:
    fail("Mips32LinuxSys::handleSignal signal %d with SigActDefault\n",sigInfo->signo);
  case SigActIgnore: 
    return SigActIgnore;
  case SigActCore:
  case SigActTerm:
    osSim->eventExit(context->gettid(),-1);
    return SigActTerm;
  case SigActStop:
#if (defined DEBUG_SIGNALS)
    suspSet.insert(context->gettid());
#endif
    context->suspend();
    return SigActStop;
  default:
    break;
  }
  // Save PC, then registers
  I(context->getIDesc()==context->virt2inst(context->getIAddr()));
  Tpointer_t pc(context->getIAddr());
  pushScalar(context,pc);
  for(RegName r=RegZero+1;r<=RegRA;r++){
    uint32_t wrVal=getReg<uint32_t,RegTypeGpr>(context,r);
    pushScalar(context,wrVal);
  }
  // Save the current signal mask and use sigaction's signal mask
  Tsigset_t oldMask(context->getSignalMask());
  pushStruct(context,oldMask);
  context->setSignalMask(sigDesc.mask);
  // Set registers and PC for execution of the handler
  setReg<uint32_t,RegTypeGpr>(context,RegA0,localToSigNum(sigInfo->signo));
  if(sigDesc.flags&SaSigInfo){
    printf("handleSignal with SA_SIGINFO not supported yet\n");
  }
  setReg<uint32_t,RegTypeGpr>(context,RegT9,sigDesc.handler);
  context->setIAddr(sysCodeAddr);
  return SigActHandle;
}

template<>
void RealLinuxSys<ExecModeMips32>::initSystem(ThreadContext *context) const{
  AddressSpace *addrSpace=context->getAddressSpace();
  sysCodeSize=6*sizeof(uint32_t);
  sysCodeAddr=addrSpace->newSegmentAddr(sysCodeSize);
  addrSpace->newSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),false,true,false,false,false);
  addrSpace->addFuncName(sysCodeAddr,"sysCode","");
  addrSpace->addFuncName(sysCodeAddr+sysCodeSize,"End of sysCode","");
  // jalr t9
  context->writeMemRaw(sysCodeAddr+0*sizeof(uint32_t),fixEndian(uint32_t(0x0320f809)));
  // nop
  context->writeMemRaw(sysCodeAddr+1*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  // li v0,4193 (syscall number for rt_sigreturn)
  context->writeMemRaw(sysCodeAddr+2*sizeof(uint32_t),fixEndian(uint32_t(0x24020000+4193)));
  // Syscall     
  context->writeMemRaw(sysCodeAddr+3*sizeof(uint32_t),fixEndian(uint32_t(0x0000000C)));
  // Unconditional branch to itself
  context->writeMemRaw(sysCodeAddr+4*sizeof(uint32_t),fixEndian(uint32_t(0x1000ffff)));
  // Delay slot nop
  context->writeMemRaw(sysCodeAddr+5*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  addrSpace->protectSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),true,false,true);
  // Set RegSys to zero. It is used by system call functions to indicate
  // that a signal mask has been already saved to the stack and needs to be restored
  setReg<uint32_t,RegSys>(context,RegSys,0);
}
#endif
#if (defined SUPPORT_MIPSEL32)
template<>
SignalAction RealLinuxSys<ExecModeMipsel32>::handleSignal(ThreadContext *context, SigInfo *sigInfo) const{
  // Pop signal mask if it's been saved
  uint32_t dopop=getReg<uint32_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<uint32_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  SignalDesc &sigDesc=(*(context->getSignalTable()))[sigInfo->signo];
#if (defined DEBUG_SIGNALS)
  printf("handleSignal for pid=%d, signal=%d, action=0x%08x\n",context->gettid(),sigInfo->signo,sigDesc.handler);
#endif
  SignalAction sact((SignalAction)(sigDesc.handler));
  if(sact==SigActDefault)
    sact=getDflSigAction(sigInfo->signo);
  switch(sact){
  case SigActDefault:
    fail("Mips32LinuxSys::handleSignal signal %d with SigActDefault\n",sigInfo->signo);
  case SigActIgnore: 
    return SigActIgnore;
  case SigActCore:
  case SigActTerm:
    osSim->eventExit(context->gettid(),-1);
    return SigActTerm;
  case SigActStop:
#if (defined DEBUG_SIGNALS)
    suspSet.insert(context->gettid());
#endif
    context->suspend();
    return SigActStop;
  default:
    break;
  }
  // Save PC, then registers
  I(context->getIDesc()==context->virt2inst(context->getIAddr()));
  Tpointer_t pc(context->getIAddr());
  pushScalar(context,pc);
  for(RegName r=RegZero+1;r<=RegRA;r++){
    uint32_t wrVal=getReg<uint32_t,RegTypeGpr>(context,r);
    pushScalar(context,wrVal);
  }
  // Save the current signal mask and use sigaction's signal mask
  Tsigset_t oldMask(context->getSignalMask());
  pushStruct(context,oldMask);
  context->setSignalMask(sigDesc.mask);
  // Set registers and PC for execution of the handler
  setReg<uint32_t,RegTypeGpr>(context,RegA0,localToSigNum(sigInfo->signo));
  if(sigDesc.flags&SaSigInfo){
    printf("handleSignal with SA_SIGINFO not supported yet\n");
  }
  setReg<uint32_t,RegTypeGpr>(context,RegT9,sigDesc.handler);
  context->setIAddr(sysCodeAddr);
  return SigActHandle;
}

template<>
void RealLinuxSys<ExecModeMipsel32>::initSystem(ThreadContext *context) const{
  AddressSpace *addrSpace=context->getAddressSpace();
  sysCodeSize=6*sizeof(uint32_t);
  sysCodeAddr=addrSpace->newSegmentAddr(sysCodeSize);
  addrSpace->newSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),false,true,false,false,false);
  addrSpace->addFuncName(sysCodeAddr,"sysCode","");
  addrSpace->addFuncName(sysCodeAddr+sysCodeSize,"End of sysCode","");
  // jalr t9
  context->writeMemRaw(sysCodeAddr+0*sizeof(uint32_t),fixEndian(uint32_t(0x0320f809)));
  // nop
  context->writeMemRaw(sysCodeAddr+1*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  // li v0,4193 (syscall number for rt_sigreturn)
  context->writeMemRaw(sysCodeAddr+2*sizeof(uint32_t),fixEndian(uint32_t(0x24020000+4193)));
  // Syscall     
  context->writeMemRaw(sysCodeAddr+3*sizeof(uint32_t),fixEndian(uint32_t(0x0000000C)));
  // Unconditional branch to itself
  context->writeMemRaw(sysCodeAddr+4*sizeof(uint32_t),fixEndian(uint32_t(0x1000ffff)));
  // Delay slot nop
  context->writeMemRaw(sysCodeAddr+5*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  addrSpace->protectSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),true,false,true);
  // Set RegSys to zero. It is used by system call functions to indicate
  // that a signal mask has been already saved to the stack and needs to be restored
  setReg<uint32_t,RegSys>(context,RegSys,0);
}
#endif
#if (defined SUPPORT_MIPS64)
template<>
SignalAction RealLinuxSys<ExecModeMips64>::handleSignal(ThreadContext *context, SigInfo *sigInfo) const{
  // Pop signal mask if it's been saved
  uint32_t dopop=getReg<uint32_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<uint32_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  SignalDesc &sigDesc=(*(context->getSignalTable()))[sigInfo->signo];
#if (defined DEBUG_SIGNALS)
  printf("handleSignal for pid=%d, signal=%d, action=0x%08x\n",context->gettid(),sigInfo->signo,sigDesc.handler);
#endif
  SignalAction sact((SignalAction)(sigDesc.handler));
  if(sact==SigActDefault)
    sact=getDflSigAction(sigInfo->signo);
  switch(sact){
  case SigActDefault:
    fail("Mips32LinuxSys::handleSignal signal %d with SigActDefault\n",sigInfo->signo);
  case SigActIgnore: 
    return SigActIgnore;
  case SigActCore:
  case SigActTerm:
    osSim->eventExit(context->gettid(),-1);
    return SigActTerm;
  case SigActStop:
#if (defined DEBUG_SIGNALS)
    suspSet.insert(context->gettid());
#endif
    context->suspend();
    return SigActStop;
  default:
    break;
  }
  // Save PC, then registers
  I(context->getIDesc()==context->virt2inst(context->getIAddr()));
  Tpointer_t pc(context->getIAddr());
  pushScalar(context,pc);
  for(RegName r=RegZero+1;r<=RegRA;r++){
    uint64_t wrVal=getReg<uint64_t,RegTypeGpr>(context,r);
    pushScalar(context,wrVal);
  }
  // Save the current signal mask and use sigaction's signal mask
  Tsigset_t oldMask(context->getSignalMask());
  pushStruct(context,oldMask);
  context->setSignalMask(sigDesc.mask);
  // Set registers and PC for execution of the handler
  setReg<Tregv_t,RegTypeGpr>(context,RegA0,localToSigNum(sigInfo->signo));
  if(sigDesc.flags&SaSigInfo){
    printf("handleSignal with SA_SIGINFO not supported yet\n");
  }
  setReg<Tregv_t,RegTypeGpr>(context,RegT9,sigDesc.handler);
  context->setIAddr(sysCodeAddr);
  return SigActHandle;
}

template<>
void RealLinuxSys<ExecModeMips64>::initSystem(ThreadContext *context) const{
  AddressSpace *addrSpace=context->getAddressSpace();
  sysCodeSize=6*sizeof(uint32_t);
  sysCodeAddr=addrSpace->newSegmentAddr(sysCodeSize);
  addrSpace->newSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),false,true,false,false,false);
  addrSpace->addFuncName(sysCodeAddr,"sysCode","");
  addrSpace->addFuncName(sysCodeAddr+sysCodeSize,"End of sysCode","");
  // jalr t9
  context->writeMemRaw(sysCodeAddr+0*sizeof(uint32_t),fixEndian(uint32_t(0x0320f809)));
  // nop
  context->writeMemRaw(sysCodeAddr+1*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  // li v0,V__NR_rt_sigreturn
  context->writeMemRaw(sysCodeAddr+2*sizeof(uint32_t),fixEndian(uint32_t(0x24020000+V__NR_rt_sigreturn)));
  // Syscall     
  context->writeMemRaw(sysCodeAddr+3*sizeof(uint32_t),fixEndian(uint32_t(0x0000000C)));
  // Unconditional branch to itself
  context->writeMemRaw(sysCodeAddr+4*sizeof(uint32_t),fixEndian(uint32_t(0x1000ffff)));
  // Delay slot nop
  context->writeMemRaw(sysCodeAddr+5*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  addrSpace->protectSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),true,false,true);
  // Set RegSys to zero. It is used by system call functions to indicate
  // that a signal mask has been already saved to the stack and needs to be restored
  setReg<Tregv_t,RegSys>(context,RegSys,0);
}
#endif
#if (defined SUPPORT_MIPSEL64)
template<>
SignalAction RealLinuxSys<ExecModeMipsel64>::handleSignal(ThreadContext *context, SigInfo *sigInfo) const{
  // Pop signal mask if it's been saved
  uint32_t dopop=getReg<uint32_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<uint32_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  SignalDesc &sigDesc=(*(context->getSignalTable()))[sigInfo->signo];
#if (defined DEBUG_SIGNALS)
  printf("handleSignal for pid=%d, signal=%d, action=0x%08x\n",context->gettid(),sigInfo->signo,sigDesc.handler);
#endif
  SignalAction sact((SignalAction)(sigDesc.handler));
  if(sact==SigActDefault)
    sact=getDflSigAction(sigInfo->signo);
  switch(sact){
  case SigActDefault:
    fail("Mips32LinuxSys::handleSignal signal %d with SigActDefault\n",sigInfo->signo);
  case SigActIgnore: 
    return SigActIgnore;
  case SigActCore:
  case SigActTerm:
    osSim->eventExit(context->gettid(),-1);
    return SigActTerm;
  case SigActStop:
#if (defined DEBUG_SIGNALS)
    suspSet.insert(context->gettid());
#endif
    context->suspend();
    return SigActStop;
  default:
    break;
  }
  // Save PC, then registers
  I(context->getIDesc()==context->virt2inst(context->getIAddr()));
  Tpointer_t pc(context->getIAddr());
  pushScalar(context,pc);
  for(RegName r=RegZero+1;r<=RegRA;r++){
    uint64_t wrVal=getReg<uint64_t,RegTypeGpr>(context,r);
    pushScalar(context,wrVal);
  }
  // Save the current signal mask and use sigaction's signal mask
  Tsigset_t oldMask(context->getSignalMask());
  pushStruct(context,oldMask);
  context->setSignalMask(sigDesc.mask);
  // Set registers and PC for execution of the handler
  setReg<Tregv_t,RegTypeGpr>(context,RegA0,localToSigNum(sigInfo->signo));
  if(sigDesc.flags&SaSigInfo){
    printf("handleSignal with SA_SIGINFO not supported yet\n");
  }
  setReg<Tregv_t,RegTypeGpr>(context,RegT9,sigDesc.handler);
  context->setIAddr(sysCodeAddr);
  return SigActHandle;
}

template<>
void RealLinuxSys<ExecModeMipsel64>::initSystem(ThreadContext *context) const{
  AddressSpace *addrSpace=context->getAddressSpace();
  sysCodeSize=6*sizeof(uint32_t);
  sysCodeAddr=addrSpace->newSegmentAddr(sysCodeSize);
  addrSpace->newSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),false,true,false,false,false);
  addrSpace->addFuncName(sysCodeAddr,"sysCode","");
  addrSpace->addFuncName(sysCodeAddr+sysCodeSize,"End of sysCode","");
  // jalr t9
  context->writeMemRaw(sysCodeAddr+0*sizeof(uint32_t),fixEndian(uint32_t(0x0320f809)));
  // nop
  context->writeMemRaw(sysCodeAddr+1*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  // li v0,V__NR_rt_sigreturn
  context->writeMemRaw(sysCodeAddr+2*sizeof(uint32_t),fixEndian(uint32_t(0x24020000+V__NR_rt_sigreturn)));
  // Syscall     
  context->writeMemRaw(sysCodeAddr+3*sizeof(uint32_t),fixEndian(uint32_t(0x0000000C)));
  // Unconditional branch to itself
  context->writeMemRaw(sysCodeAddr+4*sizeof(uint32_t),fixEndian(uint32_t(0x1000ffff)));
  // Delay slot nop
  context->writeMemRaw(sysCodeAddr+5*sizeof(uint32_t),fixEndian(uint32_t(0x00000000)));
  addrSpace->protectSegment(sysCodeAddr,alignUp(sysCodeSize,addrSpace->getPageSize()),true,false,true);
  // Set RegSys to zero. It is used by system call functions to indicate
  // that a signal mask has been already saved to the stack and needs to be restored
  setReg<Tregv_t,RegSys>(context,RegSys,0);
}
#endif
template<ExecMode mode>
void RealLinuxSys<mode>::createStack(ThreadContext *context) const{
  AddressSpace *addrSpace=context->getAddressSpace();
  I(addrSpace);
  // Stack size starts at 16MB, but can autogrow
  size_t stackSize=0x1000000;
  VAddr  stackStart=addrSpace->newSegmentAddr(stackSize);
  I(stackStart);
  // Stack is created with read/write permissions, and autogrows down
  addrSpace->newSegment(stackStart,stackSize,true,true,false);
  addrSpace->setGrowth(stackStart,true,true);
  context->writeMemWithByte(stackStart,stackSize,0);
  context->setStack(stackStart,stackStart+stackSize);
  setStackPointer(context,stackStart+((stackGrowthSign()>0)?0:stackSize));
}

template<ExecMode mode>
void RealLinuxSys<mode>::setProgArgs(ThreadContext *context, int argc, char **argv, int envc, char **envp) const{
  // We will push arg and env string pointer arrays later, with nulls at end
  Tpointer_t envVAddrs[envc+1];
  I(sizeof(envVAddrs)==(envc+1)*sizeof(Tpointer_t));
  Tpointer_t argVAddrs[argc+1];
  I(sizeof(argVAddrs)==(argc+1)*sizeof(Tpointer_t));
  // Put the env strings on the stack and initialize the envVAddrs array
  for(int envIdx=envc-1;envIdx>=0;envIdx--){
    Tsize_t    strSize=strlen(envp[envIdx])+1;
    Tpointer_t oldsp=getStackPointer(context);
    Tpointer_t newsp=oldsp+stackGrowthSign()*strSize;
    setStackPointer(context,newsp);
    Tpointer_t strAddr=(stackGrowthSign()>0)?oldsp:newsp;
    context->writeMemFromBuf(strAddr,strSize,envp[envIdx]);
    envVAddrs[envIdx]=strAddr;
  }
  envVAddrs[envc]=0;
  // Put the arg strings on the stack and initialize the argVAddrs array
  for(int argIdx=argc-1;argIdx>=0;argIdx--){
    Tsize_t strSize=strlen(argv[argIdx])+1;
    Tpointer_t oldsp=getStackPointer(context);
    Tpointer_t newsp=oldsp+stackGrowthSign()*strSize;
    setStackPointer(context,newsp);
    Tpointer_t strAddr=(stackGrowthSign()>0)?oldsp:newsp;
    context->writeMemFromBuf(strAddr,strSize,argv[argIdx]);
    argVAddrs[argIdx]=strAddr;
  }
  argVAddrs[argc]=0;
  // Put the aux vector on the stack
  // Align stack pointer to Tauxv_t granularity
  Tpointer_t lomask=Tauxv_t::getSize()-1;
  setStackPointer(context,(getStackPointer(context)+((stackGrowthSign()>0)?lomask:0))&(~lomask));
  // Push aux vector elements
  pushStruct(context,Tauxv_t(AT_NULL,0));
  pushStruct(context,Tauxv_t(AT_PHNUM,context->getAddressSpace()->getFuncAddr("PrgHdrNum")));
  pushStruct(context,Tauxv_t(AT_PHENT,context->getAddressSpace()->getFuncAddr("PrgHdrEnt")));
  pushStruct(context,Tauxv_t(AT_PHDR,context->getAddressSpace()->getFuncAddr("PrgHdrAddr")));
  pushStruct(context,Tauxv_t(AT_PAGESZ,AddrSpacPageSize));
  pushStruct(context,Tauxv_t(AT_ENTRY,context->getAddressSpace()->getFuncAddr("UserEntry")));
  // Put the envp array (with NULL at the end) on the stack
  for(int envi=envc;envi>=0;envi--)
    pushScalar(context,envVAddrs[envi]);
  // Put the argv array (with NULL at the end) on the stack
  for(int argi=argc;argi>=0;argi--)
    pushScalar(context,argVAddrs[argi]);
  // Put the argc on the stack
  pushScalar(context,Tsize_t(argc));
}

#include "OSSim.h"

template<ExecMode mode>
void RealLinuxSys<mode>::sysClone(ThreadContext *context, InstDesc *inst){
  Tint       flags;
  Tpointer_t child_stack;
  Tpointer_t ptid;
  Tpointer_t tarea;
  Tpointer_t ctid;
  CallArgs(context) >> flags >> child_stack >> ptid >> tarea >> ctid;
  if(flags&VCLONE_VFORK)
    fail("sysCall32_clone with CLONE_VFORK not supported yet at 0x%08x, flags=0x%08x\n",context->getIAddr(),flags);
  if(flags&VCLONE_UNTRACED)
    fail("sysCall32_clone with CLONE_UNTRACED not supported yet at 0x%08x, flags=0x%08x\n",context->getIAddr(),flags);
  if(flags&VCLONE_STOPPED)
    fail("sysCall32_clone with CLONE_STOPPED not supported yet at 0x%08x, flags=0x%08x\n",context->getIAddr(),flags);
  SignalID sig=(flags&VCLONE_DETACHED)?SigDetached:sigNumToLocal(flags&VCSIGNAL);
  ThreadContext *newContext= new ThreadContext(*context,
					       flags&VCLONE_PARENT,flags&VCLONE_FS,flags&VCLONE_NEWNS,
					       flags&VCLONE_FILES,flags&VCLONE_SIGHAND,
					       flags&VCLONE_VM,flags&VCLONE_THREAD,
					       sig,(flags&VCLONE_CHILD_CLEARTID)?ctid:0);
  I(newContext!=0);
  // Fork returns an error only if there is no memory, which should not happen here
  // Set return values for parent and child
  setSysRet(context,newContext->gettid()+BaseSimTid);
  setSysRet(newContext);
  if(child_stack){
    newContext->setStack(newContext->getAddressSpace()->getSegmentAddr(child_stack),child_stack);
    setStackPointer(newContext,child_stack);
#if (defined DEBUG_BENCH)
    newContext->clearCallStack();
#endif
  }else{
    I(!(flags&VCLONE_VM));
  }
  if(flags&VCLONE_PARENT_SETTID)
    context->writeMemRaw(ptid,fixEndian(Tpid_t(newContext->gettid()+BaseSimTid)));
  if(flags&VCLONE_CHILD_SETTID)
    newContext->writeMemRaw(ctid,fixEndian(Tpid_t(newContext->gettid()+BaseSimTid)));
  if(flags&VCLONE_SETTLS)
    setThreadArea(newContext,tarea);
  // Inform SESC that this process is created
  osSim->eventSpawn(-1,newContext->gettid(),0);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysFork(ThreadContext *context, InstDesc *inst){
  ThreadContext *newContext=new ThreadContext(*context,false,false,false,false,false,false,false,SigChld,0);
  I(newContext!=0);
  // Fork returns an error only if there is no memory, which should not happen here
  // Set return values for parent and child
  setSysRet(context,newContext->gettid()+BaseSimTid);
  setSysRet(newContext);
  // Inform SESC that this process is created
  osSim->eventSpawn(-1,newContext->gettid(),0);
}
template<ExecMode mode>
InstDesc *RealLinuxSys<mode>::sysExecVe(ThreadContext *context, InstDesc *inst){
  Tpointer_t fnamep;
  Tpointer_t argv;
  Tpointer_t envp;
  CallArgs(context) >> fnamep >> argv >> envp;
  Tstr fname(context,fnamep);
  if(!fname)
    return inst;
  const std::string exeLinkName(context->getFileSys()->toHost((const char *)fname));
  const std::string exeRealName(FileSys::Node::resolve(exeLinkName));
  if(exeRealName.empty()){
    setSysErr(context,VELOOP);
    return inst;
  }
  FileSys::Node *node=FileSys::Node::lookup(exeRealName);
  if(!node){
    setSysErr(context,VENOENT);
    return inst;
  }
  FileSys::FileNode *fnode=dynamic_cast<FileSys::FileNode *>(node);
  if(!fnode){
    setSysErr(context,VEACCES);
    return inst;
  }
  FileSys::FileDescription *fdesc=new FileSys::FileDescription(fnode,O_RDONLY);
  FileSys::Description::pointer pdesc(fdesc);
  ExecMode emode=getExecMode(fdesc);
  if(emode==ExecModeNone){
    setSysErr(context,VENOEXEC);
    return inst;
  }
  // Pipeline flush to avoid mixing old and new InstDesc's in the pipeline
  if(context->getNDInsts()){
    context->updIAddr(-inst->aupdate,-1);
    return 0;
  }
  // Find the length of the argv list
  size_t argvNum=0;
  size_t argvLen=0;
  while(true){
    Tpointer_t argAddr=fixEndian(context->readMemRaw<Tpointer_t>(argv+sizeof(Tpointer_t)*argvNum));
    if(!argAddr)
      break;
    ssize_t argLen=context->readMemString(argAddr,0,0);
    I(argLen>=1);
    argvLen+=argLen;
    argvNum++;
  }
  // Find the length of the envp list
  size_t envpNum=0;
  size_t envpLen=0;
  while(true){
    Tpointer_t envAddr=fixEndian(context->readMemRaw<Tpointer_t>(envp+sizeof(Tpointer_t)*envpNum));
    if(!envAddr)
      break;
    ssize_t envLen=context->readMemString(envAddr,0,0);
    I(envLen>=1);
    envpLen+=envLen;
    envpNum++;
  }
  char *argvArr[argvNum];
  char argvBuf[argvLen];
  char *argvPtr=argvBuf;
  for(size_t arg=0;arg<argvNum;arg++){
    Tpointer_t argAddr=fixEndian(context->readMemRaw<Tpointer_t>(argv+sizeof(Tpointer_t)*arg));
    ssize_t argLen=context->readMemString(argAddr,0,0);
    I(argLen>=1);
    context->readMemString(argAddr,argLen,argvPtr);
    argvArr[arg]=argvPtr;
    argvPtr+=argLen;
  }
  char *envpArr[envpNum];
  char envpBuf[envpLen];
  char *envpPtr=envpBuf;
  for(size_t env=0;env<envpNum;env++){
    Tpointer_t envAddr=fixEndian(context->readMemRaw<Tpointer_t>(envp+sizeof(Tpointer_t)*env));
    ssize_t envLen=context->readMemString(envAddr,0,0);
    I(envLen>=1);
    context->readMemString(envAddr,envLen,envpPtr);
    envpArr[env]=envpPtr;
    envpPtr+=envLen;
  }
#if (defined DEBUG_BENCH)
  printf("execve fname is %s\n",realName);
  for(size_t arg=0;arg<argvNum;arg++){
    printf("execve argv[%ld] is %s\n",(long int)arg,argvArr[arg]);
  }
  for(size_t env=0;env<envpNum;env++){
    printf("execve envp[%ld] is %s\n",(long int)env,envpArr[env]);
  }
#endif
  // Close files that are still open and are cloexec
  context->getOpenFiles()->exec();
  // Clear up the address space and load the new object file
  context->setAddressSpace(new AddressSpace());
//  context->getAddressSpace()->clear(true);
  // TODO: Use ELF_ET_DYN_BASE instead of a constant here
  loadElfObject(context,fdesc,0x200000);
  // We need to go thorugh getSystem() because the execution mode may be different
  context->getSystem()->initSystem(context);
  context->getSystem()->createStack(context);
  context->getSystem()->setProgArgs(context,argvNum,argvArr,envpNum,envpArr);
#if (defined DEBUG_BENCH)
  context->clearCallStack();
#endif
  // The InstDesc is gone now and we can't put it through the pipeline
  return 0;
}
template<ExecMode mode>
InstDesc *RealLinuxSys<mode>::sysExit(ThreadContext *context, InstDesc *inst){
  Tint status;
  CallArgs(context) >> status;
  if(context->exit(status))
    return 0;
#if (defined DEBUG_SIGNALS)
  printf("Suspend %d in sysExit\n",context->gettid());
  context->dumpCallStack();
  printf("Also suspended:");
  for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
    printf(" %d",*suspIt);
  printf("\n");
  suspSet.insert(context->gettid());
#endif
  // Deliver the exit signal to the parent process if needed
  SignalID exitSig=context->getExitSig();
  // If no signal to be delivered, just wait to be reaped
  if(exitSig==SigDetached){
    context->reap();
    return 0;
  }
  if(exitSig!=SigNone){
    SigInfo *sigInfo=new SigInfo(exitSig,SigCodeChldExit);
    sigInfo->pid=context->gettid();
    sigInfo->data=status;
    ThreadContext::getContext(context->getParentID())->signal(sigInfo);
  }
  return inst;
}
template<ExecMode mode>
InstDesc *RealLinuxSys<mode>::sysExitGroup(ThreadContext *context, InstDesc *inst){
  if(context->gettgtids(0,0)>0)
    fail("RealLinuxSys::sysExitGroup can't handle live thread grup members\n");
  return sysExit(context,inst);
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysGetPid(ThreadContext *context, InstDesc *inst){
  setSysRet(context,context->gettgid()+BaseSimTid);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetTid(ThreadContext *context, InstDesc *inst){
  setSysRet(context,context->gettid()+BaseSimTid);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetPPid(ThreadContext *context, InstDesc *inst){
  setSysRet(context,context->getppid()+BaseSimTid);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetPGid(ThreadContext *context, InstDesc *inst){
  Tpid_t pid;
  CallArgs(context) >> pid;
  ThreadContext *ccontext=(pid==0)?context:osSim->getContext(pid-BaseSimTid);
  if(!ccontext)
    return setSysErr(context,VESRCH);
  setSysRet(context,Tpid_t(ccontext->getpgid()+BaseSimTid));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetPGrp(ThreadContext *context, InstDesc *inst){
  setSysRet(context,context->getpgid()+BaseSimTid);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysFutex(ThreadContext *context, InstDesc *inst){
  Tpointer_t futex;
  Tuint      op;
  CallArgs   args(context);
  args >> futex>> op;
  // Ignore FUTEX_PRIVATE_FLAG and FUTEX_CLOCK_REALTIME
  switch(op&VFUTEX_CMD_MASK){
  case VFUTEX_WAIT_BITSET:
  case VFUTEX_WAIT: {
    Tint       val;
    Tpointer_t timeout;
    args >> val >> timeout;
    if(timeout)
      fail("LinuxSys::sysFutex non-zero timeout unsupported for FUTEX_WAIT");
    if(futexCheck(context,futex,val))
      futexWait(context,futex);
  } break;
  case VFUTEX_WAKE_BITSET:
  case VFUTEX_WAKE: {
    Tint       nr_wake;
    Tpointer_t timeout;
    args >> nr_wake >> timeout;
    if(timeout)
      fail("LinuxSys::sysFutex non-zero timeout unsupported for FUTEX_WAKE");
    setSysRet(context,futexWake(context,futex,nr_wake));
  } break;
  case VFUTEX_REQUEUE: {
    Tint       nr_wake;
    Tint       nr_move;
    Tpointer_t futex2;
    args >> nr_wake >> nr_move >> futex2;
    setSysRet(context,futexWake(context,futex,nr_wake)+futexMove(context,futex,futex2,nr_move));
  } break;
  case VFUTEX_CMP_REQUEUE: {
    Tint       nr_wake;
    Tint       nr_move;
    Tpointer_t futex2;
    Tint       val;
    args >> nr_wake >> nr_move >> futex2 >> val;
    if(futexCheck(context,futex,val))
      setSysRet(context,futexWake(context,futex,nr_wake)+futexMove(context,futex,futex2,nr_move));
  } break;
  case VFUTEX_WAKE_OP: {
    Tint       nr_wake;
    Tint       nr_wake2;
    Tpointer_t futex2;
    Tuint      wakeop;
    args >> nr_wake >> nr_wake2 >> futex2 >> wakeop;
    if(!context->canRead(futex2,sizeof(Tint))||!context->canWrite(futex2,sizeof(Tint)))
      return setSysErr(context,VEFAULT);
    Tuint atmop=(wakeop>>28)&0x07;
    Tuint cmpop=(wakeop>>24)&0x0f;
    Tint  atmarg=((Tint(wakeop)<<8)>>20);
    Tint  cmparg=((Tint(wakeop)<<20)>>20);
    if((wakeop>>28)&VFUTEX_OP_OPARG_SHIFT)
      atmarg=1<<atmarg;
    Tint val=fixEndian(context->readMemRaw<Tint>(futex2));
    bool cond;
    switch(cmpop){
    case VFUTEX_OP_CMP_EQ: cond=(val==cmparg); break;
    case VFUTEX_OP_CMP_NE: cond=(val!=cmparg); break;
    case VFUTEX_OP_CMP_LT: cond=(val< cmparg); break;
    case VFUTEX_OP_CMP_LE: cond=(val<=cmparg); break;
    case VFUTEX_OP_CMP_GT: cond=(val> cmparg); break;
    case VFUTEX_OP_CMP_GE: cond=(val>=cmparg); break;
    default:
      return setSysErr(context,VENOSYS);
    }
    switch(atmop){
    case VFUTEX_OP_SET:  val = atmarg; break;
    case VFUTEX_OP_ADD:  val+= atmarg; break;
    case VFUTEX_OP_OR:   val|= atmarg; break;
    case VFUTEX_OP_ANDN: val&=~atmarg; break;
    case VFUTEX_OP_XOR:  val^= atmarg; break;
    default:
      return setSysErr(context,VENOSYS);
    }
    context->writeMemRaw(futex2,fixEndian(val));
    setSysRet(context,futexWake(context,futex,nr_wake)+(cond?futexWake(context,futex2,nr_wake2):0));
  } break;
  default:
    fail("LinuxSys::sysFutex with unsupported op=%d\n",(int)op);
  }
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysSetRobustList(ThreadContext *context, InstDesc *inst){
  Tpointer_t head;
  Tsize_t    len;
  CallArgs(context) >> head >> len;
  if(len!=Trobust_list_head::getSize())
    return setSysErr(context,VEINVAL);
  context->setRobustList(head);
  setSysRet(context,0);
}
template<ExecMode mode>
void RealLinuxSys<mode>::exitRobustList(ThreadContext *context, VAddr robust_list){
  // Any errors that occur here are silently ignored
  if(!context->canRead(robust_list,Trobust_list_head::getSize()))
    return;
  Trobust_list_head head(context,robust_list);
  if(head.list.next!=robust_list)
    printf("RealLinuxSys::exitRobustList called with non-empty robust_list\n");
}
template<ExecMode mode>
void RealLinuxSys<mode>::clearChildTid(ThreadContext *context, VAddr &clear_child_tid){
  if(!clear_child_tid)
    return;
  context->writeMemRaw(clear_child_tid,fixEndian(Tint(0)));
  futexWake(context,clear_child_tid,1);
  clear_child_tid=0;
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSetTidAddress(ThreadContext *context, InstDesc *inst){
  Tpointer_t tidptr;
  CallArgs(context) >> tidptr;
  context->setTidAddress(tidptr);
  return setSysRet(context,Tpid_t(context->gettid()+BaseSimTid));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetUid(ThreadContext *context, InstDesc *inst){
  // TODO: With different users, need to track simulated uid
  return setSysRet(context,Tuid_t(getuid()));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetEuid(ThreadContext *context, InstDesc *inst){
  // TODO: With different users, need to track simulated euid
  return setSysRet(context,Tuid_t(geteuid()));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetGid(ThreadContext *context, InstDesc *inst){
  // TODO: With different users, need to track simulated gid
  return setSysRet(context,Tgid_t(getgid()));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetEgid(ThreadContext *context, InstDesc *inst){
  // TODO: With different users, need to track simulated egid
  return setSysRet(context,Tgid_t(getegid()));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetGroups(ThreadContext *context, InstDesc *inst){
  Tsize_t    size;
  Tpointer_t list;
  CallArgs(context) >> size >> list;
#if (defined DEBUG_BENCH)
  printf("sysCall32_getgroups(%ld,0x%08x)called\n",(long)size,list);
#endif
  setSysRet(context);
}

template<ExecMode mode>
void RealLinuxSys<mode>::doWait4(ThreadContext *context, InstDesc *inst,
                                 Tpid_t cpid, Tpointer_t status,
                                 Tuint options, Tpointer_t rusage){
  int realCTid=0;
  if(cpid>0){
    realCTid=cpid-BaseSimTid;
    if(!context->isChildID(realCTid))
      return setSysErr(context,VECHILD);
    ThreadContext *ccontext=osSim->getContext(realCTid);
    if((!ccontext->isExited())&&(!ccontext->isKilled()))
      realCTid=0;
  }else if(cpid==-1){
    if(!context->hasChildren())
      return setSysErr(context,VECHILD);
    realCTid=context->findZombieChild();
  }else{
    fail("LinuxSys::doWait4 Only supported for pid -1 or >0\n");
  }
  if(realCTid){
    ThreadContext *ccontext=osSim->getContext(realCTid);
    if(status){
      Tint statVal=0xDEADBEEF;
      if(ccontext->isExited()){
        statVal=(ccontext->getExitCode()<<8);
      }else{
        I(ccontext->isKilled());
        fail("LinuxSys::doWait4 for killed child not supported\n");
      }
      context->writeMemRaw(status,fixEndian(statVal));
    }
    if(rusage)
      fail("LinuxSys::doWait4 with rusage parameter not supported\n");
#if (defined DEBUG_SIGNALS)
    suspSet.erase(pid);
#endif
    ccontext->reap();
    return setSysRet(context,Tpid_t(realCTid+BaseSimTid));
  }
  if(options&VWNOHANG)
    return setSysRet(context);
  context->updIAddr(-inst->aupdate,-1);
  I(inst==context->getIDesc());
  I(inst==context->virt2inst(context->getIAddr()));
#if (defined DEBUG_SIGNALS)
  printf("Suspend %d in sysCall32_wait4(pid=%d,status=%x,options=%x)\n",
         context->gettid(),pid,status,options.val);
  printf("Also suspended:");
  for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
    printf(" %d",*suspIt);
  printf("\n");
  suspSet.insert(context->gettid());
#endif
  context->suspend();
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysWait4(ThreadContext *context, InstDesc *inst){
  Tpid_t     pid;
  Tpointer_t status;
  Tuint      options;
  Tpointer_t rusage;
  CallArgs(context) >> pid >> status >> options >> rusage;
  doWait4(context,inst,pid,status,options,rusage);
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysWaitpid(ThreadContext *context, InstDesc *inst){
  Tpid_t     pid;
  Tpointer_t status;
  Tuint      options;
  Tpointer_t rusage;
  CallArgs(context) >> pid >> status >> options;
  doWait4(context,inst,pid,status,options,0);
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysKill(ThreadContext *context, InstDesc *inst){
  Tpid_t   pid;
  Tint     sig;
  CallArgs(context) >> pid >> sig;
  if(pid<=0)
    fail("sysKill with pid=%d\n",pid);
  ThreadContext *kcontext=osSim->getContext(pid-BaseSimTid);
  if(!kcontext)
    return setSysErr(context,VESRCH);
  SigInfo *sigInfo=new SigInfo(sigNumToLocal(sig),SigCodeUser);
  sigInfo->pid=context->gettid();
  kcontext->signal(sigInfo);
#if (defined DEBUG_SIGNALS)
  printf("sysCall32_kill: signal %d sent from process %d to %d\n",sig,context->gettid(),pid);
#endif
  setSysRet(context);
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysTKill(ThreadContext *context, InstDesc *inst){
  Tpid_t   tid;
  Tint     sig;
  CallArgs(context) >> tid >> sig;
  if(tid<=0)
    fail("sysTKill with tid=%d\n",tid);
  ThreadContext *kcontext=osSim->getContext(tid-BaseSimTid);
  if(!kcontext)
    return setSysErr(context,VESRCH);
  SigInfo *sigInfo=new SigInfo(sigNumToLocal(sig),SigCodeUser);
  sigInfo->pid=context->gettid();
  kcontext->signal(sigInfo);
#if (defined DEBUG_SIGNALS)
  printf("sysCall32_tkill: signal %d sent from process %d to thread %d\n",sig.val,context->gettid(),tid);
#endif
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysTgKill(ThreadContext *context, InstDesc *inst){
  Tpid_t   tgid;
  Tpid_t   pid;
  Tint     sig;
  CallArgs(context) >> tgid >> pid >> sig;
  if(pid<=0)
    fail("sysTgKill with pid=%d\n",pid);
  ThreadContext *kcontext=osSim->getContext(pid-BaseSimTid);
  if(!kcontext)
    return setSysErr(context,VESRCH);
  if(kcontext->gettgid()!=tgid)
    return setSysErr(context,VESRCH);
  SigInfo *sigInfo=new SigInfo(sigNumToLocal(sig),SigCodeUser);
  sigInfo->pid=context->gettid();
  kcontext->signal(sigInfo);
#if (defined DEBUG_SIGNALS)
  printf("sysCall32_tgkill: signal %d sent from process %d to thread %d in %d\n",sig.val,context->gettid(),pid,tgid);
#endif
  setSysRet(context);
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysRtSigAction(ThreadContext *context, InstDesc *inst){
  Tint       sig;
  Tpointer_t act;
  Tpointer_t oact;
  Tsize_t    size;
  CallArgs(context) >> sig >> act >> oact >> size;
  if(size!=Tsigset_t::getSize())
    return setSysErr(context,VEINVAL);
  if((act)&&(!context->canRead(act,Tsigaction::getSize())))
    return setSysErr(context,VEFAULT);
  if((oact)&&!context->canWrite(oact,Tsigaction::getSize()))
    return setSysErr(context,VEFAULT);
  SignalID localSig(sigNumToLocal(sig));
  SignalTable *sigTable=context->getSignalTable();
  SignalDesc  &sigDesc=(*sigTable)[localSig];
  // Get the existing signal handler into oactBuf
  if(oact)
    Tsigaction(sigDesc).write(context,oact);
  if(act){
    sigDesc=Tsigaction(context,act);
    // Without SA_NODEFER, mask signal out in its own handler
    if(!(sigDesc.flags&SaNoDefer))
      sigDesc.mask.set(localSig);
  }
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysRtSigProcMask(ThreadContext *context, InstDesc *inst){
  Tint       how;
  Tpointer_t nset;
  Tpointer_t oset;
  Tsize_t    size;
  CallArgs(context) >> how >> nset >> oset >> size;
  if(size!=Tsigset_t::getSize())
    fail("sysRtSigProcMask: mask size mismatch\n");
  if((nset)&&(!context->canRead(nset,size)))      
    return setSysErr(context,VEFAULT);
  if((oset)&&(!context->canWrite(oset,size)))
    return setSysErr(context,VEFAULT);
  SignalSet oldMask=context->getSignalMask();
  if(oset)
    Tsigset_t(oldMask).write(context,oset);
  if(nset){
    SignalSet lset(Tsigset_t(context,nset));
    switch(how){
    case VSIG_BLOCK:   context->setSignalMask(oldMask|lset);        break;
    case VSIG_UNBLOCK: context->setSignalMask((oldMask|lset)^lset); break;
    case VSIG_SETMASK: context->setSignalMask(lset);                break;
    default: fail("sysRtSigProcMask: Unsupported value %d of how\n",how);
    }
  }
  setSysRet(context);
}
#if (defined SUPPORT_MIPS32)
template<>
void RealLinuxSys<ExecModeMips32>::sysRtSigSuspend(ThreadContext *context, InstDesc *inst){
  // If this is a suspend following a wakeup, we need to pop the already-saved mask
  uint32_t dopop=getReg<uint32_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<uint32_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  Tpointer_t nset;
  Tsize_t    size;
  CallArgs(context) >> nset >> size;
  if(size!=Tsigset_t::getSize())
    fail("sysRtSigProcMask: mask size mismatch\n");
  if(!context->canRead(nset,size))
    return setSysErr(context,VEFAULT);
  // Change signal mask while suspended
  SignalSet oldMask=context->getSignalMask();
  SignalSet newMask(Tsigset_t(context,nset));
  context->setSignalMask(newMask);
  if(context->hasReadySignal()){
    SigInfo *sigInfo=context->nextReadySignal();
    context->setSignalMask(oldMask);
    handleSignal(context,sigInfo);
    delete sigInfo;
    return setSysErr(context,VEINTR);
  }
#if (defined DEBUG_SIGNALS)
  Tpid_t pid=context->gettid();
  printf("Suspend %d in sysCall32_rt_sigsuspend\n",pid);
  context->dumpCallStack();
  printf("Also suspended:");
  for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
    printf(" %d",*suspIt);
  printf("\n");
  suspSet.insert(context->gettid());
#endif
  // Save the old signal mask on stack so it can be restored
  Tsigset_t saveMask(oldMask);
  pushStruct(context,saveMask);
  setReg<uint32_t,RegSys>(context,RegSys,1);
  // Suspend and redo this system call when woken up
  context->updIAddr(-inst->aupdate,-1);
  I(inst==context->getIDesc());
  I(inst==context->virt2inst(context->getIAddr()));
  context->suspend();
}

template<>
void RealLinuxSys<ExecModeMips32>::sysRtSigReturn(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_SIGNALS)
  printf("sysCall32_rt_sigreturn pid %d to ",context->gettid());
#endif    
  // Restore old signal mask
  Tsigset_t oldMask(popStruct<Tsigset_t>(context));
  // Restore registers, then PC
  for(RegName r=RegRA;r>RegZero;r--){
    uint32_t rdVal=popScalar<uint32_t>(context);
    setReg<uint32_t,RegTypeGpr>(context,r,rdVal);
  }
  context->setIAddr(popScalar<Tpointer_t>(context));
  context->setSignalMask(oldMask);
}
#endif
#if (defined SUPPORT_MIPSEL32)
template<>
void RealLinuxSys<ExecModeMipsel32>::sysRtSigSuspend(ThreadContext *context, InstDesc *inst){
  // If this is a suspend following a wakeup, we need to pop the already-saved mask
  uint32_t dopop=getReg<uint32_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<uint32_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  Tpointer_t nset;
  Tsize_t    size;
  CallArgs(context) >> nset >> size;
  if(size!=Tsigset_t::getSize())
    fail("sysRtSigProcMask: mask size mismatch\n");
  if(!context->canRead(nset,size))
    return setSysErr(context,VEFAULT);
  // Change signal mask while suspended
  SignalSet oldMask=context->getSignalMask();
  SignalSet newMask(Tsigset_t(context,nset));
  context->setSignalMask(newMask);
  if(context->hasReadySignal()){
    SigInfo *sigInfo=context->nextReadySignal();
    context->setSignalMask(oldMask);
    handleSignal(context,sigInfo);
    delete sigInfo;
    return setSysErr(context,VEINTR);
  }
#if (defined DEBUG_SIGNALS)
  Tpid_t pid=context->gettid();
  printf("Suspend %d in sysCall32_rt_sigsuspend\n",pid);
  context->dumpCallStack();
  printf("Also suspended:");
  for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
    printf(" %d",*suspIt);
  printf("\n");
  suspSet.insert(context->gettid());
#endif
  // Save the old signal mask on stack so it can be restored
  Tsigset_t saveMask(oldMask);
  pushStruct(context,saveMask);
  setReg<uint32_t,RegSys>(context,RegSys,1);
  // Suspend and redo this system call when woken up
  context->updIAddr(-inst->aupdate,-1);
  I(inst==context->getIDesc());
  I(inst==context->virt2inst(context->getIAddr()));
  context->suspend();
}

template<>
void RealLinuxSys<ExecModeMipsel32>::sysRtSigReturn(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_SIGNALS)
  printf("sysCall32_rt_sigreturn pid %d to ",context->gettid());
#endif    
  // Restore old signal mask
  Tsigset_t oldMask(popStruct<Tsigset_t>(context));
  // Restore registers, then PC
  for(RegName r=RegRA;r>RegZero;r--){
    uint32_t rdVal=popScalar<uint32_t>(context);
    setReg<uint32_t,RegTypeGpr>(context,r,rdVal);
  }
  context->setIAddr(popScalar<Tpointer_t>(context));
  context->setSignalMask(oldMask);
}
#endif
#if (defined SUPPORT_MIPS64)
template<>
void RealLinuxSys<ExecModeMips64>::sysRtSigSuspend(ThreadContext *context, InstDesc *inst){
  // If this is a suspend following a wakeup, we need to pop the already-saved mask
  Tregv_t dopop=getReg<Tregv_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<Tregv_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  Tpointer_t nset;
  Tsize_t    size;
  CallArgs(context) >> nset >> size;
  if(size!=Tsigset_t::getSize())
    fail("sysRtSigProcMask: mask size mismatch\n");
  if(!context->canRead(nset,size))
    return setSysErr(context,VEFAULT);
  // Change signal mask while suspended
  SignalSet oldMask=context->getSignalMask();
  SignalSet newMask(Tsigset_t(context,nset));
  context->setSignalMask(newMask);
  if(context->hasReadySignal()){
    SigInfo *sigInfo=context->nextReadySignal();
    context->setSignalMask(oldMask);
    handleSignal(context,sigInfo);
    delete sigInfo;
    return setSysErr(context,VEINTR);
  }
#if (defined DEBUG_SIGNALS)
  Tpid_t pid=context->gettid();
  printf("Suspend %d in sysCall32_rt_sigsuspend\n",pid);
  context->dumpCallStack();
  printf("Also suspended:");
  for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
    printf(" %d",*suspIt);
  printf("\n");
  suspSet.insert(context->gettid());
#endif
  // Save the old signal mask on stack so it can be restored
  Tsigset_t saveMask(oldMask);
  pushStruct(context,saveMask);
  setReg<Tregv_t,RegSys>(context,RegSys,1);
  // Suspend and redo this system call when woken up
  context->updIAddr(-inst->aupdate,-1);
  I(inst==context->getIDesc());
  I(inst==context->virt2inst(context->getIAddr()));
  context->suspend();
}

template<>
void RealLinuxSys<ExecModeMips64>::sysRtSigReturn(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_SIGNALS)
  printf("sysCall32_rt_sigreturn pid %d to ",context->gettid());
#endif    
  // Restore old signal mask
  Tsigset_t oldMask(popStruct<Tsigset_t>(context));
  // Restore registers, then PC
  for(RegName r=RegRA;r>RegZero;r--){
    Tregv_t rdVal=popScalar<Tregv_t>(context);
    setReg<Tregv_t,RegTypeGpr>(context,r,rdVal);
  }
  context->setIAddr(popScalar<Tpointer_t>(context));
  context->setSignalMask(oldMask);
}
#endif
#if (defined SUPPORT_MIPSEL64)
template<>
void RealLinuxSys<ExecModeMipsel64>::sysRtSigSuspend(ThreadContext *context, InstDesc *inst){
  // If this is a suspend following a wakeup, we need to pop the already-saved mask
  Tregv_t dopop=getReg<Tregv_t,RegSys>(context,RegSys);
  if(dopop){
    setReg<Tregv_t,RegSys>(context,RegSys,0);
    context->setSignalMask(popStruct<Tsigset_t>(context));
  }
  Tpointer_t nset;
  Tsize_t    size;
  CallArgs(context) >> nset >> size;
  if(size!=Tsigset_t::getSize())
    fail("sysRtSigProcMask: mask size mismatch\n");
  if(!context->canRead(nset,size))
    return setSysErr(context,VEFAULT);
  // Change signal mask while suspended
  SignalSet oldMask=context->getSignalMask();
  SignalSet newMask(Tsigset_t(context,nset));
  context->setSignalMask(newMask);
  if(context->hasReadySignal()){
    SigInfo *sigInfo=context->nextReadySignal();
    context->setSignalMask(oldMask);
    handleSignal(context,sigInfo);
    delete sigInfo;
    return setSysErr(context,VEINTR);
  }
#if (defined DEBUG_SIGNALS)
  Tpid_t pid=context->gettid();
  printf("Suspend %d in sysCall32_rt_sigsuspend\n",pid);
  context->dumpCallStack();
  printf("Also suspended:");
  for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
    printf(" %d",*suspIt);
  printf("\n");
  suspSet.insert(context->gettid());
#endif
  // Save the old signal mask on stack so it can be restored
  Tsigset_t saveMask(oldMask);
  pushStruct(context,saveMask);
  setReg<Tregv_t,RegSys>(context,RegSys,1);
  // Suspend and redo this system call when woken up
  context->updIAddr(-inst->aupdate,-1);
  I(inst==context->getIDesc());
  I(inst==context->virt2inst(context->getIAddr()));
  context->suspend();
}

template<>
void RealLinuxSys<ExecModeMipsel64>::sysRtSigReturn(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_SIGNALS)
  printf("sysCall32_rt_sigreturn pid %d to ",context->gettid());
#endif    
  // Restore old signal mask
  Tsigset_t oldMask(popStruct<Tsigset_t>(context));
  // Restore registers, then PC
  for(RegName r=RegRA;r>RegZero;r--){
    Tregv_t rdVal=popScalar<Tregv_t>(context);
    setReg<Tregv_t,RegTypeGpr>(context,r,rdVal);
  }
  context->setIAddr(popScalar<Tpointer_t>(context));
  context->setSignalMask(oldMask);
}
#endif

template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedYield(ThreadContext *context, InstDesc *inst){
  osSim->eventYield(context->gettid(),-1);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetPriority(ThreadContext *context, InstDesc *inst){
  Tint     which;
  Tint     who;
  CallArgs(context) >> which >> who;
  printf("sysGetPriority(%d,%d) called (continuing).\n",which,who);
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedGetParam(ThreadContext *context, InstDesc *inst){
  Tpid_t     pid;
  Tpointer_t param;
  CallArgs(context) >> pid >> param;
  printf("sysSchedGetParam(%d,%ld) called (continuing with EINVAL).\n",pid,(long)param);
  ThreadContext *kcontext=pid?osSim->getContext(pid-BaseSimTid):context;
  if(!kcontext)
    return setSysErr(context,VESRCH);
  // TODO: Check if we can write to param
  // TODO: Return a meaningful value. for now, we just reject the call
  return setSysErr(context,VEINVAL);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedSetScheduler(ThreadContext *context, InstDesc *inst){
  Tpid_t     pid;
  Tint       policy;
  Tpointer_t param;
  CallArgs(context) >> pid >> policy >> param;
  printf("sysSchedSetScheduler(%d,%d,%ld) called (continuing).\n",pid,policy,(long)param);
  // TODO: Set the actual scheduling policy
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedGetScheduler(ThreadContext *context, InstDesc *inst){
  Tpid_t    pid;
  CallArgs(context) >> pid;
  printf("sysSchedGetScheduler(%d) called (continuing).\n",pid);
   // TODO: Get the actual scheduling policy, we just return zero now
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedGetPriorityMax(ThreadContext *context, InstDesc *inst){
  Tint     policy;
  CallArgs(context) >> policy;
  printf("sysSchedGetPriorityMax(%d) called (continuing with 0).\n",policy);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedGetPriorityMin(ThreadContext *context, InstDesc *inst){
  Tint     policy;
  CallArgs(context) >> policy;
  printf("sysSchedGetPriorityMin(%d) called (continuing with 0).\n",policy);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedSetAffinity(ThreadContext *context, InstDesc *inst){
  Tpid_t     pid;
  Tuint      len;
  Tpointer_t mask;
  CallArgs(context) >> pid >> len >> mask;
  if(!context->canRead(mask,len))
    return setSysRet(context,VEFAULT);
  ThreadContext *kcontext=pid?osSim->getContext(pid-BaseSimTid):context;
  if(!kcontext)
    return setSysErr(context,VESRCH);
  // TODO: We need to look at the affinity mask here
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSchedGetAffinity(ThreadContext *context, InstDesc *inst){
  Tpid_t     pid;
  Tuint      len;
  Tpointer_t mask;
  CallArgs(context) >> pid >> len >> mask;
  // TODO: Use the real number of CPUs instead of 1024
  if(len<1024/sizeof(uint8_t))
    return setSysErr(context,VEINVAL);
  if(!context->canWrite(mask,len))
    return setSysErr(context,VEFAULT);
  ThreadContext *kcontext=pid?osSim->getContext(pid-BaseSimTid):context;
  if(!kcontext)
    return setSysErr(context,VESRCH);
  // TODO: We need to look at the affinity mask here
  // for now, we just return an all-ones mask
  uint8_t buf[len];
  memset(buf,0,len);
  context->writeMemFromBuf(mask,len,buf);
  setSysRet(context);
}

// Simulated wall clock time (for time() syscall)
static time_t wallClock=(time_t)-1;
// TODO: This is a hack to get applications working.
// We need a real user/system time estimator
static uint64_t myUsrUsecs=0;
static uint64_t mySysUsecs=0;

template<ExecMode mode>
void RealLinuxSys<mode>::sysTimes(ThreadContext *context, InstDesc *inst){
  Tpointer_t buffer;
  CallArgs(context) >> buffer;
  if(!context->canWrite(buffer,Ttms::getSize()))
    return setSysErr(context,VEFAULT);
  Ttms simTms;
  // TODO: This is a hack. See above.
  myUsrUsecs+=100;
  mySysUsecs+=1;
  simTms.tms_utime=simTms.tms_cutime=myUsrUsecs/1000;
  simTms.tms_stime=simTms.tms_cstime=mySysUsecs/1000;
  Tclock_t rv=(myUsrUsecs+mySysUsecs)/1000;
  simTms.write(context,buffer);
  return setSysRet(context,rv);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysTime(ThreadContext *context, InstDesc *inst){
  Tpointer_t tloc;
  CallArgs(context) >> tloc;
  // TODO: This should actually take into account simulated time
  if(wallClock==(time_t)-1)
    wallClock=time(0);
  I(wallClock>0);
  Ttime_t rv=wallClock;
  if(tloc!=(VAddr)0){
    if(!context->canWrite(tloc,sizeof(Ttime_t)))
      return setSysErr(context,VEFAULT);
    context->writeMemRaw(tloc,fixEndian(rv));
  }
  return setSysRet(context,rv);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetTimeOfDay(ThreadContext *context, InstDesc *inst){
  Tpointer_t tv;
  Tpointer_t tz;
  CallArgs(context) >> tv >> tz;
  if(tv&&!context->canWrite(tv,Ttimeval::getSize()))
    return setSysErr(context,VEFAULT);
  if(tz&&!context->canWrite(tz,Ttimezone::getSize()))
    return setSysErr(context,VEFAULT);
  if(tv)
    Ttimeval(15,0).write(context,tv);
  if(tz)
    Ttimezone(0,0).write(context,tz);
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSetITimer(ThreadContext *context, InstDesc *inst){
  // TODO: Read the actual parameters
  // for now, we just reject the call
  printf("sysSetITimer called (continuing with EINVAL).\n");
  return setSysErr(context,VEINVAL);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysClockGetRes(ThreadContext *context, InstDesc *inst){
  // TODO: Read the actual parameters
  // for now, we just reject the call
  printf("sysClockGetRes called (continuing with EINVAL).\n");
  return setSysErr(context,VEINVAL);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysClockGetTime(ThreadContext *context, InstDesc *inst){
  // TODO: Read the actual parameters
  // for now, we just reject the call
  printf("sysClockGettime called (continuing with EINVAL).\n");
  return setSysErr(context,VEINVAL);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysAlarm(ThreadContext *context, InstDesc *inst){
  Tuint    seconds;
  CallArgs(context) >> seconds;
  // TODO: Clear existing alarms
  Tuint    oseconds=0;
  if(seconds){
    // Set new alarm
    fail("sysCall32_alarm(%d): not implemented at 0x%08x\n",seconds,context->getIAddr());
  }
  setSysRet(context,oseconds);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysNanoSleep(ThreadContext *context, InstDesc *inst){
  Tpointer_t req;
  Tpointer_t rem;
  CallArgs(context) >> req >> rem;
  if(!context->canRead(req,Ttimespec::getSize()))
    return setSysErr(context,VEFAULT);
  if(rem&&(!context->canWrite(rem,Ttimespec::getSize())))
    return setSysErr(context,VEFAULT);
  Ttimespec ts(context,req);
  // TODO: We need to actually suspend this thread for the specified time
  wallClock+=ts.tv_sec;
  if(rem){
    ts.tv_sec=0;
    ts.tv_nsec=0;
    ts.write(context,rem);
  }
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysClockNanoSleep(ThreadContext *context, InstDesc *inst){
  Tclockid_t clock_id;
  Tint       flags;
  Tpointer_t req;
  Tpointer_t rem;
  CallArgs(context) >> clock_id >> flags >> req >> rem;
  if(!context->canRead(req,Ttimespec::getSize()))
    return setSysErr(context,VEFAULT);
  if(rem&&(!context->canWrite(rem,Ttimespec::getSize())))
    return setSysErr(context,VEFAULT);
  Ttimespec ts(context,req);
  // TODO: We need to actually suspend this thread for the specified time
  wallClock+=ts.tv_sec;
  if(rem){
    ts.tv_sec=0;
    ts.tv_nsec=0;
    ts.write(context,rem);
  }
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysBrk(ThreadContext *context, InstDesc *inst){
  Tpointer_t addr;
  CallArgs(context) >> addr;
  // Page-align address
  addr=alignUp(addr,context->getAddressSpace()->getPageSize());
  Tpointer_t brkBase(context->getAddressSpace()->getBrkBase());
  Tpointer_t segStart(context->getAddressSpace()->getSegmentAddr(brkBase-1));
  Tsize_t    oldSegSize(context->getAddressSpace()->getSegmentSize(segStart));
  if(!addr)
    return setSysRet(context,segStart+oldSegSize);
  if(addr<brkBase)
    fail("sysCall32_brk: new break 0x%08x below brkBase 0x%08x\n",addr,brkBase);
  Tsize_t   newSegSize=addr-segStart;
  if((newSegSize<=oldSegSize)||context->getAddressSpace()->isNoSegment(segStart+oldSegSize,newSegSize-oldSegSize)){
    context->getAddressSpace()->resizeSegment(segStart,newSegSize);
    return setSysRet(context,addr);
  }
  return setSysErr(context,VENOMEM);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSetThreadArea(ThreadContext *context, InstDesc *inst){
  Tpointer_t addr;
  CallArgs(context) >> addr;
  setThreadArea(context,addr);
  setSysRet(context);
}
template<ExecMode mode>
template<size_t offsmul>
void RealLinuxSys<mode>::sysMMap(ThreadContext *context, InstDesc *inst){
  Tpointer_t start;
  Tsize_t    length;
  Tint       prot;
  Tint       flags;
  CallArgs args(context);
  args >> start >> length >> prot >> flags;
  Tint       fd;
  Toff_t     offset;
  if(!(flags&VMAP_ANONYMOUS)){
    args >> fd >> offset;
  }else{
    fd=-1;
    offset=0;
  }
  Tpointer_t addr=start;
  if(context->getAddressSpace()->pageAlignDown(addr)!=addr){
    // Address is not page-aligned, error if MAP_FIXED, ignore addr otherwise
    if(flags&VMAP_FIXED)
      return setSysErr(context,VEINVAL);
    addr=0;
  }
  // Length parameter need not be page-aligned, so we silently align it now
  Tsize_t len=context->getAddressSpace()->pageAlignUp(length);
  // Either MAP_SHARED or MAP_PRIVATE must be set, but not both
  if(((flags&VMAP_SHARED)!=0)==((flags&VMAP_PRIVATE)!=0))
    return setSysErr(context,VEINVAL);
  // Find the return value (actual address of the mapped block)
  if(flags&VMAP_FIXED){
    if(!addr)
      return setSysErr(context,VEINVAL);
    context->getAddressSpace()->deleteSegment(addr,len);
  }else{
    if((!addr)||(!context->getAddressSpace()->isNoSegment(addr,len))){
      addr=context->getAddressSpace()->newSegmentAddr(len);
      if(!addr)
        return setSysErr(context,VENOMEM);
    }
  }
  // Now figure out the file maping (if any)
  FileSys::SeekableDescription *sdesc=0;
  size_t             offs=0;
  if(!(flags&VMAP_ANONYMOUS)){
    FileSys::OpenFiles *openFiles=context->getOpenFiles();
    if(!openFiles->isOpen(fd))
      return setSysErr(context,VEBADF);
    sdesc=dynamic_cast<FileSys::SeekableDescription *>(openFiles->getDescription(fd));
    if(!sdesc)
      return setSysErr(context,VENODEV);
    if((!sdesc->canRd())||((flags&VMAP_SHARED)&&!sdesc->canWr()))
      return setSysErr(context,VEACCES);
    offs=offset*offsmul;
    if(context->getAddressSpace()->pageAlignDown(offs)!=offs)
      return setSysErr(context,VEINVAL);      
  }
  context->getAddressSpace()->newSegment(addr,len,
                                         prot&VPROT_READ,prot&VPROT_WRITE,prot&VPROT_EXEC,
                                         flags&VMAP_SHARED,sdesc,offs);
  if(sdesc){
    ExecMode exmode=getExecMode(sdesc);
    if(exmode)
      mapFuncNames(context,sdesc,exmode,addr,len,offs);
  }
#if (defined DEBUG_MEMORY)
  printf("sysCall32_mmap addr 0x%08x len 0x%08lx R%d W%d X%d S%d\n",
	 rv,(unsigned long)length,
	 prot&VPROT_READ,
	 prot&VPROT_WRITE,
	 prot&VPROT_EXEC,
	 flags&VMAP_SHARED);
#endif
#if (defined DEBUG_FILES) || (defined DEBUG_VMEM)
  printf("[%d] mmap %d start 0x%08x len 0x%08x offset 0x%08x prot %c%c%c return 0x%08x\n",
	 context->gettid(),fd,(uint32_t)start,(uint32_t)length,(uint32_t)offset,
         (prot&VPROT_READ)?'R':' ',(prot&VPROT_WRITE)?'W':' ',(prot&VPROT_EXEC)?'E':' ',
	 (uint32_t)addr);
#endif
  return setSysRet(context,addr);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysMReMap(ThreadContext *context, InstDesc *inst){
  Tpointer_t  oldaddr;
  Tsize_t     oldsize;
  Tsize_t     newsize;
  Tint        flags;
  CallArgs(context) >> oldaddr >> oldsize >> newsize >> flags;
  // Page-align oldsize and newsize
  oldsize=alignUp(oldsize,context->getAddressSpace()->getPageSize());
  newsize=alignUp(newsize,context->getAddressSpace()->getPageSize());
  AddressSpace *addrSpace=context->getAddressSpace();
  if(!addrSpace->isSegment(oldaddr,oldsize))
    fail("sysCall32_mremap: old range not a segment\n");
  if(newsize<=oldsize){
    if(newsize<oldsize)
      addrSpace->resizeSegment(oldaddr,newsize);
    return setSysRet(context,oldaddr);
  }
  Tpointer_t rv=0;
  if(addrSpace->isNoSegment(oldaddr+oldsize,newsize-oldsize)){
    rv=oldaddr;
  }else if(flags&VMREMAP_MAYMOVE){
    rv=addrSpace->newSegmentAddr(newsize);
    if(rv)
      addrSpace->moveSegment(oldaddr,rv);
  }
  if(!rv)
    return setSysErr(context,VENOMEM);
  addrSpace->resizeSegment(rv,newsize);
  context->writeMemWithByte(rv+oldsize,newsize-oldsize,0);      
  return setSysRet(context,rv);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysMUnMap(ThreadContext *context, InstDesc *inst){
  Tpointer_t addr;
  Tsize_t    len;
  CallArgs(context) >> addr >> len;
  // Page-align length 
  len=alignUp(len,context->getAddressSpace()->getPageSize());
  context->getAddressSpace()->deleteSegment(addr,len);
#if (defined DEBUG_MEMORY)
  printf("sysCall32_munmap addr 0x%08x len 0x%08lx\n",             
         addr,(unsigned long)len);

#endif
#if (defined DEBUG_VMEM)
  printf("[%d] munmap addr 0x%08x len 0x%08x\n",
         context->gettid(),(uint32_t)addr,(uint32_t)len);
#endif
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysMProtect(ThreadContext *context, InstDesc *inst){
  Tpointer_t addr;
  Tsize_t    len;
  Tint       prot;
  CallArgs(context) >> addr >> len >> prot;
  // Page-align length 
  len=alignUp(len,context->getAddressSpace()->getPageSize());
#if (defined DEBUG_MEMORY)
  printf("sysCall32_mprotect addr 0x%08x len 0x%08lx R%d W%d X%d\n",
         (unsigned long)addr,(unsigned long)len,
         prot&VPROT_READ,prot&VPROT_WRITE,prot&VPROT_EXEC);
#endif
#if (defined DEBUG_VMEM)
  printf("[%d] mprotect addr 0x%08x len 0x%08x prot %c%c%c\n",
         context->gettid(),(uint32_t)addr,(uint32_t)len,
         (prot&VPROT_READ)?'R':' ',(prot&VPROT_WRITE)?'W':' ',
         (prot&VPROT_EXEC)?'E':' '
	 );
#endif
  if(!context->getAddressSpace()->isMapped(addr,len))
    return setSysErr(context,VENOMEM);
  context->getAddressSpace()->protectSegment(addr,len,
					     prot&VPROT_READ,prot&VPROT_WRITE,prot&VPROT_EXEC);
  setSysRet(context);
}

template<ExecMode mode>
void RealLinuxSys<mode>::sysOpen(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathp;
  Tint       flags;
  Tmode_t    fmode;
  CallArgs(context) >> pathp >> flags >> fmode;
  Tstr path(context,pathp);
  if(!path)
    return;
  std::string natPath(context->getFileSys()->toHost((const char *)path));
  if(flags&VO_NOFOLLOW){
    if(FileSys::Node::resolve(natPath)!=natPath)
      return setSysErr(context,VELOOP);
  }else{
    natPath=FileSys::Node::resolve(natPath);
  }
  FileSys::Node *node=FileSys::Node::lookup(natPath);
  if(node){
    if(flags&VO_EXCL)
      return setSysErr(context,VEEXIST);
    if(dynamic_cast<FileSys::DirectoryNode *>(node)){
      if((flags&VO_ACCMODE)!=VO_RDONLY)
        return setSysErr(context,VEACCES);        
    }else{
      if(flags&VO_DIRECTORY)
        return setSysErr(context,VENOTDIR);
    }
  }else{
    if(!(flags&VO_CREAT))
      return setSysErr(context,VENOENT);
    FileSys::fd_t fd=::open(natPath.c_str(),O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW,mode_tToNative(fmode));           
    if(fd==-1)
      fail("sysOpen can't create %s\n",natPath.c_str());
    struct stat stbuf;           
    if(fstat(fd,&stbuf)!=0)
      fail("sysOpen could not fstat %s\n",natPath.c_str());
    node=new FileSys::FileNode(stbuf);
    FileSys::Node::insert(natPath,node);
    close(fd);
  }
  FileSys::Description *description=FileSys::Description::create(node,openFlagsToNative(flags));
  if(!description)
    return setSysErr(context);
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  Tint newfd=openFiles->nextFreeFd(0);
#ifdef DEBUG_FILES
  printf("(%d) open %s as %d\n",context->gettid(),(const char *)path,newfd);
#endif
  openFiles->openDescriptor(newfd,description);
  setSysRet(context,newfd);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysPipe(ThreadContext *context, InstDesc *inst){
  FileSys::PipeNode *node=new FileSys::PipeNode();
  I(node);
  FileSys::PipeDescription *rdescription= new FileSys::PipeDescription(node,O_RDONLY);
  I(rdescription);
  FileSys::PipeDescription *wdescription= new FileSys::PipeDescription(node,O_WRONLY);
  I(wdescription);
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  Tint rfd(openFiles->nextFreeFd(0));
  openFiles->openDescriptor(rfd,rdescription);
  Tint wfd(openFiles->nextFreeFd(0));
  openFiles->openDescriptor(wfd,wdescription);
#ifdef DEBUG_FILES
  printf("[%d] pipe rd %d wr %d\n",context->gettid(),rfd,wfd);
#endif
  setSysRet(context,rfd,wfd);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysDup(ThreadContext *context, InstDesc *inst){
  Tint     oldfd;
  CallArgs(context) >> oldfd;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(oldfd))
    return setSysErr(context,VEBADF);
  Tint newfd=openFiles->nextFreeFd(0);
  openFiles->openDescriptor(newfd,openFiles->getDescription(oldfd));
#ifdef DEBUG_FILES
  printf("[%d] dup %d as %d\n",context->gettid(),oldfd,newfd);
#endif
  setSysRet(context,newfd);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysDup2(ThreadContext *context, InstDesc *inst){
  Tint     oldfd;
  Tint     newfd;
  CallArgs(context) >> oldfd >> newfd;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if((!openFiles->isOpen(oldfd))||(newfd<0))
    return setSysErr(context,VEBADF);
  if(newfd!=oldfd){
    if(openFiles->isOpen(newfd))
      openFiles->closeDescriptor(newfd);
    openFiles->openDescriptor(newfd,openFiles->getDescription(oldfd));
  }
#ifdef DEBUG_FILES
  printf("[%d] dup2 %d as %d\n",context->gettid(),oldfd,newfd);
#endif
  setSysRet(context,newfd);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysFCntl(ThreadContext *context, InstDesc *inst){
  // We implement both fcntl and fcntl64 here. the only difference is that
  // fcntl64 also handles F_GETLK64, F_SETLK64, and F_SETLKW64 and fcntl does not
  Tint     fd;
  Tint     cmd;
  CallArgs args(context);
  args >> fd >> cmd;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  switch(cmd){
  case VF_DUPFD: {
#ifdef DEBUG_FILES
    printf("[%d] dupfd %d called\n",context->gettid(),fd);
#endif
    Tint minfd;
    args >> minfd;
    if(minfd<0)
      return setSysErr(context,VEINVAL);
    Tint newfd=openFiles->nextFreeFd(minfd);
    openFiles->openDescriptor(newfd,openFiles->getDescription(fd));
#ifdef DEBUG_FILES
    printf("[%d] dupfd %d as %d\n",context->gettid(),fd,newfd);
#endif
    return setSysRet(context,newfd);
  } break;
  case VF_GETFD: {
#ifdef DEBUG_FILES
    printf("[%d] getfd %d called\n",context->gettid(),fd);
#endif
    bool cloex=openFiles->getCloexec(fd);
    return setSysRet(context,cloex?VFD_CLOEXEC:0);
  } break;
  case VF_SETFD: {
    Tint cloex;
    args >> cloex;
#ifdef DEBUG_FILES
    printf("[%d] setfd %d to %d called\n",context->gettid(),fd,(int)cloex);
#endif
    openFiles->setCloexec(fd,(cloex&VFD_CLOEXEC)==VFD_CLOEXEC);
#ifdef DEBUG_FILES
    printf("[%d] setfd %d to %d \n",context->gettid(),fd,cloex);
#endif
    return setSysRet(context);
  } break;
  case VF_GETFL: {
#ifdef DEBUG_FILES
    printf("[%d] getfl %d called\n",context->gettid(),fd);
#endif
    FileSys::flags_t flags=openFiles->getDescription(fd)->getFlags();
    return setSysRet(context,openFlagsFromNative(flags));
  } break;
  default:
    fail("sysCall32_fcntl64 unknown command %d on file %d\n",cmd,fd);
  }
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysRead(ThreadContext *context, InstDesc *inst){
  Tint       fd;
  Tpointer_t buf;
  Tsize_t    count;
  CallArgs(context) >> fd >> buf >> count;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  FileSys::Description *description=openFiles->getDescription(fd);
  if(!description->canRd())
    return setSysErr(context,VEBADF);
  if(!context->canWrite(buf,count))
    return setSysErr(context,VEFAULT);
  FileSys::StreamDescription *sdescription=dynamic_cast<FileSys::StreamDescription *>(description);
  if(sdescription&&sdescription->willRdBlock()){
    if(sdescription->isNonBlock())
      return setSysErr(context,VEAGAIN);
    I(!context->getSignalMask().test(SigIO));
    if(context->hasReadySignal()){
      setSysErr(context,VEINTR);
      SigInfo *sigInfo=context->nextReadySignal();
      handleSignal(context,sigInfo);
      delete sigInfo;
      return;
    }
    context->updIAddr(-inst->aupdate,-1);
    I(inst==context->getIDesc());
    I(inst==context->virt2inst(context->getIAddr()));
    sdescription->rdBlock(context->gettid());
#if (defined DEBUG_SIGNALS)
    printf("Suspend %d in sysCall32_read(fd=%d)\n",context->gettid(),fd);
    context->dumpCallStack();
    printf("Also suspended:");
    for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
      printf(" %d",*suspIt);
    printf("\n");
    suspSet.insert(context->gettid());
#endif
    context->suspend();
    return;    
  }
  uint8_t rbuf[count];
  ssize_t rcount=description->read(rbuf,count);
  I(rcount>=0);
#ifdef DEBUG_FILES
  printf("[%d] read %d wants %ld gets %ld bytes\n",context->gettid(),fd,(long)count,(long)rcount);
#endif
  if(rcount==-1)
    return setSysErr(context);
  context->writeMemFromBuf(buf,(size_t)rcount,rbuf);
  return setSysRet(context,Tssize_t(rcount));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysWrite(ThreadContext *context, InstDesc *inst){
  Tint       fd;
  Tpointer_t buf;
  Tsize_t    count;
  CallArgs(context) >> fd >> buf >> count;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  FileSys::Description *description=openFiles->getDescription(fd);
  if(!description->canWr())
    return setSysErr(context,VEBADF);
  if(!context->canRead(buf,count))
    return setSysErr(context,VEFAULT);
  FileSys::StreamDescription *sdescription=dynamic_cast<FileSys::StreamDescription *>(description);
  if(sdescription&&sdescription->willWrBlock()){
    if(sdescription->isNonBlock())
      return setSysErr(context,VEAGAIN);
    I(!context->getSignalMask().test(SigIO));
    if(context->hasReadySignal()){
      setSysErr(context,VEINTR);
      SigInfo *sigInfo=context->nextReadySignal();
      handleSignal(context,sigInfo);
      delete sigInfo;
      return;
    }
    context->updIAddr(-inst->aupdate,-1);
    I(inst==context->getIDesc());
    I(inst==context->virt2inst(context->getIAddr()));
#if (defined DEBUG_SIGNALS)
    printf("Suspend %d in sysCall32_write(fd=%d)\n",context->gettid(),fd);
    context->dumpCallStack();
    printf("Also suspended:");
    for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
      printf(" %d",*suspIt);
    printf("\n");
    suspSet.insert(context->gettid());
#endif
    sdescription->wrBlock(context->gettid());
    context->suspend();
    return;
  }
  uint8_t rbuf[count];
  context->readMemToBuf(buf,(size_t)count,rbuf);
  ssize_t wcount=description->write(rbuf,count);
  I(wcount>=0);
#ifdef DEBUG_FILES
  printf("[%d] write %d wants %ld gets %ld bytes\n",context->gettid(),fd,(long)count,(long)wcount);
#endif
  if(wcount==-1)
    return setSysErr(context);
  return setSysRet(context,Tssize_t(wcount));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysWriteV(ThreadContext *context, InstDesc *inst){
  Tint       fd;
  Tpointer_t vector;
  Tint       iovcnt;
  CallArgs(context) >> fd >> vector >> iovcnt;
  if(iovcnt<=0)
    return setSysErr(context,VEINVAL);
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  if(!context->canRead(vector,iovcnt*Tiovec::getSize()))
    return setSysErr(context,VEFAULT);
  Tssize_t count=0;
  for(Tint i=0;i<iovcnt;i++){
    Tiovec iov(context,vector+i*Tiovec::getSize());
    if(!context->canRead(iov.iov_base,iov.iov_len))
      return setSysErr(context,VEFAULT);
    count+=iov.iov_len;
  }
  if(!count)
    return setSysRet(context,0);
  FileSys::Description *description=openFiles->getDescription(fd);
  if(!description->canWr())
    return setSysErr(context,VEBADF);
  uint8_t wbuf[count];
  size_t  pos=0;
  for(Tint i=0;i<iovcnt;i++){
    Tiovec iov(context,vector+i*Tiovec::getSize());
    I(context->canRead(iov.iov_base,iov.iov_len));
    context->readMemToBuf(iov.iov_base,iov.iov_len,wbuf+pos);
    pos+=iov.iov_len;
  }
  FileSys::StreamDescription *sdescription=dynamic_cast<FileSys::StreamDescription *>(description);
  if(sdescription&&sdescription->willWrBlock()){
    fail("writev would block!\n");
  }
  ssize_t wcount=description->write(wbuf,count);
#ifdef DEBUG_FILES
  int e=errno;
  printf("[%d] writev %d wants %ld gets %ld bytes\n",context->gettid(),fd,(long)count,(long)wcount);
  errno=e;
#endif
  if(wcount==-1)
    return setSysErr(context);
  return setSysRet(context,Tssize_t(wcount));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysLSeek(ThreadContext *context, InstDesc *inst){
  Tint     fd;
  Toff_t   offset;
  Tint     whence;
  CallArgs(context) >> fd >> offset >> whence;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  FileSys::SeekableDescription *description=dynamic_cast<FileSys::SeekableDescription *>(openFiles->getDescription(fd));
  if(!description)
    return setSysErr(context,VESPIPE);
  off_t newOffset=offset;
  switch(whence){
  case VSEEK_SET: break;
  case VSEEK_CUR: newOffset+=description->getPos(); break;
  case VSEEK_END: newOffset+=description->getSize(); break;
  default:
    return setSysErr(context,VEINVAL);
  }
  if(newOffset<0)
    return setSysErr(context,VEINVAL);
  description->setPos(newOffset);
#ifdef DEBUG_FILES
  printf("[%d] lseek %d to %ld whence %d returns %ld\n",context->gettid(),fd,(long)offset,(int)whence,(long)newOffset);
#endif
  return setSysRet(context,Toff_t(newOffset));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysLLSeek(ThreadContext *context, InstDesc *inst){
  Tint       fd;
  Tulong     offset_high;
  Tulong     offset_low;
  Tpointer_t result;
  Tint       whence;
  CallArgs(context) >> fd >> offset_high >> offset_low >> result >> whence;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  if(!context->canWrite(result,sizeof(Tloff_t)))
    return setSysErr(context,VEFAULT);
  FileSys::SeekableDescription *description=dynamic_cast<FileSys::SeekableDescription *>(openFiles->getDescription(fd));
  if(!description)
    return setSysErr(context,VESPIPE);
  off_t newOffset=(off_t)((((uint64_t)offset_high)<<32)|((uint64_t)offset_low));
  switch(whence){
  case VSEEK_SET: break;
  case VSEEK_CUR: newOffset+=description->getPos(); break;
  case VSEEK_END: newOffset+=description->getSize(); break;
  default:
    return setSysErr(context,VEINVAL);
  }
  if(newOffset<0)
    return setSysErr(context,VEINVAL);
  description->setPos(newOffset);
#ifdef DEBUG_FILES
  printf("[%d] llseek %d to %ld whence %d returns %ld\n",context->gettid(),fd,(long)((off_t)((((uint64_t)offset_high)<<32)|((uint64_t)offset_low))),(int)whence,(long)newOffset);
#endif
  context->writeMemRaw(result,fixEndian(Tloff_t(newOffset)));
  setSysRet(context);
}
template<ExecMode mode>
template<class Tdirent_t>
void RealLinuxSys<mode>::sysGetDEnts(ThreadContext *context, InstDesc *inst){
  Tint       fd;
  Tpointer_t dirp;
  Tsize_t    count;
  CallArgs(context) >> fd >> dirp >> count;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  if(!context->canWrite(dirp,count))
    return setSysErr(context,VEFAULT);
#ifdef DEBUG_FILES 
  printf("[%d] getdents from %d (%ld entries to 0x%08lx)\n",context->gettid(),fd,(long)count,(long unsigned)dirp);
#endif
  FileSys::DirectoryDescription *desc=dynamic_cast<FileSys::DirectoryDescription *>(openFiles->getDescription(fd));
  if(!desc){
#ifdef DEBUG_FILES 
    printf("Returning ENOTDIR\n");
#endif
    return setSysErr(context,VENOTDIR);
  }
  // If at the end of the directory, just return zero
  if(desc->getPos()==desc->getSize())
    return setSysRet(context,0);
  // How many bytes at dirp have been read so far
  Tsize_t rcount=0;
  while(true){
    off_t cpos=desc->getPos();
    if(cpos>=desc->getSize())
      break;
    std::string fname=desc->readDir();
    ino_t ino;
    std::string pname=desc->getName() + "/" + fname;
    FileSys::Node *fnode=FileSys::Node::lookup(pname);
    if(!fnode)
      continue;
    Tdirent_t simDent(fname,cpos+1,fnode->getIno());
    if(rcount+simDent.d_reclen>count){
      desc->setPos(cpos);
      break;
    }
    //    printf("Dirent off %d ino %d nam %s\n",
    //	   (int)(simDent.d_off),(int)(simDent.d_ino),simDent.d_name);
    simDent.write(context,dirp+rcount);
    rcount+=simDent.d_reclen;
  }
  if(rcount==0){
#ifdef DEBUG_FILES 
    printf("Returning EINVAL\n");
#endif
    return setSysErr(context,VEINVAL);
  }
  return setSysRet(context,rcount);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysIOCtl(ThreadContext *context, InstDesc *inst){
  Tint     fd;
  Tint     cmd;
  CallArgs args(context);
  args >> fd >> cmd;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  FileSys::Description *description=openFiles->getDescription(fd);
  switch(cmd){
  case VTCGETS:
  case VTIOCGWINSZ:
  case VTCSETSW: {
    FileSys::TtyDescription *ttydescription=dynamic_cast<FileSys::TtyDescription *>(description);
    if(!ttydescription)
      return setSysErr(context,VENOTTY);
    // TODO: For now, it never succeeds (assume no file is a terminal)
    return setSysErr(context,VENOTTY);
  } break;
  default:
    fail("sysCall32_ioctl called with fd %d and req 0x%x at 0x%08x\n",
         fd,cmd,context->getIAddr());
  }
  return setSysRet(context);
//   case VTIOCGWINSZ: {
//     Tpointer_t wsiz;
//     args >> wsiz;
//     if(!context->canWrite(wsiz,Twinsize::getSize()))
//       return setSysErr(context,VEFAULT);
//     struct winsize natWinsize;
//     int retVal=ioctl(realfd,TIOCGWINSZ,&natWinsize);
//     if(retVal==-1)
//       return setSysErr(context);
//     Twinsize(natWinsize).write(context,wsiz);
//   } break;
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysPoll(ThreadContext *context, InstDesc *inst){
  fail("sysPoll called\n");
  Tpointer_t fds;
  Tuint      nfds;
  Tlong      timeout;
  CallArgs(context) >> fds >> nfds >> timeout;
  if((!context->canRead(fds,nfds*Tpollfd::getSize()))||
     (!context->canWrite(fds,nfds*Tpollfd::getSize())))
    return setSysErr(context,VEFAULT);
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  std::vector<FileSys::StreamDescription *> rdList;
  std::vector<FileSys::StreamDescription *> wrList;
  Tlong rv=0;
  for(size_t i=0;i<nfds;i++){
    Tpollfd myFd(context,fds+i*Tpollfd::getSize());
    myFd.revents=0;
    if(myFd.fd<0){
      // Do nothing, revents is 0
    }else if(!openFiles->isOpen(myFd.fd)){
      myFd.revents|=VPOLLNVAL;
    }else{
      FileSys::StreamDescription *sdesc=dynamic_cast<FileSys::StreamDescription *>(openFiles->getDescription(myFd.fd));
      if(myFd.events&VPOLLIN){
	if((!sdesc)||(!sdesc->willRdBlock()))
	  myFd.revents|=VPOLLIN;
	else
	  rdList.push_back(sdesc);
      }
      if(myFd.events&VPOLLOUT){
	if((!sdesc)||(!sdesc->willWrBlock()))
	  myFd.revents|=VPOLLOUT;
	else
	  wrList.push_back(sdesc);
      }
    }
    if(myFd.revents)
      rv++;
    myFd.write(context,fds+i*Tpollfd::getSize());
  }
  // If any of the files could be read withut blocking, we're done
  if(rv)
    return setSysRet(context,rv);
  // We need to block and wait
  // Enable SigIO
  SignalSet newMask=context->getSignalMask();
  if(newMask.test(SigIO))
    fail("sysCall32_read: SigIO masked out, not supported\n");
  //     newMask.reset(SigIO);
  //     sstate->pushMask(newMask);
  if(context->hasReadySignal()){
    setSysErr(context,VEINTR);
    SigInfo *sigInfo=context->nextReadySignal();
    //       sstate->popMask();
    handleSignal(context,sigInfo);
    delete sigInfo;
    return;
  }
  for(size_t ri=0;ri<rdList.size();ri++)
    rdList[ri]->rdBlock(context->gettid());
  for(size_t wi=0;wi<wrList.size();wi++)
    wrList[wi]->wrBlock(context->gettid());
#if (defined DEBUG_SIGNALS)
  printf("Suspending %d in sysPoll. Also suspended:",context->gettid());
  for(PidSet::iterator suspIt=suspSet.begin();suspIt!=suspSet.end();suspIt++)
    printf(" %d",*suspIt);
  printf("\n");
  suspSet.insert(context->gettid());
#endif
  context->suspend();
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysClose(ThreadContext *context, InstDesc *inst){
  Tint     fd;
  CallArgs(context) >> fd;
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  // Do the actual call
  openFiles->closeDescriptor(fd);
#ifdef DEBUG_FILES
  printf("[%d] close %d\n",context->gettid(),fd);
#endif
  setSysRet(context);
}
template<ExecMode mode>
template<class Toffs>
void RealLinuxSys<mode>::sysTruncate(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathp;
  Toffs      length;
  CallArgs(context) >> pathp >> length;
  Tstr path(context,pathp);
  if(!path)
    return;
  if(length<0)
    return setSysErr(context,VEINVAL);
#ifdef DEBUG_FILES
  printf("[%d] truncate %s to %ld\n",context->gettid(),(const char *)path,(long)length);
#endif
  const std::string linkName(context->getFileSys()->toHost((const char *)path));
  const std::string realName(FileSys::Node::resolve(linkName));
  if(realName.empty())
    return setSysErr(context,VELOOP);
  FileSys::Node *node=FileSys::Node::lookup(realName);
  if(!node)
    return setSysErr(context,VENOENT);
  FileSys::FileNode *fnode=dynamic_cast<FileSys::FileNode *>(node);
  if(!fnode){
    if(dynamic_cast<FileSys::DirectoryNode *>(node))
      return setSysErr(context,VEISDIR);
    return setSysErr(context,VEINVAL);
  }
  fnode->setSize(length);
  return setSysRet(context);
}
template<ExecMode mode>
template<class Toffs>
void RealLinuxSys<mode>::sysFTruncate(ThreadContext *context, InstDesc *inst){
  Tint  fd;
  Toffs length;
  CallArgs(context) >> fd >> length;
  if(length<0) 
    return setSysErr(context,VEINVAL);
#ifdef DEBUG_FILES
  printf("[%d] ftruncate %d to %ld\n",context->gettid(),fd,(long)length);
#endif
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  FileSys::Description *desc=openFiles->getDescription(fd);
  if(!desc->canWr())
    return setSysErr(context,VEBADF);
  FileSys::FileNode *node=dynamic_cast<FileSys::FileNode *>(desc->getNode());
  if(!node)
    return setSysErr(context,VEINVAL);
  node->setSize(length);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysChMod(ThreadContext *context, InstDesc *inst){    
  Tpointer_t pathp;
  Tmode_t    fmode;
  CallArgs(context) >> pathp >> fmode;
  Tstr path(context,pathp);
  if(!path)                 
    return;
#ifdef DEBUG_FILES
  printf("[%d] chmod %s to %d\n",context->gettid(),(const char *)path,fmode);
#endif
  if(chmod(context->getFileSys()->toHost((const char *)path).c_str(),mode_tToNative(fmode))==-1)
    return setSysErr(context);               
  setSysRet(context);
}
template<ExecMode mode>
template<bool link, class Tstat_t>
void RealLinuxSys<mode>::sysStat(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathp;
  Tpointer_t buf;
  CallArgs(context) >> pathp >> buf;
  Tstr path(context,pathp);
  if(!path)
    return;
#ifdef DEBUG_FILES
  printf("[%d] %cstat %s\n",context->gettid(),link?'l':' ',(const char *)path);
#endif
  if(!context->canWrite(buf,Tstat_t::getSize()))
    return setSysErr(context,VEFAULT);
  struct stat natStat;
  const std::string natPath=context->getFileSys()->toHost((const char *)path);
  FileSys::Node *node=FileSys::Node::lookup(natPath);
  if(link?lstat(natPath.c_str(),&natStat):stat(natPath.c_str(),&natStat))
    return setSysErr(context);
  Tstat_t targStat(natStat);
  targStat.st_ino=node->getIno();
  targStat.write(context,buf);
#ifdef DEBUG_FILES
  printf("Dev %d Ino %d\n\n",(int)(natStat.st_dev),(int)(targStat.st_ino));
#endif
  setSysRet(context);
}
template<ExecMode mode>      
template<class Tstat_t>
void RealLinuxSys<mode>::sysFStat(ThreadContext *context, InstDesc *inst){
  Tint       fd;
  Tpointer_t buf;
  CallArgs(context) >> fd >> buf;
  if(!context->canWrite(buf,Tstat_t::getSize()))
    return setSysErr(context,VEFAULT);
  FileSys::OpenFiles *openFiles=context->getOpenFiles();
  if(!openFiles->isOpen(fd))
    return setSysErr(context,VEBADF);
  const FileSys::Node *node=openFiles->getDescription(fd)->getNode();
  Tstat_t simStat;
  simStat.st_dev=node->getDev();
  simStat.st_ino=node->getIno();
  simStat.st_uid=node->getUid();
  simStat.st_gid=node->getGid();
  simStat.st_mode=node->getMode();
  simStat.st_nlink=1;
  simStat.st_blksize=context->getAddressSpace()->getPageSize();
  const FileSys::SeekableNode *seeknode=dynamic_cast<const FileSys::SeekableNode *>(node);
  simStat.st_size=seeknode?seeknode->getSize():0;
  simStat.st_blocks=context->getAddressSpace()->pageAlignUp(simStat.st_size);
  const FileSys::StreamNode *streamnode=dynamic_cast<const FileSys::StreamNode *>(node);
  simStat.st_rdev=streamnode?streamnode->getRdev():0;
  simStat.write(context,buf);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysFStatFS(ThreadContext *context, InstDesc *inst){
  Tint       fd;
  Tpointer_t buf;
  CallArgs(context) >> fd >> buf;
  printf("sysFStatFS(%d,%ld) called (continuing with ENOSYS).\n",fd,(long)buf);
  setSysErr(context,VENOSYS);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysUnlink(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathp;
  CallArgs(context) >> pathp;
  Tstr pathname(context,pathp);
  if(!pathname)
    return;
#ifdef DEBUG_FILES
  printf("[%d] unlink %s\n",context->gettid(),(const char *)pathname);
#endif
  std::string natName(context->getFileSys()->toHost((const char *)pathname));
  FileSys::Node *node=FileSys::Node::lookup(natName);
  if(!node)
    return setSysErr(context);
  if(unlink(natName.c_str())!=0)
    fail("sysUnlink could not unlink %s\n",natName.c_str());
  FileSys::Node::remove(natName,node);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSymLink(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathp1;
  Tpointer_t pathp2;
  CallArgs(context) >> pathp1 >> pathp2;
  Tstr     path1(context,pathp1);
  if(!path1)
    return;
  Tstr     path2(context,pathp2);
  if(!path2)
    return;
#ifdef DEBUG_FILES
  printf("[%d] symlink %s to %s\n",context->gettid(),(const char *)path2,(const char *)path1);
#endif
  if(symlink(path1,context->getFileSys()->toHost((const char *)path2).c_str())==-1)
    return setSysErr(context);
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysRename(ThreadContext *context, InstDesc *inst){
  Tpointer_t oldpathp;
  Tpointer_t newpathp;
  CallArgs(context) >> oldpathp >> newpathp;
  Tstr     oldpath(context,oldpathp);
  if(!oldpath)
    return;
  Tstr     newpath(context,newpathp);
  if(!oldpath)
    return;
  std::string natOldName(context->getFileSys()->toHost((const char *)oldpath));
  std::string natNewName(context->getFileSys()->toHost((const char *)newpath));
  FileSys::Node *node=FileSys::Node::lookup(natOldName);
  if(!node)
    return setSysErr(context);
#ifdef DEBUG_FILES
  printf("[%d] rename %s to %s\n",context->gettid(),(const char *)oldpath,(const char *)newpath);
#endif
  if(rename(natOldName.c_str(),natNewName.c_str())!=0)
    fail("sysRename could not rename %s to %s\n",natOldName.c_str(),natNewName.c_str());
  FileSys::Node::insert(natNewName,node);
  FileSys::Node::remove(natOldName,node);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysChdir(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathp;
  CallArgs(context) >> pathp;
  Tstr path(context,pathp);
  if(!path)
    return;
#ifdef DEBUG_FILES
  printf("[%d] chdir %s\n",context->gettid(),(const char *)path);
#endif
  // Remember current working directory of the simulator
  char cwd[PATH_MAX];
  getcwd(cwd,PATH_MAX);
  // Change to the simulated working directory to check for errors
  int res=chdir(context->getFileSys()->toHost((const char *)path).c_str());
  if(res){
    setSysErr(context);
  }else{
    context->getFileSys()->setCwd((const char *)path);
    setSysRet(context);
  }
  // Restore current working directory of the simulator
  chdir(cwd);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysAccess(ThreadContext *context, InstDesc *inst){
  Tpointer_t fnamep;
  Tint       amode;
  CallArgs(context) >> fnamep >> amode;
  Tstr fname(context,fnamep);
  if(!fname)
    return;
  // TODO: Translate file name using mount info
#ifdef DEBUG_FILES
  printf("[%d] access %s\n",context->gettid(),(const char *)fname);
#endif
  if(access(context->getFileSys()->toHost((const char *)fname).c_str(),accessModeToNative(amode))==-1)
    return setSysErr(context);
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetCWD(ThreadContext *context, InstDesc *inst){
  Tpointer_t buf;
  Tsize_t    size;
  CallArgs(context) >> buf >> size;
  if(!size)
    return setSysErr(context,VEINVAL);
  const std::string &cwd=context->getFileSys()->getCwd();
  Tsize_t cwdLen=cwd.length()+1;
  if(size<cwdLen)
    return setSysErr(context,VERANGE);
  if(!context->canWrite(buf,cwdLen))
    return setSysErr(context,VEFAULT);
  context->writeMemFromBuf(buf,cwdLen,cwd.c_str());
  return setSysRet(context,Tint(cwdLen));
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysMkdir(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathnamep;
  Tmode_t    fmode;
  CallArgs(context) >> pathnamep >> fmode;
  Tstr     pathname(context,pathnamep);
  if(!pathname)
    return;
  if(mkdir(context->getFileSys()->toHost((const char *)pathname).c_str(),mode_tToNative(fmode))!=0)
    return setSysErr(context);
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysRmdir(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathnamep;
  CallArgs(context) >> pathnamep;
  Tstr     pathname(context,pathnamep);
  if(!pathname)
    return;
  if(rmdir(context->getFileSys()->toHost((const char *)pathname).c_str())!=0)
    return setSysErr(context);
  setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysUmask(ThreadContext *context, InstDesc *inst){
  Tmode_t  mask;         
  CallArgs(context) >> mask;             
  // TODO: Implement this call for non-zero mask
  if(mask)             
    printf("sysUmask_(0x%08x) called\n",mask);
  setSysRet(context);
}
template<ExecMode mode>  
void RealLinuxSys<mode>::sysReadLink(ThreadContext *context, InstDesc *inst){
  Tpointer_t pathp;
  Tpointer_t buf;
  Tsize_t    bufsiz;
  CallArgs(context) >> pathp >> buf >> bufsiz;
  Tstr path(context,pathp);
  if(!path)             
    return;
#ifdef DEBUG_FILES
  printf("[%d] readlink %s\n",context->gettid(),(const char *)path);
#endif
  char bufBuf[bufsiz];
  Tssize_t bufLen=readlink(context->getFileSys()->toHost((const char *)path).c_str(),bufBuf,bufsiz);
  if(bufLen==-1)
    return setSysErr(context);
  if(!context->canWrite(buf,bufLen))
    return setSysErr(context,VEFAULT);
  context->writeMemFromBuf(buf,bufLen,bufBuf);
  setSysRet(context,bufLen);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSocket(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_BENCH) || (defined DEBUG_SOCKET)
  printf("sysCall32_socket: not implemented at 0x%08x\n",context->getIAddr());
#endif
  setSysErr(context,VEAFNOSUPPORT);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysBind(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_BENCH) || (defined DEBUG_SOCKET)
  printf("sysBind called (continuing with EAFNOSUPPORT)\n");
#endif
  setSysErr(context,VEAFNOSUPPORT);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysConnect(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_BENCH) || (defined DEBUG_SOCKET)
  printf("sysConnect called (continuing with EAFNOSUPPORT)\n");
#endif
  setSysErr(context,VEAFNOSUPPORT);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSend(ThreadContext *context, InstDesc *inst){
#if (defined DEBUG_BENCH) || (defined DEBUG_SOCKET)
  printf("sysSend called (continuing with EAFNOSUPPORT)\n");
#endif
  setSysErr(context,VEAFNOSUPPORT);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSetRLimit(ThreadContext *context, InstDesc *inst){
  Tint       resource;
  Tpointer_t rlim;
  CallArgs(context) >> resource >> rlim;
  if(!context->canRead(rlim,Trlimit::getSize()))
    return setSysErr(context,VEFAULT);
  Trlimit buf(context,rlim);
  switch(resource){
  case VRLIMIT_STACK: break;
  case VRLIMIT_DATA:
    // Limit is already RLIM_INFINITY, so we don't care what the new size is
    break;
  case VRLIMIT_CORE:
    if(buf.rlim_cur!=buf.rlim_max)
      fail("sysSetRLimit for RLIMIT_CORE with rlim_cur != rlim_max\n");
    if(buf.rlim_cur!=0)
      fail("sysSetRLimit for RLIMIT_CORE with non-zero rlim_cur\n");      
    break;
  default:
    fail("sysCall32_setrlimit called for resource %d at 0x%08x\n",resource,context->getIAddr());
  }
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetRLimit(ThreadContext *context, InstDesc *inst){
  Tint       resource;
  Tpointer_t rlim;
  CallArgs(context) >> resource >>rlim;
  if(!context->canWrite(rlim,Trlimit::getSize()))
    return setSysErr(context,VEFAULT);
  Trlimit buf(VRLIM_INFINITY,VRLIM_INFINITY);
  switch(resource){
  case VRLIMIT_STACK:
  case VRLIMIT_NOFILE:
  case VRLIMIT_DATA:
  case VRLIMIT_AS:
  case VRLIMIT_CPU:
    break;
  case VRLIMIT_CORE:
    buf.rlim_cur=buf.rlim_max=0;
    break;
  defualt:
    printf("sysCall32_getrlimit called for unknown resource %d. Return RLIM_INFINITY.\n",resource);
  }
  buf.write(context,rlim);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysGetRUsage(ThreadContext *context, InstDesc *inst){
  Tint       who;
  Tpointer_t r_usage;
  CallArgs(context) >> who >> r_usage;
  if(!context->canWrite(r_usage,Trusage::getSize()))
    return setSysErr(context,VEFAULT);

  // TODO: This is a hack. See definition of these vars
  myUsrUsecs+=100;
  mySysUsecs+=1;

  Trusage(Ttimeval(myUsrUsecs/1000000,myUsrUsecs%1000000),
	  Ttimeval(mySysUsecs/1000000,mySysUsecs%1000000)).write(context,r_usage);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysUname(ThreadContext *context, InstDesc *inst){
  Tpointer_t buf;
  CallArgs(context) >> buf;
  Tutsname   bufBuf;
  if(!context->canWrite(buf,Tutsname::getSize()))
    return setSysErr(context,VEFAULT);
  strcpy((char *)(bufBuf.sysname),"GNU/Linux");
  strcpy((char *)(bufBuf.nodename),"sesc");
  strcpy((char *)(bufBuf.release),"2.6.22");
  strcpy((char *)(bufBuf.version),"#1 SMP Tue Jun 4 16:05:29 CDT 2002");
  strcpy((char *)(bufBuf.machine),"mips");
  bufBuf.write(context,buf);
  return setSysRet(context);
}
template<ExecMode mode>
void RealLinuxSys<mode>::sysSysCtl(ThreadContext *context, InstDesc *inst){
  Tpointer_t argsp;
  CallArgs(context) >> argsp;
  if(!context->canRead(argsp,T__sysctl_args::getSize()))
    return setSysErr(context,VEFAULT);
  T__sysctl_args argsbuf(context,argsp);
  if(!context->canRead(argsbuf.name,argsbuf.nlen*sizeof(Tint)))
    return setSysErr(context,VEFAULT);
  Tint name[argsbuf.nlen];
  for(int i=0;i<argsbuf.nlen;i++)
    name[i]=fixEndian(context->readMemRaw<Tint>(argsbuf.name+i*sizeof(Tint)));
  switch(name[0]){
  case VCTL_KERN:
    if(argsbuf.nlen<=1)
      return setSysErr(context,VENOTDIR);
    switch(name[1]){
    case VKERN_VERSION: {
      if(argsbuf.newval!=0)
        return setSysErr(context,VEPERM);
      if(!context->canRead(argsbuf.oldlenp,sizeof(Tsize_t)))
        return setSysErr(context,VEFAULT);
      Tsize_t oldlen(fixEndian(context->readMemRaw<Tsize_t>(argsbuf.oldlenp)));
      char ver[]="#1 SMP Tue Jun 4 16:05:29 CDT 2002";
      Tsize_t verlen=strlen(ver)+1;
      if(oldlen<verlen)
        return setSysErr(context,VEFAULT);
      if(!context->canWrite(argsbuf.oldlenp,sizeof(Tsize_t)))
        return setSysErr(context,VEFAULT);
      if(!context->canWrite(argsbuf.oldval,verlen))
        return setSysErr(context,VEFAULT);
      context->writeMemFromBuf(argsbuf.oldval,verlen,ver);
      context->writeMemRaw(argsbuf.oldlenp,fixEndian(verlen));
    } break;
    default:
      fail("sysCall32__sysctl: KERN name %d not supported\n",name[1]);
      return setSysErr(context,VENOTDIR);
    }
    break;
  default: 
    fail("sysCall32__sysctl: top-level name %d not supported\n",name[0]);
    return setSysErr(context,VENOTDIR);
  }
  return setSysRet(context);
}

#if (defined SUPPORT_MIPS32)
template<>
void RealLinuxSys<ExecModeMips32>::setThreadArea(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegTPtr>(context,RegTPtr,addr);
}
#endif
#if (defined SUPPORT_MIPSEL32)
template<>
void RealLinuxSys<ExecModeMipsel32>::setThreadArea(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegTPtr>(context,RegTPtr,addr);
}
#endif
#if (defined SUPPORT_MIPS64)
template<>
void RealLinuxSys<ExecModeMips64>::setThreadArea(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegTPtr>(context,RegTPtr,addr);
}
#endif
#if (defined SUPPORT_MIPSEL64)
template<>
void RealLinuxSys<ExecModeMipsel64>::setThreadArea(ThreadContext *context, Tpointer_t addr){
  setReg<Tpointer_t,RegTPtr>(context,RegTPtr,addr);
}
#endif

// On MIPS the stack grows down
#if (defined SUPPORT_MIPS32)
template<> int RealLinuxSys<ExecModeMips32>::stackGrowthSign(void){ return -1; }
#endif
#if (defined SUPPORT_MIPSEL32)
template<> int RealLinuxSys<ExecModeMipsel32>::stackGrowthSign(void){ return -1; }
#endif
#if (defined SUPPORT_MIPS64)
template<> int RealLinuxSys<ExecModeMips64>::stackGrowthSign(void){ return -1; }
#endif
#if (defined SUPPORT_MIPSEL64)
template<> int RealLinuxSys<ExecModeMipsel64>::stackGrowthSign(void){ return -1; }
#endif

#if (defined SUPPORT_MIPS32)
template<> RealLinuxSys<ExecModeMips32>::Tint RealLinuxSys<ExecModeMips32>::getSysCallNum(const ThreadContext *context){
  return getReg<Tint,RegV0>(context,RegV0);
}
#endif
#if (defined SUPPORT_MIPSEL32)
template<> RealLinuxSys<ExecModeMipsel32>::Tint RealLinuxSys<ExecModeMipsel32>::getSysCallNum(const ThreadContext *context){
  return getReg<Tint,RegV0>(context,RegV0);
}
#endif
#if (defined SUPPORT_MIPS64)
template<> RealLinuxSys<ExecModeMips64>::Tint RealLinuxSys<ExecModeMips64>::getSysCallNum(const ThreadContext *context){
  return getReg<Tint,RegV0>(context,RegV0);
}
#endif
#if (defined SUPPORT_MIPSEL64)
template<> RealLinuxSys<ExecModeMipsel64>::Tint RealLinuxSys<ExecModeMipsel64>::getSysCallNum(const ThreadContext *context){
  return getReg<Tint,RegV0>(context,RegV0);
}
#endif

template<ExecMode mode>
InstDesc *RealLinuxSys<mode>::sysCall(ThreadContext *context, InstDesc *inst){
  context->updIAddr(inst->aupdate,1);
  Tint sysCallNum=getSysCallNum(context);
//   switch(sysCallNum){
//   case 4001: case 4003: case 4004: case 4005: case 4006: case 4007: case 4010: 
//   case 4011: case 4013: case 4019:
//   case 4020: case 4024: case 4033: case 4038: case 4041: case 4042: case 4043: case 4045: case 4047: case 4049: 
//   case 4050:
//   case 4054: case 4060:
//   case 4063: case 4064: case 4075: case 4076: case 4077: case 4080: case 4090: case 4091: case 4106: 
//   case 4108: case 4114: case 4120: case 4122: 
//   case 4125: case 4140: case 4146: case 4153: case 4166: case 4167: case 4169: case 4183: case 4194: case 4195:
//   case 4203: case 4210:
//   case 4212: case 4213: case 4214: case 4215: case 4219: case 4220: case 4238: case 4246: case 4252:
//   case 4283: case 4309:
//     break;
//   default:
//     printf("Using Mips32Linuxsys::sysCall %d\n",sysCallNum);
//   }
  switch(sysCallNum){
  // Thread/process creation and destruction system calls
  case V__NR_clone:           sysClone(context,inst); break;
  case V__NR_fork:            sysFork(context,inst); break;
  case V__NR_execve:          sysExecVe(context,inst); break;
  case V__NR_exit:            return sysExit(context,inst); break;
  case V__NR_exit_group:      return sysExitGroup(context,inst); break;
  case V__NR_waitpid:         sysWaitpid(context,inst); break;
  case V__NR_wait4:           sysWait4(context,inst); break;
  // Thread/Process relationships system calls
  case V__NR_getpid:          sysGetPid(context,inst); break;
  case V__NR_gettid:          sysGetTid(context,inst); break;
  case V__NR_getppid:         sysGetPPid(context,inst); break;
  case V__NR_setpgid:         fail("V__NR_setpgid not implemented\n"); break;
  case V__NR_getpgid:         sysGetPGid(context,inst); break;
  case V__NR_getpgrp:         sysGetPGrp(context,inst); break;
  case V__NR_setsid:          fail("V__NR_setsid not implemented\n"); break;
  case V__NR_getsid:          fail("V__NR_getsid not implemented\n"); break;
  // Synchronization system calls
  case V__NR_futex:           sysFutex(context,inst); break;
  case V__NR_set_robust_list: sysSetRobustList(context,inst); break;
  case V__NR_get_robust_list: fail("V__NR_get_robust_list not implemented\n"); break;
  case V__NR_set_tid_address: sysSetTidAddress(context,inst); break;
  // Signal-related system calls
  case V__NR_kill:            sysKill(context,inst); break;
  case V__NR_tkill:           sysTKill(context,inst); break;
  case V__NR_tgkill:          sysTgKill(context,inst); break;
  case V__NR_rt_sigqueueinfo: fail("V__NR_rt_sigqueueinfo not implemented\n"); break;
    //  case V__NR_sigaction:       fail("V__NR_sigaction not implemented\n"); break;
  case V__NR_rt_sigaction:    sysRtSigAction(context,inst); break;
    //  case V__NR_sigprocmask:     fail("V__NR_sigprocmask not implemented\n"); break;
  case V__NR_rt_sigprocmask:  sysRtSigProcMask(context,inst); break;
    //  case V__NR_sigpending:      fail("V__NR_sigpending not implemented\n"); break;
  case V__NR_rt_sigpending:   fail("V__NR_rt_sigpending not implemented\n"); break;
    //  case V__NR_sigsuspend:      fail("V__NR_sigsuspend not implemented\n"); break;
  case V__NR_rt_sigsuspend:   sysRtSigSuspend(context,inst); break;
    //  case V__NR_signal:          fail("V__NR_signal not implemented\n"); break;
  case V__NR_signalfd:        fail("V__NR_signalfd not implemented\n"); break;
  case V__NR_sigaltstack:     fail("V__NR_sigaltstack not implemented\n"); break;
  case V__NR_rt_sigtimedwait: fail("V__NR_sigtimedwait not implemented\n"); break;
    //  case V__NR_sigreturn:       fail("V__NR_sigreturn not implemented\n"); break;
  case V__NR_rt_sigreturn:    sysRtSigReturn(context,inst); break;
  // Time-related system calls
  case V__NR_times:           sysTimes(context,inst); break;
  case V__NR_time:            sysTime(context,inst); break;
  case V__NR_settimeofday:    fail("V__NR_settimeofday not implemented\n"); break;
  case V__NR_gettimeofday:    sysGetTimeOfDay(context,inst); break;
  case V__NR_setitimer:       sysSetITimer(context,inst); break;
  case V__NR_getitimer:       fail("V__NR_getitimer not implemented\n"); break;
  case V__NR_clock_getres:    sysClockGetRes(context,inst); break;
  case V__NR_clock_settime:   fail("V__NR_clock_settime not implemented\n"); break;
  case V__NR_clock_gettime:   sysClockGetTime(context,inst); break;
  case V__NR_alarm:           sysAlarm(context,inst); break;
  case V__NR_nanosleep:       sysNanoSleep(context,inst); break;
  case V__NR_clock_nanosleep: sysClockNanoSleep(context,inst); break;
  // Process/thread scheduling calls
  case V__NR_sched_yield:            sysSchedYield(context,inst); break;
  case V__NR_setpriority:            fail("V__NR_setpriority not implemented\n"); break;
  case V__NR_getpriority:            sysGetPriority(context,inst); break;
  case V__NR_sched_getparam:         sysSchedGetParam(context,inst); break;
  case V__NR_sched_setparam:         fail("V__NR_sched_setparam not implemented\n"); break;
  case V__NR_sched_setscheduler:     sysSchedSetScheduler(context,inst); break;
  case V__NR_sched_getscheduler:     sysSchedGetScheduler(context,inst); break;
  case V__NR_sched_get_priority_max: sysSchedGetPriorityMax(context,inst); break;
  case V__NR_sched_get_priority_min: sysSchedGetPriorityMin(context,inst); break;
  case V__NR_sched_setaffinity:      sysSchedSetAffinity(context,inst); break;
  case V__NR_sched_getaffinity:      sysSchedGetAffinity(context,inst); break;
  case V__NR_sched_rr_get_interval:  fail("V__NR_sched_rr_get_interval not implemented\n"); break;
  // User info calls
  case V__NR_setuid:          fail("V__NR_setuid not implemented\n"); break;
  case V__NR_getuid:          sysGetUid(context,inst); break;
  case V__NR_setreuid:        fail("V__NR_setreuid not implemented\n"); break;
  case V__NR_setresuid:       fail("V__NR_setresuid not implemented\n"); break;
  case V__NR_setfsuid:        fail("V__NR_setfsuid not implemented\n"); break;
  case V__NR_geteuid:         sysGetEuid(context,inst); break;
  case V__NR_getresuid:       fail("V__NR_getresuid not implemented\n"); break;
  case V__NR_setgid:          fail("V__NR_setgid not implemented\n"); break;
  case V__NR_getgid:          sysGetGid(context,inst); break;
  case V__NR_setregid:        fail("V__NR_setregid not implemented\n"); break;
  case V__NR_setresgid:       fail("V__NR_setresgid not implemented\n"); break;
  case V__NR_setfsgid:        fail("V__NR_setfsgid not implemented\n"); break;
  case V__NR_getegid:         sysGetEgid(context,inst); break;
  case V__NR_getresgid:       fail("V__NR_getresgid not implemented\n"); break;
  case V__NR_setgroups:       fail("V__NR_setgroups not implemented\n"); break;
  case V__NR_getgroups:       sysGetGroups(context,inst); break;
  // Memory management calls
  case V__NR_brk:             sysBrk(context,inst); break;
  case V__NR_set_thread_area: sysSetThreadArea(context,inst); break;
  case V__NR_mmap:            sysMMap<1>(context,inst); break;
  case V__NR_mmap2:           sysMMap<4096>(context,inst); break;
  case V__NR_mremap:          sysMReMap(context,inst); break;
  case V__NR_munmap:          sysMUnMap(context,inst); break;
  case V__NR_mprotect:        sysMProtect(context,inst); break;
  // File-related system calls
  case V__NR_open:            sysOpen(context,inst); break;
  case V__NR_pipe:            sysPipe(context,inst); break;
  case V__NR_dup:             sysDup(context,inst); break;
  case V__NR_dup2:            sysDup2(context,inst); break;
  case V__NR_fcntl:           sysFCntl(context,inst); break;
  case V__NR_fcntl64:         sysFCntl(context,inst); break;
  case V__NR_read:            sysRead(context,inst); break;
  case V__NR_write:           sysWrite(context,inst); break;
  case V__NR_writev:          sysWriteV(context,inst); break;
  case V__NR_lseek:           sysLSeek(context,inst); break;
  case V__NR__llseek:         sysLLSeek(context,inst); break;
  case V__NR_getdents:        sysGetDEnts<Tdirent>(context,inst); break;
  case V__NR_getdents64:      sysGetDEnts<Tdirent64>(context,inst); break;
  case V__NR_ioctl:           sysIOCtl(context,inst); break;
  case V__NR_poll:            sysPoll(context,inst); break;
  case V__NR_close:           sysClose(context,inst); break;
  case V__NR_truncate:        sysTruncate<Toff_t>(context,inst); break;
  case V__NR_truncate64:      sysTruncate<Tloff_t>(context,inst); break;
  case V__NR_ftruncate:       sysFTruncate<Toff_t>(context,inst); break;
  case V__NR_ftruncate64    : sysFTruncate<Tloff_t>(context,inst); break;
  case V__NR_chmod:           sysChMod(context,inst); break;
  case V__NR_stat:            sysStat<false,Tstat>(context,inst); break;
  case V__NR_stat64:          sysStat<false,Tstat64>(context,inst); break;
  case V__NR_lstat:           sysStat<true,Tstat>(context,inst); break;
  case V__NR_lstat64:         sysStat<true,Tstat64>(context,inst); break;
  case V__NR_fstat:           sysFStat<Tstat>(context,inst); break;
  case V__NR_fstat64:         sysFStat<Tstat64>(context,inst); break;
  case V__NR_fstatfs:         sysFStatFS(context,inst); break;
  case V__NR_fstatfs64:       fail("V__NR_fstatfs64 not implemented\n"); break;
  case V__NR_unlink:          sysUnlink(context,inst); break;
  case V__NR_symlink:         sysSymLink(context,inst); break;
  case V__NR_rename:          sysRename(context,inst); break;
  case V__NR_chdir:           sysChdir(context,inst); break;
  case V__NR_access:          sysAccess(context,inst); break;
  case V__NR_getcwd:          sysGetCWD(context,inst); break;
  case V__NR_mkdir:           sysMkdir(context,inst); break;
  case V__NR_rmdir:           sysRmdir(context,inst); break;
  case V__NR_umask:           sysUmask(context,inst); break;
  case V__NR_readlink:        sysReadLink(context,inst); break;
  //#define __NR_utimes                     (__NR_Linux + 267)
  // Network related system calls
  case V__NR_socket:          sysSocket(context,inst); break;
  case V__NR_socketpair:      fail("V__NR_socketpair not implemented\n"); break;
  case V__NR_bind:            sysBind(context,inst); break;
  case V__NR_accept:          fail("V__NR_accept not implemented\n"); break;
  case V__NR_connect:         sysConnect(context,inst); break;
  //  case V__NR_send:            sysSend(context,inst); break;
    /*
  case 4171: sysCall32_getpeername(inst,context); break;
  case 4172: sysCall32_getsockname(inst,context); break;
  case 4173: sysCall32_getsockopt(inst,context); break;
  case 4174: sysCall32_listen(inst,context); break;
  case 4175: sysCall32_recv(inst,context); break;
  case 4176: sysCall32_recvfrom(inst,context); break;
  case 4177: sysCall32_recvmsg(inst,context); break;
  case 4179: sysCall32_sendmsg(inst,context); break;
  case 4180: sysCall32_sendto(inst,context); break;
  case 4181: sysCall32_setsockopt(inst,context); break;
    */
  // Process info system calls
  case V__NR_setrlimit:       sysSetRLimit(context,inst); break;
  case V__NR_getrlimit:       sysGetRLimit(context,inst); break;
  case V__NR_getrusage:       sysGetRUsage(context,inst); break;
  // System info calls
  case V__NR_uname:           sysUname(context,inst); break;
  case V__NR__sysctl:         sysSysCtl(context,inst); break;
	  
//  case 4000: sysCall32_syscall(inst,context); break;
//  case 4008: sysCall32_creat(inst,context); break;
//  case 4009: sysCall32_link(inst,context); break;
//  case 4014: sysCall32_mknod(inst,context); break;
//  case 4016: sysCall32_lchown(inst,context); break;
//  case 4017: sysCall32_break(inst,context); break;
//  case 4018: sysCall32_oldstat(inst,context); break;
//  case 4021: sysCall32_mount(inst,context); break;
//  case 4022: sysCall32_umount(inst,context); break;
//  case 4025: sysCall32_stime(inst,context); break;
//  case 4026: sysCall32_ptrace(inst,context); break;
//  case 4028: sysCall32_oldfstat(inst,context); break;
//  case 4029: sysCall32_pause(inst,context); break;
//  case 4030: sysCall32_utime(inst,context); break;
//  case 4031: sysCall32_stty(inst,context); break;
//  case 4032: sysCall32_gtty(inst,context); break;
//  case 4034: sysCall32_nice(inst,context); break;
//  case 4035: sysCall32_ftime(inst,context); break;
//  case 4036: sysCall32_sync(inst,context); break;
//  case 4044: sysCall32_prof(inst,context); break;
//  case 4051: sysCall32_acct(inst,context); break;
//  case 4052: sysCall32_umount2(inst,context); break;
//  case 4053: sysCall32_lock(inst,context); break;
//  case 4056: sysCall32_mpx(inst,context); break;
//  case 4057: sysCall32_setpgid(inst,context); break;
//  case 4058: sysCall32_ulimit(inst,context); break;
//  case 4059: sysCall32_unused59(inst,context); break;
//  case 4061: sysCall32_chroot(inst,context); break;
//  case 4062: sysCall32_ustat(inst,context); break;
//  case 4068: sysCall32_sgetmask(inst,context); break;
//  case 4069: sysCall32_ssetmask(inst,context); break;
//  case 4074: sysCall32_sethostname(inst,context); break;
//  case 4082: sysCall32_reserved82(inst,context); break;
//  case 4084: sysCall32_oldlstat(inst,context); break;
//  case 4086: sysCall32_uselib(inst,context); break;
//  case 4087: sysCall32_swapon(inst,context); break;
//  case 4088: sysCall32_reboot(inst,context); break;
//  case 4089: sysCall32_readdir(inst,context); break;
//  case 4094: sysCall32_fchmod(inst,context); break;
//  case 4095: sysCall32_fchown(inst,context); break;
//  case 4098: sysCall32_profil(inst,context); break;
//  case 4099: sysCall32_statfs(inst,context); break;
//  case 4101: sysCall32_ioperm(inst,context); break;
//  case 4102: sysCall32_socketcall(inst,context); break;
//  case 4103: sysCall32_syslog(inst,context); break;
//  case 4109: sysCall32_unused109(inst,context); break;
//  case 4110: sysCall32_iopl(inst,context); break;
//  case 4111: sysCall32_vhangup(inst,context); break;
//  case 4112: sysCall32_idle(inst,context); break;
//  case 4113: sysCall32_vm86(inst,context); break;
//  case 4115: sysCall32_swapoff(inst,context); break;
//  case 4116: sysCall32_sysinfo(inst,context); break;
// If you implement ipc() you must handle CLONE_SYSVSEM in clone()
//  case 4117: sysCall32_ipc(inst,context); break;
//  case 4118: sysCall32_fsync(inst,context); break;
//  case 4119: sysCall32_sigreturn(inst,context); break;
//  case 4121: sysCall32_setdomainname(inst,context); break;
//  case 4123: sysCall32_modify_ldt(inst,context); break;
//  case 4124: sysCall32_adjtimex(inst,context); break;
//  case 4127: sysCall32_create_module(inst,context); break;
//  case 4128: sysCall32_init_module(inst,context); break;
//  case 4129: sysCall32_delete_module(inst,context); break;
//  case 4130: sysCall32_get_kernel_syms(inst,context); break;
//  case 4131: sysCall32_quotactl(inst,context); break;
//  case 4133: sysCall32_fchdir(inst,context); break;
//  case 4134: sysCall32_bdflush(inst,context); break;
//  case 4135: sysCall32_sysfs(inst,context); break;
//  case 4136: sysCall32_personality(inst,context); break;
//  case 4137: sysCall32_afs_syscall(inst,context); break;
//  case 4142: sysCall32__newselect(inst,context); break;
//  case 4143: sysCall32_flock(inst,context); break;
//  case 4144: sysCall32_msync(inst,context); break;
//  case 4145: sysCall32_readv(inst,context); break;
//  case 4147: sysCall32_cacheflush(inst,context); break;
//  case 4148: sysCall32_cachectl(inst,context); break;
//  case 4149: sysCall32_sysmips(inst,context); break;
//  case 4150: sysCall32_unused150(inst,context); break;
//  case 4152: sysCall32_fdatasync(inst,context); break;
//  case 4154: sysCall32_mlock(inst,context); break;
//  case 4155: sysCall32_munlock(inst,context); break;
//  case 4156: sysCall32_mlockall(inst,context); break;
//  case 4157: sysCall32_munlockall(inst,context); break;
//  case 4182: sysCall32_shutdown(inst,context); break;
//  case 4187: sysCall32_query_module(inst,context); break;
//  case 4189: sysCall32_nfsservctl(inst,context); break;
//  case 4192: sysCall32_prctl(inst,context); break;
//  case 4200: sysCall32_pread64(inst,context); break;
//  case 4201: sysCall32_pwrite64(inst,context); break;
//  case 4202: sysCall32_chown(inst,context); break;
//  case 4204: sysCall32_capget(inst,context); break;
//  case 4205: sysCall32_capset(inst,context); break;
//  case 4207: sysCall32_sendfile(inst,context); break;
//  case 4208: sysCall32_getpmsg(inst,context); break;
//  case 4209: sysCall32_putpmsg(inst,context); break;
//  case 4216: sysCall32_root_pivot(inst,context); break;
//  case 4217: sysCall32_mincore(inst,context); break;
//  case 4218: sysCall32_madvise(inst,context); break;
//#define __NR_reserved221                (__NR_Linux + 221)
//#define __NR_readahead                  (__NR_Linux + 223)
//#define __NR_setxattr                   (__NR_Linux + 224)
//#define __NR_lsetxattr                  (__NR_Linux + 225)
//#define __NR_fsetxattr                  (__NR_Linux + 226)
//#define __NR_getxattr                   (__NR_Linux + 227)
//#define __NR_lgetxattr                  (__NR_Linux + 228)
//#define __NR_fgetxattr                  (__NR_Linux + 229)
//#define __NR_listxattr                  (__NR_Linux + 230)
//#define __NR_llistxattr                 (__NR_Linux + 231)
//#define __NR_flistxattr                 (__NR_Linux + 232)
//#define __NR_removexattr                (__NR_Linux + 233)
//#define __NR_lremovexattr               (__NR_Linux + 234)
//#define __NR_fremovexattr               (__NR_Linux + 235)
//#define __NR_sendfile64                 (__NR_Linux + 237)
//#define __NR_io_setup                   (__NR_Linux + 241)
//#define __NR_io_destroy                 (__NR_Linux + 242)
//#define __NR_io_getevents               (__NR_Linux + 243)
//#define __NR_io_submit                  (__NR_Linux + 244)
//#define __NR_io_cancel                  (__NR_Linux + 245)
//#define __NR_lookup_dcookie             (__NR_Linux + 247)
//#define __NR_epoll_create               (__NR_Linux + 248)
//#define __NR_epoll_ctl                  (__NR_Linux + 249)
//#define __NR_epoll_wait                 (__NR_Linux + 250)
//#define __NR_remap_file_pages           (__NR_Linux + 251)
//#define __NR_restart_syscall            (__NR_Linux + 253)
//#define __NR_fadvise64                  (__NR_Linux + 254)
//#define __NR_statfs64                   (__NR_Linux + 255)
//#define __NR_timer_create               (__NR_Linux + 257)
//#define __NR_timer_settime              (__NR_Linux + 258)
//#define __NR_timer_gettime              (__NR_Linux + 259)
//#define __NR_timer_getoverrun           (__NR_Linux + 260)
//#define __NR_timer_delete               (__NR_Linux + 261)
//#define __NR_mbind                      (__NR_Linux + 268)
//#define __NR_get_mempolicy              (__NR_Linux + 269)
//#define __NR_set_mempolicy              (__NR_Linux + 270)
//#define __NR_mq_open                    (__NR_Linux + 271)
//#define __NR_mq_unlink                  (__NR_Linux + 272)
//#define __NR_mq_timedsend               (__NR_Linux + 273)
//#define __NR_mq_timedreceive            (__NR_Linux + 274)
//#define __NR_mq_notify                  (__NR_Linux + 275)
//#define __NR_mq_getsetattr              (__NR_Linux + 276)
//#define __NR_vserver                    (__NR_Linux + 277)
//#define __NR_waitid                     (__NR_Linux + 278)
//#define __NR___NR_setaltroot             (__NR_Linux + 279)
//#define __NR_add_key                    (__NR_Linux + 280)
//#define __NR_request_key                (__NR_Linux + 281)
//#define __NR_keyctl                     (__NR_Linux + 282)
//#define __NR_inotify_init               (__NR_Linux + 284)
//#define __NR_inotify_add_watch          (__NR_Linux + 285)
//#define __NR_inotify_rm_watch           (__NR_Linux + 286)
//#define __NR_migrate_pages              (__NR_Linux + 287)
//#define __NR_openat                     (__NR_Linux + 288)
//#define __NR_mkdirat                    (__NR_Linux + 289)
//#define __NR_mknodat                    (__NR_Linux + 290)
//#define __NR_fchownat                   (__NR_Linux + 291)
//#define __NR_futimesat                  (__NR_Linux + 292)
//#define __NR_fstatat64                  (__NR_Linux + 293)
//#define __NR_unlinkat                   (__NR_Linux + 294)
//#define __NR_renameat                   (__NR_Linux + 295)
//#define __NR_linkat                     (__NR_Linux + 296)
//#define __NR_symlinkat                  (__NR_Linux + 297)
//#define __NR_readlinkat                 (__NR_Linux + 298)
//#define __NR_fchmodat                   (__NR_Linux + 299)
//#define __NR_faccessat                  (__NR_Linux + 300)
//#define __NR_pselect6                   (__NR_Linux + 301)
//#define __NR_ppoll                      (__NR_Linux + 302)
//#define __NR_unshare                    (__NR_Linux + 303)
//#define __NR_splice                     (__NR_Linux + 304)
//#define __NR_sync_file_range            (__NR_Linux + 305)
//#define __NR_tee                        (__NR_Linux + 306)
//#define __NR_vmsplice                   (__NR_Linux + 307)
//#define __NR_move_pages                 (__NR_Linux + 308)
//#define __NR_kexec_load                 (__NR_Linux + 311)
//#define __NR_getcpu                     (__NR_Linux + 312)
//#define __NR_epoll_pwait                (__NR_Linux + 313)
//#define __NR_ioprio_set                 (__NR_Linux + 314)
//#define __NR_ioprio_get                 (__NR_Linux + 315)
//#define __NR_utimensat                  (__NR_Linux + 316)
//#define __NR_timerfd                    (__NR_Linux + 318)
//#define __NR_eventfd                    (__NR_Linux + 319)
  default:
    fail("Unknown Mips32 syscall %d at 0x%08x\n",sysCallNum,context->getIAddr());
  }
#if (defined DEBUG_SYSCALLS)
  printf("[%d] sysCall %d\n",context->gettid(),sysCallNum);
#endif
  return inst;
}
