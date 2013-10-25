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
//===========================================================================

#ifndef MTL_COMPRESSED_CONTIG2D_H
#define MTL_COMPRESSED_CONTIG2D_H

#include "mtl_iterator.h"
#include <algorithm>

#include "entry.h"
#include "compressed_iter.h"
#include "reverse_iter.h"
#include "linalg_vec.h"
#include "matrix_traits.h"
#include "dense2D.h"
#include "dimension.h"
#include "dense1D.h"

#include "utils.h" /* debug */

extern "C" void foobar(int);

namespace mtl {

using std::less;
using std::lower_bound;

template <class T, class SizeType, int IndexOff, int M, int N>
struct gen_compressed2D;
template <class T, class SizeType, int IndexOff, int M, int N>
struct gen_ext_comp2D;

//: Compressed 2-D Container
//
//   The compressed2D container uses the compressed row/column storage
//   format. There is an array of matrix elements, an array of indices,
//   and an array of pointers to the row or column vector starts.
//   A compressed2D matrix can be created from scratch
//   with the constructor:
//   <codeblock>
//   compressed2D(size_type m, size_type n)
//   </codeblock>
//   One can also use preexsisting arrays with the constructor:
//   <codeblock>
//   compressed2D(size_type m, size_type n, size_type nnz,
//                T* val, size_type* ptr, size_type* ind)
//   </codeblock>
//
//   The stored indices (in the ptr and int arrays) are indexed
//   from 1 ala LAPACK and BLAS conventions (Fortran style).
//
//!category: containers
//!component: type
//!definition: compressed2D.h
//!models: TwoDContainerRef
//!example: sparse_matrix.h
//!tparam: ValsType - The container type to use for the values array
//!tparam: ValPtr - A pointer to the ValsType
//!tparam: IndType - The container type to use for the indices and pointer arrays
//!tparam: IndPtr - A pointer to IndType
//!tparam: IND_OFFSET - To handle indexing from 0 or 1
template <class ValsType, class ValPtr, class IndType, class IndPtr, int IND_OFFSET>
class generic_comp2D {
public:
  typedef generic_comp2D self;
  typedef ValsType values_t;
  typedef typename ValsType::value_type TT;
  typedef typename values_t::iterator        value_iterator;
  typedef typename values_t::const_iterator  const_value_iterator;
  typedef IndType indices_t;
  typedef typename indices_t::iterator       index_iterator;
  typedef typename indices_t::const_iterator const_index_iterator;
  typedef IndType starts_t;
  typedef typename starts_t::iterator        starts_iterator;
  typedef typename starts_t::const_iterator  const_starts_iterator;

  typedef typename IndType::value_type size_type;

  typedef dimension<size_type> dim_type;

  /* Type Definitions */
  typedef internal_tag storage_loc;
  enum { M = 0, N = 0 };

  //: This vector reference is created on-the-fly as needed.
  class vec_ref {
  public:
    enum { N = 0 };
    typedef TT value_type;
    typedef TT* pointer;
    typedef typename IndType::value_type size_type;
    typedef typename IndType::difference_type difference_type;
    typedef elt_ref<vec_ref> reference;
    typedef const_elt_ref<vec_ref> const_reference;
    typedef sparse_tag sparsity;
    typedef oned_tag dimension;
    typedef compressed_iter<0,values_t, indices_t, IND_OFFSET> iterator;
    typedef compressed_iter<1,values_t, indices_t, IND_OFFSET> const_iterator;

    typedef reverse_iter< iterator > reverse_iterator;
    typedef reverse_iter< const_iterator > const_reverse_iterator;

    typedef light1D<size_type,0,IND_OFFSET> IndexArrayRef;
    typedef light1D<size_type,0,IND_OFFSET> IndexArray;
    typedef vec_ref subrange_type;

    inline vec_ref(ValPtr val, IndPtr ind, IndPtr s, size_type major_)
      : values(val), indices(ind), starts(s), major(major_) {
    }
    inline vec_ref(const vec_ref& x)
      : values(x.values), indices(x.indices), starts(x.starts),
        major(x.major) { }

