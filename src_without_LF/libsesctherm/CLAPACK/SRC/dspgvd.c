#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dspgvd_(integer *itype, char *jobz, char *uplo, integer *
	n, doublereal *ap, doublereal *bp, doublereal *w, doublereal *z__, 
	integer *ldz, doublereal *work, integer *lwork, integer *iwork, 
	integer *liwork, integer *info)
{
/*  -- LAPACK driver routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    DSPGVD computes all the eigenvalues, and optionally, the eigenvectors   
    of a real generalized symmetric-definite eigenproblem, of the form   
    A*x=(lambda)*B*x,  A*Bx=(lambda)*x,  or B*A*x=(lambda)*x.  Here A and   
    B are assumed to be symmetric, stored in packed format, and B is also   
    positive definite.   
    If eigenvectors are desired, it uses a divide and conquer algorithm.   

    The divide and conquer algorithm makes very mild assumptions about   
    floating point arithmetic. It will work on machines with a guard   
    digit in add/subtract, or on those binary machines without guard   
    digits which subtract like the Cray X-MP, Cray Y-MP, Cray C-90, or   
    Cray-2. It could conceivably fail on hexadecimal or decimal machines   
    without guard digits, but we know of none.   

    Arguments   
    =========   

    ITYPE   (input) INTEGER   
            Specifies the problem type to be solved:   
            = 1:  A*x = (lambda)*B*x   
            = 2:  A*B*x = (lambda)*x   
            = 3:  B*A*x = (lambda)*x   

    JOBZ    (input) CHARACTER*1   
            = 'N':  Compute eigenvalues only;   
            = 'V':  Compute eigenvalues and eigenvectors.   

    UPLO    (input) CHARACTER*1   
            = 'U':  Upper triangles of A and B are stored;   
            = 'L':  Lower triangles of A and B are stored.   

    N       (input) INTEGER   
            The order of the matrices A and B.  N >= 0.   

    AP      (input/output) DOUBLE PRECISION array, dimension (N*(N+1)/2)   
            On entry, the upper or lower triangle of the symmetric matrix   
            A, packed columnwise in a linear array.  The j-th column of A   
            is stored in the array AP as follows:   
            if UPLO = 'U', AP(i + (j-1)*j/2) = A(i,j) for 1<=i<=j;   
            if UPLO = 'L', AP(i + (j-1)*(2*n-j)/2) = A(i,j) for j<=i<=n.   

            On exit, the contents of AP are destroyed.   

    BP      (input/output) DOUBLE PRECISION array, dimension (N*(N+1)/2)   
            On entry, the upper or lower triangle of the symmetric matrix   
            B, packed columnwise in a linear array.  The j-th column of B   
            is stored in the array BP as follows:   
            if UPLO = 'U', BP(i + (j-1)*j/2) = B(i,j) for 1<=i<=j;   
            if UPLO = 'L', BP(i + (j-1)*(2*n-j)/2) = B(i,j) for j<=i<=n.   

            On exit, the triangular factor U or L from the Cholesky   
            factorization B = U**T*U or B = L*L**T, in the same storage   
            format as B.   

    W       (output) DOUBLE PRECISION array, dimension (N)   
            If INFO = 0, the eigenvalues in ascending order.   

    Z       (output) DOUBLE PRECISION array, dimension (LDZ, N)   
            If JOBZ = 'V', then if INFO = 0, Z contains the matrix Z of   
            eigenvectors.  The eigenvectors are normalized as follows:   
            if ITYPE = 1 or 2, Z**T*B*Z = I;   
            if ITYPE = 3, Z**T*inv(B)*Z = I.   
            If JOBZ = 'N', then Z is not referenced.   

    LDZ     (input) INTEGER   
            The leading dimension of the array Z.  LDZ >= 1, and if   
            JOBZ = 'V', LDZ >= max(1,N).   

    WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)   
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK.   
            If N <= 1,               LWORK >= 1.   
            If JOBZ = 'N' and N > 1, LWORK >= 2*N.   
            If JOBZ = 'V' and N > 1, LWORK >= 1 + 6*N + 2*N**2.   

            If LWORK = -1, then a workspace query is assumed; the routine   
            only calculates the optimal size of the WORK array, returns   
            this value as the first entry of the WORK array, and no error   
            message related to LWORK is issued by XERBLA.   

    IWORK   (workspace/output) INTEGER array, dimension (LIWORK)   
            On exit, if INFO = 0, IWORK(1) returns the optimal LIWORK.   

    LIWORK  (input) INTEGER   
            The dimension of the array IWORK.   
            If JOBZ  = 'N' or N <= 1, LIWORK >= 1.   
            If JOBZ  = 'V' and N > 1, LIWORK >= 3 + 5*N.   

            If LIWORK = -1, then a workspace query is assumed; the   
            routine only calculates the optimal size of the IWORK array,   
            returns this value as the first entry of the IWORK array, and   
            no error message related to LIWORK is issued by XERBLA.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   
            > 0:  DPPTRF or DSPEVD returned an error code:   
               <= N:  if INFO = i, DSPEVD failed to converge;   
                      i off-diagonal elements of an intermediate   
                      tridiagonal form did not converge to zero;   
               > N:   if INFO = N + i, for 1 <= i <= N, then the leading   
                      minor of order i of B is not positive definite.   
                      The factorization of B could not be completed and   
                      no eigenvalues or eigenvectors were computed.   

    Further Details   
    ===============   

    Based on contributions by   
       Mark Fahey, Department of Mathematics, Univ. of Kentucky, USA   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__2 = 2;
    static integer c__1 = 1;
    
    /* System generated locals */
    integer z_dim1, z_offset, i__1;
    doublereal d__1, d__2;
    /* Builtin functions */
    double log(doublereal);
    integer pow_ii(integer *, integer *);
    /* Local variables */
    static integer neig, j;
    extern logical lsame_(char *, char *);
    static integer lwmin;
    static char trans[1];
    static logical upper;
    extern /* Subroutine */ int dtpmv_(char *, char *, char *, integer *, 
	    doublereal *, doublereal *, integer *), 
	    dtpsv_(char *, char *, char *, integer *, doublereal *, 
	    doublereal *, integer *);
    static logical wantz;
    extern /* Subroutine */ int xerbla_(char *, integer *), dspevd_(
	    char *, char *, integer *, doublereal *, doublereal *, doublereal 
	    *, integer *, doublereal *, integer *, integer *, integer *, 
	    integer *);
    static integer liwmin;
    extern /* Subroutine */ int dpptrf_(char *, integer *, doublereal *, 
	    integer *), dspgst_(integer *, char *, integer *, 
	    doublereal *, doublereal *, integer *);
    static logical lquery;
    static integer lgn;
#define z___ref(a_1,a_2) z__[(a_2)*z_dim1 + a_1]


    --ap;
    --bp;
    --w;
    z_dim1 = *ldz;
    z_offset = 1 + z_dim1 * 1;
    z__ -= z_offset;
    --work;
    --iwork;

    /* Function Body */
    wantz = lsame_(jobz, "V");
    upper = lsame_(uplo, "U");
    lquery = *lwork == -1 || *liwork == -1;

    *info = 0;
    if (*n <= 1) {
	lgn = 0;
	liwmin = 1;
	lwmin = 1;
    } else {
	lgn = (integer) (log((doublereal) (*n)) / log(2.));
	if (pow_ii(&c__2, &lgn) < *n) {
	    ++lgn;
	}
	if (pow_ii(&c__2, &lgn) < *n) {
	    ++lgn;
	}
	if (wantz) {
	    liwmin = *n * 5 + 3;
/* Computing 2nd power */
	    i__1 = *n;
	    lwmin = *n * 5 + 1 + (*n << 1) * lgn + (i__1 * i__1 << 1);
	} else {
	    liwmin = 1;
	    lwmin = *n << 1;
	}
    }

    if (*itype < 0 || *itype > 3) {
	*info = -1;
    } else if (! (wantz || lsame_(jobz, "N"))) {
	*info = -2;
    } else if (! (upper || lsame_(uplo, "L"))) {
	*info = -3;
    } else if (*n < 0) {
	*info = -4;
    } else if (*ldz < max(1,*n)) {
	*info = -9;
    } else if (*lwork < lwmin && ! lquery) {
	*info = -11;
    } else if (*liwork < liwmin && ! lquery) {
	*info = -13;
    }

    if (*info == 0) {
	work[1] = (doublereal) lwmin;
	iwork[1] = liwmin;
    }

    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DSPGVD", &i__1);
	return 0;
    } else if (lquery) {
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

/*     Form a Cholesky factorization of BP. */

    dpptrf_(uplo, n, &bp[1], info);
    if (*info != 0) {
	*info = *n + *info;
	return 0;
    }

/*     Transform problem to standard eigenvalue problem and solve. */

    dspgst_(itype, uplo, n, &ap[1], &bp[1], info);
    dspevd_(jobz, uplo, n, &ap[1], &w[1], &z__[z_offset], ldz, &work[1], 
	    lwork, &iwork[1], liwork, info);
/* Computing MAX */
    d__1 = (doublereal) lwmin;
    lwmin = (integer) max(d__1,work[1]);
/* Computing MAX */
    d__1 = (doublereal) liwmin, d__2 = (doublereal) iwork[1];
    liwmin = (integer) max(d__1,d__2);

    if (wantz) {

/*        Backtransform eigenvectors to the original problem. */

	neig = *n;
	if (*info > 0) {
	    neig = *info - 1;
	}
	if (*itype == 1 || *itype == 2) {

/*           For A*x=(lambda)*B*x and A*B*x=(lambda)*x;   
             backtransform eigenvectors: x = inv(L)'*y or inv(U)*y */

	    if (upper) {
		*(unsigned char *)trans = 'N';
	    } else {
		*(unsigned char *)trans = 'T';
	    }

	    i__1 = neig;
	    for (j = 1; j <= i__1; ++j) {
		dtpsv_(uplo, trans, "Non-unit", n, &bp[1], &z___ref(1, j), &
			c__1);
/* L10: */
	    }

	} else if (*itype == 3) {

/*           For B*A*x=(lambda)*x;   
             backtransform eigenvectors: x = L*y or U'*y */

	    if (upper) {
		*(unsigned char *)trans = 'T';
	    } else {
		*(unsigned char *)trans = 'N';
	    }

	    i__1 = neig;
	    for (j = 1; j <= i__1; ++j) {
		dtpmv_(uplo, trans, "Non-unit", n, &bp[1], &z___ref(1, j), &
			c__1);
/* L20: */
	    }
	}
    }

    work[1] = (doublereal) lwmin;
    iwork[1] = liwmin;

    return 0;

/*     End of DSPGVD */

} /* dspgvd_ */

#undef z___ref


