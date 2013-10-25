#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zptsv_(integer *n, integer *nrhs, doublereal *d__, 
	doublecomplex *e, doublecomplex *b, integer *ldb, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 25, 1997   


    Purpose   
    =======   

    ZPTSV computes the solution to a complex system of linear equations   
    A*X = B, where A is an N-by-N Hermitian positive definite tridiagonal   
    matrix, and X and B are N-by-NRHS matrices.   

    A is factored as A = L*D*L**H, and the factored form of A is then   
    used to solve the system of equations.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    D       (input/output) DOUBLE PRECISION array, dimension (N)   
            On entry, the n diagonal elements of the tridiagonal matrix   
            A.  On exit, the n diagonal elements of the diagonal matrix   
            D from the factorization A = L*D*L**H.   

    E       (input/output) COMPLEX*16 array, dimension (N-1)   
            On entry, the (n-1) subdiagonal elements of the tridiagonal   
            matrix A.  On exit, the (n-1) subdiagonal elements of the   
            unit bidiagonal factor L from the L*D*L**H factorization of   
            A.  E can also be regarded as the superdiagonal of the unit   
            bidiagonal factor U from the U**H*D*U factorization of A.   

    B       (input/output) COMPLEX*16 array, dimension (LDB,N)   
            On entry, the N-by-NRHS right hand side matrix B.   
            On exit, if INFO = 0, the N-by-NRHS solution matrix X.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, the leading minor of order i is not   
                  positive definite, and the solution has not been   
                  computed.  The factorization has not been completed   
                  unless i = N.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* System generated locals */
    integer b_dim1, b_offset, i__1;
    /* Local variables */
    extern /* Subroutine */ int xerbla_(char *, integer *), zpttrf_(
	    integer *, doublereal *, doublecomplex *, integer *), zpttrs_(
	    char *, integer *, integer *, doublereal *, doublecomplex *, 
	    doublecomplex *, integer *, integer *);

    --d__;
    --e;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;

    /* Function Body */
    *info = 0;
    if (*n < 0) {
	*info = -1;
    } else if (*nrhs < 0) {
	*info = -2;
    } else if (*ldb < max(1,*n)) {
	*info = -6;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZPTSV ", &i__1);
	return 0;
    }

/*     Compute the L*D*L' (or U'*D*U) factorization of A. */

    zpttrf_(n, &d__[1], &e[1], info);
    if (*info == 0) {

/*        Solve the system A*X = B, overwriting B with X. */

	zpttrs_("Lower", n, nrhs, &d__[1], &e[1], &b[b_offset], ldb, info);
    }
    return 0;

/*     End of ZPTSV */

} /* zptsv_ */

