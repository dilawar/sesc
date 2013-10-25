#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int claev2_(complex *a, complex *b, complex *c__, real *rt1, 
	real *rt2, real *cs1, complex *sn1)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    CLAEV2 computes the eigendecomposition of a 2-by-2 Hermitian matrix   
       [  A         B  ]   
       [  CONJG(B)  C  ].   
    On return, RT1 is the eigenvalue of larger absolute value, RT2 is the   
    eigenvalue of smaller absolute value, and (CS1,SN1) is the unit right   
    eigenvector for RT1, giving the decomposition   

    [ CS1  CONJG(SN1) ] [    A     B ] [ CS1 -CONJG(SN1) ] = [ RT1  0  ]   
    [-SN1     CS1     ] [ CONJG(B) C ] [ SN1     CS1     ]   [  0  RT2 ].   

    Arguments   
    =========   

    A      (input) COMPLEX   
           The (1,1) element of the 2-by-2 matrix.   

    B      (input) COMPLEX   
           The (1,2) element and the conjugate of the (2,1) element of   
           the 2-by-2 matrix.   

    C      (input) COMPLEX   
           The (2,2) element of the 2-by-2 matrix.   

    RT1    (output) REAL   
           The eigenvalue of larger absolute value.   

    RT2    (output) REAL   
           The eigenvalue of smaller absolute value.   

    CS1    (output) REAL   
    SN1    (output) COMPLEX   
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
    real r__1, r__2, r__3;
    complex q__1, q__2;
    /* Builtin functions */
    double c_abs(complex *);
    void r_cnjg(complex *, complex *);
    /* Local variables */
    static real t;
    static complex w;
    extern /* Subroutine */ int slaev2_(real *, real *, real *, real *, real *
	    , real *, real *);



    if (c_abs(b) == 0.f) {
	w.r = 1.f, w.i = 0.f;
    } else {
	r_cnjg(&q__2, b);
	r__1 = c_abs(b);
	q__1.r = q__2.r / r__1, q__1.i = q__2.i / r__1;
	w.r = q__1.r, w.i = q__1.i;
    }
    r__1 = a->r;
    r__2 = c_abs(b);
    r__3 = c__->r;
    slaev2_(&r__1, &r__2, &r__3, rt1, rt2, cs1, &t);
    q__1.r = t * w.r, q__1.i = t * w.i;
    sn1->r = q__1.r, sn1->i = q__1.i;
    return 0;

/*     End of CLAEV2 */

} /* claev2_ */

