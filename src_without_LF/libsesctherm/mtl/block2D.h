#ifndef MTL_BLOCK2D_H
#define MTL_BLOCK2D_H

#include "dimension.h"
#include "meta_if.h"

namespace mtl {

template <class Block, class OffsetGen, int M, int N>
struct gen_block2D;

//: Block View TwoD Storage
//
// For use in blocked algorithms with rectangle dense matrices. The
// blocks all have the same size (vs. variable sizes as in a
// partitioned matrix). The matrix objects for each block are not
// stored, they are generated on the fly as they are requested, and
// they are lightweight object on the stack so no overhead is
// incurred.  <p> The blocking size must divide evenly into the
// original matrix size. One good way to ensure this is to partition
// the original matrix into a main region that divides evenly and into
// the blocks, and 3 others edge regions that do not get blocked.  <p>
// Use the block_view type constructor and the blocked function to
// create matrices of this type.
//
//!tparam: Block - The submatrix block, a dense external matrix.
//!tparam: OffsetGen - The Offset generator.
//!category: containers, adaptors
//!component: type
//!example: blocked_matrix.cc
//!definition: block2D.h
//
template <class Block, class OffsetGen>
class block2D {
public:
  typedef block2D<Block, OffsetGen> self;
  typedef typename Block::value_type T;

  enum { M = 0, N = 0, BM = Block::M, BN = Block::N };

  //: The 1D container type
  typedef typename Block::size_type size_type;
  //: The type for differences between iterators
  typedef typename Block::difference_type difference_type;
public:
  typedef dimension<size_type, BM, BN> block_dim_type;
  typedef typename OffsetGen:: MTL_TEMPLATE bind<size_type>::type Offset;
  typedef typename Offset::dim_type dim_type;
public:
  //: This is a dense 2D container
  typedef dense_tag sparsity;
  //: This has external storage
  typedef external_tag storage_loc;
  //: This is strideable
  typedef strideable strideability;

#if 0
  /* bogus ? */
  template <class SubMatrix>
  struct partitioned {
    typedef block2D<SubMatrix, OffsetGen> type;
    typedef gen_block2D<SubMatrix, OffsetGen> generator;
  };
#endif

  class block_vector {
  public:
    typedef Block reference;
    typedef const Block const_reference;
    typedef Block value_type;
    typedef Block* pointer;
    typedef typename Block::size_type size_type;
    typedef typename Block::difference_type difference_type;

    enum { M = 0, N = 0 };

    typedef block_vector subrange_type;
    typedef dense_tag sparsity;
    typedef block_vector IndexArray; /* bogus */
    typedef block_vector IndexArrayRef; /* bogus */

    typedef oned_tag dimension; /* bogus */

    template <int isConst>
    class __iterator {
      typedef __iterator self;
    public:
      typedef typename IF<isConst, block_vector::const_reference,
                 block_vector::reference>::RET reference;
      typedef block_vector::value_type value_type;
      typedef block_vector::pointer pointer;
      typedef block_vector::size_type size_type;
      typedef block_vector::difference_type difference_type;
      typedef std::random_access_iterator_tag iterator_category;

      inline __iterator(T* s, size_type p, size_type str, size_type ld_,
		      block_dim_type bd)
	: start(s), pos(p), stride(str), ld(ld_), bdim(bd) { }

      inline __iterator(const self& x)
	: start(x.start), pos(x.pos), stride(x.stride),
	  ld(x.ld), bdim(x.bdim) { }

      inline self& operator=(const self& x) {
	start = x.start; pos = x.pos; stride = x.stride;
	ld = x.ld; bdim = x.bdim;
	return *this;
      }

      inline __iterator( ) : start(0), pos(0), stride(0), ld(0) { }

      inline reference operator*() const {
	typedef typename Block::orien orienter;
	return Block(start + pos * stride,
		     orienter::map(bdim).first(),
		     orienter::map(bdim).second(),
		     ld);
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
      size_type stride;
      size_type ld;
      block_dim_type bdim;
    };

    typedef __iterator<0> iterator;
    typedef __iterator<1> const_iterator;

    inline block_vector(T* s, size_type l, block_dim_type bd,
		      size_type str, size_type ld_)
    : start(s), len(l), bdim(bd), stride(str * bd.second()), ld(ld_) { }

    inline block_vector(const block_vector& x)
      : start(x.start), len(x.len), bdim(x.bdim),
	stride(x.stride), ld(x.ld) { }

    inline block_vector& operator=(const block_vector& x) {
      start = x.start; len = x.len;
      bdim = x.bdim; stride = x.stride;
      ld = x.ld;
      return *this;
    }

    inline block_vector()
      : start(0), len(0), stride(0) { }

    inline ~block_vector() { }

    inline reference operator[](size_type n) {
      typedef typename Block::orien orienter;
      return Block(start + n * stride,
		   orienter::map(bdim).first(),
		   orienter::map(bdim).second(),
		   ld);
    }

    inline const_reference operator[](size_type n) const {
      typedef typename Block::orien orienter;
      return Block(start + n * stride,
		   orienter::map(bdim).first(),
		   orienter::map(bdim).second(),
		   ld);
    }

    inline iterator begin() {
      return iterator(start, 0, stride, ld, bdim);
    }
    inline iterator end() {
      return iterator(start, len, stride, ld, bdim);
    }

