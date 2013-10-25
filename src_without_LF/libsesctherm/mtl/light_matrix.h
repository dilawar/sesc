#ifndef MTL_LIGHT_MATRIX_H
#define MTL_LIGHT_MATRIX_H

#include "matrix_traits.h"
#include "dimension.h"
#include "meta_if.h"
#include "meta_equal.h"

namespace mtl {


template <int Orien>
struct TRANS {
  enum { RET = 0 };
};

template<>
struct TRANS<ROW_MAJOR> {
  enum { RET = COL_MAJOR };
};

template<>
struct TRANS<COL_MAJOR> {
  enum { RET = ROW_MAJOR };
};


template <class T, class SizeType, int Orien, int Strided>
class light_matrix {
public:
  typedef light_matrix self;
  typedef light_matrix light_matrix_t; // VC++ workaround
  typedef T* DataPtr;

  typedef rectangle_tag shape;
  typedef typename IF< EQUAL<Orien,ROW_MAJOR>::RET,
              row_tag, column_tag>::RET orientation; // mostly wrong

  typedef typename IF< EQUAL<Orien,ROW_MAJOR>::RET,
              row_orien, column_orien>::RET orien;

  typedef light_matrix<T, SizeType, TRANS<Orien>::RET, Strided> transpose_type;
  typedef light_matrix<T, SizeType, Orien, !Strided> strided_type;
  typedef light_matrix<T, SizeType, Orien, Strided> scaled_type;// wrong

  typedef light_matrix<T, SizeType, Orien, Strided> submatrix_type;

  typedef int DiffType;

  //: The size type
  typedef SizeType size_type;
  //: The type for differences between iterators
  typedef DiffType difference_type;

  typedef T value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef value_type* pointer;

  enum { M = 0, N = 0 };

protected:

  static inline size_type& twod_pos(size_type& i, size_type& j) {
    if (Orien == ROW_MAJOR)
	return i;
    else
	return j;
  }

  static inline const size_type& twod_pos(const size_type& i,
					  const size_type& j) {
    if (Orien == ROW_MAJOR)
	return i;
    else
	return j;
  }

  static inline size_type& oned_pos(size_type& i, size_type& j) {
    if (Orien == ROW_MAJOR)
	return j;
    else
	return i;
  }

  static inline const size_type& oned_pos(const size_type& i,
					  const size_type& j) {
    if (Orien == ROW_MAJOR)
	return j;
    else
	return i;
  }

  // idea: completely separate stride/offset/positioning from indexing
  //  but encapsulate both somehow

public:

  //: This is a dense 2D container
  typedef dense_tag sparsity;
  //: This has external storage
  typedef external_tag storage_loc;
  //: This is strideable
  typedef strideable strideability;

  class oned {
  public:
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef T* pointer;
    typedef SizeType size_type;
    typedef int difference_type;

    enum { M = 0, N = 0 };

    typedef oned subrange_type;
    typedef dense_tag sparsity;
    typedef oned IndexArray; /* bogus */
    typedef oned IndexArrayRef; /* bogus */

    typedef oned_tag dimension; /* bogus */

    template <int isConst>
    class __iterator {
      typedef __iterator self;
    public:
      typedef typename oned::value_type value_type;
      typedef typename oned::pointer    pointer;
      typedef typename oned::size_type size_type;
      typedef typename oned::difference_type difference_type;

      typedef typename IF<isConst, typename oned::const_reference,
	                           typename oned::reference>::RET reference;

      typedef std::random_access_iterator_tag iterator_category;

      inline __iterator(DataPtr d,
			size_type ii, size_type jj,
			size_type os, size_type s)
	: data(d), i(ii), j(jj), offset(os), stride(s) { }

      inline __iterator(const self& x)
	: data(x.data), i(x.i), j(x.j), offset(x.offset), stride(x.stride) { }

      inline self& operator=(const self& x) {
	data = x.data; i = x.i; j = x.j; offset = x.offset; stride = x.stride;
	return *this;
      }

      inline __iterator() : data(0), i(0), j(0), offset(0), stride(0) { }

      inline reference operator*() const { return data[offset]; }
      inline self& operator++() { ++pos(); offset += stride; return *this; }
      inline self& operator+=(size_type n) {
	pos() += n; offset += stride*n; return *this;
      }
      inline self operator++(int) { self t = *this; ++(*this); return t; }
      inline self& operator--() { --pos(); offset -= stride; return *this; }
      inline self& operator-=(size_type n) {
	pos() -= n; offset -= stride*n; return *this; }
      inline self operator--(int) { self t = *this; --(*this); return t; }
      inline bool operator!=(const self& x) const { return pos() != x.pos(); }
      inline bool operator==(const self& x) const { return pos() == x.pos(); }
      inline bool operator<(const self& x) const { return pos() < x.pos(); }
      inline size_type index() const { return pos(); }

