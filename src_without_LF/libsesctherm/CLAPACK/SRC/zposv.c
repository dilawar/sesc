#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zposv_(char *uplo, integer *n, integer *nrhs, 
	doublecomplex *a, integer *lda, doublecomplex *b, integer *ldb, 
	integer *info)
{
/*  -- LAPACK driver routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    ZPOSV computes the solution to a complex system of linear equations   
       A * X = B,   
    where A is an N-by-N Hermitian positive definite matrix and X and B   
    are N-by-NRHS matrices.   

    The Cholesky decomposition is used to factor A as   
       A = U**H* U,  if UPLO = 'U', or   
       A = L * L**H,  if UPLO = 'L',   
    where U is an upper triangular matrix and  L is a lower triangular   
    matrix.  The factored form of A is then used to solve the system of   
    equations A * X = B.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  Upper triangle of A is stored;   
            = 'L':  Lower triangle of A is stored.   

    N       (input) INTEGER   
            The number of linear equations, i.e., the order of the   
            matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the Hermitian matrix A.  If UPLO = 'U', the leading   
            N-by-N upper triangular part of A contains the upper   
            triangular part of the matrix A, and the strictly lower   
            triangular part of A is not referenced.  If UPLO = 'L', the   
            leading N-by-N lower triangular part of A contains the lower   
            triangular part of the matrix A, and the strictly upper   
            triangular part of A is not referenced.   

            On exit, if INFO = 0, the factor U or L from the Cholesky   
            factorization A = U**H*U or A = L*L**H.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,N).   

    B       (input/output) COMPLEX*16 array, dimension (LDB,NRHS)   
            On entry, the N-by-NRHS right hand side matrix B.   
            On exit, if INFO = 0, the N-by-NRHS solution matrix X.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, the leading minor of order i of A is not   
                  positive definite, so the factorization could not be   
                  completed, and the solution has not been computed.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* System generated locals */
    integer a_dim1, a_offset, b_dim1, b_offset, i__1;
    /* Local variables */
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int xerbla_(char *, integer *), zpotrf_(
	    char *, integer *, doublecomplex *, integer *, integer *),
	     zpotrs_(char *, integer *, integer *, doublecomplex *, integer *,
	     doublecomplex *, integer *, integer *);

    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;

    /* Function Body */
    *info = 0;
    if (! lsame_(uplo, "U") && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*nrhs < 0) {
	*info = -3;
    } else if (*lda < max(1,*n)) {
	*info = -5;
    } else if (*ldb < max(1,*n)) {
	*info = -7;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZPOSV ", &i__1);
	return 0;
    }

/*     Compute the Cholesky factorization A = U'*U or A = L*L'. */

    zpotrf_(uplo, n, &a[a_offset], lda, info);
    if (*info == 0) {

/*        Solve the system A*X = B, overwriting B with X. */

	zpotrs_(uplo, n, nrhs, &a[a_offset], lda, &b[b_offset], ldb, info);

    }
    return 0;

/*     End of ZPOSV */

} /* zposv_ */

