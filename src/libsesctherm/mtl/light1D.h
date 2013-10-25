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

#ifndef MTL_LIGHT1D_H
#define MTL_LIGHT1D_H

#include "mtl_iterator.h"

#include "mtl_config.h"
#include "dense_iterator.h"
#include "reverse_iter.h"
#include "matrix_traits.h"
#include "scaled1D.h"
#include <stdlib.h>

namespace mtl {

/**
  This is a {\em light} version of {\tt dense1D}.
  It does no memory management (or reference counting)
  and can only be used with pre-existing memory.
  The purpose of {\tt light1D} is to be used
  in the high performance kernels.

  @memo Light 1-D Container
 */
template <class T, int NN = 0, int IND_OFFSET = 0>
class light1D {
  typedef light1D self;
public:
  enum { N = NN };

  typedef light1D<int> IndexArray; /* JGS */

  /**@name Type Definitions */
  //@{
  ///
  typedef dense_tag sparsity;
  ///
  typedef scaled1D< light1D<T> > scaled_type;
  ///
  typedef T value_type;
  ///
  typedef T& reference;
  ///
  typedef T* pointer;
  ///
  typedef const T& const_reference;
  ///
  typedef const T* const_pointer;

  typedef int size_type;
  typedef ptrdiff_t difference_type;

#if defined(_MSVCPP_)
  ///
  typedef dense_iterator<T, 0, IND_OFFSET> iterator;
  ///
  typedef dense_iterator<T, 1, IND_OFFSET> const_iterator;
  ///
#elif defined( _MSVCPP7_ )
  /// used std::_Ptrit in order to support iterator_traits for
  /// pointers masquerading as iterators as per std::vector and std::basic_string - BEL
  //
  typedef std::_Ptrit<value_type, difference_type, pointer, reference, pointer, reference> ptr_iterator;
  typedef std::_Ptrit<value_type, difference_type, const_pointer, const_reference, pointer, reference> ptr_const_iterator;
  ///
  typedef dense_iterator<ptr_iterator, IND_OFFSET> iterator;
  ///
  typedef dense_iterator<ptr_const_iterator, IND_OFFSET> const_iterator;
  ///
#else
  ///
  typedef dense_iterator<T*, IND_OFFSET> iterator;
  ///
  typedef dense_iterator<const T*, IND_OFFSET> const_iterator;

#endif
  ///
  typedef reverse_iter<iterator> reverse_iterator;
  ///
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  typedef self IndexArrayRef;

  typedef self subrange_type;

  typedef oned_tag dimension;

  //@}

  /**@name Constructors */
  //@{
  /// Default Constructor
  inline light1D() : rep(0), size_(0), first(0) { }

  /// Preallocated Memory Constructor with optional non-zero starting index
  inline light1D(T* data, size_type n, size_type start = 0)
    : rep(data), size_(n), first(start) { }

  /// Copy Constructor
  inline light1D(const self& x)
    : rep(x.rep), size_(x.size_), first(x.first) { }

  inline ~light1D() { }

  //@}

  /**@name Access Methods */
  //@{
  /**@name Iterator Access Methods */
  //@{
  ///
  inline iterator begin() { return iterator(rep, 0, first); }
  ///
  inline iterator end() { return iterator(rep, size_, first); }
  ///
  inline const_iterator begin() const {
    return const_iterator(rep, 0, first);
  }
  ///
  inline const_iterator end() const{
    return const_iterator(rep, size_, first);
  }
  ///
  inline reverse_iterator rbegin() {

    return reverse_iterator(end());
  }
  ///
  inline reverse_iterator rend() { return reverse_iterator(begin()); }
  ///
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  ///
  inline const_reverse_iterator rend() const{
    return const_reverse_iterator(begin());
  }
  //@}
  /**@name Element Access Methods */
  //@{
  ///
  inline reference operator[](size_type n) { return rep[n - first]; }
  ///
  inline const_reference operator[](size_type n) const {
    return rep[n - first];
  }

  inline subrange_type operator()(size_type s, size_type f) const {
    return subrange_type(rep + s - first, f - s, 0);
  }
  //@}
  /**@name Size Methods */
  //@{
  ///
  inline int size() const { return size_; }
  ///
  inline int nnz() const { return size_; }
  ///
  inline void resize(int n) {
    if (rep) delete [] rep;
    size_ = n;
    rep = new T[size_];
  }
  inline self& adjust_index(size_type delta) {
    first += delta;
    return *this;
  }

  /// Memory Access
  inline T* data() const { return rep; }

protected:
  T* rep;
  int size_;
  int first;
};

} /* namespace mtl */

#endif // MTL_LIGHT1D_H

