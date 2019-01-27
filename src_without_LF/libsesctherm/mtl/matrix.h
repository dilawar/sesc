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


#ifndef _MTL_MATRIX_H_
#define _MTL_MATRIX_H_


/* namespace polution from <sys/sysmacros.h> */
#undef major
#undef minor

#include <set>
#include <list>
#include <vector>

#include "meta_equal.h"
#include "meta_if.h"

#include "matrix_traits.h"
#include "compressed1D.h"

#include "matrix_implementation.h"
#include "rect_indexer.h"
#include "banded_indexer.h"
#include "diagonal_indexer.h"

#include "dense2D.h"
#include "array2D.h"
#include "compressed2D.h"
#include "envelope2D.h"

#include "linalg_vec.h"
#include "sparse1D.h"
#include "entry.h"
#include "uplo.h"

namespace mtl {

  /* Shapes */

  //: rectangle shape type selector
  //
  // A MTL rectangular matrix is one in which elements could appear in
  // any position in the matrix, i.e., there can be any element
  // <tt>A(i,j)</tt> where <tt>0 <= i <= M</tt> and <tt>0 <= i <=
  // M</tt>.  Both dense and sparse matrices can fit into this
  // category.  The compatible storage types are <tt>dense</tt>,
  // <tt>compressed</tt>, and <tt>array</tt>.  If the matrix size is known
  // at compile time, the matrix size can be built in to the shape
  // (for dense external matrices). This allows some algorithms to
  // optimize for small matrices.  Here are a few examples of creating
  // rectangular matrix types:
  //
  // <codeblock>
  // typedef matrix < double,
  //                  rectangle<>,
  //                  dense<>,
  //                  column_major >::type FortranMatrix;
  //
  // typedef matrix < double,
  //                  rectangle<>,
  //                  compressed<>,
  //                  row_major >::type CSR;
  //
  // typedef matrix < double,
  //                  rectangle<4,4>,
  //                  dense<>,
  //                  row_major >::type Quaternion;
  //
  // typedef matrix < double,
  //                  rectangle<>,
  //                  array< compressed<> >,
  //                  row_major >::type SparseArrayMat;
  // </codeblock>
  //
  //!tparam: MM - The number of rows of the matrix, if the matrix has static size (known at compile time) - not static
  //!tparam: NN - The number of columns of the matrix, if the matrix has static size (known at compile time) - not static
  //!component: type
  //!category: containers,selectors
  //!definition: matrix.h
  template <int MM = 0, int NN = 0>
  class rectangle {
  public:
    enum { M = MM, N = NN, id = RECT, uplo };
  };

  //: banded shape type selectors, also banded storage type selectors
  //
  // <h4>Shape Type Selectors</h4>
  // A banded shape matrix is one in which non-zero matrix elements only
  // appear within a ``band'' of a matrix. The bandwidth of a matrix is
  // described by the number of diagonals in the band that are below the
  // main diagonal, refered to as the <i>sub</i> diagonals, and the number
  // of diagonals in the band above the main diagonal, refered to as the
  // <i>super</i> diagonals. The following is an example of a matrix with a
  // bandwidth of (1,2).
  //
  // <codeblock>
  // [  1   2   3   0   0 ]
  // [  4   5   6   7   0 ]
  // [  0   8   9  10  11 ]
  // [  0   0  12  13  14 ]
  // [  0   0   0  15  16 ]
  // </codeblock>
  //
  // There are many storage types that can be used to efficiently
  // represent banded matrices. The MTL storage types that can be
  // used are <tt>banded</tt>, <tt>packed</tt>, <tt>banded_view</tt>,
  // and <tt>array</tt>. Here are some examples of creating
  // banded matrix types:
  //
  // <codeblock>
  // typedef matrix < double,
  //                  banded<>,
  //                  packed<>,
  //                  column_major >::type BLAS_Packed;
  //
  // typedef matrix < double,
  //                  banded<>,
  //                  banded<>,
  //                  column_major >::type BLAS_Banded;
  // </codeblock>
  //
  // <h4>Storage Type Selectors</h4>
  // <tt>banded</tt> is also the type selectors for the banded storage format.
  // This storage format is equivalent to the banded storage used in the
  // BLAS and LAPACK. Similar to the <tt>dense</tt> storage format, a
  // single contiguous chunk of memory is allocated. The banded storage
  // format maps the bands of the matrix to a twod-array of dimension (sub
  // + super + 1) by min(M, N + sub). In MTL the 2D array can be row or
  // column major (for the BLAS it is always column major). The twod-array
  // is then in turn mapped the the linear memory space of the single chunk
  // of memory. The following is an example banded matrix with the mapping
  // to the row-major and column-major 2D arrays. The x's represent
  // memory locations that are not used.
  //
  // <codeblock>
  // [  1   2   3   0   0   0  ]
  // [  4   5   6   7   0   0  ]
  // [  0   8   9  10  11   0  ]
  // [  0   0  12  13  14  15  ]
  // [  0   0   0  16  17  18  ]
  // [  0   0   0   0  19  20  ]
  //
  // row-major
  // [  1   2   3   x  ]
  // [  4   5   6   7  ]
  // [  8   9  10  11  ]
  // [ 12  13  14  15  ]
  // [  x  16  17  18  ]
  // [  x   x  19  20  ]
  //
  // column-major
  // [  x   x   3   7  11  15  ]
  // [  x   2   6  10  14  18  ]
  // [  1   5   9  13  17  20  ]
  // [  4   8  12  16  19   x  ]
  // </codeblock>
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  template <int MemLoc=internal>
  struct banded {
    typedef int size_type;
    enum { id = BAND, oned_id, uplo, ext=MemLoc, M=0, N=0,
           issparse=0, index };
  };

