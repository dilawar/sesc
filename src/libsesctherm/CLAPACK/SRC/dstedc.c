#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dstedc_(char *compz, integer *n, doublereal *d__, 
	doublereal *e, doublereal *z__, integer *ldz, doublereal *work, 
	integer *lwork, integer *iwork, integer *liwork, integer *info)
{
/*  -- LAPACK driver routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    DSTEDC computes all eigenvalues and, optionally, eigenvectors of a   
    symmetric tridiagonal matrix using the divide and conquer method.   
    The eigenvectors of a full or band real symmetric matrix can also be   
    found if DSYTRD or DSPTRD or DSBTRD has been used to reduce this   
    matrix to tridiagonal form.   

    This code makes very mild assumptions about floating point   
    arithmetic. It will work on machines with a guard digit in   
    add/subtract, or on those binary machines without guard digits   
    which subtract like the Cray X-MP, Cray Y-MP, Cray C-90, or Cray-2.   
    It could conceivably fail on hexadecimal or decimal machines   
    without guard digits, but we know of none.  See DLAED3 for details.   

    Arguments   
    =========   

    COMPZ   (input) CHARACTER*1   
            = 'N':  Compute eigenvalues only.   
            = 'I':  Compute eigenvectors of tridiagonal matrix also.   
            = 'V':  Compute eigenvectors of original dense symmetric   
                    matrix also.  On entry, Z contains the orthogonal   
                    matrix used to reduce the original matrix to   
                    tridiagonal form.   

    N       (input) INTEGER   
            The dimension of the symmetric tridiagonal matrix.  N >= 0.   

    D       (input/output) DOUBLE PRECISION array, dimension (N)   
            On entry, the diagonal elements of the tridiagonal matrix.   
            On exit, if INFO = 0, the eigenvalues in ascending order.   

    E       (input/output) DOUBLE PRECISION array, dimension (N-1)   
            On entry, the subdiagonal elements of the tridiagonal matrix.   
            On exit, E has been destroyed.   

    Z       (input/output) DOUBLE PRECISION array, dimension (LDZ,N)   
            On entry, if COMPZ = 'V', then Z contains the orthogonal   
            matrix used in the reduction to tridiagonal form.   
            On exit, if INFO = 0, then if COMPZ = 'V', Z contains the   
            orthonormal eigenvectors of the original symmetric matrix,   
            and if COMPZ = 'I', Z contains the orthonormal eigenvectors   
            of the symmetric tridiagonal matrix.   
            If  COMPZ = 'N', then Z is not referenced.   

    LDZ     (input) INTEGER   
            The leading dimension of the array Z.  LDZ >= 1.   
            If eigenvectors are desired, then LDZ >= max(1,N).   

    WORK    (workspace/output) DOUBLE PRECISION array,   
                                           dimension (LWORK)   
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK.   
            If COMPZ = 'N' or N <= 1 then LWORK must be at least 1.   
            If COMPZ = 'V' and N > 1 then LWORK must be at least   
                           ( 1 + 3*N + 2*N*lg N + 3*N**2 ),   
                           where lg( N ) = smallest integer k such   
                           that 2**k >= N.   
            If COMPZ = 'I' and N > 1 then LWORK must be at least   
                           ( 1 + 4*N + N**2 ).   

            If LWORK = -1, then a workspace query is assumed; the routine   
            only calculates the optimal size of the WORK array, returns   
            this value as the first entry of the WORK array, and no error   
            message related to LWORK is issued by XERBLA.   

    IWORK   (workspace/output) INTEGER array, dimension (LIWORK)   
            On exit, if INFO = 0, IWORK(1) returns the optimal LIWORK.   

    LIWORK  (input) INTEGER   
            The dimension of the array IWORK.   
            If COMPZ = 'N' or N <= 1 then LIWORK must be at least 1.   
            If COMPZ = 'V' and N > 1 then LIWORK must be at least   
                           ( 6 + 6*N + 5*N*lg N ).   
            If COMPZ = 'I' and N > 1 then LIWORK must be at least   
                           ( 3 + 5*N ).   

            If LIWORK = -1, then a workspace query is assumed; the   
            routine only calculates the optimal size of the IWORK array,   
            returns this value as the first entry of the IWORK array, and   
            no error message related to LIWORK is issued by XERBLA.   

    INFO    (output) INTEGER   
            = 0:  successful exit.   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   
            > 0:  The algorithm failed to compute an eigenvalue while   
                  working on the submatrix lying in rows and columns   
                  INFO/(N+1) through mod(INFO,N+1).   

    Further Details   
    ===============   

    Based on contributions by   
       Jeff Rutter, Computer Science Division, University of California   
       at Berkeley, USA   
    Modified by Francoise Tisseur, University of Tennessee.   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__2 = 2;
    static integer c__9 = 9;
    static integer c__0 = 0;
    static doublereal c_b18 = 0.;
    static doublereal c_b19 = 1.;
    static integer c__1 = 1;
    
    /* System generated locals */
    integer z_dim1, z_offset, i__1, i__2;
    doublereal d__1, d__2;
    /* Builtin functions */
    double log(doublereal);
    integer pow_ii(integer *, integer *);
    double sqrt(doublereal);
    /* Local variables */
    static doublereal tiny;
    static integer i__, j, k, m;
    static doublereal p;
    extern /* Subroutine */ int dgemm_(char *, char *, integer *, integer *, 
	    integer *, doublereal *, doublereal *, integer *, doublereal *, 
	    integer *, doublereal *, doublereal *, integer *);
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int dswap_(integer *, doublereal *, integer *, 
	    doublereal *, integer *);
    static integer lwmin;
    extern /* Subroutine */ int dlaed0_(integer *, integer *, integer *, 
	    doublereal *, doublereal *, doublereal *, integer *, doublereal *,
	     integer *, doublereal *, integer *, integer *);
    static integer start, ii;
    extern doublereal dlamch_(char *);
    extern /* Subroutine */ int dlascl_(char *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, integer *, doublereal *, 
	    integer *, integer *), dlacpy_(char *, integer *, integer 
	    *, doublereal *, integer *, doublereal *, integer *), 
	    dlaset_(char *, integer *, integer *, doublereal *, doublereal *, 
	    doublereal *, integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen);
    extern /* Subroutine */ int xerbla_(char *, integer *);
    extern doublereal dlanst_(char *, integer *, doublereal *, doublereal *);
    extern /* Subroutine */ int dsterf_(integer *, doublereal *, doublereal *,
	     integer *), dlasrt_(char *, integer *, doublereal *, integer *);
    static integer liwmin, icompz;
    extern /* Subroutine */ int dsteqr_(char *, integer *, doublereal *, 
	    doublereal *, doublereal *, integer *, doublereal *, integer *);
    static doublereal orgnrm;
    static logical lquery;
    static integer smlsiz, dtrtrw, storez, end, lgn;
    static doublereal eps;
#define z___ref(a_1,a_2) z__[(a_2)*z_dim1 + a_1]


    --d__;
    --e;
    z_dim1 = *ldz;
    z_offset = 1 + z_dim1 * 1;
    z__ -= z_offset;
    --work;
    --iwork;

    /* Function Body */
    *info = 0;
    lquery = *lwork == -1 || *liwork == -1;

    if (lsame_(compz, "N")) {
	icompz = 0;
    } else if (lsame_(compz, "V")) {
	icompz = 1;
    } else if (lsame_(compz, "I")) {
	icompz = 2;
    } else {
	icompz = -1;
    }
    if (*n <= 1 || icompz <= 0) {
	liwmin = 1;
	lwmin = 1;
    } else {
	lgn = (integer) (log((doublereal) (*n)) / log(2.));
	if (pow_ii(&c__2, &lgn) < *n) {
	    ++lgn;
	}
	if (pow_ii(&c__2, &lgn) < *n) {
	    ++lgn;
	}
	if (icompz == 1) {
/* Computing 2nd power */
	    i__1 = *n;
	    lwmin = *n * 3 + 1 + (*n << 1) * lgn + i__1 * i__1 * 3;
	    liwmin = *n * 6 + 6 + *n * 5 * lgn;
	} else if (icompz == 2) {
/* Computing 2nd power */
	    i__1 = *n;
	    lwmin = (*n << 2) + 1 + i__1 * i__1;
	    liwmin = *n * 5 + 3;
	}
    }
    if (icompz < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*ldz < 1 || icompz > 0 && *ldz < max(1,*n)) {
	*info = -6;
    } else if (*lwork < lwmin && ! lquery) {
	*info = -8;
    } else if (*liwork < liwmin && ! lquery) {
	*info = -10;
    }

    if (*info == 0) {
	work[1] = (doublereal) lwmin;
	iwork[1] = liwmin;
    }

    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DSTEDC", &i__1);
	return 0;
    } else if (lquery) {
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	return 0;
    }
    if (*n == 1) {
	if (icompz != 0) {
	    z___ref(1, 1) = 1.;
	}
	return 0;
    }

    smlsiz = ilaenv_(&c__9, "DSTEDC", " ", &c__0, &c__0, &c__0, &c__0, (
	    ftnlen)6, (ftnlen)1);

