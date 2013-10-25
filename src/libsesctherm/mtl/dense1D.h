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
//===========================================================================

#ifndef MTL_DENSE1D_H
#define MTL_DENSE1D_H

#include <vector>
#include <utility>

#include "refcnt_ptr.h"
#include "dense_iterator.h"
#include "reverse_iter.h"
#include "light1D.h"
#include "mtl_config.h"
#include "matrix_traits.h"
#include "scaled1D.h"
#include "mtl_exception.h"

/*
#include "concept_checks.h"
*/

/**
  @name 1 Dimensional Containers and Adaptors

  The MTL 1-dimensional containers can be used as vectors and also to
  compose 2-dimensional containers. There are also 2 adaptors, the
  scaled1D and strided1D classes.  The adaptors are not meant to be
  used directly. Instead they are used to implement the scaled() and
  strided() functions for wrapping up MTL function arguments.

  One can always access the row and columns vectors of a matrix, which
  are first-class vectors in MTL.  They can be used with the MTL
  vector algorithms.  This is demonstrated in several of the example
  programs.

  */


/*
 Think about having a non-zero starting index


 note, do not use std::vector any more because
 it puts all kinds of checks into the operator[]
 or find out how to turn off the checking.
 */


namespace mtl {

  //: Dense 1-D Container
  //!category: containers
  //!component: type
  //
  //   This is the primary class that you will need to use as a Vector.
  //   This class uses the STL <tt>vector</tt> for its implementation.
  //   The <TT>dense1D</TT> class serves as a handle
  //   to the <TT>vector</TT>. The MTL algorithms assume that the
  //   vector and matrix arguments are handles, and will not
  //   work with the STL style containers.
  //   For interoperability, one can create a <TT>dense1D</TT>
  //   from pre-existing memory. In this case, the mtl reference
  //   counting does not delete the memory.
  //
  //!definition: dense1D.h
  //!tparam: RepType - the underlying representation
  //!models: Vector

template <class T, int NN = 0>
class dense1D {

  /*  CLASS_REQUIRES(T, Assignable);*/

  typedef dense1D<T,NN> self;
  typedef std::vector<T> rep_type;
  /*  typedef RepType rep_type; */
  typedef refcnt_ptr< rep_type > rep_ptr;
public:
  /**@name Type Definitions */
  enum { N = NN };

  /*  enum { dimension = 1 }; */
  typedef oned_tag dimension;

  //: The sparsity tag
  typedef dense_tag sparsity;
  //: The scaled type of this container
  //!wheredef: Scalable
  typedef scaled1D< self > scaled_type;
  //: The value type
  //!wheredef: Container
  typedef typename rep_type::value_type value_type;
  //: The reference type
  //!wheredef: Container
  typedef typename rep_type::reference reference;
  //: blah
  //!wheredef: Container
  typedef typename rep_type::const_reference const_reference;
#if !defined( _MSVCPP_ )
  //: blah
  //!wheredef: Container
  typedef typename rep_type::pointer pointer;
#else
  typedef value_type* pointer;
#endif
  //: blah
  //!wheredef: Container
  typedef typename rep_type::size_type size_type;
  //: blah
  //!wheredef: Container
  typedef typename rep_type::difference_type difference_type;
  //: blah
  //!wheredef: Container
#if !defined( _MSVCPP_ )
  typedef dense_iterator<typename rep_type::iterator> iterator;
  //: blah
  //!wheredef: Container
  typedef dense_iterator<typename rep_type::const_iterator> const_iterator;
#else
  typedef dense_iterator<typename rep_type::value_type, 0> iterator;
  typedef dense_iterator<typename rep_type::value_type, 1> const_iterator;
#endif
  //: blah
  //!wheredef: Reversible Container
  typedef reverse_iter<iterator> reverse_iterator;
  //: blah
  //!wheredef: Reversible Container
  typedef reverse_iter<const_iterator> const_reverse_iterator;


  typedef light1D<value_type> subrange_type;

  typedef std::pair<size_type,size_type> range_type;

  /**@name Constructors */

  typedef self Vec;

  typedef size_type Vec_size_type;
  typedef difference_type Vec_difference_type;
  typedef iterator Vec_iterator;
  typedef const_iterator Vec_const_iterator;
  typedef value_type Vec_value_type;

#if 1
  class IndexArray {
  public:

