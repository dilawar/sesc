#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zlaein_(logical *rightv, logical *noinit, integer *n, 
	doublecomplex *h__, integer *ldh, doublecomplex *w, doublecomplex *v, 
	doublecomplex *b, integer *ldb, doublereal *rwork, doublereal *eps3, 
	doublereal *smlnum, integer *info)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZLAEIN uses inverse iteration to find a right or left eigenvector   
    corresponding to the eigenvalue W of a complex upper Hessenberg   
    matrix H.   

    Arguments   
    =========   

    RIGHTV   (input) LOGICAL   
            = .TRUE. : compute right eigenvector;   
            = .FALSE.: compute left eigenvector.   

    NOINIT   (input) LOGICAL   
            = .TRUE. : no initial vector supplied in V   
            = .FALSE.: initial vector supplied in V.   

    N       (input) INTEGER   
            The order of the matrix H.  N >= 0.   

    H       (input) COMPLEX*16 array, dimension (LDH,N)   
            The upper Hessenberg matrix H.   

    LDH     (input) INTEGER   
            The leading dimension of the array H.  LDH >= max(1,N).   

    W       (input) COMPLEX*16   
            The eigenvalue of H whose corresponding right or left   
            eigenvector is to be computed.   

    V       (input/output) COMPLEX*16 array, dimension (N)   
            On entry, if NOINIT = .FALSE., V must contain a starting   
            vector for inverse iteration; otherwise V need not be set.   
            On exit, V contains the computed eigenvector, normalized so   
            that the component of largest magnitude has magnitude 1; here   
            the magnitude of a complex number (x,y) is taken to be   
            |x| + |y|.   

    B       (workspace) COMPLEX*16 array, dimension (LDB,N)   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    RWORK   (workspace) DOUBLE PRECISION array, dimension (N)   

    EPS3    (input) DOUBLE PRECISION   
            A small machine-dependent value which is used to perturb   
            close eigenvalues, and to replace zero pivots.   

    SMLNUM  (input) DOUBLE PRECISION   
            A machine-dependent value close to the underflow threshold.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            = 1:  inverse iteration did not converge; V is set to the   
                  last iterate.   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer b_dim1, b_offset, h_dim1, h_offset, i__1, i__2, i__3, i__4, i__5;
    doublereal d__1, d__2, d__3, d__4;
    doublecomplex z__1, z__2;
    /* Builtin functions */
    double sqrt(doublereal), d_imag(doublecomplex *);
    /* Local variables */
    static integer ierr;
    static doublecomplex temp;
    static integer i__, j;
    static doublereal scale;
    static doublecomplex x;
    static char trans[1];
    static doublereal rtemp, rootn, vnorm;
    extern doublereal dznrm2_(integer *, doublecomplex *, integer *);
    static doublecomplex ei, ej;
    extern /* Subroutine */ int zdscal_(integer *, doublereal *, 
	    doublecomplex *, integer *);
    extern integer izamax_(integer *, doublecomplex *, integer *);
    extern /* Double Complex */ VOID zladiv_(doublecomplex *, doublecomplex *,
	     doublecomplex *);
    static char normin[1];
    extern doublereal dzasum_(integer *, doublecomplex *, integer *);
    static doublereal nrmsml;
    extern /* Subroutine */ int zlatrs_(char *, char *, char *, char *, 
	    integer *, doublecomplex *, integer *, doublecomplex *, 
	    doublereal *, doublereal *, integer *);
    static doublereal growto;
    static integer its;
#define b_subscr(a_1,a_2) (a_2)*b_dim1 + a_1
#define b_ref(a_1,a_2) b[b_subscr(a_1,a_2)]
#define h___subscr(a_1,a_2) (a_2)*h_dim1 + a_1
#define h___ref(a_1,a_2) h__[h___subscr(a_1,a_2)]


    h_dim1 = *ldh;
    h_offset = 1 + h_dim1 * 1;
    h__ -= h_offset;
    --v;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    --rwork;

    /* Function Body */
    *info = 0;

/*     GROWTO is the threshold used in the acceptance test for an   
       eigenvector. */

    rootn = sqrt((doublereal) (*n));
    growto = .1 / rootn;
/* Computing MAX */
    d__1 = 1., d__2 = *eps3 * rootn;
    nrmsml = max(d__1,d__2) * *smlnum;

/*     Form B = H - W*I (except that the subdiagonal elements are not   
       stored). */

    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	i__2 = j - 1;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    i__3 = b_subscr(i__, j);
	    i__4 = h___subscr(i__, j);
	    b[i__3].r = h__[i__4].r, b[i__3].i = h__[i__4].i;
