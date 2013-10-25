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

#ifndef MTL_DENSE2D_H
#define MTL_DENSE2D_H

#include "mtl_iterator.h"
#include <utility>
#include <assert.h>
#include <vector>

#include "mtl_config.h"
#include "linalg_vec.h"
#include "strided1D.h"
#include "initialize.h"
#include "reverse_iter.h"
#include "matrix_traits.h"
#include "dimension.h"

#ifndef MTL_DISABLE_BLOCKING
#include "block2D.h"
#endif

namespace mtl {

template <class size_t, int MM, int NN>
class strided_offset;


struct strided_tag { enum { id = 1 }; };
struct not_strided_tag { enum { id = 0 }; };

template <class size_t, int MM, int NN>
class band_view_offset;

template <class size_t, int MM, int NN>
class strided_band_view_offset;

template <int M, int N> struct gen_rect_offset;
template <int M, int N> struct gen_strided_offset;
template <int M, int N> struct gen_banded_offset;
template <int M, int N> struct gen_banded_view_offset;
template <int M, int N> struct gen_strided_band_view_offset;
template <int M, int N> struct gen_packed_offset;


//: Rectangular Offset Class
//!models: Offset
//!category: utilities
//!component: type
template <class size_t, int MM, int NN>
class rect_offset {
public:
#if !defined(_MSVCPP_)
  template <class Vec>
  struct bind_oned {
    typedef Vec type;
  };
#endif
  typedef not_strided_tag is_strided;
  typedef size_t size_type;
  enum { M = MM, N = NN, IS_STRIDED = 0 };
  typedef dimension<size_type, MM, NN> dim_type;
  typedef dimension<int> band_type;
  typedef strided_offset<size_type, MM, NN> transpose_type;
  typedef strideable strideability;
  // VC++ doesn't like this
  //friend class transpose_type;

  //what is that for? -- llee
  //inline rect_offset() : dim(4444,4444), ld(4444) { }
  inline rect_offset() : dim(0,0), ld(0) { }
  inline rect_offset(const rect_offset& x) : dim(x.dim), ld(x.ld) { }
  inline rect_offset(size_type m, size_type n, size_type ld_)
    : dim(m, n), ld(ld_) { }
  inline rect_offset(size_type m, size_type n, size_type ld_, band_type)
    : dim(m, n), ld(ld_) { }
  rect_offset(const transpose_type& x); /* see below strided_offset for def */
  inline rect_offset& operator=(const rect_offset& x) {
    dim = x.dim; ld = x.ld; return *this;
  }
  inline size_type elt(size_type i, size_type j) const { return i * ld + j; }
  inline size_type oned_offset(size_type i) const { return i * ld; }
  inline size_type oned_length(size_type) const { return dim.second(); }
  inline size_type twod_length() const { return dim.first(); }
  inline size_type stride() const { return 1; }
  inline static size_type size(size_type m, size_type n,
                               size_type , size_type) { return m * n; }
  inline size_type major() const { return dim.first(); }
  inline size_type minor() const { return dim.second(); }
  /* private: */
  dim_type dim;
  size_type ld;
};


//: blah
//!noindex:
template <int M, int N>
struct gen_rect_offset {
#if defined( _MSVCPP_ )
   typedef rect_offset<unsigned int, M, N> type;
#else
  template <class size_type>
  struct bind {
    typedef rect_offset<size_type, M, N> type;
  };
#endif
  typedef gen_strided_offset<M,N> transpose_type;
  typedef gen_banded_view_offset<M,N> banded_view_type;

};

//: Strided Rectangular Offset Class
//!models: Offset
//!category: utilities
//!component: type
template <class size_t, int MM, int NN>
class strided_offset {
public:
#if !defined(_MSVCPP_)
  template <class Vec>
  struct bind_oned {
    typedef strided1D<Vec> type;
  };
#endif
  /*  typedef strided_band_view_offset<size_t,MM,NN> banded_view_type;
   */
  typedef strided_tag is_strided;
  enum { M = MM, N = NN, IS_STRIDED = 1 };
  typedef size_t size_type;
  typedef dimension<size_type, MM, NN> dim_type;
  typedef dimension<int> band_type;
  typedef rect_offset<size_type,MM,NN> transpose_type;
  typedef strideable strideability;
// VC++ doesn't like this
  //friend class transpose_type;
  inline strided_offset() : dim(0,0), ld(0) { }
  inline strided_offset(size_type m, size_type n, size_type ld_)
    : dim(m, n), ld(ld_) { }
  inline strided_offset(const transpose_type& x) : dim(x.dim), ld(x.ld) { }
  inline strided_offset& operator=(const strided_offset& x) {
    dim = x.dim; ld = x.ld; return *this;
  }
  inline size_type elt(size_type i, size_type j) const { return j * ld + i; }
  inline size_type oned_offset(size_type i) const { return i; }
  inline size_type oned_length(size_type) const { return dim.first() * ld; }
  inline size_type twod_length() const { return dim.second(); }
  inline size_type stride() const { return ld; }
  inline static size_type size(size_type m, size_type n,
                               size_type , size_type) { return m * n; }
  inline size_type major() const { return dim.first(); }
  inline size_type minor() const { return dim.second(); }
  /* private: */
  dim_type dim;
  size_type ld;
};

//: blah
//!noindex:
template <int M, int N>
struct gen_strided_offset {
#if defined( _MSVCPP_ )
  typedef strided_offset<unsigned int, M, N> type;
#else
  template <class size_type>
  struct bind {
    typedef strided_offset<size_type, M, N> type;
  };
#endif
  typedef gen_rect_offset<M,N> transpose_type;
  typedef gen_strided_band_view_offset<M,N> banded_view_type;
};

template <class size_t, int MM, int NN>
inline rect_offset<size_t,MM,NN>::rect_offset(const rect_offset<size_t,MM,NN>::transpose_type& x)
  : dim(x.dim), ld(x.ld) { }


//: Banded View Offset Class
// This creates a banded view into a full matrix.
//!models: Offset
//!category: utilities
//!component: type
template <class size_t, int MM, int NN>
class banded_view_offset {
public:
#if !defined(_MSVCPP_)
  template <class Vec>
  struct bind_oned {
    typedef Vec type;
  };
#endif

