#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dlarzb_(char *side, char *trans, char *direct, char *
	storev, integer *m, integer *n, integer *k, integer *l, doublereal *v,
	 integer *ldv, doublereal *t, integer *ldt, doublereal *c__, integer *
	ldc, doublereal *work, integer *ldwork)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       December 1, 1999   


    Purpose   
    =======   

    DLARZB applies a real block reflector H or its transpose H**T to   
    a real distributed M-by-N  C from the left or the right.   

    Currently, only STOREV = 'R' and DIRECT = 'B' are supported.   

    Arguments   
    =========   

    SIDE    (input) CHARACTER*1   
            = 'L': apply H or H' from the Left   
            = 'R': apply H or H' from the Right   

    TRANS   (input) CHARACTER*1   
            = 'N': apply H (No transpose)   
            = 'C': apply H' (Transpose)   

    DIRECT  (input) CHARACTER*1   
            Indicates how H is formed from a product of elementary   
            reflectors   
            = 'F': H = H(1) H(2) . . . H(k) (Forward, not supported yet)   
            = 'B': H = H(k) . . . H(2) H(1) (Backward)   

    STOREV  (input) CHARACTER*1   
            Indicates how the vectors which define the elementary   
            reflectors are stored:   
            = 'C': Columnwise                        (not supported yet)   
            = 'R': Rowwise   

    M       (input) INTEGER   
            The number of rows of the matrix C.   

    N       (input) INTEGER   
            The number of columns of the matrix C.   

    K       (input) INTEGER   
            The order of the matrix T (= the number of elementary   
            reflectors whose product defines the block reflector).   

    L       (input) INTEGER   
            The number of columns of the matrix V containing the   
            meaningful part of the Householder reflectors.   
            If SIDE = 'L', M >= L >= 0, if SIDE = 'R', N >= L >= 0.   

    V       (input) DOUBLE PRECISION array, dimension (LDV,NV).   
            If STOREV = 'C', NV = K; if STOREV = 'R', NV = L.   

    LDV     (input) INTEGER   
            The leading dimension of the array V.   
            If STOREV = 'C', LDV >= L; if STOREV = 'R', LDV >= K.   

    T       (input) DOUBLE PRECISION array, dimension (LDT,K)   
            The triangular K-by-K matrix T in the representation of the   
            block reflector.   

    LDT     (input) INTEGER   
            The leading dimension of the array T. LDT >= K.   

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,N)   
            On entry, the M-by-N matrix C.   
            On exit, C is overwritten by H*C or H'*C or C*H or C*H'.   

    LDC     (input) INTEGER   
            The leading dimension of the array C. LDC >= max(1,M).   

    WORK    (workspace) DOUBLE PRECISION array, dimension (LDWORK,K)   

    LDWORK  (input) INTEGER   
            The leading dimension of the array WORK.   
            If SIDE = 'L', LDWORK >= max(1,N);   
            if SIDE = 'R', LDWORK >= max(1,M).   

    Further Details   
    ===============   

    Based on contributions by   
      A. Petitet, Computer Science Dept., Univ. of Tenn., Knoxville, USA   

    =====================================================================   


       Quick return if possible   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static doublereal c_b13 = 1.;
    static doublereal c_b23 = -1.;
    
    /* System generated locals */
    integer c_dim1, c_offset, t_dim1, t_offset, v_dim1, v_offset, work_dim1, 
	    work_offset, i__1, i__2;
    /* Local variables */
    static integer info, i__, j;
    extern /* Subroutine */ int dgemm_(char *, char *, integer *, integer *, 
	    integer *, doublereal *, doublereal *, integer *, doublereal *, 
	    integer *, doublereal *, doublereal *, integer *);
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int dcopy_(integer *, doublereal *, integer *, 
	    doublereal *, integer *), dtrmm_(char *, char *, char *, char *, 
	    integer *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *, integer *), xerbla_(
	    char *, integer *);
    static char transt[1];
#define work_ref(a_1,a_2) work[(a_2)*work_dim1 + a_1]
#define c___ref(a_1,a_2) c__[(a_2)*c_dim1 + a_1]


    v_dim1 = *ldv;
    v_offset = 1 + v_dim1 * 1;
    v -= v_offset;
    t_dim1 = *ldt;
    t_offset = 1 + t_dim1 * 1;
    t -= t_offset;
    c_dim1 = *ldc;
    c_offset = 1 + c_dim1 * 1;
    c__ -= c_offset;
    work_dim1 = *ldwork;
    work_offset = 1 + work_dim1 * 1;
    work -= work_offset;

    /* Function Body */
    if (*m <= 0 || *n <= 0) {
	return 0;
    }

