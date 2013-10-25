#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dbdsdc_(char *uplo, char *compq, integer *n, doublereal *
	d__, doublereal *e, doublereal *u, integer *ldu, doublereal *vt, 
	integer *ldvt, doublereal *q, integer *iq, doublereal *work, integer *
	iwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       December 1, 1999   


    Purpose   
    =======   

    DBDSDC computes the singular value decomposition (SVD) of a real   
    N-by-N (upper or lower) bidiagonal matrix B:  B = U * S * VT,   
    using a divide and conquer method, where S is a diagonal matrix   
    with non-negative diagonal elements (the singular values of B), and   
    U and VT are orthogonal matrices of left and right singular vectors,   
    respectively. DBDSDC can be used to compute all singular values,   
    and optionally, singular vectors or singular vectors in compact form.   

    This code makes very mild assumptions about floating point   
    arithmetic. It will work on machines with a guard digit in   
    add/subtract, or on those binary machines without guard digits   
    which subtract like the Cray X-MP, Cray Y-MP, Cray C-90, or Cray-2.   
    It could conceivably fail on hexadecimal or decimal machines   
    without guard digits, but we know of none.  See DLASD3 for details.   

    The code currently call DLASDQ if singular values only are desired.   
    However, it can be slightly modified to compute singular values   
    using the divide and conquer method.   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            = 'U':  B is upper bidiagonal.   
            = 'L':  B is lower bidiagonal.   

    COMPQ   (input) CHARACTER*1   
            Specifies whether singular vectors are to be computed   
            as follows:   
            = 'N':  Compute singular values only;   
            = 'P':  Compute singular values and compute singular   
                    vectors in compact form;   
            = 'I':  Compute singular values and singular vectors.   

    N       (input) INTEGER   
            The order of the matrix B.  N >= 0.   

    D       (input/output) DOUBLE PRECISION array, dimension (N)   
            On entry, the n diagonal elements of the bidiagonal matrix B.   
            On exit, if INFO=0, the singular values of B.   

    E       (input/output) DOUBLE PRECISION array, dimension (N)   
            On entry, the elements of E contain the offdiagonal   
            elements of the bidiagonal matrix whose SVD is desired.   
            On exit, E has been destroyed.   

    U       (output) DOUBLE PRECISION array, dimension (LDU,N)   
            If  COMPQ = 'I', then:   
               On exit, if INFO = 0, U contains the left singular vectors   
               of the bidiagonal matrix.   
            For other values of COMPQ, U is not referenced.   

    LDU     (input) INTEGER   
            The leading dimension of the array U.  LDU >= 1.   
            If singular vectors are desired, then LDU >= max( 1, N ).   

    VT      (output) DOUBLE PRECISION array, dimension (LDVT,N)   
            If  COMPQ = 'I', then:   
               On exit, if INFO = 0, VT' contains the right singular   
               vectors of the bidiagonal matrix.   
            For other values of COMPQ, VT is not referenced.   

    LDVT    (input) INTEGER   
            The leading dimension of the array VT.  LDVT >= 1.   
            If singular vectors are desired, then LDVT >= max( 1, N ).   

    Q       (output) DOUBLE PRECISION array, dimension (LDQ)   
            If  COMPQ = 'P', then:   
               On exit, if INFO = 0, Q and IQ contain the left   
               and right singular vectors in a compact form,   
               requiring O(N log N) space instead of 2*N**2.   
               In particular, Q contains all the DOUBLE PRECISION data in   
               LDQ >= N*(11 + 2*SMLSIZ + 8*INT(LOG_2(N/(SMLSIZ+1))))   
               words of memory, where SMLSIZ is returned by ILAENV and   
               is equal to the maximum size of the subproblems at the   
               bottom of the computation tree (usually about 25).   
            For other values of COMPQ, Q is not referenced.   

    IQ      (output) INTEGER array, dimension (LDIQ)   
            If  COMPQ = 'P', then:   
               On exit, if INFO = 0, Q and IQ contain the left   
               and right singular vectors in a compact form,   
               requiring O(N log N) space instead of 2*N**2.   
               In particular, IQ contains all INTEGER data in   
               LDIQ >= N*(3 + 3*INT(LOG_2(N/(SMLSIZ+1))))   
               words of memory, where SMLSIZ is returned by ILAENV and   
               is equal to the maximum size of the subproblems at the   
               bottom of the computation tree (usually about 25).   
            For other values of COMPQ, IQ is not referenced.   

    WORK    (workspace) DOUBLE PRECISION array, dimension (LWORK)   
            If COMPQ = 'N' then LWORK >= (4 * N).   
            If COMPQ = 'P' then LWORK >= (6 * N).   
            If COMPQ = 'I' then LWORK >= (3 * N**2 + 4 * N).   

    IWORK   (workspace) INTEGER array, dimension (8*N)   

    INFO    (output) INTEGER   
            = 0:  successful exit.   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   
            > 0:  The algorithm failed to compute an singular value.   
                  The update process of divide and conquer failed.   

    Further Details   
    ===============   

    Based on contributions by   
       Ming Gu and Huan Ren, Computer Science Division, University of   
       California at Berkeley, USA   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__9 = 9;
    static integer c__0 = 0;
    static doublereal c_b15 = 1.;
    static integer c__1 = 1;
    static doublereal c_b29 = 0.;
    
    /* System generated locals */
    integer u_dim1, u_offset, vt_dim1, vt_offset, i__1, i__2;
    doublereal d__1;
    /* Builtin functions */
    double d_sign(doublereal *, doublereal *), log(doublereal);
    /* Local variables */
    static integer difl, difr, ierr, perm, mlvl, sqre, i__, j, k;
    static doublereal p, r__;
    static integer z__;
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int dlasr_(char *, char *, char *, integer *, 
	    integer *, doublereal *, doublereal *, doublereal *, integer *), dcopy_(integer *, doublereal *, integer *
	    , doublereal *, integer *), dswap_(integer *, doublereal *, 
	    integer *, doublereal *, integer *);
    static integer poles, iuplo, nsize, start;
    extern /* Subroutine */ int dlasd0_(integer *, integer *, doublereal *, 
	    doublereal *, doublereal *, integer *, doublereal *, integer *, 
	    integer *, integer *, doublereal *, integer *);
    static integer ic, ii, kk;
    static doublereal cs;
    extern doublereal dlamch_(char *);
    extern /* Subroutine */ int dlasda_(integer *, integer *, integer *, 
	    integer *, doublereal *, doublereal *, doublereal *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, doublereal *,
	     doublereal *, integer *, integer *, integer *, integer *, 
	    doublereal *, doublereal *, doublereal *, doublereal *, integer *,
	     integer *);
    static integer is, iu;
    static doublereal sn;
    extern /* Subroutine */ int dlascl_(char *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, integer *, doublereal *, 
	    integer *, integer *), dlasdq_(char *, integer *, integer 
	    *, integer *, integer *, integer *, doublereal *, doublereal *, 
	    doublereal *, integer *, doublereal *, integer *, doublereal *, 
	    integer *, doublereal *, integer *), dlaset_(char *, 
	    integer *, integer *, doublereal *, doublereal *, doublereal *, 
	    integer *), dlartg_(doublereal *, doublereal *, 
	    doublereal *, doublereal *, doublereal *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen);
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static integer givcol;
    extern doublereal dlanst_(char *, integer *, doublereal *, doublereal *);
    static integer icompq;
    static doublereal orgnrm;
    static integer givnum, givptr, nm1, qstart, smlsiz, wstart, smlszp;
    static doublereal eps;
    static integer ivt;
