#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int clahqr_(logical *wantt, logical *wantz, integer *n, 
	integer *ilo, integer *ihi, complex *h__, integer *ldh, complex *w, 
	integer *iloz, integer *ihiz, complex *z__, integer *ldz, integer *
	info)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    CLAHQR is an auxiliary routine called by CHSEQR to update the   
    eigenvalues and Schur decomposition already computed by CHSEQR, by   
    dealing with the Hessenberg submatrix in rows and columns ILO to IHI.   

    Arguments   
    =========   

    WANTT   (input) LOGICAL   
            = .TRUE. : the full Schur form T is required;   
            = .FALSE.: only eigenvalues are required.   

    WANTZ   (input) LOGICAL   
            = .TRUE. : the matrix of Schur vectors Z is required;   
            = .FALSE.: Schur vectors are not required.   

    N       (input) INTEGER   
            The order of the matrix H.  N >= 0.   

    ILO     (input) INTEGER   
    IHI     (input) INTEGER   
            It is assumed that H is already upper triangular in rows and   
            columns IHI+1:N, and that H(ILO,ILO-1) = 0 (unless ILO = 1).   
            CLAHQR works primarily with the Hessenberg submatrix in rows   
            and columns ILO to IHI, but applies transformations to all of   
            H if WANTT is .TRUE..   
            1 <= ILO <= max(1,IHI); IHI <= N.   

    H       (input/output) COMPLEX array, dimension (LDH,N)   
            On entry, the upper Hessenberg matrix H.   
            On exit, if WANTT is .TRUE., H is upper triangular in rows   
            and columns ILO:IHI, with any 2-by-2 diagonal blocks in   
            standard form. If WANTT is .FALSE., the contents of H are   
            unspecified on exit.   

    LDH     (input) INTEGER   
            The leading dimension of the array H. LDH >= max(1,N).   

    W       (output) COMPLEX array, dimension (N)   
            The computed eigenvalues ILO to IHI are stored in the   
            corresponding elements of W. If WANTT is .TRUE., the   
            eigenvalues are stored in the same order as on the diagonal   
            of the Schur form returned in H, with W(i) = H(i,i).   

    ILOZ    (input) INTEGER   
    IHIZ    (input) INTEGER   
            Specify the rows of Z to which transformations must be   
            applied if WANTZ is .TRUE..   
            1 <= ILOZ <= ILO; IHI <= IHIZ <= N.   

    Z       (input/output) COMPLEX array, dimension (LDZ,N)   
            If WANTZ is .TRUE., on entry Z must contain the current   
            matrix Z of transformations accumulated by CHSEQR, and on   
            exit Z has been updated; transformations are applied only to   
            the submatrix Z(ILOZ:IHIZ,ILO:IHI).   
            If WANTZ is .FALSE., Z is not referenced.   

    LDZ     (input) INTEGER   
            The leading dimension of the array Z. LDZ >= max(1,N).   

    INFO    (output) INTEGER   
            = 0: successful exit   
            > 0: if INFO = i, CLAHQR failed to compute all the   
                 eigenvalues ILO to IHI in a total of 30*(IHI-ILO+1)   
                 iterations; elements i+1:ihi of W contain those   
                 eigenvalues which have been successfully computed.   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__2 = 2;
    static integer c__1 = 1;
    
    /* System generated locals */
    integer h_dim1, h_offset, z_dim1, z_offset, i__1, i__2, i__3, i__4, i__5;
    real r__1, r__2, r__3, r__4, r__5, r__6;
    complex q__1, q__2, q__3, q__4;
    /* Builtin functions */
    double r_imag(complex *);
    void c_sqrt(complex *, complex *), r_cnjg(complex *, complex *);
    double c_abs(complex *);
    /* Local variables */
    static complex temp;
    static integer i__, j, k, l, m;
    static real s;
    static complex t, u, v[2], x, y;
    extern /* Subroutine */ int cscal_(integer *, complex *, complex *, 
	    integer *), ccopy_(integer *, complex *, integer *, complex *, 
	    integer *);
    static real rtemp;
    static integer i1, i2;
    static real rwork[1];
    static complex t1;
    static real t2;
    static complex v2;
    static real h10;
    static complex h11;
    static real h21;
    static complex h22;
    static integer nh;
    extern /* Subroutine */ int clarfg_(integer *, complex *, complex *, 
	    integer *, complex *);
    extern /* Complex */ VOID cladiv_(complex *, complex *, complex *);
    extern doublereal slamch_(char *);
    static integer nz;
    extern doublereal clanhs_(char *, integer *, complex *, integer *, real *);
    static real smlnum;
    static complex h11s;
    static integer itn, its;
    static real ulp;
    static complex sum;
    static real tst1;
