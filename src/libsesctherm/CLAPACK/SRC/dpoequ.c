#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dpoequ_(integer *n, doublereal *a, integer *lda, 
	doublereal *s, doublereal *scond, doublereal *amax, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    DPOEQU computes row and column scalings intended to equilibrate a   
    symmetric positive definite matrix A and reduce its condition number   
    (with respect to the two-norm).  S contains the scale factors,   
    S(i) = 1/sqrt(A(i,i)), chosen so that the scaled matrix B with   
    elements B(i,j) = S(i)*A(i,j)*S(j) has ones on the diagonal.  This   
    choice of S puts the condition number of B within a factor N of the   
    smallest possible condition number over all possible diagonal   
    scalings.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    A       (input) DOUBLE PRECISION array, dimension (LDA,N)   
            The N-by-N symmetric positive definite matrix whose scaling   
            factors are to be computed.  Only the diagonal elements of A   
            are referenced.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,N).   

    S       (output) DOUBLE PRECISION array, dimension (N)   
            If INFO = 0, S contains the scale factors for A.   

    SCOND   (output) DOUBLE PRECISION   
            If INFO = 0, S contains the ratio of the smallest S(i) to   
            the largest S(i).  If SCOND >= 0.1 and AMAX is neither too   
            large nor too small, it is not worth scaling by S.   

    AMAX    (output) DOUBLE PRECISION   
            Absolute value of largest matrix element.  If AMAX is very   
            close to overflow or very close to underflow, the matrix   
            should be scaled.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  if INFO = i, the i-th diagonal element is nonpositive.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* System generated locals */
    integer a_dim1, a_offset, i__1;
    doublereal d__1, d__2;
    /* Builtin functions */
    double sqrt(doublereal);
    /* Local variables */
    static doublereal smin;
    static integer i__;
    extern /* Subroutine */ int xerbla_(char *, integer *);
#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]

    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --s;

    /* Function Body */
    *info = 0;
    if (*n < 0) {
	*info = -1;
    } else if (*lda < max(1,*n)) {
	*info = -3;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DPOEQU", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	*scond = 1.;
	*amax = 0.;
	return 0;
    }

/*     Find the minimum and maximum diagonal elements. */

    s[1] = a_ref(1, 1);
    smin = s[1];
    *amax = s[1];
    i__1 = *n;
    for (i__ = 2; i__ <= i__1; ++i__) {
	s[i__] = a_ref(i__, i__);
/* Computing MIN */
	d__1 = smin, d__2 = s[i__];
	smin = min(d__1,d__2);
/* Computing MAX */
	d__1 = *amax, d__2 = s[i__];
	*amax = max(d__1,d__2);
/* L10: */
    }

    if (smin <= 0.) {

/*        Find the first non-positive diagonal element and return. */

	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    if (s[i__] <= 0.) {
		*info = i__;
		return 0;
	    }
/* L20: */
	}
    } else {

/*        Set the scale factors to the reciprocals   
          of the diagonal elements. */

	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    s[i__] = 1. / sqrt(s[i__]);
/* L30: */
	}

/*        Compute SCOND = min(S(I)) / max(S(I)) */

	*scond = sqrt(smin) / sqrt(*amax);
    }
    return 0;

/*     End of DPOEQU */

} /* dpoequ_ */

#undef a_ref


