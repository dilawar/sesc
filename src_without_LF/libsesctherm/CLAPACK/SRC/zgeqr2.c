#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zgeqr2_(integer *m, integer *n, doublecomplex *a, 
	integer *lda, doublecomplex *tau, doublecomplex *work, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZGEQR2 computes a QR factorization of a complex m by n matrix A:   
    A = Q * R.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the m by n matrix A.   
            On exit, the elements on and above the diagonal of the array   
            contain the min(m,n) by n upper trapezoidal matrix R (R is   
            upper triangular if m >= n); the elements below the diagonal,   
            with the array TAU, represent the unitary matrix Q as a   
            product of elementary reflectors (see Further Details).   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    TAU     (output) COMPLEX*16 array, dimension (min(M,N))   
            The scalar factors of the elementary reflectors (see Further   
            Details).   

    WORK    (workspace) COMPLEX*16 array, dimension (N)   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The matrix Q is represented as a product of elementary reflectors   

       Q = H(1) H(2) . . . H(k), where k = min(m,n).   

    Each H(i) has the form   

       H(i) = I - tau * v * v'   

    where tau is a complex scalar, and v is a complex vector with   
    v(1:i-1) = 0 and v(i) = 1; v(i+1:m) is stored on exit in A(i+1:m,i),   
    and tau in TAU(i).   

    =====================================================================   


       Test the input arguments   

       Parameter adjustments */
    /* Table of constant values */
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2, i__3;
    doublecomplex z__1;
    /* Builtin functions */
    void d_cnjg(doublecomplex *, doublecomplex *);
    /* Local variables */
    static integer i__, k;
    static doublecomplex alpha;
    extern /* Subroutine */ int zlarf_(char *, integer *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, doublecomplex *, 
	    integer *, doublecomplex *), xerbla_(char *, integer *), zlarfg_(integer *, doublecomplex *, doublecomplex *, 
	    integer *, doublecomplex *);
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]


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
	xerbla_("ZGEQR2", &i__1);
	return 0;
    }

    k = min(*m,*n);

    i__1 = k;
    for (i__ = 1; i__ <= i__1; ++i__) {

/*        Generate elementary reflector H(i) to annihilate A(i+1:m,i)   

   Computing MIN */
	i__2 = i__ + 1;
	i__3 = *m - i__ + 1;
	zlarfg_(&i__3, &a_ref(i__, i__), &a_ref(min(i__2,*m), i__), &c__1, &
		tau[i__]);
	if (i__ < *n) {

/*           Apply H(i)' to A(i:m,i+1:n) from the left */

	    i__2 = a_subscr(i__, i__);
	    alpha.r = a[i__2].r, alpha.i = a[i__2].i;
	    i__2 = a_subscr(i__, i__);
	    a[i__2].r = 1., a[i__2].i = 0.;
	    i__2 = *m - i__ + 1;
	    i__3 = *n - i__;
	    d_cnjg(&z__1, &tau[i__]);
	    zlarf_("Left", &i__2, &i__3, &a_ref(i__, i__), &c__1, &z__1, &
		    a_ref(i__, i__ + 1), lda, &work[1]);
	    i__2 = a_subscr(i__, i__);
	    a[i__2].r = alpha.r, a[i__2].i = alpha.i;
	}
/* L10: */
    }
    return 0;

/*     End of ZGEQR2 */

} /* zgeqr2_ */

#undef a_ref
#undef a_subscr