#define h___subscr(a_1,a_2) (a_2)*h_dim1 + a_1
#define h___ref(a_1,a_2) h__[h___subscr(a_1,a_2)]
#define z___subscr(a_1,a_2) (a_2)*z_dim1 + a_1
#define z___ref(a_1,a_2) z__[z___subscr(a_1,a_2)]


    h_dim1 = *ldh;
    h_offset = 1 + h_dim1 * 1;
    h__ -= h_offset;
    --w;
    z_dim1 = *ldz;
    z_offset = 1 + z_dim1 * 1;
    z__ -= z_offset;

    /* Function Body */
    *info = 0;

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }
    if (*ilo == *ihi) {
	i__1 = *ilo;
	i__2 = h___subscr(*ilo, *ilo);
	w[i__1].r = h__[i__2].r, w[i__1].i = h__[i__2].i;
	return 0;
    }

    nh = *ihi - *ilo + 1;
    nz = *ihiz - *iloz + 1;

/*     Set machine-dependent constants for the stopping criterion.   
       If norm(H) <= sqrt(OVFL), overflow should not occur. */

    ulp = slamch_("Precision");
    smlnum = slamch_("Safe minimum") / ulp;

/*     I1 and I2 are the indices of the first row and last column of H   
       to which transformations must be applied. If eigenvalues only are   
       being computed, I1 and I2 are set inside the main loop. */

    if (*wantt) {
	i1 = 1;
	i2 = *n;
    }

/*     ITN is the total number of QR iterations allowed. */

    itn = nh * 30;

/*     The main loop begins here. I is the loop index and decreases from   
       IHI to ILO in steps of 1. Each iteration of the loop works   
       with the active submatrix in rows and columns L to I.   
       Eigenvalues I+1 to IHI have already converged. Either L = ILO, or   
       H(L,L-1) is negligible so that the matrix splits. */

    i__ = *ihi;
L10:
    if (i__ < *ilo) {
	goto L130;
    }

