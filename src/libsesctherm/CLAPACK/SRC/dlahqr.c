#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dlahqr_(logical *wantt, logical *wantz, integer *n, 
	integer *ilo, integer *ihi, doublereal *h__, integer *ldh, doublereal 
	*wr, doublereal *wi, integer *iloz, integer *ihiz, doublereal *z__, 
	integer *ldz, integer *info)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    DLAHQR is an auxiliary routine called by DHSEQR to update the   
    eigenvalues and Schur decomposition already computed by DHSEQR, by   
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
            It is assumed that H is already upper quasi-triangular in   
            rows and columns IHI+1:N, and that H(ILO,ILO-1) = 0 (unless   
            ILO = 1). DLAHQR works primarily with the Hessenberg   
            submatrix in rows and columns ILO to IHI, but applies   
            transformations to all of H if WANTT is .TRUE..   
            1 <= ILO <= max(1,IHI); IHI <= N.   

    H       (input/output) DOUBLE PRECISION array, dimension (LDH,N)   
            On entry, the upper Hessenberg matrix H.   
            On exit, if WANTT is .TRUE., H is upper quasi-triangular in   
            rows and columns ILO:IHI, with any 2-by-2 diagonal blocks in   
            standard form. If WANTT is .FALSE., the contents of H are   
            unspecified on exit.   

    LDH     (input) INTEGER   
            The leading dimension of the array H. LDH >= max(1,N).   

    WR      (output) DOUBLE PRECISION array, dimension (N)   
    WI      (output) DOUBLE PRECISION array, dimension (N)   
            The real and imaginary parts, respectively, of the computed   
            eigenvalues ILO to IHI are stored in the corresponding   
            elements of WR and WI. If two eigenvalues are computed as a   
            complex conjugate pair, they are stored in consecutive   
            elements of WR and WI, say the i-th and (i+1)th, with   
            WI(i) > 0 and WI(i+1) < 0. If WANTT is .TRUE., the   
            eigenvalues are stored in the same order as on the diagonal   
            of the Schur form returned in H, with WR(i) = H(i,i), and, if   
            H(i:i+1,i:i+1) is a 2-by-2 diagonal block,   
            WI(i) = sqrt(H(i+1,i)*H(i,i+1)) and WI(i+1) = -WI(i).   

    ILOZ    (input) INTEGER   
    IHIZ    (input) INTEGER   
            Specify the rows of Z to which transformations must be   
            applied if WANTZ is .TRUE..   
            1 <= ILOZ <= ILO; IHI <= IHIZ <= N.   

    Z       (input/output) DOUBLE PRECISION array, dimension (LDZ,N)   
            If WANTZ is .TRUE., on entry Z must contain the current   
            matrix Z of transformations accumulated by DHSEQR, and on   
            exit Z has been updated; transformations are applied only to   
            the submatrix Z(ILOZ:IHIZ,ILO:IHI).   
            If WANTZ is .FALSE., Z is not referenced.   

    LDZ     (input) INTEGER   
            The leading dimension of the array Z. LDZ >= max(1,N).   

    INFO    (output) INTEGER   
            = 0: successful exit   
            > 0: DLAHQR failed to compute all the eigenvalues ILO to IHI   
                 in a total of 30*(IHI-ILO+1) iterations; if INFO = i,   
                 elements i+1:ihi of WR and WI contain those eigenvalues   
                 which have been successfully computed.   

    Further Details   
    ===============   

    2-96 Based on modifications by   
       David Day, Sandia National Laboratory, USA   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer h_dim1, h_offset, z_dim1, z_offset, i__1, i__2, i__3, i__4;
    doublereal d__1, d__2;
    /* Builtin functions */
    double sqrt(doublereal), d_sign(doublereal *, doublereal *);
    /* Local variables */
    static doublereal h43h34, disc, unfl, ovfl;
    extern /* Subroutine */ int drot_(integer *, doublereal *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *);
    static doublereal work[1];
    static integer i__, j, k, l, m;
    static doublereal s, v[3];
    extern /* Subroutine */ int dcopy_(integer *, doublereal *, integer *, 
	    doublereal *, integer *);
    static integer i1, i2;
    static doublereal t1, t2, t3, v1, v2, v3;
    extern /* Subroutine */ int dlanv2_(doublereal *, doublereal *, 
	    doublereal *, doublereal *, doublereal *, doublereal *, 
	    doublereal *, doublereal *, doublereal *, doublereal *), dlabad_(
	    doublereal *, doublereal *);
    static doublereal h00, h10, h11, h12, h21, h22, h33, h44;
    static integer nh;
    static doublereal cs;
    extern doublereal dlamch_(char *);
    extern /* Subroutine */ int dlarfg_(integer *, doublereal *, doublereal *,
	     integer *, doublereal *);
    static integer nr;
    static doublereal sn;
    static integer nz;
    extern doublereal dlanhs_(char *, integer *, doublereal *, integer *, 
	    doublereal *);
    static doublereal smlnum, ave, h33s, h44s;
    static integer itn, its;
    static doublereal ulp, sum, tst1;
