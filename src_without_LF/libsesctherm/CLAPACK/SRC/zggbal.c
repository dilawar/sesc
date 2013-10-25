#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zggbal_(char *job, integer *n, doublecomplex *a, integer 
	*lda, doublecomplex *b, integer *ldb, integer *ilo, integer *ihi, 
	doublereal *lscale, doublereal *rscale, doublereal *work, integer *
	info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZGGBAL balances a pair of general complex matrices (A,B).  This   
    involves, first, permuting A and B by similarity transformations to   
    isolate eigenvalues in the first 1 to ILO$-$1 and last IHI+1 to N   
    elements on the diagonal; and second, applying a diagonal similarity   
    transformation to rows and columns ILO to IHI to make the rows   
    and columns as close in norm as possible. Both steps are optional.   

    Balancing may reduce the 1-norm of the matrices, and improve the   
    accuracy of the computed eigenvalues and/or eigenvectors in the   
    generalized eigenvalue problem A*x = lambda*B*x.   

    Arguments   
    =========   

    JOB     (input) CHARACTER*1   
            Specifies the operations to be performed on A and B:   
            = 'N':  none:  simply set ILO = 1, IHI = N, LSCALE(I) = 1.0   
                    and RSCALE(I) = 1.0 for i=1,...,N;   
            = 'P':  permute only;   
            = 'S':  scale only;   
            = 'B':  both permute and scale.   

    N       (input) INTEGER   
            The order of the matrices A and B.  N >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the input matrix A.   
            On exit, A is overwritten by the balanced matrix.   
            If JOB = 'N', A is not referenced.   

    LDA     (input) INTEGER   
            The leading dimension of the array A. LDA >= max(1,N).   

    B       (input/output) COMPLEX*16 array, dimension (LDB,N)   
            On entry, the input matrix B.   
            On exit, B is overwritten by the balanced matrix.   
            If JOB = 'N', B is not referenced.   

    LDB     (input) INTEGER   
            The leading dimension of the array B. LDB >= max(1,N).   

    ILO     (output) INTEGER   
    IHI     (output) INTEGER   
            ILO and IHI are set to integers such that on exit   
            A(i,j) = 0 and B(i,j) = 0 if i > j and   
            j = 1,...,ILO-1 or i = IHI+1,...,N.   
            If JOB = 'N' or 'S', ILO = 1 and IHI = N.   

    LSCALE  (output) DOUBLE PRECISION array, dimension (N)   
            Details of the permutations and scaling factors applied   
            to the left side of A and B.  If P(j) is the index of the   
            row interchanged with row j, and D(j) is the scaling factor   
            applied to row j, then   
              LSCALE(j) = P(j)    for J = 1,...,ILO-1   
                        = D(j)    for J = ILO,...,IHI   
                        = P(j)    for J = IHI+1,...,N.   
            The order in which the interchanges are made is N to IHI+1,   
            then 1 to ILO-1.   

    RSCALE  (output) DOUBLE PRECISION array, dimension (N)   
            Details of the permutations and scaling factors applied   
            to the right side of A and B.  If P(j) is the index of the   
            column interchanged with column j, and D(j) is the scaling   
            factor applied to column j, then   
              RSCALE(j) = P(j)    for J = 1,...,ILO-1   
                        = D(j)    for J = ILO,...,IHI   
                        = P(j)    for J = IHI+1,...,N.   
            The order in which the interchanges are made is N to IHI+1,   
            then 1 to ILO-1.   

    WORK    (workspace) DOUBLE PRECISION array, dimension (6*N)   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   

    Further Details   
    ===============   

    See R.C. WARD, Balancing the generalized eigenvalue problem,   
                   SIAM J. Sci. Stat. Comp. 2 (1981), 141-152.   

    =====================================================================   


       Test the input parameters   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    static doublereal c_b35 = 10.;
    static doublereal c_b71 = .5;
    
    /* System generated locals */
    integer a_dim1, a_offset, b_dim1, b_offset, i__1, i__2, i__3, i__4;
    doublereal d__1, d__2, d__3;
    /* Builtin functions */
    double d_lg10(doublereal *), d_imag(doublecomplex *), z_abs(doublecomplex 
	    *), d_sign(doublereal *, doublereal *), pow_di(doublereal *, 
	    integer *);
    /* Local variables */
    static integer lcab;
    static doublereal beta, coef;
    static integer irab, lrab;
    static doublereal basl, cmax;
    extern doublereal ddot_(integer *, doublereal *, integer *, doublereal *, 
	    integer *);
    static doublereal coef2, coef5;
    static integer i__, j, k, l, m;
    static doublereal gamma, t, alpha;
    extern /* Subroutine */ int dscal_(integer *, doublereal *, doublereal *, 
	    integer *);
    extern logical lsame_(char *, char *);
    static doublereal sfmin, sfmax;
    static integer iflow;
    extern /* Subroutine */ int daxpy_(integer *, doublereal *, doublereal *, 
	    integer *, doublereal *, integer *);
    static integer kount;
    extern /* Subroutine */ int zswap_(integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *);
    static integer jc;
    static doublereal ta, tb, tc;
    extern doublereal dlamch_(char *);
    static integer ir, it;
    static doublereal ew;
    static integer nr;
    static doublereal pgamma;
    extern /* Subroutine */ int xerbla_(char *, integer *), zdscal_(
	    integer *, doublereal *, doublecomplex *, integer *);
    static integer lsfmin;
    extern integer izamax_(integer *, doublecomplex *, integer *);
    static integer lsfmax, ip1, jp1, lm1;
    static doublereal cab, rab, ewc, cor, sum;
    static integer nrp2, icab;
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]
#define b_subscr(a_1,a_2) (a_2)*b_dim1 + a_1
#define b_ref(a_1,a_2) b[b_subscr(a_1,a_2)]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    --lscale;
    --rscale;
    --work;

    /* Function Body */
    *info = 0;
    if (! lsame_(job, "N") && ! lsame_(job, "P") && ! lsame_(job, "S") 
	    && ! lsame_(job, "B")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*n)) {
	*info = -4;
    } else if (*ldb < max(1,*n)) {
	*info = -5;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZGGBAL", &i__1);
	return 0;
    }

    k = 1;
    l = *n;

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }

    if (lsame_(job, "N")) {
	*ilo = 1;
	*ihi = *n;
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    lscale[i__] = 1.;
	    rscale[i__] = 1.;
/* L10: */
	}
	return 0;
    }

    if (k == l) {
	*ilo = 1;
	*ihi = 1;
	lscale[1] = 1.;
	rscale[1] = 1.;
	return 0;
    }

    if (lsame_(job, "S")) {
	goto L190;
    }

    goto L30;