  //: diagonal shape type selectors
  //
  // The diagonal matrix shape is similar to the banded matrix in that
  // there is a bandwidth that describes the area of the matrix in which
  // non-zero matrix elements can reside. The difference between the banded
  // matrix shape lies in how the MTL iterators traverse the matrix, which
  // is explained in DiagonalMatrix. The MTL storage types that
  // can be used are <tt>banded</tt>, <tt>packed</tt>,
  // <tt>banded_view</tt>, and <tt>array</tt>. To get the traditional
  // tridiagonal matrix format, one just has to specify the bandwith to be
  // (1,1) and use the <tt> array &lt; dense &lt; &gt; &gt;</tt> storage format.
  //
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  template <int MemLoc=internal>
  struct diagonal {
    enum { uplo, id = DIAG, ext=MemLoc, M=0, N=0 };
  };

  //: triangle shape type selectors
  //
  // The triangular shape is a special case of the banded shape. There
  // are four kinds of triangular matrices in MTL, based on the
  // <tt>Uplo</tt> argument:
  //
  // <table border=1>
  // <tr> <td> Uplo type </td> <td> Sub </td> <td> Super </td> </tr>
  // <tr> <td> upper </td> <td> 0 </td> <td> N - 1 </td> </tr>
  // <tr> <td> unit_upper </td> <td> -1 </td> <td> N - 1 </td> </tr>
  // <tr> <td> lower </td> <td> M - 1 </td> <td> 0 </td> </tr>
  // <tr> <td> unit_lower </td> <td> M - 1 </td> <td> -1 </td> </tr>
  // </table>
  //
  // The following is an example of a <tt>triangle&lt;upper&gt;</tt> shaped
  // matrix:
  //
  // <codeblock>
  // [  1   2   3   4   5  ]
  // [  0   6   7   8   9  ]
  // [  0   0  10  11  12  ]
  // [  0   0   0  13  14  ]
  // [  0   0   0   0  15  ]
  // </codeblock>
  //
  // The next example is of a <tt>triangle&lt;unit_lower&gt;</tt>
  // matrix.  The main diagonal is not stored, since it consists of
  // all ones.  The MTL algorithms recognize when a matrix is ``unit''
  // and perform a slightly different operation to take this into
  // account.  The ones will not show up in an iteration of the
  // matrix, and access to the A(i,i) element of a unit lower/upper
  // matrix is an error.
  //
  // <codeblock>
  // [  1   0   0   0   0  ]
  // [  1   1   0   0   0  ]
  // [  2   3   1   0   0  ]
  // [  4   5   6   1   0  ]
  // [  7   8   9  10   1  ]
  // </codeblock>
  //
  //
  // Here are a couple examples of creating some triangular matrix types:
  //
  // <codeblock>
  // typedef matrix < double,
  //                  triangle<upper>,
  //                  banded<>,
  //                  column_major >::type UpperTriangle;
  //
  // typedef matrix < double,
  //                 triangle<unit_lower>,
  //                 packed<>,
  //                 row_major >::type UnitLowerTriangle;
  // </codeblock>
  //
  //!tparam: Uplo - The type of triangular matrix. Either upper, lower, unit_upper, or unit_lower.
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  template <int Uplo = dynamic_uplo>
  struct triangle {
    enum { id = TRI, uplo = Uplo, M=0, N=0 };
  };



  //: symmetric shape type selectors
  //
  // Symmetric matrices are similar to banded matrices in that there
  // is only access to a particular band of the matrix. The difference
  // is that in an MTL symmetric matrix, <tt>A(i,j)</tt> and
  // <tt>A(j,i)</tt> refer to the same element. The following is an
  // example of a symmetric matrix:
  //
  // <codeblock>
  // the full symmetric matrix
  // [  1   2   3   4   5  ]
  // [  2   6   7   8   9  ]
  // [  3   7  10  11  12  ]
  // [  4   8  11  13  14  ]
  // [  5   9  12  14  15  ]
  //
  // the symmetric matrix in packed storage
  // [  1  ]
  // [  2   6  ]
  // [  3   7  10  ]
  // [  4   8  11  13  ]
  // [  5   9  12  14  15  ]
  // </codeblock>
  //
  // Similar to the triangle shape, the user must provide an
  // <tt>Uplo</tt> argument which specifies which part of the matrix is
  // actually stored.  The valid choices are <tt>upper</tt> and
  // <tt>lower</tt> for symmetric matrices.
  //
  // <codeblock>
  // typedef matrix < double,
  //                  symmetric<lower>,
  //                  packed<>,
  //                  row_major >::type SymmMatrix;
  // </codeblock>
  //
  //!component: type
  //!category: containers, selectors
  //!tparam: Uplo - The portion of the matrix that is stored. Either upper or lower.
  //!example: symm_packed_vec_prod.cc, symm_banded_vec_prod.cc
  template <int Uplo = dynamic_uplo>
  struct symmetric {
    enum { id = SYMM, uplo = Uplo, M=0, N=0 };
  };

