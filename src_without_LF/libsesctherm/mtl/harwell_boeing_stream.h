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
#ifndef HARWELL_BOEING_STREAM_H
#define HARWELL_BOEING_STREAM_H

#include <stdlib.h>
#include <assert.h>
#include "mtl_config.h"
#include "mtl_complex.h"
#include "iohb.h"
#include "entry.h"

namespace mtl {

using std::complex;

//: A Matrix File Input Stream for Harwell-Boeing Matrix Files
//
//  This class simplifies the job of creating matrices from files stored
//  in the Harwell-Boeing format. All matrix types have a constructor that
//  takes a <tt> harwell_boeing_stream </tt> object. One can also access the
//  elements from a matrix stream using  <tt>operator>>()</tt>. The stream
//  handles both real and complex numbers.
//
// <codeblock>
//   Usage:
//     harwell_boeing_stream mms( fielname );
//     Matrix A(mms);
//
// </codeblock>
//
//!component: type
//!category: utilities
//!tparam: T - the matrix element type (double or complex<double>)
template <class T>
class harwell_boeing_stream {
public:
  //: Construct from file name
  harwell_boeing_stream(char* filename) {
    int Nrhs;
    char* Type;
    Type = new char[4];
    isComplex = false;
    readHB_info(filename, &M, &N, &nonzeros, &Type, &Nrhs);
    colptr = (int *)malloc((N+1)*sizeof(int));
    if ( colptr == NULL ) IOHBTerminate("Insufficient memory for colptr.\n");
    rowind = (int *)malloc(nonzeros*sizeof(int));
    if ( rowind == NULL ) IOHBTerminate("Insufficient memory for rowind.\n");

    if ( Type[0] == 'C' ) {
      isComplex = true;
      val = (double *)malloc(nonzeros*sizeof(double)*2);
      if ( val == NULL ) IOHBTerminate("Insufficient memory for val.\n");

    } else {
      if ( Type[0] != 'P' ) {
	val = (double *)malloc(nonzeros*sizeof(double));
	if ( val == NULL ) IOHBTerminate("Insufficient memory for val.\n");
      }
    }

    readHB_mat_double(filename, colptr, rowind, val);

    cnt = 0;
    col = 0;
    delete [] Type;
  }
  //: Destructor
  ~harwell_boeing_stream() {
    free(colptr);
    free(rowind);
    free(val);
  }
  //: Number of rows in matrix
  inline int nrows() const { return M; }

  //: Number of columns in matrix
  inline int ncols() const { return N; }

  //: Number of non-zeroes in matrix
  inline int nnz() const { return nonzeros; }

  //: At the end of the file?
  inline bool eof() { return cnt == nonzeros; }

  inline bool is_complex() { return isComplex; }


  /* JGS SGI Compiler Doesn't like this
  template <class Type, class PR_>
    friend
    harwell_boeing_stream<Type>&
    operator>>(harwell_boeing_stream<Type>& hbs, entry2<PR_>& e);
  template <class Type, class PR_>
    friend
    harwell_boeing_stream<Type>&
    operator>>(harwell_boeing_stream<Type>& hbs, entry2<complex<PR_> >& e);
    */


  /*  JGS see above */
  int cnt;
  int col; /* use it in >> to refer to currrent col index */
  int* colptr;
  bool isComplex;
  int M;
  int N;
  int nonzeros;
  int* rowind;
  double* val;

};

template <class T, class PR_>
harwell_boeing_stream<T>&
operator>>(harwell_boeing_stream<T>& hbs, entry2<PR_>& e)
{
  int hbs_cnt, hbs_col;
  if ( hbs.eof() ) {
    e = entry2<PR_>();
    return hbs;
  }
  hbs_cnt = hbs.cnt;
  hbs_col = hbs.col;
  if ( hbs.colptr[hbs_col+1]-1 <= hbs_cnt )
    hbs.col++; /* change to another col */

  if (!hbs.isComplex) {
    int row = hbs.rowind[hbs.cnt] - 1;
    int col = hbs.col;
    e.row = row;
    e.col = col;
    e.value = hbs.val[hbs.cnt];
  } else {
    std::cout << "Numerical types of Matrix and entry are incompatible."
	      << std::endl;
    assert(0);
  }

  hbs.cnt++;

  return hbs;

}

#if _MSVCPP_

inline harwell_boeing_stream<float>&
operator>>(harwell_boeing_stream<float>& hbs, entry2<complex<float> >& e)
{

  if ( hbs.eof() ) {
    e = entry2<complex<float> >();
    return hbs;
  }
  if ( hbs.colptr[hbs.col+1]-1 <= hbs.cnt ) hbs.col++; /* change to another col */

  if (hbs.isComplex) {
    e.row = hbs.rowind[hbs.cnt] - 1;
    e.col = hbs.col;
    e.value = complex<float>(hbs.val[2*hbs.cnt], hbs.val[2*hbs.cnt+1]);

  } else {
    std::cout << "Numerical types of Matrix and entry are incompatible."
	      << std::endl;
    assert(0);
  }

  hbs.cnt++;

  return hbs;

}


inline harwell_boeing_stream<double>&
operator>>(harwell_boeing_stream<double>& hbs, entry2<complex<double> >& e)
{

  if ( hbs.eof() ) {
    e = entry2<complex<double> >();
    return hbs;
  }
  if ( hbs.colptr[hbs.col+1]-1 <= hbs.cnt ) hbs.col++; /* change to another col */

  if (hbs.isComplex) {
    e.row = hbs.rowind[hbs.cnt] - 1;
    e.col = hbs.col;
    e.value = complex<double>(hbs.val[2*hbs.cnt], hbs.val[2*hbs.cnt+1]);

  } else {
    std::cout << "Numerical types of Matrix and entry are incompatible."
	      << std::endl;
    assert(0);
  }

  hbs.cnt++;

  return hbs;

}

#else

template <class T, class PR_>
harwell_boeing_stream<T>&
operator>>(harwell_boeing_stream<T>& hbs, entry2<complex<PR_> >& e)
{

  if ( hbs.eof() ) {
    e = entry2<complex<PR_> >();
    return hbs;
  }
  if ( hbs.colptr[hbs.col+1]-1 <= hbs.cnt ) hbs.col++; /* change to another col */

  if (hbs.isComplex) {
    e.row = hbs.rowind[hbs.cnt] - 1;
    e.col = hbs.col;
    e.value = complex<PR_>(hbs.val[2*hbs.cnt], hbs.val[2*hbs.cnt+1]);

  } else {
    std::cout << "Numerical types of Matrix and entry are incompatible."
	      << std::endl;
    assert(0);
  }

  hbs.cnt++;

  return hbs;

}

#endif

} /* namespace mtl */

#endif
