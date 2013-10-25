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

#ifndef MTL_SPARSE1D_H
#define MTL_SPARSE1D_H

#include <algorithm>
#include <set>

#include "mtl_config.h"
#include "entry.h"
#include "sparse_iterator.h"
#include "reverse_iter.h"
#include "not_at.h"
#include "utils.h"
#include "matrix_traits.h"
#include "scaled1D.h"
#include "refcnt_ptr.h"

namespace mtl {

using std::lower_bound;
using std::find;


//: class for implementing a view into just the index array of a sparse
//   row/column
//!noindex:
template <class RepType>
class sp1D_index_array {
  typedef sp1D_index_array<RepType> self;
public:
  typedef int value_type;
  typedef int* pointer;
  typedef const int& reference;
  typedef const int& const_reference;

  typedef typename RepType::const_iterator rep_iter;

  class iterator {
    typedef iterator self;
  public:
    typedef const int& reference;
    inline iterator() { }
    inline iterator(rep_iter i) : iter(i) { }
    inline reference operator*() const { return iter.index(); }
    inline reference operator[](int n) const { return iter.index(); }
    inline self& operator++() { ++iter; return *this; }
	inline self operator++(int) { self t = *this; ++iter; return t; }
	inline self& operator--() { --iter; return *this; }
	inline self operator--(int) { self t = *this; --iter; return t; }
	inline bool operator!=(const self& x) { return iter != x.iter; }
    inline bool operator<(const self& x) { return iter < x.iter; }
    inline bool operator==(const self& x) { return iter == x.iter; }
    inline iterator operator+(int n) {
      return iterator(iter + n);
    }
    inline iterator operator-(int n) {
      return iterator(iter - n);
    }
  protected:
	rep_iter iter;
  };

  typedef iterator const_iterator;

  typedef reverse_iter<iterator> reverse_iterator;
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  sp1D_index_array(rep_iter s, rep_iter f) : start(s), finish(f) { }

  sp1D_index_array(const self& x) : start(x.start), finish(x.finish) { }

  inline const_iterator begin() { return start; }
  inline const_iterator end() { return finish; }
  inline const_reverse_iterator rbegin() {
    return const_reverse_iterator(finish); }
  inline const_reverse_iterator rend() {
    return const_reverse_iterator(start); }
  inline reference operator[](int n) {
    return start[n];
  }

  /* JGS
    protected:
  */

  rep_iter start;
  rep_iter finish;
};

#if defined ( _MSVCPP_ )
/* VC++ work around -JGS */
template <class T>
inline typename T::iterator __sparseoned_find(T* oned, int i, std::set<typename T::entry_type>*) {
  return oned->__find_set(i);
}
#endif

template <class T, class R>
inline typename T::iterator __sparseoned_find(T* oned, int i, R*) {
  return oned->__find_normal(i);
}

//: Sparse 1-D Container Adaptor
//
// This is a sparse vector implementation that can use several
// different underlying containers, including <tt>std::vector</tt>,
// <tt>std::list</tt>, and <tt>std::set</tt>.
// <p>
// This adaptor is used in the implementation of the linked_list,
// tree, and sparse_pair OneD storage types (used with the array
// matrix storage type). This adaptor can also be used as a stand-alone
// Vector.
// <p>
// The value_type of the underlying containers must be entry1, which
// is just an index-value pair.
// <p>
// The elements are ordered by their index as they are inserted.
//
//!category: containers, adaptors
//!component: type
//!definition: sparse1D.h
//!tparam: RepType - The Container type used to store the index value pairs.
//!typereqs: The <TT>value_type</TT> of RepType must be of type <TT>entry1</TT>
//!models: ContainerRef?
//!example: gather_scatter.cc
template <class RepType>
class sparse1D {
  typedef sparse1D<RepType> self;
  typedef RepType rep_type;
  typedef rep_type ref_cont;
  typedef typename rep_type::iterator rep_type_iterator;
  typedef typename rep_type::const_iterator const_rep_type_iterator;
public:
  /**@name Type Definitions */

  enum { N = 0 };

  //: This is a sparse vector
  typedef sparse_tag sparsity;
  //: The index-value pair type
  typedef typename RepType::value_type entry_type;

  //: This is a 1D container
  typedef oned_tag dimension;

  //: The scaled type
  typedef scaled1D< sparse1D< RepType > > scaled_type;
  //: The value type
  typedef typename entry_type::value_type value_type;
  //: The type for pointers to the value type
  typedef value_type* pointer;
  //: The unsigned integral type for dimensions and indices
  typedef typename RepType::size_type size_type;
  //: The type for differences between iterators
  typedef typename RepType::difference_type difference_type;

  //: The type for references to the value type
  typedef elt_ref<self> reference;
  //: The type for const references to the value type
  typedef value_type const_reference;
  //: The iterator type
  typedef sparse_iterator<rep_type_iterator,value_type> iterator;
  //: The const iterator type
  typedef const_sparse_iterator<const_rep_type_iterator,value_type>
          const_iterator;
  //: The reverse iterator type
  typedef reverse_iter<iterator> reverse_iterator;
  //: The const reverse iterator type
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  //: The type for the index array
  typedef sp1D_index_array<RepType> IndexArray;

  //: The reference type for the index array
  typedef sp1D_index_array<RepType> IndexArrayRef;

