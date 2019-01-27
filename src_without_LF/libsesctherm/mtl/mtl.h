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

#ifndef _MTL_MTL_H_
#define _MTL_MTL_H_

#include "mtl_decl.h"

#include <functional>
#include <iostream>
#include "mtl_limits.h"
#include "mtl_complex.h"

#include "fast.h"
#include "dense1D.h"
#include "mtl_exception.h"
#include "matrix_traits.h"
#include "transform_iterator.h"
#include "scaled1D.h"
#include "abs.h"

#ifdef USE_DOUBLE_DOUBLE
#include "contrib/double_double/double_double.h"
#endif

#if USE_BLAIS
#include "blais.h"
#endif

#include "matrix.h"

/*
  This is a nasty hack necessitated by several things:

  1. C++ does not allow temporaries to be passed
    into a reference argument.
  2. Many MTL expressions result in temporaries
  3. Some MTL matrix classes (static matrix) can
    not be passed by value for the output argument
    since they are not handles.
 */
// #define MTL_OUT(X) const X& // moved to mtl_decl.h

namespace mtl {

template <class T>
inline T sign(const T& x) { return (x < 0) ? T(-1) : T(1); }

template <class T>
inline T xfer_sign(const T& x, const T& y)
{
  return (y < 0) ? -MTL_ABS(x) : MTL_ABS(x);
}

//: for tri_solve and others
//!noindex:
class right_side { };

//: for tri_solve and others
//!noindex:
class left_side { };

#include "dim_calc.h"

template <class Vector> inline
typename linalg_traits<Vector>::value_type
sum__(const Vector& x, fast::count<0>)
{
  typedef typename linalg_traits<Vector>::value_type vt;
  return mtl_algo::accumulate(x.begin(), x.end(), vt());
}

#if USE_BLAIS
template <class Vector, int N> inline
typename linalg_traits<Vector>::value_type
sum__(const Vector& x, fast::count<N>)
{
  typedef typename linalg_traits<Vector>::value_type vt;
  return fast::accumulate(x.begin(), fast::count<N>(), vt());
}
#endif

//: Sum:  <tt>s <- sum_i(x(i))</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vec_sum.cc
//!complexity: linear
//!typereqs: The addition operator must be defined for <TT>Vector::value_type</TT>.
// The sum of all of the elements in the container.
template <class Vector> inline
typename linalg_traits<Vector>::value_type
sum(const Vector& x)
{
  return sum__(x, dim_n<Vector>::RET());
}

#include "mtl_set.h"

template <class S, class T, class R>
struct mtl_multiplies : std::binary_function<S, T, R> {
  typedef S first_argument_type;
  typedef T second_argument_type;
  typedef R result_type;
  R operator () (const S& x, const T& y) const { return x * y; }
};


template <class Vector, class T> inline
void
oned_scale(Vector& x, const T& alpha, fast::count<0>)
{
  typedef typename Vector::value_type VT;
  mtl_algo::transform(x.begin(), x.end(), x.begin(),
                      std::bind1st(mtl_multiplies<T,VT,VT>(), alpha));
}
#if USE_BLAIS
template <class Vector, class T, int N> inline
void
oned_scale(Vector& x, const T& alpha, fast::count<N>)
{
  typedef typename Vector::value_type VT;
  fast::transform(x.begin(), fast::count<N>(), x.begin(),
                  std::bind1st(mtl_multiplies<T,VT,VT>(), alpha));
}
#endif

template <class Vector, class T> inline
void
scale_dim(Vector& x, const T& alpha, oned_tag)
{
  oned_scale(x, alpha, typename dim_n<Vector>::RET());
}

template <class Matrix, class T>
inline void
scale_dim(Matrix& A, const T& alpha, twod_tag)
{
  typename Matrix::iterator i;
  typename Matrix::OneD::iterator j, jend;
  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      *j *= alpha;
  }
}


//: Scale:  <tt>A <- alpha*A or x <- alpha x</tt>
//
// Multiply all the elements in <tt>A</tt> (or <tt>x</tt>) by
// <tt>alpha</tt>.
//
//!category: algorithms
//!component: function
//!example: vec_scale_algo.cc
//!complexity: O(n)
//!definition: mtl.h
//!typereqs: <TT>Vector</TT> must be mutable
//!typereqs: <TT>T</TT> is convertible to <TT>Vector</TT>'s <TT>value_type</TT>
//!typereqs: The multiplication operator must be defined for <TT>Vector::value_type</TT> and <tt>T</tt>
template <class LinalgObj, class T>
inline void
scale(MTL_OUT(LinalgObj) A, const T& alpha)
{
  typedef typename linalg_traits<LinalgObj>::dimension Dim;
  scale_dim(const_cast<LinalgObj&>(A), alpha, Dim());
}



//: Set Diagonal:  <tt>A(i,i) <- alpha</tt>
//
// Set the value of the elements on the main diagonal of A to alpha.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: tri_pack_sol.cc
//!typereqs: <tt>T</tt> must be convertible to <tt>Matrix::value_type</tt>.
//!complexity: O(min(m,n)) for dense matrices, O(nnz) for sparse matrices (except envelope, which is O(m))
template <class Matrix, class T>
inline void
set_diagonal(MTL_OUT(Matrix) A_, const T& alpha)
{
  Matrix& A = const_cast<Matrix&>(A_);
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  if (! A.is_unit())
    for (Int i = 0; i < A.nrows() && i < A.ncols(); ++i)
      A(i,i) = alpha;
}


//: add absolute value
//!noindex:
struct abs_add {
  template <class T, class U>
  T operator()(const T& a, const U& b) {
    return a + MTL_ABS(b);
  }
};

template <class Vector>
inline typename linalg_traits<Vector>::magnitude_type
oned_one_norm(const Vector& x, fast::count<0>)
{
  typedef typename linalg_traits<Vector>::magnitude_type T;
  return mtl_algo::accumulate(x.begin(), x.end(), T(), abs_add());
}

#if USE_BLAIS
template <class Vector, int N>
inline typename linalg_traits<Vector>::magnitude_type
oned_one_norm(const Vector& x, fast::count<N>)
{
  typedef typename
     number_traits<typename Vector::value_type>::magnitude_type T;
  return fast::accumulate(x.begin(), fast::count<N>(), T(), abs_add());
}
#endif

template <class Vector>
inline typename linalg_traits<Vector>::magnitude_type
one_norm(const Vector& x, oned_tag)
{
  return oned_one_norm(x, typename dim_n<Vector>::RET());
}


//: add square
//!noindex:
struct sqr_add {
  template <class T, class U>
  T operator()(const T& a, const U& b) {
    return a + MTL_ABS(b * b);
  }
};

template <class Vector>
inline typename linalg_traits<Vector>::magnitude_type
oned_two_norm(const Vector& x, fast::count<0>)
{
  typedef typename Vector::value_type T;
  typedef typename number_traits<T>::magnitude_type M;
  using std::sqrt;
  return ::sqrt(mtl_algo::accumulate(x.begin(), x.end(), M(), sqr_add()));
}

#if USE_BLAIS
template <class Vector, int N>
inline typename linalg_traits<Vector>::magnitude_type
oned_two_norm(const Vector& x, fast::count<N>)
{
  typedef typename Vector::value_type T;
  typedef typename number_traits<T>::magnitude_type M;
  using std::sqrt;
  return ::sqrt(fast::accumulate(x.begin(), fast::count<N>(), M(), sqr_add()));
}
#endif

//: Two Norm: <tt>s <- sqrt(sum_i(|x(i)^2|))</tt>
//
//  The square root of the sum of the squares of the elements of the container.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vec_two_norm.cc
//!complexity: O(n)
//!typereqs: <tt>Vector</tt> must have an associated magnitude_type that is the type of the absolute value of <tt>Vector::value_type</tt>.
//!typereqs: There must be <tt>abs()</tt> defined for <tt>Vector::value_type</tt>.
//!typereqs: The addition must be defined for magnitude_type.
//!typereqs: <tt>sqrt()</tt> must be defined for magnitude_type.
template <class Vector>
inline typename linalg_traits<Vector>::magnitude_type
two_norm(const Vector& x)
{
  return oned_two_norm(x, typename dim_n<Vector>::RET());
}

//: add square
//!noindex:
struct sqr_ {
  template <class T, class U>
  T operator()(const T& a, const U& b) {
    return a + MTL_ABS(b * b);
  }
};


//: Sum of the Squares
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n)
template <class Vector>
inline typename linalg_traits<Vector>::value_type
sum_squares(const Vector& x)
{
  typedef typename linalg_traits<Vector>::value_type T;
  return mtl_algo::accumulate(x.begin(), x.end(), T(), sqr_add());
}



//: compare absolute values
//!noindex:
struct abs_cmp { template <class T>
bool operator()(const T& a, const T& b) {
  return MTL_ABS(a) < MTL_ABS(b);
}};


template <class Vec>
inline typename linalg_traits<Vec>::magnitude_type
infinity_norm(const Vec& x, oned_tag)
{
  return MTL_ABS(*mtl_algo::max_element(x.begin(), x.end(), abs_cmp()));
}



//: use by one and inf norm
//!noindex:
template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
major_norm__(const Matrix& A)
{
  typedef typename linalg_traits<Matrix>::magnitude_type T;
  typedef typename matrix_traits<Matrix>::size_type Int;
  T norm = 0;
  T sum = 0;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j;
  i = A.begin();

  /* get the first sum */
  if (i != A.end()) {
    j = (*i).begin();
    sum = T(0);
    for (; j != (*i).end(); ++j)
      sum = sum + MTL_ABS(*j);
    norm = sum;
    ++i;
  }

  for (; i != A.end(); ++i) {
    j = (*i).begin();
    if (A.is_unit() && Int(i.index()) < MTL_MIN(A.nrows(), A.ncols()))
      sum = T(1);
    else sum = T(0);

    for (; j != (*i).end(); ++j)
      sum = sum + MTL_ABS(*j);
    norm = MTL_MAX(MTL_ABS(norm), MTL_ABS(sum));
  }
  return norm;
}

//: used by one and inf norm
//!noindex:
template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
minor_norm__(const Matrix& A)
{
  typedef typename linalg_traits<Matrix>::magnitude_type T;
  typedef typename matrix_traits<Matrix>::size_type Int;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;

  dense1D<T> sums(A.minor(), T());
  if (A.is_unit()) {
    for (Int x = 0; x < MTL_MIN(A.nrows(), A.ncols()); ++x)
      sums[x] = T(1);
  }

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      sums[j.index()] += MTL_ABS(*j);
  }

  return infinity_norm(sums, oned_tag());
}


/* this handles both the major and minor norm
 for symmetric matrices */

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
symmetric_norm(const Matrix& A, row_tag)
{
  typedef typename linalg_traits<Matrix>::magnitude_type T;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;

  dense1D<T> sums(A.minor(), T(0));

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin();
    jend = (*i).end();
    if (A.is_upper()) { /* handle the diagonal elements */
      sums[j.row()] += MTL_ABS(*j);
      ++j;
    } else
      --jend;
    for (; j != jend; ++j) {
      sums[j.row()] += MTL_ABS(*j);
      sums[j.column()] += MTL_ABS(*j);
    }
    if (A.is_lower())
      sums[j.row()] += MTL_ABS(*j);
  }
  return infinity_norm(sums, oned_tag());
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
symmetric_norm(const Matrix& A, column_tag)
{
  typedef typename linalg_traits<Matrix>::magnitude_type T;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;

  dense1D<T> sums(A.minor(), T(0));

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin();
    jend = (*i).end();
    if (A.is_lower()) { /* handle the diagonal elements */
      sums[j.row()] += MTL_ABS(*j);
      ++j;
    } else
      --jend;
    for (; j != jend; ++j) {
      sums[j.row()] += MTL_ABS(*j);
      sums[j.column()] += MTL_ABS(*j);
    }
    if (A.is_upper())
      sums[j.row()] += MTL_ABS(*j);
  }
  return infinity_norm(sums, oned_tag());
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
symmetric_norm(const Matrix& A)
{
  typedef typename matrix_traits<Matrix>::orientation Orien;
  return symmetric_norm(A, Orien());
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
diagonal_one_norm(const Matrix& A)
{
  typedef typename linalg_traits<Matrix>::magnitude_type T;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;

  dense1D<T> sums(A.ncols(), T(0));

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      sums[j.column()] += MTL_ABS(*j);
  }

  return infinity_norm(sums);
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
diagonal_infinity_norm(const Matrix& A)
{
  typedef typename linalg_traits<Matrix>::magnitude_type T;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;

  dense1D<T> sums(A.nrows(), T(0));

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      sums[j.row()] += MTL_ABS(*j);
  }

  return infinity_norm(sums);
}



