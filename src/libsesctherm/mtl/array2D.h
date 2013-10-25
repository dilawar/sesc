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

#ifndef MTL_ARRAY2D_H
#define MTL_ARRAY2D_H

#include "mtl_config.h"
#include "refcnt_ptr.h"
#include "dense_iterator.h"
#include "reverse_iter.h"
#include "scaled1D.h"
#include "initialize.h"
#include "matrix_traits.h"
#include "utils.h"
//#include "compressed2D.h" // only for bogus :(
#include "dimension.h"

namespace mtl {

template <class T>
struct gen_array2D;

//: Array 2-D Container
//!category: containers
//!component: type
//!definition: array2D.h
//!tparam: OneD - the one dimensional container the array is composed of
//!models: TwoDStorage
//
// This is the model of TwoDStorage that implements the <tt>array</tt>
// storage format. <TT>array2D</TT> is actually implemented with a
// Container of 1-D Containers (whereas the other TwoDStorage types
// just act like Containers of Containers). In this way the
// <tt>array2D</tt> storage type is the most flexible, since many
// different types of 1-D containers can be used in conjunction with
// this class. The 1-D containers include <tt>linked_list</tt>
// (sparse1D<std::list>), <tt>tree</tt> (sparse1D<std::set>),
// <tt>dense</tt> (std::vector), <tt>sparse_pair</tt>
// (sparse1D<std::vector>), and <tt>compressed</tt>
// (mtl::compressed1D).
//
// One special feature of the <tt>array2D</tt> is that one can
// swap and assign the Vectors inside the array in constant time,
// as mentioned in the array interface description.
//
// The backbone container of the array2D is not implemented with
// std::vector due to some subtle interactions between copy/default
// constructors and handle objects.
//
//!example: array2D.cc
template <class OneD_>
class array2D {
  typedef array2D<OneD_> self;
  typedef std::vector<OneD_> rep_type;
  typedef refcnt_ptr< rep_type > rep_ptr;
  typedef typename OneD_::reference OneD_reference;
  typedef typename OneD_::const_reference OneD_const_reference;
  typedef typename OneD_::value_type T;
public:
  /**@name Type Definitions */


  /* this is bogus, need a way to put the new
     element type "SubMatrix" into OneD
  */
  template <class SubMatrix>
  struct partitioned {
    typedef array2D<OneD_> type;
    typedef gen_array2D<OneD_> generator;
  };

#if 0
  // JGS, bad to have this dependence
  typedef compressed2D<int, int,0> transpose_type; /* bogus */
  typedef compressed2D<int, int,0> submatrix_type; /* bogus */
  typedef compressed2D<int, int,0> banded_view_type; /* bogus */
#else
  typedef array2D<OneD_> transpose_type; /* bogus */
  typedef array2D<OneD_> submatrix_type; /* bogus */
  typedef array2D<OneD_> banded_view_type; /* bogus */
#endif

  enum { M = 0, N = 0 };

  typedef OneD_ OneD;
  typedef OneD& OneDRef;
  typedef const OneD& ConstOneDRef;

  typedef internal_tag storage_loc;

  typedef typename OneD::sparsity sparsity;

  typedef not_strideable strideability; /* JGS need a better name */

  //: The 1D container type
  typedef typename rep_type::value_type value_type;
  //: A reference to the value type
  typedef typename rep_type::reference reference;
  //: A const reference to the value type
  typedef typename rep_type::const_reference const_reference;
  //: The integral type for dimensions and indices
  typedef typename rep_type::size_type size_type;
#if !defined( _MSVCPP_ )
  //: The iterator type
  typedef dense_iterator<typename rep_type::iterator> iterator;
  //: The const iterator type
  typedef dense_iterator<typename rep_type::const_iterator> const_iterator;
#else
  typedef dense_iterator<typename rep_type::value_type, 0> iterator;
  typedef dense_iterator<typename rep_type::value_type, 1> const_iterator;
#endif
  //: The reverse iterator type
  typedef reverse_iter<iterator> reverse_iterator;
  //: The const reverse iterator type
  typedef reverse_iter<const_iterator> const_reverse_iterator;
  //: A pair type for the dimension
  typedef dimension<size_type> dim_type;
  //: A pair type for the bandwidth
  typedef dimension<int> band_type;

protected:
  inline void resize_oned(sparse_tag) {
    for (iterator i = this->begin(); i != this->end(); ++i)
      *i = OneD();
  }

  inline void resize_oned(dense_tag) {
    for (iterator i = this->begin(); i != this->end(); ++i)
      *i = OneD(dim.second());
    /*    for (size_type i = 0; i < dim.first(); ++i)
     *      (*rep)[i].resize(dim.second());
     */
  }

  inline void resize_banded(band_type , sparse_tag) {
    for (iterator i = this->begin(); i != this->end(); ++i)
      *i = OneD();
  }
  inline void resize_banded(band_type bw, dense_tag) {
    for (iterator i = this->begin(); i != this->end(); ++i) {
      band_type sf = calc_start_fini(i.index(), dim.second(), bw);
      *i = OneD(sf.second() - sf.first());
    }
    /*    for (size_type i = 0; i < dim.first(); ++i) {
     *      band_type sf = calc_start_fini(i, dim.second(), bw);
     *      (*rep)[i].resize(sf.second() - sf.first());
     *    }
     */
  }
  inline void resize_banded(band_type bw) {
    resize_banded(bw, sparsity());
  }

public:

  /* Constructors */

  //: Default Constructor
  inline array2D()
    : dim(0,0), rep(0), start_index(0) { }

