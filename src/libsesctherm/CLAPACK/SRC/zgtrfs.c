#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zgtrfs_(char *trans, integer *n, integer *nrhs, 
	doublecomplex *dl, doublecomplex *d__, doublecomplex *du, 
	doublecomplex *dlf, doublecomplex *df, doublecomplex *duf, 
	doublecomplex *du2, integer *ipiv, doublecomplex *b, integer *ldb, 
	doublecomplex *x, integer *ldx, doublereal *ferr, doublereal *berr, 
	doublecomplex *work, doublereal *rwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZGTRFS improves the computed solution to a system of linear   
    equations when the coefficient matrix is tridiagonal, and provides   
    error bounds and backward error estimates for the solution.   

    Arguments   
    =========   

    TRANS   (input) CHARACTER*1   
            Specifies the form of the system of equations:   
            = 'N':  A * X = B     (No transpose)   
            = 'T':  A**T * X = B  (Transpose)   
            = 'C':  A**H * X = B  (Conjugate transpose)   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    DL      (input) COMPLEX*16 array, dimension (N-1)   
            The (n-1) subdiagonal elements of A.   

    D       (input) COMPLEX*16 array, dimension (N)   
            The diagonal elements of A.   

    DU      (input) COMPLEX*16 array, dimension (N-1)   
            The (n-1) superdiagonal elements of A.   

    DLF     (input) COMPLEX*16 array, dimension (N-1)   
            The (n-1) multipliers that define the matrix L from the   
            LU factorization of A as computed by ZGTTRF.   

    DF      (input) COMPLEX*16 array, dimension (N)   
            The n diagonal elements of the upper triangular matrix U from   
            the LU factorization of A.   

    DUF     (input) COMPLEX*16 array, dimension (N-1)   
            The (n-1) elements of the first superdiagonal of U.   

    DU2     (input) COMPLEX*16 array, dimension (N-2)   
            The (n-2) elements of the second superdiagonal of U.   

    IPIV    (input) INTEGER array, dimension (N)   
            The pivot indices; for 1 <= i <= n, row i of the matrix was   
            interchanged with row IPIV(i).  IPIV(i) will always be either   
            i or i+1; IPIV(i) = i indicates a row interchange was not   
            required.   

    B       (input) COMPLEX*16 array, dimension (LDB,NRHS)   
            The right hand side matrix B.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    X       (input/output) COMPLEX*16 array, dimension (LDX,NRHS)   
            On entry, the solution matrix X, as computed by ZGTTRS.   
            On exit, the improved solution matrix X.   

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

    WORK    (workspace) COMPLEX*16 array, dimension (2*N)   

    RWORK   (workspace) DOUBLE PRECISION array, dimension (N)   

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
    static doublereal c_b18 = -1.;
    static doublereal c_b19 = 1.;
    static doublecomplex c_b26 = {1.,0.};
    
    /* System generated locals */
    integer b_dim1, b_offset, x_dim1, x_offset, i__1, i__2, i__3, i__4, i__5, 
	    i__6, i__7, i__8, i__9;
    doublereal d__1, d__2, d__3, d__4, d__5, d__6, d__7, d__8, d__9, d__10, 
	    d__11, d__12, d__13, d__14;
    doublecomplex z__1;
    /* Builtin functions */
    double d_imag(doublecomplex *);
    /* Local variables */
    static integer kase;
    static doublereal safe1, safe2;
    static integer i__, j;
    static doublereal s;
    extern logical lsame_(char *, char *);
    static integer count;
    extern /* Subroutine */ int zcopy_(integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zaxpy_(integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *);
    extern doublereal dlamch_(char *);
    static integer nz;
    static doublereal safmin;
    extern /* Subroutine */ int xerbla_(char *, integer *), zlacon_(
	    integer *, doublecomplex *, doublecomplex *, doublereal *, 
	    integer *), zlagtm_(char *, integer *, integer *, doublereal *, 
	    doublecomplex *, doublecomplex *, doublecomplex *, doublecomplex *
	    , integer *, doublereal *, doublecomplex *, integer *);
    static logical notran;
    static char transn[1], transt[1];
    static doublereal lstres;
    extern /* Subroutine */ int zgttrs_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, doublecomplex *, doublecomplex *
	    , integer *, doublecomplex *, integer *, integer *);
    static doublereal eps;
