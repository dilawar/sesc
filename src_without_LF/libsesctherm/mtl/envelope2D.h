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

#ifndef MTL_ENVELOPE2D_H
#define MTL_ENVELOPE2D_H


#include "dense1D.h"
#include "scaled1D.h"
#include "light1D.h"
#include "entry.h"
#include "reverse_iter.h"
#include "utils.h"
#include "dimension.h"

namespace mtl {


template <class T, int M, int N>
struct gen_envelope2D;

  //: Envelope Sparse Matrix Storage Implementation
  //
  // This is the TwoDStorage type that implements the envelope
  // matrix storage format.
  //
  // The data structures used are two arrays. The VAL array holds the
  // values in the sparse matrix, and the PTR array points to the
  // diagonal elements in VAL. The OneD segments of VAL are actually
  // dense in the sense that zeros are stored. Each OneD segment
  // starts at the first non-zero element in the row.
  //
  //  <pre>
  //  operator()(i,j) { return val[ptr[i] - i + j]; }
  //  </pre>
  //
  // Use with banded<lower> shape. Use with upper not yet supported.
  //
  //!models: TwoDStorage
  //!category: containers
  //!component: type
  //!tparam: T - the element type
template <class T>
class envelope2D {
public:
  /*  need to replace the values implementation with something
      that allows for external pointers
      */

  typedef dense1D<T> values_t;
  typedef typename values_t::iterator values_iterator;
  typedef typename values_t::const_iterator const_values_iterator;

  typedef typename values_t::size_type size_type;
  typedef typename values_t::difference_type difference_type;
  typedef dense1D<size_type> ptr_t;

  //: A pair type for the dimensions of the container
  typedef dimension<size_type> dim_type;
  enum { M = 0, N = 0 };
  //: This container uses internal storage
  typedef internal_tag storage_loc;
  //: This container is 2D
  typedef twod_tag dimension;

  class vec_ref {
  public:
    enum { N = 0 };
#if !defined( _MSVCPP_ )
    typedef dense_iterator<values_iterator> iterator;
    typedef dense_iterator<const_values_iterator> const_iterator;
#else
    typedef dense_iterator<T, 0> iterator;
    typedef dense_iterator<T, 1> const_iterator;
#endif
    typedef reverse_iter<iterator> reverse_iterator;
    typedef reverse_iter<const_iterator> const_reverse_iterator;
    typedef typename values_t::value_type value_type;
    typedef typename values_t::pointer pointer;
    typedef elt_ref<vec_ref> reference;
    typedef const_elt_ref<vec_ref> const_reference;
    typedef typename values_t::size_type size_type;
    typedef typename values_t::difference_type difference_type;
    typedef scaled1D<vec_ref> scaled_type;
    typedef dense_tag sparsity;
    typedef oned_tag dimension;

    /* JGS, these need */
    typedef light1D<T> subrange_type;
    typedef dense1D<size_type> IndexArrayRef;

    inline vec_ref(size_type major, values_t* v, ptr_t* p)
      : i(major), val_p(v), ptr_p(p) { }

    inline vec_ref(const vec_ref& x)
      : i(x.i), val_p(x.val_p), ptr_p(x.ptr_p) { }

    inline iterator begin() {
      return iterator(__begin(), 0, start_index());
    }
    inline iterator end() {
      return iterator(__begin(), size(), finish_index());
    }
    inline const_iterator begin() const {
      return const_iterator(__begin(), 0, start_index());
    }
    inline const_iterator end() const {
      return const_iterator(__begin(), size(), finish_index());
    }
    inline reverse_iterator rbegin() { return reverse_iterator(end()); }
    inline reverse_iterator rend() { return reverse_iterator(begin()); }

    inline const_reverse_iterator rbegin() const {
      return const_reverse_iterator(end());
    }
    inline const_reverse_iterator rend() const {
      return const_reverse_iterator(begin());
    }

    inline reference operator[](size_type ii) {
      return reference(*this, ii);
    }
    inline const_reference operator[](size_type ii) const {
      return const_reference(*this, ii);
    }

    inline size_type size() const {
      if (i > 0)
	return (*ptr_p)[i] - (*ptr_p)[i-1];
      else
	return (*ptr_p)[i] + 1;
    }
    inline size_type nnz() const { return size(); }

    /*  elt_ref required functions
	the elt_ref interface doesn't seem to fit
	envelope2D very well. perhaps need to rethink elt_ref
     */