    inline const_iterator begin() const {
      return const_iterator(start, 0, stride, ld, bdim);
    }
    inline const_iterator end() const {
      return const_iterator(start, len, stride, ld, bdim);
    }

    T* start;
    size_type len;
    block_dim_type bdim;
    size_type stride;
    size_type ld;
  };
  //: The 1D container type
  typedef block_vector value_type;
  //: A reference to the value type
  typedef block_vector reference;
  //: The type for pointers to the value type
  typedef block_vector* pointer;

  //: The iterator type
  template <int isConst>
  class __iterator {
    typedef __iterator self;
  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef block_vector value_type;
    typedef typename IF<isConst, const block_vector, block_vector>::RET reference;
    typedef value_type* pointer;
    typedef typename Block::size_type size_type;
    typedef typename Block::difference_type difference_type;

    inline __iterator(T* s, size_type ld_, size_type p,
		    Offset os, block_dim_type bd)
      : start(s), ld(ld_), pos(p), offset(os), bdim(bd) { }

    inline __iterator() : start(0), ld(0), pos(0) { }

    inline __iterator(const self& x)
      : start(x.start), ld(x.ld), pos(x.pos),
	offset(x.offset), bdim(x.bdim) { }

    inline self& operator=(const self& x) {
      start = x.start; ld = x.ld; pos = x.pos;
      offset = x.offset; bdim = x.bdim;
      return *this;
    }

    inline reference operator*() const {
      return block_vector(start + offset.oned_offset(pos * bdim.first()),
			  //offset.oned_length(pos * bdim.first()),
			  // JGS, problem
			  offset.minor(),
			  bdim,
			  offset.stride(),
			  ld);
    }
    inline reference operator[](size_type n) const {
      return block_vector((T*)start +
			  offset.oned_offset((pos + n) * bdim.first()),
			  // offset.oned_length((pos + n) * bdim.first()),
			  // JGS, problem
			  offset.minor(),
			  bdim,
			  offset.stride(),
			  ld);
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

  protected:
    T* start;
    size_type pos;
    size_type ld;
    Offset offset;
    block_dim_type bdim;
  };

  typedef __iterator<0> iterator;
  typedef __iterator<1> const_iterator;

  typedef dimension<size_type> dyn_dim;
  typedef block_dim_type bdt;

  //: Constructor from underlying 2D container
  template <class TwoD>
  inline block2D(TwoD& x, dyn_dim b)
    : data_((T*)x.data()),
      ld_(x.ld()), block_dim(b), offset(x.major() / bdt(b).first(),
					x.minor() / bdt(b).second(),
					x.ld()) { }
  //: Copy Constructor
  inline block2D(const block2D& x)
    : data_(x.data_), ld_(x.ld_), block_dim(x.block_dim), offset(x.offset) { }

  inline const block2D& operator=(const block2D& x) {
    data_ = x.data_; ld_ = x.ld_;
    block_dim = x.block_dim; offset = x.offset;
    return *this;
  }
  //: Default Constructor
  inline block2D() : data_(0), ld_(0) { }
  //: Destructor
  inline ~block2D() { }

  //: Return an iterator pointing to the first 1D container
  inline iterator begin() {
    return iterator(data_, ld_, 0, offset, block_dim);
  }
  //: Return an iterator pointing past the end of the 2D container
  inline iterator end() {
    return iterator(data_, ld_, offset.major(), offset, block_dim);
  }

  //: Return a const iterator pointing to the first 1D container
  inline const_iterator begin() const {
    return const_iterator(data_, ld_, 0, offset, block_dim);
  }
  //: Return a const iterator pointing past the end of the 2D container
  inline const_iterator end() const {
    return const_iterator(data_, ld_, offset.major(), offset, block_dim);
  }

  //: Return a reference to the ith 1D container
  inline block_vector operator[](size_type i) {
    return block_vector(s, offset.oned_length(i * block_dim.first()),
			block_dim, offset.stride(), ld_);
  }

  //: Return a reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline Block operator()(size_type i, size_type j) {
    typedef typename Block::orien orienter;
    return Block(data_ + offset.elt(i * block_dim.first(),
				    j * block_dim.second()),
		 orienter::map(block_dim).first(),
		 orienter::map(block_dim).second(),
		 ld_);
  }

  //: Return a const reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline const Block operator()(size_type i, size_type j) const {
    typedef typename Block::orien orienter;
    return Block(data_ + offset.elt(i * block_dim.first(),
				    j * block_dim.second()),
		 orienter::map(block_dim).first(),
		 orienter::map(block_dim).second(),
		 ld_);
  }
  //: The leading dimension
  inline size_type ld() const { return ld_; }



protected:
  T* data_;
  size_type ld_;
  block_dim_type block_dim;
  Offset offset;
};

template <class T, class OffsetGen, int M, int N>
struct gen_external2D;

//: blah
//!noindex:
template <class Block, class OffsetGen, int M, int N>
struct gen_block2D {

  typedef gen_block2D<Block,
           typename OffsetGen::transpose_type, M, N> transpose_type;
  typedef gen_external2D<Block, OffsetGen, M, N> submatrix_type;
  typedef gen_block2D<Block, typename OffsetGen::banded_view_type, M, N>
           banded_view_type;

  typedef block2D<Block, OffsetGen> type;

};


} /* namespace mtl */

#endif /* MTL_BLOCK2D_H */
