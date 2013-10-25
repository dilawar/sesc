#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dtbrfs_(char *uplo, char *trans, char *diag, integer *n, 
	integer *kd, integer *nrhs, doublereal *ab, integer *ldab, doublereal 
	*b, integer *ldb, doublereal *x, integer *ldx, doublereal *ferr, 
	doublereal *berr, doublereal *work, integer *iwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DTBRFS provides error bounds and backward error estimates for the   
    solution to a system of linear equations with a triangular band   
    coefficient matrix.   

    The solution matrix X must be computed by DTBTRS or some other   
    means before entering this routine.  DTBRFS does not do iterative   
    refinement because doing so cannot improve the backward error.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  A is upper triangular;   
            = 'L':  A is lower triangular.   

    TRANS   (input) CHARACTER*1   
            Specifies the form of the system of equations:   
            = 'N':  A * X = B  (No transpose)   
            = 'T':  A**T * X = B  (Transpose)   
            = 'C':  A**H * X = B  (Conjugate transpose = Transpose)   

    DIAG    (input) CHARACTER*1   
            = 'N':  A is non-unit triangular;   
            = 'U':  A is unit triangular.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    KD      (input) INTEGER   
            The number of superdiagonals or subdiagonals of the   
            triangular band matrix A.  KD >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrices B and X.  NRHS >= 0.   

    AB      (input) DOUBLE PRECISION array, dimension (LDAB,N)   
            The upper or lower triangular band matrix A, stored in the   
            first kd+1 rows of the array. The j-th column of A is stored   
            in the j-th column of the array AB as follows:   
            if UPLO = 'U', AB(kd+1+i-j,j) = A(i,j) for max(1,j-kd)<=i<=j;   
            if UPLO = 'L', AB(1+i-j,j)    = A(i,j) for j<=i<=min(n,j+kd).   
            If DIAG = 'U', the diagonal elements of A are not referenced   
            and are assumed to be 1.   

    LDAB    (input) INTEGER   
            The leading dimension of the array AB.  LDAB >= KD+1.   

    B       (input) DOUBLE PRECISION array, dimension (LDB,NRHS)   
            The right hand side matrix B.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    X       (input) DOUBLE PRECISION array, dimension (LDX,NRHS)   
            The solution matrix X.   

    LDX     (input) INTEGER   
            The leading dimension of the array X.  LDX >= max(1,N).   

    FERR    (output) DOUBLE PRECISION array, dimension (NRHS)   
            The estimated forward error bound for each solution vector   
            X(j) (the j-th column of the solution matrix X).   
            If XTRUE is the true solution corresponding to X(j), FERR(j)   
            is an estimated upper bound for the magnitude of the largest   
            element in (X(j) - XTRUE) divided by the magnitude of the   
            largest element in X(j).  The estimate is as reliable as   
            the estimate for RCOND, and is almost always a slight   
            overestimate of the true error.   

    BERR    (output) DOUBLE PRECISION array, dimension (NRHS)   
            The componentwise relative backward error of each solution   
            vector X(j) (i.e., the smallest relative change in   
            any element of A or B that makes X(j) an exact solution).   

    WORK    (workspace) DOUBLE PRECISION array, dimension (3*N)   

    IWORK   (workspace) INTEGER array, dimension (N)   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static doublereal c_b19 = -1.;
    
    /* System generated locals */
    integer ab_dim1, ab_offset, b_dim1, b_offset, x_dim1, x_offset, i__1, 
	    i__2, i__3, i__4, i__5;
    doublereal d__1, d__2, d__3;
    /* Local variables */
    static integer kase;
    static doublereal safe1, safe2;
    static integer i__, j, k;
    static doublereal s;
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int dtbmv_(char *, char *, char *, integer *, 
	    integer *, doublereal *, integer *, doublereal *, integer *), dcopy_(integer *, doublereal *, integer *
	    , doublereal *, integer *), dtbsv_(char *, char *, char *, 
	    integer *, integer *, doublereal *, integer *, doublereal *, 
	    integer *), daxpy_(integer *, doublereal *
	    , doublereal *, integer *, doublereal *, integer *);
    static logical upper;
    extern doublereal dlamch_(char *);
    extern /* Subroutine */ int dlacon_(integer *, doublereal *, doublereal *,
	     integer *, doublereal *, integer *);
    static doublereal xk;
    static integer nz;
    static doublereal safmin;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static logical notran;
    static char transt[1];
    static logical nounit;
    static doublereal lstres, eps;
