#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dgbbrd_(char *vect, integer *m, integer *n, integer *ncc,
	 integer *kl, integer *ku, doublereal *ab, integer *ldab, doublereal *
	d__, doublereal *e, doublereal *q, integer *ldq, doublereal *pt, 
	integer *ldpt, doublereal *c__, integer *ldc, doublereal *work, 
	integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    DGBBRD reduces a real general m-by-n band matrix A to upper   
    bidiagonal form B by an orthogonal transformation: Q' * A * P = B.   

    The routine computes B, and optionally forms Q or P', or computes   
    Q'*C for a given matrix C.   

    Arguments   
    =========   

    VECT    (input) CHARACTER*1   
            Specifies whether or not the matrices Q and P' are to be   
            formed.   
            = 'N': do not form Q or P';   
            = 'Q': form Q only;   
            = 'P': form P' only;   
            = 'B': form both.   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    NCC     (input) INTEGER   
            The number of columns of the matrix C.  NCC >= 0.   

    KL      (input) INTEGER   
            The number of subdiagonals of the matrix A. KL >= 0.   

    KU      (input) INTEGER   
            The number of superdiagonals of the matrix A. KU >= 0.   

    AB      (input/output) DOUBLE PRECISION array, dimension (LDAB,N)   
            On entry, the m-by-n band matrix A, stored in rows 1 to   
            KL+KU+1. The j-th column of A is stored in the j-th column of   
            the array AB as follows:   
            AB(ku+1+i-j,j) = A(i,j) for max(1,j-ku)<=i<=min(m,j+kl).   
            On exit, A is overwritten by values generated during the   
            reduction.   

    LDAB    (input) INTEGER   
            The leading dimension of the array A. LDAB >= KL+KU+1.   

    D       (output) DOUBLE PRECISION array, dimension (min(M,N))   
            The diagonal elements of the bidiagonal matrix B.   

    E       (output) DOUBLE PRECISION array, dimension (min(M,N)-1)   
            The superdiagonal elements of the bidiagonal matrix B.   

    Q       (output) DOUBLE PRECISION array, dimension (LDQ,M)   
            If VECT = 'Q' or 'B', the m-by-m orthogonal matrix Q.   
            If VECT = 'N' or 'P', the array Q is not referenced.   

    LDQ     (input) INTEGER   
            The leading dimension of the array Q.   
            LDQ >= max(1,M) if VECT = 'Q' or 'B'; LDQ >= 1 otherwise.   

    PT      (output) DOUBLE PRECISION array, dimension (LDPT,N)   
            If VECT = 'P' or 'B', the n-by-n orthogonal matrix P'.   
            If VECT = 'N' or 'Q', the array PT is not referenced.   

    LDPT    (input) INTEGER   
            The leading dimension of the array PT.   
            LDPT >= max(1,N) if VECT = 'P' or 'B'; LDPT >= 1 otherwise.   

    C       (input/output) DOUBLE PRECISION array, dimension (LDC,NCC)   
            On entry, an m-by-ncc matrix C.   
            On exit, C is overwritten by Q'*C.   
            C is not referenced if NCC = 0.   

    LDC     (input) INTEGER   
            The leading dimension of the array C.   
            LDC >= max(1,M) if NCC > 0; LDC >= 1 if NCC = 0.   

    WORK    (workspace) DOUBLE PRECISION array, dimension (2*max(M,N))   

    INFO    (output) INTEGER   
            = 0:  successful exit.   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   

    =====================================================================   


       Test the input parameters   

       Parameter adjustments */
    /* Table of constant values */
    static doublereal c_b8 = 0.;
    static doublereal c_b9 = 1.;
    static integer c__1 = 1;
    
    /* System generated locals */
    integer ab_dim1, ab_offset, c_dim1, c_offset, pt_dim1, pt_offset, q_dim1, 
	    q_offset, i__1, i__2, i__3, i__4, i__5, i__6, i__7;
    /* Local variables */
    static integer inca;
    extern /* Subroutine */ int drot_(integer *, doublereal *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *);
    static integer i__, j, l;
    extern logical lsame_(char *, char *);
    static logical wantb, wantc;
    static integer minmn;
    static logical wantq;
    static integer j1, j2, kb;
    static doublereal ra, rb;
    static integer kk;
    static doublereal rc;
    static integer ml, mn, nr, mu;
    static doublereal rs;
    extern /* Subroutine */ int dlaset_(char *, integer *, integer *, 
	    doublereal *, doublereal *, doublereal *, integer *), 
	    dlartg_(doublereal *, doublereal *, doublereal *, doublereal *, 
	    doublereal *), xerbla_(char *, integer *), dlargv_(
	    integer *, doublereal *, integer *, doublereal *, integer *, 
	    doublereal *, integer *), dlartv_(integer *, doublereal *, 
	    integer *, doublereal *, integer *, doublereal *, doublereal *, 
	    integer *);
    static integer kb1, ml0;
    static logical wantpt;
    static integer mu0, klm, kun, nrt, klu1;