    inline iterator begin() {
      return iterator(values->begin(), indices->begin(),
                      (*starts)[major] + IND_OFFSET); /* F to C */
    }
    inline iterator end() {
      return iterator(values->begin(), indices->begin(),
                      (*starts)[major+1] + IND_OFFSET); /* F to C */
    }
    inline const_iterator begin() const {
      return const_iterator(((const values_t*)values)->begin(),
                            ((const indices_t*)indices)->begin(),
                            (*starts)[major] + IND_OFFSET); /* F to C */
    }
    inline const_iterator end() const {
      return const_iterator(((const values_t*)values)->begin(),
                            ((const indices_t*)indices)->begin(),
                            (*starts)[major+1] + IND_OFFSET); /* F to C */
    }

    inline reverse_iterator rbegin() {
      return reverse_iterator(end());
    }
    inline reverse_iterator rend() {
      return reverse_iterator(begin());
    }
    inline const_reverse_iterator rbegin() const {
      return const_reverse_iterator(end());
    }
    inline const_reverse_iterator rend() const {
      return const_reverse_iterator(begin());
    }

    inline reference operator[](size_type i) {
      return reference(*this, i);
    }

    inline const_reference operator[](size_type i) const {
      return const_reference(*this, i);
    }

    //add this for BEAM
    value_type& get_ref(size_type i) {
      iterator iter = find(i);
      /*
      if ( iter != end() ) {
	if ( iter.index() != i )
	  iter = insert(iter, i, value_type(0));
      } else
	iter = insert(iter, i, value_type(0));
      */
	  assert(iter.index() == i);
      return *iter;
    }

    /* JGS this is wrong */
    inline size_type size() const { return nnz(); }

    inline size_type nnz() const {
      return (*starts)[major+1] - (*starts)[major];
    }
    inline IndexArrayRef nz_struct() const {
      size_type first = (*starts)[major] + IND_OFFSET; /* F to C */
      size_type last = (*starts)[major + 1] + IND_OFFSET; /* F to C */
      return IndexArrayRef(&(*indices)[0] + first, last - first);
    }

    inline iterator find(size_type i) {
      difference_type first = (*starts)[major] + IND_OFFSET; /* F to C */
      difference_type last = (*starts)[major + 1] + IND_OFFSET; /* F to C */
      index_iterator indices_begin = indices->begin();
      index_iterator iter = lower_bound(indices_begin + first,
                                        indices_begin + last,
                                        size_type(i - IND_OFFSET)); /* F to C */
      size_type n = iter - indices_begin;
      return iterator(values->begin(), indices_begin, n);
    }

    inline const_iterator find(size_type i) const {
      difference_type first = (*starts)[major] + IND_OFFSET; /* F to C */
      difference_type last = (*starts)[major + 1] + IND_OFFSET; /* F to C */
      const_index_iterator indices_begin =
        ((const indices_t*)indices)->begin();
      const_index_iterator iter = lower_bound(indices_begin + first,
                                              indices_begin + last,
                                              i - IND_OFFSET); /* F to C */
      size_type n = iter - indices_begin;
      return const_iterator(((const values_t*)values)->begin(),
                            indices_begin, n);
    }

    inline iterator insert(iterator iter, size_type i, TT v) {
      index_iterator ind = indices->insert(iter.index_iter(),
                                            size_type(i - IND_OFFSET)); /* F to C */
      values->insert(iter.value_iter(), v);
      size_type n = ind - indices->begin();
      increment_starts(major + 1);
      return iterator(values->begin(), indices->begin(), n);
    }

    inline void increment_starts(size_type i) {
      while (i < size_type(starts->size()))
        ++(*starts)[i++];
    }

    inline void resize(size_type) { }

  private:

    ValPtr values;
    IndPtr indices;
    IndPtr starts;
    size_type major;
  };

public:

  //: The 1D container type
  typedef vec_ref value_type;
  //: A reference to the value type
  typedef vec_ref reference;
  //: A const reference to the value type
  typedef const vec_ref const_reference;

  /* old typedefs */
  typedef vec_ref MajorVector;
  typedef vec_ref MajorVectorRef;
  typedef const vec_ref ConstMajorVectorRef;
public:
  //: Specify that this matrix is sparse
  typedef sparse_tag sparsity;

  //: Specify that this matrix is not strideable (can not use rows(A), columns(A))
  typedef not_strideable strideability;

