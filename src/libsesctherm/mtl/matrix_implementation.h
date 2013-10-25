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

#ifndef _MTL_MATRIX_IMPLEMENTATION_H_
#define _MTL_MATRIX_IMPLEMENTATION_H_

#include "mtl_iterator.h" /* for advance() */
#include <utility>
#include "reverse_iter.h"

#include "oned_part.h"
#include "scaled2D.h"
#include "meta_if.h"
#include "meta_equal.h"
#include "dimension.h"
#include "matrix_stream.h"
#include "linalg_vec.h"
#include "banded_indexer.h"
#include "partition.h"
#include "light_matrix.h"

#include "initialize.h"

namespace mtl {

template < class T, class Shape, class Storage, class Orientation>
struct matrix;

template <int MM, int NN>
class rectangle;

template <int External>
struct dense;


//: The main MTL matrix implementation type.
// This class synthesizes all of the various components
// that go into a matrix and presents the functionality
// to the user with the proper interface.
// The other matrix implementation types derive from
// this class. This class is not used directly.
//
//!category: container
//!component: type
template <class TwoDGen, class IndexerGen>
class matrix_implementation {
  typedef matrix_implementation<TwoDGen,IndexerGen> self;
public:
  typedef typename IndexerGen::twod_dim_type twoddim;

  enum { M = twoddim::M, N = twoddim::N };
  typedef typename TwoDGen::type TwoD;

  typedef typename TwoD::size_type size_type;
  typedef mtl::dimension<size_type> dyn_dim;

  typedef twod_tag dimension; /* name clash a problem */

  typedef typename IndexerGen::type Indexer;

  typedef typename TwoD::value_type OldOneD;
  typedef typename TwoD::reference OldOneDRef;
  typedef typename TwoD::iterator oned_iterator;
  typedef typename TwoD::const_iterator const_oned_iterator;

  typedef typename Indexer::dim_type dim_type;
  typedef typename Indexer::band_type band_type;

  typedef oned_part<OldOneD, OldOneD, typename Indexer::OneDIndexer> OneD;
  typedef oned_part<OldOneD, OldOneDRef, typename Indexer::OneDIndexer> OneDRef;
  typedef typename TwoD::value_type TwoD_value_type;
  typedef typename TwoD_value_type::value_type value_type;
  typedef typename TwoD_value_type::reference reference;
  typedef typename TwoD_value_type::const_reference const_reference;
  typedef typename TwoD_value_type::pointer pointer;
  typedef typename TwoD_value_type::difference_type difference_type;

  typedef typename Indexer::shape shape;
  typedef typename Indexer::orientation orientation;
  typedef typename TwoD::sparsity sparsity;
  typedef typename TwoD::storage_loc storage_loc;

  typedef matrix_implementation< TwoDGen,
                         typename IndexerGen::transpose_type > transpose_type;

  typedef matrix_implementation< typename TwoDGen::transpose_type,
                         typename IndexerGen::strided_type > strided_type;

  typedef matrix_implementation< typename TwoDGen::banded_view_type,
                         typename IndexerGen::strided_type > banded_view_type;

  typedef typename TwoD::strideability strideability;

  typedef matrix_implementation< gen_scaled2D<TwoD, value_type>,
                                 IndexerGen > scaled_type;

  typedef OneD NewOneD;

  template <int isConst>
  class _iterator {
    typedef _iterator self;
    typedef typename IF<isConst, const_oned_iterator,oned_iterator>::RET Iterator;
  public:
#if !defined( _MSVCPP_ )
	  typedef typename std::iterator_traits<Iterator>::difference_type
                               difference_type;
#else
	  typedef typename std::iterator_traits<Iterator>::distance_type difference_type;
	  typedef difference_type distance_type;
#endif

    typedef typename IF<isConst, const NewOneD, NewOneD>::RET value_type;
    typedef typename IF<isConst, const NewOneD, NewOneD>::RET reference;
    typedef typename IF<isConst, const NewOneD*, NewOneD*>::RET pointer;

    typedef typename std::iterator_traits<Iterator>::iterator_category
                               iterator_category;

    typedef difference_type Distance;
    typedef Iterator iterator_type;

    inline difference_type index() const { return iter.index(); }

    inline _iterator() { }

    inline _iterator(Iterator x, Indexer ind)
      : iter(x), indexer(ind) { }

    inline _iterator(const self& x)
      : iter(x.iter), indexer(x.indexer) { }

    inline self& operator=(const self& x) {
      iter = x.iter; indexer = x.indexer; return *this;
    }

    inline operator Iterator() { return iter; }

    inline Iterator base() const { return iter; }

    inline reference operator*() const {
      typename Indexer::OneDIndexer oned_indexer = indexer.deref(iter);
#if 0
	  typename Iterator::value_type v = *iter; /* VC++ */
#endif
      return reference(*iter, oned_indexer);
	}

    inline self& operator++() { ++iter; return *this; }

    inline self operator++(int) {
      self tmp = (*this);
      ++(*this);
      return tmp;
    }

    inline self& operator--() { --iter; return *this; }

    inline self operator--(int) {
      self tmp = (*this);
      --(*this);
      return tmp;
    }

    inline self operator+(Distance n) const {
      self tmp = (*this);
      tmp += n;
      return tmp;
    }

    inline self& operator+=(Distance n) {
      iter += n; return (*this);
    }

    inline self operator-(Distance n) const {
      self tmp = (*this);
      tmp -= n;
      return tmp;
    }

    inline self& operator-=(Distance n) {
      iter -= n; return (*this);
    }

    inline value_type operator[](Distance n) const {
      self tmp = (*this);
      return *(tmp += n);
    }

    inline Distance operator-(const self& y) {
      return iter - y.iter;
    }

    inline bool operator==(const self& y) const {
      return iter == y.iter;
    }

