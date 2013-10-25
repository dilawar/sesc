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
//===========================================================================

#ifndef MTL_BARE_BONES_ARRAY_H
#define MTL_BARE_BONES_ARRAY_H

#include "light1D.h"
#include <algorithm>

namespace mtl {

template <class T>
class bare_bones_array : public light1D<T> {
public:
  inline bare_bones_array(T* data, int n, int start = 0)
    : light1D<T>(data, n, start), owns_data(false) { }
  inline bare_bones_array(int n) : light1D<T>(new T[n], n), owns_data(true) { }
  inline bare_bones_array(const bare_bones_array& x) 
    : light1D<T>(new T[x.size()], x.size()), owns_data(false) {
      std::copy(x.begin(), x.end(), begin());
  }
  template <class Container>
  inline bare_bones_array& operator=(const Container& x) {
    // JGS assert sizes are the same
    MTL_ASSERT(size() == x.size(), "bare__bones_array::operator=");
    std::copy(x.begin(), x.end(), begin());    
  }
  inline ~bare_bones_array() { if (rep && owns_data) delete rep; }
protected:
  bool owns_data;
};

} /* namespace mtl */

#endif /* MTL_BARE_BONES_ARRAY_H */