#define b_subscr(a_1,a_2) (a_2)*b_dim1 + a_1
#define b_ref(a_1,a_2) b[b_subscr(a_1,a_2)]
#define x_subscr(a_1,a_2) (a_2)*x_dim1 + a_1
#define x_ref(a_1,a_2) x[x_subscr(a_1,a_2)]


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
    --rwork;

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
	xerbla_("ZGTRFS", &i__1);
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
	*(unsigned char *)transn = 'N';
	*(unsigned char *)transt = 'C';
    } else {
	*(unsigned char *)transn = 'C';
	*(unsigned char *)transt = 'N';
    }

/*     NZ = maximum number of nonzero elements in each row of A, plus 1 */

    nz = 4;
    eps = dlamch_("Epsilon");
    safmin = dlamch_("Safe minimum");
    safe1 = nz * safmin;
    safe2 = safe1 / eps;

/*     Do for each right hand side */

    i__1 = *nrhs;
    for (j = 1; j <= i__1; ++j) {

	count = 1;
	lstres = 3.;
L20:

/*        Loop until stopping criterion is satisfied.   

          Compute residual R = B - op(A) * X,   
          where op(A) = A, A**T, or A**H, depending on TRANS. */

	zcopy_(n, &b_ref(1, j), &c__1, &work[1], &c__1);
	zlagtm_(trans, n, &c__1, &c_b18, &dl[1], &d__[1], &du[1], &x_ref(1, j)
		, ldx, &c_b19, &work[1], n);

/*        Compute abs(op(A))*abs(x) + abs(b) for use in the backward   
          error bound. */

	if (notran) {
	    if (*n == 1) {
		i__2 = b_subscr(1, j);
		i__3 = x_subscr(1, j);
		rwork[1] = (d__1 = b[i__2].r, abs(d__1)) + (d__2 = d_imag(&
			b_ref(1, j)), abs(d__2)) + ((d__3 = d__[1].r, abs(
			d__3)) + (d__4 = d_imag(&d__[1]), abs(d__4))) * ((
			d__5 = x[i__3].r, abs(d__5)) + (d__6 = d_imag(&x_ref(
			1, j)), abs(d__6)));
	    } else {
		i__2 = b_subscr(1, j);
		i__3 = x_subscr(1, j);
		i__4 = x_subscr(2, j);
		rwork[1] = (d__1 = b[i__2].r, abs(d__1)) + (d__2 = d_imag(&
			b_ref(1, j)), abs(d__2)) + ((d__3 = d__[1].r, abs(
			d__3)) + (d__4 = d_imag(&d__[1]), abs(d__4))) * ((
			d__5 = x[i__3].r, abs(d__5)) + (d__6 = d_imag(&x_ref(
			1, j)), abs(d__6))) + ((d__7 = du[1].r, abs(d__7)) + (
			d__8 = d_imag(&du[1]), abs(d__8))) * ((d__9 = x[i__4]
			.r, abs(d__9)) + (d__10 = d_imag(&x_ref(2, j)), abs(
			d__10)));
		i__2 = *n - 1;
		for (i__ = 2; i__ <= i__2; ++i__) {
		    i__3 = b_subscr(i__, j);
		    i__4 = i__ - 1;
		    i__5 = x_subscr(i__ - 1, j);
		    i__6 = i__;
		    i__7 = x_subscr(i__, j);
		    i__8 = i__;
		    i__9 = x_subscr(i__ + 1, j);
		    rwork[i__] = (d__1 = b[i__3].r, abs(d__1)) + (d__2 = 
			    d_imag(&b_ref(i__, j)), abs(d__2)) + ((d__3 = dl[
			    i__4].r, abs(d__3)) + (d__4 = d_imag(&dl[i__ - 1])
			    , abs(d__4))) * ((d__5 = x[i__5].r, abs(d__5)) + (
			    d__6 = d_imag(&x_ref(i__ - 1, j)), abs(d__6))) + (
			    (d__7 = d__[i__6].r, abs(d__7)) + (d__8 = d_imag(&
			    d__[i__]), abs(d__8))) * ((d__9 = x[i__7].r, abs(
			    d__9)) + (d__10 = d_imag(&x_ref(i__, j)), abs(
			    d__10))) + ((d__11 = du[i__8].r, abs(d__11)) + (
			    d__12 = d_imag(&du[i__]), abs(d__12))) * ((d__13 =
			     x[i__9].r, abs(d__13)) + (d__14 = d_imag(&x_ref(
			    i__ + 1, j)), abs(d__14)));
/* L30: */
		}
		i__2 = b_subscr(*n, j);
		i__3 = *n - 1;
		i__4 = x_subscr(*n - 1, j);
		i__5 = *n;
		i__6 = x_subscr(*n, j);
		rwork[*n] = (d__1 = b[i__2].r, abs(d__1)) + (d__2 = d_imag(&
			b_ref(*n, j)), abs(d__2)) + ((d__3 = dl[i__3].r, abs(
			d__3)) + (d__4 = d_imag(&dl[*n - 1]), abs(d__4))) * ((
			d__5 = x[i__4].r, abs(d__5)) + (d__6 = d_imag(&x_ref(*
			n - 1, j)), abs(d__6))) + ((d__7 = d__[i__5].r, abs(
			d__7)) + (d__8 = d_imag(&d__[*n]), abs(d__8))) * ((
			d__9 = x[i__6].r, abs(d__9)) + (d__10 = d_imag(&x_ref(
			*n, j)), abs(d__10)));
	    }
	} else {
	    if (*n == 1) {
		i__2 = b_subscr(1, j);
		i__3 = x_subscr(1, j);
		rwork[1] = (d__1 = b[i__2].r, abs(d__1)) + (d__2 = d_imag(&
			b_ref(1, j)), abs(d__2)) + ((d__3 = d__[1].r, abs(
			d__3)) + (d__4 = d_imag(&d__[1]), abs(d__4))) * ((
			d__5 = x[i__3].r, abs(d__5)) + (d__6 = d_imag(&x_ref(
			1, j)), abs(d__6)));
	    } else {
		i__2 = b_subscr(1, j);
		i__3 = x_subscr(1, j);
		i__4 = x_subscr(2, j);
		rwork[1] = (d__1 = b[i__2].r, abs(d__1)) + (d__2 = d_imag(&
			b_ref(1, j)), abs(d__2)) + ((d__3 = d__[1].r, abs(
			d__3)) + (d__4 = d_imag(&d__[1]), abs(d__4))) * ((
			d__5 = x[i__3].r, abs(d__5)) + (d__6 = d_imag(&x_ref(
			1, j)), abs(d__6))) + ((d__7 = dl[1].r, abs(d__7)) + (
			d__8 = d_imag(&dl[1]), abs(d__8))) * ((d__9 = x[i__4]
			.r, abs(d__9)) + (d__10 = d_imag(&x_ref(2, j)), abs(
			d__10)));
		i__2 = *n - 1;
		for (i__ = 2; i__ <= i__2; ++i__) {
		    i__3 = b_subscr(i__, j);
		    i__4 = i__ - 1;
		    i__5 = x_subscr(i__ - 1, j);
		    i__6 = i__;
		    i__7 = x_subscr(i__, j);
		    i__8 = i__;
		    i__9 = x_subscr(i__ + 1, j);
		    rwork[i__] = (d__1 = b[i__3].r, abs(d__1)) + (d__2 = 
			    d_imag(&b_ref(i__, j)), abs(d__2)) + ((d__3 = du[
			    i__4].r, abs(d__3)) + (d__4 = d_imag(&du[i__ - 1])
			    , abs(d__4))) * ((d__5 = x[i__5].r, abs(d__5)) + (
			    d__6 = d_imag(&x_ref(i__ - 1, j)), abs(d__6))) + (
			    (d__7 = d__[i__6].r, abs(d__7)) + (d__8 = d_imag(&
			    d__[i__]), abs(d__8))) * ((d__9 = x[i__7].r, abs(
			    d__9)) + (d__10 = d_imag(&x_ref(i__, j)), abs(
			    d__10))) + ((d__11 = dl[i__8].r, abs(d__11)) + (
			    d__12 = d_imag(&dl[i__]), abs(d__12))) * ((d__13 =
			     x[i__9].r, abs(d__13)) + (d__14 = d_imag(&x_ref(
			    i__ + 1, j)), abs(d__14)));
/* L40: */
		}
		i__2 = b_subscr(*n, j);
		i__3 = *n - 1;
		i__4 = x_subscr(*n - 1, j);
		i__5 = *n;
		i__6 = x_subscr(*n, j);
		rwork[*n] = (d__1 = b[i__2].r, abs(d__1)) + (d__2 = d_imag(&
			b_ref(*n, j)), abs(d__2)) + ((d__3 = du[i__3].r, abs(
			d__3)) + (d__4 = d_imag(&du[*n - 1]), abs(d__4))) * ((
			d__5 = x[i__4].r, abs(d__5)) + (d__6 = d_imag(&x_ref(*
			n - 1, j)), abs(d__6))) + ((d__7 = d__[i__5].r, abs(
			d__7)) + (d__8 = d_imag(&d__[*n]), abs(d__8))) * ((
			d__9 = x[i__6].r, abs(d__9)) + (d__10 = d_imag(&x_ref(
			*n, j)), abs(d__10)));
	    }
	}

/*        Compute componentwise relative backward error from formula   

          max(i) ( abs(R(i)) / ( abs(op(A))*abs(X) + abs(B) )(i) )   

          where abs(Z) is the componentwise absolute value of the matrix   
          or vector Z.  If the i-th component of the denominator is less   
          than SAFE2, then SAFE1 is added to the i-th components of the   
          numerator and denominator before dividing. */

	s = 0.;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    if (rwork[i__] > safe2) {
/* Computing MAX */
		i__3 = i__;
		d__3 = s, d__4 = ((d__1 = work[i__3].r, abs(d__1)) + (d__2 = 
			d_imag(&work[i__]), abs(d__2))) / rwork[i__];
		s = max(d__3,d__4);
	    } else {
/* Computing MAX */
		i__3 = i__;
		d__3 = s, d__4 = ((d__1 = work[i__3].r, abs(d__1)) + (d__2 = 
			d_imag(&work[i__]), abs(d__2)) + safe1) / (rwork[i__] 
			+ safe1);
		s = max(d__3,d__4);
	    }
/* L50: */
	}
	berr[j] = s;