//: dispatch function
//!noindex:
template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
one_norm__(const Matrix& A, column_tag)
{
  return major_norm__(A);
}


//: dispatch function
//!noindex:
template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
one_norm__(const Matrix& A, row_tag)
{
  return minor_norm__(A);
}


template <class Matrix, class Shape>
inline typename linalg_traits<Matrix>::magnitude_type
twod_one_norm(const Matrix& A, Shape)
{
  typedef typename Matrix::orientation Orien;
  return one_norm__(A, Orien());
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
twod_one_norm(const Matrix& A, symmetric_tag)
{
  return symmetric_norm(A);
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
twod_one_norm(const Matrix& A, diagonal_tag)
{
  return diagonal_one_norm(A);
}


template <class Linalg>
inline typename linalg_traits<Linalg>::magnitude_type
one_norm(const Linalg& A, twod_tag)
{
  typedef typename matrix_traits<Linalg>::shape Shape;
  return twod_one_norm(A, Shape());
}

//: One Norm:  <tt>s <- sum(|x_i|) or s <- max_i(sum_j(|A(i,j)|))</tt>
//
// For vectors, the sum of the absolute values of the elements.
// For matrices, the maximum of the column sums.
// Note: not implemented yet for unit triangle matrices.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vec_one_norm.cc
//!complexity: O(n)
//!typereqs: The vector or matrix must have an associated magnitude_type that
//   is the type of the absolute value of its <tt>value_type</tt>.
//!typereqs: There must be <tt>abs()</tt> defined for <tt>Vector::value_type</tt>.
//!typereqs: The addition must be defined for magnitude_type.
template <class LinalgObj>
inline typename linalg_traits<LinalgObj>::magnitude_type
one_norm(const LinalgObj& A)
{
  typedef typename linalg_traits<LinalgObj>::dimension Dim;
  return one_norm(A, Dim());
}


//: dispatch function
//!noindex:
template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
infinity_norm__(const Matrix& A, row_tag)
{
  return major_norm__(A);
}

//: dispatch function
//!noindex:
template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
infinity_norm__(const Matrix& A, column_tag)
{
  return minor_norm__(A);
}

template <class Matrix, class Shape>
inline typename linalg_traits<Matrix>::magnitude_type
twod_infinity_norm(const Matrix& A, Shape)
{
  typedef typename Matrix::orientation Orien;
  return infinity_norm__(A, Orien());
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
twod_infinity_norm(const Matrix& A, symmetric_tag)
{
  return symmetric_norm(A);
}

template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
twod_infinity_norm(const Matrix& A, diagonal_tag)
{
  return diagonal_infinity_norm(A);
}


template <class Matrix>
inline typename linalg_traits<Matrix>::magnitude_type
infinity_norm(const Matrix& A, twod_tag)
{
  typedef typename matrix_traits<Matrix>::shape Shape;
  return twod_infinity_norm(A, Shape());
}


//: Infinity Norm: <tt>s <- max_j(sum_i(|A(i,j)|)) or s <- max_i(|x(i)|)</tt>
//
// For matrices, the maximum of the row sums.
// For vectors, the maximum absolute value of any of its element.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n) for vectors, O(m*n) for dense matrices, O(nnz) for sparse
//!example: vec_inf_norm.cc
//!typereqs: The vector or matrix must have an associated magnitude_type that is the type of the absolute value of its <tt>value_type</tt>.
//!typereqs: There must be <tt>abs()</tt> defined for <tt>Vector::value_type</tt>.
//!typereqs: The addition must be defined for magnitude_type.
template <class LinalgObj>
inline typename linalg_traits<LinalgObj>::magnitude_type
infinity_norm(const LinalgObj& A)
{
  typedef typename linalg_traits<LinalgObj>::dimension Dim;
  return infinity_norm(A, Dim());
}


//: Max Index:  <tt>i <- index of max(|x(i)|)</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n)
// The location (index) of the element with the maximum absolute value.
//!example: max_index.cc
//!typereqs: <tt>Vec::value_type</tt> must be LessThanComparible.
template <class Vec>
inline typename Vec::size_type
max_index(const Vec& x)
{
  typename Vec::const_iterator maxi =
    mtl_algo::max_element(x.begin(), x.end(), abs_cmp());
  return maxi.index();
}


//: Maximum Absolute Index:  <tt>i <- index of max(|x(i)|)</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n)
// The location (index) of the element with the maximum absolute value.
//!example: max_abs_index.cc
//!typereqs: The vector or matrix must have an associated magnitude_type that
//   is the type of the absolute value of its <tt>value_type</tt>.
//!typereqs: There must be <tt>abs()</tt> defined for <tt>Vector::value_type</tt>.
//!typereqs: The magnitude type must be LessThanComparible.
template <class Vec>
inline typename Vec::size_type
max_abs_index(const Vec& x)
{
  typename Vec::const_iterator maxi =
    mtl_algo::max_element(x.begin(), x.end(), abs_cmp());
  return maxi.index();
}


//: Minimum Index:  <tt>i <- index of min(x(i))</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n)
// The location (index) of the element with the minimum value.
//!example: min_abs_index.cc
//!typereqs: <tt>Vec::value_type</tt> must be LessThanComparible.
template<class Vec>
inline typename Vec::size_type
min_index(const Vec& x)
{
  typename Vec::const_iterator mini =
    mtl_algo::min_element(x.begin(), x.end());
  return mini.index();
}

//: Minimum Absolute Index:  <tt>i <- index of min(|x(i)|)</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n)
// The location (index) of the element with the minimum absolute value.
//!example: max_index.cc
//!typereqs: The vector or matrix must have an associated magnitude_type that
//   is the type of the absolute value of its <tt>value_type</tt>.
//!typereqs: There must be <tt>abs()</tt> defined for <tt>Vector::value_type</tt>.
//!typereqs: The magnitude type must be LessThanComparible.
template<class Vec>
inline typename Vec::size_type
min_abs_index(const Vec& x)
{
  typename Vec::const_iterator mini =
    mtl_algo::min_element(x.begin(), x.end(), abs_cmp());
  return mini.index();
}


//: Max Value:  <tt>s <- max(x(i))</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vec_max.cc
//!complexity: O(n)
//!typereqs: <tt>Vec::value_type</tt> must be LessThanComparible.
// Returns the value of the element with the maximum value
template <class VectorT>
inline typename VectorT::value_type
max(const VectorT& x)
{
  return *mtl_algo::max_element(x.begin(), x.end());
}



//: Min Value:  <tt>s <- min(x_i)</tt>
//!category: algorithms
//!component: function
//!complexity: O(n)
//!definition: mtl.h
//!typereqs: <tt>Vec::value_type</tt> must be LessThanComparible.
template <class VectorT>
inline typename VectorT::value_type
min(const VectorT& x)
{
  return *mtl_algo::min_element(x.begin(), x.end());
}

#define MTL_BLAS_GROT
//use blas version always, since there is a bug in the lapack verions
// of givens_rotation according to Andy's email

//: Givens Plane Rotation
//!category: functors
//!component: type
//!definition: mtl.h
//!example: apply_givens.cc
//
// Input a and b to the constructor to create a givens plane rotation
// object. Then apply the rotation to two vectors. There is a
// specialization of the givens rotation for complex numbers.
//
// <codeblock>
// [  c  s ] [ a ] = [ r ]
// [ -s  c ] [ b ]   [ 0 ]
// </codeblock>
//
//!typereqs: the addition operator must be defined for <tt>T</tt>
//!typereqs: the multiplication operator must be defined for <tt>T</tt>
//!typereqs: the division operator must be defined for <tt>T</tt>
//!typereqs: the abs() function must be defined for <tt>T</tt>
template <class T>
class givens_rotation {
public:

  //: Default constructor
  inline givens_rotation()
    :
#ifdef MTL_BLAS_GROT
    a_(0), b_(0),
#endif
    c_(0), s_(0)
#ifndef MTL_BLAS_GROT
    , r_(0)
#endif
  { }

  //: Givens Plane Rotation Constructor
  inline givens_rotation(T a_in, T b_in) {
#ifdef MTL_BLAS_GROT // old BLAS version
    T roe;
    if (MTL_ABS(a_in) > MTL_ABS(b_in))
      roe = a_in;
    else
      roe = b_in;

    T scal = MTL_ABS(a_in) + MTL_ABS(b_in);
    T r, z;
    if (scal != T(0)) {
      T a_scl = a_in / scal;
      T b_scl = b_in / scal;
      r = scal * sqrt(a_scl * a_scl + b_scl * b_scl);
      if (roe < T(0)) r *= -1;
      c_ = a_in / r;
      s_ = b_in / r;
      z = 1;
      if (MTL_ABS(a_in) > MTL_ABS(b_in))
        z = s_;
      else if (MTL_ABS(b_in) >= MTL_ABS(a_in) && c_ != T(0))
        z = T(1) / c_;
    } else {
      c_ = 1; s_ = 0; r = 0; z = 0;
    }
    a_ = r;
    b_ = z;
#else // similar LAPACK slartg version, modified to the NEW BLAS proposal
    T a = a_in, b = b_in;
    if (b == T(0)) {
      c_ = T(1);
      s_ = T(0);
      r_ = a;
    } else if (a == T(0)) {
      c_ = T(0);
      s_ = sign(b);
      r_ = b;
    } else {

      // cs = |a| / sqrt(|a|^2 + |b|^2)
      // sn = sign(a) * b / sqrt(|a|^2 + |b|^2)
      T abs_a = MTL_ABS(a);
      T abs_b = MTL_ABS(b);
      if (abs_a > abs_b) {
        // 1/cs = sqrt( 1 + |b|^2 / |a|^2 )
        T t = abs_b / abs_a;
        T tt = sqrt(T(1) + t * t);
        c_ = T(1) / tt;
        s_ = t * c_;
        r_ = a * tt;
      } else {
        // 1/sn = sign(a) * sqrt( 1 + |a|^2/|b|^2 )
        T t = abs_a / abs_b;
        T tt = sqrt(T(1) + t * t);
        s_ = sign(a) / tt;
        c_ = t * s_;
        r_ = b * tt;
      }
    }
#endif
  }

  inline void set_cs(T cin, T sin) { c_ = cin; s_ = sin; }

  //: Apply plane rotation to two real scalars. (name change a VC++ workaround)
  inline void scalar_apply(T& x, T& y) {
    T tmp = c_ * x + s_ * y;
    y = c_ * y - s_ * x;
    x = tmp;
  }

  //: Apply plane rotation to two vectors.
  template <class VecX, class VecY>
  inline void apply(MTL_OUT(VecX) x_, MTL_OUT(VecY) y_) MTL_THROW_ASSERTION {
    VecX& x = const_cast<VecX&>(x_);
    VecY& y = const_cast<VecY&>(y_);

    MTL_ASSERT(x.size() <= y.size(), "mtl::givens_rotation::apply()");

    typename VecX::iterator xi = x.begin();
    typename VecX::iterator xend = x.end();
    typename VecY::iterator yi = y.begin();

    while (mtl::not_at(xi, xend)) {
      scalar_apply(*xi, *yi);
      ++xi; ++yi;
    }
  }

#ifdef MTL_BLAS_GROT
  inline T a() { return a_; }
  inline T b() { return b_; }
#endif
  inline T c() { return c_; }
  inline T s() { return s_; }
#ifndef MTL_BLAS_GROT
  inline T r() { return r_; }
#endif
protected:
#ifdef MTL_BLAS_GROT
  T a_, b_;
#endif
  T c_, s_;
#ifndef MTL_BLAS_GROT
  T r_;
#endif
};

using std::real;
using std::imag;



#if MTL_PARTIAL_SPEC
//:  The specialization for complex numbers.
//!category: functors
//!component: type
template <class T>
class givens_rotation < std::complex<T> > {
  typedef std::complex<T> C;
public:
  //:
  inline givens_rotation() : cs(0), sn(0)
#ifndef MTL_BLAS_GROT
    , r_(0)
#endif
  { }

  inline T abs_sq(C t) { return real(t) * real(t) + imag(t) * imag(t); }
  inline T abs1(C t) { return MTL_ABS(real(t)) + MTL_ABS(imag(t)); }

  //:
  inline givens_rotation(C a_in, C b_in) {
#ifdef MTL_BLAS_GROT
    T a = std::abs(a_in), b = std::abs(b_in);
    if ( a == T(0) ) {
      cs = T(0);
      sn = C(1.);
      //in zrotg there is an assignment for ca, what is that for?
    } else {
      T scale = a + b;
      T norm = std::sqrt(abs_sq(a_in/scale)+abs_sq(b_in/scale)) * scale;

      cs = a / norm;
      sn = a_in/a * std::conj(b_in)/norm;
      //in zrotg there is an assignment for ca, what is that for?
    }
#else // LAPACK version, clartg
    C f(a_in), g(b_in);
    if (g == C(0)) {
      cs = T(1);
      sn = C(0);
      r_ = f;
    } else if (f == C(0)) {
      cs = T(0);
      sn = MTL_CONJ(g) / MTL_ABS(g);
      r_ = MTL_ABS(g);
    } else {
      C fs, gs, ss, t;
      T d, di, f1, f2, fa, g1, g2, ga;
      f1 = abs1(f);
      g1 = abs1(g);
      if (f1 >= g1) {
        gs = g / f1;
        g2 = abs_sq(gs);
        fs = f / f1;
        f2 = abs_sq(fs);
        d = sqrt(T(1) + g2 / f2);
        cs = T(1) / d;
        sn = MTL_CONJ(gs) * fs * (cs / f2);
        r_ = f * d;
      } else {
        fs = f / g1;
        f2 = abs_sq(fs);
        fa = sqrt(f2);
        gs = g / g1;
        g2 = abs_sq(gs);
        ga = sqrt(g2);
        d = sqrt(T(1) + f2 / g2);
        di = T(1) / d;
        cs = (fa / ga ) * di;
        ss = (MTL_CONJ(gs) * fs) / (fa * ga);
        sn = ss * di;
        r_ = g * ss * d;
      }
    }
#endif
  }
  //:  Apply plane rotation to two vectors.
  template <class VecX, class VecY>
  inline void apply(MTL_OUT(VecX) x_, MTL_OUT(VecY) y_) MTL_THROW_ASSERTION {
    VecX& x = const_cast<VecX&>(x_);
    VecY& y = const_cast<VecY&>(y_);

    MTL_ASSERT(x.size() <= y.size(), "mtl::givens_rotation::apply()");

    typename VecX::iterator xi = x.begin();
    typename VecX::iterator xend = x.end();
    typename VecY::iterator yi = y.begin();

    while (mtl::not_at(xi, xend)) {
      scalar_apply(*xi, *yi);
      ++xi; ++yi;
    }
  }
  //: Apply plane rotation to two complex scalars.
  inline void scalar_apply(C& x, C& y) {
    complex<T> temp  =  MTL_CONJ(cs) * x + MTL_CONJ(sn) * y;
    y = cs * y - sn * x;
    x = temp;
  }
  inline void set_cs(const T& cs_, const C& sn_) {
    cs = cs_; sn = sn_;
  }

  inline T c() { return cs; }
  inline C s() { return sn; }
#ifndef MTL_BLAS_GROT
  inline C r() { return r_; }
#endif

protected:
  T cs;
  C sn;
#ifndef MTL_BLAS_GROT
  C r_;
#endif
};

#else

//:  The specialization for complex numbers.
//!category: functors
//!component: type

class givens_rotation < std::complex<double> > {
  typedef double T;
  typedef std::complex<T> C;
public:
  //:
  inline givens_rotation() : cs(0), sn(0)
#ifndef MTL_BLAS_GROT
    , r_(0)
#endif
  { }
  inline T abs_sq(C t) { return real(t) * real(t) + imag(t) * imag(t); }
  inline T abs1(C t) { return MTL_ABS(real(t)) + MTL_ABS(imag(t)); }

  //:
  inline givens_rotation(C a_in, C b_in) {
#ifdef MTL_BLAS_GROT
    T a = std::abs(a_in), b = std::abs(b_in);
    if ( a == T(0) ) {
      cs = T(0);
      sn = C(1.);
      //in zrotg there is an assignment for ca, what is that for?
    } else {
      T scale = a + b;
      T norm = std::sqrt(abs_sq(a_in/scale)+abs_sq(b_in/scale)) * scale;

      cs = a / norm;
      sn = a_in/a * std::conj(b_in)/norm;
      //in zrotg there is an assignment for ca, what is that for?
    }
#else // LAPACK version, clartg
    C f(a_in), g(b_in);
    if (g == C(0)) {
      cs = T(1);
      sn = C(0);
      r_ = f;
    } else if (f == C(0)) {
      cs = T(0);
      sn = MTL_CONJ(g) / MTL_ABS(g);
      r_ = MTL_ABS(g);
    } else {
      C fs, gs, ss, t;
      T d, di, f1, f2, fa, g1, g2, ga;
      f1 = abs1(f);
      g1 = abs1(g);
      if (f1 >= g1) {
        gs = g / f1;
        g2 = abs_sq(gs);
        fs = f / f1;
        f2 = abs_sq(fs);
        d = sqrt(T(1) + g2 / f2);
        cs = T(1) / d;
        sn = MTL_CONJ(gs) * fs * (cs / f2);
        r_ = f * d;
      } else {
        fs = f / g1;
        f2 = abs_sq(fs);
        fa = sqrt(f2);
        gs = g / g1;
        g2 = abs_sq(gs);
        ga = sqrt(g2);
        d = sqrt(T(1) + f2 / g2);
        di = T(1) / d;
        cs = (fa / ga ) * di;
        ss = (MTL_CONJ(gs) * fs) / (fa * ga);
        sn = ss * di;
        r_ = g * ss * d;
      }
    }
#endif
  }
  //:  Apply plane rotation to two vectors.
  template <class VecX, class VecY>
  inline void apply(MTL_OUT(VecX) x_, MTL_OUT(VecY) y_) MTL_THROW_ASSERTION {
    VecX& x = const_cast<VecX&>(x_);
    VecY& y = const_cast<VecY&>(y_);

    MTL_ASSERT(x.size() <= y.size(), "mtl::givens_rotation::apply()");

    typename VecX::iterator xi = x.begin();
    typename VecX::iterator xend = x.end();
    typename VecY::iterator yi = y.begin();

    while (mtl::not_at(xi, xend)) {
      scalar_apply(*xi, *yi);
      ++xi; ++yi;
    }
  }
  //: Apply plane rotation to two complex scalars.
  inline void scalar_apply(C& x, C& y) {
    complex<T> temp  =  MTL_CONJ(cs) * x + MTL_CONJ(sn) * y;
    y = cs * y - sn * x;
    x = temp;
  }
  T c() { return cs; }
  C s() { return sn; }
#ifndef MTL_BLAS_GROT
  inline C r() { return r_; }
#endif
protected:
  T cs;
  C sn;
#ifndef MTL_BLAS_GROT
  C r_;
#endif
};

//:  The specialization for complex numbers.
//!category: functors
//!component: type
class givens_rotation < std::complex<float> > {
  typedef float T;
  typedef std::complex<T> C;
public:
  //:
  inline givens_rotation() : cs(0), sn(0)
#ifndef MTL_BLAS_GROT
    , r_(0)
#endif
  { }

  inline T abs_sq(C t) { return real(t) * real(t) + imag(t) * imag(t); }
  inline T abs1(C t) { return MTL_ABS(real(t)) + MTL_ABS(imag(t)); }

  //:
  inline givens_rotation(C a_in, C b_in) {
#ifdef MTL_BLAS_GROT
    T a = std::abs(a_in), b = std::abs(b_in);
    if ( a == T(0) ) {
      cs = T(0);
      sn = C(1.);
      //in zrotg there is an assignment for ca, what is that for?
    } else {
      T scale = a + b;
      T norm = std::sqrt(abs_sq(a_in/scale)+abs_sq(b_in/scale)) * scale;

      cs = a / norm;
      sn = a_in/a * std::conj(b_in)/norm;
      //in zrotg there is an assignment for ca, what is that for?
    }
#else // LAPACK version, clartg
    C f(a_in), g(b_in);
    if (g == C(0)) {
      cs = T(1);
      sn = C(0);
      r_ = f;
    } else if (f == C(0)) {
      cs = T(0);
      sn = MTL_CONJ(g) / MTL_ABS(g);
      r_ = MTL_ABS(g);
    } else {
      C fs, gs, ss, t;
      T d, di, f1, f2, fa, g1, g2, ga;
      f1 = abs1(f);
      g1 = abs1(g);
      if (f1 >= g1) {
        gs = g / f1;
        g2 = abs_sq(gs);
        fs = f / f1;
        f2 = abs_sq(fs);
        d = sqrt(T(1) + g2 / f2);
        cs = T(1) / d;
        sn = MTL_CONJ(gs) * fs * (cs / f2);
        r_ = f * d;
      } else {
        fs = f / g1;
        f2 = abs_sq(fs);
        fa = sqrt(f2);
        gs = g / g1;
        g2 = abs_sq(gs);
        ga = sqrt(g2);
        d = sqrt(T(1) + f2 / g2);
        di = T(1) / d;
        cs = (fa / ga ) * di;
        ss = (MTL_CONJ(gs) * fs) / (fa * ga);
        sn = ss * di;
        r_ = g * ss * d;
      }
    }
#endif
  }
  //:  Apply plane rotation to two vectors.
  template <class VecX, class VecY>
  inline void apply(MTL_OUT(VecX) x_, MTL_OUT(VecY) y_) MTL_THROW_ASSERTION {
    VecX& x = const_cast<VecX&>(x_);
    VecY& y = const_cast<VecY&>(y_);

    MTL_ASSERT(x.size() <= y.size(), "mtl::givens_rotation::apply()");

    typename VecX::iterator xi = x.begin();
    typename VecX::iterator xend = x.end();
    typename VecY::iterator yi = y.begin();

    while (mtl::not_at(xi, xend)) {
      scalar_apply(*xi, *yi);
      ++xi; ++yi;
    }
  }
  //: Apply plane rotation to two complex scalars.
  inline void scalar_apply(C& x, C& y) {
    complex<T> temp  =  MTL_CONJ(cs) * x + MTL_CONJ(sn) * y;
    y = cs * y - sn * x;
    x = temp;
  }
  T c() { return cs; }
  C s() { return sn; }
#ifndef MTL_BLAS_GROT
  inline C r() { return r_; }
#endif

protected:
  T cs;
  C sn;
#ifndef MTL_BLAS_GROT
  C r_;
#endif
};
#endif


#undef MTL_BLAS_GROT
//do allow internal macro escape out of the scope

//: Modified Givens Transformation
//!category: functors
//!component: type
//
//  This class is under construction.  Like the givens rotation class,
//  there will be a real and complex class.
template <class T>
class modified_givens {

};


template <class T>
inline T two_norm3(const T& x, const T& y, const T& z) {
  return sqrt(x*x + y*y + z*z);
}

//: Generate Householder Transform
//
// Ok to alias x and v to the same vector.
// T can be real or complex.
// Equivalent to LAPACK's xLARFG
template <class T, class Vec>
inline void generate_householder(T& alpha, const Vec& x,
                                 Vec& v, T& tau) MTL_THROW_ASSERTION
{
  MTL_ASSERT(x.size() == v.size(), "mtl::generate_householder");
  typedef typename number_traits<T>::magnitude_type Real;
  typename Vec::subrange_type subx = x(0, x.size() - 1);
  typename Vec::subrange_type subv = v(0, v.size() - 1);
  v[v.size() - 1] = x[x.size() - 1];
  Real xnorm = two_norm(x);
  Real alpha_r = real(alpha);
  Real alpha_i = imag(alpha);

  if (xnorm == Real(0) && alpha_i == Real(0))
    tau = T(0); // H = I
  else {
    Real beta = -xfer_sign(two_norm3(alpha_r, alpha_i, xnorm), alpha_r);
    Real safe_min = std::numeric_limits<Real>::min();
    Real r_safe_min = Real(1) / safe_min;

    int count = 0;
    while (MTL_ABS(beta) < safe_min) { // xnorm and beta may be inaccurate
      if (count == 0)                  // so scale x and recompute them
        copy(mtl::scaled(subx, r_safe_min), subv);
      else
        scale(subv, r_safe_min);
      beta *= r_safe_min;
      alpha *= r_safe_min;
      ++count;
    }
    if (count != 0) {
      alpha_r = real(alpha);
      alpha_i = imag(alpha);
      xnorm = two_norm(x);
      beta = -xfer_sign(two_norm3(alpha_r, alpha_i, xnorm), alpha_r);
    }
    tau = beta - (alpha / beta);
    alpha = T(1) / (alpha - beta);
    scale(subv, alpha);
    alpha = beta;
    for (int j = 0; j < count; ++j)
      alpha *= safe_min;
  }
}



//: Householder Transform
//
// Constructor does the generation, then call apply
//
template <class T>
class householder_transform {
  typedef typename number_traits<T>::magnitude_type Real;
  typedef dense1D<T> Vec;
public:
  template <class Vec>
  inline householder_transform(const T& alpha, Vec& x, const T& tau)
    : _alpha(alpha), _v(x.size()), _tau(tau) {
    generate_householder(_alpha, x, _v, _tau);
  }

#if 0
  // JGS conj ???
  // Equivalent to LAPACK xLARF
  template <class MatrixC>
  inline void apply(MatrixC& C, right_side) {
    if (_tau != T(0)) {
      Vec w(C.nrows());
      mult(C, _v, w);		       // w <- C * v
      rank_one_update(mtl::caled(w, -_tau),// C <- C - w * v'
		      conj(v), C);
    }
  }
  template <class MatrixC>
  inline void apply(MatrixC& C, left_side) {
    if (_tau != T(0)) {
      Vec w(C.nrows());
      mult(conj(C), _v, w);	       // w <- C' * v
      rank_one_update(mtl::scaled(w,-_tau), // C <- C - w * v'
		      conj(v), C);
    }
  }
#endif
protected:
  T _alpha;
  Vec _v;
  T _tau;
};



//: Transpose in Place:  <tt>A <- A^T</tt>
// Currently this algorithm only applies to square dense matrices
// Plan to include all rectangular dense matrices..
//!category: algorithms
//!component: function
//!definition: mtl.h
template <class Matrix>
inline void
transpose(MTL_OUT(Matrix) A_) MTL_THROW_ASSERTION
{
  Matrix& A = const_cast<Matrix&>(A_);
  MTL_ASSERT(A.nrows() == A.ncols(), "mat::transpose()");
  typedef typename matrix_traits<Matrix>::value_type T;
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  for (Int i = 0; i < A.nrows(); ++i)
    for (Int j = i; j < A.ncols(); ++j) {
      T tmp = A(i, j);
      A(i, j) = A(j, i);
      A(j, i) = tmp;
    }
}


//: Transpose: <tt>B <- A^T</tt>
//!precond:  <tt> B(i,j) = 0 & B = A^T </tt>
//
//  When matrix B is banded, it is up to the user to ensure
//  that the bandwidth is sufficient to contain the elements
//  from A^T. If there are elements of A^T that do not
//  fall within the bandwidth, an exception will be thrown.
//  (exception not implemented yet).
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n^2)
template <class MatA, class MatB>
inline void
transpose(const MatA& A, MTL_OUT(MatB) B_) MTL_THROW_ASSERTION
{
  MatB& B = const_cast<MatB&>(B_);
  MTL_ASSERT(A.nrows() <= B.ncols(), "matmat::transpose()");
  MTL_ASSERT(A.ncols() <= B.nrows(), "matmat::transpose()");

  typename MatA::const_iterator i;
  typename MatA::OneD::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      B(j.column(), j.row()) = *j;
  }
}


/*
  This version of the algorithm depends on the compiler
  hoisting the reference of z[j.row()] out of the inner loop
  (for the row major case)
  KCC doesn't do this, and niether does the underlying Sun C
  compiler.

  In order to hoist the reference by hand, I'll have to write
  specializations for column major matrix and for row major matrices.
  While I'm at it I'll the the unrolling stuff too.

*/

/* this is generic
 */
template <class Matrix, class VecX, class VecZ>
inline void
mult_generic__(const Matrix& A, const VecX& xx, VecZ& zz) MTL_THROW_ASSERTION
{
  MTL_ASSERT(A.nrows() <= zz.size(), "mtl::mult()");
  MTL_ASSERT(A.ncols() <= xx.size(), "mtl::mult()");
  typedef typename matrix_traits<Matrix>::value_type T;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;
  typename VecX::const_iterator x = xx.begin();
  typename VecZ::iterator z = zz.begin();

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      z[j.row()] += *j * x[j.column()];
  }
}