  //: hermitian shape type selectors
  // Hermitian matrices are not yet implemented.
  //!component: type
  //!category: containers, selectors
  //!tparam: Uplo - The portion of the matrix that is stored. Either upper or lower.
  //!definition: matrix.h
  template <int Uplo = dynamic_uplo>
  struct hermitian {
    enum { id = HERM, uplo = Uplo, M=0, N=0 };
  };

  /* Ordering */
  //: Row major ordering type selectors
  //!category: containers, selectors
  struct row_major {
    enum { id = ROW_MAJOR };
  };

  //: Column major ordering type selectors
  //!category: containers, selectors
  struct column_major {
    enum { id = COL_MAJOR };
  };

  /* Storage */

  //: Dense storage type selectors (for both TwoD and OneD storage)
  //
  // <h4>TwoD Storage Type Selectors</h4>
  //This is the most common way of storing matrices, and consists of
  //one contiguous piece of memory that is divided up into rows or
  //columns of equal length.  The following example shows how a matrix
  //can be mapped to linear memory in either a row-major or
  //column-major fashion.
  //
  // <codeblock>
  // [ 1 2 3 ]
  // [ 4 5 6 ]
  // [ 7 8 9 ]
  //
  // row major:
  // [ 1 2 3 4 5 6 7 8 9 ]
  //
  // column major:
  // [ 1 4 7 2 5 8 3 6 9 ]
  // </codeblock>
  //
  // <h4>OneD Storage Type Selectors</h4>
  // This specifies a normal dense vector to be used as the OneD
  // part of matrix with array storage.
  //
  //
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!component: type
  //!category: containers, selectors
  //!example: swap_rows.cc, general_matvec_mult.cc
  //!definition: matrix.h
  template <int MemLoc=internal>
  struct dense {
    typedef int size_type;
    enum { id = DENSE, oned_id, ext=MemLoc, issparse=0, index };
  };


  //: Packed storage type selectors
  // This storage type is equivalent to the BLAS/LAPACK packed
  // storage format.
  //
  // The packed storage format is similar to the banded format,
  // except that the storage for each row/column of the band
  // is variable so there is no wasted space. This is better
  // for efficiently storing triangular matrices.
  //
  // <codeblock>
  // [  1   2   3   4   5  ]
  // [  0   6   7   8   9  ]
  // [  0   0  10  11  12  ]
  // [  0   0   0  13  14  ]
  // [  0   0   0   0  15  ]
  //
  // [  1   2   3   4   5  ]
  // [  6   7   8   9  ]
  // [  10  11  12  ]
  // [  13  14  ]
  // [  15  ]
  //
  // mapped to linear memory with row-major order:
  //
  // [  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15  ]
  // </codeblock>
  //
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!component: type
  //!category: containers, selectors
  //!example: tri_pack_vect.cc
  template <int MemLoc=internal>
  struct packed {
    typedef int size_type;
    enum { id = PACKED, oned_id, ext=MemLoc, issparse=0, index };
  };

  //: Banded view storage type selectors
  // This storage type is used for creating matrices that
  // are "views" into existing full dense matrices.
  // For instance, one could create a triangular view
  // of a full matrix.
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  template <int MemLoc=internal>
  struct banded_view {
    typedef int size_type;
    enum{id = BAND_VIEW,oned_id, ext=MemLoc, issparse=0, index };
  };

  //: Compressed sparse storage type selectors (for both OneD and TwoD)
  //
  // <h4> TwoD Storage Type Selectors</h4>
  // This storage type is the traditional compressed row
  // or compressed column format.
  //
  // The storage consists of three arrays, one array for
  // all of the elements, one array consisting of the
  // row or column index (row for column-major and column
  // for row-major matrices), and one array consisting
  // of pointers to the start of each row/column.
  //
  // The following is an example sparse matrix in compressed for
  // format, with the stored indices specified as
  // <tt>index_from_one</tt>. Note that the MTL interface is still
  // indexed from zero whether or not the underlying stored indices
  // are from one.
  //
  // <codeblock>
  // [  1      2   3      ]
  // [     4           5  ]
  // [  6      7   8      ]
  // [  9         10      ]
  // [        11      12  ]
  //
  // row pointer array
  // [  1  4  6  9  11 13 ]
  //
  // element value array
  // [  1  2  3  4  5  6  7  8  9 10 11 12 ]
  //
  // element column index array
  // [  1  3  4  2  5  1  3  4  1  4  3  5 ]
  // </codeblock>
  //
  // Of course, the user of the MTL sparse matrix does not
  // need to concern his or herself with the implementation
  // details of this matrix storage format. The interface
  // to an MTL compressed row matrix is the same as that
  // of any MTL matrix, as described in Matrix.
  //
  // <h4> OneD Storage Type Selectors</h4>
  // This is a OneD type used to construct array matrices.
  // The compressed OneD format uses two arrays, one to hold
  // the elements of the vector, and the other to hold the
  // indices that coorespond to each element (either their
  // row or column number).
  //
  //
  //!tparam: SizeType - The type used in the index and pointer array - int
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!tparam: IndexStyle - Specify whether the underlying index array stores indices starting from one (fortan style) or from zero (c-style)  - index_from_zero
  //!component: type
  //!category: containers, selectors
  //!example: sparse_matrix.cc
  //!definition: matrix.h
  template <class SizeType = int, int MemLoc=internal,
            int IndexStyle = index_from_zero>
  struct compressed {
    typedef SizeType size_type;
    enum { id = COMPRESSED, oned_id, ext=MemLoc,
           issparse=1, index=IndexStyle };
  };