  typedef not_strided_tag is_strided;
  enum { M = MM, N = NN, IS_STRIDED = 0 };
  typedef size_t size_type;
  typedef dimension<size_type, MM, NN> dim_type;
  typedef dimension<int, MM, NN> band_type;
  typedef strided_band_view_offset<size_type, MM, NN> transpose_type;

  typedef not_strideable strideability;
// VC++ doesn't like this
  //friend class transpose_type;
  inline banded_view_offset()
    : dim(0,0), ld(0), bw(std::make_pair(0,0)) { }
  inline banded_view_offset(size_type m, size_type n, size_type leading_dim,
                            band_type band)
    : dim(m, n), ld(leading_dim), bw(band) { }
  inline banded_view_offset(size_type m, size_type n, size_type leading_dim)
    : dim(m, n), ld(leading_dim), bw(band_type(0,0)) { }

  template <class Offset>
  inline banded_view_offset(Offset os, band_type band)
    : dim(os.dim), ld(os.ld), bw(band) { }

  inline banded_view_offset& operator=(const banded_view_offset& x) {
    dim = x.dim; ld = x.ld; bw = x.bw; return *this;
  }

  inline size_type elt(size_type i, size_type j) const {
    size_type start = MTL_MAX(int(i) - bw.first(), 0);
    return i * ld + j + start;
  }
  inline size_type oned_offset(size_type i) const {
    size_type start = MTL_MAX(int(i) - bw.first(), 0);
    return i * ld + start;
  }
  inline size_type oned_length(size_type i) const {
    return MTL_MAX(0, MTL_MIN(int(dim.second()), int(i) + bw.second() + 1)
               - MTL_MAX(0, int(i) - bw.first()));
  }
  inline size_type twod_length() const { return dim.first(); }
  inline int stride() const { return 1; }
  inline static size_type size(size_type m, size_type n,
                               size_type , size_type) {
    return m * n;
  }
  inline size_type major() const { return dim.first(); }
  inline size_type minor() const { return dim.second(); }

  /* private: */
  dim_type dim;
  size_type ld;
  band_type bw; /* bandwidth */
};


//: blah
//!noindex:
template <int M, int N>
struct gen_banded_view_offset {
#if defined( _MSVCPP_ )
  typedef banded_view_offset<unsigned int, M, N> type;
#else
  template <class size_type>
  struct bind {
    typedef banded_view_offset<size_type, M, N> type;
  };
#endif
  typedef gen_strided_band_view_offset<M,N> transpose_type;
  typedef gen_banded_view_offset<M,N> banded_view_type; /* bogus */
};


//: Strided Band View Offset Class
//
// This creates a strided band view into a full matrix.
// This class is to banded_view as strided_offset is to rect_offset.
//
//!models: Offset
//!category: utilities
//!component: type
template <class size_t, int MM, int NN>
class strided_band_view_offset {
public:
#if !defined(_MSVCPP_)
  template <class Vec>
  struct bind_oned {
    typedef strided1D<Vec> type;
  };
#endif
  typedef strided_tag is_strided;
  enum { M = MM, N = NN, IS_STRIDED = 1 };
  typedef size_t size_type;
  typedef dimension<size_type, MM, NN> dim_type;
  typedef dimension<int, MM, NN> band_type;
  typedef banded_view_offset<size_type, MM, NN> transpose_type;

  typedef not_strideable strideability;
// VC++ doesn't like this
  //friend class transpose_type;
  inline strided_band_view_offset()
    : dim(0,0), ld(0), bw(std::make_pair(0,0)) { }
  inline strided_band_view_offset(size_type m, size_type n,
				  size_type leading_dim,
				  band_type band)
    : dim(m, n), ld(leading_dim), bw(band) { }

  template <class Offset>
  inline strided_band_view_offset(Offset os, band_type band)
    : dim(os.dim), ld(os.ld), bw(band) { }

  inline strided_band_view_offset&
  operator=(const strided_band_view_offset& x) {
    dim = x.dim; ld = x.ld; bw = x.bw; return *this;
  }

  inline size_type elt(size_type i, size_type j) const {
    size_type start = MTL_MAX(int(i) - bw.first(), 0);
    return (j + start) * ld + i;
  }
  inline size_type oned_offset(size_type i) const {
    size_type start = MTL_MAX(int(i) - bw.first(), 0);
    return start * ld + i;
  }
  inline size_type oned_length(size_type i) const {
    /* use dim.first() here */
    size_type len =  MTL_MAX(0, MTL_MIN(int(dim.first()), int(i) + bw.second() + 1)
                          - MTL_MAX(0, int(i) - bw.first()));
    return len * ld;
  }
  inline size_type twod_length() const { return dim.second(); }
  inline int stride() const { return ld; }
  inline static size_type size(size_type m, size_type n,
                               size_type , size_type) {
    return m * n;
  }
  inline size_type major() const { return dim.first(); }
  inline size_type minor() const { return dim.second(); }

