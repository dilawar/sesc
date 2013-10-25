#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zgeqp3_(integer *m, integer *n, doublecomplex *a, 
	integer *lda, integer *jpvt, doublecomplex *tau, doublecomplex *work, 
	integer *lwork, doublereal *rwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    ZGEQP3 computes a QR factorization with column pivoting of a   
    matrix A:  A*P = Q*R  using Level 3 BLAS.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A. M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the M-by-N matrix A.   
            On exit, the upper triangle of the array contains the   
            min(M,N)-by-N upper trapezoidal matrix R; the elements below   
            the diagonal, together with the array TAU, represent the   
            unitary matrix Q as a product of min(M,N) elementary   
            reflectors.   

    LDA     (input) INTEGER   
            The leading dimension of the array A. LDA >= max(1,M).   

    JPVT    (input/output) INTEGER array, dimension (N)   
            On entry, if JPVT(J).ne.0, the J-th column of A is permuted   
            to the front of A*P (a leading column); if JPVT(J)=0,   
            the J-th column of A is a free column.   
            On exit, if JPVT(J)=K, then the J-th column of A*P was the   
            the K-th column of A.   

    TAU     (output) COMPLEX*16 array, dimension (min(M,N))   
            The scalar factors of the elementary reflectors.   

    WORK    (workspace/output) COMPLEX*16 array, dimension (LWORK)   
            On exit, if INFO=0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK. LWORK >= N+1.   
            For optimal performance LWORK >= ( N+1 )*NB, where NB   
            is the optimal blocksize.   

            If LWORK = -1, then a workspace query is assumed; the routine   
            only calculates the optimal size of the WORK array, returns   
            this value as the first entry of the WORK array, and no error   
            message related to LWORK is issued by XERBLA.   

    RWORK   (workspace) DOUBLE PRECISION array, dimension (2*N)   

    INFO    (output) INTEGER   
            = 0: successful exit.   
            < 0: if INFO = -i, the i-th argument had an illegal value.   

    Further Details   
    ===============   

    The matrix Q is represented as a product of elementary reflectors   

       Q = H(1) H(2) . . . H(k), where k = min(m,n).   

    Each H(i) has the form   

       H(i) = I - tau * v * v'   

    where tau is a real/complex scalar, and v is a real/complex vector   
    with v(1:i-1) = 0 and v(i) = 1; v(i+1:m) is stored on exit in   
    A(i+1:m,i), and tau in TAU(i).   

    Based on contributions by   
      G. Quintana-Orti, Depto. de Informatica, Universidad Jaime I, Spain   
      X. Sun, Computer Science Dept., Duke University, USA   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static integer c_n1 = -1;
    static integer c__3 = 3;
    static integer c__2 = 2;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3;
    /* Local variables */
    static integer nfxd, j, nbmin, minmn, minws;
    extern /* Subroutine */ int zswap_(integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zlaqp2_(integer *, integer *, 
	    integer *, doublecomplex *, integer *, integer *, doublecomplex *,
	     doublereal *, doublereal *, doublecomplex *);
    static integer jb;
    extern doublereal dznrm2_(integer *, doublecomplex *, integer *);
    static integer na, nb, sm, sn, nx;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen);
    extern /* Subroutine */ int zgeqrf_(integer *, integer *, doublecomplex *,
	     integer *, doublecomplex *, doublecomplex *, integer *, integer *
	    );
    static integer topbmn, sminmn;
    extern /* Subroutine */ int zlaqps_(integer *, integer *, integer *, 
	    integer *, integer *, doublecomplex *, integer *, integer *, 
	    doublecomplex *, doublereal *, doublereal *, doublecomplex *, 
	    doublecomplex *, integer *);
    static integer lwkopt;
    static logical lquery;
    extern /* Subroutine */ int zunmqr_(char *, char *, integer *, integer *, 
	    integer *, doublecomplex *, integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *, integer *);
    static integer fjb, iws;
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --jpvt;
    --tau;
    --work;
    --rwork;

    /* Function Body */
    iws = *n + 1;
    minmn = min(*m,*n);

/*     Test input arguments   
       ==================== */

    *info = 0;
    nb = ilaenv_(&c__1, "ZGEQRF", " ", m, n, &c_n1, &c_n1, (ftnlen)6, (ftnlen)
	    1);
    lwkopt = (*n + 1) * nb;
    work[1].r = (doublereal) lwkopt, work[1].i = 0.;
    lquery = *lwork == -1;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*m)) {
	*info = -4;
    } else if (*lwork < iws && ! lquery) {
	*info = -8;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZGEQP3", &i__1);
	return 0;
    } else if (lquery) {
	return 0;
    }

/*     Quick return if possible. */

    if (minmn == 0) {
	work[1].r = 1., work[1].i = 0.;
	return 0;
    }

