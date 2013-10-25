#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int ztrevc_(char *side, char *howmny, logical *select, 
	integer *n, doublecomplex *t, integer *ldt, doublecomplex *vl, 
	integer *ldvl, doublecomplex *vr, integer *ldvr, integer *mm, integer 
	*m, doublecomplex *work, doublereal *rwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    ZTREVC computes some or all of the right and/or left eigenvectors of   
    a complex upper triangular matrix T.   

    The right eigenvector x and the left eigenvector y of T corresponding   
    to an eigenvalue w are defined by:   

                 T*x = w*x,     y'*T = w*y'   

    where y' denotes the conjugate transpose of the vector y.   

    If all eigenvectors are requested, the routine may either return the   
    matrices X and/or Y of right or left eigenvectors of T, or the   
    products Q*X and/or Q*Y, where Q is an input unitary   
    matrix. If T was obtained from the Schur factorization of an   
    original matrix A = Q*T*Q', then Q*X and Q*Y are the matrices of   
    right or left eigenvectors of A.   

    Arguments   
    =========   

    SIDE    (input) CHARACTER*1   
            = 'R':  compute right eigenvectors only;   
            = 'L':  compute left eigenvectors only;   
            = 'B':  compute both right and left eigenvectors.   

    HOWMNY  (input) CHARACTER*1   
            = 'A':  compute all right and/or left eigenvectors;   
            = 'B':  compute all right and/or left eigenvectors,   
                    and backtransform them using the input matrices   
                    supplied in VR and/or VL;   
            = 'S':  compute selected right and/or left eigenvectors,   
                    specified by the logical array SELECT.   

    SELECT  (input) LOGICAL array, dimension (N)   
            If HOWMNY = 'S', SELECT specifies the eigenvectors to be   
            computed.   
            If HOWMNY = 'A' or 'B', SELECT is not referenced.   
            To select the eigenvector corresponding to the j-th   
            eigenvalue, SELECT(j) must be set to .TRUE..   

    N       (input) INTEGER   
            The order of the matrix T. N >= 0.   

    T       (input/output) COMPLEX*16 array, dimension (LDT,N)   
            The upper triangular matrix T.  T is modified, but restored   
            on exit.   

    LDT     (input) INTEGER   
            The leading dimension of the array T. LDT >= max(1,N).   

    VL      (input/output) COMPLEX*16 array, dimension (LDVL,MM)   
            On entry, if SIDE = 'L' or 'B' and HOWMNY = 'B', VL must   
            contain an N-by-N matrix Q (usually the unitary matrix Q of   
            Schur vectors returned by ZHSEQR).   
            On exit, if SIDE = 'L' or 'B', VL contains:   
            if HOWMNY = 'A', the matrix Y of left eigenvectors of T;   
                             VL is lower triangular. The i-th column   
                             VL(i) of VL is the eigenvector corresponding   
                             to T(i,i).   
            if HOWMNY = 'B', the matrix Q*Y;   
            if HOWMNY = 'S', the left eigenvectors of T specified by   
                             SELECT, stored consecutively in the columns   
                             of VL, in the same order as their   
                             eigenvalues.   
            If SIDE = 'R', VL is not referenced.   

    LDVL    (input) INTEGER   
            The leading dimension of the array VL.  LDVL >= max(1,N) if   
            SIDE = 'L' or 'B'; LDVL >= 1 otherwise.   

    VR      (input/output) COMPLEX*16 array, dimension (LDVR,MM)   
            On entry, if SIDE = 'R' or 'B' and HOWMNY = 'B', VR must   
            contain an N-by-N matrix Q (usually the unitary matrix Q of   
            Schur vectors returned by ZHSEQR).   
            On exit, if SIDE = 'R' or 'B', VR contains:   
            if HOWMNY = 'A', the matrix X of right eigenvectors of T;   
                             VR is upper triangular. The i-th column   
                             VR(i) of VR is the eigenvector corresponding   
                             to T(i,i).   
            if HOWMNY = 'B', the matrix Q*X;   
            if HOWMNY = 'S', the right eigenvectors of T specified by   
                             SELECT, stored consecutively in the columns   
                             of VR, in the same order as their   
                             eigenvalues.   
            If SIDE = 'L', VR is not referenced.   

    LDVR    (input) INTEGER   
            The leading dimension of the array VR.  LDVR >= max(1,N) if   
             SIDE = 'R' or 'B'; LDVR >= 1 otherwise.   

    MM      (input) INTEGER   
            The number of columns in the arrays VL and/or VR. MM >= M.   

    M       (output) INTEGER   
            The number of columns in the arrays VL and/or VR actually   
            used to store the eigenvectors.  If HOWMNY = 'A' or 'B', M   
            is set to N.  Each selected eigenvector occupies one   
            column.   

    WORK    (workspace) COMPLEX*16 array, dimension (2*N)   

    RWORK   (workspace) DOUBLE PRECISION array, dimension (N)   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The algorithm used in this program is basically backward (forward)   
    substitution, with scaling to make the the code robust against   
    possible overflow.   

    Each eigenvector is normalized so that the element of largest   
    magnitude has magnitude 1; here the magnitude of a complex number   
    (x,y) is taken to be |x| + |y|.   

    =====================================================================   


       Decode and test the input parameters   

       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b2 = {1.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer t_dim1, t_offset, vl_dim1, vl_offset, vr_dim1, vr_offset, i__1, 
	    i__2, i__3, i__4, i__5;
    doublereal d__1, d__2, d__3;
    doublecomplex z__1, z__2;
    /* Builtin functions */
    double d_imag(doublecomplex *);
    void d_cnjg(doublecomplex *, doublecomplex *);
    /* Local variables */
    static logical allv;
    static doublereal unfl, ovfl, smin;
    static logical over;
    static integer i__, j, k;
    static doublereal scale;
    extern logical lsame_(char *, char *);
    static doublereal remax;
    static logical leftv, bothv;
    extern /* Subroutine */ int zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *);
    static logical somev;
    extern /* Subroutine */ int zcopy_(integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), dlabad_(doublereal *, doublereal *);
    static integer ii, ki;
    extern doublereal dlamch_(char *);
    static integer is;
    extern /* Subroutine */ int xerbla_(char *, integer *), zdscal_(
	    integer *, doublereal *, doublecomplex *, integer *);
    extern integer izamax_(integer *, doublecomplex *, integer *);
    static logical rightv;
    extern doublereal dzasum_(integer *, doublecomplex *, integer *);
    static doublereal smlnum;
    extern /* Subroutine */ int zlatrs_(char *, char *, char *, char *, 
	    integer *, doublecomplex *, integer *, doublecomplex *, 
	    doublereal *, doublereal *, integer *);
    static doublereal ulp;