  /* private: */
  dim_type dim;
  size_type ld;
  band_type bw; /* bandwidth */
};


//: blah
//!noindex:
template <int M, int N>
struct gen_strided_band_view_offset {
#if defined( _MSVCPP_ )
  typedef strided_band_view_offset<unsigned int, M, N> type;
#else
  template <class size_type>
  struct bind {
    typedef strided_band_view_offset<size_type, M, N> type;
  };
#endif
  typedef gen_banded_view_offset<M,N> transpose_type;
  typedef gen_strided_band_view_offset<M,N> banded_view_type; /* bogus */
};


template <class size_t, int MM, int NN>
class packed_offset;


//: Banded Offset Class
// This cooresponds to lapack/blas banded storage format.
//!models: Offset
//!category: utilities
//!component: type
template <class size_t, int MM, int NN>
class banded_offset {
public:
#if !defined(_MSVCPP_)
  template <class Vec>
  struct bind_oned {
    typedef Vec type;
  };
#endif
  typedef not_strided_tag is_strided;
  enum { M = MM, N = NN, IS_STRIDED = 0 };
  typedef size_t size_type;
  typedef dimension<size_type, MM, NN> dim_type;
  typedef dimension<int> band_type;
  typedef packed_offset<size_type,MM,NN> transpose_type; /* bogus */
  typedef not_strideable strideability;
  inline banded_offset()
    : dim(0,0), bw(band_type(0,0)), ndiag(0) { }

  inline banded_offset(size_type m, size_type n, size_type /* lead */,
                       band_type band)
    : dim(m,n), bw(band), ndiag(band.first() + band.second() + 1) { }

 inline banded_offset(size_type m, size_type n, size_type /* lead */)
    : dim(m,n), bw(band_type(0,0)), ndiag(0) { }

  inline banded_offset& operator=(const banded_offset& x) {
    dim = x.dim; ndiag = x.ndiag; bw = x.bw; return *this;
  }
  inline size_type elt(size_type i, size_type j) const {
    return this->oned_offset(i) + j;
  }
  inline size_type oned_offset(size_type i) const {
    return i * ndiag + MTL_MAX(0, bw.first() - int(i));
  }
  inline size_type oned_length(size_type i) const {
    return MTL_MAX(0, MTL_MIN(int(dim.second()), int(i) + bw.second() + 1)
               - MTL_MAX(0, int(i) - bw.first()));
  }
  inline size_type twod_length() const { return dim.first(); }

  inline int stride() const { return 1; }

  inline static size_type size(size_type m, size_type n,
                               size_type low, size_type up) {
    /* M' = number of diagonals = low + up + 1
       N' = min (m, n + low) */
    return (low + up + 1) * MTL_MIN(m, n + low);
  }
  inline size_type major() const { return dim.first(); }
  inline size_type minor() const { return dim.second(); }
private:
  dim_type dim;
  band_type bw; /* bandwidth */
  size_type ndiag;
};


//: blah
//!noindex:
template <int M, int N>
struct gen_banded_offset {
#if defined( _MSVCPP_ )
  typedef banded_offset<unsigned int, M, N> type;
#else
  template <class size_type>
  struct bind {
    typedef banded_offset<size_type, M, N> type;
  };
#endif
  typedef gen_packed_offset<M,N> transpose_type; // bogus
  typedef gen_banded_view_offset<M,N> banded_view_type; /* bogus */
};


//: Packed Offset Class
// This cooresponds to lapack/blas packed storage format
//!models: Offset
//!category: utilities
//!component: type
template <class size_t, int MM, int NN>
class packed_offset {
public:
#if !defined(_MSVCPP_)
  template <class Vec>
  struct bind_oned {
    typedef Vec type;
  };
#endif
  typedef not_strided_tag is_strided;
  enum { M = MM, N = NN, IS_STRIDED = 0 };
  typedef size_t size_type;
  typedef dimension<size_type, MM, NN> dim_type;
  typedef dimension<int> band_type;
  typedef banded_offset<size_type, MM, NN> transpose_type; /* bogus */
  typedef not_strideable strideability;
  inline packed_offset()
    : dim(0,0), bw(band_type(0,0)) { }

  inline packed_offset(size_type m, size_type n, size_type /* lead */,
                       band_type bandwidth)
    : dim(m,n), bw(bandwidth) { }

  inline packed_offset& operator=(const packed_offset& x) {
    dim = x.dim; bw = x.bw; return *this;
  }
  inline int elt(size_type i, size_type j) const {
    return this->oned_offset(i) + j;
  }

  inline int calc_low(int i, int low) const {
    int l = MTL_MIN(low, int(i));
    int lower_area = low * i;
    lower_area -= ( - l*l + 2*low*l + l) / 2;
    return lower_area;
  }
  inline int calc_up(int i, int up) const {
    int upper_area = up * i;
    int n = i + up - dim.second();
    if (n > 0) {
      int n1 = MTL_MAX(n - up, 0);
      int n2 = n - n1;
      upper_area -= n1 * up;
      upper_area -= ((n2 + 1) * n2) / 2;
    }
    return upper_area;
  }

  inline int oned_offset(size_type i) const { /* the ith major container */
    int low = bw.first();
    int up = bw.second();
    int upper_area, lower_area;

    if (up < -1)
      upper_area = - calc_low(i, - (up + 1));
    else if (up > 0)
      upper_area = calc_up(i, up);
    else
      upper_area = 0;

    if (low < -1)
      lower_area = - calc_up(i, - (low + 1));
    else if (low > 0)
      lower_area = calc_low(i, low);
    else
      lower_area = 0;

    size_type diagonal_len;
    if (up < 0 || low < 0)
      diagonal_len = 0;
    else
      diagonal_len = MTL_MIN(MTL_MIN(i, dim.first()), dim.second());

    size_type ret =  upper_area + lower_area + diagonal_len;
    return ret;
  }

  inline int stride() const { return 1; }

