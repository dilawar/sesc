#include "blaswrap.h"
#include "f2c.h"

doublereal clanht_(char *norm, integer *n, real *d__, complex *e)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1992   


    Purpose   
    =======   

    CLANHT  returns the value of the one norm,  or the Frobenius norm, or   
    the  infinity norm,  or the  element of  largest absolute value  of a   
    complex Hermitian tridiagonal matrix A.   

    Description   
    ===========   

    CLANHT returns the value   

       CLANHT = ( max(abs(A(i,j))), NORM = 'M' or 'm'   
                (   
                ( norm1(A),         NORM = '1', 'O' or 'o'   
                (   
                ( normI(A),         NORM = 'I' or 'i'   
                (   
                ( normF(A),         NORM = 'F', 'f', 'E' or 'e'   

    where  norm1  denotes the  one norm of a matrix (maximum column sum),   
    normI  denotes the  infinity norm  of a matrix  (maximum row sum) and   
    normF  denotes the  Frobenius norm of a matrix (square root of sum of   
    squares).  Note that  max(abs(A(i,j)))  is not a  matrix norm.   

    Arguments   
    =========   

    NORM    (input) CHARACTER*1   
            Specifies the value to be returned in CLANHT as described   
            above.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.  When N = 0, CLANHT is   
            set to zero.   

    D       (input) REAL array, dimension (N)   
            The diagonal elements of A.   

    E       (input) COMPLEX array, dimension (N-1)   
            The (n-1) sub-diagonal or super-diagonal elements of A.   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer i__1;
    real ret_val, r__1, r__2, r__3;
    /* Builtin functions */
    double c_abs(complex *), sqrt(doublereal);
    /* Local variables */
    static integer i__;
    static real scale;
    extern logical lsame_(char *, char *);
    static real anorm;
    extern /* Subroutine */ int classq_(integer *, complex *, integer *, real 
	    *, real *), slassq_(integer *, real *, integer *, real *, real *);
    static real sum;


    --e;
    --d__;

    /* Function Body */
    if (*n <= 0) {
	anorm = 0.f;
    } else if (lsame_(norm, "M")) {

/*        Find max(abs(A(i,j))). */

	anorm = (r__1 = d__[*n], dabs(r__1));
	i__1 = *n - 1;
	for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing MAX */
	    r__2 = anorm, r__3 = (r__1 = d__[i__], dabs(r__1));
	    anorm = dmax(r__2,r__3);
/* Computing MAX */
	    r__1 = anorm, r__2 = c_abs(&e[i__]);
	    anorm = dmax(r__1,r__2);
/* L10: */
	}
    } else if (lsame_(norm, "O") || *(unsigned char *)
	    norm == '1' || lsame_(norm, "I")) {

/*        Find norm1(A). */

	if (*n == 1) {
	    anorm = dabs(d__[1]);
	} else {
/* Computing MAX */
	    r__2 = dabs(d__[1]) + c_abs(&e[1]), r__3 = c_abs(&e[*n - 1]) + (
		    r__1 = d__[*n], dabs(r__1));
	    anorm = dmax(r__2,r__3);
	    i__1 = *n - 1;
	    for (i__ = 2; i__ <= i__1; ++i__) {
/* Computing MAX */
		r__2 = anorm, r__3 = (r__1 = d__[i__], dabs(r__1)) + c_abs(&e[
			i__]) + c_abs(&e[i__ - 1]);
		anorm = dmax(r__2,r__3);
/* L20: */
	    }
	}
    } else if (lsame_(norm, "F") || lsame_(norm, "E")) {

/*        Find normF(A). */

	scale = 0.f;
	sum = 1.f;
	if (*n > 1) {
	    i__1 = *n - 1;
	    classq_(&i__1, &e[1], &c__1, &scale, &sum);
	    sum *= 2;
	}
	slassq_(n, &d__[1], &c__1, &scale, &sum);
	anorm = scale * sqrt(sum);
    }

    ret_val = anorm;
    return ret_val;

/*     End of CLANHT */

} /* clanht_ */