/*     Perform QR iterations on rows and columns ILO to I until a   
       submatrix of order 1 splits off at the bottom because a   
       subdiagonal element has become negligible. */

    l = *ilo;
    i__1 = itn;
    for (its = 0; its <= i__1; ++its) {

/*        Look for a single small subdiagonal element. */

	i__2 = l + 1;
	for (k = i__; k >= i__2; --k) {
	    i__3 = h___subscr(k - 1, k - 1);
	    i__4 = h___subscr(k, k);
	    tst1 = (r__1 = h__[i__3].r, dabs(r__1)) + (r__2 = r_imag(&h___ref(
		    k - 1, k - 1)), dabs(r__2)) + ((r__3 = h__[i__4].r, dabs(
		    r__3)) + (r__4 = r_imag(&h___ref(k, k)), dabs(r__4)));
	    if (tst1 == 0.f) {
		i__3 = i__ - l + 1;
		tst1 = clanhs_("1", &i__3, &h___ref(l, l), ldh, rwork);
	    }
	    i__3 = h___subscr(k, k - 1);
/* Computing MAX */
	    r__2 = ulp * tst1;
	    if ((r__1 = h__[i__3].r, dabs(r__1)) <= dmax(r__2,smlnum)) {
		goto L30;
	    }
/* L20: */
	}
L30:
	l = k;
	if (l > *ilo) {

/*           H(L,L-1) is negligible */

	    i__2 = h___subscr(l, l - 1);
	    h__[i__2].r = 0.f, h__[i__2].i = 0.f;
	}

/*        Exit from loop if a submatrix of order 1 has split off. */

	if (l >= i__) {
	    goto L120;
	}

/*        Now the active submatrix is in rows and columns L to I. If   
          eigenvalues only are being computed, only the active submatrix   
          need be transformed. */

	if (! (*wantt)) {
	    i1 = l;
	    i2 = i__;
	}

	if (its == 10 || its == 20) {

/*           Exceptional shift. */

	    i__2 = h___subscr(i__, i__ - 1);
	    s = (r__1 = h__[i__2].r, dabs(r__1)) * .75f;
	    i__2 = h___subscr(i__, i__);
	    q__1.r = s + h__[i__2].r, q__1.i = h__[i__2].i;
	    t.r = q__1.r, t.i = q__1.i;
	} else {

/*           Wilkinson's shift. */

	    i__2 = h___subscr(i__, i__);
	    t.r = h__[i__2].r, t.i = h__[i__2].i;
	    i__2 = h___subscr(i__ - 1, i__);
	    i__3 = h___subscr(i__, i__ - 1);
	    r__1 = h__[i__3].r;
	    q__1.r = r__1 * h__[i__2].r, q__1.i = r__1 * h__[i__2].i;
	    u.r = q__1.r, u.i = q__1.i;
	    if (u.r != 0.f || u.i != 0.f) {
		i__2 = h___subscr(i__ - 1, i__ - 1);
		q__2.r = h__[i__2].r - t.r, q__2.i = h__[i__2].i - t.i;
		q__1.r = q__2.r * .5f, q__1.i = q__2.i * .5f;
		x.r = q__1.r, x.i = q__1.i;
		q__3.r = x.r * x.r - x.i * x.i, q__3.i = x.r * x.i + x.i * 
			x.r;
		q__2.r = q__3.r + u.r, q__2.i = q__3.i + u.i;
		c_sqrt(&q__1, &q__2);
		y.r = q__1.r, y.i = q__1.i;
		if (x.r * y.r + r_imag(&x) * r_imag(&y) < 0.f) {
		    q__1.r = -y.r, q__1.i = -y.i;
		    y.r = q__1.r, y.i = q__1.i;
		}
		q__3.r = x.r + y.r, q__3.i = x.i + y.i;
		cladiv_(&q__2, &u, &q__3);
		q__1.r = t.r - q__2.r, q__1.i = t.i - q__2.i;
		t.r = q__1.r, t.i = q__1.i;
	    }
	}

/*        Look for two consecutive small subdiagonal elements. */

	i__2 = l + 1;
	for (m = i__ - 1; m >= i__2; --m) {

/*           Determine the effect of starting the single-shift QR   
             iteration at row M, and see if this would make H(M,M-1)   
             negligible. */

	    i__3 = h___subscr(m, m);
	    h11.r = h__[i__3].r, h11.i = h__[i__3].i;
	    i__3 = h___subscr(m + 1, m + 1);
	    h22.r = h__[i__3].r, h22.i = h__[i__3].i;
	    q__1.r = h11.r - t.r, q__1.i = h11.i - t.i;
	    h11s.r = q__1.r, h11s.i = q__1.i;
	    i__3 = h___subscr(m + 1, m);
	    h21 = h__[i__3].r;
	    s = (r__1 = h11s.r, dabs(r__1)) + (r__2 = r_imag(&h11s), dabs(
		    r__2)) + dabs(h21);
	    q__1.r = h11s.r / s, q__1.i = h11s.i / s;
	    h11s.r = q__1.r, h11s.i = q__1.i;
	    h21 /= s;
	    v[0].r = h11s.r, v[0].i = h11s.i;
	    v[1].r = h21, v[1].i = 0.f;
	    i__3 = h___subscr(m, m - 1);
	    h10 = h__[i__3].r;
	    tst1 = ((r__1 = h11s.r, dabs(r__1)) + (r__2 = r_imag(&h11s), dabs(
		    r__2))) * ((r__3 = h11.r, dabs(r__3)) + (r__4 = r_imag(&
		    h11), dabs(r__4)) + ((r__5 = h22.r, dabs(r__5)) + (r__6 = 
		    r_imag(&h22), dabs(r__6))));
	    if ((r__1 = h10 * h21, dabs(r__1)) <= ulp * tst1) {
		goto L50;
	    }
/* L40: */
	}
	i__2 = h___subscr(l, l);
	h11.r = h__[i__2].r, h11.i = h__[i__2].i;
	i__2 = h___subscr(l + 1, l + 1);
	h22.r = h__[i__2].r, h22.i = h__[i__2].i;
	q__1.r = h11.r - t.r, q__1.i = h11.i - t.i;
	h11s.r = q__1.r, h11s.i = q__1.i;
	i__2 = h___subscr(l + 1, l);
	h21 = h__[i__2].r;
	s = (r__1 = h11s.r, dabs(r__1)) + (r__2 = r_imag(&h11s), dabs(r__2)) 
		+ dabs(h21);
	q__1.r = h11s.r / s, q__1.i = h11s.i / s;
	h11s.r = q__1.r, h11s.i = q__1.i;
	h21 /= s;
	v[0].r = h11s.r, v[0].i = h11s.i;
	v[1].r = h21, v[1].i = 0.f;
L50:

/*        Single-shift QR step */

	i__2 = i__ - 1;
	for (k = m; k <= i__2; ++k) {

/*           The first iteration of this loop determines a reflection G   
             from the vector V and applies it from left and right to H,   
             thus creating a nonzero bulge below the subdiagonal.   

             Each subsequent iteration determines a reflection G to   
             restore the Hessenberg form in the (K-1)th column, and thus   
             chases the bulge one step toward the bottom of the active   
             submatrix.   

             V(2) is always real before the call to CLARFG, and hence   
             after the call T2 ( = T1*V(2) ) is also real. */

	    if (k > m) {
		ccopy_(&c__2, &h___ref(k, k - 1), &c__1, v, &c__1);
	    }
	    clarfg_(&c__2, v, &v[1], &c__1, &t1);
	    if (k > m) {
		i__3 = h___subscr(k, k - 1);
		h__[i__3].r = v[0].r, h__[i__3].i = v[0].i;
		i__3 = h___subscr(k + 1, k - 1);
		h__[i__3].r = 0.f, h__[i__3].i = 0.f;
	    }
	    v2.r = v[1].r, v2.i = v[1].i;
	    q__1.r = t1.r * v2.r - t1.i * v2.i, q__1.i = t1.r * v2.i + t1.i * 
		    v2.r;
	    t2 = q__1.r;

/*           Apply G from the left to transform the rows of the matrix   
             in columns K to I2. */

	    i__3 = i2;
	    for (j = k; j <= i__3; ++j) {
		r_cnjg(&q__3, &t1);
		i__4 = h___subscr(k, j);
		q__2.r = q__3.r * h__[i__4].r - q__3.i * h__[i__4].i, q__2.i =
			 q__3.r * h__[i__4].i + q__3.i * h__[i__4].r;
		i__5 = h___subscr(k + 1, j);
		q__4.r = t2 * h__[i__5].r, q__4.i = t2 * h__[i__5].i;
		q__1.r = q__2.r + q__4.r, q__1.i = q__2.i + q__4.i;
		sum.r = q__1.r, sum.i = q__1.i;
		i__4 = h___subscr(k, j);
		i__5 = h___subscr(k, j);
		q__1.r = h__[i__5].r - sum.r, q__1.i = h__[i__5].i - sum.i;
		h__[i__4].r = q__1.r, h__[i__4].i = q__1.i;
		i__4 = h___subscr(k + 1, j);
		i__5 = h___subscr(k + 1, j);
		q__2.r = sum.r * v2.r - sum.i * v2.i, q__2.i = sum.r * v2.i + 
			sum.i * v2.r;
		q__1.r = h__[i__5].r - q__2.r, q__1.i = h__[i__5].i - q__2.i;
		h__[i__4].r = q__1.r, h__[i__4].i = q__1.i;
/* L60: */
	    }

/*           Apply G from the right to transform the columns of the   
             matrix in rows I1 to min(K+2,I).   

   Computing MIN */
	    i__4 = k + 2;
	    i__3 = min(i__4,i__);
	    for (j = i1; j <= i__3; ++j) {
		i__4 = h___subscr(j, k);
		q__2.r = t1.r * h__[i__4].r - t1.i * h__[i__4].i, q__2.i = 
			t1.r * h__[i__4].i + t1.i * h__[i__4].r;
		i__5 = h___subscr(j, k + 1);
		q__3.r = t2 * h__[i__5].r, q__3.i = t2 * h__[i__5].i;
		q__1.r = q__2.r + q__3.r, q__1.i = q__2.i + q__3.i;
		sum.r = q__1.r, sum.i = q__1.i;
		i__4 = h___subscr(j, k);
		i__5 = h___subscr(j, k);
		q__1.r = h__[i__5].r - sum.r, q__1.i = h__[i__5].i - sum.i;
		h__[i__4].r = q__1.r, h__[i__4].i = q__1.i;
		i__4 = h___subscr(j, k + 1);
		i__5 = h___subscr(j, k + 1);
		r_cnjg(&q__3, &v2);
		q__2.r = sum.r * q__3.r - sum.i * q__3.i, q__2.i = sum.r * 
			q__3.i + sum.i * q__3.r;
		q__1.r = h__[i__5].r - q__2.r, q__1.i = h__[i__5].i - q__2.i;
		h__[i__4].r = q__1.r, h__[i__4].i = q__1.i;
/* L70: */
	    }

	    if (*wantz) {

/*              Accumulate transformations in the matrix Z */

		i__3 = *ihiz;
		for (j = *iloz; j <= i__3; ++j) {
		    i__4 = z___subscr(j, k);
		    q__2.r = t1.r * z__[i__4].r - t1.i * z__[i__4].i, q__2.i =
			     t1.r * z__[i__4].i + t1.i * z__[i__4].r;
		    i__5 = z___subscr(j, k + 1);
		    q__3.r = t2 * z__[i__5].r, q__3.i = t2 * z__[i__5].i;
		    q__1.r = q__2.r + q__3.r, q__1.i = q__2.i + q__3.i;
		    sum.r = q__1.r, sum.i = q__1.i;
		    i__4 = z___subscr(j, k);
		    i__5 = z___subscr(j, k);
		    q__1.r = z__[i__5].r - sum.r, q__1.i = z__[i__5].i - 
			    sum.i;
		    z__[i__4].r = q__1.r, z__[i__4].i = q__1.i;
		    i__4 = z___subscr(j, k + 1);
		    i__5 = z___subscr(j, k + 1);
		    r_cnjg(&q__3, &v2);
		    q__2.r = sum.r * q__3.r - sum.i * q__3.i, q__2.i = sum.r *
			     q__3.i + sum.i * q__3.r;
		    q__1.r = z__[i__5].r - q__2.r, q__1.i = z__[i__5].i - 
			    q__2.i;
		    z__[i__4].r = q__1.r, z__[i__4].i = q__1.i;
/* L80: */
		}
	    }

	    if (k == m && m > l) {

/*              If the QR step was started at row M > L because two   
                consecutive small subdiagonals were found, then extra   
                scaling must be performed to ensure that H(M,M-1) remains   
                real. */

		q__1.r = 1.f - t1.r, q__1.i = 0.f - t1.i;
		temp.r = q__1.r, temp.i = q__1.i;
		r__1 = c_abs(&temp);
		q__1.r = temp.r / r__1, q__1.i = temp.i / r__1;
		temp.r = q__1.r, temp.i = q__1.i;
		i__3 = h___subscr(m + 1, m);
		i__4 = h___subscr(m + 1, m);
		r_cnjg(&q__2, &temp);
		q__1.r = h__[i__4].r * q__2.r - h__[i__4].i * q__2.i, q__1.i =
			 h__[i__4].r * q__2.i + h__[i__4].i * q__2.r;
		h__[i__3].r = q__1.r, h__[i__3].i = q__1.i;
		if (m + 2 <= i__) {
		    i__3 = h___subscr(m + 2, m + 1);
		    i__4 = h___subscr(m + 2, m + 1);
		    q__1.r = h__[i__4].r * temp.r - h__[i__4].i * temp.i, 
			    q__1.i = h__[i__4].r * temp.i + h__[i__4].i * 
			    temp.r;
		    h__[i__3].r = q__1.r, h__[i__3].i = q__1.i;
		}
		i__3 = i__;
		for (j = m; j <= i__3; ++j) {
		    if (j != m + 1) {
			if (i2 > j) {
			    i__4 = i2 - j;
			    cscal_(&i__4, &temp, &h___ref(j, j + 1), ldh);
			}
			i__4 = j - i1;
			r_cnjg(&q__1, &temp);
			cscal_(&i__4, &q__1, &h___ref(i1, j), &c__1);
			if (*wantz) {
			    r_cnjg(&q__1, &temp);
			    cscal_(&nz, &q__1, &z___ref(*iloz, j), &c__1);
			}
		    }
/* L90: */
		}
	    }
/* L100: */
	}

/*        Ensure that H(I,I-1) is real. */

	i__2 = h___subscr(i__, i__ - 1);
	temp.r = h__[i__2].r, temp.i = h__[i__2].i;
	if (r_imag(&temp) != 0.f) {
	    rtemp = c_abs(&temp);
	    i__2 = h___subscr(i__, i__ - 1);
	    h__[i__2].r = rtemp, h__[i__2].i = 0.f;
	    q__1.r = temp.r / rtemp, q__1.i = temp.i / rtemp;
	    temp.r = q__1.r, temp.i = q__1.i;
	    if (i2 > i__) {
		i__2 = i2 - i__;
		r_cnjg(&q__1, &temp);
		cscal_(&i__2, &q__1, &h___ref(i__, i__ + 1), ldh);
	    }
	    i__2 = i__ - i1;
	    cscal_(&i__2, &temp, &h___ref(i1, i__), &c__1);
	    if (*wantz) {
		cscal_(&nz, &temp, &z___ref(*iloz, i__), &c__1);
	    }
	}

/* L110: */
    }

/*     Failure to converge in remaining number of iterations */

    *info = i__;
    return 0;

L120:

/*     H(I,I-1) is negligible: one eigenvalue has converged. */

    i__1 = i__;
    i__2 = h___subscr(i__, i__);
    w[i__1].r = h__[i__2].r, w[i__1].i = h__[i__2].i;

/*     Decrement number of remaining iterations, and return to start of   
       the main loop with new value of I. */

    itn -= its;
    i__ = l - 1;
    goto L10;

L130:
    return 0;

/*     End of CLAHQR */

} /* clahqr_ */

#undef z___ref
#undef z___subscr
#undef h___ref
#undef h___subscr


