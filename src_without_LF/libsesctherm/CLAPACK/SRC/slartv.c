#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int slartv_(integer *n, real *x, integer *incx, real *y, 
	integer *incy, real *c__, real *s, integer *incc)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    SLARTV applies a vector of real plane rotations to elements of the   
    real vectors x and y. For i = 1,2,...,n   

       ( x(i) ) := (  c(i)  s(i) ) ( x(i) )   
       ( y(i) )    ( -s(i)  c(i) ) ( y(i) )   

    Arguments   
    =========   

    N       (input) INTEGER   
            The number of plane rotations to be applied.   

    X       (input/output) REAL array,   
                           dimension (1+(N-1)*INCX)   
            The vector x.   

    INCX    (input) INTEGER   
            The increment between elements of X. INCX > 0.   

    Y       (input/output) REAL array,   
                           dimension (1+(N-1)*INCY)   
            The vector y.   

    INCY    (input) INTEGER   
            The increment between elements of Y. INCY > 0.   

    C       (input) REAL array, dimension (1+(N-1)*INCC)   
            The cosines of the plane rotations.   

    S       (input) REAL array, dimension (1+(N-1)*INCC)   
            The sines of the plane rotations.   

    INCC    (input) INTEGER   
            The increment between elements of C and S. INCC > 0.   

    =====================================================================   


       Parameter adjustments */
    /* System generated locals */
    integer i__1;
    /* Local variables */
    static integer i__, ic, ix, iy;
    static real xi, yi;

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
	xi = x[ix];
	yi = y[iy];
	x[ix] = c__[ic] * xi + s[ic] * yi;
	y[iy] = c__[ic] * yi - s[ic] * xi;
	ix += *incx;
	iy += *incy;
	ic += *incc;
/* L10: */
    }
    return 0;

/*     End of SLARTV */

} /* slartv_ */