    inline bool operator!=(const self& y) const {
      return iter != y.iter;
    }

    inline bool operator<(const self& y) const {
      return iter < y.iter;
    }

  protected:
    Iterator iter;
    Indexer indexer;
  };

  typedef _iterator<0> iterator;
  typedef _iterator<1> const_iterator;

  typedef reverse_iter<iterator> reverse_iterator;
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  typedef typename Indexer::orienter orien;

  /* constructors */
  inline matrix_implementation() { }

  inline matrix_implementation(dim_type dim, size_type nnz_max)
    : twod(Indexer::twod_dim(orien::map(dim)), nnz_max),
      indexer(orien::map(dim)) { }

  inline matrix_implementation(dim_type dim)
    : twod(Indexer::twod_dim(orien::map(dim))),
      indexer(orien::map(dim)) { }

  inline matrix_implementation(dim_type dim, band_type bw)
    : twod(Indexer::twod_dim(orien::map(dim), orien::map(bw)),
           Indexer::twod_band(orien::map(dim), orien::map(bw))),
      indexer(orien::map(dim), orien::map(bw)) { }

  inline matrix_implementation(const TwoD& x, Indexer ind)
    : twod(x), indexer(ind) { }

  // copy constructor
  inline matrix_implementation(const matrix_implementation& x)
    : twod(x.twod), indexer(x.indexer) { }

  inline matrix_implementation(const matrix_implementation& x,
                               do_strided s)
    : twod(x.twod), indexer(x.indexer) { }

  inline self& operator=(const self& x) {
    twod = x.twod; indexer = x.indexer;
    return *this;
  }


  /* char added to fix compiler error with KCC with multiply
     matching constructors */
  inline matrix_implementation(const transpose_type& x,
                               do_transpose, do_transpose)
    : twod(x.get_twod()), indexer(x.get_indexer(), not_strideable()) { }

  inline matrix_implementation(const strided_type& x, do_strided, do_strided)
    : twod(x.get_twod(), do_transpose(), do_transpose()),
      indexer(x.get_indexer(), strideable()) { }

  template <class MatrixT, class ScalarT>
  inline matrix_implementation(const MatrixT& x, const ScalarT& y, do_scaled)
    : twod(x.get_twod(), y), indexer(x.get_indexer()) { }

  /* construct from external (pre-existing) memory,
   *    only for use with external2D
   */

  inline matrix_implementation(pointer data, dim_type dim, char)
    : twod(data, orien::map(dim)), indexer(orien::map(dim)) { }

  inline matrix_implementation(pointer data, dim_type dim, size_type ld)
    : twod(data, orien::map(dim), ld), indexer(orien::map(dim)) { }

  //: With non-zero upper-left corner starts
  inline matrix_implementation(pointer data, dim_type dim, size_type ld,
                               dyn_dim starts, char)
    : twod(data, orien::map(dim), ld, orien::map(starts), char()),
      indexer(orien::map(dim)) { }

  inline matrix_implementation(pointer data, dim_type dim,
                               band_type bw)
    : twod(data, orien::map(dim), orien::map(bw)),
      indexer(orien::map(dim), orien::map(bw)) { }

  inline matrix_implementation(pointer data, dim_type dim, size_type ld,
                               band_type bw)
    : twod(data, orien::map(dim), ld, orien::map(bw)),
      indexer(orien::map(dim)) { }

  //: compressed2D external data constructor
  inline matrix_implementation(dim_type dim, size_type nnz,
                        pointer val, size_type* ptrs, size_type* inds)
    : twod(orien::map(dim), nnz, val, ptrs, inds),
      indexer(orien::map(dim)) { }

  //: banded view constructor
  template <class MatrixT>
  inline matrix_implementation(const MatrixT& x, band_type bw)
    : twod(x.twod, orien::map(bw), banded_tag()),
      indexer(orien::map(dim_type(x.nrows(), x.ncols())), orien::map(bw)) { }

  //: block view constructor
  template <class Matrix, class ST, int BM, int BN>
  inline matrix_implementation(const Matrix& x,
                               mtl::dimension<ST,BM,BN> bd, char)
    : twod(x.twod, orien::map(bd)),
      indexer(orien::map(dim_type(x.nrows()/bd.first(),
                                  x.ncols()/bd.second()))) { }

  //: Static M, N Constructor
  inline matrix_implementation(pointer data)
    : twod(data, orien::map(dim_type(0, 0))),
      indexer(orien::map(dim_type(0, 0))) { }

  inline matrix_implementation(pointer data, size_type ld)
    : twod(data, orien::map(dim_type(0, 0)), ld),
      indexer(orien::map(dim_type(0, 0))) { }

  //: matrix stream constructor
  typedef matrix_market_stream<value_type> mmstream;
  typedef harwell_boeing_stream<value_type> hbstream;

  template <class Me>
  inline matrix_implementation(mmstream& m_in, Me& me)
    : twod(m_in, orien()), indexer(orien::map(dim_type(m_in.nrows(),
                                                       m_in.ncols())))
  {
    mtl::initialize(me, m_in);
  }
  template <class Me>
  inline matrix_implementation(hbstream& m_in, Me& me)
    : twod(m_in, orien()), indexer(orien::map(dim_type(m_in.nrows(),
                                                       m_in.ncols())))
  {
    mtl::initialize(me, m_in);
  }
  template <class Me>
  inline matrix_implementation(mmstream& m_in, band_type bw, Me& me)
    : twod(m_in, orien(),
           Indexer::twod_band(orien::map(dim_type(m_in.nrows(),
                                                  m_in.ncols())),
                              orien::map(bw))),
      indexer(orien::map(dim_type(m_in.nrows(), m_in.ncols())),
              orien::map(bw))
  {
    mtl::initialize(me, m_in);
  }
  template <class Me>
  inline matrix_implementation(hbstream& m_in, band_type bw, Me& me)
    : twod(m_in, orien(),
           Indexer::twod_band(orien::map(dim_type(m_in.nrows(),
                                                  m_in.ncols())),
                              orien::map(bw))),
      indexer(orien::map(dim_type(m_in.nrows(), m_in.ncols())),
              orien::map(bw))
  {
    mtl::initialize(me, m_in);
  }