#define t_subscr(a_1,a_2) (a_2)*t_dim1 + a_1
#define t_ref(a_1,a_2) t[t_subscr(a_1,a_2)]
#define vl_subscr(a_1,a_2) (a_2)*vl_dim1 + a_1
#define vl_ref(a_1,a_2) vl[vl_subscr(a_1,a_2)]
#define vr_subscr(a_1,a_2) (a_2)*vr_dim1 + a_1
#define vr_ref(a_1,a_2) vr[vr_subscr(a_1,a_2)]


    --select;
    t_dim1 = *ldt;
    t_offset = 1 + t_dim1 * 1;
    t -= t_offset;
    vl_dim1 = *ldvl;
    vl_offset = 1 + vl_dim1 * 1;
    vl -= vl_offset;
    vr_dim1 = *ldvr;
    vr_offset = 1 + vr_dim1 * 1;
    vr -= vr_offset;
    --work;
    --rwork;

    /* Function Body */
    bothv = lsame_(side, "B");
    rightv = lsame_(side, "R") || bothv;
    leftv = lsame_(side, "L") || bothv;

    allv = lsame_(howmny, "A");
    over = lsame_(howmny, "B");
    somev = lsame_(howmny, "S");

/*     Set M to the number of columns required to store the selected   
       eigenvectors. */

    if (somev) {
	*m = 0;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    if (select[j]) {
		++(*m);
	    }
/* L10: */
	}
    } else {
	*m = *n;
    }

    *info = 0;
    if (! rightv && ! leftv) {
	*info = -1;
    } else if (! allv && ! over && ! somev) {
	*info = -2;
    } else if (*n < 0) {
	*info = -4;
    } else if (*ldt < max(1,*n)) {
	*info = -6;
    } else if (*ldvl < 1 || leftv && *ldvl < *n) {
	*info = -8;
    } else if (*ldvr < 1 || rightv && *ldvr < *n) {
	*info = -10;
    } else if (*mm < *m) {
	*info = -11;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZTREVC", &i__1);
	return 0;
    }

/*     Quick return if possible. */

    if (*n == 0) {
	return 0;
    }

/*     Set the constants to control overflow. */

    unfl = dlamch_("Safe minimum");
    ovfl = 1. / unfl;
    dlabad_(&unfl, &ovfl);
    ulp = dlamch_("Precision");
    smlnum = unfl * (*n / ulp);

