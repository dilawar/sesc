#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dlacon_(integer *n, doublereal *v, doublereal *x, 
	integer *isgn, doublereal *est, integer *kase)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DLACON estimates the 1-norm of a square, real matrix A.   
    Reverse communication is used for evaluating matrix-vector products.   

    Arguments   
    =========   

    N      (input) INTEGER   
           The order of the matrix.  N >= 1.   

    V      (workspace) DOUBLE PRECISION array, dimension (N)   
           On the final return, V = A*W,  where  EST = norm(V)/norm(W)   
           (W is not returned).   

    X      (input/output) DOUBLE PRECISION array, dimension (N)   
           On an intermediate return, X should be overwritten by   
                 A * X,   if KASE=1,   
                 A' * X,  if KASE=2,   
           and DLACON must be re-called with all the other parameters   
           unchanged.   

    ISGN   (workspace) INTEGER array, dimension (N)   

    EST    (output) DOUBLE PRECISION   
           An estimate (a lower bound) for norm(A).   

    KASE   (input/output) INTEGER   
           On the initial call to DLACON, KASE should be 0.   
           On an intermediate return, KASE will be 1 or 2, indicating   
           whether X should be overwritten by A * X  or A' * X.   
           On the final return from DLACON, KASE will again be 0.   

    Further Details   
    ======= =======   

    Contributed by Nick Higham, University of Manchester.   
    Originally named SONEST, dated March 16, 1988.   

    Reference: N.J. Higham, "FORTRAN codes for estimating the one-norm of   
    a real or complex matrix, with applications to condition estimation",   
    ACM Trans. Math. Soft., vol. 14, no. 4, pp. 381-396, December 1988.   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static doublereal c_b11 = 1.;
    
    /* System generated locals */
    integer i__1;
    doublereal d__1;
    /* Builtin functions */
    double d_sign(doublereal *, doublereal *);
    integer i_dnnt(doublereal *);
    /* Local variables */
    static integer iter;
    static doublereal temp;
    static integer jump, i__, j;
    extern doublereal dasum_(integer *, doublereal *, integer *);
    static integer jlast;
    extern /* Subroutine */ int dcopy_(integer *, doublereal *, integer *, 
	    doublereal *, integer *);
    extern integer idamax_(integer *, doublereal *, integer *);
    static doublereal altsgn, estold;


    --isgn;
    --x;
    --v;

    /* Function Body */
    if (*kase == 0) {
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    x[i__] = 1. / (doublereal) (*n);
/* L10: */
	}
	*kase = 1;
	jump = 1;
	return 0;
    }

    switch (jump) {
	case 1:  goto L20;
	case 2:  goto L40;
	case 3:  goto L70;
	case 4:  goto L110;
	case 5:  goto L140;
    }

/*     ................ ENTRY   (JUMP = 1)   
       FIRST ITERATION.  X HAS BEEN OVERWRITTEN BY A*X. */

L20:
    if (*n == 1) {
	v[1] = x[1];
	*est = abs(v[1]);
/*        ... QUIT */
	goto L150;
    }
    *est = dasum_(n, &x[1], &c__1);

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	x[i__] = d_sign(&c_b11, &x[i__]);
	isgn[i__] = i_dnnt(&x[i__]);
/* L30: */
    }
    *kase = 2;
    jump = 2;
    return 0;

/*     ................ ENTRY   (JUMP = 2)   
       FIRST ITERATION.  X HAS BEEN OVERWRITTEN BY TRANDPOSE(A)*X. */

L40:
    j = idamax_(n, &x[1], &c__1);
    iter = 2;

/*     MAIN LOOP - ITERATIONS 2,3,...,ITMAX. */

L50:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	x[i__] = 0.;
/* L60: */
    }
    x[j] = 1.;
    *kase = 1;
    jump = 3;
    return 0;

/*     ................ ENTRY   (JUMP = 3)   
       X HAS BEEN OVERWRITTEN BY A*X. */

L70:
    dcopy_(n, &x[1], &c__1, &v[1], &c__1);
    estold = *est;
    *est = dasum_(n, &v[1], &c__1);
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	d__1 = d_sign(&c_b11, &x[i__]);
	if (i_dnnt(&d__1) != isgn[i__]) {
	    goto L90;
	}
/* L80: */
    }
/*     REPEATED SIGN VECTOR DETECTED, HENCE ALGORITHM HAS CONVERGED. */
    goto L120;

L90:
/*     TEST FOR CYCLING. */
    if (*est <= estold) {
	goto L120;
    }

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	x[i__] = d_sign(&c_b11, &x[i__]);
	isgn[i__] = i_dnnt(&x[i__]);
/* L100: */
    }
    *kase = 2;
    jump = 4;
    return 0;

/*     ................ ENTRY   (JUMP = 4)   
       X HAS BEEN OVERWRITTEN BY TRANDPOSE(A)*X. */

L110:
    jlast = j;
    j = idamax_(n, &x[1], &c__1);
    if (x[jlast] != (d__1 = x[j], abs(d__1)) && iter < 5) {
	++iter;
	goto L50;
    }

/*     ITERATION COMPLETE.  FINAL STAGE. */

L120:
    altsgn = 1.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	x[i__] = altsgn * ((doublereal) (i__ - 1) / (doublereal) (*n - 1) + 
		1.);
	altsgn = -altsgn;
/* L130: */
    }
    *kase = 1;
    jump = 5;
    return 0;

/*     ................ ENTRY   (JUMP = 5)   
       X HAS BEEN OVERWRITTEN BY A*X. */

L140:
    temp = dasum_(n, &x[1], &c__1) / (doublereal) (*n * 3) * 2.;
    if (temp > *est) {
	dcopy_(n, &x[1], &c__1, &v[1], &c__1);
	*est = temp;
    }

L150:
    *kase = 0;
    return 0;

/*     End of DLACON */

} /* dlacon_ */

