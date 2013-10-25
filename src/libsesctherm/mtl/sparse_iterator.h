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
//==================================================================

#ifndef MTL_SPARSE_ITERATOR_H
#define MTL_SPARSE_ITERATOR_H

#include "mtl_iterator.h"

namespace mtl {

using std::random_access_iterator_tag;
using std::output_iterator_tag;
using std::advance;
using std::distance;


  template <class Cat> struct choose {
    template <class Iterator, class self>
    static inline self add(Iterator i, int, int n, self*) {
      std::advance(i, n);
      return self(i);
    }
    template <class Iterator, class self>
    static inline self sub(Iterator i, int n, self*) {
      std::advance(i, -n);
      return self(i);
    }
    template <class self>
    static inline int diff(const self& x, const self& y) {
      return distance(x.iter, y.iter);
    }
    template <class Iterator>
    static inline void init(Iterator& iter, int& pos,
			    const Iterator& /*start*/,
			    const Iterator& finish) {
      iter = finish;
      pos = 0;
    }
    template <class self>
    static inline bool lessthan(const self& x, const self& y) {
      return x.pos < y.pos; }

    template <class self>
    static inline bool notequal(const self& x, const self& y) {
      return x.pos != y.pos; }

    template <class Iterator>
    static inline Iterator convert(const Iterator& iter, int) {
      return iter; }

    template <class Iterator>
    static inline int index(Iterator iter, int) {
      return (*iter).index; }

    template <class Iterator>
    static inline void inc(Iterator& iter, int&) { ++iter; }

    template <class Iterator>
    static inline void dec(Iterator& iter, int&) { --iter; }

    template <class Iterator, class Val>
    static inline Val&
    deref(Iterator iter, int, Val*) {
     return (*iter).value; }

    template <class Iterator>
    inline void add_assign(Iterator& iter, int&, int n) {
      advance(iter,n);  }

    template <class Iterator>
    inline void sub_assign(Iterator& iter, int&, int n) {
      advance(iter,n);  }

  };

  template <> struct choose<random_access_iterator_tag> {

    template <class Iterator, class self>
    static inline self add(Iterator iter, int pos, int n, self*) {
      return self(iter, pos + n); }

    template <class Iterator, class self>
    static inline self sub(Iterator iter, int pos, int n, self*) {
      return self(iter, pos - n); }

    template <class self>
    static inline int diff(const self& x, const self& y) {
      return x.pos - y.pos; }

    template <class Iterator>
    static inline void init(Iterator& iter, int& pos,
			    const Iterator& start, const Iterator& finish) {
      iter = start;
      pos = finish - start;
    }

    template <class self>
    static inline bool lessthan(const self& x, const self& y) {
      return x.iter < y.iter; }

    template <class self>
    static inline bool notequal(const self& x, const self& y) {
      return x.iter != y.iter; }

    template <class Iterator>
    static inline Iterator convert(const Iterator& iter, int pos) {
      return iter + pos; }

    template <class Iterator>
    static inline int index(Iterator iter, int pos) {
      return (*(iter + pos)).index; }

    template <class Iterator>
    static inline void inc(Iterator&, int& pos) { ++pos; }

    template <class Iterator>
    static inline void dec(Iterator&, int& pos) { --pos; }

    template <class Iterator, class Val>
    static inline Val&
    deref(const Iterator& iter, int pos, Val*) {
      return (*(iter + pos)).value; }

    template <class Iterator>
    static inline void add_assign(Iterator&, int& pos, int n) {
      pos += n; }

    template <class Iterator>
    static inline void sub_assign(Iterator&, int& pos, int n) {
      pos -= n; }

  };




template <class Iterator, class T>
class const_sparse_iterator;

//: Sparse Vector Iterator
//
// This iterators is used to implement the sparse1D adaptor.  The base
// iterator returns a entry1 (an index-value pair) and this iterator
// makes it look like we are just dealing with the value for
// dereference, while the index() method return the index.
//
//!category: iterators, adaptors
//!component: type
//!tparam: Iterator - the underlying iterator type
//!tparam: T - the value type
template <class Iterator, class T>
class sparse_iterator {
  typedef sparse_iterator<Iterator,T> self;
  typedef typename std::iterator_traits<Iterator>::iterator_category cat;
public:
  typedef cat iterator_category;
  typedef T value_type;
  typedef int difference_type;
  typedef T& reference;
  typedef T* pointer;

public:

  inline sparse_iterator() { }