  /* bogus types, need to change */
  typedef generic_comp2D<ValsType,ValPtr,IndType,IndPtr,IND_OFFSET> transpose_type;
  typedef generic_comp2D<ValsType,ValPtr,IndType,IndPtr,IND_OFFSET> submatrix_type;
  typedef generic_comp2D<ValsType,ValPtr,IndType,IndPtr,IND_OFFSET> banded_view_type;

#if !defined(__MWERKS__) // internal compiler errors

  //: The type for the iterators
  template <int isConst>
  class _iterator {
    typedef _iterator self;
  public:
    typedef typename IF<isConst, const_value_iterator, value_iterator>::RET
                 value_iter;

    typedef typename std::iterator_traits<value_iter>::iterator_category
                 iterator_category;
#if defined(_MSVCPP_)
    typedef typename std::iterator_traits<value_iter>::distance_type
                 difference_type;
    typedef difference_type distance_type;
#else
    typedef typename std::iterator_traits<value_iter>::difference_type
                 difference_type;
#endif

    typedef MajorVectorRef value_type;

    typedef typename IF<isConst, const MajorVectorRef, MajorVectorRef>::RET reference;
    typedef typename IF<isConst, const MajorVectorRef*, MajorVectorRef*>::RET pointer;

    typedef typename IF<isConst, const ValPtr, ValPtr>::RET myValPtr;
    typedef typename IF<isConst, const IndPtr, IndPtr>::RET myIndPtr;

    inline _iterator() : pos(0) { }
    inline _iterator(myValPtr val, myIndPtr ind, myIndPtr s, size_type p)
      : values(val), indices(ind), starts(s), pos(p) { }
    inline _iterator(const self& x)
      : values(x.values), indices(x.indices), starts(x.starts), pos(x.pos) { }

    inline size_type index() const { return pos; }

    inline reference operator*() const {
      return reference(values, indices, starts, pos);
    }
    inline self& operator++() { ++pos; return *this; }
    inline self& operator--() { --pos; return *this; }
    inline self& operator+=(size_type n) { pos += n; return *this; }
    inline self& operator-=(size_type n) { pos -= n; return *this; }
    inline difference_type operator-(const self& x) const {
      return pos - x.pos; }
    inline bool operator<(const self& x) const { return pos < x.pos; }
    inline bool operator!=(const self& x) const { return pos != x.pos; }
    inline bool operator==(const self& x) const { return pos == x.pos; }
  private:
    ValPtr values;
    IndPtr indices;
    IndPtr starts;
    size_type pos;
  };

  typedef _iterator<0> iterator;
  typedef _iterator<1> const_iterator;

#else

  //: The type for the iterators
  class iterator {
    typedef iterator self;
  public:
    typedef value_iterator value_iter;

    typedef typename std::iterator_traits<value_iter>::iterator_category
                 iterator_category;
#if defined(_MSVCPP_)
    typedef typename std::iterator_traits<value_iter>::distance_type
                 difference_type;
        typedef difference_type distance_type;
#else
    typedef typename std::iterator_traits<value_iter>::difference_type
                 difference_type;
#endif

    typedef MajorVectorRef value_type;

    typedef MajorVectorRef reference;
    typedef MajorVectorRef pointer;

    typedef ValPtr myValPtr;
    typedef IndPtr myIndPtr;

    inline iterator() : pos(0) { }
    inline iterator(myValPtr val, myIndPtr ind, myIndPtr s, size_type p)
      : values(val), indices(ind), starts(s), pos(p) { }
    inline iterator(const self& x)
      : values(x.values), indices(x.indices), starts(x.starts), pos(x.pos) { }

    inline size_type index() const { return pos; }

    inline reference operator*() const {
      return reference(values, indices, starts, pos);
    }
    inline self& operator++() { ++pos; return *this; }
    inline self& operator--() { --pos; return *this; }
    inline self& operator+=(size_type n) { pos += n; return *this; }
    inline self& operator-=(size_type n) { pos -= n; return *this; }
    inline difference_type operator-(const self& x) const {
      return pos - x.pos; }
    inline bool operator<(const self& x) const { return pos < x.pos; }
    inline bool operator!=(const self& x) const { return pos != x.pos; }
    inline bool operator==(const self& x) const { return pos == x.pos; }
  private:
    ValPtr values;
    IndPtr indices;
    IndPtr starts;
    size_type pos;
  };

