#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dpptrs_(char *uplo, integer *n, integer *nrhs, 
	doublereal *ap, doublereal *b, integer *ldb, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    DPPTRS solves a system of linear equations A*X = B with a symmetric   
    positive definite matrix A in packed storage using the Cholesky   
    factorization A = U**T*U or A = L*L**T computed by DPPTRF.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  Upper triangle of A is stored;   
            = 'L':  Lower triangle of A is stored.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    AP      (input) DOUBLE PRECISION array, dimension (N*(N+1)/2)   
            The triangular factor U or L from the Cholesky factorization   
            A = U**T*U or A = L*L**T, packed columnwise in a linear   
            array.  The j-th column of U or L is stored in the array AP   
            as follows:   
            if UPLO = 'U', AP(i + (j-1)*j/2) = U(i,j) for 1<=i<=j;   
            if UPLO = 'L', AP(i + (j-1)*(2n-j)/2) = L(i,j) for j<=i<=n.   

    B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)   
            On entry, the right hand side matrix B.   
            On exit, the solution matrix X.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer b_dim1, b_offset, i__1;
    /* Local variables */
    static integer i__;
    extern logical lsame_(char *, char *);
    static logical upper;
    extern /* Subroutine */ int dtpsv_(char *, char *, char *, integer *, 
	    doublereal *, doublereal *, integer *), 
	    xerbla_(char *, integer *);
#define b_ref(a_1,a_2) b[(a_2)*b_dim1 + a_1]


    --ap;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;

    /* Function Body */
    *info = 0;
    upper = lsame_(uplo, "U");
    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*nrhs < 0) {
	*info = -3;
    } else if (*ldb < max(1,*n)) {
	*info = -6;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DPPTRS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0) {
	return 0;
    }

    if (upper) {

/*        Solve A*X = B where A = U'*U. */

	i__1 = *nrhs;
	for (i__ = 1; i__ <= i__1; ++i__) {

/*           Solve U'*X = B, overwriting B with X. */

	    dtpsv_("Upper", "Transpose", "Non-unit", n, &ap[1], &b_ref(1, i__)
		    , &c__1);

/*           Solve U*X = B, overwriting B with X. */

	    dtpsv_("Upper", "No transpose", "Non-unit", n, &ap[1], &b_ref(1, 
		    i__), &c__1);
/* L10: */
	}
    } else {

/*        Solve A*X = B where A = L*L'. */

	i__1 = *nrhs;
	for (i__ = 1; i__ <= i__1; ++i__) {

/*           Solve L*Y = B, overwriting B with X. */

	    dtpsv_("Lower", "No transpose", "Non-unit", n, &ap[1], &b_ref(1, 
		    i__), &c__1);

/*           Solve L'*X = Y, overwriting B with X. */

	    dtpsv_("Lower", "Transpose", "Non-unit", n, &ap[1], &b_ref(1, i__)
		    , &c__1);
/* L20: */
	}
    }

    return 0;

/*     End of DPPTRS */

} /* dpptrs_ */

#undef b_ref


