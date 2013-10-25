#include "blaswrap.h"
#include "f2c.h"

doublereal slantr_(char *norm, char *uplo, char *diag, integer *m, integer *n,
	 real *a, integer *lda, real *work)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    SLANTR  returns the value of the one norm,  or the Frobenius norm, or   
    the  infinity norm,  or the  element of  largest absolute value  of a   
    trapezoidal or triangular matrix A.   

    Description   
    ===========   

    SLANTR returns the value   

       SLANTR = ( max(abs(A(i,j))), NORM = 'M' or 'm'   
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
            Specifies the value to be returned in SLANTR as described   
            above.   

    UPLO    (input) CHARACTER*1   
            Specifies whether the matrix A is upper or lower trapezoidal.   
            = 'U':  Upper trapezoidal   
            = 'L':  Lower trapezoidal   
            Note that A is triangular instead of trapezoidal if M = N.   

    DIAG    (input) CHARACTER*1   
            Specifies whether or not the matrix A has unit diagonal.   
            = 'N':  Non-unit diagonal   
            = 'U':  Unit diagonal   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0, and if   
            UPLO = 'U', M <= N.  When M = 0, SLANTR is set to zero.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0, and if   
            UPLO = 'L', N <= M.  When N = 0, SLANTR is set to zero.   

    A       (input) REAL array, dimension (LDA,N)   
            The trapezoidal matrix A (A is triangular if M = N).   
            If UPLO = 'U', the leading m by n upper trapezoidal part of   
            the array A contains the upper trapezoidal matrix, and the   
            strictly lower triangular part of A is not referenced.   
            If UPLO = 'L', the leading m by n lower trapezoidal part of   
            the array A contains the lower trapezoidal matrix, and the   
            strictly upper triangular part of A is not referenced.  Note   
            that when DIAG = 'U', the diagonal elements of A are not   
            referenced and are assumed to be one.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(M,1).   

    WORK    (workspace) REAL array, dimension (LWORK),   
            where LWORK >= M when NORM = 'I'; otherwise, WORK is not   
            referenced.   

   =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3, i__4;
    real ret_val, r__1, r__2, r__3;
    /* Builtin functions */
    double sqrt(doublereal);
    /* Local variables */
    static integer i__, j;
    static real scale;
    static logical udiag;
    extern logical lsame_(char *, char *);
    static real value;
    extern /* Subroutine */ int slassq_(integer *, real *, integer *, real *, 
	    real *);
    static real sum;
#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --work;

    /* Function Body */
    if (min(*m,*n) == 0) {
	value = 0.f;
    } else if (lsame_(norm, "M")) {

/*        Find max(abs(A(i,j))). */

	if (lsame_(diag, "U")) {
	    value = 1.f;
	    if (lsame_(uplo, "U")) {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		    i__3 = *m, i__4 = j - 1;
		    i__2 = min(i__3,i__4);
		    for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
			r__2 = value, r__3 = (r__1 = a_ref(i__, j), dabs(r__1)
				);
			value = dmax(r__2,r__3);
/* L10: */
		    }
/* L20: */
		}
	    } else {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = *m;
		    for (i__ = j + 1; i__ <= i__2; ++i__) {
/* Computing MAX */
			r__2 = value, r__3 = (r__1 = a_ref(i__, j), dabs(r__1)
				);
			value = dmax(r__2,r__3);
/* L30: */
		    }
/* L40: */
		}
	    }
	} else {
	    value = 0.f;
	    if (lsame_(uplo, "U")) {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = min(*m,j);
		    for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
			r__2 = value, r__3 = (r__1 = a_ref(i__, j), dabs(r__1)
				);
			value = dmax(r__2,r__3);
/* L50: */
		    }
/* L60: */
		}
	    } else {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = *m;
		    for (i__ = j; i__ <= i__2; ++i__) {
/* Computing MAX */
			r__2 = value, r__3 = (r__1 = a_ref(i__, j), dabs(r__1)
				);
			value = dmax(r__2,r__3);
/* L70: */
		    }
/* L80: */
		}
	    }
	}
    } else if (lsame_(norm, "O") || *(unsigned char *)
	    norm == '1') {

/*        Find norm1(A). */

	value = 0.f;
	udiag = lsame_(diag, "U");
	if (lsame_(uplo, "U")) {
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
		if (udiag && j <= *m) {
		    sum = 1.f;
		    i__2 = j - 1;
		    for (i__ = 1; i__ <= i__2; ++i__) {
			sum += (r__1 = a_ref(i__, j), dabs(r__1));
/* L90: */
		    }
		} else {
		    sum = 0.f;
		    i__2 = min(*m,j);
		    for (i__ = 1; i__ <= i__2; ++i__) {
			sum += (r__1 = a_ref(i__, j), dabs(r__1));
/* L100: */
		    }
		}
		value = dmax(value,sum);
/* L110: */
	    }
	} else {
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
		if (udiag) {
		    sum = 1.f;
		    i__2 = *m;
		    for (i__ = j + 1; i__ <= i__2; ++i__) {
			sum += (r__1 = a_ref(i__, j), dabs(r__1));
/* L120: */
		    }
		} else {
		    sum = 0.f;
		    i__2 = *m;
		    for (i__ = j; i__ <= i__2; ++i__) {
			sum += (r__1 = a_ref(i__, j), dabs(r__1));
/* L130: */
		    }
		}
		value = dmax(value,sum);
/* L140: */
	    }
	}
    } else if (lsame_(norm, "I")) {

/*        Find normI(A). */

	if (lsame_(uplo, "U")) {
	    if (lsame_(diag, "U")) {
		i__1 = *m;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 1.f;
/* L150: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		    i__3 = *m, i__4 = j - 1;
		    i__2 = min(i__3,i__4);
		    for (i__ = 1; i__ <= i__2; ++i__) {
			work[i__] += (r__1 = a_ref(i__, j), dabs(r__1));
/* L160: */
		    }
/* L170: */
		}
	    } else {
		i__1 = *m;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 0.f;
/* L180: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = min(*m,j);
		    for (i__ = 1; i__ <= i__2; ++i__) {
			work[i__] += (r__1 = a_ref(i__, j), dabs(r__1));
/* L190: */
		    }