#define h___ref(a_1,a_2) h__[(a_2)*h_dim1 + a_1]
#define z___ref(a_1,a_2) z__[(a_2)*z_dim1 + a_1]


    h_dim1 = *ldh;
    h_offset = 1 + h_dim1 * 1;
    h__ -= h_offset;
    --wr;
    --wi;
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
	wr[*ilo] = h___ref(*ilo, *ilo);
	wi[*ilo] = 0.;
	return 0;
    }

    nh = *ihi - *ilo + 1;
    nz = *ihiz - *iloz + 1;

/*     Set machine-dependent constants for the stopping criterion.   
       If norm(H) <= sqrt(OVFL), overflow should not occur. */

    unfl = dlamch_("Safe minimum");
    ovfl = 1. / unfl;
    dlabad_(&unfl, &ovfl);
    ulp = dlamch_("Precision");
    smlnum = unfl * (nh / ulp);

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
       IHI to ILO in steps of 1 or 2. Each iteration of the loop works   
       with the active submatrix in rows and columns L to I.   
       Eigenvalues I+1 to IHI have already converged. Either L = ILO or   
       H(L,L-1) is negligible so that the matrix splits. */

    i__ = *ihi;
L10:
    l = *ilo;
    if (i__ < *ilo) {
	goto L150;
    }

