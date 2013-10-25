#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int ztzrqf_(integer *m, integer *n, doublecomplex *a, 
	integer *lda, doublecomplex *tau, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    This routine is deprecated and has been replaced by routine ZTZRZF.   

    ZTZRQF reduces the M-by-N ( M<=N ) complex upper trapezoidal matrix A   
    to upper triangular form by means of unitary transformations.   

    The upper trapezoidal matrix A is factored as   

       A = ( R  0 ) * Z,   

    where Z is an N-by-N unitary matrix and R is an M-by-M upper   
    triangular matrix.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= M.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the leading M-by-N upper trapezoidal part of the   
            array A must contain the matrix to be factorized.   
            On exit, the leading M-by-M upper triangular part of A   
            contains the upper triangular matrix R, and elements M+1 to   
            N of the first M rows of A, with the array TAU, represent the   
            unitary matrix Z as a product of M elementary reflectors.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    TAU     (output) COMPLEX*16 array, dimension (M)   
            The scalar factors of the elementary reflectors.   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The  factorization is obtained by Householder's method.  The kth   
    transformation matrix, Z( k ), whose conjugate transpose is used to   
    introduce zeros into the (m - k + 1)th row of A, is given in the form   

       Z( k ) = ( I     0   ),   
                ( 0  T( k ) )   

    where   

       T( k ) = I - tau*u( k )*u( k )',   u( k ) = (   1    ),   
                                                   (   0    )   
                                                   ( z( k ) )   

    tau is a scalar and z( k ) is an ( n - m ) element vector.   
    tau and z( k ) are chosen to annihilate the elements of the kth row   
    of X.   

    The scalar tau is returned in the kth element of TAU and the vector   
    u( k ) in the kth row of A, such that the elements of z( k ) are   
    in  a( k, m + 1 ), ..., a( k, n ). The elements of R are returned in   
    the upper triangular part of A.   

    Z is given by   

       Z =  Z( 1 ) * Z( 2 ) * ... * Z( m ).   

   =====================================================================   


       Test the input parameters.   

       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {1.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2;
    doublecomplex z__1, z__2;
    /* Builtin functions */
    void d_cnjg(doublecomplex *, doublecomplex *);
    /* Local variables */
    static integer i__, k;
    static doublecomplex alpha;
    extern /* Subroutine */ int zgerc_(integer *, integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *);
    static integer m1;
    extern /* Subroutine */ int zcopy_(integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zaxpy_(integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *), xerbla_(
	    char *, integer *), zlarfg_(integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *), zlacgv_(integer *, 
	    doublecomplex *, integer *);
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --tau;

    /* Function Body */
    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < *m) {
	*info = -2;
    } else if (*lda < max(1,*m)) {
	*info = -4;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZTZRQF", &i__1);
	return 0;
    }

/*     Perform the factorization. */

    if (*m == 0) {
	return 0;
    }
    if (*m == *n) {
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    i__2 = i__;
	    tau[i__2].r = 0., tau[i__2].i = 0.;
/* L10: */
	}
    } else {
/* Computing MIN */
	i__1 = *m + 1;
	m1 = min(i__1,*n);
	for (k = *m; k >= 1; --k) {

/*           Use a Householder reflection to zero the kth row of A.   
             First set up the reflection. */

	    i__1 = a_subscr(k, k);
	    d_cnjg(&z__1, &a_ref(k, k));
	    a[i__1].r = z__1.r, a[i__1].i = z__1.i;
	    i__1 = *n - *m;
	    zlacgv_(&i__1, &a_ref(k, m1), lda);
	    i__1 = a_subscr(k, k);
	    alpha.r = a[i__1].r, alpha.i = a[i__1].i;
	    i__1 = *n - *m + 1;
	    zlarfg_(&i__1, &alpha, &a_ref(k, m1), lda, &tau[k]);
	    i__1 = a_subscr(k, k);
	    a[i__1].r = alpha.r, a[i__1].i = alpha.i;
	    i__1 = k;
	    d_cnjg(&z__1, &tau[k]);
	    tau[i__1].r = z__1.r, tau[i__1].i = z__1.i;

	    i__1 = k;
	    if ((tau[i__1].r != 0. || tau[i__1].i != 0.) && k > 1) {

/*              We now perform the operation  A := A*P( k )'.   

                Use the first ( k - 1 ) elements of TAU to store  a( k ),   
                where  a( k ) consists of the first ( k - 1 ) elements of   
                the  kth column  of  A.  Also  let  B  denote  the  first   
                ( k - 1 ) rows of the last ( n - m ) columns of A. */

		i__1 = k - 1;
		zcopy_(&i__1, &a_ref(1, k), &c__1, &tau[1], &c__1);

/*              Form   w = a( k ) + B*z( k )  in TAU. */

		i__1 = k - 1;
		i__2 = *n - *m;
		zgemv_("No transpose", &i__1, &i__2, &c_b1, &a_ref(1, m1), 
			lda, &a_ref(k, m1), lda, &c_b1, &tau[1], &c__1);

/*              Now form  a( k ) := a( k ) - conjg(tau)*w   
                and       B      := B      - conjg(tau)*w*z( k )'. */

		i__1 = k - 1;
		d_cnjg(&z__2, &tau[k]);
		z__1.r = -z__2.r, z__1.i = -z__2.i;
		zaxpy_(&i__1, &z__1, &tau[1], &c__1, &a_ref(1, k), &c__1);
		i__1 = k - 1;
		i__2 = *n - *m;
		d_cnjg(&z__2, &tau[k]);
		z__1.r = -z__2.r, z__1.i = -z__2.i;
		zgerc_(&i__1, &i__2, &z__1, &tau[1], &c__1, &a_ref(k, m1), 
			lda, &a_ref(1, m1), lda);
	    }
/* L20: */
	}
    }

    return 0;

/*     End of ZTZRQF */

} /* ztzrqf_ */

#undef a_ref
#undef a_subscr