  //: The type for subrange vectors
  typedef self subrange_type;

  friend class elt_ref<self>;
  friend class const_elt_ref<self>;


  inline iterator __find_set(int i) {
    rep_type_iterator pos = rep->lower_bound(entry_type(i));
    return iterator(pos);
  }

  inline iterator __find_normal(int i) {
    rep_type_iterator pos = lower_bound(rep->begin(), rep->end(),
					entry_type(i));
    return iterator(rep->begin(), pos);
  }

protected:

  inline iterator find(int i) {
    return __sparseoned_find(this, i, rep.operator->());
  }

  /* const versions of find */

  inline const_iterator __find(int i, std::set<entry_type>*) const {
    const_rep_type_iterator pos = rep->lower_bound(entry_type(i));
    return const_iterator(pos);
  }

  template <class R>
  inline const_iterator __find(int i, R*) const {
    const_rep_type_iterator pos = lower_bound(rep->begin(), rep->end(),
					entry_type(i));
    return const_iterator(rep->begin(), pos);
  }

  inline const_iterator find(int i) const {
    return __find(i, rep.operator->());
  }

  inline iterator insert(iterator iter, int i, value_type v) {
    rep_type_iterator pos = rep->insert(iter.base(), entry_type(i, v));
    return iterator(rep->begin(), pos);
  }


public:

  /**@name Constructors */

  //: Default Constructor
  inline sparse1D()
    : rep(new ref_cont()), size_(0) { }

  //: Length N Constructor
  inline sparse1D(size_type n)
    : rep(new ref_cont()), size_(n) { }

  //: Copy Constructor
  inline sparse1D(const self& x)
    : rep(x.rep), size_(x.size_) { }

  //: Construct from index array
  template <class IndexArray>
  inline sparse1D(const IndexArray& x, size_type n)
    : rep(new ref_cont()), size_(n) {
      typename IndexArray::const_iterator i;
      for (i = x.begin(); i != x.end(); ++i)
	rep->push_back(entry_type(*i, value_type(0)));
  }

  //: Assignment Operator
  inline self& operator=(const self& x) {
    rep = x.rep; size_ = x.size_; return *this;
  }

  /**@name Access Methods */


  /**@name  Iterator Access Methods */

  //: Return an iterator pointing to the beginning of the vector
  //!wheredef: Container
  inline iterator begin() {
    return iterator(rep->begin());
  }
  //: Return an iterator pointing past the end of the vector
  //!wheredef: Container
  inline iterator end() {
    return iterator(rep->begin(), rep->end());
  }
  //: Return a const iterator pointing to the begining of the vector
  //!wheredef: Container
  inline const_iterator begin() const {
    return const_iterator(rep->begin());
  }
  //: Return a const iterator pointing past the end of the vector
  //!wheredef: Container
  inline const_iterator end() const {
    return const_iterator(rep->begin(), rep->end());
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

  //: Element Access, return element with index i
  inline const_reference operator[](int i) const MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "sparse1D:operator[]");
    const_iterator iter = find(i);
    if (iter != end() && iter.index() == i)
      return *iter;
    else
      return value_type(0);
  }

  //: Element Access, return element with index i
  inline reference operator[](int i) MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "sparse1D::operator[]");
    return reference(*this, i);
  }


  //: Insert the value at index i of the vector
  inline iterator insert(int i, const value_type& value) MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "sparse1D::insert");
    rep_type_iterator pos = lower_bound(rep->begin(), rep->end(),
					 entry_type(i));
    return rep->insert(pos, entry_type(i, value));
  }

  inline void clear() { rep->clear(); }

  inline void push_back(int i, const value_type& value) {
    MTL_ASSERT(i < size(), "sparse1D::push_back");
    rep->insert(rep->end(), entry_type(i, value));
  }

  /**@name Size Methods */

  //: Returns length of the vector (including non-zeroes)
  inline int size() const {
    return size_;
  }
  //: Number of non-zero (stored) elements
  inline int nnz() const {
    return rep->size();
  }

  inline void resize_imp(int n, std::set<entry_type>*) { }

  template <class R>
  inline void resize_imp(int n, R*) {
    rep->resize(n);

  }

  //: Resizes the vector to size n
  inline void resize(int n) {
    resize_imp(n, rep.operator->());
  }


  rep_type& get_rep() { return *rep; }


  inline void print() const {
    print_vector(*rep);
  }
  //: Return an array of indices describing the non-zero structure
  inline IndexArrayRef nz_struct() const {
    return IndexArrayRef(rep->begin(), rep->end());
  }

protected:

  refcnt_ptr<rep_type> rep;
  size_type size_;
};

} /* namespace mtl */

#endif









  /* obsolete, not used anymore
  inline void set(int i, value_type val) {
    rep_type_iterator pos = lower_bound(rep->begin(), rep->end(),
					entry_type(i));
    if (pos != rep->end())
      if ((*pos).index != i)
	rep->insert(pos, entry_type(i, val));
      else
	(*pos).value = val;
    else
      rep->insert(pos, entry_type(i, val));
  }

  inline value_type get(int i) {
    value_type ret;
    const_rep_type_iterator pos = std::find(rep->begin(), rep->end(),
					    entry_type(i));
    if (pos != rep->end())
      ret = (*pos).value;
    else
      ret = 0.0;
    return ret;
  }
  */
