#if !(defined ADDRESSING_H)
#define ADDRESSING_H

#include "SizedTypes.h"
// We need size_t for VAddr_hash
#include <stddef.h>

// There are three types of addresses used in the simulator

// Type for Real addresses (actual address in the simulator)
typedef uintptr_t RAddr;
// Type for Physical addresses (in simulated machine)
typedef uint32_t PAddr;
// Type for Virtual addresses (in simulated application)
#if (defined MIPS_EMUL)
typedef uint64_t VAddr;
#else
typedef uint32_t VAddr;
#endif
struct VAddr_hash{
  size_t operator()(VAddr __x) const { return ((__x>>8)^__x); }
};

// These are the native types for the target (simulated) machine
typedef uint16_t targUShort;
typedef int16_t  targShort;
typedef uint32_t targUInt;
typedef int32_t  targInt;
typedef uint32_t targULong;
typedef int32_t  targLong;
typedef uint64_t targULongLong;
typedef int64_t  targLongLong;

// Return value v aligned (up or down) to a multiple of a
template<class V, class A>
inline V alignDown(V v, A a){
  return v-(v&(a-1));
}
template<class V, class A>
inline V alignUp(V v, A a){
  return alignDown(v+a-1,a);
}
#endif