/*     Permute the matrices A and B to isolate the eigenvalues.   

       Find row with one nonzero in columns 1 through L */

L20:
    l = lm1;
    if (l != 1) {
	goto L30;
    }

    rscale[1] = 1.;
    lscale[1] = 1.;
    goto L190;

L30:
    lm1 = l - 1;
    for (i__ = l; i__ >= 1; --i__) {
	i__1 = lm1;
	for (j = 1; j <= i__1; ++j) {
	    jp1 = j + 1;
	    i__2 = a_subscr(i__, j);
	    i__3 = b_subscr(i__, j);
	    if (a[i__2].r != 0. || a[i__2].i != 0. || (b[i__3].r != 0. || b[
		    i__3].i != 0.)) {
		goto L50;
	    }
/* L40: */
	}
	j = l;
	goto L70;

L50:
	i__1 = l;
	for (j = jp1; j <= i__1; ++j) {
	    i__2 = a_subscr(i__, j);
	    i__3 = b_subscr(i__, j);
	    if (a[i__2].r != 0. || a[i__2].i != 0. || (b[i__3].r != 0. || b[
		    i__3].i != 0.)) {
		goto L80;
	    }
/* L60: */
	}
	j = jp1 - 1;

L70:
	m = l;
	iflow = 1;
	goto L160;
L80:
	;
    }
    goto L100;

