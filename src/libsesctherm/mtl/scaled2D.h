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

#ifndef MTL_SCALED2D_H
#define MTL_SCALED2D_H

#include "mtl_config.h"
#include "scaled1D.h"
#include "reverse_iter.h"

namespace mtl {

//: twod iter for scaled2D container type
//!noindex:
template <class twod_iter, class T, class ValType, class Ref>
class scaled2D_iter {
  typedef scaled2D_iter<twod_iter, T, ValType, Ref> self;
public:
  typedef ValType value_type;
  typedef Ref reference;
  typedef value_type* pointer;
#if defined(_MSVCPP_)
  typedef typename std::iterator_traits<twod_iter>::distance_type difference_type;
  typedef difference_type distance_type;
#else
  typedef typename std::iterator_traits<twod_iter>::difference_type difference_type;
#endif
  typedef typename std::iterator_traits<twod_iter>::iterator_category iterator_category;
  typedef typename ValType::size_type size_type;

  inline scaled2D_iter() { }
  inline scaled2D_iter(const twod_iter& x, const T& a)
    : iter(x), alpha(a) {}
  inline scaled2D_iter(const self& x) : iter(x.iter), alpha(x.alpha) { }
  inline self& operator=(const self& x) {
    iter = x.iter;
    alpha = x.alpha;
    return *this;
  }

  inline reference operator*() const { return reference(*iter, alpha); }
  inline reference operator[](size_type n) const {
    return reference(iter[n], alpha);
  }

  inline self& operator++() { ++iter; return *this; }
  inline self& operator+=(size_type n) { iter += n; return *this; }
  inline self operator++(int) { self t = *this; ++(*this); return t; }
  inline self& operator--() { --iter; return *this; }
  inline self& operator-=(size_type n) { iter -= n; return *this; }
  inline self operator--(int) { self t = *this; --(*this); return t; }
  inline bool operator!=(const self& x) const { return iter != x.iter; }
  inline bool operator==(const self& x) const { return iter == x.iter; }
  inline bool operator<(const self& x) const { return iter < x.iter; }
  inline difference_type operator-(const self& x) {
    return iter - x.iter; }
  inline size_type index() const { return iter.index(); }

protected:
  twod_iter iter;
  T alpha;
};

template <class TwoD, class T>
struct gen_scaled2D;

//: Scaled 2D container adaptor
//!category: containers, adaptors
//!component: type
//
//  This class is not meant to be used directly. Instead,
//  use the <TT>scaled()</TT> function to create a scaled matrix
//  to pass into an MTL algorithm.
//
template <class TwoD, class T>
class scaled2D {
  typedef scaled2D<TwoD,T> self;
public:

  template <class SubMatrix>
  struct partitioned {
    typedef scaled2D< TwoD, SubMatrix> type;
    typedef gen_scaled2D<TwoD, SubMatrix> generator;
  };

  /**@name Type Definitions */

  enum { M = TwoD::M, N = TwoD::N };

  //: The unsigned integral type for dimensions and indices
  typedef typename TwoD::size_type size_type;

  //: The 1D container type
  typedef scaled1D<typename TwoD::value_type> value_type;
  //: The type for references to value type
  typedef scaled1D<typename TwoD::value_type> reference;
  //: The type for const references to value type
  typedef const scaled1D<typename TwoD::value_type> const_reference;
  //: The iterator type (not used)
  typedef scaled2D_iter<typename TwoD::iterator, T,
                        value_type, reference>      iterator;
  //: The const iterator type
  typedef scaled2D_iter<typename TwoD::const_iterator, T,
                        value_type, reference>      const_iterator;
  //: The reverse iterator type (not used)
  typedef reverse_iter< scaled2D_iter<typename TwoD::reverse_iterator, T,
                        value_type, reference> >    reverse_iterator;
  //: The const reverse iterator type
  typedef reverse_iter< scaled2D_iter<typename TwoD::const_reverse_iterator, T,
                        value_type, reference> >    const_reverse_iterator;
  //: Either sparse_tag or dense_tag
  typedef typename TwoD::sparsity sparsity;
  //: Whether the underlying 2D container is strideable
  typedef typename TwoD::strideability strideability;
  //: Either internal or external storage
  typedef typename TwoD::storage_loc storage_loc;
  //: The transpose type
  typedef scaled2D< typename TwoD::transpose_type, T > transpose_type;

  /**@name Constructors */

  //: Default Constructor
  inline scaled2D() : alpha(0) { }
  //: Normal Constructor
  inline scaled2D(const TwoD& x, const T& a) : twod(x), alpha(a) { }


  /**@name Access Methods */


  /**@name Iterator Access Methods */

  //: Return a const iterator pointing to the first 1D container
  inline const_iterator begin() const {
    return const_iterator(twod.begin(), alpha);
  }
  //: Return a const iterator pointing past the end of the 2D container
  inline const_iterator end() const {
    return const_iterator(twod.end(), alpha);
  }

  /* reverse iterators */

  //: Return a const reverse iterator pointing to the last 1D container
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  //: Return a const reverse iterator pointing past the start of the 2D container
  inline const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  /**@name Element Access Methods */

  //: Return a const reference to the ith 1D container
  inline reference operator[](int i) const {
    return reference(twod[i], alpha);
  }
  //: Return a const reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline T operator()(int i, int j) const {
    return twod(i, j) * alpha;
  }


  /**@name Size Methods */

  //: The dimension of the 2D container
  inline int major() const { return twod.major(); }
  //: The dimension of the 1D containers
  inline int minor() const { return twod.minor(); }
  //: The number of non-zeros
  inline size_type nnz() const { return twod.nnz(); }

  /*
  inline self sub_matrix(int m_start, int n_start, int m, int n) {

  }
  */


protected:
  TwoD twod;
  T alpha;
};

//: Type Generator Class
//!noindex:
template <class TwoD, class T>
struct gen_scaled2D {
  typedef gen_scaled2D<typename TwoD::transpose_type, T> transpose_type;
  typedef gen_scaled2D<typename TwoD::submatrix_type, T> submatrix_type;
  typedef gen_scaled2D<typename TwoD::banded_view_type, T> banded_view_type;

  typedef scaled2D<TwoD, T> type;
#if 0
  //: blah
  //!noindex:
  template <int M, int N>
  struct bind {
    typedef scaled2D<TwoD, T> type;
  };
#endif
};


} /* namespace mtl */



#endif
