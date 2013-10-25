#if !(defined EndianDefs_H)
#define EndianDefs_H

#include "ExecMode.h"

namespace Endian {

  template<typename T>
  inline T flip(T val){
    switch(sizeof(T)){
    case sizeof(uint8_t):
      return val;
    case sizeof(uint16_t):
      return (((val>> 8)&0x00ffu)|((val&0x00fful)<< 8));
    case sizeof(uint32_t):
      val=(((val>>16)&0x0000fffful)|((val&0x0000fffful)<<16));
      return (((val>> 8)&0x00ff00fful)|((val&0x00ff00fful)<< 8));
    case sizeof(uint64_t):
      val=(((val/0x0000000100000000ull)&0x00000000ffffffffull)+((val&0x00000000ffffffffull)*0x0000000100000000ull));
      val=(((val>>16)&0x0000ffff0000ffffull)|((val&0x0000ffff0000ffffull)<<16));
      return (((val>> 8)&0x00ff00ff00ff00ffull)|((val&0x00ff00ff00ff00ffull)<< 8));
    }
    fail("EndianHelper::flip with unexpected operand size %d\n",sizeof(T));
  }

  template<>
  inline float32_t flip<float32_t>(float32_t val){
    uint32_t tmp=flip(*(reinterpret_cast<uint32_t *>(&val)));
    return *(reinterpret_cast<float32_t *>(&tmp));
  }

  template<>
  inline float64_t flip<float64_t>(float64_t val){
    uint64_t tmp=flip(*(reinterpret_cast<uint64_t *>(&val)));
    return *(reinterpret_cast<float64_t *>(&tmp));
  }

  class BaseDefs{
  public:
    const static int natendian=__BYTE_ORDER;
  };

} // End of namespace EndianHelper

template<ExecMode mode>
class EndianDefs : public Endian::BaseDefs {
 private:
  const static int simendian=(((mode&ExecModeEndianMask)==ExecModeEndianBig)?__BIG_ENDIAN:__LITTLE_ENDIAN);
 public:
  template<typename T>
  inline static void cvtEndian(T &val){
    if(simendian==natendian)
      return;
    val=Endian::flip(val);
  }
  template<typename T>
  inline static T fixEndian(T val){
    if(simendian==natendian)
      return val;
    return Endian::flip(val);
  }
};


#endif // // !(defined EndianDefs_H)
