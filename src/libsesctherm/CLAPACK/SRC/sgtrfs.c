#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int sgtrfs_(char *trans, integer *n, integer *nrhs, real *dl,
	 real *d__, real *du, real *dlf, real *df, real *duf, real *du2, 
	integer *ipiv, real *b, integer *ldb, real *x, integer *ldx, real *
	ferr, real *berr, real *work, integer *iwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    SGTRFS improves the computed solution to a system of linear   
    equations when the coefficient matrix is tridiagonal, and provides   
    error bounds and backward error estimates for the solution.   

    Arguments   
    =========   

    TRANS   (input) CHARACTER*1   
            Specifies the form of the system of equations:   
            = 'N':  A * X = B     (No transpose)   
            = 'T':  A**T * X = B  (Transpose)   
            = 'C':  A**H * X = B  (Conjugate transpose = Transpose)   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    DL      (input) REAL array, dimension (N-1)   
            The (n-1) subdiagonal elements of A.   

    D       (input) REAL array, dimension (N)   
            The diagonal elements of A.   

    DU      (input) REAL array, dimension (N-1)   
            The (n-1) superdiagonal elements of A.   

    DLF     (input) REAL array, dimension (N-1)   
            The (n-1) multipliers that define the matrix L from the   
            LU factorization of A as computed by SGTTRF.   

    DF      (input) REAL array, dimension (N)   
            The n diagonal elements of the upper triangular matrix U from   
            the LU factorization of A.   

    DUF     (input) REAL array, dimension (N-1)   
            The (n-1) elements of the first superdiagonal of U.   

    DU2     (input) REAL array, dimension (N-2)   
            The (n-2) elements of the second superdiagonal of U.   

    IPIV    (input) INTEGER array, dimension (N)   
            The pivot indices; for 1 <= i <= n, row i of the matrix was   
            interchanged with row IPIV(i).  IPIV(i) will always be either   
            i or i+1; IPIV(i) = i indicates a row interchange was not   
            required.   

    B       (input) REAL array, dimension (LDB,NRHS)   
            The right hand side matrix B.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    X       (input/output) REAL array, dimension (LDX,NRHS)   
            On entry, the solution matrix X, as computed by SGTTRS.   
            On exit, the improved solution matrix X.   

    LDX     (input) INTEGER   
            The leading dimension of the array X.  LDX >= max(1,N).   

    FERR    (output) REAL array, dimension (NRHS)   
            The estimated forward error bound for each solution vector   
            X(j) (the j-th column of the solution matrix X).   
            If XTRUE is the true solution corresponding to X(j), FERR(j)   
            is an estimated upper bound for the magnitude of the largest   
            element in (X(j) - XTRUE) divided by the magnitude of the   
            largest element in X(j).  The estimate is as reliable as   
            the estimate for RCOND, and is almost always a slight   
            overestimate of the true error.   

    BERR    (output) REAL array, dimension (NRHS)   
            The componentwise relative backward error of each solution   
            vector X(j) (i.e., the smallest relative change in   
            any element of A or B that makes X(j) an exact solution).   

    WORK    (workspace) REAL array, dimension (3*N)   

    IWORK   (workspace) INTEGER array, dimension (N)   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    Internal Parameters   
    ===================   

    ITMAX is the maximum number of steps of iterative refinement.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static real c_b18 = -1.f;
    static real c_b19 = 1.f;
    
    /* System generated locals */
    integer b_dim1, b_offset, x_dim1, x_offset, i__1, i__2;
    real r__1, r__2, r__3, r__4;
    /* Local variables */
    static integer kase;
    static real safe1, safe2;
    static integer i__, j;
    static real s;
    extern logical lsame_(char *, char *);
    static integer count;
    extern /* Subroutine */ int scopy_(integer *, real *, integer *, real *, 
	    integer *), saxpy_(integer *, real *, real *, integer *, real *, 
	    integer *);
    extern doublereal slamch_(char *);
    static integer nz;
    static real safmin;
    extern /* Subroutine */ int xerbla_(char *, integer *), slacon_(
	    integer *, real *, real *, integer *, real *, integer *), slagtm_(
	    char *, integer *, integer *, real *, real *, real *, real *, 
	    real *, integer *, real *, real *, integer *);
    static logical notran;
    static char transn[1], transt[1];
    static real lstres;
    extern /* Subroutine */ int sgttrs_(char *, integer *, integer *, real *, 
	    real *, real *, real *, integer *, real *, integer *, integer *);
    static real eps;
