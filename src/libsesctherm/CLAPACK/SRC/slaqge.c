#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int slaqge_(integer *m, integer *n, real *a, integer *lda, 
	real *r__, real *c__, real *rowcnd, real *colcnd, real *amax, char *
	equed)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    SLAQGE equilibrates a general M by N matrix A using the row and   
    scaling factors in the vectors R and C.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    A       (input/output) REAL array, dimension (LDA,N)   
            On entry, the M by N matrix A.   
            On exit, the equilibrated matrix.  See EQUED for the form of   
            the equilibrated matrix.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(M,1).   

    R       (input) REAL array, dimension (M)   
            The row scale factors for A.   

    C       (input) REAL array, dimension (N)   
            The column scale factors for A.   

    ROWCND  (input) REAL   
            Ratio of the smallest R(i) to the largest R(i).   

    COLCND  (input) REAL   
            Ratio of the smallest C(i) to the largest C(i).   

    AMAX    (input) REAL   
            Absolute value of largest matrix entry.   

    EQUED   (output) CHARACTER*1   
            Specifies the form of equilibration that was done.   
            = 'N':  No equilibration   
            = 'R':  Row equilibration, i.e., A has been premultiplied by   
                    diag(R).   
            = 'C':  Column equilibration, i.e., A has been postmultiplied   
                    by diag(C).   
            = 'B':  Both row and column equilibration, i.e., A has been   
                    replaced by diag(R) * A * diag(C).   

    Internal Parameters   
    ===================   

    THRESH is a threshold value used to decide if row or column scaling   
    should be done based on the ratio of the row or column scaling   
    factors.  If ROWCND < THRESH, row scaling is done, and if   
    COLCND < THRESH, column scaling is done.   

    LARGE and SMALL are threshold values used to decide if row scaling   
    should be done based on the absolute size of the largest matrix   
    element.  If AMAX > LARGE or AMAX < SMALL, row scaling is done.   

    =====================================================================   


       Quick return if possible   

       Parameter adjustments */
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2;
    /* Local variables */
    static integer i__, j;
    static real large, small, cj;
    extern doublereal slamch_(char *);
#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]

    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --r__;
    --c__;

    /* Function Body */
    if (*m <= 0 || *n <= 0) {
	*(unsigned char *)equed = 'N';
	return 0;
    }

/*     Initialize LARGE and SMALL. */

    small = slamch_("Safe minimum") / slamch_("Precision");
    large = 1.f / small;

    if (*rowcnd >= .1f && *amax >= small && *amax <= large) {

/*        No row scaling */

	if (*colcnd >= .1f) {

/*           No column scaling */

	    *(unsigned char *)equed = 'N';
	} else {

/*           Column scaling */

	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
		cj = c__[j];
		i__2 = *m;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    a_ref(i__, j) = cj * a_ref(i__, j);
/* L10: */
		}
/* L20: */
	    }
	    *(unsigned char *)equed = 'C';
	}
    } else if (*colcnd >= .1f) {

/*        Row scaling, no column scaling */

	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = *m;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		a_ref(i__, j) = r__[i__] * a_ref(i__, j);
/* L30: */
	    }
/* L40: */
	}
	*(unsigned char *)equed = 'R';
    } else {

/*        Row and column scaling */

	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    cj = c__[j];
	    i__2 = *m;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		a_ref(i__, j) = cj * r__[i__] * a_ref(i__, j);
/* L50: */
	    }
/* L60: */
	}
	*(unsigned char *)equed = 'B';
    }

    return 0;

/*     End of SLAQGE */

} /* slaqge_ */

#undef a_ref


