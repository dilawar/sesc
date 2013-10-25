#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int slaexc_(logical *wantq, integer *n, real *t, integer *
	ldt, real *q, integer *ldq, integer *j1, integer *n1, integer *n2, 
	real *work, integer *info)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    SLAEXC swaps adjacent diagonal blocks T11 and T22 of order 1 or 2 in   
    an upper quasi-triangular matrix T by an orthogonal similarity   
    transformation.   

    T must be in Schur canonical form, that is, block upper triangular   
    with 1-by-1 and 2-by-2 diagonal blocks; each 2-by-2 diagonal block   
    has its diagonal elemnts equal and its off-diagonal elements of   
    opposite sign.   

    Arguments   
    =========   

    WANTQ   (input) LOGICAL   
            = .TRUE. : accumulate the transformation in the matrix Q;   
            = .FALSE.: do not accumulate the transformation.   

    N       (input) INTEGER   
            The order of the matrix T. N >= 0.   

    T       (input/output) REAL array, dimension (LDT,N)   
            On entry, the upper quasi-triangular matrix T, in Schur   
            canonical form.   
            On exit, the updated matrix T, again in Schur canonical form.   

    LDT     (input)  INTEGER   
            The leading dimension of the array T. LDT >= max(1,N).   

    Q       (input/output) REAL array, dimension (LDQ,N)   
            On entry, if WANTQ is .TRUE., the orthogonal matrix Q.   
            On exit, if WANTQ is .TRUE., the updated matrix Q.   
            If WANTQ is .FALSE., Q is not referenced.   

    LDQ     (input) INTEGER   
            The leading dimension of the array Q.   
            LDQ >= 1; and if WANTQ is .TRUE., LDQ >= N.   

    J1      (input) INTEGER   
            The index of the first row of the first block T11.   

    N1      (input) INTEGER   
            The order of the first block T11. N1 = 0, 1 or 2.   

    N2      (input) INTEGER   
            The order of the second block T22. N2 = 0, 1 or 2.   

    WORK    (workspace) REAL array, dimension (N)   

    INFO    (output) INTEGER   
            = 0: successful exit   
            = 1: the transformed matrix T would be too far from Schur   
                 form; the blocks are not swapped and T and Q are   
                 unchanged.   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static integer c__4 = 4;
    static logical c_false = FALSE_;
    static integer c_n1 = -1;
    static integer c__2 = 2;
    static integer c__3 = 3;
    
    /* System generated locals */
    integer q_dim1, q_offset, t_dim1, t_offset, i__1;
    real r__1, r__2, r__3, r__4, r__5, r__6;
    /* Local variables */
    static integer ierr;
    static real temp;
    extern /* Subroutine */ int srot_(integer *, real *, integer *, real *, 
	    integer *, real *, real *);
    static real d__[16]	/* was [4][4] */;
    static integer k;
    static real u[3], scale, x[4]	/* was [2][2] */, dnorm;
    static integer j2, j3, j4;
    static real xnorm, u1[3], u2[3];
    extern /* Subroutine */ int slanv2_(real *, real *, real *, real *, real *
	    , real *, real *, real *, real *, real *), slasy2_(logical *, 
	    logical *, integer *, integer *, integer *, real *, integer *, 
	    real *, integer *, real *, integer *, real *, real *, integer *, 
	    real *, integer *);
    static integer nd;
    static real cs, t11, t22, t33, sn;
    extern doublereal slamch_(char *), slange_(char *, integer *, 
	    integer *, real *, integer *, real *);
    extern /* Subroutine */ int slarfg_(integer *, real *, real *, integer *, 
	    real *), slacpy_(char *, integer *, integer *, real *, integer *, 
	    real *, integer *), slartg_(real *, real *, real *, real *
	    , real *);
    static real thresh;
    extern /* Subroutine */ int slarfx_(char *, integer *, integer *, real *, 
	    real *, real *, integer *, real *);
    static real smlnum, wi1, wi2, wr1, wr2, eps, tau, tau1, tau2;
