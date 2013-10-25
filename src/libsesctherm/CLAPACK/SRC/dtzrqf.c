#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dtzrqf_(integer *m, integer *n, doublereal *a, integer *
	lda, doublereal *tau, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    This routine is deprecated and has been replaced by routine DTZRZF.   

    DTZRQF reduces the M-by-N ( M<=N ) real upper trapezoidal matrix A   
    to upper triangular form by means of orthogonal transformations.   

    The upper trapezoidal matrix A is factored as   

       A = ( R  0 ) * Z,   

    where Z is an N-by-N orthogonal matrix and R is an M-by-M upper   
    triangular matrix.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= M.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the leading M-by-N upper trapezoidal part of the   
            array A must contain the matrix to be factorized.   
            On exit, the leading M-by-M upper triangular part of A   
            contains the upper triangular matrix R, and elements M+1 to   
            N of the first M rows of A, with the array TAU, represent the   
            orthogonal matrix Z as a product of M elementary reflectors.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    TAU     (output) DOUBLE PRECISION array, dimension (M)   
            The scalar factors of the elementary reflectors.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    Further Details   
    ===============   

    The factorization is obtained by Householder's method.  The kth   
    transformation matrix, Z( k ), which is used to introduce zeros into   
    the ( m - k + 1 )th row of A, is given in the form   

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
    static integer c__1 = 1;
    static doublereal c_b8 = 1.;
    
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2;
    doublereal d__1;
    /* Local variables */
    extern /* Subroutine */ int dger_(integer *, integer *, doublereal *, 
	    doublereal *, integer *, doublereal *, integer *, doublereal *, 
	    integer *);
    static integer i__, k;
    extern /* Subroutine */ int dgemv_(char *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, doublereal *, integer *, 
	    doublereal *, doublereal *, integer *), dcopy_(integer *, 
	    doublereal *, integer *, doublereal *, integer *), daxpy_(integer 
	    *, doublereal *, doublereal *, integer *, doublereal *, integer *)
	    ;
    static integer m1;
    extern /* Subroutine */ int dlarfg_(integer *, doublereal *, doublereal *,
	     integer *, doublereal *), xerbla_(char *, integer *);
#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]


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
	xerbla_("DTZRQF", &i__1);
	return 0;
    }

/*     Perform the factorization. */

    if (*m == 0) {
	return 0;
    }
    if (*m == *n) {
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    tau[i__] = 0.;
/* L10: */
	}
    } else {
/* Computing MIN */
	i__1 = *m + 1;
	m1 = min(i__1,*n);
	for (k = *m; k >= 1; --k) {

/*           Use a Householder reflection to zero the kth row of A.   
             First set up the reflection. */

	    i__1 = *n - *m + 1;
	    dlarfg_(&i__1, &a_ref(k, k), &a_ref(k, m1), lda, &tau[k]);

	    if (tau[k] != 0. && k > 1) {

/*              We now perform the operation  A := A*P( k ).   

                Use the first ( k - 1 ) elements of TAU to store  a( k ),   
                where  a( k ) consists of the first ( k - 1 ) elements of   
                the  kth column  of  A.  Also  let  B  denote  the  first   
                ( k - 1 ) rows of the last ( n - m ) columns of A. */

		i__1 = k - 1;
		dcopy_(&i__1, &a_ref(1, k), &c__1, &tau[1], &c__1);

/*              Form   w = a( k ) + B*z( k )  in TAU. */

		i__1 = k - 1;
		i__2 = *n - *m;
		dgemv_("No transpose", &i__1, &i__2, &c_b8, &a_ref(1, m1), 
			lda, &a_ref(k, m1), lda, &c_b8, &tau[1], &c__1);

/*              Now form  a( k ) := a( k ) - tau*w   
                and       B      := B      - tau*w*z( k )'. */

		i__1 = k - 1;
		d__1 = -tau[k];
		daxpy_(&i__1, &d__1, &tau[1], &c__1, &a_ref(1, k), &c__1);
		i__1 = k - 1;
		i__2 = *n - *m;
		d__1 = -tau[k];
		dger_(&i__1, &i__2, &d__1, &tau[1], &c__1, &a_ref(k, m1), lda,
			 &a_ref(1, m1), lda);
	    }
/* L20: */
	}
    }

    return 0;

/*     End of DTZRQF */

} /* dtzrqf_ */

#undef a_ref