  //: Array storage type selectors
  //
  // This storage type gives an "array of pointers" style
  // implementation of a matrix. Each row or column of the matrix
  // is allocated separately. The type of vector used for the rows
  // or columns is very flexible, and one can choose from any of the
  // OneD storage types, which include <tt>dense</tt>,
  // <tt>compressed</tt>, <tt>sparse_pair</tt>, <tt>tree</tt>,
  // and <tt>linked_list</tt>.
  //
  // <codeblock>
  // matrix < double,
  //          rectangle<>,
  //          array< dense<> >,
  //          row_major >::type
  // [ ] -> [  1  0  0  4  0 ]
  // [ ] -> [  0  7  8  0  0 ]
  // [ ] -> [ 11  0 13 14  0 ]
  // [ ] -> [ 16  0 18  0 20 ]
  // [ ] -> [  0 22  0 24  0 ]
  //
  // matrix < double,
  //          rectangle<>,
  //          array< sparse_pair<> >,
  //          row_major >::type
  // [ ] -> [ (1,0) (4,3) ]
  // [ ] -> [ (7,1) (8,2) ]
  // [ ] -> [ (11,0) (13,2) (14,3) ]
  // [ ] -> [ (16,0) (18,2) (20,4) ]
  // [ ] -> [ (22,1) (24,3) ]
  // </codeblock>
  //
  // <p>One advantage of this type of storage is that rows can be
  // swapped in constant time. For instance, one could swap the
  // row 3 and 4 of a matrix in the following way.
  //
  // <codeblock>
  // Matrix::OneD tmp = A[3];
  // A[3] = A[4];
  // A[4] = tmp;
  // </codeblock>
  //
  // The rows are individually reference counted so that the user
  // does not have to worry about deallocating the rows.
  //
  //!tparam: OneD - The storage type used for each row/column of the matrix - dense
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!component: type
  //!category: containers, selectors
  //!models: TwoDStorage
  //!definition: matrix.h
  template <class OneD = dense<>, int MemLoc=internal>
  struct array {
    typedef typename OneD::size_type size_type;
    enum { id=ARRAY, oned_id=OneD::id, ext=MemLoc,
           issparse = OneD::issparse, index=index_from_zero };
  };

  //: Envelope storage type selectors
  //
  // The storage scheme is for sparse symmetric matrices, where most
  // of the non-zero elements fall near the main diagonal.  The
  // storage format is useful in certain factorizations since the
  // fill-ins fall in areas already allocated.  This scheme is
  // different than most sparse matrices since the row containers are
  // actually dense, similar to a banded matrix.
  //
  // <codeblock>
  // [  1              ]
  // [  2  3           ]
  // [  4     5        ]
  // [        6  7     ]
  // [     8     9  10 ]
  //
  //
  //          [ 0  2  5  7 11 ]    Diagonals pointer array
  //    _______/__/   |  |__\___________
  //   V     V        V     V           V
  // [ 1  2  3  4  0  5  6  7  8  0  9 10 ] Element values array
  //
  // </codeblock>
  //!tparam: MemLoc - Specify whether the memory used is "owned" by the matrix or if it was provided to the matrix from some external source (with a pointer to some data) - internal
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  template <int MemLoc=internal>
  struct envelope {
    typedef int size_type;
    enum { id = ENVELOPE, oned_id, ext=MemLoc, issparse = 0, index };
  };

  /* OneD Storage*/


  //: Index-Value Pair Sparse Vector implemented with mtl::dense1D
  // This is a OneD type for constructing array matrices.
  // The implementation is a mtl::dense1D consisting of index-value
  // pairs.
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  struct sparse_pair {
    typedef int size_type;
    enum { id = SPARSE_PAIR, issparse = 1, index=index_from_zero };
  };

  //: Index-Value Pair Sparse Vector implemented with std::set
  // This is a OneD type for constructing array matrices.
  // The implementation is a std::set consisting of index-value
  // pairs.
  //
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  struct tree {
    typedef int size_type;
    enum { id = TREE, issparse = 1, index=index_from_zero};
  };

  //: Index-Value Pair Sparse Vector implemented with std::list
  // This is a OneD type for constructing array matrices.
  // The implementation is a std::list consisting of index-value
  // pairs.
  //
  //!component: type
  //!category: containers, selectors
  //!definition: matrix.h
  struct linked_list {
    typedef int size_type;
    enum { id = LINKED_LIST, issparse = 1, index=index_from_zero };
  };

