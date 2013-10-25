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

#ifndef MTL_NOT_AT_H
#define MTL_NOT_AT_H

#include "mtl_iterator.h"

#include "mtl_config.h"


namespace mtl {

using std::random_access_iterator_tag;
using std::input_iterator_tag;

template <class Ran1, class Ran2>
inline bool not_at(const Ran1& a, const Ran2& b, random_access_iterator_tag) {
  return a < b;
}

template <class Iter1, class Iter2>
inline bool not_at(const Iter1& a, const Iter2& b, input_iterator_tag) {
  return a != b;
}

template <class Iter1, class Iter2>
inline bool not_at(const Iter1& a, const Iter2& b) {
#if !defined ( _MSVCPP_ )
	typedef typename std::iterator_traits<Iter1>::iterator_category Category;
  return mtl::not_at(a, b, Category());
#else
  return a != b;
#endif
}

inline bool not_at(int a, int b) {
  return a < b;
}

} /* namespace mtl */

#endif // MTL_NOT_AT_H