/*     Store the diagonal elements of T in working array WORK. */

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	i__2 = i__ + *n;
	i__3 = t_subscr(i__, i__);
	work[i__2].r = t[i__3].r, work[i__2].i = t[i__3].i;
/* L20: */
    }

/*     Compute 1-norm of each column of strictly upper triangular   
       part of T to control overflow in triangular solver. */

    rwork[1] = 0.;
    i__1 = *n;
    for (j = 2; j <= i__1; ++j) {
	i__2 = j - 1;
	rwork[j] = dzasum_(&i__2, &t_ref(1, j), &c__1);
/* L30: */
    }

    if (rightv) {

/*        Compute right eigenvectors. */

	is = *m;
	for (ki = *n; ki >= 1; --ki) {

	    if (somev) {
		if (! select[ki]) {
		    goto L80;
		}
	    }
/* Computing MAX */
	    i__1 = t_subscr(ki, ki);
	    d__3 = ulp * ((d__1 = t[i__1].r, abs(d__1)) + (d__2 = d_imag(&
		    t_ref(ki, ki)), abs(d__2)));
	    smin = max(d__3,smlnum);

	    work[1].r = 1., work[1].i = 0.;

/*           Form right-hand side. */

	    i__1 = ki - 1;
	    for (k = 1; k <= i__1; ++k) {
		i__2 = k;
		i__3 = t_subscr(k, ki);
		z__1.r = -t[i__3].r, z__1.i = -t[i__3].i;
		work[i__2].r = z__1.r, work[i__2].i = z__1.i;
/* L40: */
	    }

/*           Solve the triangular system:   
                (T(1:KI-1,1:KI-1) - T(KI,KI))*X = SCALE*WORK. */

	    i__1 = ki - 1;
	    for (k = 1; k <= i__1; ++k) {
		i__2 = t_subscr(k, k);
		i__3 = t_subscr(k, k);
		i__4 = t_subscr(ki, ki);
		z__1.r = t[i__3].r - t[i__4].r, z__1.i = t[i__3].i - t[i__4]
			.i;
		t[i__2].r = z__1.r, t[i__2].i = z__1.i;
		i__2 = t_subscr(k, k);
		if ((d__1 = t[i__2].r, abs(d__1)) + (d__2 = d_imag(&t_ref(k, 
			k)), abs(d__2)) < smin) {
		    i__3 = t_subscr(k, k);
		    t[i__3].r = smin, t[i__3].i = 0.;
		}
/* L50: */
	    }

	    if (ki > 1) {
		i__1 = ki - 1;
		zlatrs_("Upper", "No transpose", "Non-unit", "Y", &i__1, &t[
			t_offset], ldt, &work[1], &scale, &rwork[1], info);
		i__1 = ki;
		work[i__1].r = scale, work[i__1].i = 0.;
	    }

/*           Copy the vector x or Q*x to VR and normalize. */

	    if (! over) {
		zcopy_(&ki, &work[1], &c__1, &vr_ref(1, is), &c__1);

		ii = izamax_(&ki, &vr_ref(1, is), &c__1);
		i__1 = vr_subscr(ii, is);
		remax = 1. / ((d__1 = vr[i__1].r, abs(d__1)) + (d__2 = d_imag(
			&vr_ref(ii, is)), abs(d__2)));
		zdscal_(&ki, &remax, &vr_ref(1, is), &c__1);

		i__1 = *n;
		for (k = ki + 1; k <= i__1; ++k) {
		    i__2 = vr_subscr(k, is);
		    vr[i__2].r = 0., vr[i__2].i = 0.;
/* L60: */
		}
	    } else {
		if (ki > 1) {
		    i__1 = ki - 1;
		    z__1.r = scale, z__1.i = 0.;
		    zgemv_("N", n, &i__1, &c_b2, &vr[vr_offset], ldvr, &work[
			    1], &c__1, &z__1, &vr_ref(1, ki), &c__1);
		}

		ii = izamax_(n, &vr_ref(1, ki), &c__1);
		i__1 = vr_subscr(ii, ki);
		remax = 1. / ((d__1 = vr[i__1].r, abs(d__1)) + (d__2 = d_imag(
			&vr_ref(ii, ki)), abs(d__2)));
		zdscal_(n, &remax, &vr_ref(1, ki), &c__1);
	    }

/*           Set back the original diagonal elements of T. */

	    i__1 = ki - 1;
	    for (k = 1; k <= i__1; ++k) {
		i__2 = t_subscr(k, k);
		i__3 = k + *n;
		t[i__2].r = work[i__3].r, t[i__2].i = work[i__3].i;
/* L70: */
	    }

	    --is;
L80:
	    ;
	}
    }

    if (leftv) {

/*        Compute left eigenvectors. */

	is = 1;
	i__1 = *n;
	for (ki = 1; ki <= i__1; ++ki) {

	    if (somev) {
		if (! select[ki]) {
		    goto L130;
		}
	    }
/* Computing MAX */
	    i__2 = t_subscr(ki, ki);
	    d__3 = ulp * ((d__1 = t[i__2].r, abs(d__1)) + (d__2 = d_imag(&
		    t_ref(ki, ki)), abs(d__2)));
	    smin = max(d__3,smlnum);

	    i__2 = *n;
	    work[i__2].r = 1., work[i__2].i = 0.;

/*           Form right-hand side. */

	    i__2 = *n;
	    for (k = ki + 1; k <= i__2; ++k) {
		i__3 = k;
		d_cnjg(&z__2, &t_ref(ki, k));
		z__1.r = -z__2.r, z__1.i = -z__2.i;
		work[i__3].r = z__1.r, work[i__3].i = z__1.i;
/* L90: */
	    }

/*           Solve the triangular system:   
                (T(KI+1:N,KI+1:N) - T(KI,KI))'*X = SCALE*WORK. */

	    i__2 = *n;
	    for (k = ki + 1; k <= i__2; ++k) {
		i__3 = t_subscr(k, k);
		i__4 = t_subscr(k, k);
		i__5 = t_subscr(ki, ki);
		z__1.r = t[i__4].r - t[i__5].r, z__1.i = t[i__4].i - t[i__5]
			.i;
		t[i__3].r = z__1.r, t[i__3].i = z__1.i;
		i__3 = t_subscr(k, k);
		if ((d__1 = t[i__3].r, abs(d__1)) + (d__2 = d_imag(&t_ref(k, 
			k)), abs(d__2)) < smin) {
		    i__4 = t_subscr(k, k);
		    t[i__4].r = smin, t[i__4].i = 0.;
		}
/* L100: */
	    }

	    if (ki < *n) {
		i__2 = *n - ki;
		zlatrs_("Upper", "Conjugate transpose", "Non-unit", "Y", &
			i__2, &t_ref(ki + 1, ki + 1), ldt, &work[ki + 1], &
			scale, &rwork[1], info);
		i__2 = ki;
		work[i__2].r = scale, work[i__2].i = 0.;
	    }

/*           Copy the vector x or Q*x to VL and normalize. */

	    if (! over) {
		i__2 = *n - ki + 1;
		zcopy_(&i__2, &work[ki], &c__1, &vl_ref(ki, is), &c__1);

		i__2 = *n - ki + 1;
		ii = izamax_(&i__2, &vl_ref(ki, is), &c__1) + ki - 1;
		i__2 = vl_subscr(ii, is);
		remax = 1. / ((d__1 = vl[i__2].r, abs(d__1)) + (d__2 = d_imag(
			&vl_ref(ii, is)), abs(d__2)));
		i__2 = *n - ki + 1;
		zdscal_(&i__2, &remax, &vl_ref(ki, is), &c__1);

		i__2 = ki - 1;
		for (k = 1; k <= i__2; ++k) {
		    i__3 = vl_subscr(k, is);
		    vl[i__3].r = 0., vl[i__3].i = 0.;
/* L110: */
		}
	    } else {
		if (ki < *n) {
		    i__2 = *n - ki;
		    z__1.r = scale, z__1.i = 0.;
		    zgemv_("N", n, &i__2, &c_b2, &vl_ref(1, ki + 1), ldvl, &
			    work[ki + 1], &c__1, &z__1, &vl_ref(1, ki), &c__1);
		}

		ii = izamax_(n, &vl_ref(1, ki), &c__1);
		i__2 = vl_subscr(ii, ki);
		remax = 1. / ((d__1 = vl[i__2].r, abs(d__1)) + (d__2 = d_imag(
			&vl_ref(ii, ki)), abs(d__2)));
		zdscal_(n, &remax, &vl_ref(1, ki), &c__1);
	    }

/*           Set back the original diagonal elements of T. */

	    i__2 = *n;
	    for (k = ki + 1; k <= i__2; ++k) {
		i__3 = t_subscr(k, k);
		i__4 = k + *n;
		t[i__3].r = work[i__4].r, t[i__3].i = work[i__4].i;
/* L120: */
	    }

	    ++is;
L130:
	    ;
	}
    }

    return 0;

/*     End of ZTREVC */

} /* ztrevc_ */

#undef vr_ref
#undef vr_subscr
#undef vl_ref
#undef vl_subscr
#undef t_ref
#undef t_subscr