  inline size_type oned_length(size_type i) const {
    return MTL_MAX(0, MTL_MIN(int(dim.second()), int(i) + bw.second() + 1)
               - MTL_MAX(0, int(i) - bw.first()));
  }
  inline size_type twod_length() const { return dim.first(); }

  inline static size_type size(int m, int n, int low, int up) {
    packed_offset offset(m, n, n, band_type(low, up));
    return offset.oned_offset(m);
  }
  inline size_type major() const { return dim.first(); }
  inline size_type minor() const { return dim.second(); }

private:
  dim_type dim;
  band_type bw; /* bandwidth */
};

//: blah
//!noindex:
template <int M, int N>
struct gen_packed_offset {
#if defined( _MSVCPP_ )
  typedef packed_offset<unsigned int, M, N> type;
#else
  template <class size_type>
  struct bind {
    typedef packed_offset<size_type, M, N> type;
  };
#endif
  typedef gen_banded_offset<M,N> transpose_type; /* bogus */
  typedef gen_banded_view_offset<M,N> banded_view_type; /* bogus */
};


/* egcs doesn't "see" the friend functions
 *   when the dense2D_iterator class is defined inside of dense2D
 */

//: blah
//!noindex:
template <int isConst, class T, class Offset, class InnerOneD, class OneD>
class dense2D_iterator {
public:
  typedef typename Offset::size_type size_type;
  typedef std::pair<size_type,size_type> pair_type;

  typedef typename IF<isConst, const T*,T*>::RET Iterator;

  typedef dense2D_iterator self;

  typedef int distance_type;
  typedef int difference_type;

  typedef std::random_access_iterator_tag iterator_category;

  typedef OneD*           pointer;
  typedef OneD            value_type;
  typedef OneD            reference;
  typedef difference_type Distance;
  typedef Iterator        iterator_type;

protected:

  Iterator start;
  size_type pos;
  size_type ld;    /* leading dimension */
  pair_type starts;
  Offset offset;
public:

  inline size_type index() const { return pos + starts.second; }

  inline dense2D_iterator () {}

  inline dense2D_iterator(const self& x)
    : start(x.start), pos(x.pos),
      ld(x.ld), starts(x.starts), offset(x.offset) { }

  inline self& operator=(const self& x) {
    start = x.start;
    pos = x.pos;
    ld = x.ld;
    starts = x.starts;
    offset = x.offset;
    return *this;
  }

  inline explicit
  dense2D_iterator(Iterator x, size_type ld_, size_type p, pair_type s,
                   Offset os)
    : start(x), pos(p), ld(ld_), starts(s), offset(os) { }

  inline Iterator base () const { return start + pos; }

  inline reference deref(Distance pos, not_strided_tag) const {
    return reference((T*)start + offset.oned_offset(pos),
		offset.oned_length(pos),
		starts.first);
  }
  inline reference deref(Distance pos, strided_tag) const {
    InnerOneD vec((T*)start + offset.oned_offset(pos),
                  offset.oned_length(pos),
                  starts.first);
    return strided(vec, offset.stride());
  }
  inline reference operator*() const {
    typedef typename Offset::is_strided Strided;
    return deref(pos, Strided());
  }
  inline reference operator[] (Distance n) const {
    typedef typename Offset::is_strided Strided;
    return deref(pos + n, Strided());
  }

  /*  won't work, the OneD is temporary
  pointer   operator-> () const { return & (operator* ()); }
  */

  inline self& operator++ () { ++pos; return *this; }
  inline self operator++ (int) { self tmp = *this; ++pos; return tmp; }
  inline self& operator-- () { --pos; return *this; }
  inline self operator-- (int) { self tmp = *this; --pos; return tmp; }
  inline self& operator+=(size_type n) { pos += n; return *this; }
  inline self operator+(size_type n) const {
    return self(start, ld, pos + n, starts);
  }
  inline self& operator-=(size_type n) { pos -= n; return *this; }



};

template <int isConst, class T, class Offset, class InnerOneD, class OneD>
inline typename dense2D_iterator<isConst,T,Offset,InnerOneD,OneD>::difference_type
operator-(const dense2D_iterator<isConst,T,Offset,InnerOneD,OneD>& x,
          const dense2D_iterator<isConst,T,Offset,InnerOneD,OneD>& y)
{
  return x.index() - y.index();
}

template <int isConst,class T, class Offset, class InnerOneD, class OneD>
inline bool
operator== (const dense2D_iterator<isConst,T,Offset,InnerOneD,OneD>& x,
            const dense2D_iterator<isConst,T,Offset,InnerOneD,OneD>& y)
{
  return x.index() == y.index();
}

template <int isConst, class T, class Offset, class InnerOneD, class OneD>
inline bool
operator!= (const dense2D_iterator<isConst, T,Offset,InnerOneD,OneD>& x,
            const dense2D_iterator<isConst, T,Offset,InnerOneD,OneD>& y)
{
  return x.index() != y.index();
}

template <int isConst,class T, class Offset, class InnerOneD, class OneD>
inline bool
operator< (const dense2D_iterator<isConst,T,Offset,InnerOneD,OneD>& x,
           const dense2D_iterator<isConst,T,Offset,InnerOneD,OneD>& y)
{
  return x.index() < y.index();
}



/*
  Workaround (g++ 2.91) helper class
 */

template <class Strided>
struct __bracket { };

template <>
struct __bracket<strided_tag> {
  template <class OneD, class InnerOneD, class elt_type, class size_type>
  inline OneD
  operator()(elt_type* d, size_type len, size_type f, size_type ld,
             const OneD*, const InnerOneD*) {
    InnerOneD vec(d , len, f);
    return OneD(vec, ld);
  }
};

template <>
struct __bracket<not_strided_tag> {
  template <class OneD, class InnerOneD, class elt_type, class size_type>
  inline OneD
  operator()(elt_type* d, size_type len, size_type f, size_type,
             const OneD*, const InnerOneD*) {
    return OneD(d, len, f);
  }
};



template<class T, class OffsetGen, int MM, int NN>
class dense2D;

template <class T, class OffsetGen, int MM, int NN>
class external2D;

//: Generic Dense 2-D Container
//!category: containers
//!component: type
//
// The generic_dense2D container implements sevaral of the MTL storage
// types.  They include dense, packed, banded, and banded_view.  The
// common theme here is that the matrix is stored in a contiguous
// piece of memory.  The differences in these storage types has to do
// with where to find the OneD segements in the linear
// memory. Caclulating these offsets is the job of the Offset concept,
// which has a model to handle each of the different storage types:
// rect_offset, strided_offset, banded_offset, packed_offset, and
// banded_view_offset.
//
// There are two derived classes of generic_dense2D that specify the
// memory management, dense2D and external2D. The dense2D version owns
// its memory, while the external2D imports its memory from somewhere
// else through a pointer (which allows for interoperability with
// other codes -- even with Fortran!).  <p>
//
//!definition: dense2D.h
//!tparam: RepType - The Container used to store the elements
//!tparam: RepPtr - The type used to reference to the container
//!tparam: OffsetGen - The generator that creates the Offset class
//!tparam: MM - For static sized matrix, the major dimension
//!tparam: NN - For static sized matrix, the minor dimension
//!models: TwoDStorage

template <class RepType, class RepPtr, class OffsetGen, int MM, int NN>
class generic_dense2D {
public:
  //: Static sizes (0 if dynamic)
  enum { M = MM, N = NN };