/*     Check for currently supported options */

    info = 0;
    if (! lsame_(direct, "B")) {
	info = -3;
    } else if (! lsame_(storev, "R")) {
	info = -4;
    }
    if (info != 0) {
	i__1 = -info;
	xerbla_("DLARZB", &i__1);
	return 0;
    }

    if (lsame_(trans, "N")) {
	*(unsigned char *)transt = 'T';
    } else {
	*(unsigned char *)transt = 'N';
    }

    if (lsame_(side, "L")) {

/*        Form  H * C  or  H' * C   

          W( 1:n, 1:k ) = C( 1:k, 1:n )' */

	i__1 = *k;
	for (j = 1; j <= i__1; ++j) {
	    dcopy_(n, &c___ref(j, 1), ldc, &work_ref(1, j), &c__1);
/* L10: */
	}

/*        W( 1:n, 1:k ) = W( 1:n, 1:k ) + ...   
                          C( m-l+1:m, 1:n )' * V( 1:k, 1:l )' */

	if (*l > 0) {
	    dgemm_("Transpose", "Transpose", n, k, l, &c_b13, &c___ref(*m - *
		    l + 1, 1), ldc, &v[v_offset], ldv, &c_b13, &work[
		    work_offset], ldwork);
	}

/*        W( 1:n, 1:k ) = W( 1:n, 1:k ) * T'  or  W( 1:m, 1:k ) * T */

	dtrmm_("Right", "Lower", transt, "Non-unit", n, k, &c_b13, &t[
		t_offset], ldt, &work[work_offset], ldwork);

/*        C( 1:k, 1:n ) = C( 1:k, 1:n ) - W( 1:n, 1:k )' */

	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = *k;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		c___ref(i__, j) = c___ref(i__, j) - work_ref(j, i__);
/* L20: */
	    }
/* L30: */
	}

/*        C( m-l+1:m, 1:n ) = C( m-l+1:m, 1:n ) - ...   
                              V( 1:k, 1:l )' * W( 1:n, 1:k )' */

	if (*l > 0) {
	    dgemm_("Transpose", "Transpose", l, n, k, &c_b23, &v[v_offset], 
		    ldv, &work[work_offset], ldwork, &c_b13, &c___ref(*m - *l 
		    + 1, 1), ldc);
	}

    } else if (lsame_(side, "R")) {

/*        Form  C * H  or  C * H'   

          W( 1:m, 1:k ) = C( 1:m, 1:k ) */

	i__1 = *k;
	for (j = 1; j <= i__1; ++j) {
	    dcopy_(m, &c___ref(1, j), &c__1, &work_ref(1, j), &c__1);
/* L40: */
	}

/*        W( 1:m, 1:k ) = W( 1:m, 1:k ) + ...   
                          C( 1:m, n-l+1:n ) * V( 1:k, 1:l )' */

	if (*l > 0) {
	    dgemm_("No transpose", "Transpose", m, k, l, &c_b13, &c___ref(1, *
		    n - *l + 1), ldc, &v[v_offset], ldv, &c_b13, &work[
		    work_offset], ldwork);
	}

/*        W( 1:m, 1:k ) = W( 1:m, 1:k ) * T  or  W( 1:m, 1:k ) * T' */

	dtrmm_("Right", "Lower", trans, "Non-unit", m, k, &c_b13, &t[t_offset]
		, ldt, &work[work_offset], ldwork);

/*        C( 1:m, 1:k ) = C( 1:m, 1:k ) - W( 1:m, 1:k ) */

	i__1 = *k;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = *m;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		c___ref(i__, j) = c___ref(i__, j) - work_ref(i__, j);
/* L50: */
	    }
/* L60: */
	}

/*        C( 1:m, n-l+1:n ) = C( 1:m, n-l+1:n ) - ...   
                              W( 1:m, 1:k ) * V( 1:k, 1:l ) */

	if (*l > 0) {
	    dgemm_("No transpose", "No transpose", m, l, k, &c_b23, &work[
		    work_offset], ldwork, &v[v_offset], ldv, &c_b13, &c___ref(
		    1, *n - *l + 1), ldc);
	}

    }

    return 0;

/*     End of DLARZB */

} /* dlarzb_ */

#undef c___ref
#undef work_ref