  //: The type for the iterators
  class const_iterator {
    typedef const_iterator self;
  public:
    typedef const_value_iterator value_iter;
    typedef typename std::iterator_traits<value_iter>::iterator_category
                 iterator_category;
#if defined(_MSVCPP_)
    typedef typename std::iterator_traits<value_iter>::distance_type
                 difference_type;
        typedef difference_type distance_type;
#else
    typedef typename std::iterator_traits<value_iter>::difference_type
                 difference_type;
#endif

    typedef MajorVectorRef value_type;

    typedef const MajorVectorRef reference;
    typedef const MajorVectorRef* pointer;

    typedef const ValPtr myValPtr;
    typedef const IndPtr myIndPtr;

    inline const_iterator() : pos(-1) { }
    inline const_iterator(myValPtr val, myIndPtr ind, myIndPtr s, size_type p)
      : values(val), indices(ind), starts(s), pos(p) { }
    inline const_iterator(const self& x)
      : values(x.values), indices(x.indices), starts(x.starts), pos(x.pos) { }

    inline size_type index() const { return pos; }

    inline reference operator*() const {
      return reference(values, indices, starts, pos);
    }
    inline self& operator++() { ++pos; return *this; }
    inline self& operator--() { --pos; return *this; }
    inline self& operator+=(size_type n) { pos += n; return *this; }
    inline self& operator-=(size_type n) { pos -= n; return *this; }
    inline difference_type operator-(const self& x) const {
      return pos - x.pos; }
    inline bool operator<(const self& x) const { return pos < x.pos; }
    inline bool operator!=(const self& x) const { return pos != x.pos; }
    inline bool operator==(const self& x) const { return pos == x.pos; }
  private:
    ValPtr values;
    IndPtr indices;
    IndPtr starts;
    size_type pos;
  };
#endif

  //: The type for the reverse iterators
  typedef reverse_iter< iterator > reverse_iterator;
  //: The type for the const reverse iterators
  typedef reverse_iter< const_iterator > const_reverse_iterator;

  /* Constructors */

  //: Default Constructor
  inline generic_comp2D() : dim(0,0), values(0), indices(0), starts(0) { }

  //: External Storage Constructor
  inline generic_comp2D(dim_type d, ValPtr v, IndPtr ind, IndPtr s)
    : dim(d), values(v), indices(ind), starts(s) { }

  //: Copy Constructor
  inline generic_comp2D(const self& x)
    : dim(x.dim), values(x.values), starts(x.starts), indices(x.indices) { }

  inline self& operator=(const self& x) {
    dim = x.dim; values = x.values; starts = x.starts; indices = x.indices;
    return *this;
  }

  /* Iterator Access Methods */


  //: Return an iterator pointing to the first 1D container
  inline iterator begin() {
    return iterator(values, indices, starts, 0);
  }
  //: Return an iterator pointing past the end of the 2D container
  inline iterator end() {
    return iterator(values, indices, starts, dim.first());
  }

  //: Return a const iterator pointing to the first 1D container
  inline const_iterator begin() const {
    return const_iterator(values, indices, starts, 0);
  }
  //: Return a const iterator pointing past the end of the 2D container
  const_iterator end() const {
    return const_iterator(values, indices, starts, dim.first());
  }

  /* reverse iterators */

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

  /* Element Access Methods */

  //: Return a reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline typename reference::reference
  operator()(size_type i, size_type j) {
    return MajorVectorRef(values, indices, starts, i)[j];
  }
  //: Return a const reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline typename const_reference:: const_reference
  operator()(size_type i, size_type j) const {
    return ConstMajorVectorRef(values, indices, starts, i)[j];
  }


  /* Size Methods */

  //: The dimension of the 2D container
  inline size_type major() const { return dim.first(); }
  //: The dimension of the 1D containers
  inline size_type minor() const { return dim.second(); }
  //: The number of non-zeros
  inline size_type nnz() const { return values->size(); }

  /* Vector Access Methods */

  //: Return the ith 1D container
  inline value_type operator[](size_type i) const {
    return MajorVector(values, indices, starts, i);
  }

  /* return the raw data */

  //: Return a pointer to the values array
  inline TT* get_val() { return values->data(); }
  //:
  inline const TT* get_val() const { return values->data(); }

  //: Return a pointer to the indices array
  inline size_type* get_ind() { return indices->data(); }
  //:
  inline const size_type* get_ind() const { return indices->data(); }

