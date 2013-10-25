#include "blaswrap.h"
#include "f2c.h"

doublereal dlantb_(char *norm, char *uplo, char *diag, integer *n, integer *k,
	 doublereal *ab, integer *ldab, doublereal *work)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    DLANTB  returns the value of the one norm,  or the Frobenius norm, or   
    the  infinity norm,  or the element of  largest absolute value  of an   
    n by n triangular band matrix A,  with ( k + 1 ) diagonals.   

    Description   
    ===========   

    DLANTB returns the value   

       DLANTB = ( max(abs(A(i,j))), NORM = 'M' or 'm'   
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
            Specifies the value to be returned in DLANTB as described   
            above.   

    UPLO    (input) CHARACTER*1   
            Specifies whether the matrix A is upper or lower triangular.   
            = 'U':  Upper triangular   
            = 'L':  Lower triangular   

    DIAG    (input) CHARACTER*1   
            Specifies whether or not the matrix A is unit triangular.   
            = 'N':  Non-unit triangular   
            = 'U':  Unit triangular   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.  When N = 0, DLANTB is   
            set to zero.   

    K       (input) INTEGER   
            The number of super-diagonals of the matrix A if UPLO = 'U',   
            or the number of sub-diagonals of the matrix A if UPLO = 'L'.   
            K >= 0.   

    AB      (input) DOUBLE PRECISION array, dimension (LDAB,N)   
            The upper or lower triangular band matrix A, stored in the   
            first k+1 rows of AB.  The j-th column of A is stored   
            in the j-th column of the array AB as follows:   
            if UPLO = 'U', AB(k+1+i-j,j) = A(i,j) for max(1,j-k)<=i<=j;   
            if UPLO = 'L', AB(1+i-j,j)   = A(i,j) for j<=i<=min(n,j+k).   
            Note that when DIAG = 'U', the elements of the array AB   
            corresponding to the diagonal elements of the matrix A are   
            not referenced, but are assumed to be one.   

    LDAB    (input) INTEGER   
            The leading dimension of the array AB.  LDAB >= K+1.   

    WORK    (workspace) DOUBLE PRECISION array, dimension (LWORK),   
            where LWORK >= N when NORM = 'I'; otherwise, WORK is not   
            referenced.   

   =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer ab_dim1, ab_offset, i__1, i__2, i__3, i__4, i__5;
    doublereal ret_val, d__1, d__2, d__3;
    /* Builtin functions */
    double sqrt(doublereal);
    /* Local variables */
    static integer i__, j, l;
    static doublereal scale;
    static logical udiag;
    extern logical lsame_(char *, char *);
    static doublereal value;
    extern /* Subroutine */ int dlassq_(integer *, doublereal *, integer *, 
	    doublereal *, doublereal *);
    static doublereal sum;
#define ab_ref(a_1,a_2) ab[(a_2)*ab_dim1 + a_1]


    ab_dim1 = *ldab;
    ab_offset = 1 + ab_dim1 * 1;
    ab -= ab_offset;
    --work;

    /* Function Body */
    if (*n == 0) {
	value = 0.;
    } else if (lsame_(norm, "M")) {

/*        Find max(abs(A(i,j))). */

	if (lsame_(diag, "U")) {
	    value = 1.;
	    if (lsame_(uplo, "U")) {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
		    i__2 = *k + 2 - j;
		    i__3 = *k;
		    for (i__ = max(i__2,1); i__ <= i__3; ++i__) {
/* Computing MAX */
			d__2 = value, d__3 = (d__1 = ab_ref(i__, j), abs(d__1)
				);
			value = max(d__2,d__3);
/* L10: */
		    }
/* L20: */
		}
	    } else {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		    i__2 = *n + 1 - j, i__4 = *k + 1;
		    i__3 = min(i__2,i__4);
		    for (i__ = 2; i__ <= i__3; ++i__) {
/* Computing MAX */
			d__2 = value, d__3 = (d__1 = ab_ref(i__, j), abs(d__1)
				);
			value = max(d__2,d__3);
/* L30: */
		    }
/* L40: */
		}
	    }
	} else {
	    value = 0.;
	    if (lsame_(uplo, "U")) {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
		    i__3 = *k + 2 - j;
		    i__2 = *k + 1;
		    for (i__ = max(i__3,1); i__ <= i__2; ++i__) {
/* Computing MAX */
			d__2 = value, d__3 = (d__1 = ab_ref(i__, j), abs(d__1)
				);
			value = max(d__2,d__3);
/* L50: */
		    }
/* L60: */
		}
	    } else {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		    i__3 = *n + 1 - j, i__4 = *k + 1;
		    i__2 = min(i__3,i__4);
		    for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
			d__2 = value, d__3 = (d__1 = ab_ref(i__, j), abs(d__1)
				);
			value = max(d__2,d__3);
/* L70: */
		    }
