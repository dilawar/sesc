#if !(defined _MemState_h_)
#define _MemState_h_

// Comment this out to disable all MemState code
//#define HAS_MEM_STATE

#if (defined HAS_MEM_STATE)

#include "SizedTypes.h"

// Add additional defines here to control what MemState should do

class MemState{
 public:
  enum {
    Granularity=sizeof(uint32_t)
  };
  uint8_t st;
  MemState(void) : st(0){
  };
  MemState(ChkReader &in){
    in.read(reinterpret_cast<char *>(&st),sizeof(st));    
  }
  void save(ChkWriter &out) const{
    out.write(reinterpret_cast<const char *>(&st),sizeof(st));
  }
};

#endif // (defined HAS_MEM_STATE)
#endif // !(defined _MemState_h_)
