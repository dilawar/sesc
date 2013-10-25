#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dptcon_(integer *n, doublereal *d__, doublereal *e, 
	doublereal *anorm, doublereal *rcond, doublereal *work, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    DPTCON computes the reciprocal of the condition number (in the   
    1-norm) of a real symmetric positive definite tridiagonal matrix   
    using the factorization A = L*D*L**T or A = U**T*D*U computed by   
    DPTTRF.   

    Norm(inv(A)) is computed by a direct method, and the reciprocal of   
    the condition number is computed as   
                 RCOND = 1 / (ANORM * norm(inv(A))).   

    Arguments   
    =========   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    D       (input) DOUBLE PRECISION array, dimension (N)   
            The n diagonal elements of the diagonal matrix D from the   
            factorization of A, as computed by DPTTRF.   

    E       (input) DOUBLE PRECISION array, dimension (N-1)   
            The (n-1) off-diagonal elements of the unit bidiagonal factor   
            U or L from the factorization of A,  as computed by DPTTRF.   

    ANORM   (input) DOUBLE PRECISION   
            The 1-norm of the original matrix A.   

    RCOND   (output) DOUBLE PRECISION   
            The reciprocal of the condition number of the matrix A,   
            computed as RCOND = 1/(ANORM * AINVNM), where AINVNM is the   
            1-norm of inv(A) computed in this routine.   

    WORK    (workspace) DOUBLE PRECISION array, dimension (N)   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The method used is described in Nicholas J. Higham, "Efficient   
    Algorithms for Computing the Condition Number of a Tridiagonal   
    Matrix", SIAM J. Sci. Stat. Comput., Vol. 7, No. 1, January 1986.   

    =====================================================================   


       Test the input arguments.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer i__1;
    doublereal d__1;
    /* Local variables */
    static integer i__, ix;
    extern integer idamax_(integer *, doublereal *, integer *);
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static doublereal ainvnm;


    --work;
    --e;
    --d__;

    /* Function Body */
    *info = 0;
    if (*n < 0) {
	*info = -1;
    } else if (*anorm < 0.) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DPTCON", &i__1);
	return 0;
    }

/*     Quick return if possible */

    *rcond = 0.;
    if (*n == 0) {
	*rcond = 1.;
	return 0;
    } else if (*anorm == 0.) {
	return 0;
    }

/*     Check that D(1:N) is positive. */

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (d__[i__] <= 0.) {
	    return 0;
	}
/* L10: */
    }

/*     Solve M(A) * x = e, where M(A) = (m(i,j)) is given by   

          m(i,j) =  abs(A(i,j)), i = j,   
          m(i,j) = -abs(A(i,j)), i .ne. j,   

       and e = [ 1, 1, ..., 1 ]'.  Note M(A) = M(L)*D*M(L)'.   

       Solve M(L) * x = e. */

    work[1] = 1.;
    i__1 = *n;
    for (i__ = 2; i__ <= i__1; ++i__) {
	work[i__] = work[i__ - 1] * (d__1 = e[i__ - 1], abs(d__1)) + 1.;
/* L20: */
    }

/*     Solve D * M(L)' * x = b. */

    work[*n] /= d__[*n];
    for (i__ = *n - 1; i__ >= 1; --i__) {
	work[i__] = work[i__] / d__[i__] + work[i__ + 1] * (d__1 = e[i__], 
		abs(d__1));
/* L30: */
    }

/*     Compute AINVNM = max(x(i)), 1<=i<=n. */

    ix = idamax_(n, &work[1], &c__1);
    ainvnm = (d__1 = work[ix], abs(d__1));

/*     Compute the reciprocal condition number. */

    if (ainvnm != 0.) {
	*rcond = 1. / ainvnm / *anorm;
    }

    return 0;

/*     End of DPTCON */

} /* dptcon_ */

