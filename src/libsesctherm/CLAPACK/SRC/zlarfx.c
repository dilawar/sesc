#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zlarfx_(char *side, integer *m, integer *n, 
	doublecomplex *v, doublecomplex *tau, doublecomplex *c__, integer *
	ldc, doublecomplex *work)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZLARFX applies a complex elementary reflector H to a complex m by n   
    matrix C, from either the left or the right. H is represented in the   
    form   

          H = I - tau * v * v'   

    where tau is a complex scalar and v is a complex vector.   

    If tau = 0, then H is taken to be the unit matrix   

    This version uses inline code if H has order < 11.   

    Arguments   
    =========   

    SIDE    (input) CHARACTER*1   
            = 'L': form  H * C   
            = 'R': form  C * H   

    M       (input) INTEGER   
            The number of rows of the matrix C.   

    N       (input) INTEGER   
            The number of columns of the matrix C.   

    V       (input) COMPLEX*16 array, dimension (M) if SIDE = 'L'   
                                          or (N) if SIDE = 'R'   
            The vector v in the representation of H.   

    TAU     (input) COMPLEX*16   
            The value tau in the representation of H.   

    C       (input/output) COMPLEX*16 array, dimension (LDC,N)   
            On entry, the m by n matrix C.   
            On exit, C is overwritten by the matrix H * C if SIDE = 'L',   
            or C * H if SIDE = 'R'.   

    LDC     (input) INTEGER   
            The leading dimension of the array C. LDA >= max(1,M).   

    WORK    (workspace) COMPLEX*16 array, dimension (N) if SIDE = 'L'   
                                              or (M) if SIDE = 'R'   
            WORK is not referenced if H has order < 11.   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {0.,0.};
    static doublecomplex c_b2 = {1.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer c_dim1, c_offset, i__1, i__2, i__3, i__4, i__5, i__6, i__7, i__8, 
	    i__9, i__10, i__11;
    doublecomplex z__1, z__2, z__3, z__4, z__5, z__6, z__7, z__8, z__9, z__10,
	     z__11, z__12, z__13, z__14, z__15, z__16, z__17, z__18, z__19;
    /* Builtin functions */
    void d_cnjg(doublecomplex *, doublecomplex *);
    /* Local variables */
    static integer j;
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int zgerc_(integer *, integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *);
    static doublecomplex t1, t2, t3, t4, t5, t6, t7, t8, t9, v1, v2, v3, v4, 
	    v5, v6, v7, v8, v9, t10, v10, sum;
#define c___subscr(a_1,a_2) (a_2)*c_dim1 + a_1
#define c___ref(a_1,a_2) c__[c___subscr(a_1,a_2)]


    --v;
    c_dim1 = *ldc;
    c_offset = 1 + c_dim1 * 1;
    c__ -= c_offset;
    --work;

    /* Function Body */
    if (tau->r == 0. && tau->i == 0.) {
	return 0;
    }
    if (lsame_(side, "L")) {

/*        Form  H * C, where H has order m. */

	switch (*m) {
	    case 1:  goto L10;
	    case 2:  goto L30;
	    case 3:  goto L50;
	    case 4:  goto L70;
	    case 5:  goto L90;
	    case 6:  goto L110;
	    case 7:  goto L130;
	    case 8:  goto L150;
	    case 9:  goto L170;
	    case 10:  goto L190;
	}

/*        Code for general M   

          w := C'*v */

	zgemv_("Conjugate transpose", m, n, &c_b2, &c__[c_offset], ldc, &v[1],
		 &c__1, &c_b1, &work[1], &c__1);

/*        C := C - tau * v * w' */

	z__1.r = -tau->r, z__1.i = -tau->i;
	zgerc_(m, n, &z__1, &v[1], &c__1, &work[1], &c__1, &c__[c_offset], 
		ldc);
	goto L410;
L10:

/*        Special code for 1 x 1 Householder */

	z__3.r = tau->r * v[1].r - tau->i * v[1].i, z__3.i = tau->r * v[1].i 
		+ tau->i * v[1].r;
	d_cnjg(&z__4, &v[1]);
	z__2.r = z__3.r * z__4.r - z__3.i * z__4.i, z__2.i = z__3.r * z__4.i 
		+ z__3.i * z__4.r;
	z__1.r = 1. - z__2.r, z__1.i = 0. - z__2.i;
	t1.r = z__1.r, t1.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__1.r = t1.r * c__[i__3].r - t1.i * c__[i__3].i, z__1.i = t1.r * 
		    c__[i__3].i + t1.i * c__[i__3].r;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L20: */
	}
	goto L410;
L30:

/*        Special code for 2 x 2 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__2.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__2.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__3.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__3.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__1.r = z__2.r + z__3.r, z__1.i = z__2.i + z__3.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L40: */
	}
	goto L410;
L50:

/*        Special code for 3 x 3 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__3.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__3.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__4.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__4.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__2.r = z__3.r + z__4.r, z__2.i = z__3.i + z__4.i;
	    i__4 = c___subscr(3, j);
	    z__5.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__5.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__1.r = z__2.r + z__5.r, z__1.i = z__2.i + z__5.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L60: */
	}
	goto L410;
