/* dsecndtst.f -- translated by f2c (version 19940927).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;
static integer c__100 = 100;

/* Main program */ MAIN__(void)
{
    /* Format strings */
    static char fmt_9999[] = "(\002 Time for 1,000,000 DAXPY ops  = \002,g10"
	    ".3,\002 seconds\002)";
    static char fmt_9998[] = "(\002 DAXPY performance rate        = \002,g10"
	    ".3,\002 mflops \002)";
    static char fmt_9994[] = "(\002 *** Error:  Time for operations was zer"
	    "o\002)";
    static char fmt_9997[] = "(\002 Including DSECND, time        = \002,g10"
	    ".3,\002 seconds\002)";
    static char fmt_9996[] = "(\002 Average time for DSECND       = \002,g10"
	    ".3,\002 milliseconds\002)";
    static char fmt_9995[] = "(\002 Equivalent floating point ops = \002,g10"
	    ".3,\002 ops\002)";

    /* System generated locals */
    doublereal d__1;

    /* Builtin functions */
    integer s_wsfe(cilist *), do_fio(integer *, char *, ftnlen), e_wsfe(void);

    /* Local variables */
    static integer i, j;
    static doublereal alpha, x[100], y[100];
    extern /* Subroutine */ int mysub_(integer *, doublereal *, doublereal *);
    static doublereal t1, t2;
    extern doublereal dsecnd_(void);
    static doublereal tnosec, avg;

    /* Fortran I/O blocks */
    static cilist io___8 = { 0, 6, 0, fmt_9999, 0 };
    static cilist io___9 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___10 = { 0, 6, 0, fmt_9994, 0 };
    static cilist io___12 = { 0, 6, 0, fmt_9997, 0 };
    static cilist io___14 = { 0, 6, 0, fmt_9996, 0 };
    static cilist io___15 = { 0, 6, 0, fmt_9995, 0 };



/*  -- LAPACK test routine (version 3.0) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     September 30, 1994 */

/*     .. Parameters .. */
/*     .. */
/*     .. Local Scalars .. */
/*     .. */
/*     .. Local Arrays .. */
/*     .. */
/*     .. External Functions .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Executable Statements .. */


/*     Initialize X and Y */

    for (i = 1; i <= 100; ++i) {
	x[i - 1] = 1. / (doublereal) i;
	y[i - 1] = (doublereal) (100 - i) / 100.;
/* L10: */
    }
    alpha = .315;

/*     Time 1,000,000 DAXPY operations */

    t1 = dsecnd_();
    for (j = 1; j <= 5000; ++j) {
	for (i = 1; i <= 100; ++i) {
	    y[i - 1] += alpha * x[i - 1];
/* L20: */
	}
	alpha = -alpha;
/* L30: */
    }
    t2 = dsecnd_();
    s_wsfe(&io___8);
    d__1 = t2 - t1;
    do_fio(&c__1, (char *)&d__1, (ftnlen)sizeof(doublereal));
    e_wsfe();
    if (t2 - t1 > 0.) {
	s_wsfe(&io___9);
	d__1 = 1. / (t2 - t1);
	do_fio(&c__1, (char *)&d__1, (ftnlen)sizeof(doublereal));
	e_wsfe();
    } else {
	s_wsfe(&io___10);
	e_wsfe();
    }
    tnosec = t2 - t1;

/*     Time 1,000,000 DAXPY operations with DSECND in the outer loop */

    t1 = dsecnd_();
    for (j = 1; j <= 5000; ++j) {
	for (i = 1; i <= 100; ++i) {
	    y[i - 1] += alpha * x[i - 1];
/* L40: */
	}
	alpha = -alpha;
	t2 = dsecnd_();
/* L50: */
    }

/*     Compute the time in milliseconds used by an average call */
/*     to DSECND. */

    s_wsfe(&io___12);
    d__1 = t2 - t1;
    do_fio(&c__1, (char *)&d__1, (ftnlen)sizeof(doublereal));
    e_wsfe();
    avg = (t2 - t1 - tnosec) * 1e3 / 5e3;
    s_wsfe(&io___14);
    do_fio(&c__1, (char *)&avg, (ftnlen)sizeof(doublereal));
    e_wsfe();

/*     Compute the equivalent number of floating point operations used */
/*     by an average call to DSECND. */

    if (tnosec > 0.) {
	s_wsfe(&io___15);
	d__1 = avg * 1e3 / tnosec;
	do_fio(&c__1, (char *)&d__1, (ftnlen)sizeof(doublereal));
	e_wsfe();
    }

    mysub_(&c__100, x, y);
    return 0;
} /* MAIN__ */

/* Subroutine */ int mysub_(integer *n, doublereal *x, doublereal *y)
{
    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
    return 0;
} /* mysub_ */

/* Main program alias */ int test5_ () { MAIN__ (); return 0; }
