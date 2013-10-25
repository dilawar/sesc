#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int ztptrs_(char *uplo, char *trans, char *diag, integer *n, 
	integer *nrhs, doublecomplex *ap, doublecomplex *b, integer *ldb, 
	integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZTPTRS solves a triangular system of the form   

       A * X = B,  A**T * X = B,  or  A**H * X = B,   

    where A is a triangular matrix of order N stored in packed format,   
    and B is an N-by-NRHS matrix.  A check is made to verify that A is   
    nonsingular.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  A is upper triangular;   
            = 'L':  A is lower triangular.   

    TRANS   (input) CHARACTER*1   
            Specifies the form of the system of equations:   
            = 'N':  A * X = B     (No transpose)   
            = 'T':  A**T * X = B  (Transpose)   
            = 'C':  A**H * X = B  (Conjugate transpose)   

    DIAG    (input) CHARACTER*1   
            = 'N':  A is non-unit triangular;   
            = 'U':  A is unit triangular.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    AP      (input) COMPLEX*16 array, dimension (N*(N+1)/2)   
            The upper or lower triangular matrix A, packed columnwise in   
            a linear array.  The j-th column of A is stored in the array   
            AP as follows:   
            if UPLO = 'U', AP(i + (j-1)*j/2) = A(i,j) for 1<=i<=j;   
            if UPLO = 'L', AP(i + (j-1)*(2*n-j)/2) = A(i,j) for j<=i<=n.   

    B       (input/output) COMPLEX*16 array, dimension (LDB,NRHS)   
            On entry, the right hand side matrix B.   
            On exit, if INFO = 0, the solution matrix X.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, the i-th diagonal element of A is zero,   
                  indicating that the matrix is singular and the   
                  solutions X have not been computed.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer b_dim1, b_offset, i__1, i__2;
    /* Local variables */
    static integer j;
    extern logical lsame_(char *, char *);
    static logical upper;
    extern /* Subroutine */ int ztpsv_(char *, char *, char *, integer *, 
	    doublecomplex *, doublecomplex *, integer *);
    static integer jc;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static logical nounit;
#define b_subscr(a_1,a_2) (a_2)*b_dim1 + a_1
#define b_ref(a_1,a_2) b[b_subscr(a_1,a_2)]


    --ap;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;

    /* Function Body */
    *info = 0;
    upper = lsame_(uplo, "U");
    nounit = lsame_(diag, "N");
    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (! lsame_(trans, "N") && ! lsame_(trans, 
	    "T") && ! lsame_(trans, "C")) {
	*info = -2;
    } else if (! nounit && ! lsame_(diag, "U")) {
	*info = -3;
    } else if (*n < 0) {
	*info = -4;
    } else if (*nrhs < 0) {
	*info = -5;
    } else if (*ldb < max(1,*n)) {
	*info = -8;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZTPTRS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Check for singularity. */

    if (nounit) {
	if (upper) {
	    jc = 1;
	    i__1 = *n;
	    for (*info = 1; *info <= i__1; ++(*info)) {
		i__2 = jc + *info - 1;
		if (ap[i__2].r == 0. && ap[i__2].i == 0.) {
		    return 0;
		}
		jc += *info;
/* L10: */
	    }
	} else {
	    jc = 1;
	    i__1 = *n;
	    for (*info = 1; *info <= i__1; ++(*info)) {
		i__2 = jc;
		if (ap[i__2].r == 0. && ap[i__2].i == 0.) {
		    return 0;
		}
		jc = jc + *n - *info + 1;
/* L20: */
	    }
	}
    }
    *info = 0;

/*     Solve  A * x = b,  A**T * x = b,  or  A**H * x = b. */

    i__1 = *nrhs;
    for (j = 1; j <= i__1; ++j) {
	ztpsv_(uplo, trans, diag, n, &ap[1], &b_ref(1, j), &c__1);
/* L30: */
    }

    return 0;

/*     End of ZTPTRS */

} /* ztptrs_ */

#undef b_ref
#undef b_subscr