#define d___ref(a_1,a_2) d__[(a_2)*4 + a_1 - 5]
#define q_ref(a_1,a_2) q[(a_2)*q_dim1 + a_1]
#define t_ref(a_1,a_2) t[(a_2)*t_dim1 + a_1]
#define x_ref(a_1,a_2) x[(a_2)*2 + a_1 - 3]


    t_dim1 = *ldt;
    t_offset = 1 + t_dim1 * 1;
    t -= t_offset;
    q_dim1 = *ldq;
    q_offset = 1 + q_dim1 * 1;
    q -= q_offset;
    --work;

    /* Function Body */
    *info = 0;

/*     Quick return if possible */

    if (*n == 0 || *n1 == 0 || *n2 == 0) {
	return 0;
    }
    if (*j1 + *n1 > *n) {
	return 0;
    }

    j2 = *j1 + 1;
    j3 = *j1 + 2;
    j4 = *j1 + 3;

    if (*n1 == 1 && *n2 == 1) {

/*        Swap two 1-by-1 blocks. */

	t11 = t_ref(*j1, *j1);
	t22 = t_ref(j2, j2);

/*        Determine the transformation to perform the interchange. */

	r__1 = t22 - t11;
	slartg_(&t_ref(*j1, j2), &r__1, &cs, &sn, &temp);

/*        Apply transformation to the matrix T. */

	if (j3 <= *n) {
	    i__1 = *n - *j1 - 1;
	    srot_(&i__1, &t_ref(*j1, j3), ldt, &t_ref(j2, j3), ldt, &cs, &sn);
	}
	i__1 = *j1 - 1;
	srot_(&i__1, &t_ref(1, *j1), &c__1, &t_ref(1, j2), &c__1, &cs, &sn);

	t_ref(*j1, *j1) = t22;
	t_ref(j2, j2) = t11;

	if (*wantq) {

/*           Accumulate transformation in the matrix Q. */

	    srot_(n, &q_ref(1, *j1), &c__1, &q_ref(1, j2), &c__1, &cs, &sn);
	}

    } else {

/*        Swapping involves at least one 2-by-2 block.   

          Copy the diagonal block of order N1+N2 to the local array D   
          and compute its norm. */

	nd = *n1 + *n2;
	slacpy_("Full", &nd, &nd, &t_ref(*j1, *j1), ldt, d__, &c__4);
	dnorm = slange_("Max", &nd, &nd, d__, &c__4, &work[1]);

/*        Compute machine-dependent threshold for test for accepting   
          swap. */

	eps = slamch_("P");
	smlnum = slamch_("S") / eps;
/* Computing MAX */
	r__1 = eps * 10.f * dnorm;
	thresh = dmax(r__1,smlnum);

/*        Solve T11*X - X*T22 = scale*T12 for X. */

	slasy2_(&c_false, &c_false, &c_n1, n1, n2, d__, &c__4, &d___ref(*n1 + 
		1, *n1 + 1), &c__4, &d___ref(1, *n1 + 1), &c__4, &scale, x, &
		c__2, &xnorm, &ierr);

/*        Swap the adjacent diagonal blocks. */

	k = *n1 + *n1 + *n2 - 3;
	switch (k) {
	    case 1:  goto L10;
	    case 2:  goto L20;
	    case 3:  goto L30;
	}

L10:

/*        N1 = 1, N2 = 2: generate elementary reflector H so that:   

          ( scale, X11, X12 ) H = ( 0, 0, * ) */

	u[0] = scale;
	u[1] = x_ref(1, 1);
	u[2] = x_ref(1, 2);
	slarfg_(&c__3, &u[2], u, &c__1, &tau);
	u[2] = 1.f;
	t11 = t_ref(*j1, *j1);

/*        Perform swap provisionally on diagonal block in D. */

	slarfx_("L", &c__3, &c__3, u, &tau, d__, &c__4, &work[1]);
	slarfx_("R", &c__3, &c__3, u, &tau, d__, &c__4, &work[1]);

/*        Test whether to reject swap.   

   Computing MAX */
	r__4 = (r__1 = d___ref(3, 1), dabs(r__1)), r__5 = (r__2 = d___ref(3, 
		2), dabs(r__2)), r__4 = max(r__4,r__5), r__5 = (r__3 = 
		d___ref(3, 3) - t11, dabs(r__3));
	if (dmax(r__4,r__5) > thresh) {
	    goto L50;
	}

/*        Accept swap: apply transformation to the entire matrix T. */

	i__1 = *n - *j1 + 1;
	slarfx_("L", &c__3, &i__1, u, &tau, &t_ref(*j1, *j1), ldt, &work[1]);
	slarfx_("R", &j2, &c__3, u, &tau, &t_ref(1, *j1), ldt, &work[1]);

	t_ref(j3, *j1) = 0.f;
	t_ref(j3, j2) = 0.f;
	t_ref(j3, j3) = t11;

	if (*wantq) {

/*           Accumulate transformation in the matrix Q. */

	    slarfx_("R", n, &c__3, u, &tau, &q_ref(1, *j1), ldq, &work[1]);
	}
	goto L40;

L20:

/*        N1 = 2, N2 = 1: generate elementary reflector H so that:   

          H (  -X11 ) = ( * )   
            (  -X21 ) = ( 0 )   
            ( scale ) = ( 0 ) */

	u[0] = -x_ref(1, 1);
	u[1] = -x_ref(2, 1);
	u[2] = scale;
	slarfg_(&c__3, u, &u[1], &c__1, &tau);
	u[0] = 1.f;
	t33 = t_ref(j3, j3);

/*        Perform swap provisionally on diagonal block in D. */

	slarfx_("L", &c__3, &c__3, u, &tau, d__, &c__4, &work[1]);
	slarfx_("R", &c__3, &c__3, u, &tau, d__, &c__4, &work[1]);

/*        Test whether to reject swap.   

   Computing MAX */
	r__4 = (r__1 = d___ref(2, 1), dabs(r__1)), r__5 = (r__2 = d___ref(3, 
		1), dabs(r__2)), r__4 = max(r__4,r__5), r__5 = (r__3 = 
		d___ref(1, 1) - t33, dabs(r__3));
	if (dmax(r__4,r__5) > thresh) {
	    goto L50;
	}

/*        Accept swap: apply transformation to the entire matrix T. */

	slarfx_("R", &j3, &c__3, u, &tau, &t_ref(1, *j1), ldt, &work[1]);
	i__1 = *n - *j1;
	slarfx_("L", &c__3, &i__1, u, &tau, &t_ref(*j1, j2), ldt, &work[1]);

	t_ref(*j1, *j1) = t33;
	t_ref(j2, *j1) = 0.f;
	t_ref(j3, *j1) = 0.f;

	if (*wantq) {

/*           Accumulate transformation in the matrix Q. */

	    slarfx_("R", n, &c__3, u, &tau, &q_ref(1, *j1), ldq, &work[1]);
	}
	goto L40;

L30:

/*        N1 = 2, N2 = 2: generate elementary reflectors H(1) and H(2) so   
          that:   

          H(2) H(1) (  -X11  -X12 ) = (  *  * )   
                    (  -X21  -X22 )   (  0  * )   
                    ( scale    0  )   (  0  0 )   
                    (    0  scale )   (  0  0 ) */

	u1[0] = -x_ref(1, 1);
	u1[1] = -x_ref(2, 1);
	u1[2] = scale;
	slarfg_(&c__3, u1, &u1[1], &c__1, &tau1);
	u1[0] = 1.f;

	temp = -tau1 * (x_ref(1, 2) + u1[1] * x_ref(2, 2));
	u2[0] = -temp * u1[1] - x_ref(2, 2);
	u2[1] = -temp * u1[2];
	u2[2] = scale;
	slarfg_(&c__3, u2, &u2[1], &c__1, &tau2);
	u2[0] = 1.f;

/*        Perform swap provisionally on diagonal block in D. */

	slarfx_("L", &c__3, &c__4, u1, &tau1, d__, &c__4, &work[1])
		;
	slarfx_("R", &c__4, &c__3, u1, &tau1, d__, &c__4, &work[1])
		;
	slarfx_("L", &c__3, &c__4, u2, &tau2, &d___ref(2, 1), &c__4, &work[1]);
	slarfx_("R", &c__4, &c__3, u2, &tau2, &d___ref(1, 2), &c__4, &work[1]);

/*        Test whether to reject swap.   

   Computing MAX */
	r__5 = (r__1 = d___ref(3, 1), dabs(r__1)), r__6 = (r__2 = d___ref(3, 
		2), dabs(r__2)), r__5 = max(r__5,r__6), r__6 = (r__3 = 
		d___ref(4, 1), dabs(r__3)), r__5 = max(r__5,r__6), r__6 = (
		r__4 = d___ref(4, 2), dabs(r__4));
	if (dmax(r__5,r__6) > thresh) {
	    goto L50;
	}

/*        Accept swap: apply transformation to the entire matrix T. */

	i__1 = *n - *j1 + 1;
	slarfx_("L", &c__3, &i__1, u1, &tau1, &t_ref(*j1, *j1), ldt, &work[1]);
	slarfx_("R", &j4, &c__3, u1, &tau1, &t_ref(1, *j1), ldt, &work[1]);
	i__1 = *n - *j1 + 1;
	slarfx_("L", &c__3, &i__1, u2, &tau2, &t_ref(j2, *j1), ldt, &work[1]);
	slarfx_("R", &j4, &c__3, u2, &tau2, &t_ref(1, j2), ldt, &work[1]);

	t_ref(j3, *j1) = 0.f;
	t_ref(j3, j2) = 0.f;
	t_ref(j4, *j1) = 0.f;
	t_ref(j4, j2) = 0.f;

	if (*wantq) {

/*           Accumulate transformation in the matrix Q. */

	    slarfx_("R", n, &c__3, u1, &tau1, &q_ref(1, *j1), ldq, &work[1]);
	    slarfx_("R", n, &c__3, u2, &tau2, &q_ref(1, j2), ldq, &work[1]);
	}

L40:

	if (*n2 == 2) {

/*           Standardize new 2-by-2 block T11 */

	    slanv2_(&t_ref(*j1, *j1), &t_ref(*j1, j2), &t_ref(j2, *j1), &
		    t_ref(j2, j2), &wr1, &wi1, &wr2, &wi2, &cs, &sn);
	    i__1 = *n - *j1 - 1;
	    srot_(&i__1, &t_ref(*j1, *j1 + 2), ldt, &t_ref(j2, *j1 + 2), ldt, 
		    &cs, &sn);
	    i__1 = *j1 - 1;
	    srot_(&i__1, &t_ref(1, *j1), &c__1, &t_ref(1, j2), &c__1, &cs, &
		    sn);
	    if (*wantq) {
		srot_(n, &q_ref(1, *j1), &c__1, &q_ref(1, j2), &c__1, &cs, &
			sn);
	    }
	}

	if (*n1 == 2) {

/*           Standardize new 2-by-2 block T22 */

	    j3 = *j1 + *n2;
	    j4 = j3 + 1;
	    slanv2_(&t_ref(j3, j3), &t_ref(j3, j4), &t_ref(j4, j3), &t_ref(j4,
		     j4), &wr1, &wi1, &wr2, &wi2, &cs, &sn);
	    if (j3 + 2 <= *n) {
		i__1 = *n - j3 - 1;
		srot_(&i__1, &t_ref(j3, j3 + 2), ldt, &t_ref(j4, j3 + 2), ldt,
			 &cs, &sn);
	    }
	    i__1 = j3 - 1;
	    srot_(&i__1, &t_ref(1, j3), &c__1, &t_ref(1, j4), &c__1, &cs, &sn)
		    ;
	    if (*wantq) {
		srot_(n, &q_ref(1, j3), &c__1, &q_ref(1, j4), &c__1, &cs, &sn)
			;
	    }
	}

    }
    return 0;

/*     Exit with INFO = 1 if swap was rejected. */

L50:
    *info = 1;
    return 0;

/*     End of SLAEXC */

} /* slaexc_ */

#undef x_ref
#undef t_ref
#undef q_ref
#undef d___ref