#define u_ref(a_1,a_2) u[(a_2)*u_dim1 + a_1]
#define vt_ref(a_1,a_2) vt[(a_2)*vt_dim1 + a_1]


    --d__;
    --e;
    u_dim1 = *ldu;
    u_offset = 1 + u_dim1 * 1;
    u -= u_offset;
    vt_dim1 = *ldvt;
    vt_offset = 1 + vt_dim1 * 1;
    vt -= vt_offset;
    --q;
    --iq;
    --work;
    --iwork;

    /* Function Body */
    *info = 0;

    iuplo = 0;
    if (lsame_(uplo, "U")) {
	iuplo = 1;
    }
    if (lsame_(uplo, "L")) {
	iuplo = 2;
    }
    if (lsame_(compq, "N")) {
	icompq = 0;
    } else if (lsame_(compq, "P")) {
	icompq = 1;
    } else if (lsame_(compq, "I")) {
	icompq = 2;
    } else {
	icompq = -1;
    }
    if (iuplo == 0) {
	*info = -1;
    } else if (icompq < 0) {
	*info = -2;
    } else if (*n < 0) {
	*info = -3;
    } else if (*ldu < 1 || icompq == 2 && *ldu < *n) {
	*info = -7;
    } else if (*ldvt < 1 || icompq == 2 && *ldvt < *n) {
	*info = -9;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DBDSDC", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }
    smlsiz = ilaenv_(&c__9, "DBDSDC", " ", &c__0, &c__0, &c__0, &c__0, (
	    ftnlen)6, (ftnlen)1);
    if (*n == 1) {
	if (icompq == 1) {
	    q[1] = d_sign(&c_b15, &d__[1]);
	    q[smlsiz * *n + 1] = 1.;
	} else if (icompq == 2) {
	    u_ref(1, 1) = d_sign(&c_b15, &d__[1]);
	    vt_ref(1, 1) = 1.;
	}
	d__[1] = abs(d__[1]);
	return 0;
    }
    nm1 = *n - 1;

/*     If matrix lower bidiagonal, rotate to be upper bidiagonal   
       by applying Givens rotations on the left */

    wstart = 1;
    qstart = 3;
    if (icompq == 1) {
	dcopy_(n, &d__[1], &c__1, &q[1], &c__1);
	i__1 = *n - 1;
	dcopy_(&i__1, &e[1], &c__1, &q[*n + 1], &c__1);
    }
    if (iuplo == 2) {
	qstart = 5;
	wstart = (*n << 1) - 1;
	i__1 = *n - 1;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    dlartg_(&d__[i__], &e[i__], &cs, &sn, &r__);
	    d__[i__] = r__;
	    e[i__] = sn * d__[i__ + 1];
	    d__[i__ + 1] = cs * d__[i__ + 1];
	    if (icompq == 1) {
		q[i__ + (*n << 1)] = cs;
		q[i__ + *n * 3] = sn;
	    } else if (icompq == 2) {
		work[i__] = cs;
		work[nm1 + i__] = -sn;
	    }
/* L10: */
	}
    }

