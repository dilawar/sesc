//dump Matrix to Harwell Boeing format file

#ifndef DUMPTOFILE_H
#define DUMPTOFILE_H

#include "matrix.h"
#include "mtl.h"

namespace mtl {

struct HBdumptofile {
  template<class Matrix>
  HBdumptofile(const char* filename, const Matrix& A) {

    matrix< rectangle<>, sparse<>, column_major >::type
        B(A.nrows(), A.ncols());
    matmat::copy(A, B);

    char Type[4]="RUA";
    char Title[73] ="The Matrix converted by mtl" ,
      Key[9]="DontKnow",
      Rhstype[4]="F  " ;
    char Ptrfmt[17]="          (10I8)",
      Indfmt[17]="          (10I8)",
      Valfmt[21]="            (5E16.8)",
      Rhsfmt[21]="            (5E16.8)";

    int* ptr = B.get_ptr();
    int* ind = B.get_ind();

    for (int i=0; i<B.nrows()+1; i++)
      ptr[i] += 1;

    for (int i=0; i<B.nnz(); i++)
      ind[i] += 1;

    writeHB_mat_double(filename, B.nrows(), B.ncols(),
		       B.nnz(), ptr, ind,
		       B.get_val(), 0, NULL,
		       NULL, NULL,
		       Title, Key, Type,
		       Ptrfmt, Indfmt, Valfmt, Rhsfmt,
		       Rhstype);
  }
};

} /* namespace mtl */

#endif
