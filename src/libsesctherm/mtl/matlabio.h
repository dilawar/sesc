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

#ifndef MTL_MATLABIO_H
#define MTL_MATLABIO_H

#include <assert.h>
#include <mat.h> /* matlab include file */
#include "matrix.h"

namespace mtl {

typedef matrix<double, rectangle<>,
               dense<>, column_major>::type matlab_dense;
/* JGS should use sparse<> instead of array<compressed>
   but there is currently a bug in it */
typedef matrix<double, rectangle<>,
               array< compressed<> >, column_major>::type matlab_sparse;

//: Read a Dense Real Matrix from a Matlab file (.mat)
//  The matrix type for this function is the following
//  <codeblock>
//  typedef matrix< double,
//                  rectangle<>,
//                  dense<>,
//                  column_major>::type matlab_dense;
//  </codeblock>
//
//!category: utilities
//!component: function
inline void
read_dense_matlab(matlab_dense& A, char* matrix_name, const char* file)
{
  MATFile *fp = matOpen(file, "r");
  if (fp == NULL) {
    cout << "could not open file " << file << endl;
    return;
  }
  mxArray *a = matGetArray(fp, matrix_name);
  if (a == NULL) {
    cout << "could not find matrix " << matrix_name << endl;
    return;
  }
  assert(!mxIsSparse(a));
  int M = mxGetM(a);
  int N = mxGetN(a);

  double* da = (double*)mxGetData(a);

  A = matlab_dense(M, N);
  for (int j = 0; j < N; ++j)
    for (int i = 0; i < M; ++i) {
      A(i,j) = da[j * M + i];
    }

  matClose(fp);
  mxDestroyArray(a);
}

//: Write a Dense Real Matrix to a Matlab file (.mat)
//
//  The matrix type for this function is the following
//  <codeblock>
//  typedef matrix<double,
//                 rectangle<>,
//                 dense<>,
//                 column_major>::type matlab_dense;
//  </codeblock>
//!category: utilities
//!component: function
inline void
write_dense_matlab(matlab_dense& A, char* matrix_name, const char* file)
{
  MATFile *fp = matOpen(file, "w");
  if (fp == NULL) {
    cout << "could not open file " << file << endl;
    return;
  }
  mxArray *a = mxCreateDoubleMatrix(A.nrows(), A.ncols(), mxREAL);
  double* data = (double*)mxGetData(a);
  mxSetName(a, matrix_name);
  mxSetData(a, A.data());
  if (matPutArray(fp, a) != 0)
    cout << "error in writing mat file" << endl;
  matClose(fp);
  mxSetData(a, data);
  mxDestroyArray(a);
}

//: Read a Sparse Real Matrix from a Matlab file (.mat)
//
//  The matrix type for this function is the following
//  <codeblock>
//  typedef matrix<double,
//                 rectangle<>,
//                 array< compressed<> >,
//                 column_major >::type matlab_sparse;
//  </codeblock>
//!category: utilities
//!component: function
inline void
read_sparse_matlab(matlab_sparse& A, char* matrix_name, const char* file)
{
  MATFile *fp = matOpen(file, "r");
  if (fp == NULL) {
    cout << "could not open file " << file << endl;
    return;
  }
  mxArray *a = matGetArray(fp, matrix_name);
  if (a == NULL) {
    cout << "could not find matrix " << matrix_name << endl;
    return;
  }
  assert(mxIsSparse(a));
  int M = mxGetM(a);
  int N = mxGetN(a);
  int nz = mxGetNzmax(a); /* maybe use mxGetNumberOfElements */
  int* Ir = mxGetIr(a);
  int* Jc = mxGetJc(a);
  double* data = (double*)mxGetData(a);

  A = matlab_sparse(M, N);

  for (int j = 0; j < N; ++j) {
    for (int i = Jc[j]; i < Jc[j+1]; ++i) {
      A(Ir[i], j) = data[i];
    }
  }
  matClose(fp);
  mxDestroyArray(a);

}

//: Write a Sparse Real Matrix to a Matlab file (.mat)
//
//  The matrix type for this function is the following
//  <codeblock>
//  typedef matrix< double,
//                  rectangle<>,
//                  array< compressed<> >,
//                  column_major >::type matlab_sparse;
//  </codeblock>
//!category: utilities
//!component: function
inline void
write_sparse_matlab(matlab_sparse& A, char* matrix_name, const char* file)
{
  MATFile *fp = matOpen(file, "w");
  if (fp == NULL) {
    cout << "could not open file " << file << endl;
    return;
  }
  int M = A.nrows();
  int N = A.ncols();
  int nz = A.nnz();

  mxArray* a = mxCreateSparse(A.nrows(), A.ncols(), A.nnz(), mxREAL);
  mxSetName(a, matrix_name);

  int* Ir = mxGetIr(a);
  int* Jc = mxGetJc(a);
  double* data = (double*)mxGetPr(a);

  int i = 0;
  int j = 0;

  matlab_sparse::iterator A_j;
  matlab_sparse::Column::iterator A_ji;

  for (A_j = A.begin(); A_j != A.end(); ++A_j) {
    Jc[j] = i;
    for (A_ji = (*A_j).begin(); A_ji != (*A_j).end(); ++A_ji) {
      data[i] = *A_ji;
      Ir[i] = A_ji.index();
      ++i;
    }
    ++j;
  }
  Jc[j] = i;

  if (matPutArray(fp, a) != 0)
    cout << "error in writing mat file" << endl;

  matClose(fp);
  mxDestroyArray(a);
}


} /* namespace mtl */

#endif
