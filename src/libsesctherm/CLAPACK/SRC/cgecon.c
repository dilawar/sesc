#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int cgecon_(char *norm, integer *n, complex *a, integer *lda,
	 real *anorm, real *rcond, complex *work, real *rwork, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       March 31, 1993   


    Purpose   
    =======   

    CGECON estimates the reciprocal of the condition number of a general   
    complex matrix A, in either the 1-norm or the infinity-norm, using   
    the LU factorization computed by CGETRF.   

    An estimate is obtained for norm(inv(A)), and the reciprocal of the   
    condition number is computed as   
       RCOND = 1 / ( norm(A) * norm(inv(A)) ).   

    Arguments   
    =========   

    NORM    (input) CHARACTER*1   
            Specifies whether the 1-norm condition number or the   
            infinity-norm condition number is required:   
            = '1' or 'O':  1-norm;   
            = 'I':         Infinity-norm.   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    A       (input) COMPLEX array, dimension (LDA,N)   
            The factors L and U from the factorization A = P*L*U   
            as computed by CGETRF.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,N).   

    ANORM   (input) REAL   
            If NORM = '1' or 'O', the 1-norm of the original matrix A.   
            If NORM = 'I', the infinity-norm of the original matrix A.   

    RCOND   (output) REAL   
            The reciprocal of the condition number of the matrix A,   
            computed as RCOND = 1/(norm(A) * norm(inv(A))).   

    WORK    (workspace) COMPLEX array, dimension (2*N)   

    RWORK   (workspace) REAL array, dimension (2*N)   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1;
    real r__1, r__2;
    /* Builtin functions */
    double r_imag(complex *);
    /* Local variables */
    static integer kase, kase1;
    static real scale;
    extern logical lsame_(char *, char *);
    static real sl;
    extern /* Subroutine */ int clacon_(integer *, complex *, complex *, real 
	    *, integer *);
    static integer ix;
    extern integer icamax_(integer *, complex *, integer *);
    extern doublereal slamch_(char *);
    static real su;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static real ainvnm;
    extern /* Subroutine */ int clatrs_(char *, char *, char *, char *, 
	    integer *, complex *, integer *, complex *, real *, real *, 
	    integer *), csrscl_(integer *, 
	    real *, complex *, integer *);
    static logical onenrm;
    static char normin[1];
    static real smlnum;


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --work;
    --rwork;

    /* Function Body */
    *info = 0;
    onenrm = *(unsigned char *)norm == '1' || lsame_(norm, "O");
    if (! onenrm && ! lsame_(norm, "I")) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*n)) {
	*info = -4;
    } else if (*anorm < 0.f) {
	*info = -5;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("CGECON", &i__1);
	return 0;
    }

/*     Quick return if possible */

    *rcond = 0.f;
    if (*n == 0) {
	*rcond = 1.f;
	return 0;
    } else if (*anorm == 0.f) {
	return 0;
    }

    smlnum = slamch_("Safe minimum");

/*     Estimate the norm of inv(A). */

    ainvnm = 0.f;
    *(unsigned char *)normin = 'N';
    if (onenrm) {
	kase1 = 1;
    } else {
	kase1 = 2;
    }
    kase = 0;
L10:
    clacon_(n, &work[*n + 1], &work[1], &ainvnm, &kase);
    if (kase != 0) {
	if (kase == kase1) {

/*           Multiply by inv(L). */

	    clatrs_("Lower", "No transpose", "Unit", normin, n, &a[a_offset], 
		    lda, &work[1], &sl, &rwork[1], info);

/*           Multiply by inv(U). */

	    clatrs_("Upper", "No transpose", "Non-unit", normin, n, &a[
		    a_offset], lda, &work[1], &su, &rwork[*n + 1], info);
	} else {

/*           Multiply by inv(U'). */

	    clatrs_("Upper", "Conjugate transpose", "Non-unit", normin, n, &a[
		    a_offset], lda, &work[1], &su, &rwork[*n + 1], info);

/*           Multiply by inv(L'). */

	    clatrs_("Lower", "Conjugate transpose", "Unit", normin, n, &a[
		    a_offset], lda, &work[1], &sl, &rwork[1], info);
	}

/*        Divide X by 1/(SL*SU) if doing so will not cause overflow. */

	scale = sl * su;
	*(unsigned char *)normin = 'Y';
	if (scale != 1.f) {
	    ix = icamax_(n, &work[1], &c__1);
	    i__1 = ix;
	    if (scale < ((r__1 = work[i__1].r, dabs(r__1)) + (r__2 = r_imag(&
		    work[ix]), dabs(r__2))) * smlnum || scale == 0.f) {
		goto L20;
	    }
	    csrscl_(n, &scale, &work[1], &c__1);
	}
	goto L10;
    }

/*     Compute the estimate of the reciprocal condition number. */

    if (ainvnm != 0.f) {
	*rcond = 1.f / ainvnm / *anorm;
    }

L20:
    return 0;

/*     End of CGECON */

} /* cgecon_ */

