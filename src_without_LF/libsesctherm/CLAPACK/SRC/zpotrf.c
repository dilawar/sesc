#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zpotrf_(char *uplo, integer *n, doublecomplex *a, 
	integer *lda, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZPOTRF computes the Cholesky factorization of a complex Hermitian   
    positive definite matrix A.   

    The factorization has the form   
       A = U**H * U,  if UPLO = 'U', or   
       A = L  * L**H,  if UPLO = 'L',   
    where U is an upper triangular matrix and L is lower triangular.   

    This is the block version of the algorithm, calling Level 3 BLAS.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  Upper triangle of A is stored;   
            = 'L':  Lower triangle of A is stored.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

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

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, the leading minor of order i is not   
                  positive definite, and the factorization could not be   
                  completed.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {1.,0.};
    static integer c__1 = 1;
    static integer c_n1 = -1;
    static doublereal c_b14 = -1.;
    static doublereal c_b15 = 1.;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3, i__4;
    doublecomplex z__1;
    /* Local variables */
    static integer j;
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int zgemm_(char *, char *, integer *, integer *, 
	    integer *, doublecomplex *, doublecomplex *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, doublecomplex *, 
	    integer *), zherk_(char *, char *, integer *, 
	    integer *, doublereal *, doublecomplex *, integer *, doublereal *,
	     doublecomplex *, integer *);
    static logical upper;
    extern /* Subroutine */ int ztrsm_(char *, char *, char *, char *, 
	    integer *, integer *, doublecomplex *, doublecomplex *, integer *,
	     doublecomplex *, integer *);
    static integer jb, nb;
    extern /* Subroutine */ int zpotf2_(char *, integer *, doublecomplex *, 
	    integer *, integer *), xerbla_(char *, integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen);
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;

    /* Function Body */
    *info = 0;
    upper = lsame_(uplo, "U");
    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*n)) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZPOTRF", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Determine the block size for this environment. */

    nb = ilaenv_(&c__1, "ZPOTRF", uplo, n, &c_n1, &c_n1, &c_n1, (ftnlen)6, (
	    ftnlen)1);
    if (nb <= 1 || nb >= *n) {

/*        Use unblocked code. */

	zpotf2_(uplo, n, &a[a_offset], lda, info);
    } else {

/*        Use blocked code. */

	if (upper) {

/*           Compute the Cholesky factorization A = U'*U. */

	    i__1 = *n;
	    i__2 = nb;
	    for (j = 1; i__2 < 0 ? j >= i__1 : j <= i__1; j += i__2) {

/*              Update and factorize the current diagonal block and test   
                for non-positive-definiteness.   

   Computing MIN */
		i__3 = nb, i__4 = *n - j + 1;
		jb = min(i__3,i__4);
		i__3 = j - 1;
		zherk_("Upper", "Conjugate transpose", &jb, &i__3, &c_b14, &
			a_ref(1, j), lda, &c_b15, &a_ref(j, j), lda);
		zpotf2_("Upper", &jb, &a_ref(j, j), lda, info);
		if (*info != 0) {
		    goto L30;
		}
		if (j + jb <= *n) {

/*                 Compute the current block row. */

		    i__3 = *n - j - jb + 1;
		    i__4 = j - 1;
		    z__1.r = -1., z__1.i = 0.;
		    zgemm_("Conjugate transpose", "No transpose", &jb, &i__3, 
			    &i__4, &z__1, &a_ref(1, j), lda, &a_ref(1, j + jb)
			    , lda, &c_b1, &a_ref(j, j + jb), lda);
		    i__3 = *n - j - jb + 1;
		    ztrsm_("Left", "Upper", "Conjugate transpose", "Non-unit",
			     &jb, &i__3, &c_b1, &a_ref(j, j), lda, &a_ref(j, 
			    j + jb), lda);
		}
/* L10: */
	    }

	} else {

/*           Compute the Cholesky factorization A = L*L'. */

	    i__2 = *n;
	    i__1 = nb;
	    for (j = 1; i__1 < 0 ? j >= i__2 : j <= i__2; j += i__1) {

/*              Update and factorize the current diagonal block and test   
                for non-positive-definiteness.   

   Computing MIN */
		i__3 = nb, i__4 = *n - j + 1;
		jb = min(i__3,i__4);
		i__3 = j - 1;
		zherk_("Lower", "No transpose", &jb, &i__3, &c_b14, &a_ref(j, 
			1), lda, &c_b15, &a_ref(j, j), lda);
		zpotf2_("Lower", &jb, &a_ref(j, j), lda, info);
		if (*info != 0) {
		    goto L30;
		}
		if (j + jb <= *n) {

/*                 Compute the current block column. */

		    i__3 = *n - j - jb + 1;
		    i__4 = j - 1;
		    z__1.r = -1., z__1.i = 0.;
		    zgemm_("No transpose", "Conjugate transpose", &i__3, &jb, 
			    &i__4, &z__1, &a_ref(j + jb, 1), lda, &a_ref(j, 1)
			    , lda, &c_b1, &a_ref(j + jb, j), lda);
		    i__3 = *n - j - jb + 1;
		    ztrsm_("Right", "Lower", "Conjugate transpose", "Non-unit"
			    , &i__3, &jb, &c_b1, &a_ref(j, j), lda, &a_ref(j 
			    + jb, j), lda);
		}
/* L20: */
	    }
	}
    }
    goto L40;

L30:
    *info = *info + j - 1;

L40:
    return 0;

/*     End of ZPOTRF */

} /* zpotrf_ */

#undef a_ref
#undef a_subscr