    /* find */
    inline iterator find(size_type j) MTL_THROW_ASSERTION {
      MTL_ASSERT(j <= i, "envelope2D::vec_ref::find()");
      if (j >=  start_index()) {
	return begin() + j + size() - i - 1;
      } else {
	return end();
      }
    }
    inline const_iterator find(size_type j) const MTL_THROW_ASSERTION {
      MTL_ASSERT(j <= i, "envelope2D::vec_ref::find()");
      if (j >= start_index())
	return const_iterator(__begin(), j + size() - i - 1);
      else
	return end();
    }
    /* insert
       this must be an insertion before the
       start of this row's envelope
     */
    inline iterator insert(iterator /*iter*/, size_type j, T v) {
      size_type increase = i + 1 - j - size();
      val_p->insert(__begin(), increase, T(0));
      increment_ptr(increase);
      *begin() = v;
      return begin();
    }

  protected:
    inline size_type start_index() const { return i - size() + 1; }
    inline size_type finish_index() const { return i; }

    inline values_iterator __begin() {
      if (i > 0)
	return val_p->begin() + (*ptr_p)[i-1] + 1;
      return val_p->begin();
    }
    inline const_values_iterator __begin() const {
      if (i > 0)
	return ((const values_t*)val_p)->begin() + (*ptr_p)[i-1] + 1;
      else
	return ((const values_t*)val_p)->begin();
    }
    inline void increment_ptr(size_type amount) {
      for (size_type ii = i; ii < ptr_p->size(); ++ii)
	(*ptr_p)[ii] += amount;
    }

    size_type i;
    values_t* val_p;
    ptr_t* ptr_p;
  };

  //: The 1D container type
  typedef vec_ref value_type;
  //: Reference to the value type
  typedef vec_ref reference;
  //: Const reference to the value type
  typedef const vec_ref const_reference;

  //: This is a sparse container
  typedef sparse_tag sparsity;
  //: This container is not strideable
  typedef not_strideable strideability;
  typedef envelope2D<int> transpose_type; /*JGS bogys type*/

  /* the 2-D iterator */

  //: The iterator type
  class iterator {
    typedef iterator self;
  public:
    typedef typename std::iterator_traits<values_iterator>::iterator_category iterator_category;
    typedef typename std::iterator_traits<values_iterator>::difference_type difference_type;
    typedef vec_ref value_type;
    typedef vec_ref reference;
    typedef vec_ref* pointer;
    inline iterator() : i(0), val_p(0), ptr_p(0) { }
    inline iterator(values_t* v, ptr_t* p, size_type ii)
      : val_p(v), ptr_p(p), i(ii) { }
    inline vec_ref operator*() const { return vec_ref(i, val_p, ptr_p); }
    inline self& operator++() { ++i; return *this; }
    inline self& operator+=(size_type n) { i += n; return *this; }
    inline self operator++(int) { self t = *this; ++(*this); return t; }
    inline self& operator--() { --i; return *this; }
    inline self& operator-=(size_type n) { i -= n; return *this; }
    inline self operator--(int) { self t = *this; --(*this); return t; }
    inline bool operator!=(const self& x) const { return i != x.i; }
    inline bool operator==(const self& x) const { return i == x.i; }
    inline bool operator<(const self& x) const { return i < x.i; }
    inline size_type index() const { return i; }
  protected:
    size_type i;
    values_t* val_p;
    ptr_t* ptr_p;
  };

  /* JGS would like to merge this with iterator*/

  //: The const iterator type
  class const_iterator : public iterator {
    typedef const_iterator self;
  public:
    typedef typename std::iterator_traits<values_iterator>::iterator_category iterator_category;
    typedef typename std::iterator_traits<values_iterator>::difference_type difference_type;
    typedef vec_ref value_type;
    typedef vec_ref reference;
    typedef vec_ref* pointer;
    inline const_iterator() : i(0), val_p(0), ptr_p(0) { }
    inline const_iterator(values_t* v, ptr_t* p, size_type ii)
      : val_p(v), ptr_p(p), i(ii) { }
    inline const vec_ref operator*() const { return vec_ref(i, val_p, ptr_p); }
    inline self& operator++() { ++i; return *this; }
    inline self& operator+=(size_type n) { i += n; return *this; }
    inline self operator++(int) { self t = *this; ++(*this); return t; }
    inline self& operator--() { --i; return *this; }
    inline self& operator-=(size_type n) { i -= n; return *this; }
    inline self operator--(int) { self t = *this; --(*this); return t; }
    inline bool operator!=(const self& x) const { return i != x.i; }
    inline bool operator==(const self& x) const { return i == x.i; }
    inline bool operator<(const self& x) const { return i < x.i; }
    inline size_type index() const { return i; }
  protected:
    size_type i;
    values_t* val_p;
    ptr_t* ptr_p;
  };

