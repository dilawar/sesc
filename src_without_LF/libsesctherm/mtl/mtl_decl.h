// -*- c++ -*-
//
//
// This file is part of the Matrix Template Library
//
//===========================================================================

#ifndef _MTL_MTL_DECL_H_
#define _MTL_MTL_DECL_H_

#include "matrix_traits.h"

/*
  This is a nasty hack necessitated by several things:

  1. C++ does not allow temporaries to be passed
    into a reference argument.
  2. Many MTL expressions result in temporaries
  3. Some MTL matrix classes (static matrix) can
    not be passed by value for the output argument
    since they are not handles.
 */
#define MTL_OUT(X) const X&

namespace mtl {

template <class Vec>
inline typename Vec::size_type
max_index(const Vec& x);

template <class Vec>
inline typename Vec::size_type
max_abs_index(const Vec& x);

template<class Vec>
inline typename Vec::size_type
min_index(const Vec& x);

template<class Vec>
inline typename Vec::size_type
min_abs_index(const Vec& x);

template <class VectorT>
inline typename VectorT::value_type
max(const VectorT& x);

template <class VectorT>
inline typename VectorT::value_type
min(const VectorT& x);

template <class Matrix>
inline void
transpose(MTL_OUT(Matrix) A_);

template <class MatA, class MatB>
inline void
transpose(const MatA& A, MTL_OUT(MatB) B_);

template <class Matrix, class VecX, class VecZ>
inline void
rect_mult(const Matrix& A, const VecX& xx, VecZ& zz,
          row_tag, dense_tag);

template <class Matrix, class VecX, class VecZ>
inline void
rect_mult(const Matrix& A, const VecX& xx, VecZ& zz,
          column_tag, dense_tag);

template <class Matrix, class VecX, class VecY>
void
rect_mult(const Matrix& A, const VecX& x, VecY& y,
	  column_tag, sparse_tag);

template <class Matrix, class VecX, class VecY>
void
rect_mult(const Matrix& A, const VecX& x, VecY& y,
	  row_tag, sparse_tag);

template <class Matrix, class VecX, class VecY, class VecZ>
inline void
mult(const Matrix& A, const VecX& x, const VecY& y, MTL_OUT(VecZ) z_);

template <class Matrix, class VecX, class VecY>
inline void
mult_add(const Matrix& A, const VecX& x, MTL_OUT(VecY) y_);

template <class MatA, class MatB, class MatC, class Orien>
inline void
simple_mult(const MatA& A, const MatB& B, MatC& C, dense_tag, Orien);

template <class MatrixA, class MatrixB, class MatrixC>
inline void
simple_mult(const MatrixA& A, const MatrixB& B, MatrixC& C,
            sparse_tag, row_tag);


template <class MatrixA, class MatrixB, class MatrixC>
inline void
simple_mult(const MatrixA& A, const MatrixB& B, MatrixC& C,
            sparse_tag, column_tag);

template <class MatA, class MatB, class MatC>
inline void
symm_simple_mult(const MatA& A, const MatB& B, MatC& C, row_tag);

template <class MatA, class MatB, class MatC>
inline void
symm_simple_mult(const MatA& A, const MatB& B, MatC& C, column_tag);

template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, symmetric_tag);

template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, triangle_tag);

template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, rectangle_tag);

template <class MatA, class MatB, class MatC>
inline void
matmat_mult(const MatA& A, const MatB& B, MatC& C, banded_tag);

template <class LinalgA, class LinalgB, class LinalgC>
inline void
mult(const LinalgA& A, const LinalgB& B, MTL_OUT(LinalgC) C_);

template <class TriMatrix, class VecX>
inline void
tri_solve(const TriMatrix& T, MTL_OUT(VecX) x_);

template <class MatT, class MatB, class Side>
inline void
tri_solve(const MatT& T, MTL_OUT(MatB) B, Side s);

template <class Matrix, class VecX, class VecY>
inline void
rank_one_update(MTL_OUT(Matrix) A_,
		const VecX& x, const VecY& y);

template <class Matrix, class VecX, class VecY>
inline void
rank_two_update(MTL_OUT(Matrix) A_,
		const VecX& x, const VecY& y);

template <class VecX, class VecY>
inline void
oned_copy(const VecX& x, VecY& y, dense_tag, dense_tag);

template <class VecX, class VecY, class Tag>
inline void
oned_copy(const VecX& x, VecY& y, Tag, sparse_tag);

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, rectangle_tag);

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, banded_tag);

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, symmetric_tag);

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, triangle_tag);

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, row_tag, row_tag);

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, column_tag, column_tag);

template <class MatA, class MatB>
inline void
twod_copy(const MatA& A, MatB& B, row_tag, column_tag);

template <class LinalgA, class LinalgB>
inline void
copy(const LinalgA& A, MTL_OUT(LinalgB) B_);

template <class VecX, class VecY, class VecZ>
inline void
oned_add(const VecX& x, const VecY& y, VecZ& z_, sparse_tag);

template <class VecX, class VecY, class VecZ>
inline void
oned_add(const VecX& x, const VecY& y, VecZ& z, dense_tag);

template <class VecX, class VecY, class VecZ>
inline void
add(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_);

template <class VecW, class VecX, class VecY, class VecZ>
inline void
add(const VecX& x, const VecY& y, const VecZ& z, MTL_OUT(VecW) w_);

template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, banded_tag);

template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, rectangle_tag);

template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, triangle_tag);

template <class MatA, class MatB>
inline void
twod_symmetric_add(const MatA& A, MatB& B, row_tag);

template <class MatA, class MatB>
inline void
twod_symmetric_add(const MatA& A, MatB& B, column_tag);

template <class MatA, class MatB>
inline void
twod_add(const MatA& A, MatB& B, symmetric_tag);

template <class LinalgA, class LinalgB>
inline void
add(const LinalgA& A, MTL_OUT(LinalgB) B_);

template <class VecX, class VecY, class VecZ>
inline void
ele_mult(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_);

template <class MatA, class MatB>
inline void
ele_mult(const MatA& A, MTL_OUT(MatB) B_);

template <class VecX, class VecY, class VecZ>
inline void
ele_div(const VecX& x, const VecY& y, MTL_OUT(VecZ) z_);

template <class VecX, class VecY>
inline void
swap(VecX& x, VecY& y, oned_tag);

template <class MatA, class MatB>
inline void
swap(MatA& A, MatB& B, twod_tag);

template <class LinalgA, class LinalgB>
inline void
swap(MTL_OUT(LinalgA) A, MTL_OUT(LinalgB) B);

template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, dense_tag, dense_tag);

template <class InputIterator1, class InputIterator2, class T>
inline T
sparse_inner_product(InputIterator1 f1, InputIterator1 l1,
                     InputIterator2 f2, InputIterator2 l2, T init);

template <class IndexedIterator, class RandomAccessIterator, class T>
inline T
sparse_dense_inner_product(IndexedIterator f1, IndexedIterator l1,
                           RandomAccessIterator f2, T init);

template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, sparse_tag, sparse_tag);

template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, dense_tag, sparse_tag);

template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s, sparse_tag, dense_tag);

template <class VecX, class VecY, class T>
inline T
dot(const VecX& x, const VecY& y, T s);

template <class VecX, class VecY>
inline typename VecX::value_type
dot(const VecX& x, const VecY& y);

template <class VecX, class VecY, class T>
inline T
dot_conj(const VecX& x, const VecY& y, T s);

} // namespace mtl

#endif // _MTL_MTL_DECL_H_