  //: Matrix Type Generators Error
  // If you see this in a compiler error message it means
  // you made a mistake in selecting arguments for your matrix type.
  //!noindex:
  struct generators_error { };

  //: generators for oned
  //!noindex:
  template <class T, class Storage>
  struct generate_oned {
    enum { Storage_oned_id = Storage::oned_id,
           Storage_index = Storage::index };
    typedef typename IF< EQUAL< Storage_oned_id,DENSE>::RET,
                         dense1D<T>,
                     typename IF< EQUAL< Storage_oned_id,COMPRESSED>::RET,
                         compressed1D<T, typename Storage::size_type,
                                         Storage_index>,
                     typename IF< EQUAL< Storage_oned_id,SPARSE_PAIR>::RET,
                         sparse1D< mtl::dense1D< entry1<T> > >,
                     typename IF< EQUAL< Storage_oned_id,TREE>::RET,
                         sparse1D< std::set< entry1<T> > >,
                     typename IF< EQUAL< Storage_oned_id,LINKED_LIST>::RET,
                         sparse1D< std::list< entry1<T> > >,
                         generators_error
                       >::RET
                       >::RET
                       >::RET
                       >::RET
                       >::RET RET;
  };

  //: blah
  //!noindex:
  template <class T, class Storage, int M, int N>
  struct generate_internal {
    enum { Storage_id = Storage::id, Storage_index = Storage::index };
    typedef typename IF< EQUAL< Storage_id, DENSE>::RET,
                         gen_dense2D<T, gen_rect_offset<M,N>,M,N>,
                     typename IF< EQUAL< Storage_id, COMPRESSED >::RET,
                         gen_compressed2D<T, typename Storage::size_type,
                                          Storage_index,M,N>,
                     typename IF< EQUAL< Storage_id, ENVELOPE >::RET,
                         gen_envelope2D<T,M,N>,
                     typename IF< EQUAL< Storage_id, PACKED >::RET,
                         gen_dense2D<T, gen_packed_offset<M,N>,M,N>,
                     typename IF< EQUAL< Storage_id, BAND >::RET,
                         gen_dense2D<T, gen_banded_offset<M,N>,M,N>,
                     typename IF< EQUAL< Storage_id, BAND_VIEW >::RET,
                         gen_dense2D<T, gen_banded_view_offset<M,N>,M,N>,
                     typename IF< EQUAL< Storage_id, ARRAY >::RET,
                         gen_array2D< typename generate_oned<T,Storage>::RET >,
                         generators_error
                     >::RET
                     >::RET
                     >::RET
                     >::RET
                     >::RET
                     >::RET
                     >::RET RET;
  };

  //: blah
  //!noindex:
  template <class T, class Storage, int M, int N>
  struct generate_external {
    enum { Storage_id = Storage::id, Storage_index = Storage::index };
    typedef typename IF< EQUAL< Storage_id, DENSE>::RET,
                         gen_external2D<T, gen_rect_offset<M,N>, M,N>,
                     typename IF< EQUAL< Storage_id, COMPRESSED >::RET,
                         gen_ext_comp2D<T, typename Storage::size_type,
                                        Storage_index,M,N>,
                     typename IF< EQUAL< Storage_id, ENVELOPE >::RET,
                         gen_envelope2D<T,M,N>,
                     typename IF< EQUAL< Storage_id, PACKED >::RET,
                         gen_external2D<T, gen_packed_offset<M,N>,M,N>,
                     typename IF< EQUAL< Storage_id, BAND >::RET,
                         gen_external2D<T, gen_banded_offset<M,N>,M,N>,
                     typename IF< EQUAL< Storage_id, BAND_VIEW >::RET,
                         gen_external2D<T, gen_banded_view_offset<M,N>,M,N>,
                     typename IF< EQUAL< Storage_id, ARRAY >::RET,
                         gen_array2D< typename generate_oned<T,Storage>::RET >,
                         generators_error
                     >::RET
                     >::RET
                     >::RET
                     >::RET
                     >::RET
                     >::RET
                     >::RET RET;
  };

  //: blah
  //!noindex:
  template <class T, class Storage, int M, int N>
  struct generate_storage {
    enum { Storage_ext = Storage::ext };
    typedef typename IF< Storage_ext,
                         typename generate_external<T,Storage,M,N>::RET,
                         typename generate_internal<T, Storage,M,N>::RET
                       >::RET RET;
  };

  //: blah
  //!noindex:
  template <class T, class Orientation, class Storage, int M, int N>
  struct generate_rect {
    enum { Orientation_id = Orientation::id };
    typedef typename generate_storage<T,Storage,M,N>::RET storage_t;
    typedef typename storage_t::type::size_type size_type;
    typedef typename IF< EQUAL<Orientation_id, ROW_MAJOR>::RET,
                           row_matrix< storage_t,
                              gen_rect_indexer<row_orien,M,N, size_type> >,
                     typename IF< EQUAL<Orientation_id, COL_MAJOR>::RET,
                           column_matrix< storage_t,
                               gen_rect_indexer<column_orien,M,N,size_type> >,
                        generators_error
                       >::RET
                       >::RET RET;
  };

