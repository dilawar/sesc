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

#ifndef MTL_INITIALIZE_H
#define MTL_INITIALIZE_H

#include <iostream>
#include "matrix_stream.h"

#include "entry.h"
#include "mtl_complex.h"
#include "conj.h"
#include "mtl_set.h"

namespace mtl {

using std::complex;
using std::conj;

class symmetric_tag;

//need add mmio.c on using
template <class Matrix, class T>
void
__initialize(Matrix& A, matrix_market_stream<T>& s,
	     symmetric_tag)
{
  typedef typename Matrix::value_type VT;
  entry2<VT> e;

  if ( s.is_symmetric() ) {
    while( ! s.eof() ) {
      s >> e;
      int row = e.row;
      int col = e.col;
      A(row, col) = e.value;
    }
  } else {
    std::cout << " matrix type is symmetric but the matrix in the file is not" << std::endl;
//    assert(0);
  }
}

template <class Matrix, class ANY_TAG, class T>
void
__initialize(Matrix& A, matrix_market_stream<T>& s,
	     ANY_TAG)
{
  typedef typename Matrix::value_type VT;
  entry2<VT> e;

  if ( s.is_symmetric() ) {
    while( ! s.eof() ) {
      s >> e;
      int row = e.row;		// g++ internal compiler error
      int col = e.col;		//      A(e.row, e.col) = e.value;
      A(row, col) = e.value;
      if ( s.is_hermitian() )
	A(e.col, e.row) = std::conj(e.value);
      else
	A(e.col, e.row) = e.value;
    }
  } else {
    while( ! s.eof() ) {
      s >> e;
      int row = e.row;
      int col = e.col;
      A(row, col) = e.value;
    }
  }
}

template <class Matrix, class TT>
void
initialize(Matrix& A, matrix_market_stream<TT>& s)
{
  typedef typename Matrix::value_type T;
  typedef typename Matrix::shape Shape;
  mtl::set_value(A, T(0));
  mtl::__initialize(A, s, Shape());
}

//need add iohb.c on using
template <class Matrix, class TT>
void
initialize(Matrix& A, harwell_boeing_stream<TT>& s)
{ //for hbs, shape doesn't matter
  typedef typename Matrix::size_type Int;
  typedef typename Matrix::value_type T;

  mtl::set_value(A, T(0));
  entry2<T> e;

  while( ! s.eof() ) {
    s >> e;
    Int row = e.row;
    Int col = e.col;
    A(row, col) = e.value;
  }
}

} /* namespace mtl */

#endif
