#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zsptrs_(char *uplo, integer *n, integer *nrhs, 
	doublecomplex *ap, integer *ipiv, doublecomplex *b, integer *ldb, 
	integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZSPTRS solves a system of linear equations A*X = B with a complex   
    symmetric matrix A stored in packed format using the factorization   
    A = U*D*U**T or A = L*D*L**T computed by ZSPTRF.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            Specifies whether the details of the factorization are stored   
            as an upper or lower triangular matrix.   
            = 'U':  Upper triangular, form is A = U*D*U**T;   
            = 'L':  Lower triangular, form is A = L*D*L**T.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    AP      (input) COMPLEX*16 array, dimension (N*(N+1)/2)   
            The block diagonal matrix D and the multipliers used to   
            obtain the factor U or L as computed by ZSPTRF, stored as a   
            packed triangular matrix.   

    IPIV    (input) INTEGER array, dimension (N)   
            Details of the interchanges and the block structure of D   
            as determined by ZSPTRF.   

    B       (input/output) COMPLEX*16 array, dimension (LDB,NRHS)   
            On entry, the right hand side matrix B.   
            On exit, the solution matrix X.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {1.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer b_dim1, b_offset, i__1, i__2;
    doublecomplex z__1, z__2, z__3;
    /* Builtin functions */
    void z_div(doublecomplex *, doublecomplex *, doublecomplex *);
    /* Local variables */
    static doublecomplex akm1k;
    static integer j, k;
    extern logical lsame_(char *, char *);
    static doublecomplex denom;
    extern /* Subroutine */ int zscal_(integer *, doublecomplex *, 
	    doublecomplex *, integer *), zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *);
    static logical upper;
    extern /* Subroutine */ int zgeru_(integer *, integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zswap_(integer *, doublecomplex *, 
	    integer *, doublecomplex *, integer *);
    static doublecomplex ak, bk;
    static integer kc, kp;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static doublecomplex akm1, bkm1;
#define b_subscr(a_1,a_2) (a_2)*b_dim1 + a_1
#define b_ref(a_1,a_2) b[b_subscr(a_1,a_2)]


    --ap;
    --ipiv;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;

    /* Function Body */
    *info = 0;
    upper = lsame_(uplo, "U");
    if (! upper && ! lsame_(uplo, "L")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*nrhs < 0) {
	*info = -3;
    } else if (*ldb < max(1,*n)) {
	*info = -7;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZSPTRS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0) {
	return 0;
    }

    if (upper) {

/*        Solve A*X = B, where A = U*D*U'.   

          First solve U*D*X = B, overwriting B with X.   

          K is the main loop index, decreasing from N to 1 in steps of   
          1 or 2, depending on the size of the diagonal blocks. */

	k = *n;
	kc = *n * (*n + 1) / 2 + 1;
L10:

/*        If K < 1, exit from loop. */

	if (k < 1) {
	    goto L30;
	}

	kc -= k;
	if (ipiv[k] > 0) {

/*           1 x 1 diagonal block   

             Interchange rows K and IPIV(K). */

	    kp = ipiv[k];
	    if (kp != k) {
		zswap_(nrhs, &b_ref(k, 1), ldb, &b_ref(kp, 1), ldb);
	    }

/*           Multiply by inv(U(K)), where U(K) is the transformation   
             stored in column K of A. */

	    i__1 = k - 1;
	    z__1.r = -1., z__1.i = 0.;
	    zgeru_(&i__1, nrhs, &z__1, &ap[kc], &c__1, &b_ref(k, 1), ldb, &
		    b_ref(1, 1), ldb);

/*           Multiply by the inverse of the diagonal block. */

	    z_div(&z__1, &c_b1, &ap[kc + k - 1]);
	    zscal_(nrhs, &z__1, &b_ref(k, 1), ldb);
	    --k;
	} else {

/*           2 x 2 diagonal block   

             Interchange rows K-1 and -IPIV(K). */

	    kp = -ipiv[k];
	    if (kp != k - 1) {
		zswap_(nrhs, &b_ref(k - 1, 1), ldb, &b_ref(kp, 1), ldb);
	    }

/*           Multiply by inv(U(K)), where U(K) is the transformation   
             stored in columns K-1 and K of A. */

	    i__1 = k - 2;
	    z__1.r = -1., z__1.i = 0.;
	    zgeru_(&i__1, nrhs, &z__1, &ap[kc], &c__1, &b_ref(k, 1), ldb, &
		    b_ref(1, 1), ldb);
	    i__1 = k - 2;
	    z__1.r = -1., z__1.i = 0.;
	    zgeru_(&i__1, nrhs, &z__1, &ap[kc - (k - 1)], &c__1, &b_ref(k - 1,
		     1), ldb, &b_ref(1, 1), ldb);

/*           Multiply by the inverse of the diagonal block. */

	    i__1 = kc + k - 2;
	    akm1k.r = ap[i__1].r, akm1k.i = ap[i__1].i;
	    z_div(&z__1, &ap[kc - 1], &akm1k);
	    akm1.r = z__1.r, akm1.i = z__1.i;
	    z_div(&z__1, &ap[kc + k - 1], &akm1k);
	    ak.r = z__1.r, ak.i = z__1.i;
	    z__2.r = akm1.r * ak.r - akm1.i * ak.i, z__2.i = akm1.r * ak.i + 
		    akm1.i * ak.r;
	    z__1.r = z__2.r - 1., z__1.i = z__2.i + 0.;
	    denom.r = z__1.r, denom.i = z__1.i;
	    i__1 = *nrhs;
	    for (j = 1; j <= i__1; ++j) {
		z_div(&z__1, &b_ref(k - 1, j), &akm1k);
		bkm1.r = z__1.r, bkm1.i = z__1.i;
		z_div(&z__1, &b_ref(k, j), &akm1k);
		bk.r = z__1.r, bk.i = z__1.i;
		i__2 = b_subscr(k - 1, j);
		z__3.r = ak.r * bkm1.r - ak.i * bkm1.i, z__3.i = ak.r * 
			bkm1.i + ak.i * bkm1.r;
		z__2.r = z__3.r - bk.r, z__2.i = z__3.i - bk.i;
		z_div(&z__1, &z__2, &denom);
		b[i__2].r = z__1.r, b[i__2].i = z__1.i;
		i__2 = b_subscr(k, j);
		z__3.r = akm1.r * bk.r - akm1.i * bk.i, z__3.i = akm1.r * 
			bk.i + akm1.i * bk.r;
		z__2.r = z__3.r - bkm1.r, z__2.i = z__3.i - bkm1.i;
		z_div(&z__1, &z__2, &denom);
		b[i__2].r = z__1.r, b[i__2].i = z__1.i;
/* L20: */
	    }
	    kc = kc - k + 1;
	    k += -2;
	}

	goto L10;
L30:

/*        Next solve U'*X = B, overwriting B with X.   

          K is the main loop index, increasing from 1 to N in steps of   
          1 or 2, depending on the size of the diagonal blocks. */

	k = 1;
	kc = 1;
L40:

/*        If K > N, exit from loop. */

	if (k > *n) {
	    goto L50;
	}

	if (ipiv[k] > 0) {

/*           1 x 1 diagonal block   

             Multiply by inv(U'(K)), where U(K) is the transformation   
             stored in column K of A. */

	    i__1 = k - 1;
	    z__1.r = -1., z__1.i = 0.;
	    zgemv_("Transpose", &i__1, nrhs, &z__1, &b[b_offset], ldb, &ap[kc]
		    , &c__1, &c_b1, &b_ref(k, 1), ldb);

/*           Interchange rows K and IPIV(K). */

	    kp = ipiv[k];
	    if (kp != k) {
		zswap_(nrhs, &b_ref(k, 1), ldb, &b_ref(kp, 1), ldb);
	    }
	    kc += k;
	    ++k;
	} else {

/*           2 x 2 diagonal block   

             Multiply by inv(U'(K+1)), where U(K+1) is the transformation   
             stored in columns K and K+1 of A. */

	    i__1 = k - 1;
	    z__1.r = -1., z__1.i = 0.;
	    zgemv_("Transpose", &i__1, nrhs, &z__1, &b[b_offset], ldb, &ap[kc]
		    , &c__1, &c_b1, &b_ref(k, 1), ldb);
	    i__1 = k - 1;
	    z__1.r = -1., z__1.i = 0.;
	    zgemv_("Transpose", &i__1, nrhs, &z__1, &b[b_offset], ldb, &ap[kc 
		    + k], &c__1, &c_b1, &b_ref(k + 1, 1), ldb);

/*           Interchange rows K and -IPIV(K). */

	    kp = -ipiv[k];
	    if (kp != k) {
		zswap_(nrhs, &b_ref(k, 1), ldb, &b_ref(kp, 1), ldb);
	    }
	    kc = kc + (k << 1) + 1;
	    k += 2;
	}

	goto L40;
L50:

	;
    } else {

/*        Solve A*X = B, where A = L*D*L'.   

          First solve L*D*X = B, overwriting B with X.   

          K is the main loop index, increasing from 1 to N in steps of   
          1 or 2, depending on the size of the diagonal blocks. */

	k = 1;
	kc = 1;
L60:

/*        If K > N, exit from loop. */

	if (k > *n) {
	    goto L80;
	}

	if (ipiv[k] > 0) {

/*           1 x 1 diagonal block   

             Interchange rows K and IPIV(K). */

	    kp = ipiv[k];
	    if (kp != k) {
		zswap_(nrhs, &b_ref(k, 1), ldb, &b_ref(kp, 1), ldb);
	    }

/*           Multiply by inv(L(K)), where L(K) is the transformation   
             stored in column K of A. */

	    if (k < *n) {
		i__1 = *n - k;
		z__1.r = -1., z__1.i = 0.;
		zgeru_(&i__1, nrhs, &z__1, &ap[kc + 1], &c__1, &b_ref(k, 1), 
			ldb, &b_ref(k + 1, 1), ldb);
	    }

/*           Multiply by the inverse of the diagonal block. */

	    z_div(&z__1, &c_b1, &ap[kc]);
	    zscal_(nrhs, &z__1, &b_ref(k, 1), ldb);
	    kc = kc + *n - k + 1;
	    ++k;
	} else {

/*           2 x 2 diagonal block   

             Interchange rows K+1 and -IPIV(K). */

	    kp = -ipiv[k];
	    if (kp != k + 1) {
		zswap_(nrhs, &b_ref(k + 1, 1), ldb, &b_ref(kp, 1), ldb);
	    }

/*           Multiply by inv(L(K)), where L(K) is the transformation   
             stored in columns K and K+1 of A. */

	    if (k < *n - 1) {
		i__1 = *n - k - 1;
		z__1.r = -1., z__1.i = 0.;
		zgeru_(&i__1, nrhs, &z__1, &ap[kc + 2], &c__1, &b_ref(k, 1), 
			ldb, &b_ref(k + 2, 1), ldb);
		i__1 = *n - k - 1;
		z__1.r = -1., z__1.i = 0.;
		zgeru_(&i__1, nrhs, &z__1, &ap[kc + *n - k + 2], &c__1, &
			b_ref(k + 1, 1), ldb, &b_ref(k + 2, 1), ldb);
	    }

/*           Multiply by the inverse of the diagonal block. */

	    i__1 = kc + 1;
	    akm1k.r = ap[i__1].r, akm1k.i = ap[i__1].i;
	    z_div(&z__1, &ap[kc], &akm1k);
	    akm1.r = z__1.r, akm1.i = z__1.i;
	    z_div(&z__1, &ap[kc + *n - k + 1], &akm1k);
	    ak.r = z__1.r, ak.i = z__1.i;
	    z__2.r = akm1.r * ak.r - akm1.i * ak.i, z__2.i = akm1.r * ak.i + 
		    akm1.i * ak.r;
	    z__1.r = z__2.r - 1., z__1.i = z__2.i + 0.;
	    denom.r = z__1.r, denom.i = z__1.i;
	    i__1 = *nrhs;
	    for (j = 1; j <= i__1; ++j) {
		z_div(&z__1, &b_ref(k, j), &akm1k);
		bkm1.r = z__1.r, bkm1.i = z__1.i;
		z_div(&z__1, &b_ref(k + 1, j), &akm1k);
		bk.r = z__1.r, bk.i = z__1.i;
		i__2 = b_subscr(k, j);
		z__3.r = ak.r * bkm1.r - ak.i * bkm1.i, z__3.i = ak.r * 
			bkm1.i + ak.i * bkm1.r;
		z__2.r = z__3.r - bk.r, z__2.i = z__3.i - bk.i;
		z_div(&z__1, &z__2, &denom);
		b[i__2].r = z__1.r, b[i__2].i = z__1.i;
		i__2 = b_subscr(k + 1, j);
		z__3.r = akm1.r * bk.r - akm1.i * bk.i, z__3.i = akm1.r * 
			bk.i + akm1.i * bk.r;
		z__2.r = z__3.r - bkm1.r, z__2.i = z__3.i - bkm1.i;
		z_div(&z__1, &z__2, &denom);
		b[i__2].r = z__1.r, b[i__2].i = z__1.i;
/* L70: */
	    }
	    kc = kc + (*n - k << 1) + 1;
	    k += 2;
	}

	goto L60;
L80:

/*        Next solve L'*X = B, overwriting B with X.   

          K is the main loop index, decreasing from N to 1 in steps of   
          1 or 2, depending on the size of the diagonal blocks. */

	k = *n;
	kc = *n * (*n + 1) / 2 + 1;
L90:

/*        If K < 1, exit from loop. */

	if (k < 1) {
	    goto L100;
	}

	kc -= *n - k + 1;
	if (ipiv[k] > 0) {

/*           1 x 1 diagonal block   

             Multiply by inv(L'(K)), where L(K) is the transformation   
             stored in column K of A. */

	    if (k < *n) {
		i__1 = *n - k;
		z__1.r = -1., z__1.i = 0.;
		zgemv_("Transpose", &i__1, nrhs, &z__1, &b_ref(k + 1, 1), ldb,
			 &ap[kc + 1], &c__1, &c_b1, &b_ref(k, 1), ldb);
	    }

/*           Interchange rows K and IPIV(K). */

	    kp = ipiv[k];
	    if (kp != k) {
		zswap_(nrhs, &b_ref(k, 1), ldb, &b_ref(kp, 1), ldb);
	    }
	    --k;
	} else {

/*           2 x 2 diagonal block   

             Multiply by inv(L'(K-1)), where L(K-1) is the transformation   
             stored in columns K-1 and K of A. */

	    if (k < *n) {
		i__1 = *n - k;
		z__1.r = -1., z__1.i = 0.;
		zgemv_("Transpose", &i__1, nrhs, &z__1, &b_ref(k + 1, 1), ldb,
			 &ap[kc + 1], &c__1, &c_b1, &b_ref(k, 1), ldb);
		i__1 = *n - k;
		z__1.r = -1., z__1.i = 0.;
		zgemv_("Transpose", &i__1, nrhs, &z__1, &b_ref(k + 1, 1), ldb,
			 &ap[kc - (*n - k)], &c__1, &c_b1, &b_ref(k - 1, 1), 
			ldb);
	    }

/*           Interchange rows K and -IPIV(K). */

	    kp = -ipiv[k];
	    if (kp != k) {
		zswap_(nrhs, &b_ref(k, 1), ldb, &b_ref(kp, 1), ldb);
	    }
	    kc -= *n - k + 2;
	    k += -2;
	}

	goto L90;
L100:
	;
    }

    return 0;

/*     End of ZSPTRS */

} /* zsptrs_ */

#undef b_ref
#undef b_subscr