  //: blah
  //!noindex:
  template <class T, class Orientation, class Storage, int M, int N>
  struct generate_banded {
    enum { Orientation_id = Orientation::id };
    typedef typename generate_storage<T,Storage,M,N>::RET storage_t;
    typedef typename storage_t::type::size_type size_type;
    typedef typename IF< EQUAL<Orientation_id, ROW_MAJOR>::RET,
                        row_matrix< storage_t,
                             gen_banded_indexer<row_orien,M,N,size_type> >,
                   typename IF< EQUAL<Orientation_id, COL_MAJOR>::RET,
                          column_matrix< storage_t,
                           gen_banded_indexer<column_orien,N,M,size_type> >,
                           generators_error
                       >::RET
                       >::RET RET;
  };

  //: blah
  //!noindex:
  template <class T, class Storage, int M, int N>
  struct generate_diagonal {
    typedef typename generate_storage<T,Storage,M,N>::RET storage_type;
    typedef typename storage_type::type::size_type size_type;
    typedef gen_diagonal_indexer<row_orien, M, N, size_type> indexer_gen;
    typedef diagonal_matrix<storage_type, indexer_gen> RET;
  };

  //: blah
  //!noindex:
  template <int Uplo>
  struct generate_uplo {
    typedef typename IF< EQUAL< Uplo, upper >::RET, upper__,
          typename IF< EQUAL< Uplo, unit_upper >::RET, unit_upper__,
          typename IF< EQUAL< Uplo, lower>::RET, lower__,
          typename IF< EQUAL< Uplo, unit_lower>::RET, unit_lower__,
#if defined __SUNPRO_CC
          typename IF< EQUAL< Uplo, 100>::RET, dynamic_uplo__,
#endif
          dynamic_uplo__
#if defined __SUNPRO_CC
      >::RET
#endif
      >::RET
      >::RET
      >::RET
      >::RET RET;
  };

  //: blah
  //!noindex:
  template <class T, class Shape, class Orien, class Storage, int M, int N>
  struct generate_triangle {
    enum { Orien_id = Orien::id, Shape_uplo = Shape::uplo,
           Storage_issparse = Storage::issparse };
    typedef typename generate_storage<T,Storage,M,N>::RET storage_type;
    typedef typename storage_type::type::size_type size_type;
    typedef typename
     IF< Storage_issparse,
       typename IF< EQUAL< Orien_id, ROW_MAJOR>::RET,
         triangle_matrix<row_matrix< storage_type,
            gen_banded_indexer<row_orien,M,N,size_type> >,
            typename generate_uplo<Shape_uplo>::RET >,
         triangle_matrix<column_matrix< storage_type,
            gen_banded_indexer<column_orien,M,N,size_type> >,
            typename generate_uplo<Shape_uplo>::RET >
                     >::RET
     ,
       typename IF< EQUAL< Orien_id, ROW_MAJOR>::RET,
         triangle_matrix<row_matrix< storage_type,
            gen_banded_indexer<row_orien,M,N,size_type> >,
            typename generate_uplo<Shape_uplo>::RET >,
         triangle_matrix<column_matrix< storage_type,
            gen_banded_indexer<column_orien,M,N,size_type> >,
            typename generate_uplo<Shape_uplo>::RET >
                     >::RET
     >::RET RET;
  };


  /* if storage is sparse, use rect indexer, otherwise use
   banded indexer */

  //: blah
  //!noindex:
  template <class T, class Shape, class Orien, class Storage, int M, int N>
  struct generate_symmetric {
    enum { Shape_uplo = Shape::uplo, Orien_id = Orien::id,
           Storage_issparse = Storage::issparse };
    typedef typename generate_storage<T,Storage,M,N>::RET storage_type;
    typedef typename storage_type::type::size_type size_type;
    typedef typename
     IF< Storage_issparse,
       typename IF< EQUAL< Orien_id, ROW_MAJOR>::RET,
       symmetric_matrix<row_matrix< storage_type,
           gen_rect_indexer<row_orien,M,N,size_type> >,
           typename generate_uplo<Shape_uplo>::RET >,
       symmetric_matrix<column_matrix< storage_type,
           gen_rect_indexer<column_orien,N,M,size_type> >,
           typename generate_uplo<Shape_uplo>::RET >
                     >::RET
     ,
       typename IF< EQUAL< Orien_id, ROW_MAJOR>::RET,
       symmetric_matrix<row_matrix< storage_type,
           gen_banded_indexer<row_orien,M,N,size_type> >,
           typename generate_uplo<Shape_uplo>::RET >,
       symmetric_matrix<column_matrix< storage_type,
           gen_banded_indexer<column_orien,N,M,size_type> >,
           typename generate_uplo<Shape_uplo>::RET >
                     >::RET
     >::RET RET;
  };