/*     Find column with one nonzero in rows K through N */

L90:
    ++k;

L100:
    i__1 = l;
    for (j = k; j <= i__1; ++j) {
	i__2 = lm1;
	for (i__ = k; i__ <= i__2; ++i__) {
	    ip1 = i__ + 1;
	    i__3 = a_subscr(i__, j);
	    i__4 = b_subscr(i__, j);
	    if (a[i__3].r != 0. || a[i__3].i != 0. || (b[i__4].r != 0. || b[
		    i__4].i != 0.)) {
		goto L120;
	    }
/* L110: */
	}
	i__ = l;
	goto L140;
L120:
	i__2 = l;
	for (i__ = ip1; i__ <= i__2; ++i__) {
	    i__3 = a_subscr(i__, j);
	    i__4 = b_subscr(i__, j);
	    if (a[i__3].r != 0. || a[i__3].i != 0. || (b[i__4].r != 0. || b[
		    i__4].i != 0.)) {
		goto L150;
	    }
/* L130: */
	}
	i__ = ip1 - 1;
L140:
	m = k;
	iflow = 2;
	goto L160;
L150:
	;
    }
    goto L190;

/*     Permute rows M and I */

L160:
    lscale[m] = (doublereal) i__;
    if (i__ == m) {
	goto L170;
    }
    i__1 = *n - k + 1;
    zswap_(&i__1, &a_ref(i__, k), lda, &a_ref(m, k), lda);
    i__1 = *n - k + 1;
    zswap_(&i__1, &b_ref(i__, k), ldb, &b_ref(m, k), ldb);

/*     Permute columns M and J */

L170:
    rscale[m] = (doublereal) j;
    if (j == m) {
	goto L180;
    }
    zswap_(&l, &a_ref(1, j), &c__1, &a_ref(1, m), &c__1);
    zswap_(&l, &b_ref(1, j), &c__1, &b_ref(1, m), &c__1);

L180:
    switch (iflow) {
	case 1:  goto L20;
	case 2:  goto L90;
    }

L190:
    *ilo = k;
    *ihi = l;

    if (*ilo == *ihi) {
	return 0;
    }

    if (lsame_(job, "P")) {
	return 0;
    }

/*     Balance the submatrix in rows ILO to IHI. */

    nr = *ihi - *ilo + 1;
    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	rscale[i__] = 0.;
	lscale[i__] = 0.;

	work[i__] = 0.;
	work[i__ + *n] = 0.;
	work[i__ + (*n << 1)] = 0.;
	work[i__ + *n * 3] = 0.;
	work[i__ + (*n << 2)] = 0.;
	work[i__ + *n * 5] = 0.;
/* L200: */
    }

/*     Compute right side vector in resulting linear equations */

    basl = d_lg10(&c_b35);
    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	i__2 = *ihi;
	for (j = *ilo; j <= i__2; ++j) {
	    i__3 = a_subscr(i__, j);
	    if (a[i__3].r == 0. && a[i__3].i == 0.) {
		ta = 0.;
		goto L210;
	    }
	    i__3 = a_subscr(i__, j);
	    d__3 = (d__1 = a[i__3].r, abs(d__1)) + (d__2 = d_imag(&a_ref(i__, 
		    j)), abs(d__2));
	    ta = d_lg10(&d__3) / basl;

L210:
	    i__3 = b_subscr(i__, j);
	    if (b[i__3].r == 0. && b[i__3].i == 0.) {
		tb = 0.;
		goto L220;
	    }
	    i__3 = b_subscr(i__, j);
	    d__3 = (d__1 = b[i__3].r, abs(d__1)) + (d__2 = d_imag(&b_ref(i__, 
		    j)), abs(d__2));
	    tb = d_lg10(&d__3) / basl;

L220:
	    work[i__ + (*n << 2)] = work[i__ + (*n << 2)] - ta - tb;
	    work[j + *n * 5] = work[j + *n * 5] - ta - tb;
/* L230: */
	}