  //: The reverse iterator type
  typedef reverse_iter<iterator> reverse_iterator;
  //: The const reverse iterator type
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  //: Default constructor
  inline envelope2D() : dim(0,0) { }
  //: Constructor from dimension pair
  inline envelope2D(dim_type d)
    : dim(d), val(1, T(0)), ptr(d.first(), size_type(0))
  {
    val.reserve(dim.first() * 5);
  }
  //: Constructor from dimension abd bandwidth pairs
  inline envelope2D(dim_type d, dim_type)
    : dim(d), val(1, T(0)), ptr(d.first(), size_type(0))
  {
    val.reserve(dim.first() * 5);
  }
  //: Copy Constructor
  inline envelope2D(const envelope2D& x)
    : dim(x.dim), val(x.val), ptr(x.ptr) { }

  /* Array containing the length of each row/column
     nnz is number of non-zeros (number of stored elements)
     */
  template <class Array> inline
  void initialize_nzstruct(const Array& a, size_type nnz) {
    val.resize(nnz, T(0));
    for (int x = 1; x < dim.first(); ++x)
      ptr[x] = ptr[x-1] + a[x-1];
  }

  //: Return an iterator pointing to the first 1D container
  inline iterator begin() { return iterator(&val, &ptr, 0); }

  //: Return an iterator pointing past the end of the 2D container
  inline iterator end() { return iterator(&val, &ptr, dim.first()); }

  //: Return a const iterator pointing to the first 1D container
  inline const_iterator begin() const {
    return const_iterator((values_t*)&val, (ptr_t*)&ptr, 0);
  }
  //: Return a const iterator pointing past the end of the 2D container
  inline const_iterator end() const{
    return const_iterator((values_t*)&val, (ptr_t*)&ptr, dim.first());
  }

  //: Return a reverse iterator pointing to the last 1D container
  inline reverse_iterator rbegin() {
    return reverse_iterator(end());
  }
  //: Return a reverse iterator pointing past the start of the 2D container
  inline reverse_iterator rend() {
    return reverse_iterator(begin());
  }
  //: Return a const reverse iterator pointing to the last 1D container
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  //: Return a const reverse iterator pointing past the start of the 2D container
  inline const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  //: The dimension of the 2D container
  inline size_type major() const { return dim.first(); }
  //: The dimension of the 1D container
  inline size_type minor() const { return dim.second(); }
  //: The number of non-zeros
  inline size_type nnz() const { return val.size(); }

  //: Return a reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline typename vec_ref::reference
  operator()(size_type i, size_type j) {
    return reference(i, &val, &ptr)[j];
  }
  //: Return a const reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline const typename vec_ref::reference
  operator()(size_type i, size_type j) const {
    return const_reference(i, (values_t*)&val, (ptr_t*)&ptr)[j];
  }

  //: Return a reference to the ith 1D container
  inline reference operator[](size_type i) {
    return reference(i, &val, &ptr);
  }
  //: Return a const reference to the ith 1D container
  inline const_reference operator[](size_type i) const {
    return const_reference(i, (values_t*)&val, (ptr_t*)&ptr);
  }

  inline void print() const {
    print_vector(val);
    print_vector(ptr);
  }
#if 0
  // deprecated
  //: blah
  //!noindex:
  template <class SubMatrix>
  struct partitioned {
    typedef envelope2D<SubMatrix> type;
    typedef gen_envelope2D<SubMatrix> generator;
  };
#endif
protected:
  dim_type dim;
  values_t val;
  ptr_t ptr;
};


//: blah
//!noindex:
template <class T, int M, int N>
struct gen_envelope2D {
  typedef gen_envelope2D<T,M,N> submatrix_type; /* bogus */
  typedef gen_envelope2D<int,N,M> transpose_type; /* bogus type */
  typedef gen_envelope2D<int,M,N> banded_view_type; /* bogus type */
  typedef envelope2D<T> type;
};

} /* namespace mtl */

#endif /* MTL_ENVELOPE2D_H */
