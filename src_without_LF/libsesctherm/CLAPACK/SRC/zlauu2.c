#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zlauu2_(char *uplo, integer *n, doublecomplex *a, 
	integer *lda, integer *info)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZLAUU2 computes the product U * U' or L' * L, where the triangular   
    factor U or L is stored in the upper or lower triangular part of   
    the array A.   

    If UPLO = 'U' or 'u' then the upper triangle of the result is stored,   
    overwriting the factor U in A.   
    If UPLO = 'L' or 'l' then the lower triangle of the result is stored,   
    overwriting the factor L in A.   

    This is the unblocked form of the algorithm, calling Level 2 BLAS.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            Specifies whether the triangular factor stored in the array A   
            is upper or lower triangular:   
            = 'U':  Upper triangular   
            = 'L':  Lower triangular   

    N       (input) INTEGER   
            The order of the triangular factor U or L.  N >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the triangular factor U or L.   
            On exit, if UPLO = 'U', the upper triangle of A is   
            overwritten with the upper triangle of the product U * U';   
            if UPLO = 'L', the lower triangle of A is overwritten with   
            the lower triangle of the product L' * L.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,N).   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -k, the k-th argument had an illegal value   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {1.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3;
    doublereal d__1;
    doublecomplex z__1;
    /* Local variables */
    static integer i__;
    extern logical lsame_(char *, char *);
    extern /* Double Complex */ VOID zdotc_(doublecomplex *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *);
    extern /* Subroutine */ int zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *);
    static logical upper;
    extern /* Subroutine */ int xerbla_(char *, integer *), zdscal_(
	    integer *, doublereal *, doublecomplex *, integer *), zlacgv_(
	    integer *, doublecomplex *, integer *);
    static doublereal aii;
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
	xerbla_("ZLAUU2", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

    if (upper) {

/*        Compute the product U * U'. */

	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    i__2 = a_subscr(i__, i__);
	    aii = a[i__2].r;
	    if (i__ < *n) {
		i__2 = a_subscr(i__, i__);
		i__3 = *n - i__;
		zdotc_(&z__1, &i__3, &a_ref(i__, i__ + 1), lda, &a_ref(i__, 
			i__ + 1), lda);
		d__1 = aii * aii + z__1.r;
		a[i__2].r = d__1, a[i__2].i = 0.;
		i__2 = *n - i__;
		zlacgv_(&i__2, &a_ref(i__, i__ + 1), lda);
		i__2 = i__ - 1;
		i__3 = *n - i__;
		z__1.r = aii, z__1.i = 0.;
		zgemv_("No transpose", &i__2, &i__3, &c_b1, &a_ref(1, i__ + 1)
			, lda, &a_ref(i__, i__ + 1), lda, &z__1, &a_ref(1, 
			i__), &c__1);
		i__2 = *n - i__;
		zlacgv_(&i__2, &a_ref(i__, i__ + 1), lda);
	    } else {
		zdscal_(&i__, &aii, &a_ref(1, i__), &c__1);
	    }
/* L10: */
	}

    } else {

/*        Compute the product L' * L. */

	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    i__2 = a_subscr(i__, i__);
	    aii = a[i__2].r;
	    if (i__ < *n) {
		i__2 = a_subscr(i__, i__);
		i__3 = *n - i__;
		zdotc_(&z__1, &i__3, &a_ref(i__ + 1, i__), &c__1, &a_ref(i__ 
			+ 1, i__), &c__1);
		d__1 = aii * aii + z__1.r;
		a[i__2].r = d__1, a[i__2].i = 0.;
		i__2 = i__ - 1;
		zlacgv_(&i__2, &a_ref(i__, 1), lda);
		i__2 = *n - i__;
		i__3 = i__ - 1;
		z__1.r = aii, z__1.i = 0.;
		zgemv_("Conjugate transpose", &i__2, &i__3, &c_b1, &a_ref(i__ 
			+ 1, 1), lda, &a_ref(i__ + 1, i__), &c__1, &z__1, &
			a_ref(i__, 1), lda);
		i__2 = i__ - 1;
		zlacgv_(&i__2, &a_ref(i__, 1), lda);
	    } else {
		zdscal_(&i__, &aii, &a_ref(i__, 1), lda);
	    }
/* L20: */
	}
    }

    return 0;

/*     End of ZLAUU2 */

} /* zlauu2_ */

#undef a_ref
#undef a_subscr


