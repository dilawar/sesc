#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int cgbbrd_(char *vect, integer *m, integer *n, integer *ncc,
	 integer *kl, integer *ku, complex *ab, integer *ldab, real *d__, 
	real *e, complex *q, integer *ldq, complex *pt, integer *ldpt, 
	complex *c__, integer *ldc, complex *work, real *rwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    CGBBRD reduces a complex general m-by-n band matrix A to real upper   
    bidiagonal form B by a unitary transformation: Q' * A * P = B.   

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

    AB      (input/output) COMPLEX array, dimension (LDAB,N)   
            On entry, the m-by-n band matrix A, stored in rows 1 to   
            KL+KU+1. The j-th column of A is stored in the j-th column of   
            the array AB as follows:   
            AB(ku+1+i-j,j) = A(i,j) for max(1,j-ku)<=i<=min(m,j+kl).   
            On exit, A is overwritten by values generated during the   
            reduction.   

    LDAB    (input) INTEGER   
            The leading dimension of the array A. LDAB >= KL+KU+1.   

    D       (output) REAL array, dimension (min(M,N))   
            The diagonal elements of the bidiagonal matrix B.   

    E       (output) REAL array, dimension (min(M,N)-1)   
            The superdiagonal elements of the bidiagonal matrix B.   

    Q       (output) COMPLEX array, dimension (LDQ,M)   
            If VECT = 'Q' or 'B', the m-by-m unitary matrix Q.   
            If VECT = 'N' or 'P', the array Q is not referenced.   

    LDQ     (input) INTEGER   
            The leading dimension of the array Q.   
            LDQ >= max(1,M) if VECT = 'Q' or 'B'; LDQ >= 1 otherwise.   

    PT      (output) COMPLEX array, dimension (LDPT,N)   
            If VECT = 'P' or 'B', the n-by-n unitary matrix P'.   
            If VECT = 'N' or 'Q', the array PT is not referenced.   

    LDPT    (input) INTEGER   
            The leading dimension of the array PT.   
            LDPT >= max(1,N) if VECT = 'P' or 'B'; LDPT >= 1 otherwise.   

    C       (input/output) COMPLEX array, dimension (LDC,NCC)   
            On entry, an m-by-ncc matrix C.   
            On exit, C is overwritten by Q'*C.   
            C is not referenced if NCC = 0.   

    LDC     (input) INTEGER   
            The leading dimension of the array C.   
            LDC >= max(1,M) if NCC > 0; LDC >= 1 if NCC = 0.   

    WORK    (workspace) COMPLEX array, dimension (max(M,N))   

    RWORK   (workspace) REAL array, dimension (max(M,N))   

    INFO    (output) INTEGER   
            = 0:  successful exit.   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   

    =====================================================================   


       Test the input parameters   

       Parameter adjustments */
    /* Table of constant values */
    static complex c_b1 = {0.f,0.f};
    static complex c_b2 = {1.f,0.f};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer ab_dim1, ab_offset, c_dim1, c_offset, pt_dim1, pt_offset, q_dim1, 
	    q_offset, i__1, i__2, i__3, i__4, i__5, i__6, i__7;
    complex q__1, q__2, q__3;
    /* Builtin functions */
    void r_cnjg(complex *, complex *);
    double c_abs(complex *);
    /* Local variables */
    static integer inca;
    static real abst;
    extern /* Subroutine */ int crot_(integer *, complex *, integer *, 
	    complex *, integer *, real *, complex *);
    static integer i__, j, l;
    static complex t;
    extern /* Subroutine */ int cscal_(integer *, complex *, complex *, 
	    integer *);
    extern logical lsame_(char *, char *);
    static logical wantb, wantc;
    static integer minmn;
    static logical wantq;
    static integer j1, j2, kb;
    static complex ra;
    static real rc;
    static integer kk;
    static complex rb;
    static integer ml, nr, mu;
    static complex rs;
    extern /* Subroutine */ int claset_(char *, integer *, integer *, complex 
	    *, complex *, complex *, integer *), clartg_(complex *, 
	    complex *, real *, complex *, complex *), xerbla_(char *, integer 
	    *), clargv_(integer *, complex *, integer *, complex *, 
	    integer *, real *, integer *), clartv_(integer *, complex *, 
	    integer *, complex *, integer *, real *, complex *, integer *);
    static integer kb1, ml0;
    static logical wantpt;
    static integer mu0, klm, kun, nrt, klu1;
