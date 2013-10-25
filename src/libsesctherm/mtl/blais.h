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

#ifndef MTL_BLAIS_H
#define MTL_BLAIS_H

/*
 *  Basic Linear Algebra Instruction Set (BLAIS)
 */

#include <functional>
#include "mtl_iterator.h"

#include "mtl_config.h"
#include "matrix_traits.h"
#include "fast.h"
#include "scaled1D.h"

/*
 *  Level 1
 */

namespace blais_v {

  //: Set elements of vector x to alpha
  //!tparam: N - static length of x
  //!category: blais
  //!component: type
  template <int N>
  struct set {
    template <class Vector, class T> inline
    set(Vector x, const T& alpha)
    {
      fast::fill(x.begin(), fast::count<N>(), alpha);
    }
  };


} /* blais_v */

namespace blais_vv {

using namespace mtl;

//: Copy y <- x
// Copies vector x into vector y
//!tparam: N - the length of the vectors
//!category: blais
//!component: type
template <int N> struct copy {
template <class Vec1, class Vec2> inline
copy(Vec1 x, Vec2 y)
{
  fast::copy(x.begin(), fast::count<N>(), y.begin());
}
};


//: Add y <- x + y
//  This adds vector x into vector y.
//!example: blais_add.cc
//!tparam: N - the length of the vectors
//!category: blais
//!component: type
template <int N> struct add {
template <class Vec1, class Vec2> inline
add(Vec1 x, Vec2 y)
{
  typedef typename linalg_traits<Vec1>::value_type T;
  fast::transform(x.begin(), fast::count<N>(), y.begin(),
		  y.begin(), std::plus<T>());
}
};


//: Dot Product s <- x . y
//!tparam: N - the length of the vectors
//!category: blais
//!component: type
template <int N> struct dot {
template <class T, class Vec1, class Vec2> inline
dot(Vec1 x, Vec2 y, T& prod) {
  prod = fast::inner_product(x.begin(), fast::count<N>(), y.begin(), prod);
}
};



} /* blais_vv namespace */


namespace blais_m {

//: blah
//!noindex:
template <int M, int N>
struct __recur_set {
template <class TwoDIter, class T> inline
__recur_set(TwoDIter i, const T& alpha)
{
  blais_v::set<N>(*i, alpha);
  __recur_set<M-1, N>(++i, alpha);
}
};

//: blah
//!noindex:
template <int N>
struct __recur_set<0,N> {
template <class TwoDIter, class T> inline
__recur_set(TwoDIter, const T&) { }
};


//: Set matrix A to alpha
//
//!category: blais
//!component: type
template <int M, int N>
struct set {
  template <class Matrix, class T> inline
  set(Matrix A, const T& alpha) {
    __recur_set<M,N>(A.begin(), alpha);
  }
};


}

