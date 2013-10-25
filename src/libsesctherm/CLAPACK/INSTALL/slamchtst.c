#include "f2c.h"

/* Main program */ MAIN__(void)
{
/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992 */
    /* Table of constant values */
    static integer c__9 = 9;
    static integer c__1 = 1;
    static integer c__4 = 4;
    
    /* System generated locals */
    real r__1;
    /* Builtin functions */
    integer s_wsle(cilist *), do_lio(integer *, integer *, char *, ftnlen), 
	    e_wsle(void);
    /* Local variables */
    static real base, emin, prec, emax, rmin, rmax, t, sfmin;
    extern doublereal slamch_(char *);
    static real rnd, eps;
    /* Fortran I/O blocks */
    static cilist io___11 = { 0, 6, 0, 0, 0 };
    static cilist io___12 = { 0, 6, 0, 0, 0 };
    static cilist io___13 = { 0, 6, 0, 0, 0 };
    static cilist io___14 = { 0, 6, 0, 0, 0 };
    static cilist io___15 = { 0, 6, 0, 0, 0 };
    static cilist io___16 = { 0, 6, 0, 0, 0 };
    static cilist io___17 = { 0, 6, 0, 0, 0 };
    static cilist io___18 = { 0, 6, 0, 0, 0 };
    static cilist io___19 = { 0, 6, 0, 0, 0 };
    static cilist io___20 = { 0, 6, 0, 0, 0 };
    static cilist io___21 = { 0, 6, 0, 0, 0 };




    eps = slamch_("Epsilon");
    sfmin = slamch_("Safe minimum");
    base = slamch_("Base");
    prec = slamch_("Precision");
    t = slamch_("Number of digits in mantissa");
    rnd = slamch_("Rounding mode");
    emin = slamch_("Minimum exponent");
    rmin = slamch_("Underflow threshold");
    emax = slamch_("Largest exponent");
    rmax = slamch_("Overflow threshold");

    s_wsle(&io___11);
    do_lio(&c__9, &c__1, " Epsilon                      = ", 32L);
    do_lio(&c__4, &c__1, (char *)&eps, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___12);
    do_lio(&c__9, &c__1, " Safe minimum                 = ", 32L);
    do_lio(&c__4, &c__1, (char *)&sfmin, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___13);
    do_lio(&c__9, &c__1, " Base                         = ", 32L);
    do_lio(&c__4, &c__1, (char *)&base, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___14);
    do_lio(&c__9, &c__1, " Precision                    = ", 32L);
    do_lio(&c__4, &c__1, (char *)&prec, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___15);
    do_lio(&c__9, &c__1, " Number of digits in mantissa = ", 32L);
    do_lio(&c__4, &c__1, (char *)&t, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___16);
    do_lio(&c__9, &c__1, " Rounding mode                = ", 32L);
    do_lio(&c__4, &c__1, (char *)&rnd, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___17);
    do_lio(&c__9, &c__1, " Minimum exponent             = ", 32L);
    do_lio(&c__4, &c__1, (char *)&emin, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___18);
    do_lio(&c__9, &c__1, " Underflow threshold          = ", 32L);
    do_lio(&c__4, &c__1, (char *)&rmin, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___19);
    do_lio(&c__9, &c__1, " Largest exponent             = ", 32L);
    do_lio(&c__4, &c__1, (char *)&emax, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___20);
    do_lio(&c__9, &c__1, " Overflow threshold           = ", 32L);
    do_lio(&c__4, &c__1, (char *)&rmax, (ftnlen)sizeof(real));
    e_wsle();
    s_wsle(&io___21);
    do_lio(&c__9, &c__1, " Reciprocal of safe minimum   = ", 32L);
    r__1 = 1 / sfmin;
    do_lio(&c__4, &c__1, (char *)&r__1, (ftnlen)sizeof(real));
    e_wsle();

    return 0;
} /* MAIN__   

   Main program alias */ int test2_ () { MAIN__ (); return 0; }
