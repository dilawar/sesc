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

#ifndef MTL_LINALG_VECTOR_H
#define MTL_LINALG_VECTOR_H


#include <utility>
#include <vector>

#include "refcnt_ptr.h"
#include "dense_iterator.h"
#include "reverse_iter.h"
#include "light1D.h"
#include "mtl_config.h"
#include "matrix_traits.h"
#include "scaled1D.h"
#include "mtl_exception.h"
#include "external_vector.h"


namespace mtl {

  //: Linalg Vector Adaptor
  //!category: containers, adaptors
  //!component: type
  //
  // This captures the main functionality of a dense MTL vector.  The
  // dense1D and external1D derive from this class, and specialize
  // this class to use either internal or external storage.
  //
  //!definition: linalg_vector.h
  //!tparam: RepType - the underlying representation
  //!models: Linalg_Vector

template <class RepType, class RepPtr = RepType*, int NN = 0>
class linalg_vec {
public:
  typedef linalg_vec self;
  typedef RepType rep_type;
  typedef RepPtr rep_ptr;

  enum { N = NN };

  /**@name Type Definitions */

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

  //: The const reference type
  //!wheredef: Container
  typedef typename rep_type::const_reference const_reference;

  //: The pointer (to the value_type) type
  //!wheredef: Container
  typedef typename rep_type::pointer pointer;

  //: The size type (non negative)
  //!wheredef: Container
  typedef typename rep_type::size_type size_type;

  //: The difference type (an integral type)
  //!wheredef: Container
  typedef typename rep_type::difference_type difference_type;

#if !defined( _MSVCPP_ )
  //: The iterator type
  //!wheredef: Container
  typedef dense_iterator<typename rep_type::iterator> iterator;

  //: The const iterator type
  //!wheredef: Container
  typedef dense_iterator<typename rep_type::const_iterator> const_iterator;
#else
  typedef dense_iterator<typename rep_type::value_type, 0, 0, size_type> iterator;
  typedef dense_iterator<typename rep_type::value_type, 1, 0, size_type> const_iterator;
#endif
  //: The reverse iterator type
  //!wheredef: Reversible Container
  typedef reverse_iter<iterator> reverse_iterator;

  //: The const reverse iterator type
  //!wheredef: Reversible Container
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  /* skip over the zeros and report the indices
     this implements the nonzero structure array
     */
  typedef linalg_vec<RepType, RepPtr, NN> Vec;

  typedef size_type Vec_size_type;
  typedef difference_type Vec_difference_type;
  typedef iterator Vec_iterator;
  typedef const_iterator Vec_const_iterator;

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
      typedef typename std::iterator_traits<Vec_iterator>::iterator_category iterator_category;
      iterator(Vec_iterator iter, Vec_iterator e) : i(iter), end(e) {
	while (*i == self::Vec_value_type(0)) ++i;
      }
      reference operator*() const { return i.index(); }
      iterator& operator++() {
	++i; while (*i == self::Vec_value_type(0) && i != end) ++i;
	return *this; }
      iterator operator++(int) { iterator t = *this; ++(*this); return t; }
      iterator& operator--() {
	--i; while (*i == self::Vec_value_type(0) && i != end) --i;
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
	while (*i == self::Vec_value_type(0) && i != end) ++i;
      }
      reference operator*() const { return i.index(); }
      const_iterator& operator++() {
	++i; while (*i == self::Vec_value_type(0) && i != end) ++i;
	return *this; }
      const_iterator operator++(int) {
	const_iterator t = *this; ++(*this); return t; }
      const_iterator& operator--() {
	--i; while (*i == self::Vec_value_type(0)) --i;
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
			    ((const Vec*)vec)->end()); }

    size_type size() const {
      size_type s = 0;
      Vec_const_iterator i;
      for (i = ((const Vec*)vec)->begin(); i != ((const Vec*)vec)->end(); ++i)
	if (*i != self::Vec_value_type(0)) ++s;
      return s;
    }