template <class Matrix, class VecX, class VecZ>
inline void
mult_shape__(const Matrix& A, const VecX& x, VecZ& z,
             banded_tag) MTL_THROW_ASSERTION
{
  mult_generic__(A, x, z);
}

/* this is fast
 */
template <class Matrix, class VecX, class VecZ>
inline void
rect_mult(const Matrix& A, const VecX& xx, VecZ& zz,
          row_tag, dense_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(A.nrows() <= zz.size(), "mtl::mult()");
  MTL_ASSERT(A.ncols() <= xx.size(), "mtl::mult()");
  typedef typename matrix_traits<Matrix>::value_type T;
  typename Matrix::const_iterator i, iend;
  typename Matrix::OneD::const_iterator j, jend;
  typename VecX::const_iterator x = xx.begin();
  typename VecZ::iterator z = zz.begin();

  i = A.begin();
  iend = A.end();
  for (; i != iend; ++i) {
    j = (*i).begin(); jend = (*i).end();
    T tmp = z[j.row()];
    for (; j != jend; ++j)
      tmp += *j * x[j.column()];
    z[j.row()] = tmp;
  }
}


/*
  This is slow

 */
template <class Matrix, class VecX, class VecZ>
inline void
rect_mult(const Matrix& A, const VecX& xx, VecZ& zz,
          column_tag, dense_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(A.nrows() <= zz.size(), "mtl::mult()");
  MTL_ASSERT(A.ncols() <= xx.size(), "mtl::mult()");
  typedef typename matrix_traits<Matrix>::value_type T;
  typedef typename matrix_traits<Matrix>::size_type Int;
  typename VecX::const_iterator x = xx.begin();
  typename VecZ::iterator z = zz.begin();

  typename Matrix::const_iterator i, iend;
  typename Matrix::OneD::const_iterator j, jend;
  i = A.begin();
  iend = A.end();
  for (; i != iend; ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      z[j.row()] += *j * x[j.column()];
  }
}

