#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zgbtrs_(char *trans, integer *n, integer *kl, integer *
	ku, integer *nrhs, doublecomplex *ab, integer *ldab, integer *ipiv, 
	doublecomplex *b, integer *ldb, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZGBTRS solves a system of linear equations   
       A * X = B,  A**T * X = B,  or  A**H * X = B   
    with a general band matrix A using the LU factorization computed   
    by ZGBTRF.   

    Arguments   
    =========   

    TRANS   (input) CHARACTER*1   
            Specifies the form of the system of equations.   
            = 'N':  A * X = B     (No transpose)   
            = 'T':  A**T * X = B  (Transpose)   
            = 'C':  A**H * X = B  (Conjugate transpose)   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    KL      (input) INTEGER   
            The number of subdiagonals within the band of A.  KL >= 0.   

    KU      (input) INTEGER   
            The number of superdiagonals within the band of A.  KU >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of columns   
            of the matrix B.  NRHS >= 0.   

    AB      (input) COMPLEX*16 array, dimension (LDAB,N)   
            Details of the LU factorization of the band matrix A, as   
            computed by ZGBTRF.  U is stored as an upper triangular band   
            matrix with KL+KU superdiagonals in rows 1 to KL+KU+1, and   
            the multipliers used during the factorization are stored in   
            rows KL+KU+2 to 2*KL+KU+1.   

    LDAB    (input) INTEGER   
            The leading dimension of the array AB.  LDAB >= 2*KL+KU+1.   

    IPIV    (input) INTEGER array, dimension (N)   
            The pivot indices; for 1 <= i <= N, row i of the matrix was   
            interchanged with row IPIV(i).   

    B       (input/output) COMPLEX*16 array, dimension (LDB,NRHS)   
            On entry, the right hand side matrix B.   
            On exit, the solution matrix X.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {1.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer ab_dim1, ab_offset, b_dim1, b_offset, i__1, i__2, i__3;
    doublecomplex z__1;
    /* Local variables */
    static integer i__, j, l;
    extern logical lsame_(char *, char *);
    static logical lnoti;
    extern /* Subroutine */ int zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *), 
	    zgeru_(integer *, integer *, doublecomplex *, doublecomplex *, 
	    integer *, doublecomplex *, integer *, doublecomplex *, integer *)
	    , zswap_(integer *, doublecomplex *, integer *, doublecomplex *, 
	    integer *), ztbsv_(char *, char *, char *, integer *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *);
    static integer kd, lm;
    extern /* Subroutine */ int xerbla_(char *, integer *), zlacgv_(
	    integer *, doublecomplex *, integer *);
    static logical notran;