#define c___ref(a_1,a_2) c__[(a_2)*c_dim1 + a_1]
#define q_ref(a_1,a_2) q[(a_2)*q_dim1 + a_1]
#define ab_ref(a_1,a_2) ab[(a_2)*ab_dim1 + a_1]
#define pt_ref(a_1,a_2) pt[(a_2)*pt_dim1 + a_1]


    ab_dim1 = *ldab;
    ab_offset = 1 + ab_dim1 * 1;
    ab -= ab_offset;
    --d__;
    --e;
    q_dim1 = *ldq;
    q_offset = 1 + q_dim1 * 1;
    q -= q_offset;
    pt_dim1 = *ldpt;
    pt_offset = 1 + pt_dim1 * 1;
    pt -= pt_offset;
    c_dim1 = *ldc;
    c_offset = 1 + c_dim1 * 1;
    c__ -= c_offset;
    --work;

    /* Function Body */
    wantb = lsame_(vect, "B");
    wantq = lsame_(vect, "Q") || wantb;
    wantpt = lsame_(vect, "P") || wantb;
    wantc = *ncc > 0;
    klu1 = *kl + *ku + 1;
    *info = 0;
    if (! wantq && ! wantpt && ! lsame_(vect, "N")) {
	*info = -1;
    } else if (*m < 0) {
	*info = -2;
    } else if (*n < 0) {
	*info = -3;
    } else if (*ncc < 0) {
	*info = -4;
    } else if (*kl < 0) {
	*info = -5;
    } else if (*ku < 0) {
	*info = -6;
    } else if (*ldab < klu1) {
	*info = -8;
    } else if (*ldq < 1 || wantq && *ldq < max(1,*m)) {
	*info = -12;
    } else if (*ldpt < 1 || wantpt && *ldpt < max(1,*n)) {
	*info = -14;
    } else if (*ldc < 1 || wantc && *ldc < max(1,*m)) {
	*info = -16;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGBBRD", &i__1);
	return 0;
    }

/*     Initialize Q and P' to the unit matrix, if needed */

    if (wantq) {
	dlaset_("Full", m, m, &c_b8, &c_b9, &q[q_offset], ldq);
    }
    if (wantpt) {
	dlaset_("Full", n, n, &c_b8, &c_b9, &pt[pt_offset], ldpt);
    }

