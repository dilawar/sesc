#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int sladiv_(real *a, real *b, real *c__, real *d__, real *p, 
	real *q)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       October 31, 1992   


    Purpose   
    =======   

    SLADIV performs complex division in  real arithmetic   

                          a + i*b   
               p + i*q = ---------   
                          c + i*d   

    The algorithm is due to Robert L. Smith and can be found   
    in D. Knuth, The art of Computer Programming, Vol.2, p.195   

    Arguments   
    =========   

    A       (input) REAL   
    B       (input) REAL   
    C       (input) REAL   
    D       (input) REAL   
            The scalars a, b, c, and d in the above expression.   

    P       (output) REAL   
    Q       (output) REAL   
            The scalars p and q in the above expression.   

    ===================================================================== */
    static real e, f;



    if (dabs(*d__) < dabs(*c__)) {
	e = *d__ / *c__;
	f = *c__ + *d__ * e;
	*p = (*a + *b * e) / f;
	*q = (*b - *a * e) / f;
    } else {
	e = *c__ / *d__;
	f = *d__ + *c__ * e;
	*p = (*b + *a * e) / f;
	*q = (-(*a) + *b * e) / f;
    }

    return 0;

/*     End of SLADIV */

} /* sladiv_ */