L70:

/*        Special code for 4 x 4 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	d_cnjg(&z__1, &v[4]);
	v4.r = z__1.r, v4.i = z__1.i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__4.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__4.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__5.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__5.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__3.r = z__4.r + z__5.r, z__3.i = z__4.i + z__5.i;
	    i__4 = c___subscr(3, j);
	    z__6.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__6.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__2.r = z__3.r + z__6.r, z__2.i = z__3.i + z__6.i;
	    i__5 = c___subscr(4, j);
	    z__7.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__7.i = v4.r * 
		    c__[i__5].i + v4.i * c__[i__5].r;
	    z__1.r = z__2.r + z__7.r, z__1.i = z__2.i + z__7.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(4, j);
	    i__3 = c___subscr(4, j);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L80: */
	}
	goto L410;
L90:

/*        Special code for 5 x 5 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	d_cnjg(&z__1, &v[4]);
	v4.r = z__1.r, v4.i = z__1.i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	d_cnjg(&z__1, &v[5]);
	v5.r = z__1.r, v5.i = z__1.i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__5.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__5.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__6.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__6.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__4.r = z__5.r + z__6.r, z__4.i = z__5.i + z__6.i;
	    i__4 = c___subscr(3, j);
	    z__7.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__7.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__3.r = z__4.r + z__7.r, z__3.i = z__4.i + z__7.i;
	    i__5 = c___subscr(4, j);
	    z__8.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__8.i = v4.r * 
		    c__[i__5].i + v4.i * c__[i__5].r;
	    z__2.r = z__3.r + z__8.r, z__2.i = z__3.i + z__8.i;
	    i__6 = c___subscr(5, j);
	    z__9.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__9.i = v5.r * 
		    c__[i__6].i + v5.i * c__[i__6].r;
	    z__1.r = z__2.r + z__9.r, z__1.i = z__2.i + z__9.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(4, j);
	    i__3 = c___subscr(4, j);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(5, j);
	    i__3 = c___subscr(5, j);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L100: */
	}
	goto L410;
L110:

/*        Special code for 6 x 6 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	d_cnjg(&z__1, &v[4]);
	v4.r = z__1.r, v4.i = z__1.i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	d_cnjg(&z__1, &v[5]);
	v5.r = z__1.r, v5.i = z__1.i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	d_cnjg(&z__1, &v[6]);
	v6.r = z__1.r, v6.i = z__1.i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__6.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__6.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__7.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__7.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__5.r = z__6.r + z__7.r, z__5.i = z__6.i + z__7.i;
	    i__4 = c___subscr(3, j);
	    z__8.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__8.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__4.r = z__5.r + z__8.r, z__4.i = z__5.i + z__8.i;
	    i__5 = c___subscr(4, j);
	    z__9.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__9.i = v4.r * 
		    c__[i__5].i + v4.i * c__[i__5].r;
	    z__3.r = z__4.r + z__9.r, z__3.i = z__4.i + z__9.i;
	    i__6 = c___subscr(5, j);
	    z__10.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__10.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__2.r = z__3.r + z__10.r, z__2.i = z__3.i + z__10.i;
	    i__7 = c___subscr(6, j);
	    z__11.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__11.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__1.r = z__2.r + z__11.r, z__1.i = z__2.i + z__11.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(4, j);
	    i__3 = c___subscr(4, j);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(5, j);
	    i__3 = c___subscr(5, j);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(6, j);
	    i__3 = c___subscr(6, j);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L120: */
	}
	goto L410;
L130:

/*        Special code for 7 x 7 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	d_cnjg(&z__1, &v[4]);
	v4.r = z__1.r, v4.i = z__1.i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	d_cnjg(&z__1, &v[5]);
	v5.r = z__1.r, v5.i = z__1.i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	d_cnjg(&z__1, &v[6]);
	v6.r = z__1.r, v6.i = z__1.i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	d_cnjg(&z__1, &v[7]);
	v7.r = z__1.r, v7.i = z__1.i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__7.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__7.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__8.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__8.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__6.r = z__7.r + z__8.r, z__6.i = z__7.i + z__8.i;
	    i__4 = c___subscr(3, j);
	    z__9.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__9.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__5.r = z__6.r + z__9.r, z__5.i = z__6.i + z__9.i;
	    i__5 = c___subscr(4, j);
	    z__10.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__10.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__4.r = z__5.r + z__10.r, z__4.i = z__5.i + z__10.i;
	    i__6 = c___subscr(5, j);
	    z__11.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__11.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__3.r = z__4.r + z__11.r, z__3.i = z__4.i + z__11.i;
	    i__7 = c___subscr(6, j);
	    z__12.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__12.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__2.r = z__3.r + z__12.r, z__2.i = z__3.i + z__12.i;
	    i__8 = c___subscr(7, j);
	    z__13.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__13.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__1.r = z__2.r + z__13.r, z__1.i = z__2.i + z__13.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(4, j);
	    i__3 = c___subscr(4, j);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(5, j);
	    i__3 = c___subscr(5, j);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(6, j);
	    i__3 = c___subscr(6, j);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(7, j);
	    i__3 = c___subscr(7, j);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L140: */
	}
	goto L410;
