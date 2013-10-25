#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zung2r_(integer *m, integer *n, integer *k, 
	doublecomplex *a, integer *lda, doublecomplex *tau, doublecomplex *
	work, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZUNG2R generates an m by n complex matrix Q with orthonormal columns,   
    which is defined as the first n columns of a product of k elementary   
    reflectors of order m   

          Q  =  H(1) H(2) . . . H(k)   

    as returned by ZGEQRF.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix Q. M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix Q. M >= N >= 0.   

    K       (input) INTEGER   
            The number of elementary reflectors whose product defines the   
            matrix Q. N >= K >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the i-th column must contain the vector which   
            defines the elementary reflector H(i), for i = 1,2,...,k, as   
            returned by ZGEQRF in the first k columns of its array   
            argument A.   
            On exit, the m by n matrix Q.   

    LDA     (input) INTEGER   
            The first dimension of the array A. LDA >= max(1,M).   

    TAU     (input) COMPLEX*16 array, dimension (K)   
            TAU(i) must contain the scalar factor of the elementary   
            reflector H(i), as returned by ZGEQRF.   

    WORK    (workspace) COMPLEX*16 array, dimension (N)   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument has an illegal value   

    =====================================================================   


       Test the input arguments   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3;
    doublecomplex z__1;
    /* Local variables */
    static integer i__, j, l;
    extern /* Subroutine */ int zscal_(integer *, doublecomplex *, 
	    doublecomplex *, integer *), zlarf_(char *, integer *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, doublecomplex *, 
	    integer *, doublecomplex *), xerbla_(char *, integer *);
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --tau;
    --work;

    /* Function Body */
    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0 || *n > *m) {
	*info = -2;
    } else if (*k < 0 || *k > *n) {
	*info = -3;
    } else if (*lda < max(1,*m)) {
	*info = -5;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZUNG2R", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n <= 0) {
	return 0;
    }

/*     Initialise columns k+1:n to columns of the unit matrix */

    i__1 = *n;
    for (j = *k + 1; j <= i__1; ++j) {
	i__2 = *m;
	for (l = 1; l <= i__2; ++l) {
	    i__3 = a_subscr(l, j);
	    a[i__3].r = 0., a[i__3].i = 0.;
/* L10: */
	}
	i__2 = a_subscr(j, j);
	a[i__2].r = 1., a[i__2].i = 0.;
/* L20: */
    }

    for (i__ = *k; i__ >= 1; --i__) {

/*        Apply H(i) to A(i:m,i:n) from the left */

	if (i__ < *n) {
	    i__1 = a_subscr(i__, i__);
	    a[i__1].r = 1., a[i__1].i = 0.;
	    i__1 = *m - i__ + 1;
	    i__2 = *n - i__;
	    zlarf_("Left", &i__1, &i__2, &a_ref(i__, i__), &c__1, &tau[i__], &
		    a_ref(i__, i__ + 1), lda, &work[1]);
	}
	if (i__ < *m) {
	    i__1 = *m - i__;
	    i__2 = i__;
	    z__1.r = -tau[i__2].r, z__1.i = -tau[i__2].i;
	    zscal_(&i__1, &z__1, &a_ref(i__ + 1, i__), &c__1);
	}
	i__1 = a_subscr(i__, i__);
	i__2 = i__;
	z__1.r = 1. - tau[i__2].r, z__1.i = 0. - tau[i__2].i;
	a[i__1].r = z__1.r, a[i__1].i = z__1.i;

/*        Set A(1:i-1,i) to zero */

	i__1 = i__ - 1;
	for (l = 1; l <= i__1; ++l) {
	    i__2 = a_subscr(l, i__);
	    a[i__2].r = 0., a[i__2].i = 0.;
/* L30: */
	}
/* L40: */
    }
    return 0;

/*     End of ZUNG2R */

} /* zung2r_ */

#undef a_ref
#undef a_subscr


