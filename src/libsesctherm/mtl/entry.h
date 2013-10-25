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


#ifndef ENTRY_H
#define ENTRY_H

#include <iosfwd>

#include "mtl_config.h"

namespace mtl {

  //:
  // Definintion for entries in sparse matrices
  //
  // There is one index stored with the element.
  //!category: utilities
  //!component: type

template <class T>
struct entry1 {
  /* This is kind of bad because it gets padded
     out to 16 bytes, which wastes space/bandwidth
     */

  typedef entry1<T> self;

  typedef T value_type;

  int index;
  mutable value_type value;

  inline entry1() : index(-1) { }

  inline entry1(int i, value_type v = value_type()) : index(i), value(v) { }

  inline entry1(const self& x) : index(x.index), value(x.value) { }

  inline self& operator=(const self& x) {
    index = x.index;
    value = x.value;
    return *this;
  }

  inline bool operator < (const entry1& e) const {
    return index < e.index; }

  inline bool operator == (const entry1& e) const {
    return index == e.index; }

  inline bool operator != (const entry1& e) const {
    return index != e.index; }
};

template <class T>
std::ostream& operator<<(std::ostream& os, const entry1<T>& e)
{
  os << "(" << e.index << "," << e.value << ") ";
  return os;
}

//: blah
//!noindex:
template <class OneD>
class elt_ref {
  typedef elt_ref self;
public:
  typedef typename OneD::value_type value_type;

  inline elt_ref() : vec(OneD()), i(-1) { }

  inline elt_ref(OneD& m, int i_)
    : vec(m), i(i_) {
      iter = vec.find(i);
      if (iter != vec.end()) {
        if (iter.index() == i)
          val = *iter;
        else
          val = value_type();
      } else
        val = value_type();
  }

  inline operator value_type() const {
    return val;
  }

  inline value_type operator=(value_type v) {
    if (iter != vec.end() && iter.index() == i) {
      *iter = val = v;
    } else {
      val = v;
      iter = vec.insert(iter, i, v);
    }
    return val;
  }

  inline value_type operator+=(value_type v) {
    if (iter != vec.end() && iter.index() == i) {
      *iter = val += v;
    } else {
      val = v;
      iter = vec.insert(iter, i, v);
    }
    return val;
  }

  inline value_type operator*=(value_type v) {
    if (iter != vec.end() && iter.index() == i)
      *iter = val *= v;
    return val;
  }


  inline value_type operator/=(const self& v) {
    if (iter != vec.end() && iter.index() == i)
      *iter = val /= v.val;
    return val;
  }

  inline value_type operator/=(value_type v) {
    if (iter != vec.end() && iter.index() == i)
      *iter = val /= val;
    return val;
  }

  inline value_type operator-=(value_type v) {
    if (iter != vec.end() && iter.index() == i)
      *iter = val -= v;
    else {
      val = -v;
      iter = vec.insert(iter, i, val);
    }
    return val;
  }

  inline value_type operator=(const self& a) {
    if (iter != vec.end() && iter.index() == i)
       *iter = val = a.val;
    else
      iter = vec.insert(iter, i, a.val);
    return val;
  }

  /*protected: */
  OneD vec;
  /* JGS, value_typeobably performance issue here, want
   OneD& vec;
   but vec may be temporary object in the 2D, like comvalue_typeessed2D
   */
  typename OneD::iterator iter;
  int i;
  value_type val;
};



//: blah
//!noindex:
template <class OneD>
class const_elt_ref {
  typedef const_elt_ref self;
public:
  typedef typename OneD::value_type value_type;

  /*  inline const_elt_ref() : vec(0), i(0) { }*/
  template <class EltRef>
  inline const_elt_ref(const EltRef& elt) {
    typename OneD::const_iterator iter = elt.vec.find(elt.i);
    if (iter != elt.vec.end()) {
      if (iter.index() == elt.i)
        val = *iter;
      else
        val = value_type();
    } else
      val = value_type();
  }

  inline const_elt_ref(const OneD& vec, int i) {
    typename OneD::const_iterator iter = vec.find(i);
    if (iter != vec.end()) {
      if (iter.index() == i)
        val = *iter;
      else
        val = value_type();
    } else
      val = value_type();
  }