L150:

/*        Special code for 8 x 8 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	d_cnjg(&z__1, &v[4]);
	v4.r = z__1.r, v4.i = z__1.i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	d_cnjg(&z__1, &v[5]);
	v5.r = z__1.r, v5.i = z__1.i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	d_cnjg(&z__1, &v[6]);
	v6.r = z__1.r, v6.i = z__1.i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	d_cnjg(&z__1, &v[7]);
	v7.r = z__1.r, v7.i = z__1.i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	d_cnjg(&z__1, &v[8]);
	v8.r = z__1.r, v8.i = z__1.i;
	d_cnjg(&z__2, &v8);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t8.r = z__1.r, t8.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__8.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__8.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__9.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__9.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__7.r = z__8.r + z__9.r, z__7.i = z__8.i + z__9.i;
	    i__4 = c___subscr(3, j);
	    z__10.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__10.i = v3.r 
		    * c__[i__4].i + v3.i * c__[i__4].r;
	    z__6.r = z__7.r + z__10.r, z__6.i = z__7.i + z__10.i;
	    i__5 = c___subscr(4, j);
	    z__11.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__11.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__5.r = z__6.r + z__11.r, z__5.i = z__6.i + z__11.i;
	    i__6 = c___subscr(5, j);
	    z__12.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__12.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__4.r = z__5.r + z__12.r, z__4.i = z__5.i + z__12.i;
	    i__7 = c___subscr(6, j);
	    z__13.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__13.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__3.r = z__4.r + z__13.r, z__3.i = z__4.i + z__13.i;
	    i__8 = c___subscr(7, j);
	    z__14.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__14.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__2.r = z__3.r + z__14.r, z__2.i = z__3.i + z__14.i;
	    i__9 = c___subscr(8, j);
	    z__15.r = v8.r * c__[i__9].r - v8.i * c__[i__9].i, z__15.i = v8.r 
		    * c__[i__9].i + v8.i * c__[i__9].r;
	    z__1.r = z__2.r + z__15.r, z__1.i = z__2.i + z__15.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(4, j);
	    i__3 = c___subscr(4, j);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(5, j);
	    i__3 = c___subscr(5, j);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(6, j);
	    i__3 = c___subscr(6, j);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(7, j);
	    i__3 = c___subscr(7, j);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(8, j);
	    i__3 = c___subscr(8, j);
	    z__2.r = sum.r * t8.r - sum.i * t8.i, z__2.i = sum.r * t8.i + 
		    sum.i * t8.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L160: */
	}
	goto L410;
L170:

/*        Special code for 9 x 9 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	d_cnjg(&z__1, &v[4]);
	v4.r = z__1.r, v4.i = z__1.i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	d_cnjg(&z__1, &v[5]);
	v5.r = z__1.r, v5.i = z__1.i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	d_cnjg(&z__1, &v[6]);
	v6.r = z__1.r, v6.i = z__1.i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	d_cnjg(&z__1, &v[7]);
	v7.r = z__1.r, v7.i = z__1.i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	d_cnjg(&z__1, &v[8]);
	v8.r = z__1.r, v8.i = z__1.i;
	d_cnjg(&z__2, &v8);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t8.r = z__1.r, t8.i = z__1.i;
	d_cnjg(&z__1, &v[9]);
	v9.r = z__1.r, v9.i = z__1.i;
	d_cnjg(&z__2, &v9);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t9.r = z__1.r, t9.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__9.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__9.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__10.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__10.i = v2.r 
		    * c__[i__3].i + v2.i * c__[i__3].r;
	    z__8.r = z__9.r + z__10.r, z__8.i = z__9.i + z__10.i;
	    i__4 = c___subscr(3, j);
	    z__11.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__11.i = v3.r 
		    * c__[i__4].i + v3.i * c__[i__4].r;
	    z__7.r = z__8.r + z__11.r, z__7.i = z__8.i + z__11.i;
	    i__5 = c___subscr(4, j);
	    z__12.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__12.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__6.r = z__7.r + z__12.r, z__6.i = z__7.i + z__12.i;
	    i__6 = c___subscr(5, j);
	    z__13.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__13.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__5.r = z__6.r + z__13.r, z__5.i = z__6.i + z__13.i;
	    i__7 = c___subscr(6, j);
	    z__14.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__14.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__4.r = z__5.r + z__14.r, z__4.i = z__5.i + z__14.i;
	    i__8 = c___subscr(7, j);
	    z__15.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__15.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__3.r = z__4.r + z__15.r, z__3.i = z__4.i + z__15.i;
	    i__9 = c___subscr(8, j);
	    z__16.r = v8.r * c__[i__9].r - v8.i * c__[i__9].i, z__16.i = v8.r 
		    * c__[i__9].i + v8.i * c__[i__9].r;
	    z__2.r = z__3.r + z__16.r, z__2.i = z__3.i + z__16.i;
	    i__10 = c___subscr(9, j);
	    z__17.r = v9.r * c__[i__10].r - v9.i * c__[i__10].i, z__17.i = 
		    v9.r * c__[i__10].i + v9.i * c__[i__10].r;
	    z__1.r = z__2.r + z__17.r, z__1.i = z__2.i + z__17.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(4, j);
	    i__3 = c___subscr(4, j);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(5, j);
	    i__3 = c___subscr(5, j);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(6, j);
	    i__3 = c___subscr(6, j);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(7, j);
	    i__3 = c___subscr(7, j);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(8, j);
	    i__3 = c___subscr(8, j);
	    z__2.r = sum.r * t8.r - sum.i * t8.i, z__2.i = sum.r * t8.i + 
		    sum.i * t8.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(9, j);
	    i__3 = c___subscr(9, j);
	    z__2.r = sum.r * t9.r - sum.i * t9.i, z__2.i = sum.r * t9.i + 
		    sum.i * t9.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L180: */
	}
	goto L410;
