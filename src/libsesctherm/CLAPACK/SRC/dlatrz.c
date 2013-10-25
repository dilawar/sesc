#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int dlatrz_(integer *m, integer *n, integer *l, doublereal *
	a, integer *lda, doublereal *tau, doublereal *work)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    DLATRZ factors the M-by-(M+L) real upper trapezoidal matrix   
    [ A1 A2 ] = [ A(1:M,1:M) A(1:M,N-L+1:N) ] as ( R  0 ) * Z, by means   
    of orthogonal transformations.  Z is an (M+L)-by-(M+L) orthogonal   
    matrix and, R and A1 are M-by-M upper triangular matrices.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    L       (input) INTEGER   
            The number of columns of the matrix A containing the   
            meaningful part of the Householder vectors. N-M >= L >= 0.   

    A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)   
            On entry, the leading M-by-N upper trapezoidal part of the   
            array A must contain the matrix to be factorized.   
            On exit, the leading M-by-M upper triangular part of A   
            contains the upper triangular matrix R, and elements N-L+1 to   
            N of the first M rows of A, with the array TAU, represent the   
            orthogonal matrix Z as a product of M elementary reflectors.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    TAU     (output) DOUBLE PRECISION array, dimension (M)   
            The scalar factors of the elementary reflectors.   

    WORK    (workspace) DOUBLE PRECISION array, dimension (M)   

    Further Details   
    ===============   

    Based on contributions by   
      A. Petitet, Computer Science Dept., Univ. of Tenn., Knoxville, USA   

    The factorization is obtained by Householder's method.  The kth   
    transformation matrix, Z( k ), which is used to introduce zeros into   
    the ( m - k + 1 )th row of A, is given in the form   

       Z( k ) = ( I     0   ),   
                ( 0  T( k ) )   

    where   

       T( k ) = I - tau*u( k )*u( k )',   u( k ) = (   1    ),   
                                                   (   0    )   
                                                   ( z( k ) )   

    tau is a scalar and z( k ) is an l element vector. tau and z( k )   
    are chosen to annihilate the elements of the kth row of A2.   

    The scalar tau is returned in the kth element of TAU and the vector   
    u( k ) in the kth row of A2, such that the elements of z( k ) are   
    in  a( k, l + 1 ), ..., a( k, n ). The elements of R are returned in   
    the upper triangular part of A1.   

    Z is given by   

       Z =  Z( 1 ) * Z( 2 ) * ... * Z( m ).   

    =====================================================================   


       Test the input arguments   

       Quick return if possible   

       Parameter adjustments */
    /* System generated locals */
    integer a_dim1, a_offset, i__1, i__2;
    /* Local variables */
    static integer i__;
    extern /* Subroutine */ int dlarz_(char *, integer *, integer *, integer *
	    , doublereal *, integer *, doublereal *, doublereal *, integer *, 
	    doublereal *), dlarfg_(integer *, doublereal *, 
	    doublereal *, integer *, doublereal *);
#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]

    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --tau;
    --work;

    /* Function Body */
    if (*m == 0) {
	return 0;
    } else if (*m == *n) {
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    tau[i__] = 0.;
/* L10: */
	}
	return 0;
    }

    for (i__ = *m; i__ >= 1; --i__) {

/*        Generate elementary reflector H(i) to annihilate   
          [ A(i,i) A(i,n-l+1:n) ] */

	i__1 = *l + 1;
	dlarfg_(&i__1, &a_ref(i__, i__), &a_ref(i__, *n - *l + 1), lda, &tau[
		i__]);

/*        Apply H(i) to A(1:i-1,i:n) from the right */

	i__1 = i__ - 1;
	i__2 = *n - i__ + 1;
	dlarz_("Right", &i__1, &i__2, l, &a_ref(i__, *n - *l + 1), lda, &tau[
		i__], &a_ref(1, i__), lda, &work[1]);

/* L20: */
    }

    return 0;

/*     End of DLATRZ */

} /* dlatrz_ */

#undef a_ref