  inline ~matrix_implementation() { }

  /* iterators */
  inline iterator begin() { return iterator(twod.begin(), indexer); }
  inline iterator end() { return iterator(twod.end(), indexer); }

  inline const_iterator begin() const {
    return const_iterator(twod.begin(),indexer);
  }
  inline const_iterator end() const {
    return const_iterator(twod.end(),indexer);
  }

  inline reverse_iterator rbegin() { return reverse_iterator(end()); }
  inline reverse_iterator rend() { return reverse_iterator(begin()); }

  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  inline const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  /* element access */
  inline typename OneD::reference operator()(size_type i, size_type j) {
    dyn_dim p = indexer.at(dyn_dim(i, j));
    return twod(p.first(), p.second());
  }
  inline typename OneD::const_reference operator()(size_type i,
                                                   size_type j) const {
    dyn_dim p = indexer.at(dyn_dim(i, j));
    return twod(p.first(), p.second());
  }

  /* OneD access */
  inline OneDRef operator[](size_type n) {
    return OneDRef(twod[n], indexer.deref(n));
  }
  inline const OneDRef operator[](size_type n) const {
    return OneDRef((OldOneDRef)twod[n], indexer.deref(n));
  }

  /* size */
  inline size_type nrows() const { return indexer.nrows(); }
  inline size_type ncols() const { return indexer.ncols(); }
  inline size_type noneds() const { return twod.major(); }
  inline size_type major() const { return twod.major(); }
  inline size_type minor() const { return twod.minor(); }
  inline size_type nnz() const { return twod.nnz(); }
  inline size_type capacity() const { return twod.capacity(); }

  /* bandwidth */
  inline int sub() const { return indexer.sub(); }
  inline int super() const { return indexer.super(); }

  /* some shape properties */
  inline bool is_upper() const { return false; }
  inline bool is_lower() const { return false; }
  inline bool is_unit() const { return false; }

  inline const TwoD& get_twod() const { return twod; }
  inline const Indexer& get_indexer() const { return indexer; }

  inline void print() const { twod.print(); }

  /* external storage interface */
  inline value_type* data() { return twod.data(); }
  inline const value_type* data() const { return twod.data(); }

  /* compressed2D external storage interface */
  inline value_type* get_val() { return twod.get_val(); }
  inline const value_type* get_val() const { return twod.get_val(); }
  inline size_type* get_ind() { return twod.get_ind(); }
  inline const size_type* get_ind() const { return twod.get_ind(); }
  inline size_type* get_ptr() { return twod.get_ptr(); }
  inline const size_type* get_ptr() const { return twod.get_ptr(); }

  template <class Matrix>
  inline void fast_copy(const Matrix& x) { twod.fast_copy(x); }


  /* protected: */
  TwoD twod;
  Indexer indexer;
};


template <class TwoDGen, class IndexerGen>
class column_matrix;


//: row matrix
//
// This class derives from the matrix_implementation class.
// The main purpose of this class is merely to add the "Row"
// type definition.

template <class TwoDGen, class IndexerGen>
class row_matrix : public matrix_implementation<TwoDGen, IndexerGen> {
  typedef matrix_implementation<TwoDGen, IndexerGen> Base;
public:
  typedef typename Base::value_type value_type;
  typedef typename Base::size_type size_type;
  typedef typename Base::pointer pointer;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  typedef typename Base::dim_type dim_type;
  typedef typename Base::dyn_dim dyn_dim;
  typedef typename Base::TwoD TwoD;
  typedef typename Base::OneD OneD;
  typedef typename Base::OneDRef OneDRef;
  typedef typename Base::Indexer Indexer;
  typedef OneD Row;
  typedef OneDRef RowRef;
  enum { M = Indexer::M, N = Indexer::N };
  typedef column_matrix<TwoDGen, typename IndexerGen::transpose_type>
             transpose_type;
  typedef column_matrix<typename TwoDGen::transpose_type,
                        typename IndexerGen::strided_type> strided_type;

  typedef row_matrix<typename TwoDGen::banded_view_type,
        gen_banded_indexer<typename IndexerGen::orienter,M,N,size_type> > banded_view_type;

  typedef typename Indexer::band_type band_type;

  //#if !defined( __GNUC__ ) && !defined( _MSVCPP_ )  /* internal compiler error */
#if !defined( MTL_DISABLE_BLOCKING )
  template <int BM, int BN>
  struct blocked_view {
#if 0
    typedef matrix<value_type, rectangle<BM,BN>,
                   dense<external>, row_major>::type Block;
#else
    typedef typename TwoD::is_strided IsStrided;

    //    typedef light_matrix<value_type, size_type, ROW_MAJOR, IsStrided::id> Block;
    typedef light_matrix<value_type, int, ROW_MAJOR, IsStrided::id> Block;
    //    typedef light_matrix<value_type, 1, TwoD::is_strided::id,BM,BN> Block;
#endif

    typedef typename TwoDGen:: MTL_TEMPLATE blocked_view<Block>::type BlockTwoDGen;
    typedef row_matrix<BlockTwoDGen, IndexerGen> type;
  };
#endif

  inline row_matrix() { }

