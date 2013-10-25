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


#ifndef MATRIX_MARKET_STREAM_H
#define MATRIX_MARKET_STREAM_H

#include <stdio.h>
#include "mtl_complex.h"
#include <assert.h>

#include "mtl_config.h"
#include "entry.h"
#include "mmio.h"

namespace mtl {

using std::complex;

//: A Matrix File Input Stream for Matrix Market Files
//
//   This class simplifies the job of creating matrices from files
//   stored in the Matrix Market format. All matrix types have a
//   constructor that takes a <tt>matrix_market_stream</tt>
//   object. One can also access the elements (of type entry2) from a
//   matrix stream using the stream operator. The stream handles both
//   real and complex numbers.
//
// <codeblock>
//    Usage:
//      matrix_market_stream mms( fielname );
//      Matrix A(mms);
// </codeblock>
//!component: type
//!category: utilities
//!tparam: T - the matrix element type (double or complex<double>)
template <class T>
class matrix_market_stream {
public:
  //: Construct from filename
  inline matrix_market_stream(char* filename) {
    fin = fopen(filename, "r");
    if (fin==0) {
      std::cerr << "Sorry, we can not open " << filename << std::endl;
      assert(0);
    }

    if (mm_read_banner(fin, &matcode) != 0) {
      std::cerr << "Sorry, we cannnot find the matrix market banner in "
	   << filename << std::endl;
      fclose(fin);
      assert(0);
    }

    if (mm_is_coordinate(matcode) == 0 || mm_is_matrix(matcode) == 0) {
      std::cout << "file is not coordinate storage or is not a matrix" << std::endl;
      fclose(fin);
      assert(0);
    }

    if (mm_is_pattern(matcode)) {
      std::cout << "not currently supporting pattern" << std::endl;
      fclose(fin);
      assert(0);
    }

    if (mm_is_skew(matcode)) {
      std::cout << "not currently supporting skew symmetric" << std::endl;
      fclose(fin);
      assert(0);
    }


    isSymmetric = false;
    isComplex = false;
    isHermitian = false;

    if ( mm_is_symmetric(matcode) || mm_is_hermitian(matcode) )
      isSymmetric = true;
    if ( mm_is_hermitian(matcode) )
      isHermitian = true;

    if (mm_is_complex(matcode)) isComplex = true;

    mm_read_mtx_crd_size(fin, &row, &col, &nz);

    count = 0;
  }

  //: Destructor, closes the file
  inline ~matrix_market_stream() {
    fclose(fin);
  }
  //: At the end of the file yet?
  inline bool eof() const { return count == nz; }
  //: Number of rows in matrix
  inline int nrows() const { return row; }
  //: Number of columns in matrix
  inline int ncols() const { return col; }
  //: Number of non-zeroes in matrix
  inline int nnz() const { return isSymmetric ? 2*nz : nz; }

  /* SGI compiler doesn't like these
  template <class Type, class PR_>
  friend
  matrix_market_stream<Type>& operator>>(matrix_market_stream<Type>& mms, entry2<PR_>& e);

  template <typename Type, typename PR_>
  friend
  matrix_market_stream<Type>& operator>>(matrix_market_stream<Type>& mms, entry2<complex<PR_> >& e);
  */

  inline bool is_symmetric() const { return isSymmetric; }

  inline bool is_complex() const { return isComplex; }

  inline bool is_hermitian() const { return isHermitian; }



  /* private: JGS friends not working for some compilers */

  int count;
  FILE *fin;
  MM_typecode matcode;
  int row, col, nz;
  bool isComplex;
  bool isSymmetric;
  bool isHermitian;
};


template <typename T, typename PR_>
inline matrix_market_stream<T>&
operator>>(matrix_market_stream<T>& mms, entry2<PR_>& e) {
  if ( mms.eof() ) {
    e = entry2<PR_>();
    return mms;
  }

    if ( ! mms.is_complex() ) {  /* check the nuerical type */
      fscanf(mms.fin, "%d %d %lg",  &e.row, &e.col, &e.value);
      e.row--; e.col--;
    } else {
      std::cout << "Numerical types of Matrix and entry are incompatible." << std::endl;
      assert(0);
    }

    mms.count++;

    return mms;
  }


#if _MSVCPP_

inline matrix_market_stream<float>&
operator>>(matrix_market_stream<float>& mms, entry2<complex<float> >& e) {
  if ( mms.eof() ) {
    e = entry2<complex<float> >();
    return mms;
  }

  if ( mms.is_complex() ) {  //check the nuerical type
    float r, i;
    fscanf(mms.fin, "%d %d %lg %lg",  &e.row, &e.col, &r, &i);
    e.row--; e.col--;
    e.value = complex<float>(r, i);
  } else {
    std::cout << "Numerical types of Matrix and entry are incompatible." << std::endl;
    assert(0);
  }

  mms.count++;

  return mms;
}

inline matrix_market_stream<double>&
operator>>(matrix_market_stream<double>& mms, entry2<complex<double> >& e) {
  if ( mms.eof() ) {
    e = entry2<complex<double> >();
    return mms;
  }

  if ( mms.is_complex() ) {  //check the nuerical type
    double r, i;
    fscanf(mms.fin, "%d %d %lg %lg",  &e.row, &e.col, &r, &i);
    e.row--; e.col--;
    e.value = complex<double>(r, i);
  } else {
    std::cout << "Numerical types of Matrix and entry are incompatible." << std::endl;
    assert(0);
  }

  mms.count++;

  return mms;
}

#else

template <typename T, typename PR_>
inline matrix_market_stream<T>&
operator>>(matrix_market_stream<T>& mms, entry2<complex<PR_> >& e) {
  if ( mms.eof() ) {
    e = entry2<complex<PR_> >();
    return mms;
  }

  if ( mms.is_complex() ) {  //check the nuerical type
    PR_ r, i;
    fscanf(mms.fin, "%d %d %lg %lg",  &e.row, &e.col, &r, &i);
    e.row--; e.col--;
    e.value = complex<PR_>(r, i);
  } else {
    std::cout << "Numerical types of Matrix and entry are incompatible." << std::endl;
    assert(0);
  }

  mms.count++;

  return mms;
}

#endif

} /* namespace mtl */

#endif