/*        Test stopping criterion. Continue iterating if   
             1) The residual BERR(J) is larger than machine epsilon, and   
             2) BERR(J) decreased by at least a factor of 2 during the   
                last iteration, and   
             3) At most ITMAX iterations tried. */

	if (berr[j] > eps && berr[j] * 2. <= lstres && count <= 5) {

/*           Update solution and try again. */

	    zgttrs_(trans, n, &c__1, &dlf[1], &df[1], &duf[1], &du2[1], &ipiv[
		    1], &work[1], n, info);
	    zaxpy_(n, &c_b26, &work[1], &c__1, &x_ref(1, j), &c__1);
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

          Use ZLACON to estimate the infinity-norm of the matrix   
             inv(op(A)) * diag(W),   
          where W = abs(R) + NZ*EPS*( abs(op(A))*abs(X)+abs(B) ))) */

	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    if (rwork[i__] > safe2) {
		i__3 = i__;
		rwork[i__] = (d__1 = work[i__3].r, abs(d__1)) + (d__2 = 
			d_imag(&work[i__]), abs(d__2)) + nz * eps * rwork[i__]
			;
	    } else {
		i__3 = i__;
		rwork[i__] = (d__1 = work[i__3].r, abs(d__1)) + (d__2 = 
			d_imag(&work[i__]), abs(d__2)) + nz * eps * rwork[i__]
			 + safe1;
	    }