  inline row_matrix& operator=(const row_matrix& x) {
    Base::operator=(x);
    return *this;
  }
  //: user callable constructors
  inline row_matrix(size_type m, size_type n)
    : Base(dim_type(m, n)) { }
  inline row_matrix(size_type m, size_type n, size_type nnz_max)
    : Base(dim_type(m, n), nnz_max) { }
  inline row_matrix(size_type m, size_type n, int sub, int super)
    : Base(dim_type(m, n), typename Indexer::band_type(sub, super)) { }

  //: external data constructors
  inline row_matrix(pointer data, size_type m, size_type n)
    : Base(data, dim_type(m, n), char()) { }
  inline row_matrix(pointer data, size_type m, size_type n, size_type ld)
    : Base(data, dim_type(m, n), ld) { }

  //: non zero index upper left corner
  inline row_matrix(pointer data, size_type m, size_type n,
                    size_type ld, dyn_dim starts)
    : Base(data, dim_type(m, n), ld, starts, char()) { }

  inline row_matrix(pointer data, size_type m, size_type n,
                    int sub, int super)
    : Base(data, dim_type(m, n),
           band_type(sub, super)) { }
  inline row_matrix(pointer data, size_type m, size_type n, size_type ld,
                    int sub, int super)
    : Base(data, dim_type(m, n), ld,
           band_type(sub, super)) { }
  //: Static M, N Constructor
  inline row_matrix(pointer data)
    : Base(data) { }

  //: compressed2D external data constructor
  inline row_matrix(size_type m, size_type n, size_type nnz,
             pointer val, size_type* ptrs, size_type* inds)
    : Base(dim_type(m, n), nnz, val, ptrs, inds) { }

  //: banded view constructor
  template <class Matrix>
  inline row_matrix(int sub, int super, const Matrix& x)
    : Base(x, band_type(sub, super)) { }

  //: block view constructor
  template <class Matrix, class ST, int BM, int BN>
  inline row_matrix(const Matrix& x, mtl::dimension<ST,BM,BN> bdim)
    : Base(x, bdim, char()) { }

  //: stream constructors
  typedef matrix_market_stream<value_type> mmstream;
  typedef harwell_boeing_stream<value_type> hbstream;
  inline row_matrix(mmstream & m_in) : Base(m_in, *this) { }
  inline row_matrix(hbstream & m_in) : Base(m_in, *this) { }
  inline row_matrix(mmstream& m_in, int sub, int super)
    : Base(m_in, band_type(sub, super), *this) { }
  inline row_matrix(hbstream& m_in, int sub, int super)
    : Base(m_in, band_type(sub, super), *this) { }

  template <class Subclass>
  inline row_matrix(mmstream & m_in, Subclass& s) : Base(m_in, s) { }
  template <class Subclass>
  inline row_matrix(hbstream & m_in, Subclass& s) : Base(m_in, s) { }
  template <class Subclass>
  inline row_matrix(mmstream& m_in, int sub, int super, Subclass& s)
    : Base(m_in, band_type(sub, super), s) { }
  template <class Subclass>
  inline row_matrix(hbstream& m_in, int sub, int super, Subclass& s)
    : Base(m_in, band_type(sub, super), s) { }

  //: copy constructor
  inline row_matrix(const row_matrix& x)
    : Base(x) { }

  //:: called by rows(A) and columns(A)
  inline row_matrix(const row_matrix& x, do_strided)
    : Base(x) { }

  //: called by trans helper function
  inline row_matrix(const transpose_type& x, do_transpose t)
    : Base(x, t, t) { }

  //: called by rows and columns helper functions
  inline row_matrix(const strided_type& x, do_strided s)
    : Base(x, s, s) { }

  //: called by scaled helper function
  template <class MatrixT, class ScalarT>
  inline row_matrix(const MatrixT& x, const ScalarT& y, do_scaled s)
    : Base(x, y, s) { }

  inline row_matrix(const TwoD& x, Indexer ind)
    : Base(x, ind) { }

  inline ~row_matrix() { }

  inline void resize(size_type m, size_type n) {
    Base::twod.resize(m, n);
    Base::indexer.dim = dim_type(m, n);
  }

  /* submatrix */
#if 1
  typedef typename TwoDGen::submatrix_type SubTwoDGen;
  typedef typename SubTwoDGen::type::size_type new_sizeT;
#if defined(_MSVCPP_)
  typedef IndexerGen SubMatIndexerGen;
#else
  typedef typename IndexerGen:: template bind<new_sizeT>::other SubMatIndexerGen;
#endif
  typedef row_matrix<SubTwoDGen, SubMatIndexerGen> submatrix_type;

  // problem, The new TwoDGen::submatrix_type may have a difference
  // size_type then previously. Since IndexerGen still has the
  // old size_type, there is a conflict
#else
  //  typedef light_matrix<value_type, 1, TwoD::is_strided::id, 0, 0> submatrix_type;
  typedef typename TwoD::is_strided IsStrided;
  //correct  typedef light_matrix<value_type, size_type, ROW_MAJOR, IsStrided::id> submatrix_type;
  typedef light_matrix<value_type, int, ROW_MAJOR, IsStrided::id> submatrix_type;
#endif