/*     If ICOMPQ = 0, use DLASDQ to compute the singular values. */

    if (icompq == 0) {
	dlasdq_("U", &c__0, n, &c__0, &c__0, &c__0, &d__[1], &e[1], &vt[
		vt_offset], ldvt, &u[u_offset], ldu, &u[u_offset], ldu, &work[
		wstart], info);
	goto L40;
    }

/*     If N is smaller than the minimum divide size SMLSIZ, then solve   
       the problem with another solver. */

    if (*n <= smlsiz) {
	if (icompq == 2) {
	    dlaset_("A", n, n, &c_b29, &c_b15, &u[u_offset], ldu);
	    dlaset_("A", n, n, &c_b29, &c_b15, &vt[vt_offset], ldvt);
	    dlasdq_("U", &c__0, n, n, n, &c__0, &d__[1], &e[1], &vt[vt_offset]
		    , ldvt, &u[u_offset], ldu, &u[u_offset], ldu, &work[
		    wstart], info);
	} else if (icompq == 1) {
	    iu = 1;
	    ivt = iu + *n;
	    dlaset_("A", n, n, &c_b29, &c_b15, &q[iu + (qstart - 1) * *n], n);
	    dlaset_("A", n, n, &c_b29, &c_b15, &q[ivt + (qstart - 1) * *n], n);
	    dlasdq_("U", &c__0, n, n, n, &c__0, &d__[1], &e[1], &q[ivt + (
		    qstart - 1) * *n], n, &q[iu + (qstart - 1) * *n], n, &q[
		    iu + (qstart - 1) * *n], n, &work[wstart], info);
	}
	goto L40;
    }

    if (icompq == 2) {
	dlaset_("A", n, n, &c_b29, &c_b15, &u[u_offset], ldu);
	dlaset_("A", n, n, &c_b29, &c_b15, &vt[vt_offset], ldvt);
    }

/*     Scale. */

    orgnrm = dlanst_("M", n, &d__[1], &e[1]);
    if (orgnrm == 0.) {
	return 0;
    }
    dlascl_("G", &c__0, &c__0, &orgnrm, &c_b15, n, &c__1, &d__[1], n, &ierr);
    dlascl_("G", &c__0, &c__0, &orgnrm, &c_b15, &nm1, &c__1, &e[1], &nm1, &
	    ierr);

    eps = dlamch_("Epsilon");

    mlvl = (integer) (log((doublereal) (*n) / (doublereal) (smlsiz + 1)) / 
	    log(2.)) + 1;
    smlszp = smlsiz + 1;

    if (icompq == 1) {
	iu = 1;
	ivt = smlsiz + 1;
	difl = ivt + smlszp;
	difr = difl + mlvl;
	z__ = difr + (mlvl << 1);
	ic = z__ + mlvl;
	is = ic + 1;
	poles = is + 1;
	givnum = poles + (mlvl << 1);

	k = 1;
	givptr = 2;
	perm = 3;
	givcol = perm + mlvl;
    }

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if ((d__1 = d__[i__], abs(d__1)) < eps) {
	    d__[i__] = d_sign(&eps, &d__[i__]);
	}