/* L240: */
    }

    coef = 1. / (doublereal) (nr << 1);
    coef2 = coef * coef;
    coef5 = coef2 * .5;
    nrp2 = nr + 2;
    beta = 0.;
    it = 1;

/*     Start generalized conjugate gradient iteration */

L250:

    gamma = ddot_(&nr, &work[*ilo + (*n << 2)], &c__1, &work[*ilo + (*n << 2)]
	    , &c__1) + ddot_(&nr, &work[*ilo + *n * 5], &c__1, &work[*ilo + *
	    n * 5], &c__1);

    ew = 0.;
    ewc = 0.;
    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	ew += work[i__ + (*n << 2)];
	ewc += work[i__ + *n * 5];
/* L260: */
    }

/* Computing 2nd power */
    d__1 = ew;
/* Computing 2nd power */
    d__2 = ewc;
/* Computing 2nd power */
    d__3 = ew - ewc;
    gamma = coef * gamma - coef2 * (d__1 * d__1 + d__2 * d__2) - coef5 * (
	    d__3 * d__3);
    if (gamma == 0.) {
	goto L350;
    }
    if (it != 1) {
	beta = gamma / pgamma;
    }
    t = coef5 * (ewc - ew * 3.);
    tc = coef5 * (ew - ewc * 3.);

    dscal_(&nr, &beta, &work[*ilo], &c__1);
    dscal_(&nr, &beta, &work[*ilo + *n], &c__1);

    daxpy_(&nr, &coef, &work[*ilo + (*n << 2)], &c__1, &work[*ilo + *n], &
	    c__1);
    daxpy_(&nr, &coef, &work[*ilo + *n * 5], &c__1, &work[*ilo], &c__1);

    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	work[i__] += tc;
	work[i__ + *n] += t;
/* L270: */
    }

/*     Apply matrix to vector */

    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	kount = 0;
	sum = 0.;
	i__2 = *ihi;
	for (j = *ilo; j <= i__2; ++j) {
	    i__3 = a_subscr(i__, j);
	    if (a[i__3].r == 0. && a[i__3].i == 0.) {
		goto L280;
	    }
	    ++kount;
	    sum += work[j];
L280:
	    i__3 = b_subscr(i__, j);
	    if (b[i__3].r == 0. && b[i__3].i == 0.) {
		goto L290;
	    }
	    ++kount;
	    sum += work[j];
L290:
	    ;
	}
	work[i__ + (*n << 1)] = (doublereal) kount * work[i__ + *n] + sum;
/* L300: */
    }

    i__1 = *ihi;
    for (j = *ilo; j <= i__1; ++j) {
	kount = 0;
	sum = 0.;
	i__2 = *ihi;
	for (i__ = *ilo; i__ <= i__2; ++i__) {
	    i__3 = a_subscr(i__, j);
	    if (a[i__3].r == 0. && a[i__3].i == 0.) {
		goto L310;
	    }
	    ++kount;
	    sum += work[i__ + *n];
L310:
	    i__3 = b_subscr(i__, j);
	    if (b[i__3].r == 0. && b[i__3].i == 0.) {
		goto L320;
	    }
	    ++kount;
	    sum += work[i__ + *n];
L320:
	    ;
	}
	work[j + *n * 3] = (doublereal) kount * work[j] + sum;
/* L330: */
    }

    sum = ddot_(&nr, &work[*ilo + *n], &c__1, &work[*ilo + (*n << 1)], &c__1) 
	    + ddot_(&nr, &work[*ilo], &c__1, &work[*ilo + *n * 3], &c__1);
    alpha = gamma / sum;