  //: create a submatrix view into this matrix
  // Currently only works with dense rectangular matrices
  inline submatrix_type sub_matrix(size_type row_start, size_type row_finish,
                                   size_type col_start, size_type col_finish) const
  {
    dim_type starts = Base::indexer.at(dim_type(row_start, col_start));
    size_type m = row_finish - row_start;
    size_type n = col_finish - col_start;
    typedef typename TwoD::is_strided IsStrided;
    if (IsStrided::id) {
      return submatrix_type((value_type*)Base::twod.data()
			    + starts.second() * Base::twod.ld() + starts.first(),
			    m, n, Base::twod.ld());
    } else {
      return submatrix_type((value_type*)Base::twod.data()
			    + starts.first() * Base::twod.ld() + starts.second(),
			    m, n, Base::twod.ld());
    }
  }

#if 0
  // deprecated, derive partitioned matrix type from submatrix_type instead
  // and do it in partition.h
  /* partition */
  typedef row_matrix< typename TwoD::template partitioned<submatrix_type>::generator,
                      IndexerGen> partitioned;
#endif

#if 0 // deprecated, use functions in partition.h instead
  template <class Sequence1, class Sequence2>
  inline partitioned
  partition(const Sequence1& prows, const Sequence2& pcols) const
  {
    return partition_matrix(prows, pcols, *this);
  }
  inline partitioned
  subdivide(size_type split_row, size_type split_col) const
  {
    dense1D<size_type> row_splits(1);
    dense1D<size_type> col_splits(1);

    row_splits[0] = split_row;
    col_splits[0] = split_col;

    return partition_matrix(row_splits, col_splits, *this);
  }
#endif
};

//: column matrix
//
// This class derives from the matrix_implementation class.
// The main purpose of this class is merely to add the "Column"
// type definition.

template <class TwoDGen, class IndexerGen>
class column_matrix : public matrix_implementation<TwoDGen, IndexerGen> {
  typedef matrix_implementation<TwoDGen, IndexerGen> Base;
public:
  typedef typename Base::pointer pointer;
  typedef typename Base::reference reference;
  typedef typename Base::const_reference const_reference;
  typedef typename Base::value_type value_type;
  typedef typename Base::size_type size_type;
  typedef typename Base::dim_type dim_type;
  typedef typename Base::dyn_dim dyn_dim;
  typedef typename Base::band_type band_type;
  typedef typename Base::TwoD TwoD;
  typedef typename Base::OneD OneD;
  typedef typename Base::OneDRef OneDRef;
  typedef typename Base::Indexer Indexer;
  typedef OneD Column;
  typedef OneDRef ColumnRef;
  enum { M = Indexer::M, N = Indexer::N };
  typedef row_matrix<TwoDGen,
                     typename IndexerGen::transpose_type> transpose_type;
  typedef row_matrix<typename TwoDGen::transpose_type,
                     typename IndexerGen::strided_type> strided_type;

  typedef column_matrix<typename TwoDGen::banded_view_type,
        gen_banded_indexer<typename IndexerGen::orienter,M,N,size_type> > banded_view_type;

  //#if !defined( __GNUC__) && !defined( _MSVCPP_ ) /* internal compiler error */
#if !defined( MTL_DISABLE_BLOCKING )
  template <int BM, int BN>
  struct blocked_view {
#if 0
    typedef matrix<value_type, rectangle<BM,BN>,
                   dense<external>, column_major>::type Block;
#else
    typedef typename TwoD::is_strided IsStrided;
    typedef light_matrix<value_type, size_type, COL_MAJOR, IsStrided::id> Block;
    //    typedef light_matrix<value_type, 0, TwoD::is_strided::id,BM,BN> Block;
#endif
    typedef column_matrix<typename TwoDGen:: MTL_TEMPLATE blocked_view<Block>::type,
                          IndexerGen> type;
  };
#endif

  inline column_matrix() { }

  //: user callable constructors
  inline column_matrix(size_type m, size_type n)
    : Base(dim_type(m, n)) { }
  inline column_matrix(size_type m, size_type n, size_type nnz_max)
    : Base(dim_type(m, n), nnz_max) { }
  inline column_matrix(size_type m, size_type n, int sub, int super)
    : Base(dim_type(m, n),
           band_type(sub, super)) { }

  inline column_matrix& operator=(const column_matrix& x) {
    Base::operator=(x);
    return *this;
  }
  //: external data
  inline column_matrix(pointer data, size_type m, size_type n)
    : Base(data, dim_type(m, n), char()) { }
  inline column_matrix(pointer data, size_type m, size_type n, size_type ld)
    : Base(data, dim_type(m, n), ld) { }

  //: non zero index upper left corner
  inline column_matrix(pointer data, size_type m, size_type n,
                       size_type ld, dyn_dim starts)
    : Base(data, dim_type(m, n), ld, starts, char()) { }

  inline column_matrix(pointer data, size_type m, size_type n,
                       int sub, int super)
    : Base(data, dim_type(m, n),
           band_type(sub, super)) { }
  inline column_matrix(pointer data, size_type m, size_type n, size_type ld,
                       int sub, int super)
    : Base(data, dim_type(m, n), ld,
           band_type(sub, super)) { }

  //: Static M, N Constructor
  inline column_matrix(pointer data)
    : Base(data) { }

  //: banded view constructor
  template <class Matrix>
  inline column_matrix(int sub, int super, const Matrix& x)
    : Base(x, band_type(sub, super)) { }

  //: block view constructor
  template <class Matrix, class ST, int BM, int BN>
  inline column_matrix(const Matrix& x, mtl::dimension<ST,BM,BN> bdim)
    : Base(x, bdim, char()) { }


  //: compressed2D external data constructor
  inline column_matrix(size_type m, size_type n, size_type nnz,
                pointer val, size_type* ptrs, size_type* inds)
    : Base(dim_type(m, n), nnz, val, ptrs, inds) { }

  //: streams
  typedef matrix_market_stream<value_type> mmstream;
  typedef harwell_boeing_stream<value_type> hbstream;
  inline column_matrix(mmstream& m_in) : Base(m_in, *this) { }
  inline column_matrix(mmstream& m_in, int sub, int super)
    : Base(m_in, band_type(sub,super), *this) { }
  inline column_matrix(hbstream& m_in) : Base(m_in, *this) { }
  inline column_matrix(hbstream& m_in, int sub, int super)
    : Base(m_in, band_type(sub,super), *this) { }