/*     If the following conditional clause is removed, then the routine   
       will use the Divide and Conquer routine to compute only the   
       eigenvalues, which requires (3N + 3N**2) real workspace and   
       (2 + 5N + 2N lg(N)) integer workspace.   
       Since on many architectures DSTERF is much faster than any other   
       algorithm for finding eigenvalues only, it is used here   
       as the default.   

       If COMPZ = 'N', use DSTERF to compute the eigenvalues. */

    if (icompz == 0) {
	dsterf_(n, &d__[1], &e[1], info);
	return 0;
    }

/*     If N is smaller than the minimum divide size (SMLSIZ+1), then   
       solve the problem with another solver. */

    if (*n <= smlsiz) {
	if (icompz == 0) {
	    dsterf_(n, &d__[1], &e[1], info);
	    return 0;
	} else if (icompz == 2) {
	    dsteqr_("I", n, &d__[1], &e[1], &z__[z_offset], ldz, &work[1], 
		    info);
	    return 0;
	} else {
	    dsteqr_("V", n, &d__[1], &e[1], &z__[z_offset], ldz, &work[1], 
		    info);
	    return 0;
	}
    }

/*     If COMPZ = 'V', the Z matrix must be stored elsewhere for later   
       use. */

    if (icompz == 1) {
	storez = *n * *n + 1;
    } else {
	storez = 1;
    }

    if (icompz == 2) {
	dlaset_("Full", n, n, &c_b18, &c_b19, &z__[z_offset], ldz);
    }