// x is sparse
// A is column oriented
template <class Matrix, class VecX, class VecY>
void
rect_mult(const Matrix& A, const VecX& x, VecY& y,
	  column_tag, sparse_tag)
{
  typename VecX::const_iterator xi = x.begin();
  for (; xi != x.end(); ++xi) {
    mtl::add(mtl::scaled(A[xi.index()], *xi), y);
  }
}

// x is sparse
// A is row oriented
template <class Matrix, class VecX, class VecY>
void
rect_mult(const Matrix& A, const VecX& x, VecY& y,
	  row_tag, sparse_tag)
{
  typename Matrix::const_iterator Ai;
  for (Ai = A.begin(); Ai != A.end(); ++Ai) {
    y[Ai.index()] = mtl::dot(*Ai, x); // this is a sparse dot
  }
}


template <class Matrix, class VecX, class VecZ>
inline void
mult_shape__(const Matrix& A, const VecX& x, VecZ z,
             rectangle_tag) MTL_THROW_ASSERTION
{
  typedef typename matrix_traits<Matrix>::orientation Orien;
  typedef typename linalg_traits<VecX>::sparsity SparseX;
  rect_mult(A, x, z, Orien(), SparseX());
}

template <class Matrix, class VecX, class VecZ>
inline void
mult_shape__(const Matrix& A, const VecX& x, VecZ& z,
             triangle_tag)
{
  mult_shape__(A, x, z, rectangle_tag());
  if (A.is_unit()) {
    /* actually, this still isn't quite right,
       should do
       add_n(x, z, z, MTL_MIN(A.nrows(), A.ncols()));
       instead
       */
    if (z.size() <= x.size())
      mtl::add(z, x, z);
    else
      mtl::add(x, z, z);
  }
}

template <class Matrix, class VecX, class VecZ>
inline void
mult_symm__(const Matrix& A, const VecX& x, VecZ& z, row_tag)
{
  typedef typename matrix_traits<Matrix>::value_type T;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    T tmp = z[i.index()];
    j = (*i).begin();
    jend = (*i).end();
    if (A.is_upper()) {
      tmp += *j * x[j.column()];
      ++j;
    } else
      --jend;
    for (; j != jend; ++j) {
      /* normal side */
      tmp += *j * x[j.column()];
      /* symmetric side */
      z[j.column()] += *j * x[j.row()];
    }
    if (A.is_lower())
      tmp += *j * x[j.column()];
    z[i.index()] = tmp;
  }
}

template <class Matrix, class VecX, class VecZ>
inline void
mult_symm__(const Matrix& A, const VecX& x, VecZ& z, column_tag)
{
  typedef typename matrix_traits<Matrix>::value_type T;
  typename Matrix::const_iterator i;
  typename Matrix::OneD::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    T tmp = T(0);
    j = (*i).begin();
    jend = (*i).end();
    if (A.is_lower()) {
      z[j.column()] += *j * x[j.column()];
      ++j;
    } else
      --jend;
    for (; j != jend; ++j) {
      /* normal side */
      z[j.row()] += *j * x[j.column()];
      /* symmetric side */
      tmp += *j * x[j.row()];
    }
    if (A.is_upper())
      tmp += *j * x[j.row()];
    z[i.index()] += tmp;
  }
}


template <class Matrix, class VecX, class VecZ>
inline void
mult_shape__(const Matrix& A, const VecX& x, VecZ& z,
             symmetric_tag)
{
  typedef typename matrix_traits<Matrix>::orientation Orien;
  mult_symm__(A, x, z, Orien());
}

//: Multiplication:  <tt>z <- A x + y</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!precond:  <TT>A.nrows() <= y.size()</TT>
//!precond:  <TT>A.nrows() <= z.size()</TT>
//!precond:  <TT>A.ncols() <= x.size()</TT>
//!precond:  no aliasing in the arguments
//!example: symm_sparse_vec_prod.cc
//!typereqs: <tt>Matrix::value_type</tt>, <tt>VecX::value_type</tt>, <tt>VecY::value_type</tt>, and <tt>VecZ::value_type</tt> must be the same type
//!typereqs: the multiplication operator must be defined for <tt>Matrix::value_type</tt>
//!typereqs: the addition operator must be defined for <tt>Matrix::value_type</tt>
template <class Matrix, class VecX, class VecY, class VecZ>
inline void
mult(const Matrix& A, const VecX& x, const VecY& y, MTL_OUT(VecZ) z_)
  MTL_THROW_ASSERTION
{
  VecZ& z = const_cast<VecZ&>(z_);
  mtl::copy(y, z);
  typedef typename matrix_traits<Matrix>::shape Shape;
  mult_shape__(A, x, z, Shape());
}


//: Matrix Vector Multiplication:  <tt>y <- A x</tt>
//
// Multiplies matrix A times vector x and stores the result in vector y.
// <p>
// Note: ignore the <tt>oned_tag</tt> parameter and the underscores in
// the name of this function.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: general_matvec_mult.cc, banded_matvec_mult.cc, symm_matvec_mult.cc
//!precond:  <TT>A.nrows() <= y.size()</TT>
//!precond:  <TT>A.ncols() <= x.size()</TT>
//!precond:  x and y not same vector
//!example: symm_matvec_mult.cc
//!typereqs: <tt>Matrix::value_type</tt>, <tt>VecX::value_type</tt>, and <tt>VecY::value_type</tt> must be the same type
//!typereqs: the multiplication operator must be defined for <tt>Matrix::value_type</tt>
//!typereqs: the addition operator must be defined for <tt>Matrix::value_type</tt>
template <class Matrix, class VecX, class VecY>
inline void
mult_dim__(const Matrix& A, const VecX& x, VecY& y, oned_tag) MTL_THROW_ASSERTION
{
  mtl::mult(A, x, mtl::scaled(y, 0), y);
#if 0
  typedef typename matrix_traits<Matrix>::shape Shape;
  mult_shape__(A, x, y, Shape());
#endif
}