/* Matrix-Vector Algorithms */
namespace blais_mv {

using namespace mtl;

//: blah
//!noindex:
template <int M, int N, class Orien>
struct __mult { };


/* row major version (dot product) */

//: blah
//!noindex:
template <int M, int N>
struct __mult<M, N, row_tag> {
  template <class ARowIter, class VecX, class IterY> inline
  __mult(ARowIter A_2Diter, VecX x, IterY y) {
    typedef typename std::iterator_traits<IterY>::value_type T;
    blais_vv::dot<N>(*A_2Diter, x, *y);
    blais_mv::__mult<M-1, N, row_tag>(++A_2Diter, x, ++y);
  }
};

//: blah
//!noindex:
template <int N>
struct __mult<0, N, row_tag> {
  template <class AIter, class VecX, class IterY> inline
  __mult(AIter A_2Diter, VecX x, IterY y) {
    /* do nothing */
  }
};


/* column major version (axpy) */
//: blah
//!noindex:
template <int M, int N>
struct __mult<M, N, column_tag> {
  template <class AColIter, class IterX, class VecY> inline
  __mult(AColIter Aiter, IterX x, VecY y) {
    typedef typename AColIter::value_type OneD;
    // KCC choking on the LOO for this scaled1D object
    mtl::scaled1D<OneD> sa(*Aiter, *x);
    blais_vv::add<M>(sa, y);
    blais_mv::__mult<M, N-1, column_tag>(++Aiter, ++x, y);
  }
};

//: blah
//!noindex:
template <int M>
struct __mult<M, 0, column_tag> {
  template <class AColIter, class IterX, class VecY> inline
  __mult(AColIter A_2Diter, IterX x, VecY y) {
    /* do nothing */
  }
};


//:   Multiplication y <- A x + y
//!tparam: M - Number of rows in A
//!tparam: N - Number of columns in A
//!category: blais
//!component: type
template <int M, int N>
struct mult {
  template <class Matrix, class VecX, class VecY> inline
  mult(const Matrix& A, VecX x, VecY y) {
    typedef typename matrix_traits<Matrix>::orientation Orien;
    do_mult(A, x, y, Orien());
  }
  template <class Matrix, class VecX, class VecY> inline
  void do_mult(const Matrix& A, VecX x, VecY y, row_tag) {
    blais_mv::__mult<M, N, row_tag>(A.begin(), x, y.begin());
  }
  template <class Matrix, class VecX, class VecY> inline
  void do_mult(const Matrix& A, VecX x, VecY y, column_tag) {
    blais_mv::__mult<M, N, column_tag>(A.begin(), x.begin(), y);
    //    blais_mv::__mult<M, N, row_tag>(rows(A).begin(), x, y.begin());
  }
};


//: blah
//!noindex:
template <int M, int N, class Orien>
struct __rank_one { };

//: blah
//!noindex:
template <int M, int N>
struct __rank_one<M, N, row_tag> {
  template <class Row2Diter, class IterX, class VecY> inline
  __rank_one(Row2Diter Arow, IterX x, VecY y) {
    mtl::scaled1D<VecY> sy(y, *x);
    blais_vv::add<N>(sy, *Arow);
    __rank_one<M-1, N, row_tag>(++Arow, ++x, y);
  }
};

//: blah
//!noindex:
template <int N>
struct __rank_one<0, N, row_tag> {
  template <class Row2Diter, class IterX, class VecY> inline
  __rank_one(Row2Diter Arow, IterX x, VecY y) {
    /* do nothing */
  }
};



//: blah
//!noindex:
template <int M, int N>
struct __rank_one<M, N, column_tag> {
  template <class Col2Diter, class VecX, class IterY> inline
  __rank_one(Col2Diter Acol, VecX x, IterY y) {
    mtl::scaled1D<VecX> sx(x, *y);
    blais_vv::add<N>(sx, *Acol);
    __rank_one<M-1, N, column_tag>(++Acol, x, ++y);
  }
};

//: blah
//!noindex:
template <int M>
struct __rank_one<M, 0, column_tag> {
  template <class Col2Diter, class VecX, class IterY> inline
  __rank_one(Col2Diter Acol, VecX x, IterY y) {
    /* do nothing */
  }
};


//: Rank One Update A <- A +  x * y^T
//
//
//!tparam: M - Number of rows in A
//!tparam: N - Number of columns in A
//!category: blais
//!component: type
template <int M, int N>
struct rank_one {
  template <class Matrix, class VecX, class VecY> inline
  rank_one(Matrix& A, VecX x, VecY y) {
    typedef typename matrix_traits<Matrix>::orientation Orien;
    do_update(A, x, y, Orien());
  }
  template <class Matrix, class VecX, class VecY> inline
  void do_update(Matrix& A, VecX x, VecY y, row_tag) {
    __rank_one<M, N, row_tag>(A.begin(), x.begin(), y);
  }
  template <class Matrix, class VecX, class VecY> inline
  void do_update(Matrix& A, VecX x, VecY y, column_tag) {
    __rank_one<M, N, column_tag>(A.begin(), x, y.begin());
  }
};



} /* matvec namespace */



