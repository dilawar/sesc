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
//
//===========================================================================

#ifndef MTL_SCALE_ITERATOR_H
#define MTL_SCALE_ITERATOR_H

#include "mtl_config.h"

#include "mtl_iterator.h"


namespace mtl {


  //: scale iterator
  //!category: iterators, adaptors
  //!component: type
  //
  // The scale iterator is an adaptor which multiplies the
  // value of the underlying element by some scalar as
  // they are access (through the dereference operator).
  // Scale iterators are somewhat different from most in
  // that they are always considered to be a constant iterator
  // whether or not the underlying elements are mutable.
  //
  // Typically users will not need to use scale iterator
  // directly. It is really just an implementation detail
  // of the scaled1D container.
  //
  //!definition: scale_iterator.h
  //!tparam: RandomAccessIterator - The underlying iterator
  //!tparam: T - The type of the scalar to multiply by
  //!models: RandomAccessIterator
  //!typereqs: T must be convertible to RandomAccessIterator's value_type
  //!typereqs: RandomAccessIterator's value_type must be a model of Ring
template <class RandomAccessIterator, class T>
class scale_iterator {
  typedef scale_iterator<RandomAccessIterator, T> self;
public:
  //: The value type
  typedef typename std::iterator_traits<RandomAccessIterator>::value_type
                               value_type;
#if !defined ( _MSVCPP_ )
  //: The difference type
	typedef typename std::iterator_traits<RandomAccessIterator>::difference_type
                               difference_type;
  //: The pointer type
  typedef typename std::iterator_traits<RandomAccessIterator>::pointer pointer;
#else
	typedef typename std::iterator_traits<RandomAccessIterator>::distance_type
                               difference_type;
	typedef difference_type distance_type;
	typedef value_type* pointer;
#endif
  //: The iterator category
  typedef typename std::iterator_traits<RandomAccessIterator>::iterator_category
                               iterator_category;

  typedef difference_type      Distance;
  typedef RandomAccessIterator iterator_type;
  //: The reference type
  typedef value_type reference;
  typedef value_type const_reference;

  //: The default constructor
  //!wheredef: Trivial Iterator
  inline scale_iterator() : alpha(0) { }

  //!wheredef: scale_iterator
  inline scale_iterator(const RandomAccessIterator& x)
    : current(x), alpha(1) { }

  //: Normal constructor
  //!wheredef: scale_iterator
  inline scale_iterator(const RandomAccessIterator& x, const value_type& a)
    : current(x), alpha(a) { }

  //: Copy constructor
  //!wheredef: Trivial Iterator
  inline scale_iterator(const self& x)
    : current(x.current), alpha(x.alpha) { }

  //: MTL index method
  //!wheredef: Indexible Iterator
  inline int index() const { return current.index(); }

  //: Convert to base iterator
  //!wheredef: scale_iterator
  inline operator RandomAccessIterator() { return current; }

  //: Access base iterator
  //!wheredef: scale_iterator
  inline RandomAccessIterator base() const { return current; }

  //: Dereference (and scale)
  //!wheredef: Trivial Iterator
  inline value_type operator*() const { return alpha * *current; }

  //: Preincrement
  //!wheredef: Forward Iterator
  inline self& operator++ () { ++current; return *this; }

  //: Postincrement
  //!wheredef: Forward Iterator
  inline self operator++ (int) { self tmp = *this; ++current; return tmp; }

  //: Preincrement
  //!wheredef: Bidirectional Iterator
  inline self& operator-- () { --current; return *this; }

  //: Postincrement
  //!wheredef: Bidirectional Iterator
  inline self operator-- (int) { self tmp = *this; --current; return tmp; }

  //: Iterator addition
  //!wheredef: Random Access Iterator
  inline self operator+ (Distance n) const {
    self c = current;
    c += n;
    return self(c.current, alpha);
  }

  //: Advance a distance
  //!wheredef: Random Access Iterator
  inline self& operator+= (Distance n) { current += n; return *this; }

  //: Subtract a distance
  //!wheredef: Random Access Iterator
  inline self operator- (Distance n) const {
    return self (current - n, alpha);
  }

  inline difference_type operator- (const self& x) const {
    return current - x.current;
  }

  //: Retreat a distance
  //!wheredef: Random Access Iterator
  inline self& operator-= (Distance n) { current -= n; return *this; }

  //: Access at an offset
  inline value_type operator[] (Distance n) const {
    return alpha * *(current + n);
  }
  //: Equality
  //!wheredef: Trivial Iterator
  inline bool operator==(const self& x) const { return current == x.current; }

  //: Inequality
  //!wheredef: Trivial Iterator
  inline bool operator!=(const self& x) const { return current != x.current; }

  //: Less than
  //!wheredef: Random Access Iterator
  inline bool operator<(const self& x) const { return current < x.current; }

protected:
  RandomAccessIterator current;
  T alpha;
};

/*
 * JGS taking this out ... the scale_iterator
 * cannot be optimized properly in BLAIS
 *
template <class RandomAccessIterator, class T>
inline scale_iterator<RandomAccessIterator, T>
scl(const RandomAccessIterator& x, const T& alpha) {
  return scale_iterator<RandomAccessIterator, T>(x, alpha);
}
*/

} /* namespace mtl */

#endif
