#ifndef _MTL_DIAGONAL_INDEXER_H_
#define _MTL_DIAGONAL_INDEXER_H_

#include "orien.h"
#include "matrix_traits.h"

namespace mtl {

template <class size_type, class Orien>
class diagonal_indexer {
public:

  enum { M = 0, N = 0 } ;

  typedef dimension<size_type> dim_type;
  typedef dimension<int> band_type;

  dim_type dim;
  band_type bandwidth;

  typedef typename Orien::orientation orientation;
  typedef diagonal_tag shape;

  typedef diagonal_indexer<size_type,
                         typename Orien::transpose_type> transpose_type;
  typedef diagonal_indexer<size_type,
                         typename Orien::transpose_type> strided_type;

  typedef Orien orienter;

  class OneDIndexer {
  public:
    inline OneDIndexer() { }
    inline OneDIndexer(size_type majornum, dim_type s)
      : major_num(majornum), starts(s) { }
    inline OneDIndexer(const OneDIndexer& x)
      : major_num(x.major_num), starts(x.starts) { }

    template <class OneDIterator> inline
    size_type row(OneDIterator i) const { return Orien::row(coords(i)); }

    template <class OneDIterator> inline
    size_type column(OneDIterator i) const{ return Orien::column(coords(i)); }

    template <class OneDIterator> inline
    size_type minor(OneDIterator i) const { return coords(i).second(); }

    template <class OneDIterator> inline
    OneDIterator begin(OneDIterator i) const { return i; }
  protected:
    template <class OneDIterator> inline
    dim_type coords(OneDIterator i) const {
      return dim_type(i.index() + starts.first(),
		      i.index() + starts.second());
    }
    size_type major_num;
    dim_type starts;
  };
  inline diagonal_indexer() { }
  inline diagonal_indexer(dim_type d, band_type band)
    : dim(d), bandwidth(band) { }
  inline diagonal_indexer(const diagonal_indexer& x)
    : dim(x.dim), bandwidth(x.bandwidth) { }
  inline diagonal_indexer(const strided_type& x, strideable)
    : dim(x.dim.transpose()), bandwidth(x.bandwidth) { }
  inline diagonal_indexer(const transpose_type& x, not_strideable)
    : dim(x.dim), bandwidth(x.bandwidth) { }

  template <class TwoDIterator> inline
  OneDIndexer deref(TwoDIterator iter) const {
    int row = MTL_MAX(0, int(iter.index()) - bandwidth.second());
    int col = MTL_MAX(0, bandwidth.second() - int(iter.index()));
    return OneDIndexer(iter.index(), dim_type(row, col));
  }

  OneDIndexer deref(size_type i) const {
    int row = MTL_MAX(0, int(i) - bandwidth.second());
    int col = MTL_MAX(0, bandwidth.second() - int(i));
    return OneDIndexer(i, dim_type(row, col));
  }

  inline dim_type at(dim_type p) const {
    dim_type m = Orien::map(p);
    int i = m.first(); int j = m.second();
    int ii = i - j + bandwidth.second();
    if (i >= j)
      return dim_type(ii, j);
    else
      return dim_type(ii, i);
  }

  //: Calculate the dimension that the TwoD container should have
  //  m' = num diagonals = sub + super + 1
  //  n' = min(m, n + sub)
  inline static dim_type twod_dim(dim_type dim, band_type bw) {
    return dim_type(bw.first() + bw.second() + 1,
		    MTL_MIN(dim.first(), dim.second() + bw.first()));
  }

  //: Calculate the bandwith that the TwoD container should have
    // sub' = super
    // super' = min(m - 1, n - super - 1)
  inline static band_type twod_band(dim_type dim, band_type bw) {
    return band_type(bw.second(), MTL_MIN(dim.first()- 1,
				      dim.second() - bw.second() - 1));
  }

  inline size_type nrows() const{ return Orien::row(dim); }
  inline size_type ncols() const { return Orien::column(dim); }

  inline int sub() const { return Orien::row(bandwidth); }
  inline int super() const { return Orien::column(bandwidth); }

};

//: blah
//!noindex:
template <class Orien, int MM, int NN, class size_type>
struct gen_diagonal_indexer {
  typedef dimension<char,0,0> twod_dim_type; // no static twod's with diagonal
  typedef gen_diagonal_indexer< typename Orien::transpose_type, NN, MM, size_type>
    transpose_type;
  typedef gen_diagonal_indexer< Orien, -1, -1,char> strided_type; // bogus Orien

  typedef diagonal_indexer<size_type, Orien> type;
  typedef Orien orienter;
  template <class ST>
  struct bind {
    typedef gen_diagonal_indexer<Orien, MM, NN, ST> other;
  };
};

} /* namespace mtl */

#endif /* _MTL_DIAGONAL_INDEXER_H_ */