#define b_subscr(a_1,a_2) (a_2)*b_dim1 + a_1
#define b_ref(a_1,a_2) b[b_subscr(a_1,a_2)]
#define ab_subscr(a_1,a_2) (a_2)*ab_dim1 + a_1
#define ab_ref(a_1,a_2) ab[ab_subscr(a_1,a_2)]


    ab_dim1 = *ldab;
    ab_offset = 1 + ab_dim1 * 1;
    ab -= ab_offset;
    --ipiv;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;

    /* Function Body */
    *info = 0;
    notran = lsame_(trans, "N");
    if (! notran && ! lsame_(trans, "T") && ! lsame_(
	    trans, "C")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*kl < 0) {
	*info = -3;
    } else if (*ku < 0) {
	*info = -4;
    } else if (*nrhs < 0) {
	*info = -5;
    } else if (*ldab < (*kl << 1) + *ku + 1) {
	*info = -7;
    } else if (*ldb < max(1,*n)) {
	*info = -10;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZGBTRS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0) {
	return 0;
    }

    kd = *ku + *kl + 1;
    lnoti = *kl > 0;

    if (notran) {

/*        Solve  A*X = B.   

          Solve L*X = B, overwriting B with X.   

          L is represented as a product of permutations and unit lower   
          triangular matrices L = P(1) * L(1) * ... * P(n-1) * L(n-1),   
          where each transformation L(i) is a rank-one modification of   
          the identity matrix. */

	if (lnoti) {
	    i__1 = *n - 1;
	    for (j = 1; j <= i__1; ++j) {
/* Computing MIN */
		i__2 = *kl, i__3 = *n - j;
		lm = min(i__2,i__3);
		l = ipiv[j];
		if (l != j) {
		    zswap_(nrhs, &b_ref(l, 1), ldb, &b_ref(j, 1), ldb);
		}
		z__1.r = -1., z__1.i = 0.;
		zgeru_(&lm, nrhs, &z__1, &ab_ref(kd + 1, j), &c__1, &b_ref(j, 
			1), ldb, &b_ref(j + 1, 1), ldb);
/* L10: */
	    }
	}

	i__1 = *nrhs;
	for (i__ = 1; i__ <= i__1; ++i__) {

/*           Solve U*X = B, overwriting B with X. */

	    i__2 = *kl + *ku;
	    ztbsv_("Upper", "No transpose", "Non-unit", n, &i__2, &ab[
		    ab_offset], ldab, &b_ref(1, i__), &c__1);
/* L20: */
	}

    } else if (lsame_(trans, "T")) {

/*        Solve A**T * X = B. */

	i__1 = *nrhs;
	for (i__ = 1; i__ <= i__1; ++i__) {

/*           Solve U**T * X = B, overwriting B with X. */

	    i__2 = *kl + *ku;
	    ztbsv_("Upper", "Transpose", "Non-unit", n, &i__2, &ab[ab_offset],
		     ldab, &b_ref(1, i__), &c__1);
/* L30: */
	}

/*        Solve L**T * X = B, overwriting B with X. */

	if (lnoti) {
	    for (j = *n - 1; j >= 1; --j) {
/* Computing MIN */
		i__1 = *kl, i__2 = *n - j;
		lm = min(i__1,i__2);
		z__1.r = -1., z__1.i = 0.;
		zgemv_("Transpose", &lm, nrhs, &z__1, &b_ref(j + 1, 1), ldb, &
			ab_ref(kd + 1, j), &c__1, &c_b1, &b_ref(j, 1), ldb);
		l = ipiv[j];
		if (l != j) {
		    zswap_(nrhs, &b_ref(l, 1), ldb, &b_ref(j, 1), ldb);
		}
/* L40: */
	    }
	}

    } else {

/*        Solve A**H * X = B. */

	i__1 = *nrhs;
	for (i__ = 1; i__ <= i__1; ++i__) {

/*           Solve U**H * X = B, overwriting B with X. */

	    i__2 = *kl + *ku;
	    ztbsv_("Upper", "Conjugate transpose", "Non-unit", n, &i__2, &ab[
		    ab_offset], ldab, &b_ref(1, i__), &c__1);
/* L50: */
	}

/*        Solve L**H * X = B, overwriting B with X. */

	if (lnoti) {
	    for (j = *n - 1; j >= 1; --j) {
/* Computing MIN */
		i__1 = *kl, i__2 = *n - j;
		lm = min(i__1,i__2);
		zlacgv_(nrhs, &b_ref(j, 1), ldb);
		z__1.r = -1., z__1.i = 0.;
		zgemv_("Conjugate transpose", &lm, nrhs, &z__1, &b_ref(j + 1, 
			1), ldb, &ab_ref(kd + 1, j), &c__1, &c_b1, &b_ref(j, 
			1), ldb);
		zlacgv_(nrhs, &b_ref(j, 1), ldb);
		l = ipiv[j];
		if (l != j) {
		    zswap_(nrhs, &b_ref(l, 1), ldb, &b_ref(j, 1), ldb);
		}
/* L60: */
	    }
	}
    }
    return 0;

/*     End of ZGBTRS */

} /* zgbtrs_ */

#undef ab_ref
#undef ab_subscr
#undef b_ref
#undef b_subscr


