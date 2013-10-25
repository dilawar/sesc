//
// Software License for MTL
//
// Copyright (c) 2001-2005 The Trustees of Indiana University. All rights reserved.
// Copyright (c) 1998-2001 University of Notre Dame. All rights reserved.
// Authors: Andrew Lumsdaine, Jeremy G. Siek, Lie-Quan Lee
//
// This file is part of the Matrix Template Library
//
// See also license.mtl.txt in the distribution.

#ifndef MTL_CONJ_H
#define MTL_CONJ_H

#include "mtl_config.h"

namespace std {

// dummy conj function for real numbers
inline double conj(double a) {
  return a;
}
inline float conj(float a) {
  return a;
}
inline int conj(int a) {
  return a;
}
inline bool conj(bool a) {
  return a;
}

// dummy real and imag function for real numbers
inline double real(double a) {
  return a;
}
inline double imag(double) {
  return 0.0;
}

inline float real(float a) {
  return a;
}
inline float imag(float) {
  return 0.0;
}

// JGS need to add conj() adapters for vectors and matrices
// used in application of householder transform

} /* namespace mtl */

#endif