  //: The type for dimensions and indices
  typedef typename RepType::size_type size_type;
  //: The type for differences between iterators
  typedef typename RepType::difference_type difference_type;

protected:
  typedef std::pair<size_type,size_type> pair_type;
  typedef RepType reptype;
  typedef RepPtr rep_ptr;
  typedef typename RepType::value_type elt_type;

#if defined(_MSVCPP_)
  //JGS Nasty VC++ workaround
  typedef typename OffsetGen::type Offset;
#else
  typedef typename OffsetGen:: template bind<size_type>::type Offset;
#endif

  typedef dimension<elt_type> dyn_dim;
public:
  //: A pair type for dimensions
  typedef typename Offset::dim_type dim_type;

  //: A pair type for bandwidth
  typedef typename Offset::band_type band_type;

  /* Type Definitions */

  //: This is a dense matrix
  typedef dense_tag sparsity;

  typedef typename Offset::is_strided is_strided;

protected:
  typedef external_vec<elt_type, N> InnerOneD;

#if defined(_MSVCPP_)
  enum { offset_strided = Offset::IS_STRIDED };
  typedef typename IF<offset_strided, strided1D<InnerOneD>, InnerOneD>::RET OneD;
#else
  typedef typename Offset:: template bind_oned<InnerOneD>::type OneD;
#endif

  typedef OneD OneDRef;
  typedef OneD ConstOneDRef;
public:

  //: The 1D container type
  typedef OneD value_type;
  //: The type for a reference to value_type
  typedef value_type reference;
  //: The type for a const reference to value_type
  typedef value_type const_reference;

  //: The iterator type
  typedef dense2D_iterator<0,elt_type, Offset, InnerOneD, OneD> iterator;

  //: The const iterator type
  typedef dense2D_iterator<1,elt_type, Offset, InnerOneD, OneD> const_iterator;

  //: The reverse iterator type
  typedef reverse_iter<iterator> reverse_iterator;

  //: The const reverse iterator type
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  //: The type for the transpose of this container
  typedef generic_dense2D<RepType, RepPtr,
             typename OffsetGen::transpose_type, MM, NN> transpose_type;

  //: The type for a banded view of this container
  typedef generic_dense2D<RepType, RepPtr,
             typename OffsetGen::banded_view_type, MM, NN> banded_view_type;

  //: The type for a sub-section of this 2D container
  typedef external2D<elt_type, OffsetGen, MM, NN> submatrix_type;

#ifndef MTL_DISABLE_BLOCKING
  template <class Block>
  struct blocked_view {
    typedef block2D<Block, OffsetGen> type;
  };
#endif

  //: This is a stridable container, can use rows(A), columns(A)
  typedef typename Offset::strideability strideability;

  /* Constructors */

  //: Default Constructor
  inline generic_dense2D()
    : ld_(0), data_(0), starts(std::make_pair(0,0)) { }

  //: Normal Constructor
  inline generic_dense2D(rep_ptr data, size_type m, size_type n, size_type ld)
    : ld_(ld), data_(data),
      starts(std::make_pair(0,0)), offset(m, n, ld) { }

  //: Constructor with non-zero upper-left corner indices
  inline generic_dense2D(rep_ptr data, size_type m, size_type n,
			 size_type ld, dyn_dim s, char)
    : ld_(ld), data_(data),
      starts(std::make_pair(s.first(),s.second())), offset(m, n, ld) { }

  //: Static M, N constructor
  inline generic_dense2D(rep_ptr data, size_type ld)
    : ld_(ld), data_(data),
      starts(std::make_pair(0,0)), offset(M, N, ld) { }

  //: with bandwidth constructor
  inline generic_dense2D(rep_ptr data, size_type m, size_type n, size_type ld,
                         band_type bw)
    : ld_(ld), data_(data), starts(std::make_pair(0,0)),
      offset(m, n, ld, bw) { }

  //: Static M, N with bandwith?

  //: Copy Constructor
  inline generic_dense2D(const generic_dense2D& x)
    : ld_(x.ld_), data_(x.data_), starts(x.starts),
      offset(x.offset) { }