  //: Matrix type generators class.
  //
  // Matrices that occur in real engineering and scientific applications
  // often have special structure, especially in terms of how many zeros
  // are in the matrix, and where the non-zeros are located in the matrix.
  // This means that space and time saving can be acheived by using various
  // types of compressed storage. There are a multitude of matrix storage
  // formats in use today, and the MTL tries to support many of the more
  // common storage formats. The following discussion will describe how the
  // user of MTL can select the type of matrix he or she wishes to use.
  //
  // To create a MTL matrix, one first needs to construct the
  // appropriate matrix type. This is done using the <tt>matrix</tt>
  // type generation class, which is easier to think of as a function. It
  // takes as input the characteristics of the matrix type that you want
  // and then returns the appropriate MTL matrix. The <tt>matrix</tt> type
  // generators ``function'' has defaults defined, so in order to create a
  // normal rectangular matrix type, one merely does the following:
  //
  // <codeblock>
  // typedef matrix< double >::type MyMatrix;
  // MyMatrix A(M, N);
  // </codeblock>
  //
  // The matrix type generators can take up to four arguments, the
  // element type, the matrix shape, the storage
  // format, and the orientation. The following is the
  // ``prototype'' for the <tt>matrix</tt> type generators.
  //
  // <codeblock>
  // matrix< EltType, Shape, Storage, Orientation >::type
  // </codeblock>
  //
  // This type of "generative" interface technique was developed by by
  // <a href="http://nero.prakinf.tu-ilmenau.de:80/~czarn/">Krzysztof
  // Czarnecki</a> and <a
  // href="http://home.t-online.de/home/Ulrich.Eisenecker/">Ulrich
  // Eisenecker</a> in their work on the <a href="http://nero.prakinf.tu-ilmenau.de:80/~czarn/gmcl/">Generative Matrix Computation Library</a>.
  //  <p>
  //  *Storage can be made external by specifying such in the storage
  //   parameter. eg. dense&lt;external&gt;, packed&lt;external&gt;.
  //
  //!tparam: EltType - Valid choices for this argument include <tt>double</tt>, <tt>complex<float></tt>, and <tt>bool</tt>.  In essence, any builtin or user defined type can be used for the <tt>EltType</tt>, however, if one uses the matrix with a particular algorithm, the <tt>EltType</tt> must support the operations required by the algorithm. For MTL algorithms these typically include the usual numerical operators such as addition and multiplication. The <tt>std::complex</tt> class is a good example of what is required in a numerical type. The documentation for each algorithm will include the requirements on the element type.
  //
  //!tparam: Shape - This argument specifies the general positioning of the non zero elements in the matrix, but does not specify the actual storage format.  In addition it specifies certain properties such as symmetry.  The choices for this argument include <tt>rectangle</tt>, <tt>banded</tt>, <tt>diagonal</tt>, <tt>triangle</tt>, and <tt>symmetric</tt>. Hermitian is not yet implemented.
  //
  //!tparam: Storage - The argument specifies the storage scheme used to lay out the matrix elements (and sometimes the element indices) in memory. The storage formats include <tt>dense</tt> , <tt>banded</tt>, <tt>packed</tt> , <tt>banded_view</tt>, <tt>compressed</tt>, <tt>envelope</tt>, and <tt>array</tt>.
  //
  //!tparam: Orientation - The storage order for an MTL matrix can either be <tt>row_major</tt> or <tt>column_major</tt>.
  //
  //!category: containers, generators
  //!component: type
  //!definition: matrix.h
  //
  template < class T, class Shape = rectangle<>, class Storage = dense<>, class Orientation = row_major >
  struct matrix {
    enum { Shape_id = Shape::id, Shape_M = Shape::M, Shape_N = Shape::N };
    //: The generated type
    typedef typename IF< EQUAL< Shape_id, RECT>::RET,
                        typename generate_rect<T,Orientation,Storage,
                                               Shape_M, Shape_N>::RET,
                    typename IF< EQUAL< Shape_id, DIAG>::RET,
                        typename generate_diagonal<T,Storage,
                                               Shape_M, Shape_N>::RET,
                    typename IF< EQUAL< Shape_id, BAND>::RET,
                        typename generate_banded<T,Orientation,Storage,
                                               Shape_M, Shape_N>::RET,
                    typename IF< EQUAL< Shape_id, TRI>::RET,
                        typename generate_triangle<T,Shape,
                                               Orientation,Storage,
                                               Shape_M, Shape_N>::RET,
                    typename IF< EQUAL< Shape_id, SYMM>::RET,
                        typename generate_symmetric<T,Shape,
                                              Orientation,Storage,
                                               Shape_M, Shape_N>::RET,
                        generators_error
                        >::RET
                        >::RET
                        >::RET
                        >::RET
                        >::RET type;
  };



#ifndef MTL_DISABLE_BLOCKING

  //: Block View Matrix Type Constructor
  //
  // <codeblock>
  // block_view<Matrix> bA = blocked<>(A, 16, 16);
  // or
  // block_view<Matrix,BM,BN> bA = blocked<BM,BN>(A);
  // </codeblock>
  //
  // Note: currently not supported for egcs (internal compiler error).
  //
  //!category: containers, generators
  //!component: type
  //!example: blocked_matrix.cc
  //!definition: matrix.h
  //!tparam: Matrix - The type of the Matrix to be blocked, must be dense
  //!tparam: BM - The blocking factor for the rows (M dimension) - 0 for dynamic size
  //!tparam: BN - The blocking factor for the columns (N dimension) - 0 for dynamic size
  template <class Matrix, int BM = 0, int BN = 0>
  struct block_view {
    //: The generated type
    typedef typename Matrix:: MTL_TEMPLATE blocked_view<BM,BN>::type type;
  };

