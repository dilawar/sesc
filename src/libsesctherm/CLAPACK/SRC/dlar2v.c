#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dlar2v_(integer *n, doublereal *x, doublereal *y, 
	doublereal *z__, integer *incx, doublereal *c__, doublereal *s, 
	integer *incc)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DLAR2V applies a vector of real plane rotations from both sides to   
    a sequence of 2-by-2 real symmetric matrices, defined by the elements   
    of the vectors x, y and z. For i = 1,2,...,n   

       ( x(i)  z(i) ) := (  c(i)  s(i) ) ( x(i)  z(i) ) ( c(i) -s(i) )   
       ( z(i)  y(i) )    ( -s(i)  c(i) ) ( z(i)  y(i) ) ( s(i)  c(i) )   

    Arguments   
    =========   

    N       (input) INTEGER   
            The number of plane rotations to be applied.   

    X       (input/output) DOUBLE PRECISION array,   
                           dimension (1+(N-1)*INCX)   
            The vector x.   

    Y       (input/output) DOUBLE PRECISION array,   
                           dimension (1+(N-1)*INCX)   
            The vector y.   

    Z       (input/output) DOUBLE PRECISION array,   
                           dimension (1+(N-1)*INCX)   
            The vector z.   

    INCX    (input) INTEGER   
            The increment between elements of X, Y and Z. INCX > 0.   

    C       (input) DOUBLE PRECISION array, dimension (1+(N-1)*INCC)   
            The cosines of the plane rotations.   

    S       (input) DOUBLE PRECISION array, dimension (1+(N-1)*INCC)   
            The sines of the plane rotations.   

    INCC    (input) INTEGER   
            The increment between elements of C and S. INCC > 0.   

    =====================================================================   


       Parameter adjustments */
    /* System generated locals */
    integer i__1;
    /* Local variables */
    static integer i__;
    static doublereal t1, t2, t3, t4, t5, t6;
    static integer ic;
    static doublereal ci, si;
    static integer ix;
    static doublereal xi, yi, zi;

    --s;
    --c__;
    --z__;
    --y;
    --x;

    /* Function Body */
    ix = 1;
    ic = 1;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	xi = x[ix];
	yi = y[ix];
	zi = z__[ix];
	ci = c__[ic];
	si = s[ic];
	t1 = si * zi;
	t2 = ci * zi;
	t3 = t2 - si * xi;
	t4 = t2 + si * yi;
	t5 = ci * xi + t1;
	t6 = ci * yi - t1;
	x[ix] = ci * t5 + si * t4;
	y[ix] = ci * t6 - si * t3;
	z__[ix] = ci * t4 - si * t5;
	ix += *incx;
	ic += *incc;
/* L10: */
    }

/*     End of DLAR2V */

    return 0;
} /* dlar2v_ */

