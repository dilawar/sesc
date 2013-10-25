#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zungr2_(integer *m, integer *n, integer *k, 
	doublecomplex *a, integer *lda, doublecomplex *tau, doublecomplex *
	work, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZUNGR2 generates an m by n complex matrix Q with orthonormal rows,   
    which is defined as the last m rows of a product of k elementary   
    reflectors of order n   

          Q  =  H(1)' H(2)' . . . H(k)'   

    as returned by ZGERQF.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix Q. M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix Q. N >= M.   

    K       (input) INTEGER   
            The number of elementary reflectors whose product defines the   
            matrix Q. M >= K >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the (m-k+i)-th row must contain the vector which   
            defines the elementary reflector H(i), for i = 1,2,...,k, as   
            returned by ZGERQF in the last k rows of its array argument   
            A.   
            On exit, the m-by-n matrix Q.   

    LDA     (input) INTEGER   
            The first dimension of the array A. LDA >= max(1,M).   

    TAU     (input) COMPLEX*16 array, dimension (K)   
            TAU(i) must contain the scalar factor of the elementary   
            reflector H(i), as returned by ZGERQF.   

    WORK    (workspace) COMPLEX*16 array, dimension (M)   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument has an illegal value   

    =====================================================================   


       Test the input arguments   

       Parameter adjustments */
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3;
    doublecomplex z__1, z__2;
    /* Builtin functions */
    void d_cnjg(doublecomplex *, doublecomplex *);
    /* Local variables */
    static integer i__, j, l;
    extern /* Subroutine */ int zscal_(integer *, doublecomplex *, 
	    doublecomplex *, integer *), zlarf_(char *, integer *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, doublecomplex *, 
	    integer *, doublecomplex *);
    static integer ii;
    extern /* Subroutine */ int xerbla_(char *, integer *), zlacgv_(
	    integer *, doublecomplex *, integer *);
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
    } else if (*n < *m) {
	*info = -2;
    } else if (*k < 0 || *k > *m) {
	*info = -3;
    } else if (*lda < max(1,*m)) {
	*info = -5;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZUNGR2", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*m <= 0) {
	return 0;
    }

    if (*k < *m) {

/*        Initialise rows 1:m-k to rows of the unit matrix */

	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = *m - *k;
	    for (l = 1; l <= i__2; ++l) {
		i__3 = a_subscr(l, j);
		a[i__3].r = 0., a[i__3].i = 0.;
/* L10: */
	    }
	    if (j > *n - *m && j <= *n - *k) {
		i__2 = a_subscr(*m - *n + j, j);
		a[i__2].r = 1., a[i__2].i = 0.;
	    }
/* L20: */
	}
    }

    i__1 = *k;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ii = *m - *k + i__;

/*        Apply H(i)' to A(1:m-k+i,1:n-k+i) from the right */

	i__2 = *n - *m + ii - 1;
	zlacgv_(&i__2, &a_ref(ii, 1), lda);
	i__2 = a_subscr(ii, *n - *m + ii);
	a[i__2].r = 1., a[i__2].i = 0.;
	i__2 = ii - 1;
	i__3 = *n - *m + ii;
	d_cnjg(&z__1, &tau[i__]);
	zlarf_("Right", &i__2, &i__3, &a_ref(ii, 1), lda, &z__1, &a[a_offset],
		 lda, &work[1]);
	i__2 = *n - *m + ii - 1;
	i__3 = i__;
	z__1.r = -tau[i__3].r, z__1.i = -tau[i__3].i;
	zscal_(&i__2, &z__1, &a_ref(ii, 1), lda);
	i__2 = *n - *m + ii - 1;
	zlacgv_(&i__2, &a_ref(ii, 1), lda);
	i__2 = a_subscr(ii, *n - *m + ii);
	d_cnjg(&z__2, &tau[i__]);
	z__1.r = 1. - z__2.r, z__1.i = 0. - z__2.i;
	a[i__2].r = z__1.r, a[i__2].i = z__1.i;

/*        Set A(m-k+i,n-k+i+1:n) to zero */

	i__2 = *n;
	for (l = *n - *m + ii + 1; l <= i__2; ++l) {
	    i__3 = a_subscr(ii, l);
	    a[i__3].r = 0., a[i__3].i = 0.;
/* L30: */
	}
/* L40: */
    }
    return 0;

/*     End of ZUNGR2 */

} /* zungr2_ */

#undef a_ref
#undef a_subscr