/*     Perform QR iterations on rows and columns ILO to I until a   
       submatrix of order 1 or 2 splits off at the bottom because a   
       subdiagonal element has become negligible. */

    i__1 = itn;
    for (its = 0; its <= i__1; ++its) {

/*        Look for a single small subdiagonal element. */

	i__2 = l + 1;
	for (k = i__; k >= i__2; --k) {
	    tst1 = (d__1 = h___ref(k - 1, k - 1), abs(d__1)) + (d__2 = 
		    h___ref(k, k), abs(d__2));
	    if (tst1 == 0.) {
		i__3 = i__ - l + 1;
		tst1 = dlanhs_("1", &i__3, &h___ref(l, l), ldh, work);
	    }
/* Computing MAX */
	    d__2 = ulp * tst1;
	    if ((d__1 = h___ref(k, k - 1), abs(d__1)) <= max(d__2,smlnum)) {
		goto L30;
	    }
/* L20: */
	}
L30:
	l = k;
	if (l > *ilo) {

/*           H(L,L-1) is negligible */

	    h___ref(l, l - 1) = 0.;
	}

/*        Exit from loop if a submatrix of order 1 or 2 has split off. */

	if (l >= i__ - 1) {
	    goto L140;
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

	    s = (d__1 = h___ref(i__, i__ - 1), abs(d__1)) + (d__2 = h___ref(
		    i__ - 1, i__ - 2), abs(d__2));
	    h44 = s * .75 + h___ref(i__, i__);
	    h33 = h44;
	    h43h34 = s * -.4375 * s;
	} else {

/*           Prepare to use Francis' double shift   
             (i.e. 2nd degree generalized Rayleigh quotient) */

	    h44 = h___ref(i__, i__);
	    h33 = h___ref(i__ - 1, i__ - 1);
	    h43h34 = h___ref(i__, i__ - 1) * h___ref(i__ - 1, i__);
	    s = h___ref(i__ - 1, i__ - 2) * h___ref(i__ - 1, i__ - 2);
	    disc = (h33 - h44) * .5;
	    disc = disc * disc + h43h34;
	    if (disc > 0.) {

/*              Real roots: use Wilkinson's shift twice */

		disc = sqrt(disc);
		ave = (h33 + h44) * .5;
		if (abs(h33) - abs(h44) > 0.) {
		    h33 = h33 * h44 - h43h34;
		    h44 = h33 / (d_sign(&disc, &ave) + ave);
		} else {
		    h44 = d_sign(&disc, &ave) + ave;
		}
		h33 = h44;
		h43h34 = 0.;
	    }
	}

/*        Look for two consecutive small subdiagonal elements. */

	i__2 = l;
	for (m = i__ - 2; m >= i__2; --m) {
/*           Determine the effect of starting the double-shift QR   
             iteration at row M, and see if this would make H(M,M-1)   
             negligible. */

	    h11 = h___ref(m, m);
	    h22 = h___ref(m + 1, m + 1);
	    h21 = h___ref(m + 1, m);
	    h12 = h___ref(m, m + 1);
	    h44s = h44 - h11;
	    h33s = h33 - h11;
	    v1 = (h33s * h44s - h43h34) / h21 + h12;
	    v2 = h22 - h11 - h33s - h44s;
	    v3 = h___ref(m + 2, m + 1);
	    s = abs(v1) + abs(v2) + abs(v3);
	    v1 /= s;
	    v2 /= s;
	    v3 /= s;
	    v[0] = v1;
	    v[1] = v2;
	    v[2] = v3;
	    if (m == l) {
		goto L50;
	    }
	    h00 = h___ref(m - 1, m - 1);
	    h10 = h___ref(m, m - 1);
	    tst1 = abs(v1) * (abs(h00) + abs(h11) + abs(h22));
	    if (abs(h10) * (abs(v2) + abs(v3)) <= ulp * tst1) {
		goto L50;
	    }
/* L40: */
	}
L50:

/*        Double-shift QR step */

	i__2 = i__ - 1;
	for (k = m; k <= i__2; ++k) {

/*           The first iteration of this loop determines a reflection G   
             from the vector V and applies it from left and right to H,   
             thus creating a nonzero bulge below the subdiagonal.   

             Each subsequent iteration determines a reflection G to   
             restore the Hessenberg form in the (K-1)th column, and thus   
             chases the bulge one step toward the bottom of the active   
             submatrix. NR is the order of G.   

   Computing MIN */
	    i__3 = 3, i__4 = i__ - k + 1;
	    nr = min(i__3,i__4);
	    if (k > m) {
		dcopy_(&nr, &h___ref(k, k - 1), &c__1, v, &c__1);
	    }
	    dlarfg_(&nr, v, &v[1], &c__1, &t1);
	    if (k > m) {
		h___ref(k, k - 1) = v[0];
		h___ref(k + 1, k - 1) = 0.;
		if (k < i__ - 1) {
		    h___ref(k + 2, k - 1) = 0.;
		}
	    } else if (m > l) {
		h___ref(k, k - 1) = -h___ref(k, k - 1);
	    }
	    v2 = v[1];
	    t2 = t1 * v2;
	    if (nr == 3) {
		v3 = v[2];
		t3 = t1 * v3;

/*              Apply G from the left to transform the rows of the matrix   
                in columns K to I2. */

		i__3 = i2;
		for (j = k; j <= i__3; ++j) {
		    sum = h___ref(k, j) + v2 * h___ref(k + 1, j) + v3 * 
			    h___ref(k + 2, j);
		    h___ref(k, j) = h___ref(k, j) - sum * t1;
		    h___ref(k + 1, j) = h___ref(k + 1, j) - sum * t2;
		    h___ref(k + 2, j) = h___ref(k + 2, j) - sum * t3;
/* L60: */
		}

/*              Apply G from the right to transform the columns of the   
                matrix in rows I1 to min(K+3,I).   

   Computing MIN */
		i__4 = k + 3;
		i__3 = min(i__4,i__);
		for (j = i1; j <= i__3; ++j) {
		    sum = h___ref(j, k) + v2 * h___ref(j, k + 1) + v3 * 
			    h___ref(j, k + 2);
		    h___ref(j, k) = h___ref(j, k) - sum * t1;
		    h___ref(j, k + 1) = h___ref(j, k + 1) - sum * t2;
		    h___ref(j, k + 2) = h___ref(j, k + 2) - sum * t3;
/* L70: */
		}

		if (*wantz) {

/*                 Accumulate transformations in the matrix Z */

		    i__3 = *ihiz;
		    for (j = *iloz; j <= i__3; ++j) {
			sum = z___ref(j, k) + v2 * z___ref(j, k + 1) + v3 * 
				z___ref(j, k + 2);
			z___ref(j, k) = z___ref(j, k) - sum * t1;
			z___ref(j, k + 1) = z___ref(j, k + 1) - sum * t2;
			z___ref(j, k + 2) = z___ref(j, k + 2) - sum * t3;
/* L80: */
		    }
		}
	    } else if (nr == 2) {

/*              Apply G from the left to transform the rows of the matrix   
                in columns K to I2. */

		i__3 = i2;
		for (j = k; j <= i__3; ++j) {
		    sum = h___ref(k, j) + v2 * h___ref(k + 1, j);
		    h___ref(k, j) = h___ref(k, j) - sum * t1;
		    h___ref(k + 1, j) = h___ref(k + 1, j) - sum * t2;
/* L90: */
		}

/*              Apply G from the right to transform the columns of the   
                matrix in rows I1 to min(K+3,I). */

		i__3 = i__;
		for (j = i1; j <= i__3; ++j) {
		    sum = h___ref(j, k) + v2 * h___ref(j, k + 1);
		    h___ref(j, k) = h___ref(j, k) - sum * t1;
		    h___ref(j, k + 1) = h___ref(j, k + 1) - sum * t2;
/* L100: */
		}

		if (*wantz) {

/*                 Accumulate transformations in the matrix Z */

		    i__3 = *ihiz;
		    for (j = *iloz; j <= i__3; ++j) {
			sum = z___ref(j, k) + v2 * z___ref(j, k + 1);
			z___ref(j, k) = z___ref(j, k) - sum * t1;
			z___ref(j, k + 1) = z___ref(j, k + 1) - sum * t2;
/* L110: */
		    }
		}
	    }
/* L120: */
	}

/* L130: */
    }

