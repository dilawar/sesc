#ifndef MTL_PARTITION_H
#define MTL_PARTITION_H

namespace mtl {

  //: Partition matrix A into matrix P
  //
  //  The prows array specifies on which rows to split matrix A.
  //  The pcols array specifies on which columns to split matrix A.
  //  The resulting submatices are places into matrix P.
  //  The size of matrix P should be (prows.size() + 1, pcols.size() + 1).
  //
  //!category: algorithms
  //!component: function
  //!typereqs:  The element type for matrix P should be the same as the submatrix_type of matrix A. 
  //!complexity: O(P.nrows() * P.ncols())
  //!example: partition.cc
  template <class Sequence1, class Sequence2, class Matrix, class MatrixP>
  inline void
  partition(const Sequence1& prows, 
            const Sequence2& pcols, 
            const Matrix& A, MatrixP& P) MTL_THROW_ASSERTION
  {
    typedef typename Matrix::size_type size_type;
    typedef typename Matrix::dim_type dim_type;
    
    MTL_ASSERT(P.nrows() == prows.size() + 1, "partition()");
    MTL_ASSERT(P.ncols() == pcols.size() + 1, "partition()");
    
    typename Sequence1::const_iterator mi;
    typename Sequence2::const_iterator ni;
    size_type i, j, mprev, nprev;
    
    i = 0;
    mi = prows.begin();
    mprev = 0;
    
    j = 0;
    ni = pcols.begin();
    nprev = 0;
    P(i,j) = A.sub_matrix(mprev, *mi, nprev, *ni);
    nprev = *ni;
    for (++ni, ++j; ni != pcols.end(); ++ni, ++j) {
      P(i,j) = A.sub_matrix(mprev, *mi, nprev, *ni);
      nprev = *ni;
    }
    P(i,j) = A.sub_matrix(mprev, *mi, nprev, A.ncols());
    
    mprev = *mi;
    
    for (++mi, ++i; mi != prows.end(); ++mi, ++i) {
      j = 0;
      ni = pcols.begin();
      nprev = 0;
      P(i,j) = A.sub_matrix(mprev, *mi, nprev, *ni);
      nprev = *ni;
      for (++ni, ++j; ni != pcols.end(); ++ni, ++j) {
        P(i,j) = A.sub_matrix(mprev, *mi, nprev, *ni);
        nprev = *ni;
      }
      P(i,j) = A.sub_matrix(mprev, *mi, nprev, A.ncols());
      
      mprev = *mi;
    }
    
    j = 0;
    ni = pcols.begin();
    nprev = 0;
    P(i,j) = A.sub_matrix(mprev, A.nrows(), nprev, *ni);
    nprev = *ni;
    for (++ni, ++j; ni != pcols.end(); ++ni, ++j) {
      P(i,j) = A.sub_matrix(mprev, A.nrows(), nprev, *ni);
      nprev = *ni;
    }
    P(i,j) = A.sub_matrix(mprev, A.nrows(), nprev, A.ncols());
  }

  //: Divide a matrix into 4 submatrices
  //  
  template <class Matrix, class MatrixP>
  inline void
  subdivide(typename Matrix::size_type split_row,
	    typename Matrix::size_type split_col,
	    const Matrix& A, MatrixP& P)
  {
    typedef typename Matrix::size_type size_type;
    size_type rsplits[1];
    size_type csplits[1];

    rsplits[0] = split_row;
    csplits[0] = split_col;

    partition(array_to_vec(rsplits), 
	      array_to_vec(csplits),
	      A, P);
  }

  /*
    this should be built on top of the subrange accessor 
     with VecP::value_type == Vec::subrange
   */
  template <class Splits, class Vec, class VecP>
  inline void
  partition(const Splits& s, const Vec& x, VecP& p)
  {
    typedef typename Vec::size_type size_type;
    typedef typename Vec::value_type T;
    typename Splits::const_iterator si;
    si = s.begin();
    size_type i = 0;
    size_type prev = *si;
    p[i] = external_vec<T>(x.data(), *si); // Vec::subrange
    for (++si, ++i; si != s.end(); ++si, ++i) {
      p[i] = external_vec<T>(x.data() + prev, *si - prev);
      prev = *si;
    }
    p[i] = external_vec<T>(x.data() + prev, x.size() - prev);
  }


  template <class Vec, class VecP>
  inline void
  subdivide(typename Vec::size_type s, const Vec& x, VecP& p)
  {
    typedef typename Vec::value_type T;
    p[0] = external_vec<T>(x.data(), s);
    p[1] = external_vec<T>(x.data() + s, x.size() - s);
  }
  


} /* namespace mtl */

#endif /* MTL_PARTITION_H */