#define c___subscr(a_1,a_2) (a_2)*c_dim1 + a_1
#define c___ref(a_1,a_2) c__[c___subscr(a_1,a_2)]
#define q_subscr(a_1,a_2) (a_2)*q_dim1 + a_1
#define q_ref(a_1,a_2) q[q_subscr(a_1,a_2)]
#define ab_subscr(a_1,a_2) (a_2)*ab_dim1 + a_1
#define ab_ref(a_1,a_2) ab[ab_subscr(a_1,a_2)]
#define pt_subscr(a_1,a_2) (a_2)*pt_dim1 + a_1
#define pt_ref(a_1,a_2) pt[pt_subscr(a_1,a_2)]


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
    --rwork;

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
	xerbla_("CGBBRD", &i__1);
	return 0;
    }

/*     Initialize Q and P' to the unit matrix, if needed */

    if (wantq) {
	claset_("Full", m, m, &c_b1, &c_b2, &q[q_offset], ldq);
    }
    if (wantpt) {
	claset_("Full", n, n, &c_b1, &c_b2, &pt[pt_offset], ldpt);
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

          The complex sines of the plane rotations are stored in WORK,   
          and the real cosines in RWORK.   

   Computing MIN */
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
		    clargv_(&nr, &ab_ref(klu1, j1 - klm - 1), &inca, &work[j1]
			    , &kb1, &rwork[j1], &kb1);
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
			clartv_(&nrt, &ab_ref(klu1 - l, j1 - klm + l - 1), &
				inca, &ab_ref(klu1 - l + 1, j1 - klm + l - 1),
				 &inca, &rwork[j1], &work[j1], &kb1);
		    }