  template <class Subclass>
  inline column_matrix(mmstream & m_in, Subclass& s) : Base(m_in, s) { }
  template <class Subclass>
  inline column_matrix(hbstream & m_in, Subclass& s) : Base(m_in, s) { }
  template <class Subclass>
  inline column_matrix(mmstream& m_in, int sub, int super, Subclass& s)
    : Base(m_in, band_type(sub, super), s) { }
  template <class Subclass>
  inline column_matrix(hbstream& m_in, int sub, int super, Subclass& s)
    : Base(m_in, band_type(sub, super), s) { }

  //: copy constructor
  inline column_matrix(const column_matrix& x)
    : Base(x) { }

  //: called by rows(A), columns(A)
  inline column_matrix(const column_matrix& x, do_strided)
    : Base(x) { }

  //: called by trans(A) helper function
  inline column_matrix(const transpose_type& x, do_transpose t)
    : Base(x, t, t) { }

  //: called by rows(A) and columns(A) helper functions
  inline column_matrix(const strided_type& x, do_strided s)
    : Base(x, s, s) { }

  //: scaled
  template <class MatrixT, class ScalarT>
  inline column_matrix(const MatrixT& x, const ScalarT& y, do_scaled s)
    : Base(x, y, s) { }

  inline column_matrix(const TwoD& x, Indexer ind)
    : Base(x, ind) { }

  inline ~column_matrix() { }

  inline void resize(size_type m, size_type n) {
    Base::twod.resize(n, m);
    Base::indexer.dim = dim_type(n, m);
  }

  /* submatrix */
#if 1
  typedef typename TwoDGen::submatrix_type SubTwoDGen;
  typedef typename SubTwoDGen::type::size_type new_sizeT;
#if defined(_MSVCPP_)
  typedef IndexerGen SubMatIndexerGen;
#else
  typedef typename IndexerGen:: template bind<new_sizeT>::other SubMatIndexerGen;
#endif
  typedef column_matrix<SubTwoDGen, SubMatIndexerGen> submatrix_type;

#else
  //  typedef light_matrix<value_type, 0, TwoD::is_strided::id, 0, 0> submatrix_type;
  typedef typename TwoD::is_strided IsStrided;
  typedef light_matrix<value_type, size_type, COL_MAJOR, IsStrided::id> submatrix_type;
#endif

  //: create a submatrix view into this matrix
  // Currently only works with dense rectangular matrices
  inline submatrix_type sub_matrix(size_type row_start, size_type row_finish,
                             size_type col_start, size_type col_finish) const
  {
    dim_type starts = Base::indexer.at(dim_type(row_start, col_start));
    size_type m = row_finish - row_start;
    size_type n = col_finish - col_start;
    typedef typename TwoD::is_strided IsStrided;
    if (IsStrided::id) {
      return submatrix_type((value_type*)Base::twod.data()
			    + starts.second() * Base::twod.ld() + starts.first(),
			    m, n, Base::twod.ld());
    } else {
      return submatrix_type((value_type*)Base::twod.data()
			    + starts.first() * Base::twod.ld() + starts.second(),
			    m, n, Base::twod.ld());
    }
  }

#if 0
  // JGS see row_matrix::partitioned
  /* partition */
  typedef column_matrix< typename TwoD::template partitioned<submatrix_type>::generator,
                      IndexerGen> partitioned;
#endif

#if 0 // deprecated, use functions in partition.h instead
  template <class Sequence1, class Sequence2>
  inline partitioned
  partition(const Sequence1& prows, const Sequence2& pcols) const
  {
    return partition_matrix(prows, pcols, *this);
  }

  inline partitioned
  subdivide(size_type split_row, size_type split_col) const
  {
    dense1D<size_type> row_splits(1);
    dense1D<size_type> col_splits(1);

    row_splits[0] = split_row;
    col_splits[0] = split_col;

    return partition_matrix(row_splits, col_splits, *this);
  }
#endif

};

template <class Matrix>
struct rows_type {
  typedef typename Matrix::orientation orien;
  enum { orienid = orien::id }; // VC++ workaround
  typedef typename IF<EQUAL<orienid,ROW_MAJOR>::RET,
                      Matrix, typename Matrix::strided_type>::RET type;
};

template <class Matrix>
struct columns_type {
  typedef typename Matrix::orientation orien;
  enum { orienid = orien::id }; // VC++ workaround
  typedef typename IF<EQUAL<orienid,COL_MAJOR>::RET,
                       Matrix, typename Matrix::strided_type>::RET type;
};

//: Access the row-wise view of the matrix
//
//  For matrix A, A[i] now gives you the ith row
//  and A.begin() gives you an iterator over rows
//
//!example: swap_rows.cc
//!component: function
//!category: containers
//!tparam: Matrix - The Matrix to access row-wise. Matrix must be dense.
template<class Matrix>
inline typename rows_type<Matrix>::type
rows(const Matrix& A) {
  return typename rows_type<Matrix>::type(A, do_strided());
}

//: Access the column-wise view of the matrix
//
//  For matrix A, A[i] now gives you the ith column and A.begin()
//  gives you an iterator over columns. See rows for an example.
//
//
//!component: function
//!category: containers
//!tparam: Matrix - The Matrix to access column-wise. Matrix must be dense.
template<class Matrix>
inline typename columns_type<Matrix>::type
columns(const Matrix& A) {
  return typename columns_type<Matrix>::type(A, do_strided());
}

//: Swap the orientation of a matrix.
//
// Swap the orientation of a matrix (i.e., from row-major to
// column-major). In essence this transposes the matrix. This
// operation occurs at compile time.
//
//!component: function
//!category: containers
//!tparam: Matrix - The Matrix to transpose
//!example: trans_mult.cc
template <class Matrix>
inline typename Matrix::transpose_type
trans(const Matrix& A) {
  typedef typename Matrix::transpose_type Trans;
  return Trans(A, do_transpose());
}

//: Diagonal Matrix
//
//  This class implements a DiagonalMatrix.
//
//!models: DiagonalMatrix
//!componont: type
//!category: container
template <class TwoDGen, class IndexerGen>
class diagonal_matrix : public matrix_implementation<TwoDGen, IndexerGen> {
  typedef matrix_implementation<TwoDGen, IndexerGen> Base;
  typedef diagonal_matrix self;
public:
  typedef typename Base::pointer pointer;
  typedef typename Base::reference reference;
  typedef typename Base::value_type value_type;
  typedef typename Base::size_type size_type;
  typedef typename Base::dim_type dim_type;
  typedef typename Base::dyn_dim dyn_dim;
  typedef typename Base::band_type band_type;
  typedef typename Base::TwoD TwoD;
  typedef typename Base::OneD OneD;
  typedef typename Base::OneDRef OneDRef;
  typedef typename Base::Indexer Indexer;
  typedef typename Base::const_reference const_reference;


