// -*- c++ -*-
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


#ifndef MTL_COMPRESSED1D_H
#define MTL_COMPRESSED1D_H


#include <vector>
#include <algorithm>

#include "mtl_config.h"
#include "entry.h"
#include "compressed_iter.h"
#include "reverse_iter.h"
#include "matrix_traits.h"
#include "refcnt_ptr.h"
#include "scaled1D.h"
#include "light1D.h"
#include "dense1D.h" /* for VC++ workaround -JGS */

namespace mtl {

using std::lower_bound;
using std::find;

//: Compressed Sparse Vector
//
// The compressed1D Vector is a sparse vector implemented with
// a pair of parallel arrays. One array is for the element values,
// and the other array is for the indices of those elements.
// The elements are ordered by their index as they are inserted
// into the compressed1D. compressed1D's can be used to build
// matrices with the array storage format, and they can also
// be used on their own.
//
// <pre>
// [ (1.2, 3), (4.6, 5), (1.0, 10), (3.7, 32) ] A Sparse Vector
//
// [ 1.2, 4.6, 1.0, 3.7 ]  Value Array
// [  3,   5,  10,  32 ]   Index Array
// </pre>
//
// <p>The <tt>compressed1D::iterator</tt> dereferences (<tt>*i</tt>) to
// return the element value. One can access the index of that element
// with the <tt>i.index()</tt> function. One can also access
// the array of indices through the <tt>nz_struct()</tt> method.
//
// <p>One particularly useful fact is that one can perform
// scatters and gathers of sparse elements by using the
// <tt>mtl::copy(x,y)</tt> function with a sparse and a
// dense vector.
//
//!category: containers
//!component: type
//!models: Vector
//!definition: compressed1D.h
//!tparam: T - the element type
//!tparam: SizeType - the type for the stored indices - int
//!tparam: IND_OFFSET - To handle indexing from 0 or 1 - index_from_zero
//!example: gather_scatter.cc, array2D.cc, sparse_copy.cc
template <class T, class SizeType = int, int IND_OFFSET = index_from_zero>
class compressed1D {

  typedef std::vector<T> values_vec;
  typedef std::vector<SizeType> indices_vec;
  typedef indices_vec indices_t;
  typedef values_vec  values_t;

  typedef refcnt_ptr< values_vec > values_ptr;
  typedef refcnt_ptr< indices_vec > indices_ptr;

  typedef compressed1D<T,SizeType,IND_OFFSET> self;

  typedef typename indices_vec::iterator index_iterator;
  typedef typename values_vec::iterator value_iterator;
  typedef typename indices_vec::const_iterator index_const_iterator;
  typedef typename values_vec::const_iterator value_const_iterator;

  friend class elt_ref<self>;
  friend class const_elt_ref<self>;

public:
  /**@name Type Definitions */

  enum { N = 0 };

  //: This is a sparse vector
  typedef sparse_tag sparsity;
  //: This is a 1D container
  typedef oned_tag dimension;
  //: Scaled type of this vector
  typedef scaled1D< self > scaled_type;
  //: Element type
  typedef typename values_t::value_type value_type;
#if defined(_MSVCPP_)
  typedef value_type* pointer;
#else
  //: A pointer to the element type
  typedef typename values_t::pointer pointer;
#endif
  //: Unsigned integral type for dimensions and indices
  typedef SizeType size_type;
  //: Integral type for differences in iterators
  typedef typename values_t::difference_type difference_type;
  //: Reference to the value type
  typedef elt_ref<self> reference;
  //: Const reference to the value type
  typedef const_elt_ref<self> const_reference;
  //: Iterator type
  typedef compressed_iter<0,values_vec, indices_vec,IND_OFFSET> iterator;
  //: Const iterator type
  typedef compressed_iter<1,values_vec, indices_vec,IND_OFFSET> const_iterator;
  //: Reverse iterator type
  typedef reverse_iter<iterator> reverse_iterator;
  //: The const reverse iterator type
  typedef reverse_iter<const_iterator> const_reverse_iterator;
  //: Reference to the index array
  typedef const indices_vec& IndexArrayRef;
  //: The type for the index array
  typedef const indices_t IndexArray;

  //: The type for the subrange vector
  typedef self subrange_type; /* JGS need to think about this */



  /**@name Constructors */

  //: Default Constructor
  inline compressed1D()
    : values(new values_t(0)),
      indices(new indices_t(0)),
      size_(0) { }

  //: Length N Constructor
  inline compressed1D(size_type n)
    : values(new values_t(0)),
      indices(new indices_t(0)),
      size_(n) { }

  //: Copy Constructor
  inline compressed1D(const self& x)
    : values(x.values), indices(x.indices), size_(x.size_) { }

  //: Index Array Constructor
  template <class IndexIter>
  inline compressed1D(IndexIter first, IndexIter last,  size_type n)
    : values(new values_t(n)),
      indices(new indices_t(n)),
      size_(n)
  {
    std::copy(first, last, indices->begin());
  }

  //: Assignment Operator
  inline self& operator=(const self& x) {
    values = x.values;
    indices = x.indices;
    size_ = x.size_;
    return *this;
  }

  /**@name Access Methods */


  /**@name Iterator Access Methods */