    Vec* vec;
  };

  //: The type for an array of the indices of the element in the vector
  //!wheredef: Vector
  typedef IndexArray IndexArrayRef;

  //: The type for a subrange vector-view of the original vector
  //!wheredef: Vector
  typedef light1D<value_type> subrange_type;

  typedef std::pair<size_type, size_type> range;

  /**@name Constructors */

  //: Default Constructor (allocates the container)
  //!wheredef: Container
  inline linalg_vec() : rep(0) { }

  //: Normal Constructor
  inline linalg_vec(rep_ptr x, size_type start_index)
    : rep(x), first(start_index) { }

  //: Copy Constructor  (shallow copy)
  //!wheredef: ContainerRef
  inline linalg_vec(const self& x) : rep(x.rep), first(x.first) { }

  //: The destructor.
  //!wheredef: Container
  inline ~linalg_vec() { }

  //: Assignment Operator (shallow copy)
  //!wheredef: AssignableRef
  inline self& operator=(const self& x) {
    rep = x.rep;
    first = x.first;
    return *this;
  }

  /**@name Access Methods */

  /**@name Iterator Access Methods */

  //: Return an iterator pointing to the beginning of the vector
  //!wheredef: Container
  inline iterator begin() { return iterator(rep->begin(), 0, first); }
  //: Return an iterator pointing past the end of the vector
  //!wheredef: Container
  inline iterator end() { return iterator(rep->begin(), rep->size(), first); }
  //: Return a const iterator pointing to the begining of the vector
  //!wheredef: Container
  inline const_iterator begin() const { return const_iterator(rep->begin(),
							      0, first); }
  //: Return a const iterator pointing past the end of the vector
  //!wheredef: Container
  inline const_iterator end() const{ return const_iterator(rep->begin(),
						       rep->size(), first); }
  //: Return a reverse iterator pointing to the last element of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rbegin() { return reverse_iterator(end()); }
  //: Return a reverse iterator pointing past the end of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rend() { return reverse_iterator(begin()); }
  //: Return a const reverse iterator pointing to the last element of the vector
  //!wheredef: Reversible Container
  inline const_reverse_iterator rbegin() const {
    return reverse_iterator(end()); }
  //: Return a const reverse iterator pointing past the end of the vector
  //!wheredef: Reversible Container
  inline const_reverse_iterator rend() const{
    return reverse_iterator(begin()); }

  /**@name Element Access Methods */


  //: Return a reference to the element with the ith index
  //!wheredef: Vector
  inline reference operator[](size_type i) MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "linalg_vec::operator[]");
    return (*rep)[i - first];
  }

  inline subrange_type operator()(range r) MTL_THROW_ASSERTION {
    return subrange_type(data() + r.first, r.second - r.first);
  }
  inline subrange_type operator()(size_type s, size_type f)
    MTL_THROW_ASSERTION
  {
    return subrange_type(data() + s, f - s);
  }

  //: Return a const reference to the element with the ith index
  //!wheredef: Vector
  inline const_reference operator[](size_type i) const MTL_THROW_ASSERTION {
    MTL_ASSERT(i < size(), "linalg_vec::operator[]");
    return (*rep)[i - first];
  }


  /**@name Size Methods */

  //: The size of the vector
  //!wheredef: Container
  inline size_type size() const { return rep->size(); }
  //: The number of non-zeroes in the vector
  inline size_type nnz() const { return rep->size(); }
  //: Resize the vector to n
  inline void resize(size_type n) { rep->resize(n); }
  //: Resize the vector to n, and assign x to the new positions
  inline void resize(size_type n, const value_type& x) { rep->resize(n, x); }
  //: Return the total capacity of the vector
  size_type capacity() const { return rep->capacity(); }

  //: Reserve more space in the vector
  void reserve(size_type n) { rep->reserve(n); }

  //: Raw Memory Access
  inline const value_type* data() const { return &(*rep)[0]; }

  //: Raw Memory Access
  inline value_type* data() { return &(*rep)[0]; }

  //: Insert x at the indicated position in the vector
  //!wheredef: Container
  inline iterator
  insert (iterator position, const value_type& x = value_type()) {
    return iterator(rep->insert(position.base(), x), position.index()+1);
    /* JGS, not sure about what to do with the index here */
  }

  inline IndexArrayRef nz_struct() const { return IndexArrayRef(*this); }

  inline self& adjust_index(size_type i) {
    first += i;
    return *this;
  }

protected:

  rep_ptr rep;
  size_type first;
};


//: External 1-D Container
//!category: containers
//!component: type
//
// This is similar to dense1D, except that the memory is provided
// by the user. This allows for interoperability with other array
// packages and even with Fortran.
//
//!definition: linalg_vec.h
//!tparam: T - The element type.
//!tparam: NN - The static size of the Vector, 0 if dynamic size
//!tparam: SizeT - The size type to use - size_t
//!tparam: DiffT - ptrdiff_t
//!models: Vector
//!example: dot_prod.cc, apply_givens.cc, euclid_norm.cc, max_index.cc

template <class T, int NN = 0, class SizeType=unsigned int>
class external_vec {
  typedef external_vec self;
public:
  enum { N = NN };

  typedef external_vec<int> IndexArray; /* JGS */

  /* Type Definitions */

  //: The vector is dense
  typedef dense_tag sparsity;
  //: Scaled type for the vector
  typedef scaled1D< self > scaled_type;

  typedef SizeType size_type;
  typedef int difference_type;

  //: The element type
  typedef T value_type;
  //: The reference to the value type
  typedef T& reference;
  //: The pointer ot the value type
  typedef T* pointer;
  //: The const reference type
  typedef const T& const_reference;
  //: The const pointer to the value type
  typedef const T* const_pointer;

#if defined( _MSVCPP_ )