/* L20: */
    }

    start = 1;
    sqre = 0;

    i__1 = nm1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if ((d__1 = e[i__], abs(d__1)) < eps || i__ == nm1) {

/*        Subproblem found. First determine its size and then   
          apply divide and conquer on it. */

	    if (i__ < nm1) {

/*        A subproblem with E(I) small for I < NM1. */

		nsize = i__ - start + 1;
	    } else if ((d__1 = e[i__], abs(d__1)) >= eps) {

/*        A subproblem with E(NM1) not too small but I = NM1. */

		nsize = *n - start + 1;
	    } else {

/*        A subproblem with E(NM1) small. This implies an   
          1-by-1 subproblem at D(N). Solve this 1-by-1 problem   
          first. */

		nsize = i__ - start + 1;
		if (icompq == 2) {
		    u_ref(*n, *n) = d_sign(&c_b15, &d__[*n]);
		    vt_ref(*n, *n) = 1.;
		} else if (icompq == 1) {
		    q[*n + (qstart - 1) * *n] = d_sign(&c_b15, &d__[*n]);
		    q[*n + (smlsiz + qstart - 1) * *n] = 1.;
		}
		d__[*n] = (d__1 = d__[*n], abs(d__1));
	    }
	    if (icompq == 2) {
		dlasd0_(&nsize, &sqre, &d__[start], &e[start], &u_ref(start, 
			start), ldu, &vt_ref(start, start), ldvt, &smlsiz, &
			iwork[1], &work[wstart], info);
	    } else {
		dlasda_(&icompq, &smlsiz, &nsize, &sqre, &d__[start], &e[
			start], &q[start + (iu + qstart - 2) * *n], n, &q[
			start + (ivt + qstart - 2) * *n], &iq[start + k * *n],
			 &q[start + (difl + qstart - 2) * *n], &q[start + (
			difr + qstart - 2) * *n], &q[start + (z__ + qstart - 
			2) * *n], &q[start + (poles + qstart - 2) * *n], &iq[
			start + givptr * *n], &iq[start + givcol * *n], n, &
			iq[start + perm * *n], &q[start + (givnum + qstart - 
			2) * *n], &q[start + (ic + qstart - 2) * *n], &q[
			start + (is + qstart - 2) * *n], &work[wstart], &
			iwork[1], info);
		if (*info != 0) {
		    return 0;
		}
	    }
	    start = i__ + 1;
	}
/* L30: */
    }

/*     Unscale */

    dlascl_("G", &c__0, &c__0, &c_b15, &orgnrm, n, &c__1, &d__[1], n, &ierr);
L40:

/*     Use Selection Sort to minimize swaps of singular vectors */

    i__1 = *n;
    for (ii = 2; ii <= i__1; ++ii) {
	i__ = ii - 1;
	kk = i__;
	p = d__[i__];
	i__2 = *n;
	for (j = ii; j <= i__2; ++j) {
	    if (d__[j] > p) {
		kk = j;
		p = d__[j];
	    }
/* L50: */
	}
	if (kk != i__) {
	    d__[kk] = d__[i__];
	    d__[i__] = p;
	    if (icompq == 1) {
		iq[i__] = kk;
	    } else if (icompq == 2) {
		dswap_(n, &u_ref(1, i__), &c__1, &u_ref(1, kk), &c__1);
		dswap_(n, &vt_ref(i__, 1), ldvt, &vt_ref(kk, 1), ldvt);
	    }
	} else if (icompq == 1) {
	    iq[i__] = i__;
	}
/* L60: */
    }

/*     If ICOMPQ = 1, use IQ(N,1) as the indicator for UPLO */

    if (icompq == 1) {
	if (iuplo == 1) {
	    iq[*n] = 1;
	} else {
	    iq[*n] = 0;
	}
    }

/*     If B is lower bidiagonal, update U by those Givens rotations   
       which rotated B to be upper bidiagonal */

    if (iuplo == 2 && icompq == 2) {
	dlasr_("L", "V", "B", n, n, &work[1], &work[*n], &u[u_offset], ldu);
    }

    return 0;

/*     End of DBDSDC */

} /* dbdsdc_ */

#undef vt_ref
#undef u_ref