/*     Move initial columns up front. */

    nfxd = 1;
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	if (jpvt[j] != 0) {
	    if (j != nfxd) {
		zswap_(m, &a_ref(1, j), &c__1, &a_ref(1, nfxd), &c__1);
		jpvt[j] = jpvt[nfxd];
		jpvt[nfxd] = j;
	    } else {
		jpvt[j] = j;
	    }
	    ++nfxd;
	} else {
	    jpvt[j] = j;
	}
/* L10: */
    }
    --nfxd;

/*     Factorize fixed columns   
       =======================   

       Compute the QR factorization of fixed columns and update   
       remaining columns. */

    if (nfxd > 0) {
	na = min(*m,nfxd);
/* CC      CALL ZGEQR2( M, NA, A, LDA, TAU, WORK, INFO ) */
	zgeqrf_(m, &na, &a[a_offset], lda, &tau[1], &work[1], lwork, info);
/* Computing MAX */
	i__1 = iws, i__2 = (integer) work[1].r;
	iws = max(i__1,i__2);
	if (na < *n) {
/* CC         CALL ZUNM2R( 'Left', 'Conjugate Transpose', M, N-NA,   
   CC  $                   NA, A, LDA, TAU, A( 1, NA+1 ), LDA, WORK,   
   CC  $                   INFO ) */
	    i__1 = *n - na;
	    zunmqr_("Left", "Conjugate Transpose", m, &i__1, &na, &a[a_offset]
		    , lda, &tau[1], &a_ref(1, na + 1), lda, &work[1], lwork, 
		    info);
/* Computing MAX */
	    i__1 = iws, i__2 = (integer) work[1].r;
	    iws = max(i__1,i__2);
	}
    }

/*     Factorize free columns   
       ====================== */

    if (nfxd < minmn) {

	sm = *m - nfxd;
	sn = *n - nfxd;
	sminmn = minmn - nfxd;

/*        Determine the block size. */

	nb = ilaenv_(&c__1, "ZGEQRF", " ", &sm, &sn, &c_n1, &c_n1, (ftnlen)6, 
		(ftnlen)1);
	nbmin = 2;
	nx = 0;

	if (nb > 1 && nb < sminmn) {

/*           Determine when to cross over from blocked to unblocked code.   

   Computing MAX */
	    i__1 = 0, i__2 = ilaenv_(&c__3, "ZGEQRF", " ", &sm, &sn, &c_n1, &
		    c_n1, (ftnlen)6, (ftnlen)1);
	    nx = max(i__1,i__2);


	    if (nx < sminmn) {

/*              Determine if workspace is large enough for blocked code. */

		minws = (sn + 1) * nb;
		iws = max(iws,minws);
		if (*lwork < minws) {

/*                 Not enough workspace to use optimal NB: Reduce NB and   
                   determine the minimum value of NB. */

		    nb = *lwork / (sn + 1);
/* Computing MAX */
		    i__1 = 2, i__2 = ilaenv_(&c__2, "ZGEQRF", " ", &sm, &sn, &
			    c_n1, &c_n1, (ftnlen)6, (ftnlen)1);
		    nbmin = max(i__1,i__2);


		}
	    }
	}

/*        Initialize partial column norms. The first N elements of work   
          store the exact column norms. */

	i__1 = *n;
	for (j = nfxd + 1; j <= i__1; ++j) {
	    rwork[j] = dznrm2_(&sm, &a_ref(nfxd + 1, j), &c__1);
	    rwork[*n + j] = rwork[j];
/* L20: */
	}

	if (nb >= nbmin && nb < sminmn && nx < sminmn) {

/*           Use blocked code initially. */

	    j = nfxd + 1;

/*           Compute factorization: while loop. */


	    topbmn = minmn - nx;
L30:
	    if (j <= topbmn) {
/* Computing MIN */
		i__1 = nb, i__2 = topbmn - j + 1;
		jb = min(i__1,i__2);

/*              Factorize JB columns among columns J:N. */

		i__1 = *n - j + 1;
		i__2 = j - 1;
		i__3 = *n - j + 1;
		zlaqps_(m, &i__1, &i__2, &jb, &fjb, &a_ref(1, j), lda, &jpvt[
			j], &tau[j], &rwork[j], &rwork[*n + j], &work[1], &
			work[jb + 1], &i__3);

		j += fjb;
		goto L30;
	    }
	} else {
	    j = nfxd + 1;
	}

/*        Use unblocked code to factor the last or only block. */


	if (j <= minmn) {
	    i__1 = *n - j + 1;
	    i__2 = j - 1;
	    zlaqp2_(m, &i__1, &i__2, &a_ref(1, j), lda, &jpvt[j], &tau[j], &
		    rwork[j], &rwork[*n + j], &work[1]);
	}

    }

    work[1].r = (doublereal) iws, work[1].i = 0.;
    return 0;

/*     End of ZGEQP3 */

} /* zgeqp3_ */

#undef a_ref
#undef a_subscr