  inline sparse_iterator(const sparse_iterator& x)
    : iter(x.iter), pos(x.pos) { }

  inline sparse_iterator(const Iterator& iter_, int p = 0)
    : iter(iter_), pos(p) { }

  inline sparse_iterator(const Iterator& start, const Iterator& finish) {
    choose<cat>::init(iter, pos, start, finish); }

  inline operator Iterator() const { return choose<cat>::convert(iter, pos); }

  inline int index() const { return choose<cat>::index(iter, pos); }

  inline bool operator!=(const self& x) const {
    return choose<cat>::notequal(*this, x); }

  inline bool operator<(const self& x) const {
    return choose<cat>::lessthan(*this, x); }

  inline reference operator*() const {
    return choose<cat>::deref(iter, pos, pointer()); }

  inline int operator-(const self& x) const {
    return choose<cat>::diff(*this, x); }

  inline self& operator++() {
    choose<cat>::inc(iter, pos); return *this; }

  inline self operator++(int) {
    self tmp = *this; choose<cat>::inc(iter, pos); return tmp; }

  inline self& operator--() {
    choose<cat>::dec(iter, pos); return *this; }

  inline self& operator+=(int n) {
    choose<cat>::add_assign(iter, pos, n); return *this; }

  inline self& operator-=(int n) {
    choose<cat>::sub_assign(iter, pos, n); return *this; }

  inline Iterator base() const { return choose<cat>::convert(iter, pos); }

  inline self operator+(int n) const {
    return choose<cat>::add(iter, pos, n, this); }
  inline self operator-(int n) const {
    return choose<cat>::sub(iter, pos, n, this); }

public: /* VC++ workaround */
  Iterator iter;
  int pos;
};

template <class Iterator, class T>
inline bool
operator==(const sparse_iterator<Iterator, T>& a,
	   const sparse_iterator<Iterator, T>& b)
{
  return a.base() == b.base();
}



template <class Iterator, class T>
class const_sparse_iterator {
  typedef const_sparse_iterator<Iterator,T> self;
  typedef typename std::iterator_traits<Iterator>::iterator_category cat;
public:
  typedef cat iterator_category;
  typedef T value_type;
  typedef int difference_type;
  typedef const T& reference;
  typedef T* pointer;
public:

  inline const_sparse_iterator() { }

  inline const_sparse_iterator(const const_sparse_iterator& x)
    : iter(x.iter), pos(x.pos) { }

  inline const_sparse_iterator(const Iterator& iter_, int p = 0)
    : iter(iter_), pos(p) { }

  inline const_sparse_iterator(const Iterator& start, const Iterator& finish) {
    choose<cat>::init(iter, pos, start, finish); }

  template <class Iter>
  inline const_sparse_iterator(const sparse_iterator<Iter,T>& si)
    : iter(si.iter), pos(si.pos) { }

  inline operator Iterator() const { return choose<cat>::convert(iter, pos); }

  inline int index() const { return choose<cat>::index(iter, pos); }

  inline bool operator!=(const self& x) const {
    return choose<cat>::notequal(*this, x); }

  inline bool operator<(const self& x) const {
    return choose<cat>::lessthan(*this, x); }

  inline reference operator*() const {
    return choose<cat>::deref(iter, pos, pointer()); }

  inline int operator-(const self& x) const {
    return choose<cat>::diff(*this, x); }

  inline self& operator++() {
    choose<cat>::inc(iter, pos); return *this; }

  inline self operator++(int) {
    self tmp = *this; choose<cat>::inc(iter, pos); return tmp; }

  inline self& operator--() {
    choose<cat>::dec(iter, pos); return *this; }

  inline self& operator+=(int n) {
    choose<cat>::add_assign(iter, pos, n); return *this; }

  inline self& operator-=(int n) {
    choose<cat>::sub_assign(iter, pos, n); return *this; }

  inline Iterator base() const { return choose<cat>::convert(iter, pos); }

  inline self operator+(int n) const {
    return choose<cat>::add(iter, pos, n, this); }
  inline self operator-(int n) const {
    return choose<cat>::sub(iter, pos, n, this); }

public: /* VC++ workaround */
  Iterator iter;
  int pos;
};

template <class Iterator, class T>
inline bool
operator==(const const_sparse_iterator<Iterator, T>& a,
	   const const_sparse_iterator<Iterator, T>& b)
{
  return a.base() == b.base();
}


} /* namespace mtl */

#endif