/*     Determine correction to current iteration */

    cmax = 0.;
    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	cor = alpha * work[i__ + *n];
	if (abs(cor) > cmax) {
	    cmax = abs(cor);
	}
	lscale[i__] += cor;
	cor = alpha * work[i__];
	if (abs(cor) > cmax) {
	    cmax = abs(cor);
	}
	rscale[i__] += cor;
/* L340: */
    }
    if (cmax < .5) {
	goto L350;
    }

    d__1 = -alpha;
    daxpy_(&nr, &d__1, &work[*ilo + (*n << 1)], &c__1, &work[*ilo + (*n << 2)]
	    , &c__1);
    d__1 = -alpha;
    daxpy_(&nr, &d__1, &work[*ilo + *n * 3], &c__1, &work[*ilo + *n * 5], &
	    c__1);

    pgamma = gamma;
    ++it;
    if (it <= nrp2) {
	goto L250;
    }

/*     End generalized conjugate gradient iteration */

L350:
    sfmin = dlamch_("S");
    sfmax = 1. / sfmin;
    lsfmin = (integer) (d_lg10(&sfmin) / basl + 1.);
    lsfmax = (integer) (d_lg10(&sfmax) / basl);
    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	i__2 = *n - *ilo + 1;
	irab = izamax_(&i__2, &a_ref(i__, *ilo), lda);
	rab = z_abs(&a_ref(i__, irab + *ilo - 1));
	i__2 = *n - *ilo + 1;
	irab = izamax_(&i__2, &b_ref(i__, *ilo), lda);
/* Computing MAX */
	d__1 = rab, d__2 = z_abs(&b_ref(i__, irab + *ilo - 1));
	rab = max(d__1,d__2);
	d__1 = rab + sfmin;
	lrab = (integer) (d_lg10(&d__1) / basl + 1.);
	ir = (integer) (lscale[i__] + d_sign(&c_b71, &lscale[i__]));
/* Computing MIN */
	i__2 = max(ir,lsfmin), i__2 = min(i__2,lsfmax), i__3 = lsfmax - lrab;
	ir = min(i__2,i__3);
	lscale[i__] = pow_di(&c_b35, &ir);
	icab = izamax_(ihi, &a_ref(1, i__), &c__1);
	cab = z_abs(&a_ref(icab, i__));
	icab = izamax_(ihi, &b_ref(1, i__), &c__1);
/* Computing MAX */
	d__1 = cab, d__2 = z_abs(&b_ref(icab, i__));
	cab = max(d__1,d__2);
	d__1 = cab + sfmin;
	lcab = (integer) (d_lg10(&d__1) / basl + 1.);
	jc = (integer) (rscale[i__] + d_sign(&c_b71, &rscale[i__]));
/* Computing MIN */
	i__2 = max(jc,lsfmin), i__2 = min(i__2,lsfmax), i__3 = lsfmax - lcab;
	jc = min(i__2,i__3);
	rscale[i__] = pow_di(&c_b35, &jc);
/* L360: */
    }

/*     Row scaling of matrices A and B */

    i__1 = *ihi;
    for (i__ = *ilo; i__ <= i__1; ++i__) {
	i__2 = *n - *ilo + 1;
	zdscal_(&i__2, &lscale[i__], &a_ref(i__, *ilo), lda);
	i__2 = *n - *ilo + 1;
	zdscal_(&i__2, &lscale[i__], &b_ref(i__, *ilo), ldb);
/* L370: */
    }

/*     Column scaling of matrices A and B */

    i__1 = *ihi;
    for (j = *ilo; j <= i__1; ++j) {
	zdscal_(ihi, &rscale[j], &a_ref(1, j), &c__1);
	zdscal_(ihi, &rscale[j], &b_ref(1, j), &c__1);
/* L380: */
    }

    return 0;

/*     End of ZGGBAL */

} /* zggbal_ */

#undef b_ref
#undef b_subscr
#undef a_ref
#undef a_subscr