    typedef Vec_size_type size_type;
    typedef Vec_difference_type difference_type;
    typedef Vec_size_type value_type;

    class iterator {
    public:
      typedef size_type value_type;
      typedef size_type reference;
      typedef size_type* pointer;
      typedef Vec_difference_type difference_type;
      typedef typename std::iterator_traits<Vec_iterator>::iterator_category
                    iterator_category;
      iterator(Vec_iterator iter, Vec_iterator e) : i(iter), end(e) {
        while (*i == Vec_value_type(0)) ++i;
      }
      reference operator*() const { return i.index(); }
      iterator& operator++() {
        ++i; while (*i == Vec_value_type(0) && i != end) ++i;
        return *this; }
      iterator operator++(int) { iterator t = *this; ++(*this); return t; }
      iterator& operator--() {
        --i; while (*i == Vec_value_type(0) && i != end) --i;
        return *this; }
      iterator operator--(int) { iterator t = *this; --(*this); return t; }
      difference_type operator-(const iterator& x) const { return i - x.i; }
      bool operator==(const iterator& x) const { return i == x.i; }
      bool operator!=(const iterator& x) const { return i != x.i; }
      bool operator<(const iterator& x) const { return i < x.i; }
      Vec_iterator i;
      Vec_iterator end;
    };
    class const_iterator {
    public:
      typedef size_type value_type;
      typedef size_type reference;
      typedef size_type* pointer;
      typedef Vec_difference_type difference_type;
      typedef typename std::iterator_traits<Vec_iterator>::iterator_category iterator_category;
      const_iterator(Vec_const_iterator iter, Vec_const_iterator e)
        : i(iter), end(e) {
        while (*i == Vec_value_type(0) && i != end) ++i;
      }
      reference operator*() const { return i.index(); }
      const_iterator& operator++() {
        ++i; while (*i == Vec_value_type(0) && i != end) ++i;
        return *this; }
      const_iterator operator++(int) {
        const_iterator t = *this; ++(*this); return t; }
      const_iterator& operator--() {
        --i; while (*i == Vec_value_type(0)) --i;
        return *this; }
      const_iterator operator--(int) {
        const_iterator t = *this; --(*this); return t; }
      difference_type operator-(const const_iterator& x) const {
        return i - x.i; }
      bool operator==(const const_iterator& x) const { return i == x.i; }
      bool operator!=(const const_iterator& x) const { return i != x.i; }
      bool operator<(const const_iterator& x) const { return i < x.i; }
      Vec_const_iterator i;
      Vec_const_iterator end;
    };

    inline IndexArray(const Vec& v) : vec((Vec*)&v) { }
    inline iterator begin() { return iterator(vec->begin(), vec->end()); }
    inline iterator end() { return iterator(vec->end(), vec->end()); }
    inline const_iterator begin() const{
      return const_iterator(((const Vec*)vec)->begin(),
			    ((const Vec*)vec)->end());
    }
    inline const_iterator end() const {
      return const_iterator(((const Vec*)vec)->end(),
			    ((const Vec*)vec)->end());
    }

    size_type size() const {
      size_type s = 0;
      Vec_const_iterator i;
      for (i = ((const Vec*)vec)->begin(); i != ((const Vec*)vec)->end(); ++i)
        if (*i != Vec_value_type(0)) ++s;
      return s;
    }

    Vec* vec;
  };

  typedef IndexArray IndexArrayRef;
#else
  /* JGS, problem. IndexArray introduces several requirements on value
     type that interfere with using dense1D as the backbone for
     array2D */
  typedef self IndexArray;
  typedef self IndexArrayRef;
#endif

  //: Default Constructor
  //!wheredef: Container
  inline dense1D() : rep(new rep_type()), first(0) { }

  //: Non-Initializing Constructor not very standard :(
  //!wheredef: Sequence
  inline dense1D(size_type n) : rep(new rep_type(n)), first(0) { }

  /* JGS Don't ever try to fold these two constructors into one with a
   * defualt. The array2D depends on being able to construct without
   * initialization otherwise all vectors in the array2D end up being
   * handles to the same vector
   */