  //: Assignment Operator
  inline generic_dense2D& operator=(const generic_dense2D& x) {
    ld_ = x.ld_; data_ = x.data_; starts = x.starts; offset = x.offset;
    return *this;
  }

  //: Subclass Constructor
  inline generic_dense2D(rep_ptr d, const generic_dense2D& x)
    : ld_(x.ld_), data_(d), starts(x.starts), offset(x.offset) { }

  //: Transpose Constructor
  inline generic_dense2D(const transpose_type& x, do_transpose, do_transpose)
    : ld_(x.ld_), data_(x.data_), starts(x.starts), offset(x.offset) { }

  /* JGS, remove stream constructor, not very necessary
     just have them call another constructor
   */

  //: Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline generic_dense2D(rep_ptr data, MatrixStream& s, Orien)
    : ld_(Orien::map(dim_type(s.nrows(),s.ncols())).second()),
      data_(data),
      starts(std::make_pair(0,0)),
      offset(Orien::map(dim_type(s.nrows(),s.ncols())).first(),
             Orien::map(dim_type(s.nrows(),s.ncols())).second(),
             Orien::map(dim_type(s.nrows(),s.ncols())).second()) { }
  //: Banded Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline generic_dense2D(rep_ptr data, MatrixStream& s,
                         Orien, band_type bw)
    : ld_(Orien::map(dim_type(s.nrows(),s.ncols())).second()),
      data_(data),
      starts(std::make_pair(0,0)),
      offset(Orien::map(dim_type(s.nrows(),s.ncols())).first(),
             Orien::map(dim_type(s.nrows(),s.ncols())).second(),
             Orien::map(dim_type(s.nrows(),s.ncols())).second(),
             bw) { }

  //: Banded View Constructor
  template <class TwoD>
  inline generic_dense2D(rep_ptr data, const TwoD& x, band_type bw, banded_tag)
    : ld_(x.ld_),
      data_(data),
      starts(x.starts),
      offset(x.offset, bw) { }

// VC++ doesn't like this
  //friend class transpose_type;

  //: The destructor.
  inline ~generic_dense2D() { }

  /* Access Methods */


  /* Iterator Access Methods */

  //: Return an iterator pointing to the first 1D container
  inline iterator begin() {
    return iterator(data(), ld_, 0, starts, offset);
  }
  //: Return an iterator pointing past the end of the 2D container
  inline iterator end() {
    return iterator(data(), ld_, offset.twod_length(), starts, offset);
  }
  //: Return a const iterator pointing to the first 1D container
  inline const_iterator begin() const {
    return const_iterator(data(), ld_, 0, starts, offset);
  }
  //: Return a const iterator pointing past the end of the 2D container
  inline const_iterator end() const {
    return const_iterator(data(), ld_, offset.twod_length(),
                          starts, offset);
  }

  /* reverse iterators */

  //: Return a reverse iterator pointing to the last 1D container
  inline reverse_iterator rbegin() {
    return reverse_iterator(end());
  }
  //: Return a reverse iterator pointing past the start of the 2D container
  inline reverse_iterator rend() {
    return reverse_iterator(begin());
  }
  //: Return a const reverse iterator pointing to the last 1D container
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  //: Return a const reverse iterator pointing past the start of the 2D container
  inline const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }


  /* Element Access Methods */
  //: Return a reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline const elt_type& operator()(size_type i, size_type j) const {
    return *(data() + offset.elt(i, j));
  }
  //: Return a const reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline elt_type& operator()(size_type i, size_type j) {
    return *(data() + offset.elt(i, j));
  }

  /* Size Methods */

  //: Number of non-zeroes
  inline size_type nnz() const { return offset.major() * offset.minor(); }

  //: Capacity
  inline size_type capacity() const { return offset.major() * offset.minor(); }

  //: Major axis size
  inline size_type major() const { return offset.major(); }

  //: Minor axis size
  inline size_type minor() const { return offset.minor(); }

  //: Leading Dimension
  inline size_type ld() const { return ld_; }

  //: Memory Access
  inline const elt_type* data() const { return &(*data_)[0]; }
  inline elt_type* data() { return &(*data_)[0]; }

  /* obsolete
  inline const elt_type* get_contiguous() const { return data_->data(); }
  inline elt_type* get_contiguous() { return data_->data(); }

  inline void set_contiguous(elt_type*) { }
  */

  /* Vector Access Methods */

  //: OneD Access
  inline OneD operator[](size_type i) const {
    typedef OneD* oned_ptr;
    typedef InnerOneD* inner_oned_ptr;
    return __bracket<is_strided>()((elt_type*)data() + offset.oned_offset(i),
                                   offset.oned_length(i),
                                   starts.first, ld_,
                                   oned_ptr(),
                                   inner_oned_ptr());
  }



  /* All the submatrix stuff is in matrix_implementation for now

  inline submatrix_type sub_matrix(size_type m_start, size_type m_finish,
                              size_type n_start, size_type n_finish) {
    return submatrix_type(data_->data() + m_start * ld_ + n_start,
                     dim_type(m_finish - m_start, n_finish - n_start), ld_);
  }
  inline submatrix_type sub_matrix(size_type m_start, size_type n_start,
                              size_type m, size_type n) {
    return submatrix_type(data_->data() + m_start * ld_ + n_start,
                     dim_type(m, n), ld_);
  }

  inline submatrix_type section(size_type m_start, size_type n_start,
                           size_type m, size_type n) {
    return submatrix_type(data_->data() + m_start * ld_ + n_start,
                     dim_type(m, n), ld_, dim_type(m_start, n_start));
  }
  typedef range<size_type> Range;
  inline generic_dense2D operator()(Range m, Range n) {
    return generic_dense2D(data_->data() + m.start * ld_ + n.start,
                m.finish - m.start, n.finish - n.start, ld_);
  }

  inline OneD::subrange_type operator()(size_type i, Range n) {
    return operator[i](n);
  }
  typedef strided1D< InnerOneD > MinorVector;
  inline MinorVector minor_vector(size_type i) const {
    InnerOneD vec((elt_type*)data_->data() + i,
                  offset.major() * ld_, starts.first);
    return MinorVector(vec, ld_);
  }
  inline MinorVector::subrange_type operator()(Range m, size_type j) {
    return minor_vector(j)(m);
  }

  */

  /*JGS friend not working for transpose constructor
  protected:
  */
  size_type ld_;/* JGS redundant */
  rep_ptr data_;
  pair_type starts;
  Offset offset;
};