template <class Matrix, class VecX, class VecY>
inline void
mult_add(const Matrix& A, const VecX& x, MTL_OUT(VecY) y_) MTL_THROW_ASSERTION
{
  VecY& y = const_cast<VecY&>(y_);
  typedef typename matrix_traits<Matrix>::shape Shape;
  mult_shape__(A, x, y, Shape());
}


//: simple 3 loop version of matmat mult
//!noindex:
template <class MatA, class MatB, class MatC, class Orien>
inline void
simple_mult(const MatA& A, const MatB& B, MatC& C, dense_tag, Orien)
{
  typedef typename matrix_traits<MatA>::size_type Int;
  typename MatA::const_iterator A_k;
  typename MatA::OneD::const_iterator A_ki;

  A_k = A.begin();
  while (not_at(A_k, A.end())) {
    for (Int j = 0; j < B.ncols(); ++j) {
      A_ki = (*A_k).begin();
      while (not_at(A_ki, (*A_k).end())) {
        Int k = A_ki.column();
        Int i = A_ki.row();
        C(i,j) += *A_ki * B(k,j);
        ++A_ki;
      }
    }
    ++A_k;
  }
}

/* Assumes A and B are also row oriented */
template <class MatrixA, class MatrixB, class MatrixC>
inline void
simple_mult(const MatrixA& A, const MatrixB& B, MatrixC& C,
            sparse_tag, row_tag)
{
  typedef typename matrix_traits<MatrixA>::value_type T;
  typedef typename matrix_traits<MatrixA>::size_type Int;
  T scal;
  Int len = 0;
  Int jj, k;
  Int nzmax = C.capacity();

  Int M = A.nrows();
  Int N = B.ncols();

  dense1D<Int> ic(M + 1, 0);
  dense1D<Int> jc(nzmax);
  dense1D<T> c(nzmax);

  typedef typename dense1D<Int>::iterator di_iter;
  typedef typename dense1D<T>::iterator dt_iter;

  compressed1D<T> tmp1(N), tmp2(N), tmp3(N);
  tmp1.reserve(N);
  tmp2.reserve(N);

  typedef typename compressed1D<T>::iterator tmpiter;

  typename MatrixA::const_iterator Ai;
  typename MatrixA::Row::const_iterator Aij;

  for (Ai = A.begin(); Ai != A.end(); ++Ai) {

    copy(C[Ai.index()], tmp1);

    for (Aij = (*Ai).begin(); Aij != (*Ai).end(); ++Aij) {
      scal = *Aij;
      jj = Aij.column();
      // add B[jj] and tmp1 into tmp2
      add(mtl::scaled(B[jj], scal), tmp1, tmp2);
      tmp1.clear();
      // swap tmp1 and tmp2
      tmp3 = tmp1; tmp1 = tmp2; tmp2 = tmp3;
    }
    // copy tmp1 into C[ii]
    k = len;
    if (k + tmp1.nnz() > nzmax) {
      std::cerr << "Not enough work space, increase capacity of C" << std::endl;
      return;
    }
    for (tmpiter t = tmp1.begin(); t != tmp1.end(); ++t, ++k) {
      c[k] = *t;
      jc[k] = t.index();
    }

    len += tmp1.nnz();
    ic[Ai.index() + 1] = len;
  }
  typedef typename matrix<T, rectangle<>,
    compressed<Int, external>,
    row_major>::type  SpMat;
  SpMat CC(M, N, len, c.data(), ic.data(), jc.data());
  copy(CC, C);
}

/* Assumes A and B are also column oriented */
template <class MatrixA, class MatrixB, class MatrixC>
inline void
simple_mult(const MatrixA& A, const MatrixB& B, MatrixC& C,
            sparse_tag, column_tag)
{
  typedef typename matrix_traits<MatrixA>::value_type T;
  typedef typename matrix_traits<MatrixA>::size_type Int;
  T scal;
  Int len = 0;
  Int kk, k;
  Int nzmax = C.capacity();

  Int M = A.nrows();
  Int N = B.ncols();

  dense1D<Int> ic(N + 1, 0);
  dense1D<Int> jc(nzmax);
  dense1D<T> c(nzmax);

  typedef typename dense1D<Int>::iterator di_iter;
  typedef typename dense1D<T>::iterator dt_iter;

  compressed1D<T> tmp1(M), tmp2(M), tmp3(M);
  tmp1.reserve(M);
  tmp2.reserve(M);

  typedef typename compressed1D<T>::iterator tmpiter;

  typename MatrixB::const_iterator Bj;
  typename MatrixB::Column::const_iterator Bjk;

  for (Bj = B.begin(); Bj != B.end(); ++Bj) {

    copy(C[Bj.index()], tmp1);

    for (Bjk = (*Bj).begin(); Bjk != (*Bj).end(); ++Bjk) {
      scal = *Bjk;
      kk = Bjk.row();
      // add A[kk] and tmp1 into tmp2
      add(mtl::scaled(A[kk], scal), tmp1, tmp2);
      tmp1.clear();
      // swap tmp1 and tmp2
      tmp3 = tmp1; tmp1 = tmp2; tmp2 = tmp3;
    }
    // copy tmp1 into C[ii]
    k = len;
    if (k + tmp1.nnz() > nzmax) {
      std::cerr << "Not enough work space, increase capacity of C" << std::endl;
      return;
    }
    for (tmpiter t = tmp1.begin(); t != tmp1.end(); ++t, ++k) {
      c[k] = *t;
      jc[k] = t.index();
    }

    len += tmp1.nnz();
    ic[Bj.index() + 1] = len;
  }

  typedef typename matrix<T, rectangle<>,
                 compressed<Int, external>,
                 column_major>::type  SpMat;
  SpMat CC(M, N, len, c.data(), ic.data(), jc.data());
  copy(CC, C);
}


//: Symmetric version, row-major
//!noindex:
template <class MatA, class MatB, class MatC>
inline void
symm_simple_mult(const MatA& A, const MatB& B, MatC& C, row_tag)
{
  typedef typename matrix_traits<MatA>::size_type Int;
  typename MatA::const_iterator A_k;
  typename MatA::OneD::const_iterator A_ki, A_kiend;

  A_k = A.begin();
  while (not_at(A_k, A.end())) {
    for (Int j = 0; j < B.ncols(); ++j) {
      A_ki = (*A_k).begin();
      A_kiend = (*A_k).end();

      Int k = A_ki.column();
      Int i = A_ki.row();

      if (A.is_upper()) { /* handle the diagonal elements */
        C(i,j) += *A_ki * B(k,j);
        ++A_ki;
      } else
        --A_kiend;

      while (not_at(A_ki, A_kiend)) {
        k = A_ki.column();
        i = A_ki.row();
        C(i,j) += *A_ki * B(k,j);
        C(k,j) += *A_ki * B(i,j);
        ++A_ki;
      }
      k = A_ki.column();
      i = A_ki.row();
      if (A.is_lower())
        C(i,j) += *A_ki * B(k,j);

    }
    ++A_k;
  }
}

//: Symmetric version, column-major
//!noindex:
template <class MatA, class MatB, class MatC>
inline void
symm_simple_mult(const MatA& A, const MatB& B, MatC& C, column_tag)
{
  typedef typename matrix_traits<MatA>::size_type Int;
  typename MatA::const_iterator A_k;
  typename MatA::OneD::const_iterator A_ki, A_kiend;

  A_k = A.begin();
  while (not_at(A_k, A.end())) {
    for (Int j = 0; j < B.ncols(); ++j) {
      A_ki = (*A_k).begin();
      A_kiend = (*A_k).end();

      Int k = A_ki.column();
      Int i = A_ki.row();

      if (A.is_lower()) { /* handle the diagonal elements */
        C(i,j) += *A_ki * B(k,j);
        ++A_ki;
      } else
        --A_kiend;

      while (not_at(A_ki, A_kiend)) {
        k = A_ki.column();
        i = A_ki.row();
        C(i,j) += *A_ki * B(k,j);
        C(k,j) += *A_ki * B(i,j);
        ++A_ki;
      }
      k = A_ki.column();
      i = A_ki.row();
      if (A.is_upper())
        C(i,j) += *A_ki * B(k,j);
    }

    ++A_k;
  }
}


//: Specialization for triangular matrices
//!noindex:
template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, symmetric_tag)
{
  typedef typename matrix_traits<MatA>::orientation Orien;
  symm_simple_mult(A, B, C, Orien());
}

//: Specialization for triangular matrices
//!noindex
template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, triangle_tag)
{
  typedef typename matrix_traits<MatA>::size_type Int;
  typedef typename matrix_traits<MatA>::orientation Orien;
  if (A.is_unit()) {
    Int M = MTL_MIN(A.nrows(), A.ncols());
    Int N = B.ncols();
    for (Int i = 0; i < M; ++i)
      for (Int j = 0; j < N; ++j)
        C(i,j) += B(i,j);
  }

  simple_mult(A, B, C, mtl::dense_tag(), Orien());
}

//: Dispatch to row/column general and banded matrices
//!noindex:
template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, rectangle_tag)
{
  typedef typename matrix_traits<MatA>::sparsity Sparsity;
  typedef typename matrix_traits<MatA>::orientation Orien;
  simple_mult(A, B, C, Sparsity(), Orien());
}

template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, banded_tag)
{
  typedef typename matrix_traits<MatC>::sparsity Sparsity;
  typedef typename matrix_traits<MatA>::orientation Orien;
  simple_mult(A, B, C, Sparsity(), Orien());
}


//: Matrix multiplication  C <- C + A * B
//
//  The actual specialization of the algorithm used depends of the
//  types of matrices used. If all the matrices are dense and
//  rectangular the blocked algorithm is used (when --with-blais is
//  specified in the configure). Otherwise the traversal depends on
//  matrix A. Therefore if one is multiplying a sparse matrix by a
//  dense, one would want the sparse matrix as the A
//  argument. Typically, for performance reasons, one would not want
//  to use a sparse matrix for C.
//  <p>
//  Note: ignore the <tt>twod_tag</tt> argument and the underscores in
//  the name of this function.
//
//!precond: <tt>A.nrows() == C.nrows()</tt>
//!precond: <tt>A.ncols() == B.nrows()</tt>
//!precond: <tt>B.ncols() == C.ncols()</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!typereqs: the value types for each of the matrices must be compatible
//!typereqs: the multiplication operator must be defined for <tt>MatA::value_type</tt>
//!typereqs: the addition operator must be defined for <tt>MatA::value_type</tt>
template <class MatA, class MatB, class MatC>
inline void
mult_dim__(const MatA& A, const MatB& B, MatC& C, twod_tag)
{
  typedef typename MatA::shape Shape;
  matmat_mult(A, B, C, Shape());
}


//: Dispatch between matrix matrix and matrix vector mult.
//!noindex:
template <class LinalgA, class LinalgB, class LinalgC>
inline void
mult(const LinalgA& A, const LinalgB& B, MTL_OUT(LinalgC) C_)
{
  LinalgC& C = const_cast<LinalgC&>(C_);
  typedef typename linalg_traits<LinalgB>::dimension Dim;
  mult_dim__(A, B, C, Dim());
}

//: for column oriented
//!noindex:
template <class TriMatrix, class VecX>
inline void
tri_solve__(const TriMatrix& T, VecX& x, column_tag)
{
  typedef typename matrix_traits<TriMatrix>::size_type Int;
  typedef typename matrix_traits<TriMatrix>::value_type VT;
  typename VecX::value_type x_j;

  if (T.is_upper()) {
    typename TriMatrix::const_reverse_iterator T_j;
    typename TriMatrix::Column::const_reverse_iterator T_ji, T_jrend;

    for (T_j = T.rbegin(); T_j != T.rend(); ++T_j) {
      T_ji = (*T_j).rbegin();
      T_jrend = (*T_j).rend();
      //Int j = T_ji.column();
      Int j = T_j.index();

      //Paul C. Leopardi <leopardi@bigpond.net.au> reported the fix
      //for for a sparse matrix to have a completely empty row (or column)
      if ( (T_ji != T_jrend) && ! T.is_unit()) {
        x[j] /= *T_ji; /* the diagonal */
        ++T_ji;
      }
      x_j = x[j];

      while (T_ji != T_jrend) {
        Int i = T_ji.row();
        x[i] -= x_j * *T_ji;
        ++T_ji;
      }
    }
  } else {                      /* T is lower */
    typename TriMatrix::const_iterator T_j;
    typename TriMatrix::Column::const_iterator T_ji, T_jend;

    for (T_j = T.begin(); T_j != T.end(); ++T_j) {
      T_ji = (*T_j).begin();
      T_jend = (*T_j).end();
      //Int j = T_ji.column(); //T_ji could be T_jend
      Int j = T_j.index();

      if ( (T_ji != T_jend) && ! T.is_unit()) {
        x[j] /= *T_ji; /* the diagonal */
        ++T_ji;
      }
      x_j = x[j];

      while (T_ji != T_jend) {
        Int i = T_ji.row();
        x[i] -= x_j * *T_ji;
        ++T_ji;
      }
    }
  }
}