  //: Return a pointer to the pointers array
  inline size_type* get_ptr() { return starts->data(); }
  //:
  inline const size_type* get_ptr() const { return starts->data(); }

  //: A fast specialization for copying from a sparse matrix
  template <class SparseMat>
  void fast_copy(const SparseMat& x, sparse_tag) {
    size_type nnz = x.nnz();
    typename SparseMat::const_iterator i;
    typename SparseMat::OneD::const_iterator j, jend;

    values->resize(nnz);
    indices->resize(nnz);
    starts->resize(x.major() + 1);

    size_type curr = 0;

    for (i = x.begin(); i != x.end(); ++i) {
      (*starts)[i.index()] = curr - IND_OFFSET; /* F to C */
      jend = (*i).end();
      for (j = (*i).begin(); j != jend; ++j) {
        (*values)[curr] = *j;
        (*indices)[curr++] = j.index() - IND_OFFSET; /* F to C */
      }
    }
    (*starts)[i.index()] = curr - IND_OFFSET; /* F to C */
  }
  //: A fast specialization for copying from a dense matrix
  template <class DenseMat>
  void fast_copy(const DenseMat x, dense_tag) {
    typename DenseMat::const_iterator i;
    typename DenseMat::OneD::const_iterator j;

    size_type nnz = 0;

    /* count non-zeros in each row */
    for (i = x.begin(); i != x.end(); ++i)
      for (j = (*i).begin(); j != (*i).end(); ++j)
        if (*j != TT(0))
          ++nnz;

    values->resize(nnz);
    indices->resize(nnz);
    starts->resize(x.major() + 1);

    size_type curr = 0;

    for (i = x.begin(); i != x.end(); ++i) {
      (*starts)[i.index()] = curr - IND_OFFSET; /* F to C */
      for (j = (*i).begin(); j != (*i).end(); ++j) {
        if (*j != TT(0)) {
          (*values)[curr] = *j;
          (*indices)[curr++] = j.index() - IND_OFFSET; /* F to C */
        }
      }
    }
    (*starts)[i.index()] = curr - IND_OFFSET; /* F to C */
  }
  template <class TwoD__>
  void fast_copy(const TwoD__& x) {
    typedef typename TwoD__::sparsity Sparsity;
    fast_copy(x, Sparsity());
  }