  //: Return an iterator pointing to the beginning of the vector
  //!wheredef: Container
  inline iterator begin() {
    return iterator(values->begin(), indices->begin(), 0);
  }
  //: Return an iterator pointing past the end of the vector
  //!wheredef: Container
  inline iterator end() {
    return iterator(values->begin(), indices->begin(), indices->size());
  }
  //: Return a const iterator pointing to the begining of the vector
  //!wheredef: Container
  inline const_iterator begin() const {
    return const_iterator(values->begin(), indices->begin(), 0);
  }
  //: Return a const iterator pointing past the end of the vector
  //!wheredef: Container
  inline const_iterator end() const {
    return const_iterator(values->begin(), indices->begin(), indices->size());
  }
  //: Return a reverse iterator pointing to the last element of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rbegin() {
    return reverse_iterator(end());
  }
  //: Return a reverse iterator pointing past the end of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rend() {
    return reverse_iterator(begin());
  }
  //: Return a const reverse iterator pointing to the last element of the vector
  //!wheredef: Reversible Container
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  //: Return a const reverse iterator pointing past the end of the vector
  //!wheredef: Reversible Container
  inline const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }


  /**@name Element Access Methods */

  //: Access the element with index i
  inline reference operator[](size_type i) MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "compressed1D::operator[]");
    return reference(*this, i);
  }
  //: Access the element with index i
  inline const_reference operator[](size_type i) const MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "compressed1D::operator[]");
    return const_reference(*this, i);
  }

  //for BEAM only
  value_type& get_ref(size_type i) {
    iterator iter = find(i);
    if ( iter != end() ) {
      if ( iter.index() != i )
	iter = insert(iter, i, T(0));
    } else
      iter = insert(iter, i, T(0));

    return *iter;
  }


  /*
   * list operations
   */

  /* OneD required insert method: */
  //: Insert val into the vector at index i
  inline iterator insert(size_type i, const T& val) MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "compressed1D::insert");
    index_iterator index_iter = lower_bound(indices->begin(),
                                            indices->end(),
                                            i - IND_OFFSET);/* F to C */
    index_iter = indices->insert(index_iter, i - IND_OFFSET); /* F to C */
    size_type n = index_iter - indices->begin();
    value_iterator val_iter = values->insert(values->begin() + n, val);
    return iterator(index_iter, val_iter);
  }

  //: Push back, warning: must insert elements ordered by index.
  // This method does not change the size() of the vector.
  inline void push_back(size_type i,  const T& val) MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "compressed1D::push_back");
    values->push_back(val);
    indices->push_back(i);
  }

  //: Erase the vector
  inline void clear() { indices->clear(); values->clear(); }
  //: The size of the vector (including non-zeroes)
  inline size_type size() const { return size_; }
  //: The number of non-zero elements (the number stored)
  inline size_type nnz() const { return values->size(); }
  //: Resize the vector to size n
  inline void resize(size_type n) {
    size_ = n;
  }
  //: Reserve storage for n non-zero elements
  inline void reserve(size_type n) {
    values->reserve(n);
    indices->reserve(n);
  }
  //: Returns the array of indices
  inline IndexArrayRef nz_struct() const {
    return *indices;
  }

  //: Returns the array of indices
  inline IndexArrayRef nz_struct() {
    return *indices;
  }

protected:

  inline iterator find(size_type i) {
    index_iterator iter = lower_bound(indices->begin(), indices->end(),
                                      i - IND_OFFSET); /* F to C */
    size_type n = iter - indices->begin();
    return iterator(values->begin(), indices->begin(), n);
  }

  inline const_iterator find(size_type i) const {
    index_const_iterator iter = lower_bound(indices->begin(), indices->end(),
                                    i - IND_OFFSET); /* F to C */
    size_type n = iter - indices->begin();
    return const_iterator(values->begin(), indices->begin(), n);
  }

  inline iterator insert(iterator iter, size_type i, T v) {
    index_iterator ind = indices->insert(iter.index_iter(),
                                 i - IND_OFFSET); /* F to C */
    values->insert(iter.value_iter(), v);
    size_type n = ind - indices->begin();
    return iterator(values->begin(), indices->begin(), n);
  }


protected:
  values_ptr values;
  indices_ptr indices;
  size_type size_;
};

} /* namespace mtl */

#endif




#if 0

  class elt_inserter {
  public:
    inline elt_inserter(const elt_inserter& x) : val(x.val), ind(x.ind) { }
    inline elt_inserter(T& v, size_type& i) : val(v), ind(i) { }
    void operator=(const twod_elt<T,size_type>& elt) {
      val = elt.val();
      ind = elt.minor();
    }
  protected:
    T& val;
    size_type& ind;
  };

  class insert_iterator {
    typedef values_vec::iterator val_iter_t;
    typedef indices_vec::iterator ind_iter_t;
    typedef insert_iterator self;
  public:
    inline insert_iterator(const val_iter_t& v, const ind_iter_t& i)
      : val_iter(v), ind_iter(i) { }
    inline elt_inserter operator * () {
      return elt_inserter(*val_iter, *ind_iter);
    }
    inline self& operator ++ () { ++val_iter; ++ind_iter; return *this; }
    inline self& operator ++ (int) {
      self t = *this; ++val_iter; ++ind_iter; return t;
    }
  protected:
    val_iter_t val_iter;
    ind_iter_t ind_iter;
  };

  inline insert_iterator inserter() {
    return insert_iterator(values->begin(), indices->begin());
  }
#endif