      inline size_type& pos() { return oned_pos(i,j); }
      inline const size_type& pos() const { return oned_pos(i,j); }

      inline size_type row() const { return i; }
      inline size_type column() const { return j; }
    protected:
      DataPtr data;
      size_type i, j;
      size_type offset;
      size_type stride;
    };

    typedef __iterator<0> iterator;
    typedef __iterator<1> const_iterator;

    inline oned(DataPtr d, size_type ii, size_type jj,
		size_type ie, size_type je,
		size_type os, size_type ld)
      : data(d), i(ii), j(jj), iend(ie), jend(je),
	offset(os), ldim(ld) { }

    inline oned(const oned& x)
      : data(x.data), i(x.i), j(x.j),
	iend(x.iend), jend(x.jend),
	offset(x.offset), ldim(x.ldim) { }

    inline oned& operator=(const oned& x) {
      data = x.data; i = x.i; j = x.j;
      iend = x.iend; jend = x.jend;
      offset = x.offset; ldim = x.ldim;
      return *this;
    }
    inline oned()
      : data(0), i(0), j(0), iend(0), jend(0), offset(0), ldim(0) { }

    inline ~oned() { }

    inline reference operator[](size_type n) {
      return data[ Strided ? offset + n * ldim : offset + n];
    }

    inline const_reference operator[](size_type n) const {
      return data[ Strided ? offset + n * ldim : offset + n];
    }

    inline iterator begin() {
      return iterator(data, i, j, offset, Strided ? ldim : 1);
    }
    inline iterator end() {
      size_type iiend, jjend;
      if (Orien == ROW_MAJOR) { iiend = i; jjend = jend; }
      else { iiend = iend; jjend = j; }

      return iterator(data, iiend, jjend, offset, Strided ? ldim: 1);
    }

    inline const_iterator begin() const {
      return const_iterator(data, i, j, offset, Strided ? ldim : 1);
    }
    inline const_iterator end() const {
      size_type iiend, jjend;
      if (Orien == ROW_MAJOR) { iiend = i; jjend = jend; }
      else { iiend = iend; jjend = j; }

      return const_iterator(data, iiend, jjend, offset, Strided ? ldim : 1);
    }

  protected:
    DataPtr data;
    size_type i, j;
    size_type iend, jend;
    size_type offset;
    size_type ldim;
  };

  typedef oned OneD;
  typedef OneD OneDRef;
  typedef OneD Row;
  typedef OneD RowRef;
  typedef OneD Column;
  typedef OneD ColumnRef;

  //: The iterator type
  template <int Const>
  class __iterator {
    typedef __iterator self;
  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef oned value_type;
    typedef value_type* pointer;

#if defined(_MSVCPP_)
    typedef typename light_matrix_t::size_type size_type;
    typedef typename light_matrix_t::difference_type difference_type;
#else
    typedef SizeType size_type;
    typedef DiffType difference_type;
#endif

    typedef typename IF<Const, const oned, oned>::RET reference;

    inline __iterator(DataPtr d, size_type ii, size_type jj,
		      size_type ie, size_type je, size_type ld)
      : data(d), i(ii), j(jj), iend(ie), jend(je), offset(0), ldim(ld) {
	if (Strided) stride = 1; else stride = ldim;
    }

    inline __iterator() : data(0), i(0), j(0), iend(0), jend(0),
	offset(0), stride(0), ldim(0) { }

    inline __iterator(const self& x)
      : data(x.data), i(x.i), j(x.j),
	iend(x.iend), jend(x.jend), offset(x.offset),
	stride(x.stride), ldim(x.ldim) { }

    inline self& operator=(const self& x) {
      data = x.data; i = x.i; j = x.j;
      iend = x.iend; jend = x.jend;
      offset = x.offset; stride = x.stride; ldim = x.ldim;
      return *this;
    }
    inline reference operator*() const {
      return oned(data, i, j, iend, jend, offset, ldim);
    }

    inline self& operator++() { ++pos(); offset += stride; return *this; }
    inline self& operator+=(size_type n) {
      pos() += n; offset += stride*n; return *this; }
    inline self operator++(int) { self t = *this; ++(*this); return t; }
    inline self& operator--() { --pos(); offset -= stride; return *this; }
    inline self& operator-=(size_type n) {
      pos() -= n; offset -= stride*n; return *this; }
    inline self operator--(int) { self t = *this; --(*this); return t; }
    inline bool operator!=(const self& x) const { return pos() != x.pos(); }
    inline bool operator==(const self& x) const { return pos() == x.pos(); }
    inline bool operator<(const self& x) const { return pos() < x.pos(); }
    inline size_type index() const { return pos(); }

