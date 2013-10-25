#ifndef MTL_STD_ABS_H
#define MTL_STD_ABS_H

#include "mtl_config.h"

#ifndef HAVE_STD_ABS
namespace std {
  inline double abs(double a) {
    return a > 0 ? a : -a;
  }
  inline long double abs(long double a) {
    return a > 0 ? a : -a;
  }
  inline float abs(float a) {
    return a > 0 ? a : -a;
  }
  inline long abs(long a) {
    return a > 0 ? a : -a;
  }
#if !defined ( _MSVCPP7_ )
  inline int abs(int a) {
    return a > 0 ? a : -a;
  }
#endif
}
#endif

#ifdef __SUNPRO_CC //sigh
namespace std {
  inline int abs(int a) {
    return a > 0 ? a : -a;
  }
}
#endif

#endif
