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
//
//===========================================================================

#ifndef MTL_UPLO_H
#define MTL_UPLO_H

#include <utility>
#include "mtl_config.h"
#include "matrix_traits.h"

namespace mtl {

  /* JGS add  is_lower() */

struct lower__;
struct unit_lower__;

//: specify an upper triangle matrix
//!noindex:
struct upper__ {
  typedef lower__ transpose_type;
  std::pair<int,int> bandwidth(int, int n) { return std::make_pair(0, n); }
  bool is_upper() const { return true; }
  bool is_unit() const { return false; }
};

/* following BLAS nomenclature for triangular matrix
 without diagnal
 */

//: specify a unit upper triangle matrix
//!noindex:
struct unit_upper__ {
  typedef unit_lower__ transpose_type;
  std::pair<int,int> bandwidth(int, int n) { return std::make_pair(-1, n); }
  bool is_upper() const { return true; }
  bool is_unit() const { return true; }
};

//: specify a lower triangle matrix
//!noindex:
struct lower__ {
  typedef upper__ transpose_type;
  std::pair<int,int> bandwidth(int m, int) { return std::make_pair(m, 0); }
  bool is_upper() const { return false; }
  bool is_unit() const { return false; }
};


//: specify a unit lower triangle matrix
//!noindex:
struct unit_lower__ {
  typedef unit_upper__ transpose_type;
  std::pair<int,int> bandwidth(int m, int) { return std::make_pair(m, -1); }
  bool is_upper() const { return false; }
  bool is_unit() const { return true; }
};

//: specify the uplo type dynamically (at run time)
//!noindex:
struct dynamic_uplo__ {
  typedef dynamic_uplo__ transpose_type;
  dynamic_uplo__(int uplo_) : uplo(uplo_) { }
  std::pair<int,int> bandwidth(int m, int n) const {
    std::pair<int,int> bw;
    switch (uplo) {
    case upper:
      bw = std::make_pair(0, n);
      break;
    case unit_upper:
      bw = std::make_pair(-1, n);
      break;
    case lower:
     bw = std::make_pair(m, 0);
     break;
    case unit_lower:
     bw = std::make_pair(m, -1);
     break;
    }
    return bw;
  }
  static std::pair<int,int> bandwidth(int uplo, int m, int n) {
    dynamic_uplo__ duplo(uplo);
    return duplo.bandwidth(m, n);
  }
  bool is_unit() const { return uplo == unit_upper || uplo == unit_lower; }
  bool is_upper() const { return uplo == upper || uplo == unit_upper; }
  int uplo;
};

} /* namespace mtl */

#endif