L190:

/*        Special code for 10 x 10 Householder */

	d_cnjg(&z__1, &v[1]);
	v1.r = z__1.r, v1.i = z__1.i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	d_cnjg(&z__1, &v[2]);
	v2.r = z__1.r, v2.i = z__1.i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	d_cnjg(&z__1, &v[3]);
	v3.r = z__1.r, v3.i = z__1.i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	d_cnjg(&z__1, &v[4]);
	v4.r = z__1.r, v4.i = z__1.i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	d_cnjg(&z__1, &v[5]);
	v5.r = z__1.r, v5.i = z__1.i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	d_cnjg(&z__1, &v[6]);
	v6.r = z__1.r, v6.i = z__1.i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	d_cnjg(&z__1, &v[7]);
	v7.r = z__1.r, v7.i = z__1.i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	d_cnjg(&z__1, &v[8]);
	v8.r = z__1.r, v8.i = z__1.i;
	d_cnjg(&z__2, &v8);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t8.r = z__1.r, t8.i = z__1.i;
	d_cnjg(&z__1, &v[9]);
	v9.r = z__1.r, v9.i = z__1.i;
	d_cnjg(&z__2, &v9);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t9.r = z__1.r, t9.i = z__1.i;
	d_cnjg(&z__1, &v[10]);
	v10.r = z__1.r, v10.i = z__1.i;
	d_cnjg(&z__2, &v10);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t10.r = z__1.r, t10.i = z__1.i;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(1, j);
	    z__10.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__10.i = v1.r 
		    * c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(2, j);
	    z__11.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__11.i = v2.r 
		    * c__[i__3].i + v2.i * c__[i__3].r;
	    z__9.r = z__10.r + z__11.r, z__9.i = z__10.i + z__11.i;
	    i__4 = c___subscr(3, j);
	    z__12.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__12.i = v3.r 
		    * c__[i__4].i + v3.i * c__[i__4].r;
	    z__8.r = z__9.r + z__12.r, z__8.i = z__9.i + z__12.i;
	    i__5 = c___subscr(4, j);
	    z__13.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__13.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__7.r = z__8.r + z__13.r, z__7.i = z__8.i + z__13.i;
	    i__6 = c___subscr(5, j);
	    z__14.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__14.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__6.r = z__7.r + z__14.r, z__6.i = z__7.i + z__14.i;
	    i__7 = c___subscr(6, j);
	    z__15.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__15.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__5.r = z__6.r + z__15.r, z__5.i = z__6.i + z__15.i;
	    i__8 = c___subscr(7, j);
	    z__16.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__16.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__4.r = z__5.r + z__16.r, z__4.i = z__5.i + z__16.i;
	    i__9 = c___subscr(8, j);
	    z__17.r = v8.r * c__[i__9].r - v8.i * c__[i__9].i, z__17.i = v8.r 
		    * c__[i__9].i + v8.i * c__[i__9].r;
	    z__3.r = z__4.r + z__17.r, z__3.i = z__4.i + z__17.i;
	    i__10 = c___subscr(9, j);
	    z__18.r = v9.r * c__[i__10].r - v9.i * c__[i__10].i, z__18.i = 
		    v9.r * c__[i__10].i + v9.i * c__[i__10].r;
	    z__2.r = z__3.r + z__18.r, z__2.i = z__3.i + z__18.i;
	    i__11 = c___subscr(10, j);
	    z__19.r = v10.r * c__[i__11].r - v10.i * c__[i__11].i, z__19.i = 
		    v10.r * c__[i__11].i + v10.i * c__[i__11].r;
	    z__1.r = z__2.r + z__19.r, z__1.i = z__2.i + z__19.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(1, j);
	    i__3 = c___subscr(1, j);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(2, j);
	    i__3 = c___subscr(2, j);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(3, j);
	    i__3 = c___subscr(3, j);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(4, j);
	    i__3 = c___subscr(4, j);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(5, j);
	    i__3 = c___subscr(5, j);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(6, j);
	    i__3 = c___subscr(6, j);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(7, j);
	    i__3 = c___subscr(7, j);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(8, j);
	    i__3 = c___subscr(8, j);
	    z__2.r = sum.r * t8.r - sum.i * t8.i, z__2.i = sum.r * t8.i + 
		    sum.i * t8.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(9, j);
	    i__3 = c___subscr(9, j);
	    z__2.r = sum.r * t9.r - sum.i * t9.i, z__2.i = sum.r * t9.i + 
		    sum.i * t9.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(10, j);
	    i__3 = c___subscr(10, j);
	    z__2.r = sum.r * t10.r - sum.i * t10.i, z__2.i = sum.r * t10.i + 
		    sum.i * t10.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L200: */
	}
	goto L410;
    } else {

/*        Form  C * H, where H has order n. */

	switch (*n) {
	    case 1:  goto L210;
	    case 2:  goto L230;
	    case 3:  goto L250;
	    case 4:  goto L270;
	    case 5:  goto L290;
	    case 6:  goto L310;
	    case 7:  goto L330;
	    case 8:  goto L350;
	    case 9:  goto L370;
	    case 10:  goto L390;
	}

/*        Code for general N   

          w := C * v */

	zgemv_("No transpose", m, n, &c_b2, &c__[c_offset], ldc, &v[1], &c__1,
		 &c_b1, &work[1], &c__1);

/*        C := C - tau * w * v' */

	z__1.r = -tau->r, z__1.i = -tau->i;
	zgerc_(m, n, &z__1, &work[1], &c__1, &v[1], &c__1, &c__[c_offset], 
		ldc);
	goto L410;
L210:

/*        Special code for 1 x 1 Householder */

	z__3.r = tau->r * v[1].r - tau->i * v[1].i, z__3.i = tau->r * v[1].i 
		+ tau->i * v[1].r;
	d_cnjg(&z__4, &v[1]);
	z__2.r = z__3.r * z__4.r - z__3.i * z__4.i, z__2.i = z__3.r * z__4.i 
		+ z__3.i * z__4.r;
	z__1.r = 1. - z__2.r, z__1.i = 0. - z__2.i;
	t1.r = z__1.r, t1.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__1.r = t1.r * c__[i__3].r - t1.i * c__[i__3].i, z__1.i = t1.r * 
		    c__[i__3].i + t1.i * c__[i__3].r;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L220: */
	}
	goto L410;