template <class T, class OffsetGen, int M, int N>
struct gen_dense2D;

/* why didn't I use std::vector here?
 or perhaps I should just use plain old memory here?
 will that work with the reference counting in terms
 of deallocating?
 */

//: Dense2D Storage Type
//
// Inherits from generic_dense2D. The class "owns" its data.
//
//!category: containers
//!component: type
//!tparam: T - the element type
//!tparam: OffsetGen - the Offset class generator
//!tparam: MM - For static sized matrix, the major dimension
//!tparam: NN - For static sized matrix, the minor dimension
//!models: TwoDStorage

template<class T, class OffsetGen, int MM = 0, int NN = 0>
class dense2D
 : public generic_dense2D< std::vector<T> ,
               refcnt_ptr< std::vector<T> >, OffsetGen, MM, NN >
/* : public generic_dense2D< bare_bones_array<T> ,
               refcnt_ptr< bare_bones_array<T> >, OffsetGen, MM, NN >
*/
{
public:
  typedef generic_dense2D< std::vector<T> ,
               refcnt_ptr< std::vector<T> >, OffsetGen, MM, NN> super;
  /*  typedef generic_dense2D< bare_bones_array<T> ,
                 refcnt_ptr< bare_bones_array<T> >, OffsetGen, MM, NN> super;
  */
  typedef typename super::Offset Offset;
  //: Pair type for dimension
  typedef typename Offset::dim_type dim_type;
  //: Pair type for bandwidth
  typedef typename Offset::band_type band_type;
  typedef typename super::reptype reptype;
  typedef typename super::rep_ptr rep_ptr;
  //: Unsigned integral type for dimensions and indices
  typedef typename super::size_type size_type;
  //: The transpose type
  typedef dense2D<T, typename OffsetGen::transpose_type,
                  MM, NN> transpose_type;
// VC++ doesn't like this
  //friend class transpose_type;
  //: This has internal storage
  typedef internal_tag storage_loc;

  //: Default Constructor
  inline dense2D() { }

  //: Constructor from Dimension Pair
  inline dense2D(dim_type dim)
    : super(new reptype(Offset::size(dim.first(), dim.second(), 0, 0)),
            dim.first(),
            dim.second(),
            dim.second()) { }

  //: Constructor from Dimension and Bandwidth Pairs
  inline dense2D(dim_type dim, band_type bw)
    : super(new reptype(Offset::size(dim.first(),dim.second(),
                                     bw.first(), bw.second())),
            dim.first(),
            dim.second(),
            dim.second(),
            bw) { }

  //: Copy Constructor
  inline dense2D(const dense2D& x)
    : super(x) { }

  //: Assignment Operator
  inline dense2D& operator=(const dense2D& x) {
    super::operator=(x);
    return *this;
  }

  //: Transpose Constructor
  inline dense2D(const transpose_type& x, do_transpose t, do_transpose)
    : super(x, t, t) { }

#if !defined(_MSVCPP_)
  // JGS, use actual stream types
  //: Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline dense2D(MatrixStream& s, Orien)
    : super(new reptype(Offset::size(Orien::map(dim_type(s.nrows(),
                                                         s.ncols())).first(),
                                     Orien::map(dim_type(s.nrows(),
                                                         s.ncols())).second(),
                                     0, 0)),
            s,
            Orien()) { }

  //: Matrix Stream Constructor with bandwidth
  template <class MatrixStream, class Orien>
  inline dense2D(MatrixStream& s, Orien, band_type bw)
    : super(new reptype(Offset::size(Orien::map(dim_type(s.nrows(),
                                                         s.ncols())).first(),
                                     Orien::map(dim_type(s.nrows(),
                                                         s.ncols())).second(),
                                     bw.first(), bw.second())),
            s,
            Orien(),
            bw) { }
#endif

#if 0
  // deprecated
  template <class SubMatrix>
  struct partitioned {
    typedef dense2D<SubMatrix, OffsetGen> type;
    typedef gen_dense2D<SubMatrix, OffsetGen> generator;
  };
#endif

#if 1 // This makes no sense. dense2D can not be a "view"
  //: banded view constructor
  template <class TwoD>
  inline dense2D(const TwoD& x, band_type bw, banded_tag)
    : super(x.data_, x, bw, banded_tag()) { }
#endif

  //: Destructor
  inline ~dense2D() { }

  inline void resize(size_type m, size_type n) {
    rep_ptr newdata = new reptype(Offset::size(m, n, 0, 0));
    size_type i, j;
    size_type M = MTL_MIN(m, super::offset.major());
    size_type N = MTL_MIN(n, super::offset.minor());
    for (i = 0; i < M; ++i)
      for (j = 0; j < N; ++j)
	(*newdata)[i * n + j] = (*this)(i,j);
    for (; i < m; ++i)
      for (; j < n; ++j)
      (*newdata)[i * n + j] = T();

    super::data_ = newdata;
    super::ld_ = n;
    super::offset.dim = dim_type(m, n);
    super::offset.ld = n;
  }

};

template <class T, class OffsetGen, int M, int N>
struct gen_external2D;

