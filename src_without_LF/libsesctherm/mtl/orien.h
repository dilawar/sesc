#ifndef MTL_ORIEN_H
#define MTL_ORIEN_H

namespace mtl {

struct row_orien;
struct column_orien;
struct row_major;
struct column_major;

//: Row Orientation
// This is used in matrix_implementation and in the indexer objects
// to map (row,column) to (major,minor).
struct row_orien {
  template <int MM, int NN> struct dims { enum { M = MM, N = NN }; };
  typedef row_tag orientation;
  typedef row_major constructor_tag;
  typedef column_orien transpose_type;
  template <class Dim>
  static typename Dim::size_type row(Dim d) { return d.first(); }
  template <class Dim>
  static typename Dim::size_type column(Dim d){ return d.second();}
  template <class Dim>
  static Dim map(Dim d) { return d; }
};

//: Column Orientation
// This is used in matrix_implementation and in the indexer objects
// to map (row,column) to (minor,major).
struct column_orien {
  template <int MM, int NN> struct dims { enum { M = NN, N = MM }; };
  typedef column_tag orientation;
  typedef row_orien transpose_type;
  typedef column_major constructor_tag;
  template <class Dim>
  static typename Dim::size_type row(Dim d) { return d.second(); }
  template <class Dim>
  static typename Dim::size_type column(Dim d){ return d.first(); }
#if 0
  /* the static dim has already been transposed in matrix.h
   so no need to do it here */
  template <class Dim>
  static typename Dim::transpose_type map(Dim d) { return d.transpose(); }
#else
  template <class Dim>
  static Dim map(Dim d) { return Dim(d.second(), d.first()); }
#endif
};

} /* namespace mtl */

#endif /* MTL_ORIEN_H */