  template <int BM, int BN>
  struct blk { enum { M = BM, N = BN }; };

  //: Blocked Matrix Generator
  //
  // <codeblock>
  // block_view<Matrix> bA = blocked<>(A, 16, 16);
  // </codeblock>
  //
  // Note: currently not supported for egcs (internal compiler error).
  //
  //!category: containers
  //!component: function
  //!example: blocked_matrix.cc
  //!definition: matrix.h
  //!tparam: Matrix - The type of the Matrix to be blocked, must be dense
  template <class Matrix>
  typename block_view<Matrix, 0, 0>::type
  blocked(const Matrix& A, int bm, int bn) {
    typedef dimension<typename Matrix::size_type, 0,0> bdt;
    return typename block_view<Matrix, 0, 0>::type(A, bdt(bm, bn));
  }

  //: Blocked Matrix Generator
  //
  // This version of the blocked matrix generator is for statically
  // sized blocks.
  //
  // <codeblock>
  // block_view<Matrix,BM,BN> bA = blocked<BM,BN>(A);
  // </codeblock>
  //
  // Note: currently not supported for egcs (internal compiler error).
  //
  //!category: containers
  //!component: function
  //!example: blocked_matrix.cc
  //!definition: matrix.h
  //!tparam: Matrix - The type of the Matrix to be blocked, must be dense
  //!tparam: BM - The blocking factor for the rows (M dimension)
  //!tparam: BN - The blocking factor for the columns (N dimension)
  template <class Matrix, int BM, int BN>
  typename block_view<Matrix, BM, BN>::type
  blocked(const Matrix& A, blk<BM,BN>) {
    typedef dimension<typename Matrix::size_type, BM, BN> bdt;
    return typename block_view<Matrix, BM, BN>::type(A, bdt(BM, BN));
  }
#endif

  //: Band View Matrix Type Constructor
  // A helper class for creating a banded_view into an existing dense matrix.
  //!category: containers, generators
  //!component: type
  //!example: banded_view_test.cc
  //!definition: matrix.h
  //!tparam: Matrix - The type of the Matrix to be viewed, must be dense
  template <class Matrix>
  struct band_view {
    //: The generated type
    typedef typename Matrix::banded_view_type type;
  };

  //: Triangle View Matrix Type Constructor
  // A helper class for creating a triangle view into an existing dense
  //   or sparse matrix. For sparse matrices, the matrix must already
  //   have elements in the appropriate triangular portion of the matrix.
  //   This just provides the proper triangular matrix interface.
  //!category: containers, generators
  //!component: type
  //!example: banded_view_test.cc
  //!definition: matrix.h
  //!tparam: Matrix - The type of the Matrix to be viewed, must be dense
  //!tparam: Uplo - Whether to view the upper or lower triangle of the matrix
  template <class Matrix, int UL>
  struct triangle_view {
    typedef typename Matrix::sparsity Sparsity;
    enum { Sparsity_id = Sparsity::id };
    //: The generated type
    typedef typename generate_uplo<UL>::RET up_or_lower;
    typedef typename IF< EQUAL< Sparsity_id, DENSE>::RET,
			 triangle_matrix<typename Matrix::banded_view_type, up_or_lower>,
                         triangle_matrix< Matrix, up_or_lower >
                       >::RET type;
  };

  //: Triangle View Creation Helper Fuctor
  // Example: tri_view<upper>()(A)
  //!category: containers, generators
  //!component: type
  //!example: banded_view_test.cc
  //!definition: matrix.h
  //!tparam: Uplo - Whether to view the upper or lower triangle of the matrix
  template <int Uplo>
  struct tri_view {
    template <class Matrix>
    inline typename triangle_view<Matrix, Uplo>::type operator()(Matrix x) const {
      typedef typename triangle_view<Matrix, Uplo>::type TriView;
      return TriView(x);
    }
  };



  //: Symmetric View Matrix Type Constructor
  // A helper class for creating a symmetric view into an existing dense
  //   or sparse matrix. For sparse matrices, the matrix must already
  //   have elements in the appropriate lower/upper portion of the matrix.
  //   This just provides the proper symmetric matrix interface.
  //!category: containers, generators
  //!component: type
  //!definition: matrix.h
  //!tparam: Matrix - The type of the Matrix to be viewed, must be dense
  //!tparam: Uplo - Whether to view the upper or lower triangle of the matrix
  template <class Matrix, int Uplo>
  struct symmetric_view {
    typedef typename Matrix::sparsity Sparsity;
          enum { Sparsity_id = Sparsity::id };
    //: The generated type
    typedef typename IF< EQUAL< Sparsity_id, DENSE>::RET,
      symmetric_matrix< typename Matrix::banded_view_type,
                        typename generate_uplo<Uplo>::RET>,
                symmetric_matrix< Matrix, typename generate_uplo<Uplo>::RET>
              >::RET type;
  };


} /* namespace mtl */

#endif /* _MTL_MATRIX_H_ */

