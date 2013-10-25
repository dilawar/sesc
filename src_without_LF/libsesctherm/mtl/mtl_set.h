#ifndef MTL_SET_H
#define MTL_SET_H

#include "dim_calc.h"

#if USE_BLAIS
#include "fast.h"
#include "blais.h"
#endif

namespace mtl {

template <class Vector, class T> inline
void
oned_set(Vector x, const T& alpha, fast::count<0>)
{
  mtl_algo::fill(x.begin(), x.end(), alpha);
}
#if USE_BLAIS
template <int N, class Vector, class T> inline
void
oned_set(Vector x, const T& alpha, fast::count<N>)
{
  fast::fill(x.begin(), fast::count<N>(), alpha);
}
#endif //USE_BLAIS
template <class Vector, class T> inline
void
set__(Vector x, const T& alpha, oned_tag)
{
  oned_set(x, alpha, typename dim_n<Vector>::RET());
}


template <class Matrix, class T> inline
void
set__(Matrix A, const T& alpha, fast::count<0>)
{
  typename Matrix::iterator i;
  typename Matrix::OneD::iterator j, jend;
  for (i = A.begin(); i != A.end(); ++i) {
    j = (*i).begin(); jend = (*i).end();
    for (; j != jend; ++j)
      *j = alpha;
  }
}

#if USE_BLAIS
template <class Matrix, class T, int M> inline
void
set__(Matrix A, const T& alpha, fast::count<M>)
{
  enum { N = dim_n<Matrix>::RET::N };
  blais_m::set<M,N>(A, alpha);
}
#endif //USE_BLAIS

template <class Matrix, class T> inline
void
set__(Matrix A, const T& alpha, twod_tag)
{
  set__(A, alpha, typename dim_m<Matrix>::RET());
}


//: Set: <tt>A <- alpha or x <- alpha</tt>
//
//  Set all the elements in <tt>A</tt> (or <tt>x</tt>) to
//  <tt>alpha</tt>. Note that when using <tt>set()</tt> with banded
//  matrices, only the elements within the band are set. When using
//  sparse matrices, only nonzero elements are set.
//
//  <p>Note that you must always use the <tt>mtl::</tt> prefix with
//  this function in order to avoid name conflicts with the
//  <tt>std::set</tt>.  Do not do <tt>using namespace mtl;</tt> or
//  <tt>using mtl::set()</tt> and access this function
//  without the prefix.
//
//!category: algorithms
//!component: function
//!definition: mtl.h
//!example: vec_set.cc
//!complexity: O(m*n) for dense matrix, O(nnz) for sparse, O(n) for vector
//!typereqs: <TT>Vector</TT> must be mutable
//!typereqs: <TT>T</TT> is convertible to <TT>Vector</TT>'s <TT>value_type</TT>

#if !defined(_MSVCPP_) && !defined (_MSVCPP7_)
template <class LinalgObj, class T>
inline void
set(LinalgObj A, const T& alpha)
{
  typedef typename linalg_traits<LinalgObj>::dimension Dim;
  set__(A, alpha, Dim());
}
#endif // #if !defined(_MSVCPP_)

//use it to replace mtl::set in Visual C++
template <class LinalgObj, class T>
inline void
set_value(LinalgObj A, const T& alpha)
{
  typedef typename linalg_traits<LinalgObj>::dimension Dim;
  set__(A, alpha, Dim());
}

} /* namespace mtl */

#endif /* MTL_SET_H */