  //: Initializing Constructor
  //!wheredef: Sequence
  inline dense1D(size_type n, const value_type& init)
    : rep(new rep_type(n,init)), first(0) { }

  //: Copy Constructor  (shallow copy)
  //!wheredef: ContainerRef
  inline dense1D(const self& x) : rep(x.rep), first(x.first) { }

  //: The destructor.
  //!wheredef: Container
  inline ~dense1D() { }

  //: Assignment Operator (shallow copy)
  //!wheredef: AssignableRef
  inline self& operator=(const self& x) {
    rep = x.rep; first = x.first;
    return *this;
  }

  /* Access Methods */

  /* Iterator Access Methods */

  //: Return an iterator pointing to the beginning of the vector
  //!wheredef: Container
  inline iterator begin() { return iterator(rep->begin(),0, first); }
  //: Return an iterator pointing past the end of the vector
  //!wheredef: Container
  inline iterator end() { return iterator(rep->begin(), rep->size(), first); }
  //: Return a const iterator pointing to the begining of the vector
  //!wheredef: Container
  inline const_iterator begin() const {
    return const_iterator(rep->begin(), 0, first);
  }
  //: Return a const iterator pointing past the end of the vector
  //!wheredef: Container
  inline const_iterator end() const{
    return const_iterator(rep->begin(), rep->size(), first);
  }
  //: Return a reverse iterator pointing to the last element of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rbegin() { return reverse_iterator(end()); }
  //: Return a reverse iterator pointing past the end of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rend() { return reverse_iterator(begin()); }
  //: Return a const reverse iterator pointing to the last element
  //!wheredef: Reversible Container
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  //: Return a const reverse iterator pointing past the end of the vector
  //!wheredef: Reversible Container
  inline const_reverse_iterator rend() const{
    return const_reverse_iterator(begin());
  }

  /* Element Access Methods */

  //: Return a reference to the element with index i
  //!wheredef: Random Access Container
  inline reference operator[](size_type i) MTL_THROW_ASSERTION {
    MTL_ASSERT(i - first < size(), "dense1D::operator[]");
    return (*rep)[i - first];
  }

  //: Return a const reference to the element with index i
  //!wheredef: Random Access Container
  inline const_reference operator[](size_type i) const MTL_THROW_ASSERTION {
    MTL_ASSERT(i - first < size(), "dense1D::operator[]");
    return (*rep)[i - first];
  }

  inline subrange_type operator()(range_type r) const MTL_THROW_ASSERTION {
    return subrange_type(data() + r.first - first,
			 r.second - r.first,
			 0);
  }
  inline subrange_type operator()(size_type s, size_type f) const
    MTL_THROW_ASSERTION
  {
    return subrange_type(const_cast<value_type*>(data()) + s - first,
			 f - s,
			 0);
  }

  /* Size Methods */

  //: Return the size of the vector
  //!wheredef: Container
  inline size_type size() const { return rep->size(); }
  //: Return the number of non-zeroes
  inline size_type nnz() const { return rep->size(); }
  //:
  inline void resize(size_type n) { rep->resize(n); }
  inline void resize(size_type n, const T& x) { rep->resize(n, x); }
  //:
  inline size_type capacity() const { return rep->capacity(); }

  //:
  void reserve(size_type n) { rep->reserve(n); }

  //: Raw Memory Access
  inline const value_type* data() const { return &(*rep)[0]; }
  inline pointer data() { return &(*rep)[0]; }

  //:
  //!wheredef: Container
  iterator insert (iterator position, const value_type& x = value_type()) {
    return iterator(rep->insert(position.base(), x), position.index()+1);
    // JGS, not sure about what to do with the index here
  }

  void insert (iterator position, size_type n,
                   const value_type& x = value_type()) {
    rep->insert(position.base(), n, x);
    /*    return iterator(, position.index()+n);
     JGS, not sure about what to do with the index here
     */
  }
  void push_back(const value_type& x) { rep->push_back(x); }

  inline IndexArrayRef nz_struct() const {
    return IndexArrayRef(*this);
  }

  self& adjust_index(size_type delta) {
    first += delta;
    return *this;
  }

protected:

  rep_ptr rep;
  size_type first;


};


} /* namespace mtl */



#endif