/* L10: */
	}
	i__2 = b_subscr(j, j);
	i__3 = h___subscr(j, j);
	z__1.r = h__[i__3].r - w->r, z__1.i = h__[i__3].i - w->i;
	b[i__2].r = z__1.r, b[i__2].i = z__1.i;
/* L20: */
    }

    if (*noinit) {

/*        Initialize V. */

	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    i__2 = i__;
	    v[i__2].r = *eps3, v[i__2].i = 0.;
/* L30: */
	}
    } else {

/*        Scale supplied initial vector. */

	vnorm = dznrm2_(n, &v[1], &c__1);
	d__1 = *eps3 * rootn / max(vnorm,nrmsml);
	zdscal_(n, &d__1, &v[1], &c__1);
    }

    if (*rightv) {

/*        LU decomposition with partial pivoting of B, replacing zero   
          pivots by EPS3. */

	i__1 = *n - 1;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    i__2 = h___subscr(i__ + 1, i__);
	    ei.r = h__[i__2].r, ei.i = h__[i__2].i;
	    i__2 = b_subscr(i__, i__);
	    if ((d__1 = b[i__2].r, abs(d__1)) + (d__2 = d_imag(&b_ref(i__, 
		    i__)), abs(d__2)) < (d__3 = ei.r, abs(d__3)) + (d__4 = 
		    d_imag(&ei), abs(d__4))) {

/*              Interchange rows and eliminate. */

		zladiv_(&z__1, &b_ref(i__, i__), &ei);
		x.r = z__1.r, x.i = z__1.i;
		i__2 = b_subscr(i__, i__);
		b[i__2].r = ei.r, b[i__2].i = ei.i;
		i__2 = *n;
		for (j = i__ + 1; j <= i__2; ++j) {
		    i__3 = b_subscr(i__ + 1, j);
		    temp.r = b[i__3].r, temp.i = b[i__3].i;
		    i__3 = b_subscr(i__ + 1, j);
		    i__4 = b_subscr(i__, j);
		    z__2.r = x.r * temp.r - x.i * temp.i, z__2.i = x.r * 
			    temp.i + x.i * temp.r;
		    z__1.r = b[i__4].r - z__2.r, z__1.i = b[i__4].i - z__2.i;
		    b[i__3].r = z__1.r, b[i__3].i = z__1.i;
		    i__3 = b_subscr(i__, j);
		    b[i__3].r = temp.r, b[i__3].i = temp.i;
/* L40: */
		}
	    } else {

/*              Eliminate without interchange. */

		i__2 = b_subscr(i__, i__);
		if (b[i__2].r == 0. && b[i__2].i == 0.) {
		    i__3 = b_subscr(i__, i__);
		    b[i__3].r = *eps3, b[i__3].i = 0.;
		}
		zladiv_(&z__1, &ei, &b_ref(i__, i__));
		x.r = z__1.r, x.i = z__1.i;
		if (x.r != 0. || x.i != 0.) {
		    i__2 = *n;
		    for (j = i__ + 1; j <= i__2; ++j) {
			i__3 = b_subscr(i__ + 1, j);
			i__4 = b_subscr(i__ + 1, j);
			i__5 = b_subscr(i__, j);
			z__2.r = x.r * b[i__5].r - x.i * b[i__5].i, z__2.i = 
				x.r * b[i__5].i + x.i * b[i__5].r;
			z__1.r = b[i__4].r - z__2.r, z__1.i = b[i__4].i - 
				z__2.i;
			b[i__3].r = z__1.r, b[i__3].i = z__1.i;
/* L50: */
		    }
		}
	    }
/* L60: */
	}
	i__1 = b_subscr(*n, *n);
	if (b[i__1].r == 0. && b[i__1].i == 0.) {
	    i__2 = b_subscr(*n, *n);
	    b[i__2].r = *eps3, b[i__2].i = 0.;
	}

	*(unsigned char *)trans = 'N';

    } else {

/*        UL decomposition with partial pivoting of B, replacing zero   
          pivots by EPS3. */

	for (j = *n; j >= 2; --j) {
	    i__1 = h___subscr(j, j - 1);
	    ej.r = h__[i__1].r, ej.i = h__[i__1].i;
	    i__1 = b_subscr(j, j);
	    if ((d__1 = b[i__1].r, abs(d__1)) + (d__2 = d_imag(&b_ref(j, j)), 
		    abs(d__2)) < (d__3 = ej.r, abs(d__3)) + (d__4 = d_imag(&
		    ej), abs(d__4))) {

/*              Interchange columns and eliminate. */

		zladiv_(&z__1, &b_ref(j, j), &ej);
		x.r = z__1.r, x.i = z__1.i;
		i__1 = b_subscr(j, j);
		b[i__1].r = ej.r, b[i__1].i = ej.i;
		i__1 = j - 1;
		for (i__ = 1; i__ <= i__1; ++i__) {
		    i__2 = b_subscr(i__, j - 1);
		    temp.r = b[i__2].r, temp.i = b[i__2].i;
		    i__2 = b_subscr(i__, j - 1);
		    i__3 = b_subscr(i__, j);
		    z__2.r = x.r * temp.r - x.i * temp.i, z__2.i = x.r * 
			    temp.i + x.i * temp.r;
		    z__1.r = b[i__3].r - z__2.r, z__1.i = b[i__3].i - z__2.i;
		    b[i__2].r = z__1.r, b[i__2].i = z__1.i;
		    i__2 = b_subscr(i__, j);
		    b[i__2].r = temp.r, b[i__2].i = temp.i;
/* L70: */
		}
	    } else {

/*              Eliminate without interchange. */

		i__1 = b_subscr(j, j);
		if (b[i__1].r == 0. && b[i__1].i == 0.) {
		    i__2 = b_subscr(j, j);
		    b[i__2].r = *eps3, b[i__2].i = 0.;
		}
		zladiv_(&z__1, &ej, &b_ref(j, j));
		x.r = z__1.r, x.i = z__1.i;
		if (x.r != 0. || x.i != 0.) {
		    i__1 = j - 1;
		    for (i__ = 1; i__ <= i__1; ++i__) {
			i__2 = b_subscr(i__, j - 1);
			i__3 = b_subscr(i__, j - 1);
			i__4 = b_subscr(i__, j);
			z__2.r = x.r * b[i__4].r - x.i * b[i__4].i, z__2.i = 
				x.r * b[i__4].i + x.i * b[i__4].r;
			z__1.r = b[i__3].r - z__2.r, z__1.i = b[i__3].i - 
				z__2.i;
			b[i__2].r = z__1.r, b[i__2].i = z__1.i;
/* L80: */
		    }
		}
	    }
/* L90: */
	}
	i__1 = b_subscr(1, 1);
	if (b[i__1].r == 0. && b[i__1].i == 0.) {
	    i__2 = b_subscr(1, 1);
	    b[i__2].r = *eps3, b[i__2].i = 0.;
	}

	*(unsigned char *)trans = 'C';

    }

    *(unsigned char *)normin = 'N';
    i__1 = *n;
    for (its = 1; its <= i__1; ++its) {

/*        Solve U*x = scale*v for a right eigenvector   
            or U'*x = scale*v for a left eigenvector,   
          overwriting x on v. */

	zlatrs_("Upper", trans, "Nonunit", normin, n, &b[b_offset], ldb, &v[1]
		, &scale, &rwork[1], &ierr);
	*(unsigned char *)normin = 'Y';

/*        Test for sufficient growth in the norm of v. */

	vnorm = dzasum_(n, &v[1], &c__1);
	if (vnorm >= growto * scale) {
	    goto L120;
	}

/*        Choose new orthogonal starting vector and try again. */

	rtemp = *eps3 / (rootn + 1.);
	v[1].r = *eps3, v[1].i = 0.;
	i__2 = *n;
	for (i__ = 2; i__ <= i__2; ++i__) {
	    i__3 = i__;
	    v[i__3].r = rtemp, v[i__3].i = 0.;
/* L100: */
	}
	i__2 = *n - its + 1;
	i__3 = *n - its + 1;
	d__1 = *eps3 * rootn;
	z__1.r = v[i__3].r - d__1, z__1.i = v[i__3].i;
	v[i__2].r = z__1.r, v[i__2].i = z__1.i;
/* L110: */
    }

/*     Failure to find eigenvector in N iterations. */

    *info = 1;

L120:

/*     Normalize eigenvector. */

    i__ = izamax_(n, &v[1], &c__1);
    i__1 = i__;
    d__3 = 1. / ((d__1 = v[i__1].r, abs(d__1)) + (d__2 = d_imag(&v[i__]), abs(
	    d__2)));
    zdscal_(n, &d__3, &v[1], &c__1);

    return 0;

/*     End of ZLAEIN */

} /* zlaein_ */

#undef h___ref
#undef h___subscr
#undef b_ref
#undef b_subscr


