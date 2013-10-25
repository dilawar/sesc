#include "blaswrap.h"
#include "f2c.h"

integer izmax1_(integer *n, doublecomplex *cx, integer *incx)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    IZMAX1 finds the index of the element whose real part has maximum   
    absolute value.   

    Based on IZAMAX from Level 1 BLAS.   
    The change is to use the 'genuine' absolute value.   

    Contributed by Nick Higham for use with ZLACON.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The number of elements in the vector CX.   

    CX      (input) COMPLEX*16 array, dimension (N)   
            The vector whose elements will be summed.   

    INCX    (input) INTEGER   
            The spacing between successive values of CX.  INCX >= 1.   

   =====================================================================   


       NEXT LINE IS THE ONLY MODIFICATION.   

       Parameter adjustments */
    /* System generated locals */
    integer ret_val, i__1;
    /* Builtin functions */
    double z_abs(doublecomplex *);
    /* Local variables */
    static doublereal smax;
    static integer i__, ix;

    --cx;

    /* Function Body */
    ret_val = 0;
    if (*n < 1) {
	return ret_val;
    }
    ret_val = 1;
    if (*n == 1) {
	return ret_val;
    }
    if (*incx == 1) {
	goto L30;
    }

/*     CODE FOR INCREMENT NOT EQUAL TO 1 */

    ix = 1;
    smax = z_abs(&cx[1]);
    ix += *incx;
    i__1 = *n;
    for (i__ = 2; i__ <= i__1; ++i__) {
	if (z_abs(&cx[ix]) <= smax) {
	    goto L10;
	}
	ret_val = i__;
	smax = z_abs(&cx[ix]);
L10:
	ix += *incx;
/* L20: */
    }
    return ret_val;

/*     CODE FOR INCREMENT EQUAL TO 1 */

L30:
    smax = z_abs(&cx[1]);
    i__1 = *n;
    for (i__ = 2; i__ <= i__1; ++i__) {
	if (z_abs(&cx[i__]) <= smax) {
	    goto L40;
	}
	ret_val = i__;
	smax = z_abs(&cx[i__]);
L40:
	;
    }
    return ret_val;

/*     End of IZMAX1 */

} /* izmax1_ */

