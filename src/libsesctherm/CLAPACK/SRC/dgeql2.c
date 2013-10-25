#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dgeql2_(integer *m, integer *n, doublereal *a, integer *
	lda, doublereal *tau, doublereal *work, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    DGEQL2 computes a QL factorization of a real m by n matrix A:   
    A = Q * L.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the m by n matrix A.   
            On exit, if m >= n, the lower triangle of the subarray   
            A(m-n+1:m,1:n) contains the n by n lower triangular matrix L;   
            if m <= n, the elements on and below the (n-m)-th   
            superdiagonal contain the m by n lower trapezoidal matrix L;   
            the remaining elements, with the array TAU, represent the   
            orthogonal matrix Q as a product of elementary reflectors   
            (see Further Details).   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))   
            The scalar factors of the elementary reflectors (see Further   
            Details).   

    WORK    (workspace) DOUBLE PRECISION array, dimension (N)   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The matrix Q is represented as a product of elementary reflectors   

       Q = H(k) . . . H(2) H(1), where k = min(m,n).   

    Each H(i) has the form   

       H(i) = I - tau * v * v'   

    where tau is a real scalar, and v is a real vector with   
    v(m-k+i+1:m) = 0 and v(m-k+i) = 1; v(1:m-k+i-1) is stored on exit in   
    A(1:m-k+i-1,n-k+i), and tau in TAU(i).   

    =====================================================================   


       Test the input arguments   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2;
    /* Local variables */
    static integer i__, k;
    extern /* Subroutine */ int dlarf_(char *, integer *, integer *, 
	    doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *), dlarfg_(integer *, doublereal *, 
	    doublereal *, integer *, doublereal *), xerbla_(char *, integer *);
    static doublereal aii;
#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --tau;
    --work;

    /* Function Body */
    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*lda < max(1,*m)) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("DGEQL2", &i__1);
	return 0;
    }

    k = min(*m,*n);

    for (i__ = k; i__ >= 1; --i__) {

/*        Generate elementary reflector H(i) to annihilate   
          A(1:m-k+i-1,n-k+i) */

	i__1 = *m - k + i__;
	dlarfg_(&i__1, &a_ref(*m - k + i__, *n - k + i__), &a_ref(1, *n - k + 
		i__), &c__1, &tau[i__]);

/*        Apply H(i) to A(1:m-k+i,1:n-k+i-1) from the left */

	aii = a_ref(*m - k + i__, *n - k + i__);
	a_ref(*m - k + i__, *n - k + i__) = 1.;
	i__1 = *m - k + i__;
	i__2 = *n - k + i__ - 1;
	dlarf_("Left", &i__1, &i__2, &a_ref(1, *n - k + i__), &c__1, &tau[i__]
		, &a[a_offset], lda, &work[1]);
	a_ref(*m - k + i__, *n - k + i__) = aii;
/* L10: */
    }
    return 0;

/*     End of DGEQL2 */

} /* dgeql2_ */

#undef a_ref