#define b_ref(a_1,a_2) b[(a_2)*b_dim1 + a_1]
#define x_ref(a_1,a_2) x[(a_2)*x_dim1 + a_1]
#define ab_ref(a_1,a_2) ab[(a_2)*ab_dim1 + a_1]


    ab_dim1 = *ldab;
    ab_offset = 1 + ab_dim1 * 1;
    ab -= ab_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    x_dim1 = *ldx;
    x_offset = 1 + x_dim1 * 1;
    x -= x_offset;
    --ferr;
    --berr;
    --work;
    --iwork;

    /* Function Body */
    *info = 0;
    upper = lsame_(uplo, "U");
    notran = lsame_(trans, "N");
    nounit = lsame_(diag, "N");

    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (! notran && ! lsame_(trans, "T") && ! 
	    lsame_(trans, "C")) {
	*info = -2;
    } else if (! nounit && ! lsame_(diag, "U")) {
	*info = -3;
    } else if (*n < 0) {
	*info = -4;
    } else if (*kd < 0) {
	*info = -5;
    } else if (*nrhs < 0) {
	*info = -6;
    } else if (*ldab < *kd + 1) {
	*info = -8;
    } else if (*ldb < max(1,*n)) {
	*info = -10;
    } else if (*ldx < max(1,*n)) {
	*info = -12;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DTBRFS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0) {
	i__1 = *nrhs;
	for (j = 1; j <= i__1; ++j) {
	    ferr[j] = 0.;
	    berr[j] = 0.;
/* L10: */
	}
	return 0;
    }

    if (notran) {
	*(unsigned char *)transt = 'T';
    } else {
	*(unsigned char *)transt = 'N';
    }

/*     NZ = maximum number of nonzero elements in each row of A, plus 1 */

    nz = *kd + 2;
    eps = dlamch_("Epsilon");
    safmin = dlamch_("Safe minimum");
    safe1 = nz * safmin;
    safe2 = safe1 / eps;

/*     Do for each right hand side */

    i__1 = *nrhs;
    for (j = 1; j <= i__1; ++j) {

/*        Compute residual R = B - op(A) * X,   
          where op(A) = A or A', depending on TRANS. */

	dcopy_(n, &x_ref(1, j), &c__1, &work[*n + 1], &c__1);
	dtbmv_(uplo, trans, diag, n, kd, &ab[ab_offset], ldab, &work[*n + 1], 
		&c__1);
	daxpy_(n, &c_b19, &b_ref(1, j), &c__1, &work[*n + 1], &c__1);

/*        Compute componentwise relative backward error from formula   

          max(i) ( abs(R(i)) / ( abs(op(A))*abs(X) + abs(B) )(i) )   

          where abs(Z) is the componentwise absolute value of the matrix   
          or vector Z.  If the i-th component of the denominator is less   
          than SAFE2, then SAFE1 is added to the i-th components of the   
          numerator and denominator before dividing. */

	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    work[i__] = (d__1 = b_ref(i__, j), abs(d__1));
/* L20: */
	}

	if (notran) {

/*           Compute abs(A)*abs(X) + abs(B). */

	    if (upper) {
		if (nounit) {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			xk = (d__1 = x_ref(k, j), abs(d__1));
/* Computing MAX */
			i__3 = 1, i__4 = k - *kd;
			i__5 = k;
			for (i__ = max(i__3,i__4); i__ <= i__5; ++i__) {
			    work[i__] += (d__1 = ab_ref(*kd + 1 + i__ - k, k),
				     abs(d__1)) * xk;
/* L30: */
			}
/* L40: */
		    }
		} else {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			xk = (d__1 = x_ref(k, j), abs(d__1));
/* Computing MAX */
			i__5 = 1, i__3 = k - *kd;
			i__4 = k - 1;
			for (i__ = max(i__5,i__3); i__ <= i__4; ++i__) {
			    work[i__] += (d__1 = ab_ref(*kd + 1 + i__ - k, k),
				     abs(d__1)) * xk;
/* L50: */
			}
			work[k] += xk;
/* L60: */
		    }
		}
	    } else {
		if (nounit) {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			xk = (d__1 = x_ref(k, j), abs(d__1));
/* Computing MIN */
			i__5 = *n, i__3 = k + *kd;
			i__4 = min(i__5,i__3);
			for (i__ = k; i__ <= i__4; ++i__) {
			    work[i__] += (d__1 = ab_ref(i__ + 1 - k, k), abs(
				    d__1)) * xk;
/* L70: */
			}
/* L80: */
		    }
		} else {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			xk = (d__1 = x_ref(k, j), abs(d__1));
/* Computing MIN */
			i__5 = *n, i__3 = k + *kd;
			i__4 = min(i__5,i__3);
			for (i__ = k + 1; i__ <= i__4; ++i__) {
			    work[i__] += (d__1 = ab_ref(i__ + 1 - k, k), abs(
				    d__1)) * xk;
/* L90: */
			}
			work[k] += xk;
/* L100: */
		    }
		}
	    }
	} else {

/*           Compute abs(A')*abs(X) + abs(B). */

	    if (upper) {
		if (nounit) {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			s = 0.;
/* Computing MAX */
			i__4 = 1, i__5 = k - *kd;
			i__3 = k;
			for (i__ = max(i__4,i__5); i__ <= i__3; ++i__) {
			    s += (d__1 = ab_ref(*kd + 1 + i__ - k, k), abs(
				    d__1)) * (d__2 = x_ref(i__, j), abs(d__2))
				    ;
/* L110: */
			}
			work[k] += s;
/* L120: */
		    }
		} else {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			s = (d__1 = x_ref(k, j), abs(d__1));
/* Computing MAX */
			i__3 = 1, i__4 = k - *kd;
			i__5 = k - 1;
			for (i__ = max(i__3,i__4); i__ <= i__5; ++i__) {
			    s += (d__1 = ab_ref(*kd + 1 + i__ - k, k), abs(
				    d__1)) * (d__2 = x_ref(i__, j), abs(d__2))
				    ;
/* L130: */
			}
			work[k] += s;
/* L140: */
		    }
		}
	    } else {
		if (nounit) {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			s = 0.;
/* Computing MIN */
			i__3 = *n, i__4 = k + *kd;
			i__5 = min(i__3,i__4);
			for (i__ = k; i__ <= i__5; ++i__) {
			    s += (d__1 = ab_ref(i__ + 1 - k, k), abs(d__1)) * 
				    (d__2 = x_ref(i__, j), abs(d__2));
/* L150: */
			}
			work[k] += s;
/* L160: */
		    }
		} else {
		    i__2 = *n;
		    for (k = 1; k <= i__2; ++k) {
			s = (d__1 = x_ref(k, j), abs(d__1));
/* Computing MIN */
			i__3 = *n, i__4 = k + *kd;
			i__5 = min(i__3,i__4);
			for (i__ = k + 1; i__ <= i__5; ++i__) {
			    s += (d__1 = ab_ref(i__ + 1 - k, k), abs(d__1)) * 
				    (d__2 = x_ref(i__, j), abs(d__2));
/* L170: */
			}
			work[k] += s;
/* L180: */
		    }
		}
	    }
	}
	s = 0.;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    if (work[i__] > safe2) {
/* Computing MAX */
		d__2 = s, d__3 = (d__1 = work[*n + i__], abs(d__1)) / work[
			i__];
		s = max(d__2,d__3);
	    } else {
/* Computing MAX */
		d__2 = s, d__3 = ((d__1 = work[*n + i__], abs(d__1)) + safe1) 
			/ (work[i__] + safe1);
		s = max(d__2,d__3);
	    }
