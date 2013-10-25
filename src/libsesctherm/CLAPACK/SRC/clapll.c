#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int clapll_(integer *n, complex *x, integer *incx, complex *
	y, integer *incy, real *ssmin)
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

    X       (input/output) COMPLEX array, dimension (1+(N-1)*INCX)   
            On entry, X contains the N-vector X.   
            On exit, X is overwritten.   

    INCX    (input) INTEGER   
            The increment between successive elements of X. INCX > 0.   

    Y       (input/output) COMPLEX array, dimension (1+(N-1)*INCY)   
            On entry, Y contains the N-vector Y.   
            On exit, Y is overwritten.   

    INCY    (input) INTEGER   
            The increment between successive elements of Y. INCY > 0.   

    SSMIN   (output) REAL   
            The smallest singular value of the N-by-2 matrix A = ( X Y ).   

    =====================================================================   


       Quick return if possible   

       Parameter adjustments */
    /* System generated locals */
    integer i__1;
    real r__1, r__2, r__3;
    complex q__1, q__2, q__3, q__4;
    /* Builtin functions */
    void r_cnjg(complex *, complex *);
    double c_abs(complex *);
    /* Local variables */
    extern /* Subroutine */ int slas2_(real *, real *, real *, real *, real *)
	    ;
    static complex c__;
    extern /* Complex */ VOID cdotc_(complex *, integer *, complex *, integer 
	    *, complex *, integer *);
    extern /* Subroutine */ int caxpy_(integer *, complex *, complex *, 
	    integer *, complex *, integer *);
    static real ssmax;
    static complex a11, a12, a22;
    extern /* Subroutine */ int clarfg_(integer *, complex *, complex *, 
	    integer *, complex *);
    static complex tau;

    --y;
    --x;

    /* Function Body */
    if (*n <= 1) {
	*ssmin = 0.f;
	return 0;
    }

/*     Compute the QR factorization of the N-by-2 matrix ( X Y ) */

    clarfg_(n, &x[1], &x[*incx + 1], incx, &tau);
    a11.r = x[1].r, a11.i = x[1].i;
    x[1].r = 1.f, x[1].i = 0.f;

    r_cnjg(&q__3, &tau);
    q__2.r = -q__3.r, q__2.i = -q__3.i;
    cdotc_(&q__4, n, &x[1], incx, &y[1], incy);
    q__1.r = q__2.r * q__4.r - q__2.i * q__4.i, q__1.i = q__2.r * q__4.i + 
	    q__2.i * q__4.r;
    c__.r = q__1.r, c__.i = q__1.i;
    caxpy_(n, &c__, &x[1], incx, &y[1], incy);

    i__1 = *n - 1;
    clarfg_(&i__1, &y[*incy + 1], &y[(*incy << 1) + 1], incy, &tau);

    a12.r = y[1].r, a12.i = y[1].i;
    i__1 = *incy + 1;
    a22.r = y[i__1].r, a22.i = y[i__1].i;

/*     Compute the SVD of 2-by-2 Upper triangular matrix. */

    r__1 = c_abs(&a11);
    r__2 = c_abs(&a12);
    r__3 = c_abs(&a22);
    slas2_(&r__1, &r__2, &r__3, ssmin, &ssmax);

    return 0;

/*     End of CLAPLL */

} /* clapll_ */