  typedef OneD Diagonal;
  typedef OneDRef DiagonalRef;
  typedef diagonal_tag shape;

  typedef diagonal_matrix<TwoDGen,
                          typename IndexerGen::transpose_type> transpose_type;
  /* these are bogus */
#if 0
  typedef diagonal_matrix<typename TwoDGen::transpose_type,
                          typename IndexerGen::strided_type> strided_type;
#else
  // VC++ workaround
  typedef row_matrix<typename TwoDGen::transpose_type,
                          typename IndexerGen::strided_type> strided_type;
#endif
  inline diagonal_matrix() { }

  //: user callable constructors
  inline diagonal_matrix(size_type m, size_type n)
    : Base(dim_type(m, n)) { }
  inline diagonal_matrix(size_type m, size_type n,
                         int sub, int super)
    : Base(dim_type(m, n), band_type(sub, super)) { }

  //: constructor for external data
  inline diagonal_matrix(pointer d, size_type m, size_type n)
    : Base(d, dim_type(m, n)) { }
  inline diagonal_matrix(pointer d, size_type m, size_type n,
                  int sub, int super)
    : Base(d, dim_type(m, n), band_type(sub, super)) { }

  typedef matrix_market_stream<value_type> mmstream;
  typedef harwell_boeing_stream<value_type> hbstream;

  //: stream constructors
  inline diagonal_matrix(mmstream& m_in) : Base(m_in) { }

  //: stream constructors
  inline diagonal_matrix(hbstream& m_in) : Base(m_in) { }

  //: copy constructor
  inline diagonal_matrix(const self& x)
    : Base(x) { }

  //: called by strided(A) helper function
  inline diagonal_matrix(const self& x, do_strided s)
    : Base(x) { }

  //: called by trans(A) helper function
  inline diagonal_matrix(const transpose_type& x, do_transpose t)
    : Base(x, t, t) { }

  //: called by rows(A) and columns(A) helper functions
  inline diagonal_matrix(const strided_type& x, do_strided s)
    : Base(x, s, s) { }

  //: called by scaled(A) helper function
  template <class MatrixT, class ScalarT>
  inline diagonal_matrix(const MatrixT& x, const ScalarT& y, do_scaled s)
    : Base(x, y, s) { }

  inline ~diagonal_matrix() { }
};

//: triangle
// example and documentation
template <class Base_, class Uplo>
class triangle_matrix : public Base_ {
  typedef Base_ Base;
  Uplo uplo;
public:
  typedef typename Base::pointer pointer;
  typedef typename Base::reference reference;
  typedef typename Base::value_type value_type;
  typedef typename Base::size_type size_type;
  typedef typename Base::dim_type dim_type;
  typedef typename Base::dyn_dim dyn_dim;
  typedef typename Base::band_type band_type;
  typedef typename Base::TwoD TwoD;
  typedef typename Base::OneD OneD;
  typedef typename Base::OneDRef OneDRef;
  typedef typename Base::Indexer Indexer;
  typedef typename Base::orien orien;
  typedef typename Base::const_reference const_reference;

  typedef triangle_tag shape;

  typedef typename Uplo::transpose_type Uplotrans;
  typedef triangle_matrix<typename Base::transpose_type,
                          Uplotrans> transpose_type;
  /* JGS strided type is bogus */
  typedef triangle_matrix<typename Base::strided_type, Uplo> strided_type;


  inline triangle_matrix() { }

  inline triangle_matrix(size_type m, size_type n)
    : Base(m, n,
           uplo.bandwidth(m-1,n-1).first,
           uplo.bandwidth(m-1,n-1).second) { }

  //: constructor for external data
  inline triangle_matrix(pointer d, size_type m, size_type n)
    : Base(d, m, n,
           uplo.bandwidth(m-1, n-1).first,
           uplo.bandwidth(m-1, n-1).second) { }

  //: dynamic uplo constructor
  inline triangle_matrix(size_type m, size_type n, int uplo_)
    : Base(m, n, Uplo::bandwidth(uplo_, m, n)) , uplo(uplo_) { }

  //: constructor for external data with dynamic uplo
  inline triangle_matrix(pointer d, size_type m, size_type n, int uplo_)
    : Base(d, m, n,
           Uplo::bandwidth(uplo_, m-1, n-1).first,
           Uplo::bandwidth(uplo_, m-1, n-1).second) { }

  //: deprecated
  inline triangle_matrix(pointer d, size_type m, size_type n,
                         int sub, int super)
    : Base(d, m, n,
           uplo.bandwidth(sub,super).first,
           uplo.bandwidth(sub,super).second) { }

  //: triangular view constructor
  template <class Matrix>
  inline triangle_matrix(const Matrix& x)
    : Base(uplo.bandwidth(x.nrows()-1, x.ncols()-1).first,
           uplo.bandwidth(x.nrows()-1, x.ncols()-1).second, x) { }