    inline size_type& pos() { return twod_pos(i,j); }
    inline const size_type& pos() const { return twod_pos(i,j); }

    inline size_type row() const { return i; }
    inline size_type column() const { return j; }

  protected:
    DataPtr data;
    size_type i, j;
    size_type iend, jend;
    size_type offset;
    size_type stride;
    size_type ldim;
  };

  typedef __iterator<0> iterator;
  typedef __iterator<1> const_iterator;

  //: Standard Constructor
  inline light_matrix(DataPtr d, size_type m, size_type n, size_type ld)
    : data_(d), nrows_(m), ncols_(n), ldim(ld) { }

  inline light_matrix(DataPtr d, size_type m, size_type n)
    : data_(d), nrows_(m), ncols_(n), ldim(Orien == ROW_MAJOR ? n : m) { }

  //: Copy Constructor
  inline light_matrix(const light_matrix& x)
    : data_(x.data_), nrows_(x.nrows_), ncols_(x.ncols_), ldim(x.ldim) { }

  //: Assignment Operator
  inline const light_matrix& operator=(const light_matrix& x) {
    data_ = x.data_; nrows_ = x.nrows_; ncols_ = x.ncols_; ldim = x.ldim;
    return *this;
  }
  //: Default Constructor
  inline light_matrix() : data_(0), nrows_(0), ncols_(0), ldim(0) { }

  inline light_matrix(const strided_type& x, do_strided s)
    : data_(x.data_), nrows_(x.nrows_), ncols_(x.ncols_), ldim(x.ldim) { }

  template <class StridedType>
  inline light_matrix(const StridedType& x, do_strided s)
    : data_(x.data_), nrows_(x.nrows_), ncols_(x.ncols_), ldim(x.ldim) { }

  //: Destructor
  inline ~light_matrix() { }

  //: Return an iterator pointing to the first 1D container
  inline iterator begin() {
    return iterator(data_, 0, 0, nrows_, ncols_, ldim);
  }
  //: Return an iterator pointing past the end of the 2D container
  inline iterator end() {
    return iterator(data_, nrows_, ncols_, nrows_, ncols_, ldim);
  }

  //: Return a const iterator pointing to the first 1D container
  inline const_iterator begin() const {
    return const_iterator(data_, 0, 0, nrows_, ncols_, ldim);
  }
  //: Return a const iterator pointing past the end of the 2D container
  inline const_iterator end() const {
    return const_iterator(data_, nrows_, ncols_, nrows_, ncols_, ldim);
  }

  //: Return a reference to the ith 1D container
  inline oned operator[](size_type n) {
    if (Orien == ROW_MAJOR)
      return oned(data_, n, 0, nrows_, ncols_, Strided ? n : ldim * n, ldim);
    else
      return oned(data_, 0, n, nrows_, ncols_, Strided ? n : ldim * n, ldim);
  }

  inline const oned operator[](size_type n) const {
    if (Orien == ROW_MAJOR)
      return oned(data_, n, 0, nrows_, ncols_, Strided ? n : ldim * n, ldim);
    else
      return oned(data_, 0, n, nrows_, ncols_, Strided ? n : ldim * n, ldim);
  }

  //: Return a reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline reference operator()(size_type i, size_type j) {
    return Orien == ROW_MAJOR ? operator[](i)[j] : operator[](j)[i];
  }

  //: Return a const reference to the (i,j) element, where (i,j) is in the 2D coordinate system
  inline const_reference operator()(size_type i, size_type j) const {
    return Orien == ROW_MAJOR ? operator[](i)[j] : operator[](j)[i];
  }

  inline size_type nrows() const { return nrows_; }
  inline size_type ncols() const { return ncols_; }


  inline submatrix_type sub_matrix(size_type i, size_type iend,
				   size_type j, size_type jend) const
  {
    if (Strided)
      return submatrix_type(data_ + oned_pos(i,j) * ldim + twod_pos(iend,jend),
			    iend - i, jend - j, ldim);
    else
      return submatrix_type(data_ + twod_pos(i,j) * ldim + oned_pos(iend,jend),
			    iend - i, jend - j, ldim);
  }


  DataPtr data_;
  size_type nrows_, ncols_;
  size_type ldim;
};


} /* namespace mtl */

#endif /* MTL_LIGHT_MATRIX_H */
