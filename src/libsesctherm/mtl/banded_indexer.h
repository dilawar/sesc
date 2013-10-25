#ifndef _MTL_BANDED_INDEXER_
#define _MTL_BANDED_INDEXER_

#include "orien.h" // for row/column_orien?
#include "matrix_traits.h"

namespace mtl {


template <class size_type, class Orien>
class banded_indexer {
public:

  enum { M = 0, N = 0 } ;

  typedef dimension<size_type> dim_type;
  typedef dimension<int> band_type; // need negatives for unit upper

  dim_type dim;
  band_type bandwidth;

  typedef typename Orien::orientation orientation;
  typedef banded_tag shape;
  typedef banded_indexer<size_type,
                       typename Orien::transpose_type> transpose_type;
  typedef banded_indexer<size_type,
                       typename Orien::transpose_type> strided_type;
  //JGS VC++ doesn't like  friend class transpose_type;
  typedef Orien orienter;

  class OneDIndexer {
  public:
    inline OneDIndexer() { }
    inline OneDIndexer(size_type majornum, size_type s)
      : major_num(majornum), start(s) { }
    inline OneDIndexer(const OneDIndexer& x)
      : major_num(x.major_num), start(x.start) { }

    template <class OneDIterator> inline
    size_type row(OneDIterator i) const { return Orien::row(coords(i)); }

    template <class OneDIterator> inline
    size_type column(OneDIterator i) const { return Orien::column(coords(i)); }

    template <class OneDIterator> inline
    size_type minor(OneDIterator i) const { return coords(i).second(); }

    template <class OneDIterator> inline
    OneDIterator begin(OneDIterator i) const { return i; }

    inline size_type at(size_type i) const { return i - start; }
  protected:
    template <class OneDIterator> inline
    dim_type coords(OneDIterator i) const {
      return dim_type(major_num, i.index() + start);
    }
    size_type major_num;
    size_type start;
  };
  inline banded_indexer() { }
  inline banded_indexer(dim_type d, band_type band)
    : dim(d), bandwidth(band) { }
  inline banded_indexer(const banded_indexer& x)
    : dim(x.dim), bandwidth(x.bandwidth) { }
  inline banded_indexer(const strided_type& x, strideable)
    : dim(x.dim.transpose()), bandwidth(x.bandwidth) { }
  inline banded_indexer(const transpose_type& x, not_strideable)
    : dim(x.dim), bandwidth(x.bandwidth) { }

  template <class TwoDIterator> inline
  OneDIndexer deref(TwoDIterator iter) const {
    int i = iter.index();
    size_type start = MTL_MAX(i - int(bandwidth.first()), 0);
    return OneDIndexer(i, start);
  }
  inline OneDIndexer deref(size_type i) const {
    size_type start = MTL_MAX(int(i) - int(bandwidth.first()), 0);
    return OneDIndexer(i, start);
  }
  inline dim_type at(dim_type p) const {
    dim_type m = Orien::map(p);
    return dim_type(m.first(), m.second()
		    - MTL_MAX(int(m.first()) - int(bandwidth.first()), 0));
  }

  inline static dim_type twod_dim(dim_type d, band_type ) { return d; }

  inline static band_type twod_band(dim_type , band_type bw) { return bw; }

  inline size_type nrows() const { return Orien::row(dim); }
  inline size_type ncols() const { return Orien::column(dim); }

  inline int sub() const { return Orien::row(bandwidth); }
  inline int super() const { return Orien::column(bandwidth); }

};

//: blah
//!noindex:
template <class Orien, int MM, int NN, class size_type>
struct gen_banded_indexer {
  typedef dimension<char,0,0> twod_dim_type; // no static twod's with banded
  typedef gen_banded_indexer< typename Orien::transpose_type, MM, NN, size_type>
    transpose_type;
  typedef gen_banded_indexer< Orien, NN, MM, size_type> strided_type; // bogus Orien

  typedef banded_indexer<size_type, Orien> type;
  template <class ST>
  struct bind {
    typedef gen_banded_indexer<Orien, MM, NN, ST> other;
  };
  typedef Orien orienter;
};


} /* namespace mtl */


#endif
