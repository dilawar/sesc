#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zlapll_(integer *n, doublecomplex *x, integer *incx, 
	doublecomplex *y, integer *incy, doublereal *ssmin)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    Given two column vectors X and Y, let   

                         A = ( X Y ).   

    The subroutine first computes the QR factorization of A = Q*R,   
    and then computes the SVD of the 2-by-2 upper triangular matrix R.   
    The smaller singular value of R is returned in SSMIN, which is used   
    as the measurement of the linear dependency of the vectors X and Y.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The length of the vectors X and Y.   

    X       (input/output) COMPLEX*16 array, dimension (1+(N-1)*INCX)   
            On entry, X contains the N-vector X.   
            On exit, X is overwritten.   

    INCX    (input) INTEGER   
            The increment between successive elements of X. INCX > 0.   

    Y       (input/output) COMPLEX*16 array, dimension (1+(N-1)*INCY)   
            On entry, Y contains the N-vector Y.   
            On exit, Y is overwritten.   

    INCY    (input) INTEGER   
            The increment between successive elements of Y. INCY > 0.   

    SSMIN   (output) DOUBLE PRECISION   
            The smallest singular value of the N-by-2 matrix A = ( X Y ).   

    =====================================================================   


       Quick return if possible   

       Parameter adjustments */
    /* System generated locals */
    integer i__1;
    doublereal d__1, d__2, d__3;
    doublecomplex z__1, z__2, z__3, z__4;
    /* Builtin functions */
    void d_cnjg(doublecomplex *, doublecomplex *);
    double z_abs(doublecomplex *);
    /* Local variables */
    extern /* Subroutine */ int dlas2_(doublereal *, doublereal *, doublereal 
	    *, doublereal *, doublereal *);
    static doublecomplex c__;
    extern /* Double Complex */ VOID zdotc_(doublecomplex *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *);
    static doublereal ssmax;
    extern /* Subroutine */ int zaxpy_(integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *);
    static doublecomplex a11, a12, a22;
    extern /* Subroutine */ int zlarfg_(integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *);
    static doublecomplex tau;

    --y;
    --x;

    /* Function Body */
    if (*n <= 1) {
	*ssmin = 0.;
	return 0;
    }

/*     Compute the QR factorization of the N-by-2 matrix ( X Y ) */

    zlarfg_(n, &x[1], &x[*incx + 1], incx, &tau);
    a11.r = x[1].r, a11.i = x[1].i;
    x[1].r = 1., x[1].i = 0.;

    d_cnjg(&z__3, &tau);
    z__2.r = -z__3.r, z__2.i = -z__3.i;
    zdotc_(&z__4, n, &x[1], incx, &y[1], incy);
    z__1.r = z__2.r * z__4.r - z__2.i * z__4.i, z__1.i = z__2.r * z__4.i + 
	    z__2.i * z__4.r;
    c__.r = z__1.r, c__.i = z__1.i;
    zaxpy_(n, &c__, &x[1], incx, &y[1], incy);

    i__1 = *n - 1;
    zlarfg_(&i__1, &y[*incy + 1], &y[(*incy << 1) + 1], incy, &tau);

    a12.r = y[1].r, a12.i = y[1].i;
    i__1 = *incy + 1;
    a22.r = y[i__1].r, a22.i = y[i__1].i;

/*     Compute the SVD of 2-by-2 Upper triangular matrix. */

    d__1 = z_abs(&a11);
    d__2 = z_abs(&a12);
    d__3 = z_abs(&a22);
    dlas2_(&d__1, &d__2, &d__3, ssmin, &ssmax);

    return 0;

/*     End of ZLAPLL */

} /* zlapll_ */

