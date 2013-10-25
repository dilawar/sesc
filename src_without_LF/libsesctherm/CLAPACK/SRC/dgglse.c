#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dgglse_(integer *m, integer *n, integer *p, doublereal *
	a, integer *lda, doublereal *b, integer *ldb, doublereal *c__, 
	doublereal *d__, doublereal *x, doublereal *work, integer *lwork, 
	integer *info)
{
/*  -- LAPACK driver routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    DGGLSE solves the linear equality-constrained least squares (LSE)   
    problem:   

            minimize || c - A*x ||_2   subject to   B*x = d   

    where A is an M-by-N matrix, B is a P-by-N matrix, c is a given   
    M-vector, and d is a given P-vector. It is assumed that   
    P <= N <= M+P, and   

             rank(B) = P and  rank( ( A ) ) = N.   
                                  ( ( B ) )   

    These conditions ensure that the LSE problem has a unique solution,   
    which is obtained using a GRQ factorization of the matrices B and A.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrices A and B. N >= 0.   

    P       (input) INTEGER   
            The number of rows of the matrix B. 0 <= P <= N <= M+P.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the M-by-N matrix A.   
            On exit, A is destroyed.   

    LDA     (input) INTEGER   
            The leading dimension of the array A. LDA >= max(1,M).   

    B       (input/output) DOUBLE PRECISION array, dimension (LDB,N)   
            On entry, the P-by-N matrix B.   
            On exit, B is destroyed.   

    LDB     (input) INTEGER   
            The leading dimension of the array B. LDB >= max(1,P).   

    C       (input/output) DOUBLE PRECISION array, dimension (M)   
            On entry, C contains the right hand side vector for the   
            least squares part of the LSE problem.   
            On exit, the residual sum of squares for the solution   
            is given by the sum of squares of elements N-P+1 to M of   
            vector C.   

    D       (input/output) DOUBLE PRECISION array, dimension (P)   
            On entry, D contains the right hand side vector for the   
            constrained equation.   
            On exit, D is destroyed.   

    X       (output) DOUBLE PRECISION array, dimension (N)   
            On exit, X is the solution of the LSE problem.   

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)   
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK. LWORK >= max(1,M+N+P).   
            For optimum performance LWORK >= P+min(M,N)+max(M,N)*NB,   
            where NB is an upper bound for the optimal blocksizes for   
            DGEQRF, SGERQF, DORMQR and SORMRQ.   

            If LWORK = -1, then a workspace query is assumed; the routine   
            only calculates the optimal size of the WORK array, returns   
            this value as the first entry of the WORK array, and no error   
            message related to LWORK is issued by XERBLA.   

    INFO    (output) INTEGER   
            = 0:  successful exit.   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   

    =====================================================================   


       Test the input parameters   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static integer c_n1 = -1;
    static doublereal c_b29 = -1.;
    static doublereal c_b31 = 1.;
    
    /* System generated locals */
    integer a_dim1, a_offset, b_dim1, b_offset, i__1, i__2;
    /* Local variables */
    static integer lopt;
    extern /* Subroutine */ int dgemv_(char *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, doublereal *, integer *, 
	    doublereal *, doublereal *, integer *), dcopy_(integer *, 
	    doublereal *, integer *, doublereal *, integer *), daxpy_(integer 
	    *, doublereal *, doublereal *, integer *, doublereal *, integer *)
	    , dtrmv_(char *, char *, char *, integer *, doublereal *, integer 
	    *, doublereal *, integer *), dtrsv_(char *
	    , char *, char *, integer *, doublereal *, integer *, doublereal *
	    , integer *);
    static integer nb, mn, nr;
    extern /* Subroutine */ int dggrqf_(integer *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *, doublereal *, integer *, integer *), xerbla_(char *,
	     integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen);
    static integer nb1, nb2, nb3, nb4;
    extern /* Subroutine */ int dormqr_(char *, char *, integer *, integer *, 
	    integer *, doublereal *, integer *, doublereal *, doublereal *, 
	    integer *, doublereal *, integer *, integer *), 
	    dormrq_(char *, char *, integer *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *, integer *, integer *);
    static integer lwkopt;
    static logical lquery;
#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]
#define b_ref(a_1,a_2) b[(a_2)*b_dim1 + a_1]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    --c__;
    --d__;
    --x;
    --work;

    /* Function Body */
    *info = 0;
    mn = min(*m,*n);
    nb1 = ilaenv_(&c__1, "DGEQRF", " ", m, n, &c_n1, &c_n1, (ftnlen)6, (
	    ftnlen)1);
    nb2 = ilaenv_(&c__1, "DGERQF", " ", m, n, &c_n1, &c_n1, (ftnlen)6, (
	    ftnlen)1);
    nb3 = ilaenv_(&c__1, "DORMQR", " ", m, n, p, &c_n1, (ftnlen)6, (ftnlen)1);
    nb4 = ilaenv_(&c__1, "DORMRQ", " ", m, n, p, &c_n1, (ftnlen)6, (ftnlen)1);
/* Computing MAX */
    i__1 = max(nb1,nb2), i__1 = max(i__1,nb3);
    nb = max(i__1,nb4);
    lwkopt = *p + mn + max(*m,*n) * nb;
    work[1] = (doublereal) lwkopt;
    lquery = *lwork == -1;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*p < 0 || *p > *n || *p < *n - *m) {
	*info = -3;
    } else if (*lda < max(1,*m)) {
	*info = -5;
    } else if (*ldb < max(1,*p)) {
	*info = -7;
    } else /* if(complicated condition) */ {
/* Computing MAX */
	i__1 = 1, i__2 = *m + *n + *p;
	if (*lwork < max(i__1,i__2) && ! lquery) {
	    *info = -12;
	}
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGGLSE", &i__1);
	return 0;
    } else if (lquery) {
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Compute the GRQ factorization of matrices B and A:   

              B*Q' = (  0  T12 ) P   Z'*A*Q' = ( R11 R12 ) N-P   
                       N-P  P                  (  0  R22 ) M+P-N   
                                                 N-P  P   

       where T12 and R11 are upper triangular, and Q and Z are   
       orthogonal. */

    i__1 = *lwork - *p - mn;
    dggrqf_(p, m, n, &b[b_offset], ldb, &work[1], &a[a_offset], lda, &work[*p 
	    + 1], &work[*p + mn + 1], &i__1, info);
    lopt = (integer) work[*p + mn + 1];

/*     Update c = Z'*c = ( c1 ) N-P   
                         ( c2 ) M+P-N */

    i__1 = max(1,*m);
    i__2 = *lwork - *p - mn;
    dormqr_("Left", "Transpose", m, &c__1, &mn, &a[a_offset], lda, &work[*p + 
	    1], &c__[1], &i__1, &work[*p + mn + 1], &i__2, info);
/* Computing MAX */
    i__1 = lopt, i__2 = (integer) work[*p + mn + 1];
    lopt = max(i__1,i__2);

/*     Solve T12*x2 = d for x2 */

    dtrsv_("Upper", "No transpose", "Non unit", p, &b_ref(1, *n - *p + 1), 
	    ldb, &d__[1], &c__1);

/*     Update c1 */

    i__1 = *n - *p;
    dgemv_("No transpose", &i__1, p, &c_b29, &a_ref(1, *n - *p + 1), lda, &
	    d__[1], &c__1, &c_b31, &c__[1], &c__1);

/*     Sovle R11*x1 = c1 for x1 */

    i__1 = *n - *p;
    dtrsv_("Upper", "No transpose", "Non unit", &i__1, &a[a_offset], lda, &
	    c__[1], &c__1);

/*     Put the solutions in X */

    i__1 = *n - *p;
    dcopy_(&i__1, &c__[1], &c__1, &x[1], &c__1);
    dcopy_(p, &d__[1], &c__1, &x[*n - *p + 1], &c__1);

/*     Compute the residual vector: */

    if (*m < *n) {
	nr = *m + *p - *n;
	i__1 = *n - *m;
	dgemv_("No transpose", &nr, &i__1, &c_b29, &a_ref(*n - *p + 1, *m + 1)
		, lda, &d__[nr + 1], &c__1, &c_b31, &c__[*n - *p + 1], &c__1);
    } else {
	nr = *p;
    }
    dtrmv_("Upper", "No transpose", "Non unit", &nr, &a_ref(*n - *p + 1, *n - 
	    *p + 1), lda, &d__[1], &c__1);
    daxpy_(&nr, &c_b29, &d__[1], &c__1, &c__[*n - *p + 1], &c__1);

/*     Backward transformation x = Q'*x */

    i__1 = *lwork - *p - mn;
    dormrq_("Left", "Transpose", n, &c__1, p, &b[b_offset], ldb, &work[1], &x[
	    1], n, &work[*p + mn + 1], &i__1, info);
/* Computing MAX */
    i__1 = lopt, i__2 = (integer) work[*p + mn + 1];
    work[1] = (doublereal) (*p + mn + max(i__1,i__2));

    return 0;

/*     End of DGGLSE */

} /* dgglse_ */

#undef b_ref
#undef a_ref


