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
#ifndef MTL_UTILS_H
#define MTL_UTILS_H

#include "mtl_config.h"
#include "not_at.h"
#include "entry.h"
#include "matrix_traits.h"
#include "dimension.h"

#include <utility>
#include "mtl_complex.h"
#include <algorithm>
#include <iostream>

namespace mtl {


using std::complex;

/* Utility Functions */


template <class Vector>
inline void
print_partitioned_vector(Vector x)
{
  for (typename Vector::iterator i = x.begin();
       i != x.end(); ++i) {
    print_vector(*i);
  }
}

template <class Matrix>
inline void
print_partitioned_matrix(const Matrix& A)
{
  std::cout << "Top mat: " << A.nrows() << "x" << A.ncols() << std::endl;
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  Int i,j;
  for (i=0; i < A.nrows(); ++i) {
    for (j=0; j < A.ncols(); ++j)
      print_all_matrix( A(i,j) );
    std::cout << std::endl;
  }
}

template <class Matrix>
inline void
print_partitioned_by_row(const Matrix& A)
{
  std::cout << "Top mat: " << A.nrows() << "x" << A.ncols() << std::endl;
  typename Matrix::const_iterator A_kk;
  typename Matrix::Row::const_iterator A_i, A_iend;
  A_kk = A.begin();
  while (not_at(A_kk, A.end())) {
    A_i = (*A_kk).begin();
    A_iend = (*A_kk).end();
    while (not_at(A_i, A_iend)) {
      print_all_matrix( *A_i );
      ++A_i;
    }
    ++A_kk;
  }
}

template <class Matrix>
inline void
print_partitioned_by_column(const Matrix& A)
{
  std::cout << "Top mat: " << A.nrows() << "x" << A.ncols() << std::endl;
  typename Matrix::const_iterator A_kk;
  typename Matrix::Column::const_iterator A_i, A_iend;
  A_kk = A.begin();
  while (not_at(A_kk, A.end())) {
    A_i = (*A_kk).begin();
    A_iend = (*A_kk).end();
    while (not_at(A_i, A_iend)) {
      print_all_matrix( *A_i );
      ++A_i;
    }
    ++A_kk;
  }
}

//: utility function for banded
//  should use this in a couple more places
//!noindex:
template <class size_type>
dimension<size_type>
calc_start_fini(int i, int minor, dimension<size_type> bandwidth)
{
  int start = MTL_MAX(i - int(bandwidth.first()), 0);
  int fini = MTL_MIN(i + int(bandwidth.second()) + 1, minor);
  if (start > fini) start = fini;
  return dimension<size_type>(start,fini);
}


template <class Iterator>
inline void
print_vector(Iterator y, Iterator y_end)
{
  std::cout << "[";
  while (not_at(y, y_end)) {
    std::cout << *y << ",";
    ++y;
  }
  std::cout << "]" << std::endl;
}

///
template <class Vector>
inline void
print_vector(Vector x)
{
  typename Vector::iterator i = x.begin();
  std::cout << "[";
  while (not_at(i, x.end())) {
    std::cout << *i << ",";
    ++i;
  }
  std::cout << "]" << std::endl;
}

template <class Vector>
inline void
print_vector_index(Vector x)
{
  typename Vector::iterator i = x.begin();
  std::cout << "[";
  while (not_at(i, x.end())) {
    std::cout << i.index() << ",";
    ++i;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
inline void
print_coord(const Matrix& A)
{
  typename Matrix::const_iterator A_kk;
  typename Matrix::OneD::const_iterator A_i;
  A_kk = A.begin();
  while (not_at(A_kk, A.end())) {
    A_i = (*A_kk).begin();
    while (not_at(A_i, (*A_kk).end())) {
      std::cout << "(" << A_kk.row() << "," << A_i.column() << ") = " << *A_i << std::endl;
      ++A_i;
    }
    ++A_kk;
  }
}

///
template <class Matrix>
inline void
print_all_matrix(const Matrix& A)
{
  typedef typename matrix_traits<Matrix>::size_type Int;
  Int i,j;
  std::cout << A.nrows() << "x" << A.ncols() << std::endl;
  std::cout << "[" << std::endl;
  for (i=0; i < A.nrows(); ++i) {
    std::cout << "[";
    for (j=0; j < A.ncols(); ++j) {
      std::cout << A(i,j);
      if (j < A.ncols() - 1)
	std::cout << ",";
    }
    std::cout << "]";
    if (i < A.nrows() - 1)
      std::cout << "," << std::endl;
    else
      std::cout << std::endl;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
inline void
print_all_banded(const Matrix& A, int lo, int up)
{
  typedef typename matrix_traits<Matrix>::size_type Int;
  Int i, j;
  std::cout << A.nrows() << "x" << A.ncols() << std::endl;
  std::cout << "[" << std::endl;
  for (i = 0; i < A.nrows(); ++i) {
    Int first = MTL_MAX(0, int(i) - lo);
    Int last = MTL_MIN(int(A.ncols()), int(i) + up + 1);
    std::cout << "[";
    for (j = 0; j < A.ncols(); ++j) {
      if (j < first || j >= last)
	std::cout << 0;
      else {
	std::cout << A(i,j);
      }
      if (j < A.ncols() - 1)
	std::cout << ",";
    }
    std::cout << "]";
    if (i < A.nrows() - 1)
      std::cout << "," << std::endl;
    else
      std::cout << std::endl;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
void print_minor(const Matrix& A)
{
  typename Matrix::const_minor_iterator A_kk;
  typename Matrix::MinorVector::const_iterator A_i;
  A_kk = A.begin_minor();
  std::cout << "[" << std::endl;
  while (not_at(A_kk, A.end_minor())) {
    std::cout << "[";
    A_i = (*A_kk).begin();
    while (not_at(A_i, (*A_kk).end())) {
      std::cout << *A_i;
      ++A_i;
      std::cout << ",";
    }
    std::cout << "]";
    std::cout << "," << std::endl;
    ++A_kk;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
void print_row(const Matrix& A)
{
  typename Matrix::const_iterator A_kk;
  typename Matrix::Row::const_iterator A_i, A_iend;
  A_kk = A.begin();
  std::cout << "[" << std::endl;
  while (not_at(A_kk, A.end())) {
    std::cout << "[";
    A_i = (*A_kk).begin();
    A_iend = (*A_kk).end();
    while (not_at(A_i, A_iend)) {
      std::cout << *A_i;
      ++A_i;
      std::cout << ",";
    }
    std::cout << "]";
    std::cout << "," << std::endl;
    ++A_kk;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
void print_rev_row(const Matrix& A)
{
  typename Matrix::const_reverse_iterator A_kk;
  typename Matrix::Row::const_reverse_iterator A_i;
  typename Matrix::Row::const_reverse_iterator A_iend;
  A_kk = A.rbegin();
  std::cout << "[" << std::endl;
  while (not_at(A_kk, A.rend())) {
    std::cout << "[";
    A_i = (*A_kk).rbegin();
    A_iend = (*A_kk).rend();
    while (not_at(A_i, A_iend)) {
      std::cout << *A_i;
      ++A_i;
      std::cout << ",";
    }
    std::cout << "]";
    std::cout << "," << std::endl;
    ++A_kk;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
void print_column(const Matrix& A)
{
  typename Matrix::const_iterator A_kk;
  typename Matrix::Column::const_iterator A_i;
  A_kk = A.begin();
  std::cout << "[" << std::endl;
  while (not_at(A_kk, A.end())) {
    std::cout << "[";
    A_i = (*A_kk).begin();
    while (not_at(A_i, (*A_kk).end())) {
      std::cout << *A_i;
      ++A_i;
      std::cout << ",";
    }
    std::cout << "]";
    std::cout << "," << std::endl;
    ++A_kk;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
void print_rev_column(const Matrix& A)
{
  typename Matrix::const_reverse_iterator A_kk;
  typename Matrix::Column::const_reverse_iterator A_i;
  A_kk = A.rbegin();
  std::cout << "[" << std::endl;
  while (not_at(A_kk, A.rend())) {
    std::cout << "[";
    A_i = (*A_kk).rbegin();
    while (not_at(A_i, (*A_kk).rend())) {
      std::cout << *A_i;
      ++A_i;
      std::cout << ",";
    }
    std::cout << "]";
    std::cout << "," << std::endl;
    ++A_kk;
  }
  std::cout << "]" << std::endl;
}

///
template <class Matrix>
void print_major(const Matrix& A)
{
  typename Matrix::const_iterator A_kk;
  typename Matrix::OneD::const_iterator A_i;
  A_kk = A.begin();
  std::cout << "[" << std::endl;
  while (not_at(A_kk, A.end())) {
    std::cout << "[";
    A_i = (*A_kk).begin();
    while (not_at(A_i, (*A_kk).end())) {
      std::cout << *A_i;
      ++A_i;
      std::cout << ",";
    }
    std::cout << "]";
    std::cout << "," << std::endl;
    ++A_kk;
  }
  std::cout << "]" << std::endl;
}

#if 1
template <class MatrixA, class MatrixB>
inline bool
matrix_equal(const MatrixA& A, const MatrixB& B)
{
  typedef typename matrix_traits<MatrixA>::size_type Int;
  if (A.nrows() != Int(B.nrows()) || A.ncols() != Int(B.ncols()))
    return false;

  for (Int i = 0; i < A.nrows(); ++i)
    for (Int j = 0; j < A.ncols(); ++j)
      if (A(i,j) != B(i,j)) {
#if !defined(_MSVCPP_)
		  std::cout << "(" << i << "," << j << ") "
	     << A(i,j) << " != " << B(i,j) << std::endl;
#endif
		  return false;
      }

  return true;
}

#else

template <class MatrixA, class MatrixB>
inline bool
__matrix_equal(const MatrixA& A, const MatrixB& B, row_tag)
{
  typename MatrixA::const_row_2Diterator A_ii = A.begin_rows();
  while (not_at(A_ii, A.end_rows())) {
    typename MatrixA::RowVector::const_iterator A_j = (*A_ii).begin();
     while (not_at(A_j, (*A_ii).end())) {
       if (abs(*A_j - B(A_ii.index(), A_j.index())) > 0.00001) {
#if !defined(_MSVCPP_)
	 std::cout << "(" << A_ii.index() << "," << A_j.index() << ")="
	      << *A_j << " != " << B(A_ii.index(), A_j.index()) << std::endl;
#endif
	return false;
       }
      ++A_j;
    }
    ++A_ii;
  }
  return true;
}


template <class MatrixA, class MatrixB>
inline bool
__matrix_equal(const MatrixA& A, const MatrixB& B, column_tag)
{
  typename MatrixA::const_column_2Diterator A_jj = A.begin_columns();
  while (A_jj < A.end_columns()) {
    typename MatrixA::ColumnVector::const_iterator A_i = (*A_jj).begin();
     while (A_i != (*A_jj).end()) {
       if (abs(*A_i - B(A_i.index(), A_jj.index())) > 0.00001) {
#if !defined(_MSVCPP_)
		   std::cout << "(" << A_i.index() << "," << A_jj.index() << ")="
	      << *A_i << " != " << B(A_i.index(), A_jj.index()) << std::endl;
#endif
		   return false;
       }
      ++A_i;
    }
    ++A_jj;
  }
  return true;
}

template <class MatrixA, class MatrixB>
inline bool
matrix_row_equal(const MatrixA& A, const MatrixB& B) {
  return __matrix_equal(A, B, row_tag());
}

template <class MatrixA, class MatrixB>
inline bool
matrix_column_equal(const MatrixA& A, const MatrixB& B) {
  return __matrix_equal(A, B, column_tag());
}

///
template <class MatrixA, class MatrixB>
inline bool
matrix_equal(const MatrixA& A, const MatrixB& B)
{
  typedef typename MatrixA::orientation orien;
  return __matrix_equal(A, B, orien());
}
#endif

/*********************************************************/
/* Functions to get random numbers to fill the matrices  */
/*********************************************************/


inline float
make_rand_element(float)
{
  float r = float(rand());
  return r/float(RAND_MAX)*10.0;
}

inline double
make_rand_element(double)
{
  double r = double(rand());
  return r/double(RAND_MAX)*10.0L;
}

inline complex<float>
make_rand_element(complex<float>)
{
  return complex<float>(make_rand_element(float()),
			make_rand_element(float()));
}

inline complex<double>
make_rand_element(complex<double>)
{
  return complex<double>(make_rand_element(double()),
			 make_rand_element(double()));
}

#if 0
template <class Orien>
inline void
print_matrix(const matrix<Orien>& mA)
{
#if 0
  const Orien& A = mA.two_d();
  Orien::const_row_2Diterator row_iter = A.begin_rows();
  std::cout << "[";
  while (row_iter < A.end_rows()) {
    Orien::RowVector::const_iterator ri = (*row_iter).begin();
    std::cout << "[";
     while (ri < (*row_iter).end()) {
      std::cout << *ri;
      std::cout << ",";
      ++ri;
    }
    std::cout << "]," << std::endl;;
    ++row_iter;
  }
  std::cout << "]" << std::endl;
#endif

#if 1
  // good one
  const Orien& A = mA.two_d();
  typename Orien::const_row_2Diterator row_iter = A.begin_rows();
  while (row_iter < A.end_rows()) {
    typename Orien::RowVector::const_iterator ri = (*row_iter).begin();
     while (ri < (*row_iter).end()) {
       if (*ri != 0)
	 std::cout << "(" << row_iter.index() << "," << ri.index()
	      << ") = " << *ri << std::endl;;
      ++ri;
    }
    ++row_iter;
  }
#endif

#if 0
  int i, j;
  for (i=0; i < mA.nrows(); ++i)
    for (j=0; j < mA.ncols(); ++j)
       if (mA(i,j) != 0)
	 std::cout << "(" << i << "," << j
	      << ") = " << mA(i,j) << std::endl;;
#endif

#if 0
  int i,j;
  std::cout << "[";
  for (i=0; i < A.nrows(); ++i) {
    std::cout << "[";
    for (j=0; j < A.ncols(); ++j) {
      std::cout << A(i,j);
      if (j < A.ncols() - 1)
	std::cout << ",";
    }
    std::cout << "]";
    if (i < A.nrows() - 1)
      std::cout << "," << std::endl;
  }
  std::cout << "]" << std::endl;
#endif
}

template <class Orien>
inline void
dense_init(matrix<Orien>& A)
{
  int i, j;
  for(i = 0; i < A.nrows(); ++i)
    for (j = 0; j < A.ncols(); ++j)
      A(i,j) = i * A.ncols() + j;
}

#endif

template <class Mat>
inline void
insert_zero_matrix(Mat& A)
{
  typedef typename matrix_traits<Mat>::size_type Int;
  // idea, for sparse matrices, just zero the non zeroes
  // which should remove them
  Int i, j;
  for (i = 0; i < A.nrows(); ++i) {
    Int first = MTL_MAX(0, int(i) - int(A.sub()));
    Int last = MTL_MIN(int(A.ncols()), int(i) + int(A.super()) + 1);
    for (j = 0; j < A.ncols(); ++j)
      if (j >= first && j < last)
	A(i,j) = 0.0;
  }

}

template <class Matrix>
inline void
zero_matrix(Matrix& A){
  typedef typename matrix_traits<Matrix>::value_type T;
  typename Matrix::iterator oneD_iter = A.begin();
  while (oneD_iter < A.end()) {
    typename Matrix::OneD::iterator i = (*oneD_iter).begin();
    while (i < (*oneD_iter).end()) {
      *i = T(0);
      ++i;
    }
    ++oneD_iter;
  }
}

#if 0
template <class Orien>
inline void
sparse_init(matrix<Orien>& A, int c)
{
  int i;
  for (i=0; i < A.nrows(); ++i)
    A(i, i) = 4;
  for (i=0; i < A.nrows() - 1; ++i) {
    A(i, i + 1) = -1.0;
    A(i + 1, i) = -1.0;
  }
  for (i=0; i < A.nrows() - 2; ++i) {
    A(i, i + 2) = -1.0;
    A(i + 2, i) = -1.0;
  }
  for (i=0; i < A.nrows() - 3; ++i) {
    A(i, i + 3) = -1.0;
    A(i + 3, i) = -1.0;
  }
  for (i=0; i < A.nrows() - c; ++i) {
    A(i, i + c) = -1.0;
    A(i + c, i) = -1.0;
  }
}


//
//  mat_mat_multiply
//

template <class OA, class OB, class OC>
inline void
calc_correct(const matrix<OA>& mA, const matrix<OB>& mB,
	     matrix<OC>& mC2)
{
  int i, j, k;
  for (k = 0; k < mA.ncols(); ++k)
    for (j = 0; j < mB.ncols(); ++j)
      for (i = 0; i < mA.nrows(); ++i) {
	mC2(i,j) += mA(i,k) * mB(k,j);
      }
}

template <class OA>
inline int calc_nonzeroes(const matrix<OA>& mA)
{
  int nonzeroes = 0;

  const OA& A = mA.two_d();

  typename OA::const_row_2Diterator row_iter = A.begin_rows();
  while (row_iter < A.end_rows()) {
    typename OA::RowVector::const_iterator ri = (*row_iter).begin();
    while (ri.not_at((*row_iter).end())) {
      if (*ri != 0.0)
	++nonzeroes;
      ++ri;
    }
    ++row_iter;
  }

  return nonzeroes;
}
#endif


} /* namespace mtl */

#endif
