#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int clartv_(integer *n, complex *x, integer *incx, complex *
	y, integer *incy, real *c__, complex *s, integer *incc)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    CLARTV applies a vector of complex plane rotations with real cosines   
    to elements of the complex vectors x and y. For i = 1,2,...,n   

       ( x(i) ) := (        c(i)   s(i) ) ( x(i) )   
       ( y(i) )    ( -conjg(s(i))  c(i) ) ( y(i) )   

    Arguments   
    =========   

    N       (input) INTEGER   
            The number of plane rotations to be applied.   

    X       (input/output) COMPLEX array, dimension (1+(N-1)*INCX)   
            The vector x.   

    INCX    (input) INTEGER   
            The increment between elements of X. INCX > 0.   

    Y       (input/output) COMPLEX array, dimension (1+(N-1)*INCY)   
            The vector y.   

    INCY    (input) INTEGER   
            The increment between elements of Y. INCY > 0.   

    C       (input) REAL array, dimension (1+(N-1)*INCC)   
            The cosines of the plane rotations.   

    S       (input) COMPLEX array, dimension (1+(N-1)*INCC)   
            The sines of the plane rotations.   

    INCC    (input) INTEGER   
            The increment between elements of C and S. INCC > 0.   

    =====================================================================   


       Parameter adjustments */
    /* System generated locals */
    integer i__1, i__2, i__3, i__4;
    complex q__1, q__2, q__3, q__4;
    /* Builtin functions */
    void r_cnjg(complex *, complex *);
    /* Local variables */
    static integer i__, ic, ix, iy;
    static complex xi, yi;

    --s;
    --c__;
    --y;
    --x;

    /* Function Body */
    ix = 1;
    iy = 1;
    ic = 1;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = ix;
	xi.r = x[i__2].r, xi.i = x[i__2].i;
	i__2 = iy;
	yi.r = y[i__2].r, yi.i = y[i__2].i;
	i__2 = ix;
	i__3 = ic;
	q__2.r = c__[i__3] * xi.r, q__2.i = c__[i__3] * xi.i;
	i__4 = ic;
	q__3.r = s[i__4].r * yi.r - s[i__4].i * yi.i, q__3.i = s[i__4].r * 
		yi.i + s[i__4].i * yi.r;
	q__1.r = q__2.r + q__3.r, q__1.i = q__2.i + q__3.i;
	x[i__2].r = q__1.r, x[i__2].i = q__1.i;
	i__2 = iy;
	i__3 = ic;
	q__2.r = c__[i__3] * yi.r, q__2.i = c__[i__3] * yi.i;
	r_cnjg(&q__4, &s[ic]);
	q__3.r = q__4.r * xi.r - q__4.i * xi.i, q__3.i = q__4.r * xi.i + 
		q__4.i * xi.r;
	q__1.r = q__2.r - q__3.r, q__1.i = q__2.i - q__3.i;
	y[i__2].r = q__1.r, y[i__2].i = q__1.i;
	ix += *incx;
	iy += *incy;
	ic += *incc;
/* L10: */
    }
    return 0;

/*     End of CLARTV */

} /* clartv_ */