  //: trans(A)
  inline triangle_matrix(const transpose_type& x, do_transpose t)
    : Base(x, t) { }

  //: rows(A), columns(A) This currently doesn't work
  inline triangle_matrix(const strided_type& x, do_strided s)
    : Base(x, s) { }

  inline triangle_matrix(const triangle_matrix& x, do_strided)
    : Base(x) { }


  typedef matrix_market_stream<value_type> mmstream;
  typedef harwell_boeing_stream<value_type> hbstream;
  inline triangle_matrix(mmstream& m_in)
    : Base(m_in,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).first,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).second,
           *this) { }
  inline triangle_matrix(hbstream& m_in)
    : Base(m_in,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).first,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).second,
           *this) { }

  //: scaled(A)
  template <class MatrixT, class ScalarT>
  inline triangle_matrix(const MatrixT& x, const ScalarT& y, do_scaled s)
    : Base(x, y, s) { }

  inline ~triangle_matrix() { }

  inline bool is_upper() const { return uplo.is_upper(); }
  inline bool is_lower() const { return ! uplo.is_upper(); }
  inline bool is_unit() const { return uplo.is_unit(); }

};


//: symmetric

template <class Base_, class Uplo>
class symmetric_matrix : public Base_ {
  typedef Base_ Base;
  Uplo uplo;
public:
  typedef typename Base::pointer pointer;
  typedef typename Base::reference reference;
  typedef typename Base::value_type value_type;
  typedef typename Base::size_type size_type;
  typedef typename Base::dim_type dim_type;
  typedef typename Base::dyn_dim dyn_dim;
  typedef typename Base::band_type band_type;
  typedef typename Base::TwoD TwoD;
  typedef typename Base::OneD OneD;
  typedef typename Base::OneDRef OneDRef;
  typedef typename Base::Indexer Indexer;
  typedef typename Base::orien orien;
  typedef typename Base::const_reference const_reference;

  typedef symmetric_tag shape;

  typedef typename Uplo::transpose_type Uplotrans;
  typedef symmetric_matrix<typename Base::transpose_type,
                           Uplotrans> transpose_type;
  /* JGS strided type is bogus */
  typedef symmetric_matrix<typename Base::strided_type, Uplo> strided_type;

  inline symmetric_matrix() { }

  inline symmetric_matrix(size_type n)
    : Base(n, n,
           uplo.bandwidth(n-1,n-1).first,
           uplo.bandwidth(n-1,n-1).second) { }
  inline symmetric_matrix(size_type n, int sub)
    : Base(n, n,
           uplo.bandwidth(sub,sub).first,
           uplo.bandwidth(sub,sub).second) { }

  //: constructor for external data
  inline symmetric_matrix(pointer d, size_type n)
    : Base(d, n, n,
           uplo.bandwidth(n-1,n-1).first,
           uplo.bandwidth(n-1,n-1).second) { }

  //: dynamic uplo constructor
  inline symmetric_matrix(size_type n, int uplo_, int sub)
    : Base(n, n, Uplo::bandwidth(uplo_, sub, sub)) , uplo(uplo_) { }

  //: constructor for external data with dynamic uplo
  inline symmetric_matrix(pointer d, size_type n, int uplo_, int sub)
    : Base(d, n, n,
           Uplo::bandwidth(uplo_, sub, sub).first,
           Uplo::bandwidth(uplo_, sub, sub).second) { }

  //: compressed2D external data constructor
  inline symmetric_matrix(size_type m, size_type n, size_type nnz,
			  pointer val, size_type* ptrs, size_type* inds)
    : Base(m, n, nnz, val, ptrs, inds) { }

  //: deprecated
  inline symmetric_matrix(pointer d, size_type n, int sub)
    : Base(d, n, n,
           uplo.bandwidth(sub,sub).first,
           uplo.bandwidth(sub,sub).second) { }

  inline symmetric_matrix(const transpose_type& x, do_transpose t)
    : Base(x, t) { }

  inline symmetric_matrix(const strided_type& x, do_strided s)
    : Base(x, s) { }

  typedef matrix_market_stream<value_type> mmstream;
  typedef harwell_boeing_stream<value_type> hbstream;
  inline symmetric_matrix(mmstream& m_in)
    : Base(m_in,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).first,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).second,
           *this) { }
  inline symmetric_matrix(hbstream& m_in)
    : Base(m_in,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).first,
           uplo.bandwidth(m_in.nrows()-1,m_in.ncols()-1).second,
           *this) { }

  template <class MatrixT, class ScalarT>
  inline symmetric_matrix(const MatrixT& x, const ScalarT& y, do_scaled s)
    : Base(x, y, s) { }

  inline ~symmetric_matrix() { }

  inline bool is_upper() const { return uplo.is_upper(); }
  inline bool is_lower() const { return ! uplo.is_upper(); }

  //: element access
  inline reference
  operator()(size_type row, size_type col) {
    if (uplo.is_upper()) {
      if (col > row)
        return Base::operator()(row, col);
      else
        return Base::operator()(col, row);
    } else {
      if (row > col)
        return Base::operator()(row, col);
      else
        return Base::operator()(col, row);
    }
  }
  //:
  inline const_reference
  operator()(size_type row, size_type col) const {
    if (uplo.is_upper()) {
      if (col > row)
        return Base::operator()(row, col);
      else
        return Base::operator()(col, row);
    } else {
      if (row > col)
        return Base::operator()(row, col);
      else
        return Base::operator()(col, row);
    }
  }

  /* bandwidth (is symmetric too) */
  inline int sub() const {
    return MTL_MAX(Base::indexer.super(), Base::indexer.sub());
  }
  inline int super() const {
    return MTL_MAX(Base::indexer.super(), Base::indexer.sub());
  }

};


/* hermitian (on the TODO list) */


} /* namespace mtl */
#endif


