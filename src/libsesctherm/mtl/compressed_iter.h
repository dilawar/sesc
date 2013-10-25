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

#ifndef MTL_COMPRESSED_ITER_H
#define MTL_COMPRESSED_ITER_H

#include "mtl_iterator.h"
#include "mtl_config.h"
#include "meta_if.h"

namespace mtl {

  //: Compressed Storage Iterator
  //
  // Iterates through a value array and index array.
  // This is used by the compressed1D and compressed2D classes.
  //
  //!category: iterators
  //!component: type
  //!tparam: Value - The type of the value array
  //!tparam: Index - The type of the index array
template <int isConst, class Value, class Index, int IND_OFFSET>
class compressed_iter {
  typedef compressed_iter self;
  typedef Value values_t;
  typedef typename values_t::iterator        value_iterator;
  typedef typename values_t::const_iterator  value_const_iterator;
  typedef typename IF<isConst, value_const_iterator, value_iterator>::RET value_iter_t;
  typedef Index indices_t;
  typedef typename indices_t::iterator       index_iterator;
  typedef typename indices_t::const_iterator index_const_iterator;
  typedef typename IF<isConst, index_const_iterator, index_iterator>::RET index_iter_t;
public:
#if defined(_MSVCPP_)
	typedef std::random_access_iterator_tag iterator_category;
#else
  typedef typename std::iterator_traits<value_iterator>::iterator_category iterator_category;
#endif
  typedef typename Value::value_type value_type;
  typedef int difference_type;
  typedef difference_type distance_type;
  typedef typename IF<isConst, const value_type*,value_type*>::RET pointer;
  typedef typename IF<isConst, const value_type&, value_type&>::RET reference;
  typedef const value_type& const_reference;

  inline compressed_iter() : pos (0) { }

  inline compressed_iter(const self& x)
    : index_iter_(x.index_iter_), value_iter_(x.value_iter_), pos(x.pos) { }

  struct bogus { };

  typedef typename IF<isConst, compressed_iter<0,Value,Index,IND_OFFSET>,
                      bogus>::RET NonConst;

  inline compressed_iter(const NonConst& x)
    : index_iter_(x.index_iter_), value_iter_(x.value_iter_), pos(x.pos) { }

  inline self& operator=(const self& x) {
    index_iter_ = x.index_iter_;
    value_iter_ = x.value_iter_;
    pos         = x.pos;
    return *this;
  }

  inline compressed_iter(value_iter_t vals, index_iter_t inds, int p)
    : index_iter_(inds), value_iter_(vals), pos(p) { }

  inline int index() const {
    return index_iter_[pos] + IND_OFFSET;  /* F to C */
  }
  inline void set_index(int i) {
    index_iter_[pos] = i;
  }

  inline reference operator*() const { return value_iter_[pos]; }

  inline self& operator++() { ++pos; return *this; }
  inline self operator++(int) {
    self tmp = *this; ++pos; return tmp;
  }
  inline self& operator--() { --pos; return *this; }
  inline self operator--(int) {
    self tmp = *this; --pos; return tmp;
  }
  inline self& operator+=(int n) { pos += n; return *this; }
  inline self& operator-=(int n) { pos -= n; return *this; }

  inline self operator+(int n) {
    return self(value_iter_, index_iter_, pos + n);
  }
  inline self operator-(int n) const {
    return self(value_iter_, index_iter_, pos - n);
  }
  inline int operator-(const self& x) const {
    return pos - x.pos;
  }

  inline bool operator != (const self& y) const {
    return pos != y.pos;
  }
  inline bool operator < (const self& y) const {
    return pos < y.pos;
  }
  inline bool operator == (const self& y) const {
    return pos == y.pos;
  }

  inline index_iter_t index_iter() const { return index_iter_ + pos; }
  inline value_iter_t value_iter() const { return value_iter_ + pos; }
  //protected:
  index_iter_t index_iter_;
  value_iter_t value_iter_;
  int pos;
};



} /* namespace mtl */

#endif