  void print() const {
    std::cout << "values ";
    print_vector(*values);
    std::cout << "indices ";
    print_vector(*indices);
    std::cout << "starts ";
    print_vector(*starts);
  }

protected:
  dim_type dim;
  ValPtr values;
  IndPtr indices;
  IndPtr starts;  /* starting point for each minor vector */
};

//: Sparse matrix storage format with internal storage
//
// This version of the compressed matrix format keeps track
// of its own memory using reference counting.
//
//!component: type
//!category: container
//!definition: compressed2D.h
//!models: TwoDContainerRef
//!tparam: T - The element type
//!tparam: SizeType - The type for the stored indices
//!tparam: IND_OFFSET - To handle indexing from 0 or 1
template <class T, class SizeType, int IND_OFFSET>
class compressed2D : public generic_comp2D< dense1D<T>, dense1D<T>*,
                     dense1D<SizeType>,
                     dense1D<SizeType>* , IND_OFFSET>
{
  //: The base class
  typedef generic_comp2D< dense1D<T>, dense1D<T>*,
                     dense1D<SizeType>,
                     dense1D<SizeType>* , IND_OFFSET> Base;
  //: The type of this class
  typedef compressed2D<T, SizeType,IND_OFFSET> self;
public:
  typedef typename Base::dim_type dim_type;

  //: The integral type for dimensions, indices, etc.
  typedef SizeType size_type;
  //: A variant of a pair type to describe the band width
  typedef dimension<int> band_type;

  //: Default Constructor
  inline compressed2D() : Base(dim_type(0,0), &vals, &inds, &ptrs) { }

  //: Normal Constructor
  inline compressed2D(dim_type d)
    : Base(d, &vals, &inds, &ptrs),
      ptrs(d.first() + 1, -IND_OFFSET) /* F to C */
  {
    vals.reserve(Base::dim.first() * 5);
    inds.reserve(Base::dim.first() * 5);
  }

  inline compressed2D(dim_type d, size_type nnz)
    : Base(d, &vals, &inds, &ptrs),
      ptrs(d.first() + 1, -IND_OFFSET) /* F to C */
  {
    vals.reserve(nnz);
    inds.reserve(nnz);
  }

  //: Banded Constructor
  inline compressed2D(dim_type d, band_type)
    : Base(d, &vals, &inds, &ptrs),
      ptrs(d.first() + 1, -IND_OFFSET) /* F to C */
  {
    vals.reserve(Base::dim.first() * 5);
    inds.reserve(Base::dim.first() * 5);
  }

  //: Copy Constructor
  inline compressed2D(const self& x)
    : Base(x.dim, &vals, &inds, &ptrs),
      vals(x.vals), ptrs(x.ptrs), inds(x.inds)
  { }

  // for banded view
  inline compressed2D(const self& x, band_type, banded_tag)
    : Base(x.dim, &vals, &inds, &ptrs),
      vals(x.vals), ptrs(x.ptrs), inds(x.inds)
  { }

  inline self& operator=(const self& x) {
    vals = x.vals; ptrs = x.ptrs; inds = x.inds;
    /* fill out inhereted part */
    Base::dim = x.dim;
    Base::values = &vals; Base::indices = &inds; Base::starts = &ptrs;
    return *this;
  }

  //: Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline compressed2D(MatrixStream& s, Orien) /* F to C */
    : Base(Orien::map(dim_type(s.nrows(),s.ncols())), &vals, &inds, &ptrs),
      ptrs(1 + Orien::map(dim_type(s.nrows(),s.ncols())).first(), -IND_OFFSET)
  {
    vals.reserve(s.nnz());
    inds.reserve(s.nnz());
  }

  //: Banded Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline compressed2D(MatrixStream& s, Orien, band_type) /* F to C */
    : Base(Orien::map(dim_type(s.nrows(),s.ncols())), &vals, &inds, &ptrs),
      ptrs(1 + Orien::map(dim_type(s.nrows(),s.ncols())).first(), -IND_OFFSET)
  {
    vals.reserve(s.nnz());
    inds.reserve(s.nnz());
  }
#if 0
  // deprecated
  template <class SubMatrix>
  struct partitioned {
    typedef compressed2D<SubMatrix, size_type,IND_OFFSET> type;
    typedef gen_compressed2D<SubMatrix, size_type,IND_OFFSET> generator;
  };
#endif
  inline size_type capacity() const { return inds.capacity(); }

protected:
  dense1D<T> vals;
  dense1D<size_type> ptrs;
  dense1D<size_type> inds;
};


//: Sparse matrix storage format with external storage
//
// This version of the compressed matrix format uses memory
// provided by the user for the matrix storage.
//
//!component: type
//!category: container
//!definition: compressed2D.h
//!models: TwoDContainerRef
//!tparam: T - The element type
//!tparam: SizeType - The type for the stored indices
//!tparam: IND_OFFSET - To handle indexing from 0 or 1
template <class T, class SizeType, int IND_OFFSET>
class ext_comp2D : public generic_comp2D< external_vec<T,0,SizeType>,
                                          external_vec<T,0,SizeType>*,
                                          external_vec<SizeType,0,SizeType>,
                                          external_vec<SizeType,0,SizeType>*,
                             IND_OFFSET >
{
  //: The type of this class
  typedef ext_comp2D<T, SizeType,IND_OFFSET> self;
  //: The base class
  typedef generic_comp2D< external_vec<T,0,SizeType>,
                          external_vec<T,0,SizeType>*,
                          external_vec<SizeType,0,SizeType>,
                          external_vec<SizeType,0,SizeType>*,
                          IND_OFFSET > Base;
public:
  typedef typename Base::dim_type dim_type;

  external_vec<T,0,SizeType> vals;
  typedef SizeType size_type;
  external_vec<size_type,0,SizeType> inds;
  external_vec<size_type,0,SizeType> ptrs;

  typedef typename external_vec<size_type,0,SizeType>::size_type rep_size_t;

  inline ext_comp2D() { }
  //: Preallocated Memory Constructor
  inline ext_comp2D(dim_type d, size_type nz,
                    T* val, size_type* ptr, size_type* ind)
    : Base(d, &vals, &inds, &ptrs),
      vals(val, nz),
      inds(ind, nz),
      ptrs(ptr, rep_size_t(d.first() + 1))
  { }

  //: Copy Constructor
  inline ext_comp2D(const self& x)
    : Base(dim_type(x.major(), x.minor()), &vals, &inds, &ptrs),
      vals(x.vals),
      inds(x.inds),
      ptrs(x.ptrs)
  { }

  inline self& operator=(const self& x) {
    vals = x.vals; inds = x.inds; ptrs = x.ptrs;
    /* fill out inhereted part */
    Base::dim = x.dim;
    Base::values = &vals; Base::indices = &inds; Base::starts = &ptrs;
    return *this;
  }

  //: Banded View (including Symm and Tri) Constructor
  typedef dimension<int> band_type;
  inline ext_comp2D(const self& x, band_type, banded_tag)
    : Base(x.dim, &vals, &inds, &ptrs),
      vals(x.vals),
      inds(x.inds),
      ptrs(x.ptrs)
  { }
#if 0
  // deprecated
  template <class SubMatrix>
  struct partitioned {
    typedef ext_comp2D<SubMatrix, size_type,IND_OFFSET> type;
    typedef gen_ext_comp2D<SubMatrix, size_type,IND_OFFSET> generator;
  };
#endif
};

//: blah
//!noindex:
template <class T, class SizeType, int INDEX, int M, int N>
struct gen_compressed2D {
  typedef gen_compressed2D<T,SizeType,INDEX,M,N> submatrix_type; /* bogus */
  typedef gen_dense2D<T,gen_rect_offset<M,N>,N,M> transpose_type; /* bogus */
  typedef gen_dense2D<T,gen_rect_offset<M,N>,M,N> banded_view_type; /* bogus */