L230:

/*        Special code for 2 x 2 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__2.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__2.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__3.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__3.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__1.r = z__2.r + z__3.r, z__1.i = z__2.i + z__3.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L240: */
	}
	goto L410;
L250:

/*        Special code for 3 x 3 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__3.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__3.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__4.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__4.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__2.r = z__3.r + z__4.r, z__2.i = z__3.i + z__4.i;
	    i__4 = c___subscr(j, 3);
	    z__5.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__5.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__1.r = z__2.r + z__5.r, z__1.i = z__2.i + z__5.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L260: */
	}
	goto L410;
L270:

/*        Special code for 4 x 4 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	v4.r = v[4].r, v4.i = v[4].i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__4.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__4.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__5.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__5.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__3.r = z__4.r + z__5.r, z__3.i = z__4.i + z__5.i;
	    i__4 = c___subscr(j, 3);
	    z__6.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__6.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__2.r = z__3.r + z__6.r, z__2.i = z__3.i + z__6.i;
	    i__5 = c___subscr(j, 4);
	    z__7.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__7.i = v4.r * 
		    c__[i__5].i + v4.i * c__[i__5].r;
	    z__1.r = z__2.r + z__7.r, z__1.i = z__2.i + z__7.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 4);
	    i__3 = c___subscr(j, 4);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L280: */
	}
	goto L410;
L290:

/*        Special code for 5 x 5 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	v4.r = v[4].r, v4.i = v[4].i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	v5.r = v[5].r, v5.i = v[5].i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__5.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__5.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__6.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__6.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__4.r = z__5.r + z__6.r, z__4.i = z__5.i + z__6.i;
	    i__4 = c___subscr(j, 3);
	    z__7.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__7.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__3.r = z__4.r + z__7.r, z__3.i = z__4.i + z__7.i;
	    i__5 = c___subscr(j, 4);
	    z__8.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__8.i = v4.r * 
		    c__[i__5].i + v4.i * c__[i__5].r;
	    z__2.r = z__3.r + z__8.r, z__2.i = z__3.i + z__8.i;
	    i__6 = c___subscr(j, 5);
	    z__9.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__9.i = v5.r * 
		    c__[i__6].i + v5.i * c__[i__6].r;
	    z__1.r = z__2.r + z__9.r, z__1.i = z__2.i + z__9.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 4);
	    i__3 = c___subscr(j, 4);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 5);
	    i__3 = c___subscr(j, 5);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L300: */
	}
	goto L410;
L310:

/*        Special code for 6 x 6 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	v4.r = v[4].r, v4.i = v[4].i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	v5.r = v[5].r, v5.i = v[5].i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	v6.r = v[6].r, v6.i = v[6].i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__6.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__6.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__7.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__7.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__5.r = z__6.r + z__7.r, z__5.i = z__6.i + z__7.i;
	    i__4 = c___subscr(j, 3);
	    z__8.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__8.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__4.r = z__5.r + z__8.r, z__4.i = z__5.i + z__8.i;
	    i__5 = c___subscr(j, 4);
	    z__9.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__9.i = v4.r * 
		    c__[i__5].i + v4.i * c__[i__5].r;
	    z__3.r = z__4.r + z__9.r, z__3.i = z__4.i + z__9.i;
	    i__6 = c___subscr(j, 5);
	    z__10.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__10.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__2.r = z__3.r + z__10.r, z__2.i = z__3.i + z__10.i;
	    i__7 = c___subscr(j, 6);
	    z__11.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__11.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__1.r = z__2.r + z__11.r, z__1.i = z__2.i + z__11.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 4);
	    i__3 = c___subscr(j, 4);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 5);
	    i__3 = c___subscr(j, 5);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 6);
	    i__3 = c___subscr(j, 6);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L320: */
	}
	goto L410;
L330:

/*        Special code for 7 x 7 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	v4.r = v[4].r, v4.i = v[4].i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	v5.r = v[5].r, v5.i = v[5].i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	v6.r = v[6].r, v6.i = v[6].i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	v7.r = v[7].r, v7.i = v[7].i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__7.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__7.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__8.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__8.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__6.r = z__7.r + z__8.r, z__6.i = z__7.i + z__8.i;
	    i__4 = c___subscr(j, 3);
	    z__9.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__9.i = v3.r * 
		    c__[i__4].i + v3.i * c__[i__4].r;
	    z__5.r = z__6.r + z__9.r, z__5.i = z__6.i + z__9.i;
	    i__5 = c___subscr(j, 4);
	    z__10.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__10.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__4.r = z__5.r + z__10.r, z__4.i = z__5.i + z__10.i;
	    i__6 = c___subscr(j, 5);
	    z__11.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__11.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__3.r = z__4.r + z__11.r, z__3.i = z__4.i + z__11.i;
	    i__7 = c___subscr(j, 6);
	    z__12.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__12.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__2.r = z__3.r + z__12.r, z__2.i = z__3.i + z__12.i;
	    i__8 = c___subscr(j, 7);
	    z__13.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__13.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__1.r = z__2.r + z__13.r, z__1.i = z__2.i + z__13.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 4);
	    i__3 = c___subscr(j, 4);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 5);
	    i__3 = c___subscr(j, 5);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 6);
	    i__3 = c___subscr(j, 6);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 7);
	    i__3 = c___subscr(j, 7);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L340: */
	}
	goto L410;
L350:

/*        Special code for 8 x 8 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	v4.r = v[4].r, v4.i = v[4].i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	v5.r = v[5].r, v5.i = v[5].i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	v6.r = v[6].r, v6.i = v[6].i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	v7.r = v[7].r, v7.i = v[7].i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	v8.r = v[8].r, v8.i = v[8].i;
	d_cnjg(&z__2, &v8);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t8.r = z__1.r, t8.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__8.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__8.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__9.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__9.i = v2.r * 
		    c__[i__3].i + v2.i * c__[i__3].r;
	    z__7.r = z__8.r + z__9.r, z__7.i = z__8.i + z__9.i;
	    i__4 = c___subscr(j, 3);
	    z__10.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__10.i = v3.r 
		    * c__[i__4].i + v3.i * c__[i__4].r;
	    z__6.r = z__7.r + z__10.r, z__6.i = z__7.i + z__10.i;
	    i__5 = c___subscr(j, 4);
	    z__11.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__11.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__5.r = z__6.r + z__11.r, z__5.i = z__6.i + z__11.i;
	    i__6 = c___subscr(j, 5);
	    z__12.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__12.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__4.r = z__5.r + z__12.r, z__4.i = z__5.i + z__12.i;
	    i__7 = c___subscr(j, 6);
	    z__13.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__13.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__3.r = z__4.r + z__13.r, z__3.i = z__4.i + z__13.i;
	    i__8 = c___subscr(j, 7);
	    z__14.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__14.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__2.r = z__3.r + z__14.r, z__2.i = z__3.i + z__14.i;
	    i__9 = c___subscr(j, 8);
	    z__15.r = v8.r * c__[i__9].r - v8.i * c__[i__9].i, z__15.i = v8.r 
		    * c__[i__9].i + v8.i * c__[i__9].r;
	    z__1.r = z__2.r + z__15.r, z__1.i = z__2.i + z__15.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 4);
	    i__3 = c___subscr(j, 4);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 5);
	    i__3 = c___subscr(j, 5);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 6);
	    i__3 = c___subscr(j, 6);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 7);
	    i__3 = c___subscr(j, 7);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 8);
	    i__3 = c___subscr(j, 8);
	    z__2.r = sum.r * t8.r - sum.i * t8.i, z__2.i = sum.r * t8.i + 
		    sum.i * t8.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L360: */
	}
	goto L410;
L370:

/*        Special code for 9 x 9 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	v4.r = v[4].r, v4.i = v[4].i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	v5.r = v[5].r, v5.i = v[5].i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	v6.r = v[6].r, v6.i = v[6].i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	v7.r = v[7].r, v7.i = v[7].i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	v8.r = v[8].r, v8.i = v[8].i;
	d_cnjg(&z__2, &v8);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t8.r = z__1.r, t8.i = z__1.i;
	v9.r = v[9].r, v9.i = v[9].i;
	d_cnjg(&z__2, &v9);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t9.r = z__1.r, t9.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__9.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__9.i = v1.r * 
		    c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__10.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__10.i = v2.r 
		    * c__[i__3].i + v2.i * c__[i__3].r;
	    z__8.r = z__9.r + z__10.r, z__8.i = z__9.i + z__10.i;
	    i__4 = c___subscr(j, 3);
	    z__11.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__11.i = v3.r 
		    * c__[i__4].i + v3.i * c__[i__4].r;
	    z__7.r = z__8.r + z__11.r, z__7.i = z__8.i + z__11.i;
	    i__5 = c___subscr(j, 4);
	    z__12.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__12.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__6.r = z__7.r + z__12.r, z__6.i = z__7.i + z__12.i;
	    i__6 = c___subscr(j, 5);
	    z__13.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__13.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__5.r = z__6.r + z__13.r, z__5.i = z__6.i + z__13.i;
	    i__7 = c___subscr(j, 6);
	    z__14.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__14.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__4.r = z__5.r + z__14.r, z__4.i = z__5.i + z__14.i;
	    i__8 = c___subscr(j, 7);
	    z__15.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__15.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__3.r = z__4.r + z__15.r, z__3.i = z__4.i + z__15.i;
	    i__9 = c___subscr(j, 8);
	    z__16.r = v8.r * c__[i__9].r - v8.i * c__[i__9].i, z__16.i = v8.r 
		    * c__[i__9].i + v8.i * c__[i__9].r;
	    z__2.r = z__3.r + z__16.r, z__2.i = z__3.i + z__16.i;
	    i__10 = c___subscr(j, 9);
	    z__17.r = v9.r * c__[i__10].r - v9.i * c__[i__10].i, z__17.i = 
		    v9.r * c__[i__10].i + v9.i * c__[i__10].r;
	    z__1.r = z__2.r + z__17.r, z__1.i = z__2.i + z__17.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 4);
	    i__3 = c___subscr(j, 4);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 5);
	    i__3 = c___subscr(j, 5);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 6);
	    i__3 = c___subscr(j, 6);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 7);
	    i__3 = c___subscr(j, 7);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 8);
	    i__3 = c___subscr(j, 8);
	    z__2.r = sum.r * t8.r - sum.i * t8.i, z__2.i = sum.r * t8.i + 
		    sum.i * t8.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 9);
	    i__3 = c___subscr(j, 9);
	    z__2.r = sum.r * t9.r - sum.i * t9.i, z__2.i = sum.r * t9.i + 
		    sum.i * t9.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L380: */
	}
	goto L410;
L390:

/*        Special code for 10 x 10 Householder */

	v1.r = v[1].r, v1.i = v[1].i;
	d_cnjg(&z__2, &v1);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t1.r = z__1.r, t1.i = z__1.i;
	v2.r = v[2].r, v2.i = v[2].i;
	d_cnjg(&z__2, &v2);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t2.r = z__1.r, t2.i = z__1.i;
	v3.r = v[3].r, v3.i = v[3].i;
	d_cnjg(&z__2, &v3);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t3.r = z__1.r, t3.i = z__1.i;
	v4.r = v[4].r, v4.i = v[4].i;
	d_cnjg(&z__2, &v4);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t4.r = z__1.r, t4.i = z__1.i;
	v5.r = v[5].r, v5.i = v[5].i;
	d_cnjg(&z__2, &v5);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t5.r = z__1.r, t5.i = z__1.i;
	v6.r = v[6].r, v6.i = v[6].i;
	d_cnjg(&z__2, &v6);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t6.r = z__1.r, t6.i = z__1.i;
	v7.r = v[7].r, v7.i = v[7].i;
	d_cnjg(&z__2, &v7);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t7.r = z__1.r, t7.i = z__1.i;
	v8.r = v[8].r, v8.i = v[8].i;
	d_cnjg(&z__2, &v8);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t8.r = z__1.r, t8.i = z__1.i;
	v9.r = v[9].r, v9.i = v[9].i;
	d_cnjg(&z__2, &v9);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t9.r = z__1.r, t9.i = z__1.i;
	v10.r = v[10].r, v10.i = v[10].i;
	d_cnjg(&z__2, &v10);
	z__1.r = tau->r * z__2.r - tau->i * z__2.i, z__1.i = tau->r * z__2.i 
		+ tau->i * z__2.r;
	t10.r = z__1.r, t10.i = z__1.i;
	i__1 = *m;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = c___subscr(j, 1);
	    z__10.r = v1.r * c__[i__2].r - v1.i * c__[i__2].i, z__10.i = v1.r 
		    * c__[i__2].i + v1.i * c__[i__2].r;
	    i__3 = c___subscr(j, 2);
	    z__11.r = v2.r * c__[i__3].r - v2.i * c__[i__3].i, z__11.i = v2.r 
		    * c__[i__3].i + v2.i * c__[i__3].r;
	    z__9.r = z__10.r + z__11.r, z__9.i = z__10.i + z__11.i;
	    i__4 = c___subscr(j, 3);
	    z__12.r = v3.r * c__[i__4].r - v3.i * c__[i__4].i, z__12.i = v3.r 
		    * c__[i__4].i + v3.i * c__[i__4].r;
	    z__8.r = z__9.r + z__12.r, z__8.i = z__9.i + z__12.i;
	    i__5 = c___subscr(j, 4);
	    z__13.r = v4.r * c__[i__5].r - v4.i * c__[i__5].i, z__13.i = v4.r 
		    * c__[i__5].i + v4.i * c__[i__5].r;
	    z__7.r = z__8.r + z__13.r, z__7.i = z__8.i + z__13.i;
	    i__6 = c___subscr(j, 5);
	    z__14.r = v5.r * c__[i__6].r - v5.i * c__[i__6].i, z__14.i = v5.r 
		    * c__[i__6].i + v5.i * c__[i__6].r;
	    z__6.r = z__7.r + z__14.r, z__6.i = z__7.i + z__14.i;
	    i__7 = c___subscr(j, 6);
	    z__15.r = v6.r * c__[i__7].r - v6.i * c__[i__7].i, z__15.i = v6.r 
		    * c__[i__7].i + v6.i * c__[i__7].r;
	    z__5.r = z__6.r + z__15.r, z__5.i = z__6.i + z__15.i;
	    i__8 = c___subscr(j, 7);
	    z__16.r = v7.r * c__[i__8].r - v7.i * c__[i__8].i, z__16.i = v7.r 
		    * c__[i__8].i + v7.i * c__[i__8].r;
	    z__4.r = z__5.r + z__16.r, z__4.i = z__5.i + z__16.i;
	    i__9 = c___subscr(j, 8);
	    z__17.r = v8.r * c__[i__9].r - v8.i * c__[i__9].i, z__17.i = v8.r 
		    * c__[i__9].i + v8.i * c__[i__9].r;
	    z__3.r = z__4.r + z__17.r, z__3.i = z__4.i + z__17.i;
	    i__10 = c___subscr(j, 9);
	    z__18.r = v9.r * c__[i__10].r - v9.i * c__[i__10].i, z__18.i = 
		    v9.r * c__[i__10].i + v9.i * c__[i__10].r;
	    z__2.r = z__3.r + z__18.r, z__2.i = z__3.i + z__18.i;
	    i__11 = c___subscr(j, 10);
	    z__19.r = v10.r * c__[i__11].r - v10.i * c__[i__11].i, z__19.i = 
		    v10.r * c__[i__11].i + v10.i * c__[i__11].r;
	    z__1.r = z__2.r + z__19.r, z__1.i = z__2.i + z__19.i;
	    sum.r = z__1.r, sum.i = z__1.i;
	    i__2 = c___subscr(j, 1);
	    i__3 = c___subscr(j, 1);
	    z__2.r = sum.r * t1.r - sum.i * t1.i, z__2.i = sum.r * t1.i + 
		    sum.i * t1.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 2);
	    i__3 = c___subscr(j, 2);
	    z__2.r = sum.r * t2.r - sum.i * t2.i, z__2.i = sum.r * t2.i + 
		    sum.i * t2.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 3);
	    i__3 = c___subscr(j, 3);
	    z__2.r = sum.r * t3.r - sum.i * t3.i, z__2.i = sum.r * t3.i + 
		    sum.i * t3.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 4);
	    i__3 = c___subscr(j, 4);
	    z__2.r = sum.r * t4.r - sum.i * t4.i, z__2.i = sum.r * t4.i + 
		    sum.i * t4.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 5);
	    i__3 = c___subscr(j, 5);
	    z__2.r = sum.r * t5.r - sum.i * t5.i, z__2.i = sum.r * t5.i + 
		    sum.i * t5.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 6);
	    i__3 = c___subscr(j, 6);
	    z__2.r = sum.r * t6.r - sum.i * t6.i, z__2.i = sum.r * t6.i + 
		    sum.i * t6.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 7);
	    i__3 = c___subscr(j, 7);
	    z__2.r = sum.r * t7.r - sum.i * t7.i, z__2.i = sum.r * t7.i + 
		    sum.i * t7.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 8);
	    i__3 = c___subscr(j, 8);
	    z__2.r = sum.r * t8.r - sum.i * t8.i, z__2.i = sum.r * t8.i + 
		    sum.i * t8.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 9);
	    i__3 = c___subscr(j, 9);
	    z__2.r = sum.r * t9.r - sum.i * t9.i, z__2.i = sum.r * t9.i + 
		    sum.i * t9.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
	    i__2 = c___subscr(j, 10);
	    i__3 = c___subscr(j, 10);
	    z__2.r = sum.r * t10.r - sum.i * t10.i, z__2.i = sum.r * t10.i + 
		    sum.i * t10.r;
	    z__1.r = c__[i__3].r - z__2.r, z__1.i = c__[i__3].i - z__2.i;
	    c__[i__2].r = z__1.r, c__[i__2].i = z__1.i;
/* L400: */
	}
	goto L410;
    }
L410:
    return 0;

/*     End of ZLARFX */

} /* zlarfx_ */

#undef c___ref
#undef c___subscr