//: for row major
//!noindex:
template <class TriMatrix, class VecX>
inline void
tri_solve__(const TriMatrix& T, VecX& x, row_tag)
{
  typedef typename matrix_traits<TriMatrix>::value_type VT;
  typedef typename matrix_traits<TriMatrix>::size_type Int;

  if (T.is_upper()) {
    typename TriMatrix::const_reverse_iterator T_i, T_iend;
    typename TriMatrix::Row::const_reverse_iterator T_ij;

    T_i = T.rbegin();
    T_iend = T.rend();

    if ( (T_i != T_iend) && ! T.is_unit()) {
      T_ij = (*T_i).rbegin();
      x[T_ij.row()] /= *T_ij;
      ++T_i;
    }

    while (T_i != T_iend) {
      T_ij = (*T_i).rbegin();
      //Int i = T_ij.row();
      Int i = T_i.index();
      VT t = x[i];

      typename TriMatrix::Row::const_reverse_iterator T_iend;
      T_iend = (*T_i).rend();
      if ( (T_ij != T_iend) && ! T.is_unit())
        --T_iend;

      Int j;
      while (T_ij != T_iend) {
        j = T_ij.column();
        t -= (*T_ij) * x[j];
        ++T_ij;
      }
      if ( (*T_i).rbegin() != (*T_i).rend() && !T.is_unit()) //T_i is not empty
        t /= *T_ij;

      x[i] = t;

      ++T_i;
    }
  } else { /* T is lower */

    typename TriMatrix::const_iterator T_i;
    typename TriMatrix::Row::const_iterator T_ij;

    T_i = T.begin();

    if (T_i != T.end() && ! T.is_unit()) {
      T_ij = (*T_i).begin();
      x[T_ij.row()] *= VT(1) / *T_ij;
      ++T_i;
    }

    while (T_i != T.end()) {
      T_ij = (*T_i).begin();
      //Int i = T_ij.row(); //T_ij could be bad
      Int i = T_i.index();
      VT t = x[i];

      typename TriMatrix::Row::const_iterator T_iend;
      T_iend = (*T_i).end();
      if ( ( T_ij != T_iend ) &&  ! T.is_unit())
        --T_iend;

      Int j;
      while (T_ij != T_iend) {
        j = T_ij.column();
        t -= (*T_ij) * x[j];
        ++T_ij;
      }
      if ( (*T_i).begin() !=(*T_i).end() &&  !T.is_unit())
        t /= *T_ij;

      x[i] = t;
      ++T_i;
    }
  }
}


//: Triangular Solve:  <tt>x <- T^{-1} * x</tt>
//  Use with trianguler matrixes only ie. use the <TT>triangle</TT>
//  adaptor class.
//
//  To use with a sparse matrix, the sparse matrix must be wrapped with
//  a triangle adaptor. You must specify "packed" in the triangle
//  adaptor. The sparse matrix must only have elements in the correct
//  side.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: tri_solve.cc
//!typereqs: <tt>Matrix::value_type</tt> and <tt>VecX::value_type</tt> must be the same type
//!typereqs: the multiplication operator must be defined for <tt>Matrix::value_type</tt>
//!typereqs: the division operator must be defined for <tt>Matrix::value_type</tt>
//!typereqs: the addition operator must be defined for <tt>Matrix::value_type</tt>
template <class TriMatrix, class VecX>
inline void
tri_solve(const TriMatrix& T, MTL_OUT(VecX) x_) MTL_THROW_ASSERTION
{
  VecX& x = const_cast<VecX&>(x_);
  MTL_ASSERT(T.nrows() <= x.size(), "mtl::tri_solve()");
  MTL_ASSERT(T.ncols() <= x.size(), "mtl::tri_solve()");
  MTL_ASSERT(T.ncols() == T.nrows(), "mtl::tri_solve()");
  typedef typename TriMatrix::orientation orien;
  tri_solve__(T, x, orien());
}





//: tri solve for left side
//!noindex:
template <class MatT, class MatB>
inline void
tri_solve__(const MatT& T, MatB& B, left_side)
{
  /*  const int M = B.nrows(); */
  const int N = B.ncols();

  /* unoptimized version */
  for (int j = 0; j < B.ncols(); ++j)
    mtl::tri_solve(T, columns(B)[j]);


  /* JGS need to do an optimized version of this
  if (T.is_upper()) {
    for (int k = M-1; k > 0; --k) {
      if (B(k,j) != 0) {
        if (! T.is_unit())
          B(k,j) /= T(k,k);
        for (int i = 0; i < k; ++i)
          B(i,j) -= B(k,j) * T(i,k);
      }
    }
  } else {
    for (int j = 0; j < N; ++j)
      for (int k = 0; k < M; ++k) {
        if (B(k,j) != 0) {
          if (! T.is_unit())
            B(k,j) /= T(k,k);
          for (int i = k; i < M; ++i)
            B(i,j) -= B(k,j) * T(i,k);
        }
      }
  }
  */
}


/* JGS untested!!! */

//: tri solve for right side
//!noindex:
template <class MatT, class MatB>
inline void
tri_solve__(const MatT& T, MatB& B, right_side)
{
  const int M = B.nrows();
  const int N = B.ncols();
  typedef typename MatT::PR PR;

  if (T.is_upper()) {
    for (int j = 0; j < N; ++j) {
      for (int k = 0; k < j; ++k)
        if (T(k,j) != PR(0))
          for (int i = 0; i < M; ++i)
            B(i,j) -=  T(k,j) * B(i,k);
      if (! T.is_unit()) {
        PR tmp = PR(1) / T(j,j);
        for (int i = 1; i < M; ++i)
          B(i,j) = tmp * B(i,j);
      }
    }
  } else { // T is lower
    for (int j = N - 1; j > 0; --j) {
      for (int k = j; k < N; ++k)
        if (T(k,j) != PR(0))
          for (int i = 0; i < M; ++i)
            B(i,j) -=  T(k,j) * B(i,k);
      if (! T.is_unit()) {
        PR tmp = PR(1) / T(j,j);
        for (int i = 1; i < M; ++i)
          B(i,j) = tmp * B(i,j);
      }
    }
  }
}

//: Triangular Solve: <tt>B <- A^{-1} * B  or  B <- B * A^{-1}</tt>
//
//  This solves the equation <tt>T*X = B</tt> or <tt>X*T = B</tt> where T
//  is an upper or lower triangular matrix, and B is a general
//  matrix. The resulting matrix X is written onto matrix B. The first
//  equation is solved if <tt>left_side</tt> is specified. The second
//  equation is solved if <tt>right_side</tt> is specified.
//
//  Currently only works with dense storage format.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n^3)
//!example: matmat_trisolve.cc
//!typereqs: <tt>MatT::value_type</tt> and <tt>MatB::value_type</tt> must be the same type
//!typereqs: the multiplication operator must be defined for <tt>MatT::value_type</tt>
//!typereqs: the division operator must be defined for <tt>MatT::value_type</tt>
//!typereqs: the addition operator must be defined for <tt>MatT::value_type</tt>
template <class MatT, class MatB, class Side>
inline void
tri_solve(const MatT& T, MTL_OUT(MatB) B, Side s)
{
  tri_solve__(T, const_cast<MatB&>(B), s);
}





//: Rank One Update:   <tt>A <- A  +  x * y^T</tt>
//
// Also known as the outer product of two vectors.
// <codeblock>
//       y = [ 1  2  3 ]
//
//     [ 1 ] [ 1  2  3 ]
// x = [ 2 ] [ 2  4  6 ] => A
//     [ 3 ] [ 3  6  9 ]
//     [ 4 ] [ 4  8 12 ]
// </codeblock>
// <p>
// When using this algorithm with a symmetric matrix, x and y
// must be the same vector, or at least have the same values.
// Otherwise the resulting matrix is not symmetric.
//
//!precond:  <TT>A.nrows() <= x.size()</TT>
//!precond:  <TT>A.ncols() <= y.size()</TT>
//!precond: A has rectangle shape and is dense
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: rank_one.cc
//!typereqs: <tt>Matrix::value_type</tt>, <tt>VecX::value_type</tt>, and <tt>VecY::value_type</tt> must be the same type
//!typereqs: the multiplication operator must be defined for <tt>Matrix::value_type</tt>
//!typereqs: the addition operator must be defined for <tt>Matrix::value_type</tt>
template <class Matrix, class VecX, class VecY>
inline void
rank_one_update(MTL_OUT(Matrix) A_,
		const VecX& x, const VecY& y) MTL_THROW_ASSERTION
{
  Matrix& A = const_cast<Matrix&>(A_);
  MTL_ASSERT(A.nrows() <= x.size(), "mtl::rank_one_update()");
  MTL_ASSERT(A.ncols() <= y.size(), "mtl::rank_one_update()");
  typename Matrix::iterator i;
  typename Matrix::OneD::iterator j, jend;
  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      *j += x[j.row()] * MTL_CONJ(y[j.column()]);
  }
}



/* 1. how will the scaling by alpha work into this
 * 2. is my placement of conj() ok with respect
 *    to both row and column oriented matrices
 * 3. Perhaps split this in two, have diff version for complex
 */

//: Rank Two Update:  <tt>A <- A  +  x * y^T  +  y * x^T</tt>
//
//
//!category: algorithms
//!component: function
//!precond:   <TT>A.nrows() == A.ncols()</TT>
//!precond:   <TT>A.nrows() == x.size()</TT>
//!precond:   <TT>x.size() == y.size()</TT>
//!precond: A has rectangle shape and is dense.
//!definition: mtl.h
//!example: rank_2_symm_sparse.cc
//!typereqs: <tt>Matrix::value_type</tt>, <tt>VecX::value_type</tt>, and <tt>VecY::value_type</tt> must be the same type.
//!typereqs: The multiplication operator must be defined for <tt>Matrix::value_type</tt>.
//!typereqs: The addition operator must be defined for <tt>Matrix::value_type</tt>.
template <class Matrix, class VecX, class VecY>
inline void
rank_two_update(MTL_OUT(Matrix) A_,
		const VecX& x, const VecY& y) MTL_THROW_ASSERTION
{
  Matrix& A = const_cast<Matrix&>(A_);
  MTL_ASSERT(A.nrows() == A.ncols(), "mtl::rank_two_update()");
  MTL_ASSERT(A.nrows() <= x.size(), "mtl::rank_two_update()");
  MTL_ASSERT(A.nrows() <= y.size(), "mtl::rank_two_update()");
  typename Matrix::iterator i;
  typename Matrix::OneD::iterator j, jend;
  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      *j += x[j.row()] * MTL_CONJ(y[j.column()])
                + y[j.row()] * MTL_CONJ(x[j.column()]);
  }
}

template <class VecX, class VecY>
inline void
copy__(const VecX& x, VecY& y, fast::count<0>)
{
  mtl_algo::copy(x.begin(), x.end(), y.begin());
}
#if USE_BLAIS
template <class VecX, class VecY, int N>
inline void
copy__(const VecX& x, VecY& y, fast::count<N>)
{
  fast::copy(x.begin(), fast::count<N>(), y.begin());
}
#endif


template <class VecX, class VecY>
inline void
oned_copy(const VecX& x, VecY& y, dense_tag, dense_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(x.size() <= y.size(), "mtl::copy()");
  // copy__(x, y, dim_n<VecX>::RET()); gcc4
  copy__(x, y, typename dim_n<VecX>::RET());
}

