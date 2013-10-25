#ifndef SIGNAL_HANDLING_H
#define SIGNAL_HANDLING_H

#include "Addressing.h"
#include "GCObject.h"
#include "Checkpoint.h"
#include <bitset>
#include <vector>

// Define this to enable debugging of signal handling and delivery
//#define DEBUG_SIGNALS

typedef enum{
  SigActDefault,
  SigActIgnore,
  SigActTerm,
  SigActCore,
  SigActStop,
  SigActHandle,
} SignalAction;

typedef enum{
  SigNone=  0,
  SigNMin=  1,
  SigNMax=128,
  SigChld,
  SigAlrm,
  SigIO,
  NumSignals,
  SigDetached,
} SignalID;

typedef enum {SigCodeIn, SigCodeOut, SigCodeChldExit, SigCodeUser} SigCode;

class SigInfo{
 public:
  SignalID signo;
  SigCode  code;

  int32_t pid;
  int32_t uid;

  int32_t data; // Status for SIGCLD, fd for SIGIO
  SigInfo(){
  }
  SigInfo(SignalID signo, SigCode code) : signo(signo), code(code){
  }
  void save(ChkWriter &out) const;
  void restore(ChkReader &in);
};
inline ChkWriter &operator<< (ChkWriter &out, const SigInfo &si){
  si.save(out); return out;
}
inline ChkReader &operator>> (ChkReader &in, SigInfo &si){
  si.restore(in); return in;
}

SignalAction getDflSigAction(SignalID sig);

typedef std::bitset<NumSignals> SignalSet;

typedef std::vector<SigInfo *>  SignalQueue;

typedef enum{
  SaNoDefer=1,
  SaSigInfo=2,
  SaRestart=4,
} SaSigFlags;

class SignalDesc{
 public:
  VAddr         handler;
  SignalSet     mask;
  SaSigFlags flags;
  SignalDesc(void)
    : handler(SigActTerm), mask(), flags(SaSigFlags(0)){
  }
  SignalDesc(VAddr handler, const SignalSet &mask, SaSigFlags flags)
    : handler(handler), mask(mask), flags(flags){
  }
  SignalDesc &operator=(const SignalDesc &src){
    handler=src.handler;
    mask=src.mask;
    flags=src.flags;
    return *this;
  }
};

class SignalTable : public GCObject{
 public:
  typedef SmartPtr<SignalTable> pointer;
 private:
  SignalDesc table[NumSignals];
 public:
  SignalTable(void) : GCObject(){
    for(size_t i=0;i<NumSignals;i++)
      table[i].handler=getDflSigAction((SignalID)i);
  }
  SignalTable(const SignalTable &src) : GCObject(){
    for(size_t i=0;i<NumSignals;i++)
      table[i]=src.table[i];
  }
  ~SignalTable(void);
  SignalDesc &operator[](size_t sig){
    return table[sig];
  }
  void save(ChkWriter &out) const;
  void restore(ChkReader &in);
};
inline ChkWriter &operator<< (ChkWriter &out, const SignalTable &st){
  st.save(out); return out;
}
inline ChkReader &operator>> (ChkReader &in, SignalTable &st){
  st.restore(in); return in;
}


#endif // SIGNAL_HANDLING_H

