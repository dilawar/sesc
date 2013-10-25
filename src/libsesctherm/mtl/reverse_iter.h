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

#ifndef MTL_REVERSE_ITER_H
#define MTL_REVERSE_ITER_H


#include "mtl_iterator.h"

#include "mtl_config.h"

namespace mtl {


#if STD_REVERSE_ITER
template <class Iter>
class reverse_iter : public std::reverse_iterator<Iter> {
	typedef std::reverse_iterator<Iter> super;
#else
template <class Iter>
class reverse_iter : public std::reverse_iterator<Iter,typename Iter::value_type,
                             typename Iter::reference, typename Iter::pointer> {
  typedef std::reverse_iterator<Iter, typename Iter::value_type,
	  typename Iter::reference, typename Iter::pointer> super;
#endif
public:
  typedef typename super::value_type value_type;

#if defined(_MSVCPP_)
  typedef typename super::distance_type difference_type;
  typedef difference_type distance_type;
  typedef typename super::reference_type reference;
#else
  typedef typename super::difference_type difference_type;
  typedef typename super::reference reference;
#endif


  typedef typename super::iterator_category iterator_category;

  inline reverse_iter() {}

  inline reverse_iter(const reverse_iter& x) : super(x) { }

  inline explicit
  reverse_iter(Iter x) : super(x) {}

  inline difference_type index() const {
    Iter tmp = super::current;
    return (--tmp).index();
  }
  inline difference_type row() const {
    Iter tmp = super::current;
    return (--tmp).row();
  }
  inline difference_type column() const {
    Iter tmp = super::current;
    return (--tmp).column();
  }

};

} /* namespace mtl */

#endif
