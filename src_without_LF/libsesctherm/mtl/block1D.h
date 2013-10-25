#ifndef MTL_BLOCK1D_H
#define MTL_BLOCK1D_H

#include "linalg_vec.h"
#include "fast.h"
#include "dense1D.h"

namespace mtl {

//: Blocked View of a Vector
//
// This presents a vector (must be dense) as if it is a vector of
// subvectors, where each subvector is of equal length (specified
// statically with template arg BN or dynamically in the constructor).
// This could probably be also done with a matrix, setting the ld
// to the block size, but this is less confusing.
//
//!category: containers, adaptors
//!component: type
//!tparam: Vector - the adapted Vector
//!tparam: BN - static blocking size
//!example: blocked_vector.cc
template<class Vector, int BN = 0>
class block1D {
public:
  typedef block1D<Vector,BN> self;
  typedef typename Vector::value_type T;
  typedef external_vec<T,BN> Block;

  typedef Block value_type;
  typedef Block reference;
  typedef const Block const_reference;
  typedef Block* pointer;

  typedef typename Vector::size_type size_type;
  typedef typename Vector::difference_type difference_type;

  enum { N = 0 };

  typedef dense_tag sparsity;

  typedef scaled1D< self > scaled_type;
  typedef dense1D< self > partitioned;
  typedef external_vec<int> IndexArray;
  typedef external_vec<int> IndexArrayRef;
  typedef self subrange_type;
  typedef twod_tag dimension;

  class iterator {
    typedef iterator self;
  public:
    typedef Block reference;
    typedef Block value_type;
    typedef Block* pointer;
    typedef typename Block::size_type size_type;
    typedef typename Block::difference_type difference_type;
    typedef std::random_access_iterator_tag iterator_category;

    inline iterator(T* s, size_type p, size_type bs)
      : start(s), pos(p), bsize(bs) { }

    inline iterator( ) : start(0), pos(0), bs(0) { }

    inline reference operator*() const {
      return Block(start + pos*bsize, bsize);
    }

    inline self& operator++() { ++pos; return *this; }
    inline self& operator+=(size_type n) { pos += n; return *this; }
    inline self operator++(int) { self t = *this; ++(*this); return t; }
    inline self& operator--() { --pos; return *this; }
    inline self& operator-=(size_type n) { pos -= n; return *this; }
    inline self operator--(int) { self t = *this; --(*this); return t; }
    inline bool operator!=(const self& x) const { return pos != x.pos; }
    inline bool operator==(const self& x) const { return pos == x.pos; }
    inline bool operator<(const self& x) const { return pos < x.pos; }
    inline size_type index() const { return pos; }

    T* start;
    size_type pos;
    size_type bsize;
  };

  class const_iterator {
    typedef const_iterator self;
  public:
    typedef Block reference;
    typedef Block value_type;
    typedef Block* pointer;
    typedef typename Block::size_type size_type;
    typedef typename Block::difference_type difference_type;
    typedef std::random_access_iterator_tag iterator_category;

    inline const_iterator(T* s, size_type p, size_type bs)
      : start(s), pos(p), bsize(bs) { }

    inline const_iterator( ) : start(0), pos(0), bsize(0) { }

    inline reference operator*() const {
      return Block(start + pos*bsize, bsize);
    }

    inline self& operator++() { ++pos; return *this; }
    inline self& operator+=(size_type n) { pos += n; return *this; }
    inline self operator++(int) { self t = *this; ++(*this); return t; }
    inline self& operator--() { --pos; return *this; }
    inline self& operator-=(size_type n) { pos -= n; return *this; }
    inline self operator--(int) { self t = *this; --(*this); return t; }
    inline bool operator!=(const self& x) const { return pos != x.pos; }
    inline bool operator==(const self& x) const { return pos == x.pos; }
    inline bool operator<(const self& x) const { return pos < x.pos; }
    inline size_type index() const { return pos; }

    T* start;
    size_type pos;
    size_type bsize;
  };

  typedef reverse_iter<iterator> reverse_iterator;
  typedef reverse_iter<const_iterator> const_reverse_iterator;

  inline block1D( ) : bsize(0) { }

  inline block1D(const Vector& v, size_type block_size = BN)
    : data((T*)v.data()),
      size_(v.size() / block_size),
      bsize(block_size) { }

  inline block1D(const self& x)
    : data(x.data), size_(x.size_), bsize(x.bsize) { }

  inline ~block1D() { }

  inline iterator begin() { return iterator(data, 0, bsize); }
  inline iterator end() { return iterator(data, size_, bsize); }
  inline const_iterator begin() const {
    return const_iterator(data, 0, bsize); }
  inline const_iterator end() const {
    return const_iterator(data, size_, bsize); }

  inline reverse_iterator rbegin() {
    return reverse_iterator(end()); }
  inline reverse_iterator rend() {
    return reverse_iterator(begin()); }
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());  }
  inline const_reverse_iterator rend() const{
    return const_reverse_iterator(begin());  }

  inline reference operator[](int n) {
    return Block(data + n * bsize, bsize);
  }
  //:
  inline const_reference operator[](int n) const {
    return Block(data + n * bsize, bsize);
  }

  inline size_type size() const { return size_; }
protected:

  T* data;
  size_type size_;
  size_type bsize;
};


}

#endif /* MTL_BLOCK1D_H */