#define b_ref(a_1,a_2) b[(a_2)*b_dim1 + a_1]
#define x_ref(a_1,a_2) x[(a_2)*x_dim1 + a_1]


    --dl;
    --d__;
    --du;
    --dlf;
    --df;
    --duf;
    --du2;
    --ipiv;
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
    notran = lsame_(trans, "N");
    if (! notran && ! lsame_(trans, "T") && ! lsame_(
	    trans, "C")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*nrhs < 0) {
	*info = -3;
    } else if (*ldb < max(1,*n)) {
	*info = -13;
    } else if (*ldx < max(1,*n)) {
	*info = -15;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("SGTRFS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0) {
	i__1 = *nrhs;
	for (j = 1; j <= i__1; ++j) {
	    ferr[j] = 0.f;
	    berr[j] = 0.f;
/* L10: */
	}
	return 0;
    }

    if (notran) {
	*(unsigned char *)transn = 'N';
	*(unsigned char *)transt = 'T';
    } else {
	*(unsigned char *)transn = 'T';
	*(unsigned char *)transt = 'N';
    }

/*     NZ = maximum number of nonzero elements in each row of A, plus 1 */

    nz = 4;
    eps = slamch_("Epsilon");
    safmin = slamch_("Safe minimum");
    safe1 = nz * safmin;
    safe2 = safe1 / eps;

/*     Do for each right hand side */

    i__1 = *nrhs;
    for (j = 1; j <= i__1; ++j) {

	count = 1;
	lstres = 3.f;
L20:

/*        Loop until stopping criterion is satisfied.   

          Compute residual R = B - op(A) * X,   
          where op(A) = A, A**T, or A**H, depending on TRANS. */

	scopy_(n, &b_ref(1, j), &c__1, &work[*n + 1], &c__1);
	slagtm_(trans, n, &c__1, &c_b18, &dl[1], &d__[1], &du[1], &x_ref(1, j)
		, ldx, &c_b19, &work[*n + 1], n);

/*        Compute abs(op(A))*abs(x) + abs(b) for use in the backward   
          error bound. */

	if (notran) {
	    if (*n == 1) {
		work[1] = (r__1 = b_ref(1, j), dabs(r__1)) + (r__2 = d__[1] * 
			x_ref(1, j), dabs(r__2));
	    } else {
		work[1] = (r__1 = b_ref(1, j), dabs(r__1)) + (r__2 = d__[1] * 
			x_ref(1, j), dabs(r__2)) + (r__3 = du[1] * x_ref(2, j)
			, dabs(r__3));
		i__2 = *n - 1;
		for (i__ = 2; i__ <= i__2; ++i__) {
		    work[i__] = (r__1 = b_ref(i__, j), dabs(r__1)) + (r__2 = 
			    dl[i__ - 1] * x_ref(i__ - 1, j), dabs(r__2)) + (
			    r__3 = d__[i__] * x_ref(i__, j), dabs(r__3)) + (
			    r__4 = du[i__] * x_ref(i__ + 1, j), dabs(r__4));
/* L30: */
		}
		work[*n] = (r__1 = b_ref(*n, j), dabs(r__1)) + (r__2 = dl[*n 
			- 1] * x_ref(*n - 1, j), dabs(r__2)) + (r__3 = d__[*n]
			 * x_ref(*n, j), dabs(r__3));
	    }
	} else {
	    if (*n == 1) {
		work[1] = (r__1 = b_ref(1, j), dabs(r__1)) + (r__2 = d__[1] * 
			x_ref(1, j), dabs(r__2));
	    } else {
		work[1] = (r__1 = b_ref(1, j), dabs(r__1)) + (r__2 = d__[1] * 
			x_ref(1, j), dabs(r__2)) + (r__3 = dl[1] * x_ref(2, j)
			, dabs(r__3));
		i__2 = *n - 1;
		for (i__ = 2; i__ <= i__2; ++i__) {
		    work[i__] = (r__1 = b_ref(i__, j), dabs(r__1)) + (r__2 = 
			    du[i__ - 1] * x_ref(i__ - 1, j), dabs(r__2)) + (
			    r__3 = d__[i__] * x_ref(i__, j), dabs(r__3)) + (
			    r__4 = dl[i__] * x_ref(i__ + 1, j), dabs(r__4));
/* L40: */
		}
		work[*n] = (r__1 = b_ref(*n, j), dabs(r__1)) + (r__2 = du[*n 
			- 1] * x_ref(*n - 1, j), dabs(r__2)) + (r__3 = d__[*n]
			 * x_ref(*n, j), dabs(r__3));
	    }
	}

/*        Compute componentwise relative backward error from formula   

          max(i) ( abs(R(i)) / ( abs(op(A))*abs(X) + abs(B) )(i) )   

          where abs(Z) is the componentwise absolute value of the matrix   
          or vector Z.  If the i-th component of the denominator is less   
          than SAFE2, then SAFE1 is added to the i-th components of the   
          numerator and denominator before dividing. */

	s = 0.f;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    if (work[i__] > safe2) {
/* Computing MAX */
		r__2 = s, r__3 = (r__1 = work[*n + i__], dabs(r__1)) / work[
			i__];
		s = dmax(r__2,r__3);
	    } else {
/* Computing MAX */
		r__2 = s, r__3 = ((r__1 = work[*n + i__], dabs(r__1)) + safe1)
			 / (work[i__] + safe1);
		s = dmax(r__2,r__3);
	    }
/* L50: */
	}
	berr[j] = s;

/*        Test stopping criterion. Continue iterating if   
             1) The residual BERR(J) is larger than machine epsilon, and   
             2) BERR(J) decreased by at least a factor of 2 during the   
                last iteration, and   
             3) At most ITMAX iterations tried. */

	if (berr[j] > eps && berr[j] * 2.f <= lstres && count <= 5) {

/*           Update solution and try again. */

	    sgttrs_(trans, n, &c__1, &dlf[1], &df[1], &duf[1], &du2[1], &ipiv[
		    1], &work[*n + 1], n, info);
	    saxpy_(n, &c_b19, &work[*n + 1], &c__1, &x_ref(1, j), &c__1);
	    lstres = berr[j];
	    ++count;
	    goto L20;
	}

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

          Use SLACON to estimate the infinity-norm of the matrix   
             inv(op(A)) * diag(W),   
          where W = abs(R) + NZ*EPS*( abs(op(A))*abs(X)+abs(B) ))) */

	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    if (work[i__] > safe2) {
		work[i__] = (r__1 = work[*n + i__], dabs(r__1)) + nz * eps * 
			work[i__];
	    } else {
		work[i__] = (r__1 = work[*n + i__], dabs(r__1)) + nz * eps * 
			work[i__] + safe1;
	    }