/*     Failure to converge in remaining number of iterations */

    *info = i__;
    return 0;

L140:

    if (l == i__) {

/*        H(I,I-1) is negligible: one eigenvalue has converged. */

	wr[i__] = h___ref(i__, i__);
	wi[i__] = 0.;
    } else if (l == i__ - 1) {

/*        H(I-1,I-2) is negligible: a pair of eigenvalues have converged.   

          Transform the 2-by-2 submatrix to standard Schur form,   
          and compute and store the eigenvalues. */

	dlanv2_(&h___ref(i__ - 1, i__ - 1), &h___ref(i__ - 1, i__), &h___ref(
		i__, i__ - 1), &h___ref(i__, i__), &wr[i__ - 1], &wi[i__ - 1],
		 &wr[i__], &wi[i__], &cs, &sn);

	if (*wantt) {

/*           Apply the transformation to the rest of H. */

	    if (i2 > i__) {
		i__1 = i2 - i__;
		drot_(&i__1, &h___ref(i__ - 1, i__ + 1), ldh, &h___ref(i__, 
			i__ + 1), ldh, &cs, &sn);
	    }
	    i__1 = i__ - i1 - 1;
	    drot_(&i__1, &h___ref(i1, i__ - 1), &c__1, &h___ref(i1, i__), &
		    c__1, &cs, &sn);
	}
	if (*wantz) {

/*           Apply the transformation to Z. */

	    drot_(&nz, &z___ref(*iloz, i__ - 1), &c__1, &z___ref(*iloz, i__), 
		    &c__1, &cs, &sn);
	}
    }

/*     Decrement number of remaining iterations, and return to start of   
       the main loop with new value of I. */

    itn -= its;
    i__ = l - 1;
    goto L10;

L150:
    return 0;

/*     End of DLAHQR */

} /* dlahqr_ */

#undef z___ref
#undef h___ref


