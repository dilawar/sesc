#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zdrscl_(integer *n, doublereal *sa, doublecomplex *sx, 
	integer *incx)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZDRSCL multiplies an n-element complex vector x by the real scalar   
    1/a.  This is done without overflow or underflow as long as   
    the final result x/a does not overflow or underflow.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The number of components of the vector x.   

    SA      (input) DOUBLE PRECISION   
            The scalar a which is used to divide each component of x.   
            SA must be >= 0, or the subroutine will divide by zero.   

    SX      (input/output) COMPLEX*16 array, dimension   
                           (1+(N-1)*abs(INCX))   
            The n-element vector x.   

    INCX    (input) INTEGER   
            The increment between successive values of the vector SX.   
            > 0:  SX(1) = X(1) and SX(1+(i-1)*INCX) = x(i),     1< i<= n   

   =====================================================================   


       Quick return if possible   

       Parameter adjustments */
    static doublereal cden;
    static logical done;
    static doublereal cnum, cden1, cnum1;
    extern /* Subroutine */ int dlabad_(doublereal *, doublereal *);
    extern doublereal dlamch_(char *);
    extern /* Subroutine */ int zdscal_(integer *, doublereal *, 
	    doublecomplex *, integer *);
    static doublereal bignum, smlnum, mul;

    --sx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }

/*     Get machine parameters */

    smlnum = dlamch_("S");
    bignum = 1. / smlnum;
    dlabad_(&smlnum, &bignum);

/*     Initialize the denominator to SA and the numerator to 1. */

    cden = *sa;
    cnum = 1.;

L10:
    cden1 = cden * smlnum;
    cnum1 = cnum / bignum;
    if (abs(cden1) > abs(cnum) && cnum != 0.) {

/*        Pre-multiply X by SMLNUM if CDEN is large compared to CNUM. */

	mul = smlnum;
	done = FALSE_;
	cden = cden1;
    } else if (abs(cnum1) > abs(cden)) {

/*        Pre-multiply X by BIGNUM if CDEN is small compared to CNUM. */

	mul = bignum;
	done = FALSE_;
	cnum = cnum1;
    } else {

/*        Multiply X by CNUM / CDEN and return. */

	mul = cnum / cden;
	done = TRUE_;
    }

/*     Scale the vector X by MUL */

    zdscal_(n, &mul, &sx[1], incx);

    if (! done) {
	goto L10;
    }

    return 0;

/*     End of ZDRSCL */

} /* zdrscl_ */

