#ifndef MTL_EXTERNAL_VECTOR_H
#define MTL_EXTERNAL_VECTOR_H

#include "mtl_iterator.h"
#include "mtl_exception.h"

namespace mtl {


template <class T, int NN = 0, class SizeT = size_t, class DiffT = ptrdiff_t>
class external_vector {
  typedef external_vector<T, NN> self;
public:

  enum { N = NN }; // Static Size

  // Associated Types
  typedef T* iterator;
  typedef const T* const_iterator;
#if STD_REVERSE_ITER
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#else
  typedef std::reverse_iterator<iterator,T> reverse_iterator;
  typedef std::reverse_iterator<const_iterator,T> const_reverse_iterator;
#endif
  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef SizeT size_type;
  typedef DiffT difference_type;

private:

  // No Default Constructor
  inline external_vector() : data_(0), size_(0) { }

  // No Copy Constructor
  inline external_vector(const self& x) : data_(0), size_(0) { }

public:

  // Preallocated Memory Constructor with optional non-zero starting index
  inline external_vector(value_type* data, size_type n = N)
    : data_(data), size_(n) { }

  inline ~external_vector() { }

  // Assignment Operator
  inline self& operator=(const self& x) MTL_THROW_ASSERTION {
    MTL_ASSERT(x.size() == this->size(), "static_vector::operator=(x)");
    std::copy(x.begin(), x.end(), this->begin());
    return this;
  }

  // Iterator Access Methods
  inline iterator begin() { return iterator(data_); }
  inline iterator end() { return iterator(data_ + size()); }
  inline const_iterator begin() const {
    return const_iterator(data_); }
  inline const_iterator end() const {
    return const_iterator(data_ + size()); }

  inline reverse_iterator rbegin() { return reverse_iterator(end());  }
  inline reverse_iterator rend() { return reverse_iterator(begin()); }
  inline const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end()); }
  inline const_reverse_iterator rend() const{
    return const_reverse_iterator(begin()); }

  // Element Access Methods
  inline reference operator[](int n) { return data_[n]; }
  inline const_reference operator[](int n) const {
    return data_[n]; }

  // Size Methods
  inline size_type size() const { return N ? (size_type)N : size_; }
  inline void set_size(size_type n) { size_ = n; }

  // Memory Access
  inline pointer data() const { return data_; }
  inline void set_data(pointer d) { data_ = d; }

protected:

  pointer data_;
  size_type size_;

};


} /* namespace mtl */

#endif