/* L200: */
		}
	    }
	} else {
	    if (lsame_(diag, "U")) {
		i__1 = *n;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 1.f;
/* L210: */
		}
		i__1 = *m;
		for (i__ = *n + 1; i__ <= i__1; ++i__) {
		    work[i__] = 0.f;
/* L220: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = *m;
		    for (i__ = j + 1; i__ <= i__2; ++i__) {
			work[i__] += (r__1 = a_ref(i__, j), dabs(r__1));
/* L230: */
		    }
/* L240: */
		}
	    } else {
		i__1 = *m;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 0.f;
/* L250: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = *m;
		    for (i__ = j; i__ <= i__2; ++i__) {
			work[i__] += (r__1 = a_ref(i__, j), dabs(r__1));
/* L260: */
		    }
/* L270: */
		}
	    }
	}
	value = 0.f;
	i__1 = *m;
	for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing MAX */
	    r__1 = value, r__2 = work[i__];
	    value = dmax(r__1,r__2);
/* L280: */
	}
    } else if (lsame_(norm, "F") || lsame_(norm, "E")) {

/*        Find normF(A). */

	if (lsame_(uplo, "U")) {
	    if (lsame_(diag, "U")) {
		scale = 1.f;
		sum = (real) min(*m,*n);
		i__1 = *n;
		for (j = 2; j <= i__1; ++j) {
/* Computing MIN */
		    i__3 = *m, i__4 = j - 1;
		    i__2 = min(i__3,i__4);
		    slassq_(&i__2, &a_ref(1, j), &c__1, &scale, &sum);
/* L290: */
		}
	    } else {
		scale = 0.f;
		sum = 1.f;
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = min(*m,j);
		    slassq_(&i__2, &a_ref(1, j), &c__1, &scale, &sum);
/* L300: */
		}
	    }
	} else {
	    if (lsame_(diag, "U")) {
		scale = 1.f;
		sum = (real) min(*m,*n);
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		    i__2 = *m, i__3 = j + 1;
		    i__4 = *m - j;
		    slassq_(&i__4, &a_ref(min(i__2,i__3), j), &c__1, &scale, &
			    sum);
/* L310: */
		}
	    } else {
		scale = 0.f;
		sum = 1.f;
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = *m - j + 1;
		    slassq_(&i__2, &a_ref(j, j), &c__1, &scale, &sum);
/* L320: */
		}
	    }
	}
	value = scale * sqrt(sum);
    }

    ret_val = value;
    return ret_val;

/*     End of SLANTR */

} /* slantr_ */

#undef a_ref