  //: Normal Constructor
  inline array2D(dim_type d, size_type start_index = 0)
    : dim(d), rep(new rep_type(d.first())),
      start_index(start_index) {
	resize_oned(sparsity());
  }

  //: Banded Constructor
  inline array2D(dim_type d, band_type band, size_type start_index = 0)
    : dim(d), rep(new rep_type(d.first())),
      start_index(start_index) {
      resize_banded(band);
  }

  //: sparse banded view constructor
  template <class TwoD>
  inline array2D(const TwoD& x, band_type, banded_tag)
    : dim(x.dim), rep(x.rep), start_index(x.start_index) { }

  //: Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline array2D(MatrixStream& s, Orien)
    : dim(Orien::map(dim_type(s.nrows(),s.ncols()))),
      rep(new rep_type(Orien::map(dim_type(s.nrows(),s.ncols())).first())),
      start_index(0)
  {
    resize_oned(sparsity());
  }

  //: Banded Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline array2D(MatrixStream& s, Orien, band_type bw)
    : dim(Orien::map(dim_type(s.nrows(),s.ncols()))),
      rep(new rep_type(Orien::map(dim_type(s.nrows(),s.ncols())).first())),
      start_index(0)
  {
    resize_banded(bw);
  }

  //: Copy Constructor (shallow)
  inline array2D(const self& x)
    : dim(x.dim), rep(x.rep), start_index(x.start_index) { }


  /* Access Methods */

  /* Iterator Access Methods */

  //: Return an iterator pointing to the first 1D container
  inline iterator begin() {
    return iterator(rep->begin(), start_index);
  }
  //: Return an iterator pointing past the end of the 2D container
  inline iterator end() {
    return iterator(rep->begin(), rep->size() + start_index);
  }
  //: Return a const iterator pointing to the first 1D container
  inline const_iterator begin() const {
    return const_iterator(rep->begin(), start_index);
  }
  //: Return a const iterator pointing past the end of the 2D container
  inline const_iterator end() const {
    return const_iterator(rep->begin(),
				rep->size() + start_index);
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
  inline OneD_reference operator () (size_type i, size_type j) {
    return (*rep)[i][j];
  }
  //: Return a const reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline OneD_const_reference operator () (size_type i, size_type j) const {
    return (*rep)[i][j];
  }


  /* Vector Access Methods */

  //: Return a reference to the ith 1D container
  inline OneDRef operator [](size_type i) {
    return (*rep)[i];
  }
  //: Return a const reference to the ith 1D container
  inline ConstOneDRef operator [](size_type i) const {
    return (*rep)[i];
  }

  /* Size Methods */

  //: The dimension of the 2D container
  inline size_type major() const { return dim.first(); }
  //: The dimension of the 1D containers
  inline size_type minor() const { return dim.second(); }
  //: The number of non-zeros
  inline size_type nnz() const {
    size_type nz = 0;
    for (size_type i = 0; i < rep->size(); ++i) {
      nz += (*rep)[i].nnz();
    }
    return nz;
  }
  //: Capacity
  inline size_type capacity() const {
    size_type nz = 0;
    for (size_type i = 0; i < rep->size(); ++i) {
      nz += (*rep)[i].capacity();
    }
    return nz;
  }

  inline void print() const {
    for (typename rep_type::const_iterator i = rep->begin();
	 i != rep->end(); ++i)
      (*i).print();
  }

  inline size_type first_index() const { return start_index; }

  //: A faster specialization for copying
  template <class Matrix>
  inline void fast_copy(const Matrix& x) {
    typename Matrix::const_iterator xi = x.begin();
    for (iterator i = begin(); i != end(); ++i, ++xi) {
      typename Matrix::OneD::IndexArrayRef ind = (*xi).nz_struct();
      *i = OneD(ind.begin(), ind.end(),	(*xi).size());
      copy(*xi, *i);
    }
  }

  /* JGS add sub_matrix */

  void resize(size_type m, size_type n) {
    rep_ptr newrep = new rep_type(m);
    { for (typename rep_type::iterator i = newrep->begin();
	   i != newrep->end(); ++i)
      *i = OneD(n);
    }
    size_type M = MTL_MIN(m, dim.first());
    size_type N = MTL_MIN(n, dim.second());
    size_type i, j;
    for (i = 0; i < M; ++i)
      for (j = 0; j < N; ++j)
	(*newrep)[i][j] = (*rep)[i][j];

    for (; i < m; ++i)
      for (; j < n; ++j)
	(*newrep)[i][j] = T();

    rep = newrep;
    dim = dim_type(m, n);
  }

protected:
  dim_type dim;
  rep_ptr rep;
  size_type start_index; /* index of the first row/col */
};

//: array2D generator used in matrix.h
//!noindex:
template <class T>
struct gen_array2D {
  typedef gen_array2D<T> submatrix_type; /* bogus */
#if 0
  typedef gen_compressed2D<T,int,0,0,0> transpose_type; /* bogus */
  typedef gen_compressed2D<T,int,0,0,0> banded_view_type; /* bogus */
#else
  typedef gen_array2D<T> transpose_type;
  typedef gen_array2D<T> banded_view_type;
#endif
  typedef array2D<T> type;
};

} /* namespace mtl */

#endif








#if 0
  //: Sparse Matrix Non-Zero Info
  inline nz_structure< array2D<OneD> > nz_struct() {
    return nz_structure< array2D<OneD> >(*this);
  }

  //: Sparse Matrix Skeleton Constructor
  template <class Sparse2D>
  inline array2D(const nz_structure<Sparse2D>& x)
  : rep(new rep_type(x.major())), start_index(0) {
    for (int i = 0; i < major_; ++i)
      (*rep)[i] = OneD(x[i].nz_struct());
  }
#endif