/* L60: */
	}

	kase = 0;
L70:
	slacon_(n, &work[(*n << 1) + 1], &work[*n + 1], &iwork[1], &ferr[j], &
		kase);
	if (kase != 0) {
	    if (kase == 1) {

/*              Multiply by diag(W)*inv(op(A)**T). */

		sgttrs_(transt, n, &c__1, &dlf[1], &df[1], &duf[1], &du2[1], &
			ipiv[1], &work[*n + 1], n, info);
		i__2 = *n;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    work[*n + i__] = work[i__] * work[*n + i__];
/* L80: */
		}
	    } else {

/*              Multiply by inv(op(A))*diag(W). */

		i__2 = *n;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    work[*n + i__] = work[i__] * work[*n + i__];
/* L90: */
		}
		sgttrs_(transn, n, &c__1, &dlf[1], &df[1], &duf[1], &du2[1], &
			ipiv[1], &work[*n + 1], n, info);
	    }
	    goto L70;
	}

/*        Normalize error. */

	lstres = 0.f;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
	    r__2 = lstres, r__3 = (r__1 = x_ref(i__, j), dabs(r__1));
	    lstres = dmax(r__2,r__3);
/* L100: */
	}
	if (lstres != 0.f) {
	    ferr[j] /= lstres;
	}

/* L110: */
    }

    return 0;

/*     End of SGTRFS */

} /* sgtrfs_ */

#undef x_ref
#undef b_ref