#ifndef MTL_DISABLE_BLOCKING
template <class Block, class OffsetGen, int M, int N>
struct gen_block2D;
#endif

//: blah
//!noindex:
template <class T, class OffsetGen, int M, int N>
struct gen_dense2D {
  typedef gen_dense2D<T, typename OffsetGen::transpose_type,N,M> transpose_type;
  typedef gen_external2D<T, OffsetGen,M,N> submatrix_type;

#ifndef MTL_DISABLE_BLOCKING
  template <class Block>
  struct blocked_view {
    typedef gen_block2D<Block, OffsetGen, M, N> type;
  };
#endif

  typedef gen_dense2D<T, typename OffsetGen::banded_view_type,M,N>
           banded_view_type;

  typedef dense2D<T, OffsetGen, M, N> type;
};


//: External2D Storage Type
//
// Inherits from generic_dense2D. The class does not "own" its data.
//
//!category: containers
//!component: type
//!tparam: T - the element type
//!tparam: OffsetGen - the Offset class generator
//!tparam: MM - For static sized matrix, the major dimension
//!tparam: NN - For static sized matrix, the minor dimension
//!models: TwoDStorage
//
template <class T, class OffsetGen, int MM = 0, int NN = 0>
class external2D
 : public generic_dense2D< external_vec<T,NN>,
                           external_vec<T,NN>*, OffsetGen, MM, NN >
{
  typedef generic_dense2D< external_vec<T,NN>,
                           external_vec<T,NN>*, OffsetGen, MM, NN > super;
public:
  external_vec<T,NN> rep;
  typedef dimension<T> dyn_dim;
  typedef typename super::Offset Offset;
  //: Pair type for dimension
  typedef typename Offset::dim_type dim_type;
  //: Pair type for bandwidth
  typedef typename Offset::band_type band_type;

  typedef typename super::reptype reptype;

  //: Unsigned integral type for dimensions and indices
  typedef typename super::size_type size_type;
  //: Type for the transpose
  typedef external2D<T, typename OffsetGen::transpose_type,
                     MM, NN> transpose_type;
// VC++ doesn't like this
  //friend class transpose_type;
  //: This has external storage
  typedef external_tag storage_loc;

  //: Default Constructor
  inline external2D() { }

  //: Construct from pointer and dimensions
  inline external2D(T* data, dim_type dim)
    : super(&rep, dim.first(), dim.second(), dim.second()),
      rep(data, dim.first() * dim.second())
  { }

  //: Construct from pointer, dimensions, and leading dimension
  inline external2D(T* data, dim_type dim, size_type ld)
    : super(&rep, dim.first(), dim.second(), ld),
      rep(data, dim.first() * ld)
  { }

  //: non-zero indices in upper left corner
  inline external2D(T* data, dim_type dim, size_type ld,
		    dyn_dim s, char)
    : super(&rep, dim.first(), dim.second(), ld, s, char()),
      rep(data, dim.first() * ld)
  { }

  //: Constructor with bandwith
  inline external2D(T* data, dim_type dim, band_type bw)
    : super(&rep, dim.first(), dim.second(), dim.second(), bw),
      rep(data, dim.first() * dim.second())
  { }
  //: Constructor with leading dimension and bandwith
  inline external2D(T* data, dim_type dim, size_type ld, band_type bw)
    : super(&rep, dim.first(), dim.second(), ld, bw),
      rep(data, dim.first() * ld)
  { }

  //: Copy Constructor
  inline external2D(const external2D& x)
    :  super(&rep, x), rep(x.rep)
  { }

  //: Assignment Operator
  inline external2D& operator=(const external2D& x) {
    rep = x.rep;
    super::operator=(x);
    super::data_ = &rep;
    return *this;
  }

  //: Transpose Constructor
  inline external2D(const transpose_type& x, do_transpose t, do_transpose)
    : super(x, t, t), rep(x.rep) { }

  /* JGS This conflicts with the external2D(T* data, dim_type dim,
     band_type bw) constructor, and I am not sure this is really
     needed anyway.

  //: Matrix Stream Constructor
  template <class MatrixStream, class Orien>
  inline external2D(T* data, MatrixStream& s, Orien o)
    : rep(data, s.nrows() * s.ncols()), super(&rep, s, o) { }
  template <class MatrixStream, class Orien>
  inline external2D(T* data, MatrixStream& s, Orien o,
                    band_type bw)
    : super(&rep, s, o, bw),
      rep(data, s.nrows() * s.ncols())
  { }
  */

  //: banded view constructor
  template <class TwoD>
  inline external2D(const TwoD& x, band_type bw, banded_tag)
    : super(&rep, x, bw, banded_tag()), rep((T*)x.data(), x.major() * x.ld()) { }


  inline ~external2D() { }
#if 0
  // deprecated
  template <class SubMatrix>
  struct partitioned {
    typedef dense2D<SubMatrix, OffsetGen> type;
    typedef gen_dense2D<SubMatrix, OffsetGen> generator;
  };
#endif
};

//: blah
//!noindex:
template <class T, class OffsetGen, int M, int N>
struct gen_external2D {
  typedef gen_external2D<T, typename OffsetGen::transpose_type, N, M> transpose_type;
  typedef gen_external2D<T, OffsetGen,M,N> submatrix_type;
  typedef gen_external2D<T, typename OffsetGen::banded_view_type,M,N>
           banded_view_type;

#ifndef MTL_DISABLE_BLOCKING
  template <class Block>
  struct blocked_view {
    typedef gen_block2D<Block, OffsetGen, M, N> type;
  };
#endif

  typedef external2D<T, OffsetGen, M, N> type;
};



} /* namespace mtl */


#endif /* MTL_DENSE2D_H */