#if 0
/* perform a scatter */
template <class VecX, class VecY>
inline void
oned_copy(const VecX& x, VecY y, sparse_tag, dense_tag) MTL_THROW_ASSERTION
{
  typename VecX::const_iterator xi;
  for (xi = x.begin(); xi != x.end(); ++xi)
    y[xi.index()] = *xi;
}


/* perform a gather JGS, does this really make sense? */
template <class VecX, class VecY>
inline void
oned_copy(const VecX& x, VecY y, dense_tag, sparse_tag) MTL_THROW_ASSERTION
{
  typedef typename VecX::value_type T;
  typename VecY::iterator yi;
  for (yi = y.begin(); yi != y.end(); ++yi)
    *yi = x[yi.index()];
}
#else


template <class VecX, class VecY>
inline void
oned_copy(const VecX& x, VecY& y, sparse_tag, dense_tag) MTL_THROW_ASSERTION
{
  typedef typename linalg_traits<VecY>::value_type T;
  mtl::set_value(y, T(0));
  typename VecX::const_iterator xi;
  for (xi = x.begin(); xi != x.end(); ++xi)
    y[xi.index()] = *xi;
}


//: Scatter <tt>y <- x</tt>
//
//  Scatters the elements of the sparse vector x into
//  the dense vector y.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n) where n is the size of the sparse vector
template <class VecX, class VecY>
inline void
scatter(const VecX& x, MTL_OUT(VecY) y_) MTL_THROW_ASSERTION
{
  VecY& y = const_cast<VecY&>(y_);
  typename VecX::const_iterator xi;
  for (xi = x.begin(); xi != x.end(); ++xi)
    y[xi.index()] = *xi;
}

//: Gather <tt>y <- x</tt>
//
//  Gathers the elements of the dense vector x into
//  the sparse vector y, based on the non-zero structure of y.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n) where n is the size of the sparse vector
template <class VecX, class VecY>
inline void
gather(const VecX& x, MTL_OUT(VecY) y_) MTL_THROW_ASSERTION
{
  VecY& y = const_cast<VecY&>(y_);
  typedef typename VecX::value_type T;
  typename VecY::iterator yi;
  for (yi = y.begin(); yi != y.end(); ++yi)
    *yi = x[yi.index()];
}
#endif

template <class VecX, class VecY, class Tag>
inline void
oned_copy(const VecX& x, VecY& y, Tag, sparse_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(x.size() <= y.size(), "mtl::copy()");
  y.clear();
  typename VecX::const_iterator i = x.begin(), iend = x.end();
  for (; i != iend; ++i)
    y.push_back(i.index(), *i);
}


template <class VecX, class VecY>
inline void
copy__(const VecX& x, VecY& y, oned_tag) MTL_THROW_ASSERTION
{
  typedef typename linalg_traits<VecX>::sparsity SpX;
  typedef typename linalg_traits<VecY>::sparsity SpY;
  oned_copy(x, y, SpX(), SpY());
}


template <class MatA, class MatB>
inline void
twod_copy_default(const MatA& A, MatB& B) MTL_THROW_ASSERTION
{
  typename MatA::const_iterator i;
  typename MatA::OneD::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      B(j.row(),j.column()) = *j;
  }
}

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, rectangle_tag) MTL_THROW_ASSERTION
{
  twod_copy_default(A, B);
}

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, banded_tag) MTL_THROW_ASSERTION
{
  twod_copy_default(A, B);
}

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, symmetric_tag) MTL_THROW_ASSERTION
{
  typename MatA::const_iterator i;
  typename MatA::OneD::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j) {
      B(j.row(),j.column()) = *j;
      B(j.column(),j.row()) = *j;
    }
  }
}

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, triangle_tag) MTL_THROW_ASSERTION
{
  typedef typename matrix_traits<MatB>::value_type T;

  if (A.is_unit())
    set_diagonal(B, T(1));

  twod_copy(A, B, rectangle_tag());
}

template <class MatA, class MatB>
inline void
twod_copy__(const MatA& A, MatB& B, dense_tag)
{
  typedef typename matrix_traits<MatA>::shape Shape;
  twod_copy(A, B, Shape());
}


/*
  Sparse matrices have specialized copy functions since
  they need to optimize the creation of the non-zero structure.

  only good for same orientation!!!
 */


template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, row_tag, row_tag)
{
  B.fast_copy(A);
}
template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, column_tag, column_tag)
{
  B.fast_copy(A);
}

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, row_tag, column_tag)
{
  twod_copy__(A, B, dense_tag());
}
template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, column_tag, row_tag)
{
  twod_copy__(A, B, dense_tag());
}

template <class MatA, class MatB>
inline void
twod_copy__(const MatA& A, MatB& B, sparse_tag)
{
  typedef typename matrix_traits<MatA>::orientation OrienA;
  typedef typename matrix_traits<MatB>::orientation OrienB;
  twod_copy(A, B, OrienA(), OrienB());
}

template <class MatA, class MatB>
inline void
copy__(const MatA& A, MatB& B, twod_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(A.nrows() <= B.nrows(), "copy(A, B, twod_tag)");
  MTL_ASSERT(A.ncols() <= B.ncols(), "copy(A, B, twod_tag)");

  typedef typename matrix_traits<MatB>::sparsity Sparsity;
  twod_copy__(A, B, Sparsity());
}

//: Copy:  <tt>B <- A or y <- x</tt>
//
//  Copy the elements of matrix A into matrix B, or copy the elements
//  of vector x into vector y. For shaped and sparse matrices, this
//  copies only the elements stored in A to B.  If x is a sparse
//  vector and y is dense, a "scatter" is performed. If y is sparse
//  and x is dense, then a "gather" is performed. If both vectors
//  are sparse, but of different structure the result is undefined.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(m*n) for matrices. O(nnz) if either A or B are sparse and of the same orientation (otherwise it can be O(nnz^2). O(n) for vectors.
//!example: vecvec_copy.cc
template <class LinalgA, class LinalgB>
inline void
copy(const LinalgA& A, MTL_OUT(LinalgB) B_) MTL_THROW_ASSERTION
{
  LinalgB& B = const_cast<LinalgB&>(B_);
  typedef typename linalg_traits<LinalgA>::dimension Dim;
  copy__(A, B, Dim());
}

template <class VecX, class VecY> inline
void
add__(const VecX& x, VecY& y, fast::count<0>)
{
  typedef typename VecX::value_type T;
  mtl_algo::transform_add(x.begin(), x.end(), y.begin());

}
#if USE_BLAIS
template <class VecX, class VecY, int N> inline
void
add__(const VecX& x, VecY& y, fast::count<N>)
{
  typedef typename VecX::value_type T;
  fast::transform(x.begin(), fast::count<N>(), y.begin(),
                  y.begin(), std::plus<T>());
}
#endif
template <class VecX, class VecY> inline
void
add__(const VecX& x, VecY& y, oned_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(x.size() <= y.size(), "mtl::add()");

  add__(x, y, dim_n<VecX>::RET());
}


template <class VecX, class VecY, class VecZ> inline
void
oned_add(const VecX& x, const VecY& y, VecZ& z, fast::count<0>)
{
  typedef typename VecX::value_type T;
  mtl_algo::transform(x.begin(), x.end(), y.begin(), z.begin(), std::plus<T>());
}
#if USE_BLAIS
template <class VecX, class VecY, class VecZ, int N> inline
void
oned_add(const VecX& x, const VecY& y, VecZ& z, fast::count<N>)
{
  typedef typename VecX::value_type T;
  fast::transform(x.begin(), fast::count<N>(), y.begin(), z.begin(), std::plus<T>());
}
#endif

template <class VecX, class VecY, class VecZ>
inline void
oned_add(const VecX& x, const VecY& y, VecZ& z_, sparse_tag)
{

  typedef typename VecZ::value_type T;
  compressed1D<T> z;
  typedef typename VecX::const_iterator xiter;
  typedef typename VecY::const_iterator yiter;

  xiter xi = x.begin();
  xiter xiend = x.end();
  yiter yi = y.begin();
  yiter yiend = y.end();

  while (xi != xiend && yi != yiend) {
    if (yi.index() < xi.index()) {
      z.push_back(yi.index(), *yi);
      ++yi;
    } else if (xi.index() < yi.index()) {
      z.push_back(xi.index(), *xi);
      ++xi;
    } else {
      z.push_back(xi.index(), *yi + *xi);
      ++xi; ++yi;
    }
  }
  while (xi != xiend) {
    z.push_back(xi.index(), *xi);
    ++xi;
  }
  while (yi != yiend) {
    z.push_back(yi.index(), *yi);
    ++yi;
  }
  z_.clear();
  mtl::copy(z, z_);
}

template <class VecX, class VecY, class VecZ>
inline void
oned_add(const VecX& x, const VecY& y, VecZ& z, dense_tag) MTL_THROW_ASSERTION
{
  // oned_add(x, y, z, dim_n<VecX>::RET()); gcc4
  oned_add(x, y, z, typename dim_n<VecX>::RET());
}


//: Add:  <tt>z <- x + y</tt>
//
// Add the elements of x and y and assign into z.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: y_ax_y.cc, vecvec_add.cc
//!typereqs: <tt>VecX::value_type</tt>,  <tt>VecY::value_type</tt>,  and  <tt>VecZ::value_type</tt> should be the same type
//!typereqs: The addition operator must be defined for the value_type.
//!complexity: linear time
template <class VecX, class VecY, class VecZ>
inline void
add(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_) MTL_THROW_ASSERTION
{
  VecZ& z = const_cast<VecZ&>(z_);
  MTL_ASSERT(x.size() <= y.size(), "mtl::add()");
  MTL_ASSERT(x.size() <= z.size(), "mtl::add()");
  typedef typename linalg_traits<VecZ>::sparsity Sparsity;
  oned_add(x, y, z, Sparsity());
}

//: Add:  <tt>w <- x + y + z</tt>
//
// Add the elements of x, y, and z and assign into w.
// For now just dense vectors.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vecvec_add3.cc
//!typereqs: <tt>VecX::value_type</tt>, <tt>VecY::value_type</tt>, <tt>VecZ::value_type</tt>, and <tt>VecW::value_type</tt> should be the same type
//!typereqs: The addition operator must be defined for the value_type.
//!complexity: linear time
template <class VecW, class VecX, class VecY, class VecZ>
inline void
add(const VecX& x, const VecY& y, const VecZ& z, MTL_OUT(VecW) w_)
  MTL_THROW_ASSERTION
{
  VecW& w = const_cast<VecW&>(w_);
  MTL_ASSERT(x.size() <= y.size(), "mtl::add()");
  MTL_ASSERT(x.size() <= z.size(), "mtl::add()");
  MTL_ASSERT(x.size() <= w.size(), "mtl::add()");

  typename VecX::const_iterator x_i = x.begin();
  typename VecY::const_iterator y_i = y.begin();
  typename VecZ::const_iterator z_i = z.begin();
  typename VecW::iterator w_i = w.begin();

  while (not_at(x_i, x.end())) {
    *w_i = *x_i + *y_i + *z_i;
    ++x_i; ++y_i; ++z_i; ++w_i;
  }
}


template <class MatA, class MatB>
inline void
twod_add_default(const MatA& A, MatB& B)
{
  typename MatA::const_iterator i;
  typename MatA::OneD::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      B(j.row(), j.column()) += *j;
  }
}

template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, banded_tag)
{
  twod_add_default(A, B);
}

template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, rectangle_tag)
{
  twod_add_default(A, B);
}

template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, triangle_tag)
{
  typedef typename matrix_traits<MatA>::size_type Int;
  typedef typename matrix_traits<MatA>::value_type T;
  if (A.is_unit())
    for (Int i = 0; i < MTL_MIN(A.nrows(), A.ncols()); ++i)
      B(i,i) += T(1);

  twod_add(A, B, banded_tag());
}

/* perhaps I should add is_row() and is_column()
 methods to the matrices
 */
template <class MatA, class MatB>
inline void
twod_symmetric_add(const MatA& A, MatB& B, row_tag)
{
  typename MatA::const_iterator i;
  typename MatA::Row::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin();
    jend = (*i).end();
    if (A.is_upper()) { /* handle the diagonal elements */
      B(j.column(), j.row()) += *j;
      ++j;
    } else
      --jend;
    for (; j != jend; ++j) {
      B(j.row(), j.column()) += *j;
      B(j.column(), j.row()) += *j;
    }
    if (A.is_lower())
      B(j.column(), j.row()) += *j;
  }
}

