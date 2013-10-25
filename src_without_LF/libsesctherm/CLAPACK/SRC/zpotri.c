#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zpotri_(char *uplo, integer *n, doublecomplex *a, 
	integer *lda, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    ZPOTRI computes the inverse of a complex Hermitian positive definite   
    matrix A using the Cholesky factorization A = U**H*U or A = L*L**H   
    computed by ZPOTRF.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  Upper triangle of A is stored;   
            = 'L':  Lower triangle of A is stored.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the triangular factor U or L from the Cholesky   
            factorization A = U**H*U or A = L*L**H, as computed by   
            ZPOTRF.   
            On exit, the upper or lower triangle of the (Hermitian)   
            inverse of A, overwriting the input factor U or L.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,N).   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, the (i,i) element of the factor U or L is   
                  zero, and the inverse could not be computed.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* System generated locals */
    integer a_dim1, a_offset, i__1;
    /* Local variables */
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int xerbla_(char *, integer *), zlauum_(
	    char *, integer *, doublecomplex *, integer *, integer *),
	     ztrtri_(char *, char *, integer *, doublecomplex *, integer *, 
	    integer *);

    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;

    /* Function Body */
    *info = 0;
    if (! lsame_(uplo, "U") && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*n)) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZPOTRI", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Invert the triangular Cholesky factor U or L. */

    ztrtri_(uplo, "Non-unit", n, &a[a_offset], lda, info);
    if (*info > 0) {
	return 0;
    }

/*     Form inv(U)*inv(U)' or inv(L)'*inv(L). */

    zlauum_(uplo, n, &a[a_offset], lda, info);

    return 0;

/*     End of ZPOTRI */

} /* zpotri_ */