/*             Level 3             */
namespace blais_mm {

using namespace mtl;

//: blah
//!noindex:
template <int M, int N>
struct __copy {
  template <class IterA, class IterB> inline
  __copy(IterA Aiter, IterB Biter) {
    blais_vv::copy<N>(*Aiter, *Biter);
    blais_mm::__copy<M-1, N>(++Aiter, ++Biter);
  }
};

//: blah
//!noindex:
template <int N>
struct __copy<0, N> {
  template <class IterA, class IterB> inline
  __copy(IterA Aiter, IterB Biter) {
    /* do nothing */
  }
};



//: Copy  B <- A
//
//!tparam: M - Number of rows in A
//!tparam: N - Number of columns in A
//!category: blais
//!component: type
template <int M, int N>
struct copy {
  template <class MatrixA, class MatrixB> inline
  copy(const MatrixA& A, MatrixB& B) {
    typedef typename matrix_traits<MatrixA>::orientation Orien;
    do_copy(A, B, Orien());
  }
  template <class MatrixA, class MatrixB> inline
  void do_copy(const MatrixA& A, MatrixB& B, row_tag) {
    blais_mm::__copy<M, N>(A.begin(), rows(B).begin());
  }
  template <class MatrixA, class MatrixB> inline
  void do_copy(const MatrixA& A, MatrixB& B, column_tag) {
    blais_mm::__copy<N, M>(A.begin(), columns(B).begin());
  }
};


//: blah
//!noindex:
template <int M, int N, int K>
struct __mult {
  template <class MatrixA, class ColIterB, class ColIterC> inline
  __mult(const MatrixA& A, ColIterB Bcol, ColIterC Ccol) {
    blais_mv::mult<M,K>(A, *Bcol, *Ccol);
    blais_mm::__mult<M,N-1,K>(A, ++Bcol, ++Ccol);
  }
};

//: blah
//!noindex:
template <int M, int K>
struct __mult<M,0,K> {
  template <class MatrixA, class Col2DIterB, class Col2DIterC> inline
  __mult(const MatrixA& A, Col2DIterB Bcol, Col2DIterC Ccol) {
    /* do nothing */
  }
};


//: Multiplication   C <- A * B
//!tparam: M - Number of rows in A and C
//!tparam: N - Number of columns in B and rows in C
//!tparam: K - Number of columns in A and rows in B
//!category: blais
//!component: type
template <int M, int N, int K>
struct mult {
  template <class MatrixA, class MatrixB, class MatrixC> inline
  mult(const MatrixA& A, const MatrixB& B, MatrixC& C) {
    blais_mm::__mult<M,N,K>(A, columns(B).begin(), columns(C).begin());
  }
};



} /* matmat namespace */



#endif



#if 0
struct take1st { template <class T, class U>
  inline T operator()(const T& t, const U& u) { return t; }
};

template <int M, class IterY>
class add_op {
public:
  add_op(IterY& y_) : y(y_) { }
  template <class Col, class T>
  IterY operator()(const Col& a, const T& x) {
#ifdef __GNUC__ /* parse error :( */
    blais::blais_scaled_iter<typename Col::const_iterator> scl_a(a.begin(), x);
    blais_vv::add<M>(scl_a, y); return y;
#else
    blais_vv::add<M>(blais::scl(a.begin(),x), y); return y;
#endif
  }
protected:
  const IterY& y;
};
   /* wow, this is highly critical
 use reference to y to aid in small object optimization
 */
template <int M, int N>
struct __mult {
  template <class Matrix, class IterX, class IterY> inline
  __mult(const Matrix& A, IterX x, IterY y) {
    fast::inner_product(A.begin_columns(), fast::count<N>(), x, y,
			      take1st(), add_op<M,IterY>(y));
  }
};
#endif

#if 0

/*
  Multiply, Fixed M, Nonfixed N
  */
template <int M>
struct __mult_fixn {
  template <class TwoDIter, class IterX, class IterY> inline
  __mult_fixn(TwoDIter Arow, VecX& x, IterY& y) {
    *y = vecvec::dot((*Arow).begin(), x);
    __mult_fixn(++Arow, x, ++y);
  }
};

template <>
struct __mult_fixn<0> {
  template <class Matrix, class VecX, class VecY> inline
  __mult_fixn(const Matrix& A, VecX& x, VecY& y) {
    // do nothing
  }
};

template <int M>
struct mult_fixn {
  template <class TwoDIter, class IterX, class IterY> inline
  mult_fixn(const Matrix& A, VecX& x, VecY& y) {
    __mult_fixn(A.begin_rows(), x, y.begin());
  }
};

#endif
