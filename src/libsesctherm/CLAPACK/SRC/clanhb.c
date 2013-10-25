#include "blaswrap.h"
#include "f2c.h"

doublereal clanhb_(char *norm, char *uplo, integer *n, integer *k, complex *
	ab, integer *ldab, real *work)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    CLANHB  returns the value of the one norm,  or the Frobenius norm, or   
    the  infinity norm,  or the element of  largest absolute value  of an   
    n by n hermitian band matrix A,  with k super-diagonals.   

    Description   
    ===========   

    CLANHB returns the value   

       CLANHB = ( max(abs(A(i,j))), NORM = 'M' or 'm'   
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
            Specifies the value to be returned in CLANHB as described   
            above.   

    UPLO    (input) CHARACTER*1   
            Specifies whether the upper or lower triangular part of the   
            band matrix A is supplied.   
            = 'U':  Upper triangular   
            = 'L':  Lower triangular   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.  When N = 0, CLANHB is   
            set to zero.   

    K       (input) INTEGER   
            The number of super-diagonals or sub-diagonals of the   
            band matrix A.  K >= 0.   

    AB      (input) COMPLEX array, dimension (LDAB,N)   
            The upper or lower triangle of the hermitian band matrix A,   
            stored in the first K+1 rows of AB.  The j-th column of A is   
            stored in the j-th column of the array AB as follows:   
            if UPLO = 'U', AB(k+1+i-j,j) = A(i,j) for max(1,j-k)<=i<=j;   
            if UPLO = 'L', AB(1+i-j,j)   = A(i,j) for j<=i<=min(n,j+k).   
            Note that the imaginary parts of the diagonal elements need   
            not be set and are assumed to be zero.   

    LDAB    (input) INTEGER   
            The leading dimension of the array AB.  LDAB >= K+1.   

    WORK    (workspace) REAL array, dimension (LWORK),   
            where LWORK >= N when NORM = 'I' or '1' or 'O'; otherwise,   
            WORK is not referenced.   

   =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer ab_dim1, ab_offset, i__1, i__2, i__3, i__4;
    real ret_val, r__1, r__2, r__3;
    /* Builtin functions */
    double c_abs(complex *), sqrt(doublereal);
    /* Local variables */
    static real absa;
    static integer i__, j, l;
    static real scale;
    extern logical lsame_(char *, char *);
    static real value;
    extern /* Subroutine */ int classq_(integer *, complex *, integer *, real 
	    *, real *);
    static real sum;
#define ab_subscr(a_1,a_2) (a_2)*ab_dim1 + a_1
#define ab_ref(a_1,a_2) ab[ab_subscr(a_1,a_2)]


    ab_dim1 = *ldab;
    ab_offset = 1 + ab_dim1 * 1;
    ab -= ab_offset;
    --work;

    /* Function Body */
    if (*n == 0) {
	value = 0.f;
    } else if (lsame_(norm, "M")) {

/*        Find max(abs(A(i,j))). */

	value = 0.f;
	if (lsame_(uplo, "U")) {
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
		i__2 = *k + 2 - j;
		i__3 = *k;
		for (i__ = max(i__2,1); i__ <= i__3; ++i__) {
/* Computing MAX */
		    r__1 = value, r__2 = c_abs(&ab_ref(i__, j));
		    value = dmax(r__1,r__2);
/* L10: */
		}
/* Computing MAX */
		i__3 = ab_subscr(*k + 1, j);
		r__2 = value, r__3 = (r__1 = ab[i__3].r, dabs(r__1));
		value = dmax(r__2,r__3);
/* L20: */
	    }
	} else {
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
		i__3 = ab_subscr(1, j);
		r__2 = value, r__3 = (r__1 = ab[i__3].r, dabs(r__1));
		value = dmax(r__2,r__3);
/* Computing MIN */
		i__2 = *n + 1 - j, i__4 = *k + 1;
		i__3 = min(i__2,i__4);
		for (i__ = 2; i__ <= i__3; ++i__) {
/* Computing MAX */
		    r__1 = value, r__2 = c_abs(&ab_ref(i__, j));
		    value = dmax(r__1,r__2);
/* L30: */
		}
/* L40: */
	    }
	}
    } else if (lsame_(norm, "I") || lsame_(norm, "O") || *(unsigned char *)norm == '1') {

/*        Find normI(A) ( = norm1(A), since A is hermitian). */

	value = 0.f;
	if (lsame_(uplo, "U")) {
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
		sum = 0.f;
		l = *k + 1 - j;
/* Computing MAX */
		i__3 = 1, i__2 = j - *k;
		i__4 = j - 1;
		for (i__ = max(i__3,i__2); i__ <= i__4; ++i__) {
		    absa = c_abs(&ab_ref(l + i__, j));
		    sum += absa;
		    work[i__] += absa;
/* L50: */
		}
		i__4 = ab_subscr(*k + 1, j);
		work[j] = sum + (r__1 = ab[i__4].r, dabs(r__1));
/* L60: */
	    }
	    i__1 = *n;
	    for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing MAX */
		r__1 = value, r__2 = work[i__];
		value = dmax(r__1,r__2);
/* L70: */
	    }
	} else {
	    i__1 = *n;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		work[i__] = 0.f;
/* L80: */
	    }
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
		i__4 = ab_subscr(1, j);
		sum = work[j] + (r__1 = ab[i__4].r, dabs(r__1));
		l = 1 - j;
/* Computing MIN */
		i__3 = *n, i__2 = j + *k;
		i__4 = min(i__3,i__2);
		for (i__ = j + 1; i__ <= i__4; ++i__) {
		    absa = c_abs(&ab_ref(l + i__, j));
		    sum += absa;
		    work[i__] += absa;
/* L90: */
		}
		value = dmax(value,sum);
/* L100: */
	    }
	}
    } else if (lsame_(norm, "F") || lsame_(norm, "E")) {

/*        Find normF(A). */

	scale = 0.f;
	sum = 1.f;
	if (*k > 0) {
	    if (lsame_(uplo, "U")) {
		i__1 = *n;
		for (j = 2; j <= i__1; ++j) {
/* Computing MAX */
		    i__4 = *k + 2 - j;
/* Computing MIN */
		    i__2 = j - 1;
		    i__3 = min(i__2,*k);
		    classq_(&i__3, &ab_ref(max(i__4,1), j), &c__1, &scale, &
			    sum);
/* L110: */
		}
		l = *k + 1;
	    } else {
		i__1 = *n - 1;
		for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		    i__3 = *n - j;
		    i__4 = min(i__3,*k);
		    classq_(&i__4, &ab_ref(2, j), &c__1, &scale, &sum);
/* L120: */
		}
		l = 1;
	    }
	    sum *= 2;
	} else {
	    l = 1;
	}
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__4 = ab_subscr(l, j);
	    if (ab[i__4].r != 0.f) {
		i__4 = ab_subscr(l, j);
		absa = (r__1 = ab[i__4].r, dabs(r__1));
		if (scale < absa) {
/* Computing 2nd power */
		    r__1 = scale / absa;
		    sum = sum * (r__1 * r__1) + 1.f;
		    scale = absa;
		} else {
/* Computing 2nd power */
		    r__1 = absa / scale;
		    sum += r__1 * r__1;
		}
	    }
/* L130: */
	}
	value = scale * sqrt(sum);
    }

    ret_val = value;
    return ret_val;

/*     End of CLANHB */

} /* clanhb_ */

#undef ab_ref
#undef ab_subscr