  inline operator value_type() const { return val; }

protected:
  value_type val;
};


#if (__GNUC__ < 9) /* let the conversion operator handle all these ops */

template <class T>
std::ostream& operator<<(std::ostream& os, const elt_ref<T>& er) {
  typedef typename elt_ref<T>::value_type value_type;
  os << value_type(er);
  return os;
}

template <class OneD>
std::ostream& operator<<(std::ostream& os, const const_elt_ref<OneD>& er) {
  typedef typename const_elt_ref<OneD>::value_type value_type;
  os << value_type(er);
  return os;
}


#define ELTREF_BINARY_OP(OP) \
template <class T, class U> \
T operator OP (const T& a, const elt_ref<U>& e) { \
  return a OP T(e); \
} \
template <class T, class U> \
T operator OP (const elt_ref<U>& e, const T& a) { \
  return T(e) OP a; \
} \
template <class V, class U> \
typename elt_ref<V>::value_type \
operator OP (const elt_ref<V>& f, const elt_ref<U>& e) { \
  typedef typename elt_ref<V>::value_type T; \
  return T(f) OP T(e); \
}

ELTREF_BINARY_OP(+)
ELTREF_BINARY_OP(-)
ELTREF_BINARY_OP(*)
ELTREF_BINARY_OP(/)

#define ELTREF_COMPARISON_OP(OP) \
template <class T, class U> \
bool operator OP (const T& a, const elt_ref<U>& e) { \
  return a OP T(e); \
} \
template <class T, class U> \
bool operator OP (const elt_ref<U>& e, const T& a) { \
  return T(e) OP a; \
} \
template <class V, class U> \
bool operator OP (const elt_ref<V>& f, const elt_ref<U>& e) { \
  typedef typename elt_ref<V>::value_type T; \
  return T(f) OP T(e); \
}

ELTREF_COMPARISON_OP(==)
ELTREF_COMPARISON_OP(!=)
ELTREF_COMPARISON_OP(<)
ELTREF_COMPARISON_OP(>)
ELTREF_COMPARISON_OP(<=)
ELTREF_COMPARISON_OP(>=)

#define CONST_ELTREF_BINARY_OP(OP) \
template <class T, class U> \
T operator OP (const T& a, const const_elt_ref<U>& e) { \
  return a OP T(e); \
} \
template <class T, class U> \
T operator OP (const const_elt_ref<U>& e, const T& a) { \
  return T(e) OP a; \
} \
template <class V, class U> \
typename const_elt_ref<V>::value_type \
operator OP (const const_elt_ref<V>& f, const const_elt_ref<U>& e) { \
  typedef typename const_elt_ref<V>::value_type T; \
  return T(f) OP T(e); \
}

CONST_ELTREF_BINARY_OP(+)
CONST_ELTREF_BINARY_OP(-)
CONST_ELTREF_BINARY_OP(*)
CONST_ELTREF_BINARY_OP(/)


#define CONST_ELTREF_COMPARISON_OP(OP) \
template <class T, class U> \
bool operator OP (const T& a, const const_elt_ref<U>& e) { \
  return a OP T(e); \
} \
template <class T, class U> \
bool operator OP (const const_elt_ref<U>& e, const T& a) { \
  return T(e) OP a; \
} \
template <class V, class U> \
bool \
operator OP (const const_elt_ref<V>& f, const const_elt_ref<U>& e) { \
  typedef typename const_elt_ref<V>::value_type T; \
  return T(f) OP T(e); \
}

CONST_ELTREF_COMPARISON_OP(==)
CONST_ELTREF_COMPARISON_OP(!=)
CONST_ELTREF_COMPARISON_OP(<)
CONST_ELTREF_COMPARISON_OP(>)
CONST_ELTREF_COMPARISON_OP(<=)
CONST_ELTREF_COMPARISON_OP(>=)


#endif

  //:
  // two indices stored with each element.

template <class T>
struct entry2 {
  typedef entry2<T> self;
  typedef T value_type;

  int row, col;
  value_type value;

  inline entry2() : row(-1), col(-1), value(0.0) { }
  inline entry2(int r, int c, value_type v = value_type())
    : row(r), col(c), value(v) { }
  inline entry2(const self& x) : row(x.row), col(x.col), value(x.value) { }
  inline self& operator =(const self& x) {
    row = x.row;
    col = x.col;
    value = x.value;
    return *this;
  }
  inline bool operator == (const entry2& e) const {
    return row == e.row && col == e.col; }
};

} /* namespace mtl */


#endif


