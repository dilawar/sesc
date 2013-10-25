#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dopmtr_(char *side, char *uplo, char *trans, integer *m, 
	integer *n, doublereal *ap, doublereal *tau, doublereal *c__, integer 
	*ldc, doublereal *work, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DOPMTR overwrites the general real M-by-N matrix C with   

                    SIDE = 'L'     SIDE = 'R'   
    TRANS = 'N':      Q * C          C * Q   
    TRANS = 'T':      Q**T * C       C * Q**T   

    where Q is a real orthogonal matrix of order nq, with nq = m if   
    SIDE = 'L' and nq = n if SIDE = 'R'. Q is defined as the product of   
    nq-1 elementary reflectors, as returned by DSPTRD using packed   
    storage:   

    if UPLO = 'U', Q = H(nq-1) . . . H(2) H(1);   

    if UPLO = 'L', Q = H(1) H(2) . . . H(nq-1).   

    Arguments   
    =========   

    SIDE    (input) CHARACTER*1   
            = 'L': apply Q or Q**T from the Left;   
            = 'R': apply Q or Q**T from the Right.   

    UPLO    (input) CHARACTER*1   
            = 'U': Upper triangular packed storage used in previous   
                   call to DSPTRD;   
            = 'L': Lower triangular packed storage used in previous   
                   call to DSPTRD.   

    TRANS   (input) CHARACTER*1   
            = 'N':  No transpose, apply Q;   
            = 'T':  Transpose, apply Q**T.   

    M       (input) INTEGER   
            The number of rows of the matrix C. M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix C. N >= 0.   

    AP      (input) DOUBLE PRECISION array, dimension   
                                 (M*(M+1)/2) if SIDE = 'L'   
                                 (N*(N+1)/2) if SIDE = 'R'   
            The vectors which define the elementary reflectors, as   
            returned by DSPTRD.  AP is modified by the routine but   
            restored on exit.   

    TAU     (input) DOUBLE PRECISION array, dimension (M-1) if SIDE = 'L'   
                                       or (N-1) if SIDE = 'R'   
            TAU(i) must contain the scalar factor of the elementary   
            reflector H(i), as returned by DSPTRD.   

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,N)   
            On entry, the M-by-N matrix C.   
            On exit, C is overwritten by Q*C or Q**T*C or C*Q**T or C*Q.   

    LDC     (input) INTEGER   
            The leading dimension of the array C. LDC >= max(1,M).   

    WORK    (workspace) DOUBLE PRECISION array, dimension   
                                     (N) if SIDE = 'L'   
                                     (M) if SIDE = 'R'   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   


       Test the input arguments   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer c_dim1, c_offset, i__1, i__2;
    /* Local variables */
    static logical left;
    static integer i__;
    extern /* Subroutine */ int dlarf_(char *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *);
    extern logical lsame_(char *, char *);
    static integer i1;
    static logical upper;
    static integer i2, i3, ic, jc, ii, mi, ni, nq;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static logical notran, forwrd;
    static doublereal aii;
#define c___ref(a_1,a_2) c__[(a_2)*c_dim1 + a_1]


    --ap;
    --tau;
    c_dim1 = *ldc;
    c_offset = 1 + c_dim1 * 1;
    c__ -= c_offset;
    --work;

    /* Function Body */
    *info = 0;
    left = lsame_(side, "L");
    notran = lsame_(trans, "N");
    upper = lsame_(uplo, "U");

/*     NQ is the order of Q */

    if (left) {
	nq = *m;
    } else {
	nq = *n;
    }
    if (! left && ! lsame_(side, "R")) {
	*info = -1;
    } else if (! upper && ! lsame_(uplo, "L")) {
	*info = -2;
    } else if (! notran && ! lsame_(trans, "T")) {
	*info = -3;
    } else if (*m < 0) {
	*info = -4;
    } else if (*n < 0) {
	*info = -5;
    } else if (*ldc < max(1,*m)) {
	*info = -9;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DOPMTR", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*m == 0 || *n == 0) {
	return 0;
    }

    if (upper) {

/*        Q was determined by a call to DSPTRD with UPLO = 'U' */

	forwrd = left && notran || ! left && ! notran;

	if (forwrd) {
	    i1 = 1;
	    i2 = nq - 1;
	    i3 = 1;
	    ii = 2;
	} else {
	    i1 = nq - 1;
	    i2 = 1;
	    i3 = -1;
	    ii = nq * (nq + 1) / 2 - 1;
	}

	if (left) {
	    ni = *n;
	} else {
	    mi = *m;
	}

	i__1 = i2;
	i__2 = i3;
	for (i__ = i1; i__2 < 0 ? i__ >= i__1 : i__ <= i__1; i__ += i__2) {
	    if (left) {

/*              H(i) is applied to C(1:i,1:n) */

		mi = i__;
	    } else {

/*              H(i) is applied to C(1:m,1:i) */

		ni = i__;
	    }

/*           Apply H(i) */

	    aii = ap[ii];
	    ap[ii] = 1.;
	    dlarf_(side, &mi, &ni, &ap[ii - i__ + 1], &c__1, &tau[i__], &c__[
		    c_offset], ldc, &work[1]);
	    ap[ii] = aii;

	    if (forwrd) {
		ii = ii + i__ + 2;
	    } else {
		ii = ii - i__ - 1;
	    }
/* L10: */
	}
    } else {

/*        Q was determined by a call to DSPTRD with UPLO = 'L'. */

	forwrd = left && ! notran || ! left && notran;

	if (forwrd) {
	    i1 = 1;
	    i2 = nq - 1;
	    i3 = 1;
	    ii = 2;
	} else {
	    i1 = nq - 1;
	    i2 = 1;
	    i3 = -1;
	    ii = nq * (nq + 1) / 2 - 1;
	}

	if (left) {
	    ni = *n;
	    jc = 1;
	} else {
	    mi = *m;
	    ic = 1;
	}

	i__2 = i2;
	i__1 = i3;
	for (i__ = i1; i__1 < 0 ? i__ >= i__2 : i__ <= i__2; i__ += i__1) {
	    aii = ap[ii];
	    ap[ii] = 1.;
	    if (left) {

/*              H(i) is applied to C(i+1:m,1:n) */

		mi = *m - i__;
		ic = i__ + 1;
	    } else {

/*              H(i) is applied to C(1:m,i+1:n) */

		ni = *n - i__;
		jc = i__ + 1;
	    }

/*           Apply H(i) */

	    dlarf_(side, &mi, &ni, &ap[ii], &c__1, &tau[i__], &c___ref(ic, jc)
		    , ldc, &work[1]);
	    ap[ii] = aii;

	    if (forwrd) {
		ii = ii + nq - i__ + 1;
	    } else {
		ii = ii - nq + i__ - 2;
	    }
/* L20: */
	}
    }
    return 0;

/*     End of DOPMTR */

} /* dopmtr_ */

#undef c___ref


