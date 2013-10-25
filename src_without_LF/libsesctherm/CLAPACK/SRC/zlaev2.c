#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zlaev2_(doublecomplex *a, doublecomplex *b, 
	doublecomplex *c__, doublereal *rt1, doublereal *rt2, doublereal *cs1,
	 doublecomplex *sn1)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    ZLAEV2 computes the eigendecomposition of a 2-by-2 Hermitian matrix   
       [  A         B  ]   
       [  CONJG(B)  C  ].   
    On return, RT1 is the eigenvalue of larger absolute value, RT2 is the   
    eigenvalue of smaller absolute value, and (CS1,SN1) is the unit right   
    eigenvector for RT1, giving the decomposition   

    [ CS1  CONJG(SN1) ] [    A     B ] [ CS1 -CONJG(SN1) ] = [ RT1  0  ]   
    [-SN1     CS1     ] [ CONJG(B) C ] [ SN1     CS1     ]   [  0  RT2 ].   

    Arguments   
    =========   

    A      (input) COMPLEX*16   
           The (1,1) element of the 2-by-2 matrix.   

    B      (input) COMPLEX*16   
           The (1,2) element and the conjugate of the (2,1) element of   
           the 2-by-2 matrix.   

    C      (input) COMPLEX*16   
           The (2,2) element of the 2-by-2 matrix.   

    RT1    (output) DOUBLE PRECISION   
           The eigenvalue of larger absolute value.   

    RT2    (output) DOUBLE PRECISION   
           The eigenvalue of smaller absolute value.   

    CS1    (output) DOUBLE PRECISION   
    SN1    (output) COMPLEX*16   
           The vector (CS1, SN1) is a unit right eigenvector for RT1.   

    Further Details   
    ===============   

    RT1 is accurate to a few ulps barring over/underflow.   

    RT2 may be inaccurate if there is massive cancellation in the   
    determinant A*C-B*B; higher precision or correctly rounded or   
    correctly truncated arithmetic would be needed to compute RT2   
    accurately in all cases.   

    CS1 and SN1 are accurate to a few ulps barring over/underflow.   

    Overflow is possible only if RT1 is within a factor of 5 of overflow.   
    Underflow is harmless if the input data is 0 or exceeds   
       underflow_threshold / macheps.   

   ===================================================================== */
    /* System generated locals */
    doublereal d__1, d__2, d__3;
    doublecomplex z__1, z__2;
    /* Builtin functions */
    double z_abs(doublecomplex *);
    void d_cnjg(doublecomplex *, doublecomplex *);
    /* Local variables */
    static doublereal t;
    static doublecomplex w;
    extern /* Subroutine */ int dlaev2_(doublereal *, doublereal *, 
	    doublereal *, doublereal *, doublereal *, doublereal *, 
	    doublereal *);



    if (z_abs(b) == 0.) {
	w.r = 1., w.i = 0.;
    } else {
	d_cnjg(&z__2, b);
	d__1 = z_abs(b);
	z__1.r = z__2.r / d__1, z__1.i = z__2.i / d__1;
	w.r = z__1.r, w.i = z__1.i;
    }
    d__1 = a->r;
    d__2 = z_abs(b);
    d__3 = c__->r;
    dlaev2_(&d__1, &d__2, &d__3, rt1, rt2, cs1, &t);
    z__1.r = t * w.r, z__1.i = t * w.i;
    sn1->r = z__1.r, sn1->i = z__1.i;
    return 0;

/*     End of ZLAEV2 */

} /* zlaev2_ */