  typedef compressed2D<T, SizeType,INDEX> type;
};



//: blah
//!noindex:
template <class T, class SizeType, int INDEX, int M, int N>
struct gen_ext_comp2D {
  typedef gen_ext_comp2D<T, SizeType,INDEX,M,N> submatrix_type; /* bogus */
  typedef gen_dense2D<T,gen_rect_offset<N,M>,N,M> transpose_type; /* bogus */
  typedef gen_dense2D<T,gen_rect_offset<M,N>,M,N> banded_view_type; /* bogus */
  typedef ext_comp2D<T, SizeType,INDEX> type;
};




} /* namespace mtl */

#endif




#if 0
  template <class Sparse2D>
  inline compressed2D(const nz_structure<Sparse2D>& x)
    : major_(x.major()), minor_(x.minor()),
      starts(1 + x.major())
  {
    int nnz = x.nnz();
    values->resize(nnz);
    indices->resize(nnz);

    int curr = 0;
    int i;
    for (i = 0; i < major_; ++i) {
      starts[i] = curr;

      typename Sparse2D::MajorVector::const_iterator j = x[i].begin();
      typename Sparse2D::MajorVector::const_iterator jend = x[i].end();
      while (not_at(j, jend)) {
        indices[curr++] = j.index();
        ++j;
      }
    }
    starts[i] = curr;
  }
#endif

#if 0
    inline TT get(int i) const {
      int first = (*starts)[major];
      int last = (*starts)[major + 1];
      const_index_iterator indices_begin =
        ((const indices_t*)indices)->begin();
      const_index_iterator evi = find(indices_begin + first,
                                      indices_begin + last, i);
      return evi == (indices_begin + last) ?
        0.0 : (*values)[evi - indices_begin];
    }

    inline void set(int i, TT v) {
      int first = (*starts)[major];
      int last = (*starts)[major + 1];

      if (first == last) {
        /* this row had no entries */
        (*starts)[i] = indices->insert(indices->begin() + first, i)
          - indices->begin();
        values->insert(values->begin() + first, v);
        increment_starts(major + 1);
      }
      else {
        index_iterator evi = lower_bound(indices->begin() + first,
                                         indices->beign() + last, i);
        if (evi == (indices->begin() + last)) {
          evi = indices->insert(evi, i);
          int n = evi - indices->begin();
          values->insert(values->begin() + n, v);
          increment_starts(major + 1);
        }
        else if (*evi == j) {
          int n = evi - indices->begin();
          (*values)[n] = v;
        }
        else {
          evi = indices->insert(evi, i);
          int n = evi - indices->begin();
          values->insert(values->begin() + n, v);
          increment_starts(major + 1);
        }
      }
    }
#endif
