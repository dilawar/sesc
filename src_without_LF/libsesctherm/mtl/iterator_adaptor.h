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

#ifndef _MTL_ITERATOR_ADAPTOR_
#define _MTL_ITERATOR_ADAPTOR_

#include "mtl_iterator.h"
#include <assert.h>

//: The Iterator Adaptor Base Class
//  This class is a boiler-plate for iterator adaptor
//  classes.
//
//  Some of the operators are implemented in terms of the
//  other operators, so if the subclass wishes for the
//  normal relation between the operators, it only needs
//  to have the main operations impelemented. The
//  main operations are:
//
//  value_type operator*();
//   self& operator++();
//   self& operator--();
//   self& operator+=(Distance n);
//   self& operator-=(Distance n);
//   friend Distance operator-(const SubClass& x, const SubClass& y);
//   friend bool operator==(const SubClass& x, const SubClass& y);
//   friend bool operator<(const SubClass& x, const SubClass& y);
//
//
//   Subclass Responsibilities
//
//   default constructor:
//     call iterator_adapter(SubClass& me_)
//
//   copy constructor (and any other constructors):
//     call iterator_adaptor(SubClass& me_, const Iterator& x)
//
//   assignment operator:
//     do "me = this;" and call this operator=
//
//!category: iterators, adaptors
//!component: type
//!tparam: SubClass - The class that inherits from this class
//!tparam: Iterator - The iterator to be adapted
template <class SubClass, class Iterator>
class iterator_adaptor {
  typedef iterator_adaptor<SubClass, Iterator> self;
public:
  //: The value type
  //!wheredef: TrivialIterator
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  //: The difference type
  //!wheredef: InputIterator
#if defined(_MSVCPP_)
  typedef typename std::iterator_traits<Iterator>::distance_type
		                       difference_type;
  typedef difference_type distance_type;
  typedef value_type& reference; // JGS bad hack!, need to add template args for this
  typedef value_type* pointer;
#else
  typedef typename std::iterator_traits<Iterator>::difference_type
                               difference_type;
  //: The pointer type
  typedef typename std::iterator_traits<Iterator>::pointer pointer;

  //: The reference type
  typedef typename std::iterator_traits<Iterator>::reference reference;
#endif
  //: The iterator category
  typedef typename std::iterator_traits<Iterator>::iterator_category
                               iterator_category;


  typedef difference_type Distance;
  typedef Iterator iterator_type;

  inline difference_type index() const { return iter.index(); }

  //!wheredef: iterator_adaptor
  // call this from the Subclass default constructor
  inline iterator_adaptor(SubClass& me_)
    : me(&me_) { }

  //!wheredef: iterator_adaptor
  // call this from the Subclass copy constructor
  inline iterator_adaptor(SubClass& me_, const Iterator& x)
    : iter(x), me(&me_) { }

  inline iterator_adaptor(const self& x) {
    me = 0; iter = x.iter; /* this should never be invoked! */
  }

  // the destructor
  inline ~iterator_adaptor() { }

  // call this from the Subclass operator=, and
  // also the Subclass must perform me = this */
  inline self& operator=(const self& x) {
    iter = x.iter; return *this;
  }

  //: Convert to base iterator
  //!wheredef: iterator_adaptor
  inline operator Iterator() { return iter; }

  //: Access base iterator
  //!wheredef: iterator_adaptor
  inline Iterator base() const { return iter; }

  //: Dereference
  //!wheredef: Trivial Iterator
  inline reference operator*() const { return *iter; }

  //: Preincrement
  //!wheredef: Forward Iterator
  inline SubClass& operator++() { ++iter; return *me; }

  //: Postincrement
  //!wheredef: Forward Iterator
  inline SubClass operator++(int) {
    SubClass tmp = (*me);
    ++(*me);
    return tmp;
  }

  //: Preincrement
  //!wheredef: Bidirectional Iterator
  inline SubClass& operator--() { --iter; return *me; }

  //: Postincrement
  //!wheredef: Bidirectional Iterator
  inline SubClass operator--(int) {
    SubClass tmp = (*me);
    --(*me);
    return tmp;
  }

  //: Iterator addition
  //!wheredef: Random Access Iterator
  inline SubClass operator+(Distance n) const {
    SubClass tmp = (*me);
    tmp += n;
    return tmp;
  }

  //: Advance a distance
  //!wheredef: Random Access Iterator
  inline SubClass& operator+=(Distance n) {
    std::advance(iter, n);
    return (*me);
  }

  //: Subtract a distance
  //!wheredef: Random Access Iterator
  inline SubClass operator-(Distance n) const {
    SubClass tmp = (*me);
    tmp -= n;
    return tmp;
  }

  //: Retreat a distance
  //!wheredef: Random Access Iterator
  inline SubClass& operator-=(Distance n) {
    std::advance(iter, -n);
    return (*me);
  }

  //: Access at an offset
  inline value_type operator[](Distance n) const {
    SubClass tmp = (*me);
    return *(tmp += n);
  }

  inline Distance operator-(const SubClass& y) {
    return iter - y.iter;
  }


  /* had a problem with this compilng example y_ax_y.cc
  inline friend Distance operator-(const SubClass& x, const SubClass& y) {
    return x.iter - y.iter;
  }
  */

  /* in terms of STL algorithms, it currently does no good to have the
   * Iterator interoperable variations on these comparisons because STL
   * requires the first and last iterators to be of the same type
   */

  inline bool operator==(const SubClass& y) const {
    return iter == y.iter;
  }

  //: Inequality
  //!wheredef: Trivial Iterator
  inline bool operator!=(const SubClass& y) const {
    return !(iter == y.iter);
  }

  //: Less than
  //!wheredef: Random Access Iterator
  inline bool operator<(const SubClass& y) const {
    return iter < y.iter;
  }

protected:
  Iterator iter;
  SubClass* me;
};

#endif



#if 0
  // g++ has a linking problem when iterators definitions nested
  //  inside container classes, and won't handle these as
  //  friends, have to declare them as members instead

  //: Equality
  //!wheredef: Trivial Iterator
  inline friend bool operator==(const SubClass& x, const SubClass& y) {
    return x.iter == y.iter;
  }
  inline friend bool operator==(const SubClass& x, const Iterator& y) {
    return x.iter == y;
  }
  inline friend bool operator==(const Iterator& x, const SubClass& y) {
    return x == y.iter;
  }

  //: Inequality
  //!wheredef: Trivial Iterator
  inline friend bool operator!=(const SubClass& x, const SubClass& y) {
    return !(x == y);
  }
  inline friend bool operator!=(const SubClass& x, const Iterator& y) {
    return !(x == y);
  }
  inline friend bool operator!=(const Iterator& x, const SubClass& y) {
    return !(x == y);
  }

  //: Less than
  //!wheredef: Random Access Iterator
  inline friend bool operator<(const SubClass& x, const SubClass& y) {
    return x.iter < y.iter;
  }
  inline friend bool operator<(const SubClass& x, const Iterator& y) {
    return x.iter < y;
  }
  inline friend bool operator<(const Iterator& x, const SubClass& y) {
    return x < y.iter;
  }
#endif