/* L190: */
	}
	berr[j] = s;

/*        Bound error from formula   

          norm(X - XTRUE) / norm(X) .le. FERR =   
          norm( abs(inv(op(A)))*   
             ( abs(R) + NZ*EPS*( abs(op(A))*abs(X)+abs(B) ))) / norm(X)   

          where   
            norm(Z) is the magnitude of the largest component of Z   
            inv(op(A)) is the inverse of op(A)   
            abs(Z) is the componentwise absolute value of the matrix or   
               vector Z   
            NZ is the maximum number of nonzeros in any row of A, plus 1   
            EPS is machine epsilon   

          The i-th component of abs(R)+NZ*EPS*(abs(op(A))*abs(X)+abs(B))   
          is incremented by SAFE1 if the i-th component of   
          abs(op(A))*abs(X) + abs(B) is less than SAFE2.   

          Use DLACON to estimate the infinity-norm of the matrix   
             inv(op(A)) * diag(W),   
          where W = abs(R) + NZ*EPS*( abs(op(A))*abs(X)+abs(B) ))) */

	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    if (work[i__] > safe2) {
		work[i__] = (d__1 = work[*n + i__], abs(d__1)) + nz * eps * 
			work[i__];
	    } else {
		work[i__] = (d__1 = work[*n + i__], abs(d__1)) + nz * eps * 
			work[i__] + safe1;
	    }
/* L200: */
	}

	kase = 0;
L210:
	dlacon_(n, &work[(*n << 1) + 1], &work[*n + 1], &iwork[1], &ferr[j], &
		kase);
	if (kase != 0) {
	    if (kase == 1) {

/*              Multiply by diag(W)*inv(op(A)'). */

		dtbsv_(uplo, transt, diag, n, kd, &ab[ab_offset], ldab, &work[
			*n + 1], &c__1);
		i__2 = *n;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    work[*n + i__] = work[i__] * work[*n + i__];
/* L220: */
		}
	    } else {

/*              Multiply by inv(op(A))*diag(W). */

		i__2 = *n;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    work[*n + i__] = work[i__] * work[*n + i__];
/* L230: */
		}
		dtbsv_(uplo, trans, diag, n, kd, &ab[ab_offset], ldab, &work[*
			n + 1], &c__1);
	    }
	    goto L210;
	}

/*        Normalize error. */

	lstres = 0.;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
	    d__2 = lstres, d__3 = (d__1 = x_ref(i__, j), abs(d__1));
	    lstres = max(d__2,d__3);
/* L240: */
	}
	if (lstres != 0.) {
	    ferr[j] /= lstres;
	}

/* L250: */
    }

    return 0;

/*     End of DTBRFS */

} /* dtbrfs_ */

#undef ab_ref
#undef x_ref
#undef b_ref


