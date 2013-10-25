#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int clarf_(char *side, integer *m, integer *n, complex *v, 
	integer *incv, complex *tau, complex *c__, integer *ldc, complex *
	work)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    CLARF applies a complex elementary reflector H to a complex M-by-N   
    matrix C, from either the left or the right. H is represented in the   
    form   

          H = I - tau * v * v'   

    where tau is a complex scalar and v is a complex vector.   

    If tau = 0, then H is taken to be the unit matrix.   

    To apply H' (the conjugate transpose of H), supply conjg(tau) instead   
    tau.   

    Arguments   
    =========   

    SIDE    (input) CHARACTER*1   
            = 'L': form  H * C   
            = 'R': form  C * H   

    M       (input) INTEGER   
            The number of rows of the matrix C.   

    N       (input) INTEGER   
            The number of columns of the matrix C.   

    V       (input) COMPLEX array, dimension   
                       (1 + (M-1)*abs(INCV)) if SIDE = 'L'   
                    or (1 + (N-1)*abs(INCV)) if SIDE = 'R'   
            The vector v in the representation of H. V is not used if   
            TAU = 0.   

    INCV    (input) INTEGER   
            The increment between elements of v. INCV <> 0.   

    TAU     (input) COMPLEX   
            The value tau in the representation of H.   

    C       (input/output) COMPLEX array, dimension (LDC,N)   
            On entry, the M-by-N matrix C.   
            On exit, C is overwritten by the matrix H * C if SIDE = 'L',   
            or C * H if SIDE = 'R'.   

    LDC     (input) INTEGER   
            The leading dimension of the array C. LDC >= max(1,M).   

    WORK    (workspace) COMPLEX array, dimension   
                           (N) if SIDE = 'L'   
                        or (M) if SIDE = 'R'   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static complex c_b1 = {1.f,0.f};
    static complex c_b2 = {0.f,0.f};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer c_dim1, c_offset;
    complex q__1;
    /* Local variables */
    extern /* Subroutine */ int cgerc_(integer *, integer *, complex *, 
	    complex *, integer *, complex *, integer *, complex *, integer *),
	     cgemv_(char *, integer *, integer *, complex *, complex *, 
	    integer *, complex *, integer *, complex *, complex *, integer *);
    extern logical lsame_(char *, char *);


    --v;
    c_dim1 = *ldc;
    c_offset = 1 + c_dim1 * 1;
    c__ -= c_offset;
    --work;

    /* Function Body */
    if (lsame_(side, "L")) {

/*        Form  H * C */

	if (tau->r != 0.f || tau->i != 0.f) {

/*           w := C' * v */

	    cgemv_("Conjugate transpose", m, n, &c_b1, &c__[c_offset], ldc, &
		    v[1], incv, &c_b2, &work[1], &c__1);

/*           C := C - v * w' */

	    q__1.r = -tau->r, q__1.i = -tau->i;
	    cgerc_(m, n, &q__1, &v[1], incv, &work[1], &c__1, &c__[c_offset], 
		    ldc);
	}
    } else {

/*        Form  C * H */

	if (tau->r != 0.f || tau->i != 0.f) {

/*           w := C * v */

	    cgemv_("No transpose", m, n, &c_b1, &c__[c_offset], ldc, &v[1], 
		    incv, &c_b2, &work[1], &c__1);

/*           C := C - w * v' */

	    q__1.r = -tau->r, q__1.i = -tau->i;
	    cgerc_(m, n, &q__1, &work[1], &c__1, &v[1], incv, &c__[c_offset], 
		    ldc);
	}
    }
    return 0;

/*     End of CLARF */

} /* clarf_ */