/*     Quick return if possible. */

    if (*m == 0 || *n == 0) {
	return 0;
    }

    minmn = min(*m,*n);

    if (*kl + *ku > 1) {

/*        Reduce to upper bidiagonal form if KU > 0; if KU = 0, reduce   
          first to lower bidiagonal form and then transform to upper   
          bidiagonal */

	if (*ku > 0) {
	    ml0 = 1;
	    mu0 = 2;
	} else {
	    ml0 = 2;
	    mu0 = 1;
	}

/*        Wherever possible, plane rotations are generated and applied in   
          vector operations of length NR over the index set J1:J2:KLU1.   

          The sines of the plane rotations are stored in WORK(1:max(m,n))   
          and the cosines in WORK(max(m,n)+1:2*max(m,n)). */

	mn = max(*m,*n);
/* Computing MIN */
	i__1 = *m - 1;
	klm = min(i__1,*kl);
/* Computing MIN */
	i__1 = *n - 1;
	kun = min(i__1,*ku);
	kb = klm + kun;
	kb1 = kb + 1;
	inca = kb1 * *ldab;
	nr = 0;
	j1 = klm + 2;
	j2 = 1 - kun;

	i__1 = minmn;
	for (i__ = 1; i__ <= i__1; ++i__) {

/*           Reduce i-th column and i-th row of matrix to bidiagonal form */

	    ml = klm + 1;
	    mu = kun + 1;
	    i__2 = kb;
	    for (kk = 1; kk <= i__2; ++kk) {
		j1 += kb;
		j2 += kb;

/*              generate plane rotations to annihilate nonzero elements   
                which have been created below the band */

		if (nr > 0) {
		    dlargv_(&nr, &ab_ref(klu1, j1 - klm - 1), &inca, &work[j1]
			    , &kb1, &work[mn + j1], &kb1);
		}

/*              apply plane rotations from the left */

		i__3 = kb;
		for (l = 1; l <= i__3; ++l) {
		    if (j2 - klm + l - 1 > *n) {
			nrt = nr - 1;
		    } else {
			nrt = nr;
		    }
		    if (nrt > 0) {
			dlartv_(&nrt, &ab_ref(klu1 - l, j1 - klm + l - 1), &
				inca, &ab_ref(klu1 - l + 1, j1 - klm + l - 1),
				 &inca, &work[mn + j1], &work[j1], &kb1);
		    }
/* L10: */
		}

		if (ml > ml0) {
		    if (ml <= *m - i__ + 1) {

/*                    generate plane rotation to annihilate a(i+ml-1,i)   
                      within the band, and apply rotation from the left */

			dlartg_(&ab_ref(*ku + ml - 1, i__), &ab_ref(*ku + ml, 
				i__), &work[mn + i__ + ml - 1], &work[i__ + 
				ml - 1], &ra);
			ab_ref(*ku + ml - 1, i__) = ra;
			if (i__ < *n) {
/* Computing MIN */
			    i__4 = *ku + ml - 2, i__5 = *n - i__;
			    i__3 = min(i__4,i__5);
			    i__6 = *ldab - 1;
			    i__7 = *ldab - 1;
			    drot_(&i__3, &ab_ref(*ku + ml - 2, i__ + 1), &
				    i__6, &ab_ref(*ku + ml - 1, i__ + 1), &
				    i__7, &work[mn + i__ + ml - 1], &work[i__ 
				    + ml - 1]);
			}
		    }
		    ++nr;
		    j1 -= kb1;
		}

		if (wantq) {

/*                 accumulate product of plane rotations in Q */

		    i__3 = j2;
		    i__4 = kb1;
		    for (j = j1; i__4 < 0 ? j >= i__3 : j <= i__3; j += i__4) 
			    {
			drot_(m, &q_ref(1, j - 1), &c__1, &q_ref(1, j), &c__1,
				 &work[mn + j], &work[j]);
/* L20: */
		    }
		}

		if (wantc) {

/*                 apply plane rotations to C */

		    i__4 = j2;
		    i__3 = kb1;
		    for (j = j1; i__3 < 0 ? j >= i__4 : j <= i__4; j += i__3) 
			    {
			drot_(ncc, &c___ref(j - 1, 1), ldc, &c___ref(j, 1), 
				ldc, &work[mn + j], &work[j]);
/* L30: */
		    }
		}

		if (j2 + kun > *n) {

/*                 adjust J2 to keep within the bounds of the matrix */

		    --nr;
		    j2 -= kb1;
		}

		i__3 = j2;
		i__4 = kb1;
		for (j = j1; i__4 < 0 ? j >= i__3 : j <= i__3; j += i__4) {

/*                 create nonzero element a(j-1,j+ku) above the band   
                   and store it in WORK(n+1:2*n) */

		    work[j + kun] = work[j] * ab_ref(1, j + kun);
		    ab_ref(1, j + kun) = work[mn + j] * ab_ref(1, j + kun);
/* L40: */
		}

/*              generate plane rotations to annihilate nonzero elements   
                which have been generated above the band */

		if (nr > 0) {
		    dlargv_(&nr, &ab_ref(1, j1 + kun - 1), &inca, &work[j1 + 
			    kun], &kb1, &work[mn + j1 + kun], &kb1);
		}

/*              apply plane rotations from the right */

		i__4 = kb;
		for (l = 1; l <= i__4; ++l) {
		    if (j2 + l - 1 > *m) {
			nrt = nr - 1;
		    } else {
			nrt = nr;
		    }
		    if (nrt > 0) {
			dlartv_(&nrt, &ab_ref(l + 1, j1 + kun - 1), &inca, &
				ab_ref(l, j1 + kun), &inca, &work[mn + j1 + 
				kun], &work[j1 + kun], &kb1);
		    }
/* L50: */
		}

		if (ml == ml0 && mu > mu0) {
		    if (mu <= *n - i__ + 1) {

/*                    generate plane rotation to annihilate a(i,i+mu-1)   
                      within the band, and apply rotation from the right */

			dlartg_(&ab_ref(*ku - mu + 3, i__ + mu - 2), &ab_ref(*
				ku - mu + 2, i__ + mu - 1), &work[mn + i__ + 
				mu - 1], &work[i__ + mu - 1], &ra);
			ab_ref(*ku - mu + 3, i__ + mu - 2) = ra;
/* Computing MIN */
			i__3 = *kl + mu - 2, i__5 = *m - i__;
			i__4 = min(i__3,i__5);
			drot_(&i__4, &ab_ref(*ku - mu + 4, i__ + mu - 2), &
				c__1, &ab_ref(*ku - mu + 3, i__ + mu - 1), &
				c__1, &work[mn + i__ + mu - 1], &work[i__ + 
				mu - 1]);
		    }
		    ++nr;
		    j1 -= kb1;
		}

		if (wantpt) {

/*                 accumulate product of plane rotations in P' */

		    i__4 = j2;
		    i__3 = kb1;
		    for (j = j1; i__3 < 0 ? j >= i__4 : j <= i__4; j += i__3) 
			    {
			drot_(n, &pt_ref(j + kun - 1, 1), ldpt, &pt_ref(j + 
				kun, 1), ldpt, &work[mn + j + kun], &work[j + 
				kun]);
/* L60: */
		    }
		}

		if (j2 + kb > *m) {

/*                 adjust J2 to keep within the bounds of the matrix */

		    --nr;
		    j2 -= kb1;
		}

		i__3 = j2;
		i__4 = kb1;
		for (j = j1; i__4 < 0 ? j >= i__3 : j <= i__3; j += i__4) {

/*                 create nonzero element a(j+kl+ku,j+ku-1) below the   
                   band and store it in WORK(1:n) */

		    work[j + kb] = work[j + kun] * ab_ref(klu1, j + kun);
		    ab_ref(klu1, j + kun) = work[mn + j + kun] * ab_ref(klu1, 
			    j + kun);
/* L70: */
		}

		if (ml > ml0) {
		    --ml;
		} else {
		    --mu;
		}
/* L80: */
	    }
/* L90: */
	}
    }

    if (*ku == 0 && *kl > 0) {

/*        A has been reduced to lower bidiagonal form   

          Transform lower bidiagonal form to upper bidiagonal by applying   
          plane rotations from the left, storing diagonal elements in D   
          and off-diagonal elements in E   

   Computing MIN */
	i__2 = *m - 1;
	i__1 = min(i__2,*n);
	for (i__ = 1; i__ <= i__1; ++i__) {
	    dlartg_(&ab_ref(1, i__), &ab_ref(2, i__), &rc, &rs, &ra);
	    d__[i__] = ra;
	    if (i__ < *n) {
		e[i__] = rs * ab_ref(1, i__ + 1);
		ab_ref(1, i__ + 1) = rc * ab_ref(1, i__ + 1);
	    }
	    if (wantq) {
		drot_(m, &q_ref(1, i__), &c__1, &q_ref(1, i__ + 1), &c__1, &
			rc, &rs);
	    }
	    if (wantc) {
		drot_(ncc, &c___ref(i__, 1), ldc, &c___ref(i__ + 1, 1), ldc, &
			rc, &rs);
	    }
/* L100: */
	}
	if (*m <= *n) {
	    d__[*m] = ab_ref(1, *m);
	}
    } else if (*ku > 0) {

/*        A has been reduced to upper bidiagonal form */

	if (*m < *n) {

/*           Annihilate a(m,m+1) by applying plane rotations from the   
             right, storing diagonal elements in D and off-diagonal   
             elements in E */

	    rb = ab_ref(*ku, *m + 1);
	    for (i__ = *m; i__ >= 1; --i__) {
		dlartg_(&ab_ref(*ku + 1, i__), &rb, &rc, &rs, &ra);
		d__[i__] = ra;
		if (i__ > 1) {
		    rb = -rs * ab_ref(*ku, i__);
		    e[i__ - 1] = rc * ab_ref(*ku, i__);
		}
		if (wantpt) {
		    drot_(n, &pt_ref(i__, 1), ldpt, &pt_ref(*m + 1, 1), ldpt, 
			    &rc, &rs);
		}
/* L110: */
	    }
	} else {

/*           Copy off-diagonal elements to E and diagonal elements to D */

	    i__1 = minmn - 1;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		e[i__] = ab_ref(*ku, i__ + 1);
/* L120: */
	    }
	    i__1 = minmn;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		d__[i__] = ab_ref(*ku + 1, i__);
/* L130: */
	    }
	}
    } else {

/*        A is diagonal. Set elements of E to zero and copy diagonal   
          elements to D. */

	i__1 = minmn - 1;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    e[i__] = 0.;
/* L140: */
	}
	i__1 = minmn;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    d__[i__] = ab_ref(1, i__);
/* L150: */
	}
    }
    return 0;

/*     End of DGBBRD */

} /* dgbbrd_ */

#undef pt_ref
#undef ab_ref
#undef q_ref
#undef c___ref