/* L80: */
		}
	    }
	}
    } else if (lsame_(norm, "O") || *(unsigned char *)
	    norm == '1') {

/*        Find norm1(A). */

	value = 0.;
	udiag = lsame_(diag, "U");
	if (lsame_(uplo, "U")) {
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
		if (udiag) {
		    sum = 1.;
/* Computing MAX */
		    i__2 = *k + 2 - j;
		    i__3 = *k;
		    for (i__ = max(i__2,1); i__ <= i__3; ++i__) {
			sum += (d__1 = ab_ref(i__, j), abs(d__1));
/* L90: */
		    }
		} else {
		    sum = 0.;
/* Computing MAX */
		    i__3 = *k + 2 - j;
		    i__2 = *k + 1;
		    for (i__ = max(i__3,1); i__ <= i__2; ++i__) {
			sum += (d__1 = ab_ref(i__, j), abs(d__1));
/* L100: */
		    }
		}
		value = max(value,sum);
/* L110: */
	    }
	} else {
	    i__1 = *n;
	    for (j = 1; j <= i__1; ++j) {
		if (udiag) {
		    sum = 1.;
/* Computing MIN */
		    i__3 = *n + 1 - j, i__4 = *k + 1;
		    i__2 = min(i__3,i__4);
		    for (i__ = 2; i__ <= i__2; ++i__) {
			sum += (d__1 = ab_ref(i__, j), abs(d__1));
/* L120: */
		    }
		} else {
		    sum = 0.;
/* Computing MIN */
		    i__3 = *n + 1 - j, i__4 = *k + 1;
		    i__2 = min(i__3,i__4);
		    for (i__ = 1; i__ <= i__2; ++i__) {
			sum += (d__1 = ab_ref(i__, j), abs(d__1));
/* L130: */
		    }
		}
		value = max(value,sum);
/* L140: */
	    }
	}
    } else if (lsame_(norm, "I")) {

/*        Find normI(A). */

	value = 0.;
	if (lsame_(uplo, "U")) {
	    if (lsame_(diag, "U")) {
		i__1 = *n;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 1.;
/* L150: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    l = *k + 1 - j;
/* Computing MAX */
		    i__2 = 1, i__3 = j - *k;
		    i__4 = j - 1;
		    for (i__ = max(i__2,i__3); i__ <= i__4; ++i__) {
			work[i__] += (d__1 = ab_ref(l + i__, j), abs(d__1));
/* L160: */
		    }
/* L170: */
		}
	    } else {
		i__1 = *n;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 0.;
/* L180: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    l = *k + 1 - j;
/* Computing MAX */
		    i__4 = 1, i__2 = j - *k;
		    i__3 = j;
		    for (i__ = max(i__4,i__2); i__ <= i__3; ++i__) {
			work[i__] += (d__1 = ab_ref(l + i__, j), abs(d__1));
/* L190: */
		    }
/* L200: */
		}
	    }
	} else {
	    if (lsame_(diag, "U")) {
		i__1 = *n;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 1.;
/* L210: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    l = 1 - j;
/* Computing MIN */
		    i__4 = *n, i__2 = j + *k;
		    i__3 = min(i__4,i__2);
		    for (i__ = j + 1; i__ <= i__3; ++i__) {
			work[i__] += (d__1 = ab_ref(l + i__, j), abs(d__1));
/* L220: */
		    }
/* L230: */
		}
	    } else {
		i__1 = *n;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    work[i__] = 0.;
/* L240: */
		}
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    l = 1 - j;
/* Computing MIN */
		    i__4 = *n, i__2 = j + *k;
		    i__3 = min(i__4,i__2);
		    for (i__ = j; i__ <= i__3; ++i__) {
			work[i__] += (d__1 = ab_ref(l + i__, j), abs(d__1));
/* L250: */
		    }
/* L260: */
		}
	    }
	}
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing MAX */
	    d__1 = value, d__2 = work[i__];
	    value = max(d__1,d__2);
/* L270: */
	}
    } else if (lsame_(norm, "F") || lsame_(norm, "E")) {

/*        Find normF(A). */

	if (lsame_(uplo, "U")) {
	    if (lsame_(diag, "U")) {
		scale = 1.;
		sum = (doublereal) (*n);
		if (*k > 0) {
		    i__1 = *n;
		    for (j = 2; j <= i__1; ++j) {
/* Computing MAX */
			i__3 = *k + 2 - j;
/* Computing MIN */
			i__2 = j - 1;
			i__4 = min(i__2,*k);
			dlassq_(&i__4, &ab_ref(max(i__3,1), j), &c__1, &scale,
				 &sum);
/* L280: */
		    }
		}
	    } else {
		scale = 0.;
		sum = 1.;
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
		    i__3 = *k + 2 - j;
/* Computing MIN */
		    i__2 = j, i__5 = *k + 1;
		    i__4 = min(i__2,i__5);
		    dlassq_(&i__4, &ab_ref(max(i__3,1), j), &c__1, &scale, &
			    sum);
/* L290: */
		}
	    }
	} else {
	    if (lsame_(diag, "U")) {
		scale = 1.;
		sum = (doublereal) (*n);
		if (*k > 0) {
		    i__1 = *n - 1;
		    for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
			i__4 = *n - j;
			i__3 = min(i__4,*k);
			dlassq_(&i__3, &ab_ref(2, j), &c__1, &scale, &sum);
/* L300: */
		    }
		}
	    } else {
		scale = 0.;
		sum = 1.;
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		    i__4 = *n - j + 1, i__2 = *k + 1;
		    i__3 = min(i__4,i__2);
		    dlassq_(&i__3, &ab_ref(1, j), &c__1, &scale, &sum);
/* L310: */
		}
	    }
	}
	value = scale * sqrt(sum);
    }

    ret_val = value;
    return ret_val;

/*     End of DLANTB */

} /* dlantb_ */

#undef ab_ref