/* L10: */
		}

		if (ml > ml0) {
		    if (ml <= *m - i__ + 1) {

/*                    generate plane rotation to annihilate a(i+ml-1,i)   
                      within the band, and apply rotation from the left */

			clartg_(&ab_ref(*ku + ml - 1, i__), &ab_ref(*ku + ml, 
				i__), &rwork[i__ + ml - 1], &work[i__ + ml - 
				1], &ra);
			i__3 = ab_subscr(*ku + ml - 1, i__);
			ab[i__3].r = ra.r, ab[i__3].i = ra.i;
			if (i__ < *n) {
/* Computing MIN */
			    i__4 = *ku + ml - 2, i__5 = *n - i__;
			    i__3 = min(i__4,i__5);
			    i__6 = *ldab - 1;
			    i__7 = *ldab - 1;
			    crot_(&i__3, &ab_ref(*ku + ml - 2, i__ + 1), &
				    i__6, &ab_ref(*ku + ml - 1, i__ + 1), &
				    i__7, &rwork[i__ + ml - 1], &work[i__ + 
				    ml - 1]);
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
			r_cnjg(&q__1, &work[j]);
			crot_(m, &q_ref(1, j - 1), &c__1, &q_ref(1, j), &c__1,
				 &rwork[j], &q__1);
/* L20: */
		    }
		}

		if (wantc) {

/*                 apply plane rotations to C */

		    i__4 = j2;
		    i__3 = kb1;
		    for (j = j1; i__3 < 0 ? j >= i__4 : j <= i__4; j += i__3) 
			    {
			crot_(ncc, &c___ref(j - 1, 1), ldc, &c___ref(j, 1), 
				ldc, &rwork[j], &work[j]);
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

		    i__5 = j + kun;
		    i__6 = j;
		    i__7 = ab_subscr(1, j + kun);
		    q__1.r = work[i__6].r * ab[i__7].r - work[i__6].i * ab[
			    i__7].i, q__1.i = work[i__6].r * ab[i__7].i + 
			    work[i__6].i * ab[i__7].r;
		    work[i__5].r = q__1.r, work[i__5].i = q__1.i;
		    i__5 = ab_subscr(1, j + kun);
		    i__6 = j;
		    i__7 = ab_subscr(1, j + kun);
		    q__1.r = rwork[i__6] * ab[i__7].r, q__1.i = rwork[i__6] * 
			    ab[i__7].i;
		    ab[i__5].r = q__1.r, ab[i__5].i = q__1.i;
/* L40: */
		}

/*              generate plane rotations to annihilate nonzero elements   
                which have been generated above the band */

		if (nr > 0) {
		    clargv_(&nr, &ab_ref(1, j1 + kun - 1), &inca, &work[j1 + 
			    kun], &kb1, &rwork[j1 + kun], &kb1);
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
			clartv_(&nrt, &ab_ref(l + 1, j1 + kun - 1), &inca, &
				ab_ref(l, j1 + kun), &inca, &rwork[j1 + kun], 
				&work[j1 + kun], &kb1);
		    }
/* L50: */
		}

		if (ml == ml0 && mu > mu0) {
		    if (mu <= *n - i__ + 1) {

/*                    generate plane rotation to annihilate a(i,i+mu-1)   
                      within the band, and apply rotation from the right */

			clartg_(&ab_ref(*ku - mu + 3, i__ + mu - 2), &ab_ref(*
				ku - mu + 2, i__ + mu - 1), &rwork[i__ + mu - 
				1], &work[i__ + mu - 1], &ra);
			i__4 = ab_subscr(*ku - mu + 3, i__ + mu - 2);
			ab[i__4].r = ra.r, ab[i__4].i = ra.i;
/* Computing MIN */
			i__3 = *kl + mu - 2, i__5 = *m - i__;
			i__4 = min(i__3,i__5);
			crot_(&i__4, &ab_ref(*ku - mu + 4, i__ + mu - 2), &
				c__1, &ab_ref(*ku - mu + 3, i__ + mu - 1), &
				c__1, &rwork[i__ + mu - 1], &work[i__ + mu - 
				1]);
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
			r_cnjg(&q__1, &work[j + kun]);
			crot_(n, &pt_ref(j + kun - 1, 1), ldpt, &pt_ref(j + 
				kun, 1), ldpt, &rwork[j + kun], &q__1);
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

		    i__5 = j + kb;
		    i__6 = j + kun;
		    i__7 = ab_subscr(klu1, j + kun);
		    q__1.r = work[i__6].r * ab[i__7].r - work[i__6].i * ab[
			    i__7].i, q__1.i = work[i__6].r * ab[i__7].i + 
			    work[i__6].i * ab[i__7].r;
		    work[i__5].r = q__1.r, work[i__5].i = q__1.i;
		    i__5 = ab_subscr(klu1, j + kun);
		    i__6 = j + kun;
		    i__7 = ab_subscr(klu1, j + kun);
		    q__1.r = rwork[i__6] * ab[i__7].r, q__1.i = rwork[i__6] * 
			    ab[i__7].i;
		    ab[i__5].r = q__1.r, ab[i__5].i = q__1.i;
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

/*        A has been reduced to complex lower bidiagonal form   

          Transform lower bidiagonal form to upper bidiagonal by applying   
          plane rotations from the left, overwriting superdiagonal   
          elements on subdiagonal elements   

   Computing MIN */
	i__2 = *m - 1;
	i__1 = min(i__2,*n);
	for (i__ = 1; i__ <= i__1; ++i__) {
	    clartg_(&ab_ref(1, i__), &ab_ref(2, i__), &rc, &rs, &ra);
	    i__2 = ab_subscr(1, i__);
	    ab[i__2].r = ra.r, ab[i__2].i = ra.i;
	    if (i__ < *n) {
		i__2 = ab_subscr(2, i__);
		i__4 = ab_subscr(1, i__ + 1);
		q__1.r = rs.r * ab[i__4].r - rs.i * ab[i__4].i, q__1.i = rs.r 
			* ab[i__4].i + rs.i * ab[i__4].r;
		ab[i__2].r = q__1.r, ab[i__2].i = q__1.i;
		i__2 = ab_subscr(1, i__ + 1);
		i__4 = ab_subscr(1, i__ + 1);
		q__1.r = rc * ab[i__4].r, q__1.i = rc * ab[i__4].i;
		ab[i__2].r = q__1.r, ab[i__2].i = q__1.i;
	    }
	    if (wantq) {
		r_cnjg(&q__1, &rs);
		crot_(m, &q_ref(1, i__), &c__1, &q_ref(1, i__ + 1), &c__1, &
			rc, &q__1);
	    }
	    if (wantc) {
		crot_(ncc, &c___ref(i__, 1), ldc, &c___ref(i__ + 1, 1), ldc, &
			rc, &rs);
	    }
/* L100: */
	}
    } else {

/*        A has been reduced to complex upper bidiagonal form or is   
          diagonal */

	if (*ku > 0 && *m < *n) {

/*           Annihilate a(m,m+1) by applying plane rotations from the   
             right */

	    i__1 = ab_subscr(*ku, *m + 1);
	    rb.r = ab[i__1].r, rb.i = ab[i__1].i;
	    for (i__ = *m; i__ >= 1; --i__) {
		clartg_(&ab_ref(*ku + 1, i__), &rb, &rc, &rs, &ra);
		i__1 = ab_subscr(*ku + 1, i__);
		ab[i__1].r = ra.r, ab[i__1].i = ra.i;
		if (i__ > 1) {
		    r_cnjg(&q__3, &rs);
		    q__2.r = -q__3.r, q__2.i = -q__3.i;
		    i__1 = ab_subscr(*ku, i__);
		    q__1.r = q__2.r * ab[i__1].r - q__2.i * ab[i__1].i, 
			    q__1.i = q__2.r * ab[i__1].i + q__2.i * ab[i__1]
			    .r;
		    rb.r = q__1.r, rb.i = q__1.i;
		    i__1 = ab_subscr(*ku, i__);
		    i__2 = ab_subscr(*ku, i__);
		    q__1.r = rc * ab[i__2].r, q__1.i = rc * ab[i__2].i;
		    ab[i__1].r = q__1.r, ab[i__1].i = q__1.i;
		}
		if (wantpt) {
		    r_cnjg(&q__1, &rs);
		    crot_(n, &pt_ref(i__, 1), ldpt, &pt_ref(*m + 1, 1), ldpt, 
			    &rc, &q__1);
		}
/* L110: */
	    }
	}
    }

/*     Make diagonal and superdiagonal elements real, storing them in D   
       and E */

    i__1 = ab_subscr(*ku + 1, 1);
    t.r = ab[i__1].r, t.i = ab[i__1].i;
    i__1 = minmn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	abst = c_abs(&t);
	d__[i__] = abst;
	if (abst != 0.f) {
	    q__1.r = t.r / abst, q__1.i = t.i / abst;
	    t.r = q__1.r, t.i = q__1.i;
	} else {
	    t.r = 1.f, t.i = 0.f;
	}
	if (wantq) {
	    cscal_(m, &t, &q_ref(1, i__), &c__1);
	}
	if (wantc) {
	    r_cnjg(&q__1, &t);
	    cscal_(ncc, &q__1, &c___ref(i__, 1), ldc);
	}
	if (i__ < minmn) {
	    if (*ku == 0 && *kl == 0) {
		e[i__] = 0.f;
		i__2 = ab_subscr(1, i__ + 1);
		t.r = ab[i__2].r, t.i = ab[i__2].i;
	    } else {
		if (*ku == 0) {
		    i__2 = ab_subscr(2, i__);
		    r_cnjg(&q__2, &t);
		    q__1.r = ab[i__2].r * q__2.r - ab[i__2].i * q__2.i, 
			    q__1.i = ab[i__2].r * q__2.i + ab[i__2].i * 
			    q__2.r;
		    t.r = q__1.r, t.i = q__1.i;
		} else {
		    i__2 = ab_subscr(*ku, i__ + 1);
		    r_cnjg(&q__2, &t);
		    q__1.r = ab[i__2].r * q__2.r - ab[i__2].i * q__2.i, 
			    q__1.i = ab[i__2].r * q__2.i + ab[i__2].i * 
			    q__2.r;
		    t.r = q__1.r, t.i = q__1.i;
		}
		abst = c_abs(&t);
		e[i__] = abst;
		if (abst != 0.f) {
		    q__1.r = t.r / abst, q__1.i = t.i / abst;
		    t.r = q__1.r, t.i = q__1.i;
		} else {
		    t.r = 1.f, t.i = 0.f;
		}
		if (wantpt) {
		    cscal_(n, &t, &pt_ref(i__ + 1, 1), ldpt);
		}
		i__2 = ab_subscr(*ku + 1, i__ + 1);
		r_cnjg(&q__2, &t);
		q__1.r = ab[i__2].r * q__2.r - ab[i__2].i * q__2.i, q__1.i = 
			ab[i__2].r * q__2.i + ab[i__2].i * q__2.r;
		t.r = q__1.r, t.i = q__1.i;
	    }
	}
/* L120: */
    }
    return 0;

/*     End of CGBBRD */

} /* cgbbrd_ */

#undef pt_ref
#undef pt_subscr
#undef ab_ref
#undef ab_subscr
#undef q_ref
#undef q_subscr
#undef c___ref
#undef c___subscr