/*     Scale. */

    orgnrm = dlanst_("M", n, &d__[1], &e[1]);
    if (orgnrm == 0.) {
	return 0;
    }

    eps = dlamch_("Epsilon");

    start = 1;

/*     while ( START <= N ) */

L10:
    if (start <= *n) {

/*     Let END be the position of the next subdiagonal entry such that   
       E( END ) <= TINY or END = N if no such subdiagonal exists.  The   
       matrix identified by the elements between START and END   
       constitutes an independent sub-problem. */

	end = start;
L20:
	if (end < *n) {
	    tiny = eps * sqrt((d__1 = d__[end], abs(d__1))) * sqrt((d__2 = 
		    d__[end + 1], abs(d__2)));
	    if ((d__1 = e[end], abs(d__1)) > tiny) {
		++end;
		goto L20;
	    }
	}

/*        (Sub) Problem determined.  Compute its size and solve it. */

	m = end - start + 1;
	if (m == 1) {
	    start = end + 1;
	    goto L10;
	}
	if (m > smlsiz) {
	    *info = smlsiz;

/*           Scale. */

	    orgnrm = dlanst_("M", &m, &d__[start], &e[start]);
	    dlascl_("G", &c__0, &c__0, &orgnrm, &c_b19, &m, &c__1, &d__[start]
		    , &m, info);
	    i__1 = m - 1;
	    i__2 = m - 1;
	    dlascl_("G", &c__0, &c__0, &orgnrm, &c_b19, &i__1, &c__1, &e[
		    start], &i__2, info);

	    if (icompz == 1) {
		dtrtrw = 1;
	    } else {
		dtrtrw = start;
	    }
	    dlaed0_(&icompz, n, &m, &d__[start], &e[start], &z___ref(dtrtrw, 
		    start), ldz, &work[1], n, &work[storez], &iwork[1], info);
	    if (*info != 0) {
		*info = (*info / (m + 1) + start - 1) * (*n + 1) + *info % (m 
			+ 1) + start - 1;
		return 0;
	    }

/*           Scale back. */

	    dlascl_("G", &c__0, &c__0, &c_b19, &orgnrm, &m, &c__1, &d__[start]
		    , &m, info);

	} else {
	    if (icompz == 1) {

/*     Since QR won't update a Z matrix which is larger than the   
       length of D, we must solve the sub-problem in a workspace and   
       then multiply back into Z. */

		dsteqr_("I", &m, &d__[start], &e[start], &work[1], &m, &work[
			m * m + 1], info);
		dlacpy_("A", n, &m, &z___ref(1, start), ldz, &work[storez], n);
		dgemm_("N", "N", n, &m, &m, &c_b19, &work[storez], ldz, &work[
			1], &m, &c_b18, &z___ref(1, start), ldz);
	    } else if (icompz == 2) {
		dsteqr_("I", &m, &d__[start], &e[start], &z___ref(start, 
			start), ldz, &work[1], info);
	    } else {
		dsterf_(&m, &d__[start], &e[start], info);
	    }
	    if (*info != 0) {
		*info = start * (*n + 1) + end;
		return 0;
	    }
	}

	start = end + 1;
	goto L10;
    }

/*     endwhile   

       If the problem split any number of times, then the eigenvalues   
       will not be properly ordered.  Here we permute the eigenvalues   
       (and the associated eigenvectors) into ascending order. */

    if (m != *n) {
	if (icompz == 0) {

/*        Use Quick Sort */

	    dlasrt_("I", n, &d__[1], info);

	} else {

/*        Use Selection Sort to minimize swaps of eigenvectors */

	    i__1 = *n;
	    for (ii = 2; ii <= i__1; ++ii) {
		i__ = ii - 1;
		k = i__;
		p = d__[i__];
		i__2 = *n;
		for (j = ii; j <= i__2; ++j) {
		    if (d__[j] < p) {
			k = j;
			p = d__[j];
		    }
/* L30: */
		}
		if (k != i__) {
		    d__[k] = d__[i__];
		    d__[i__] = p;
		    dswap_(n, &z___ref(1, i__), &c__1, &z___ref(1, k), &c__1);
		}
/* L40: */
	    }
	}
    }

    work[1] = (doublereal) lwmin;
    iwork[1] = liwmin;

    return 0;

/*     End of DSTEDC */

} /* dstedc_ */

#undef z___ref


