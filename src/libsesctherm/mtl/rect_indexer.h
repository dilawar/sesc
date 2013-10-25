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

#ifndef _MTL_RECT_INDEXER_
#define _MTL_RECT_INDEXER_

#include <utility>
#include "matrix_traits.h"
#include "dimension.h"
#include "orien.h"

namespace mtl {


template <class size_type, class Orien, int MM, int NN>
class rect_indexer {
public:
  enum { M = MM, N = NN };
  typedef dimension<size_type, M, N> dim_type;
  typedef dimension<size_type> dyn_dim;

  typedef dimension<int> band_type;

  /* JGS KCC friend transpose broken */
  dim_type dim;

  typedef typename Orien::orientation orientation;
  typedef rectangle_tag shape;

  typedef rect_indexer<size_type,
                    typename Orien::transpose_type, MM, NN> transpose_type;
  typedef rect_indexer<size_type,
                    typename Orien::transpose_type, NN, MM> strided_type;
  //VC++ doesn't like
  //friend class transpose_type;

  typedef Orien orienter;

  class OneDIndexer {
  public:
    inline OneDIndexer() { }
    inline OneDIndexer(size_type majornum) : major_num(majornum) { }
    inline OneDIndexer(const OneDIndexer& x) : major_num(x.major_num) { }

    template <class OneDIterator> inline
    size_type row(OneDIterator i) const { return Orien::row(coords(i)); }

    template <class OneDIterator> inline
    size_type column(OneDIterator i) const { return Orien::column(coords(i)); }

    template <class OneDIterator> inline
    size_type minor(OneDIterator i) const { return coords(i).second(); }

    template <class OneDIterator> inline
    OneDIterator begin(OneDIterator i) const { return i; }

    inline size_type at(size_type i) const { return i; }
  protected:
    template <class OneDIterator> inline
    dyn_dim coords(OneDIterator i) const {
      return dyn_dim(major_num, i.index());
    }
    size_type major_num;
  };
  inline rect_indexer() { }
  inline rect_indexer(dim_type d) : dim(d) { }
  inline rect_indexer(dim_type d, band_type) : dim(d) { }
  inline rect_indexer(const rect_indexer& x) : dim(x.dim) { }

#if (!defined( _MSVCPP_ ))
  template <class Indexer>
  inline rect_indexer(const Indexer& x) : dim(x.dim) { }
#endif

#if (defined  __SUNPRO_CC)
  template <class S, int MMM, int NNN>
  inline rect_indexer(const dimension<S,MMM,NNN>& x) : dim(x) { }
#endif

  inline rect_indexer(const strided_type& x, strideable)
    : dim(x.dim.transpose()) { }

  inline rect_indexer(const transpose_type& x, not_strideable)
    : dim(x.dim) { }

  template <class TwoDIterator> inline
  OneDIndexer deref(TwoDIterator i) const { return OneDIndexer(i.index()); }

  inline OneDIndexer deref(size_type i) const {
    return OneDIndexer(i);
  }

#if 0
  inline dyn_dim at(dim_type p) const { return Orien::map(p); }
#endif

  /* for dynamic dim types (M = 0, N = 0) */
//#if !defined( _MSVCPP_ )
#if 1
  template <class Dim> inline
  Dim at(Dim p) const { return Orien::map(p); }
#endif
  /* used in constructor of matrix_implementation */
  inline static dim_type twod_dim(dim_type dim) { return dim; }
  inline static dim_type twod_dim(dim_type dim, band_type) { return dim; }

  inline static band_type twod_band(dim_type , band_type bw) { return bw; }

  inline size_type nrows() const{ return Orien::row(dim); }
  inline size_type ncols() const { return Orien::column(dim); }

  inline int sub() const { return nrows() - 1; }
  inline int super() const { return ncols() - 1; }

};

//: blah
//!noindex:
template <class Orien, int MM, int NN, class size_type>
struct gen_rect_indexer {
  // static equivalent to twod_dim
#if 0
  // bad for VC++
  typedef typename Orien::template dims<MM,NN> oriendim;
  enum { M = oriendim::M, N = oriendim::N };
#else
  enum { M = MM, N = NN };
#endif
  typedef dimension<char, M, N> twod_dim_type;

  typedef gen_rect_indexer< typename Orien::transpose_type, MM, NN, size_type>
    transpose_type;
  typedef gen_rect_indexer< typename Orien::transpose_type, NN, MM, size_type>
    strided_type;

  typedef rect_indexer<size_type, Orien, M, N> type;

  template <class ST>
  struct bind {
    typedef gen_rect_indexer<Orien, MM, NN, ST> other;
  };

  typedef Orien orienter;
};

} /* namespace mtl */

#endif