/* L60: */
	}

	kase = 0;
L70:
	zlacon_(n, &work[*n + 1], &work[1], &ferr[j], &kase);
	if (kase != 0) {
	    if (kase == 1) {

/*              Multiply by diag(W)*inv(op(A)**H). */

		zgttrs_(transt, n, &c__1, &dlf[1], &df[1], &duf[1], &du2[1], &
			ipiv[1], &work[1], n, info);
		i__2 = *n;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    i__3 = i__;
		    i__4 = i__;
		    i__5 = i__;
		    z__1.r = rwork[i__4] * work[i__5].r, z__1.i = rwork[i__4] 
			    * work[i__5].i;
		    work[i__3].r = z__1.r, work[i__3].i = z__1.i;
/* L80: */
		}
	    } else {

/*              Multiply by inv(op(A))*diag(W). */

		i__2 = *n;
		for (i__ = 1; i__ <= i__2; ++i__) {
		    i__3 = i__;
		    i__4 = i__;
		    i__5 = i__;
		    z__1.r = rwork[i__4] * work[i__5].r, z__1.i = rwork[i__4] 
			    * work[i__5].i;
		    work[i__3].r = z__1.r, work[i__3].i = z__1.i;
/* L90: */
		}
		zgttrs_(transn, n, &c__1, &dlf[1], &df[1], &duf[1], &du2[1], &
			ipiv[1], &work[1], n, info);
	    }
	    goto L70;
	}

/*        Normalize error. */

	lstres = 0.;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
	    i__3 = x_subscr(i__, j);
	    d__3 = lstres, d__4 = (d__1 = x[i__3].r, abs(d__1)) + (d__2 = 
		    d_imag(&x_ref(i__, j)), abs(d__2));
	    lstres = max(d__3,d__4);
/* L100: */
	}
	if (lstres != 0.) {
	    ferr[j] /= lstres;
	}

/* L110: */
    }

    return 0;

/*     End of ZGTRFS */

} /* zgtrfs_ */

#undef x_ref
#undef x_subscr
#undef b_ref
#undef b_subscr