  typedef dense_iterator<T, 0, 0, size_type> iterator;
  typedef dense_iterator<T, 1, 0, size_type> const_iterator;

#elif defined ( _MSVCPP7_ )
  /// used std::_Ptrit in order to support iterator_traits for
  /// pointers masquerading as iterators as per std::vector and std::basic_string - BEL
  //
  typedef std::_Ptrit<value_type, difference_type, pointer, reference, pointer, reference> ptr_iterator;
  typedef std::_Ptrit<value_type, difference_type, const_pointer, const_reference, pointer, reference> ptr_const_iterator;

  typedef dense_iterator<ptr_iterator,0,size_type> iterator;
  typedef dense_iterator<ptr_const_iterator,0,size_type> const_iterator;

#else

  typedef dense_iterator<T*,0,size_type> iterator;
  typedef dense_iterator<const T*,0,size_type> const_iterator;

#endif
  //: The reverse iterator type
  typedef reverse_iter<iterator> reverse_iterator;
  //: The const reverse iterator type
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  //:
  //!wheredef: Vector
  typedef self IndexArrayRef;

  //: The type for the subrange vector
  //!wheredef: Vector
  typedef self subrange_type;

  typedef std::pair<size_type, size_type> range;

  //: This is a 1D container
  typedef oned_tag dimension;


  /* Constructors */
  //: Default Constructor
  inline external_vec() : rep(0), size_(0), first(0) { }

  //: External Data Contructor
  inline external_vec(T* data)
    : rep(data), size_(N), first(0) { }

  //: Preallocated Memory Constructor with optional non-zero starting index
  inline external_vec(T* data, size_type n, size_type start = 0)
    : rep(data), size_(n), first(start) { }

  //: Copy Constructor
  inline external_vec(const self& x)
    : rep(x.rep), size_(x.size_), first(x.first) { }

  //: Assignment
  inline self& operator=(const self& x) {
    rep = x.rep; size_ = x.size_; first = x.first; return *this;
  }

  //: Destructor
  inline ~external_vec() { }


  /* Access Methods */

  /* Iterator Access Methods */

  //: Return an iterator pointing to the beginning of the vector
  //!wheredef: Container
  inline iterator begin() { return iterator(rep, 0, first); }
  //: Return an iterator pointing past the end of the vector
  //!wheredef: Container
  inline iterator end() { return iterator(rep, size(), first); }
  //: Return a const iterator pointing to the begining of the vector
  //!wheredef: Container
  inline const_iterator begin() const {
    return const_iterator(rep, 0, first);
  }
  //: Return a const iterator pointing past the end of the vector
  //!wheredef: Container
  inline const_iterator end() const{
    return const_iterator(rep, size(), first);
  }
  //: Return a reverse iterator pointing to the last element of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rbegin() {

    return reverse_iterator(end());
  }
  //: Return a reverse iterator pointing past the end of the vector
  //!wheredef: Reversible Container
  inline reverse_iterator rend() { return reverse_iterator(begin()); }
  //: Return a const reverse iterator pointing to the last element of the vector
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

  //: Return a reference to the element with the ith index
  //!wheredef: Vector
  inline reference operator[](size_type i) { return rep[i - first]; }
  //: Return a const reference to the element with the ith index
  //!wheredef: Vector
  inline const_reference operator[](size_type i) const { return rep[i - first]; }
  //: Return a subrange vector with start at s and finish at f
  //!wheredef: Vector
  inline subrange_type operator()(size_type s, size_type f) const {
    return subrange_type(rep + s - first, f - s, 0);
  }

  inline subrange_type operator()(range r) MTL_THROW_ASSERTION {
    return subrange_type(data() + r.first, r.second - r.first, 0);
  }

  /* Size Methods */
  //: The size of the vector
  //!wheredef: Container
  inline size_type size() const { return N ? (size_type)N : size_; }

  //: The number of non-zeroes in the vector
  //!wheredef: Vector
  inline size_type nnz() const { return size(); }

  //: Resize the vector to size n
#if 0
  inline void resize(size_type n) {
    if (rep) delete [] rep;
    size_ = n;
    rep = new T[size_];
  }
#else
  inline void resize(size_type n) { size_ = n; }
  inline void clear() { size_ = 0; }
#endif

  //:  Raw Memory Access
  inline value_type* data() const { return rep; }

  inline self& adjust_index(size_type i) {
    first += i;
    return *this;
  }

  //: Push x onto the end of the vector, increasing the size
  // This function does not allocation memory.
  // Better hope enough memory is already there!
  void push_back(const T& x) {
    rep[size_] = x;
    ++size_;
  }

protected:
  T* rep;
  size_type size_;
  size_type first;
};



//: blah
//!noindex:
template <int N>
struct __make_external {
  template <class T>
  inline external_vec<T,N> operator()(T* x) {
    return external_vec<T,N>(x);
  }
};

/* For converting static arrays into MTL vectors */
#define array_to_vec(x) mtl::__make_external<sizeof(x)/sizeof(*x)>()(x)


template <class Container>
inline linalg_vec<Container>
vec(const Container& x)
{
  return linalg_vec<Container>(x);
}

} /* namespace mtl */

#endif