template <class MatA, class MatB>
inline void
twod_symmetric_add(const MatA& A, MatB& B, column_tag)
{
  typename MatA::const_iterator i;
  typename MatA::Column::const_iterator j, jend;

  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin();
    jend = (*i).end();
    if (A.is_lower()) { /* handle the diagonal elements */
      B(j.column(), j.row()) += *j;
      ++j;
    } else
      --jend;
    for (; j != jend; ++j) {
      B(j.row(), j.column()) += *j;
      B(j.column(), j.row()) += *j;
    }
    if (A.is_upper())
      B(j.column(), j.row()) += *j;
  }
}


template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, symmetric_tag)
{
  typedef typename matrix_traits<MatA>::orientation Orien;
  twod_symmetric_add(A, B, Orien());
}


template <class MatA, class MatB>
inline void
add__(const MatA& A, MatB& B, twod_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(A.nrows() <= B.nrows(), "matmat::add()");
  MTL_ASSERT(A.ncols() <= B.ncols(), "matmat::add()");

  typedef typename matrix_traits<MatA>::shape Shape;
  twod_add(A, B, Shape());
}

//: Add:  <tt>B <- A + B  or  y <- x + y</tt>
//  The function adds the element of A to B, or the elements of x to y.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(m*n) for a dense A, O(nnz) for a sparse A. O(n) for a vector.

template <class LinalgA, class LinalgB>
inline void
add(const LinalgA& A, MTL_OUT(LinalgB) B_) MTL_THROW_ASSERTION
{
  LinalgB& B = const_cast<LinalgB&>(B_);
  typedef typename linalg_traits<LinalgA>::dimension Dim;
  add__(A, B, Dim());
}



template <class VecX, class VecY, class VecZ>
inline void
ele_mult(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_, fast::count<0>)
{
  VecZ& z = const_cast<VecZ&>(z_);
  typedef typename VecX::value_type T;
  mtl_algo::transform(x.begin(), x.end(), y.begin(), z.begin(),
                      std::multiplies<T>());
}
#if USE_BLAIS
template <class VecX, class VecY, class VecZ, int N>
inline void
ele_mult(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_, fast::count<N>)
{
  VecZ& z = const_cast<VecZ&>(z_);
  typedef typename VecX::value_type T;
  fast::transform(x.begin(), fast::count<N>(), y.begin(), z.begin(),
                  std::multiplies<T>());
}
#endif

//: Element-wise Multiplication:  <tt>z <- x O* y</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vecvec_ele_mult.cc
template <class VecX, class VecY, class VecZ>
inline void
ele_mult(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_) MTL_THROW_ASSERTION
{
  VecZ& z = const_cast<VecZ&>(z_);
  MTL_ASSERT(x.size() <= y.size(), "mtl::ele_mult()");
  MTL_ASSERT(x.size() <= z.size(), "mtl::ele_mult()");

  ele_mult(x, y, z, typename dim_n<VecX>::RET());
}



//: Element-wise Multiply:  <tt>B <- A O* B</tt>
//
//  This function multiplies each of the elements
//  of B by the corresponding element of A.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n^2)
template <class MatA, class MatB>
inline void
ele_mult(const MatA& A, MTL_OUT(MatB) B_) MTL_THROW_ASSERTION
{
  MatB& B = const_cast<MatB&>(B_);
  /* Note: have to iterator over B, since
   * elements of B may get zeroed out,
   * but zero elements of B stay zero
   */
  //typename MatB::row_2Diterator B_i;
  //typename MatB::RowVector::iterator j, jend;
  typename MatB::iterator i;
  typename MatB::OneD::iterator j, jend;

  for (i = B.begin(); i != B.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      *j *= A(j.row(),j.column());
  }
}


//: Element-wise Division:  <tt>z <- x O/ y</tt>
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vecvec_ele_div.cc
template <class VecX, class VecY, class VecZ>
inline void
ele_div(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_) MTL_THROW_ASSERTION
{
  VecZ& z = const_cast<VecZ&>(z_);
  MTL_ASSERT(x.size() <= y.size(), "mtl::ele_div()");
  MTL_ASSERT(x.size() <= z.size(), "mtl::ele_div()");

  typedef typename VecX::value_type T;
  mtl_algo::transform(x.begin(), x.end(), y.begin(), z.begin(),
                      std::divides<T>());
}




template <class VecX, class VecY>
inline void
swap(VecX& x, VecY& y, fast::count<0>)
{
  mtl_algo::swap_ranges(x.begin(), x.end(), y.begin());
}
#if USE_BLAIS
template <class VecX, class VecY, int N>
inline void
swap(VecX& x, VecY& y, fast::count<N>)
{
  fast::swap_ranges(x.begin(), fast::count<N>(), y.begin());
}
#endif

template <class VecX, class VecY>
inline void
swap(VecX& x, VecY& y, oned_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(x.size() <= y.size(), "mtl::swap()");
  swap(x, y, typename dim_n<VecX>::RET());
}



template <class MatA, class MatB>
inline void
swap(MatA& A, MatB& B, twod_tag) MTL_THROW_ASSERTION
{
  MTL_ASSERT(A.nrows() == B.nrows(), "matmat::swap()");
  MTL_ASSERT(A.ncols() == B.ncols(), "matmat::swap()");

  typename MatA::iterator A_i;
  typename MatA::OneD::iterator A_ij, A_ijend;
  typename MatB::iterator B_i;
  typename MatB::Row::iterator B_ij;

  A_i = A.begin();  B_i = B.begin();
  while (A_i != A.end()) {
    A_ij = (*A_i).begin();  B_ij = (*B_i).begin();
    A_ijend = (*A_i).end();
    while (A_ij != A_ijend) {
      typename matrix_traits<MatA>::value_type tmp = *B_ij;
      *B_ij = *A_ij;
      *A_ij = tmp;
      ++A_ij; ++B_ij;
    }
    ++A_i; ++B_i;
  }
}


//: Swap:   <tt>B <-> A or y <-> x</tt>
//
// Exchanges the elements of the containers.
//  Not compatible with sparse matrices. For banded matrices
//  and other shaped matrices, A and B must be the same shape.
//  Also, the two matrices must be the same orientation.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!complexity: O(n^2)
//!example: vecvec_swap.cc
template <class LinalgA, class LinalgB>
inline void
swap(MTL_OUT(LinalgA) A, MTL_OUT(LinalgB) B) MTL_THROW_ASSERTION
{
  typedef typename linalg_traits<LinalgA>::dimension Dim;
  swap(const_cast<LinalgA&>(A), const_cast<LinalgB&>(B), Dim());
}


template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, fast::count<0>)
{
  return mtl_algo::inner_product(x.begin(), x.end(), y.begin(), s);
}
#if USE_BLAIS
template <class VecX, class VecY, class T, int N>
inline T
dot(const VecX& x, const VecY& y, T s, fast::count<N>)
{
  return fast::inner_product(x.begin(), fast::count<N>(), y.begin(), s);
}
#endif


template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, dense_tag, dense_tag)
{
  return dot(x, y, s, typename dim_n<VecX>::RET());
}

template <class InputIterator1, class InputIterator2, class T>
inline T
sparse_inner_product(InputIterator1 f1, InputIterator1 l1,
                     InputIterator2 f2, InputIterator2 l2, T init)
{
  InputIterator1 first1 = f1;
  InputIterator1 last1 = l1;
  InputIterator2 first2 = f2;
  InputIterator2 last2 = l2;

  while (first1 != last1 && first2 != last2) {
    if (first1.index() == first2.index())
      init += (*first1++ * *first2++);
    else if (first1.index() < first2.index())
      ++first1;
    else
      ++first2;
  }
  return init;
}

template <class IndexedIterator, class RandomAccessIterator, class T>
inline T
sparse_dense_inner_product(IndexedIterator f1, IndexedIterator l1,
                           RandomAccessIterator f2, T init)
{
  IndexedIterator first1 = f1, last1 = l1;
  RandomAccessIterator first2 = f2;

  while (first1 != last1) {
    init += (*first1 * first2[first1.index()]);
    ++first1;
  }
  return init;
}


template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, sparse_tag, sparse_tag)
{
  if (x.nnz() < y.nnz())
    return sparse_inner_product(x.begin(), x.end(), y.begin(), y.end(), s);
  else
    return sparse_inner_product(y.begin(), y.end(), x.begin(), x.end(), s);
}

template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, dense_tag, sparse_tag)
{
  return sparse_dense_inner_product(y.begin(), y.end(), x.begin(), s);
}

template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, sparse_tag, dense_tag)
{
  return sparse_dense_inner_product(x.begin(), x.end(), y.begin(), s);
}


//: Dot Product:  <tt>s <- x . y + s</tt>
//  The type used for argument s determines the
//  type of the resulting product.
//!category: algorithms
//!component: function
//!definition: mtl.h
template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s) MTL_THROW_ASSERTION
{
  MTL_ASSERT(x.size() == y.size(), "mtl::dot()");
  typedef typename linalg_traits<VecX>::sparsity SparseX;
  typedef typename linalg_traits<VecY>::sparsity SparseY;
  return dot(x, y, s, SparseX(), SparseY());
}


//: Dot Product:  <tt>s <- x . y</tt>
//  The type of the resulting product is <TT>VecX::value_type</TT>.
//!category: algorithms
//!component: function
//!example: vecvec_dot.cc, dot_prod.cc
//!definition: mtl.h
template <class VecX, class VecY>
inline typename VecX::value_type
dot(const VecX& x, const VecY& y) MTL_THROW_ASSERTION
{
  typedef typename VecX::value_type T;
  return mtl::dot(x, y, T(0));
}

#ifdef USE_DOUBLE_DOUBLE
//: Dot Product (extended precision):  <tt>s <- x . y + s</tt>
//  The type of the resulting product is double_double
//  Extended precision is used internally.
//!category: algorithms
//!component: function
//!definition: mtl.h
template <class VecX, class VecY>
inline double_double
dot(const VecX& x, const VecY& y, double_double s) MTL_THROW_ASSERTION
{
  typedef typename VecX::value_type x_type;
  typedef typename VecY::value_type y_type;
  // x_type and y_type must be either float or double
  multiply<double_double, x_type, y_type> m;
  addition<double_double, double_double, double_double> a;
  return mtl_algo::inner_product(x.begin(), x.end(), y.begin(), s, a, m);
}
#endif /* USE_DOUBLE_DOUBLE */

template <class T>
struct conj_func {
  typedef T result_type;
  inline T operator()(const T& x) const { return MTL_CONJ(x); }
};

template <class VecX, class VecY, class T>
inline T
dot_conj(const VecX& x, const VecY& y, T s, fast::count<0>)
{
  return mtl_algo::inner_product(x.begin(), x.end(),
                                 trans_iter(y.begin(), conj_func<T>()), s);
}
#if USE_BLAIS
template <class VecX, class VecY, class T, int N>
inline T
dot_conj(const VecX& x, const VecY& y, T s, fast::count<N>)
{
  return fast::inner_product(x.begin(), x.end(),
                             trans_iter(y.begin(), conj_func<T>()), s);
}
#endif

//: Dot Conjugate:  <tt>s <- x . conj(y) + s</tt>
//   Similar to dot product. The complex conjugate of the elements of y
//   is used. For real numbers, the conjugate is just that real number.
//   Note that the type of parameter s is the return type of this
//   function.
//!category: algorithms
//!component: function
//!definition: mtl.h
template <class VecX, class VecY, class T>
inline T
dot_conj(const VecX& x, const VecY& y, T s) MTL_THROW_ASSERTION
{
  MTL_ASSERT(x.size() <= y.size(), "mtl::dot_conj()");
  return dot_conj(x, y, s, dim_n<VecX>::RET());
}

//: Dot Conjugate:   <tt>s <- x . conj(y)</tt>
//  A slightly simpler version of the dot conjugate.
//  The return type is the element type of vector x.
//!category: algorithms
//!component: function
//!definition: mtl.h
template <class VecX, class VecY>
inline typename VecX::value_type
dot_conj(const VecX& x, const VecY& y) MTL_THROW_ASSERTION
{
  typedef typename VecX::value_type T;
  return mtl::dot_conj(x, y, T(0));
}






} /* namespace mtl */

#endif /* _MTL_MTL_H_ */

