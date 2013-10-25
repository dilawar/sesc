#include "f2c.h"

/* Main program */ MAIN__(void)
{
/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   



       Determine the character set. */
    /* Table of constant values */
    static integer c__9 = 9;
    static integer c__1 = 1;
    static integer c__3 = 3;
    
    /* Format strings */
    static char fmt_9999[] = "(\002 *** Error:  LSAME( \002,a1,\002, \002,"
	    "a1,\002) is .FALSE.\002)";
    static char fmt_9998[] = "(\002 *** Error:  LSAME( \002,a1,\002, \002,"
	    "a1,\002) is .TRUE.\002)";
    /* System generated locals */
    integer i__1;
    /* Builtin functions */
    integer s_wsle(cilist *), do_lio(integer *, integer *, char *, ftnlen), 
	    e_wsle(void), s_wsfe(cilist *), do_fio(integer *, char *, ftnlen),
	     e_wsfe(void);
    /* Local variables */
    extern logical lsame_(char *, char *);
    static integer i1, i2;
    /* Fortran I/O blocks */
    static cilist io___3 = { 0, 6, 0, 0, 0 };
    static cilist io___4 = { 0, 6, 0, 0, 0 };
    static cilist io___5 = { 0, 6, 0, fmt_9999, 0 };
    static cilist io___6 = { 0, 6, 0, fmt_9999, 0 };
    static cilist io___7 = { 0, 6, 0, fmt_9999, 0 };
    static cilist io___8 = { 0, 6, 0, fmt_9999, 0 };
    static cilist io___9 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___10 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___11 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___12 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___13 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___14 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___15 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___16 = { 0, 6, 0, fmt_9998, 0 };
    static cilist io___17 = { 0, 6, 0, 0, 0 };



    i1 = 'A';
    i2 = 'a';
    if (i2 - i1 == 32) {
	s_wsle(&io___3);
	do_lio(&c__9, &c__1, " ASCII character set", 20L);
	e_wsle();
    } else {
	s_wsle(&io___4);
	do_lio(&c__9, &c__1, " Non-ASCII character set, IOFF should be ", 41L)
		;
	i__1 = i2 - i1;
	do_lio(&c__3, &c__1, (char *)&i__1, (ftnlen)sizeof(integer));
	e_wsle();
    }

/*     Test LSAME. */

    if (! lsame_("A", "A")) {
	s_wsfe(&io___5);
	do_fio(&c__1, "A", 1L);
	do_fio(&c__1, "A", 1L);
	e_wsfe();
    }
    if (! lsame_("A", "a")) {
	s_wsfe(&io___6);
	do_fio(&c__1, "A", 1L);
	do_fio(&c__1, "a", 1L);
	e_wsfe();
    }
    if (! lsame_("a", "A")) {
	s_wsfe(&io___7);
	do_fio(&c__1, "a", 1L);
	do_fio(&c__1, "A", 1L);
	e_wsfe();
    }
    if (! lsame_("a", "a")) {
	s_wsfe(&io___8);
	do_fio(&c__1, "a", 1L);
	do_fio(&c__1, "a", 1L);
	e_wsfe();
    }
    if (lsame_("A", "B")) {
	s_wsfe(&io___9);
	do_fio(&c__1, "A", 1L);
	do_fio(&c__1, "B", 1L);
	e_wsfe();
    }
    if (lsame_("A", "b")) {
	s_wsfe(&io___10);
	do_fio(&c__1, "A", 1L);
	do_fio(&c__1, "b", 1L);
	e_wsfe();
    }
    if (lsame_("a", "B")) {
	s_wsfe(&io___11);
	do_fio(&c__1, "a", 1L);
	do_fio(&c__1, "B", 1L);
	e_wsfe();
    }
    if (lsame_("a", "b")) {
	s_wsfe(&io___12);
	do_fio(&c__1, "a", 1L);
	do_fio(&c__1, "b", 1L);
	e_wsfe();
    }
    if (lsame_("O", "/")) {
	s_wsfe(&io___13);
	do_fio(&c__1, "O", 1L);
	do_fio(&c__1, "/", 1L);
	e_wsfe();
    }
    if (lsame_("/", "O")) {
	s_wsfe(&io___14);
	do_fio(&c__1, "/", 1L);
	do_fio(&c__1, "O", 1L);
	e_wsfe();
    }
    if (lsame_("o", "/")) {
	s_wsfe(&io___15);
	do_fio(&c__1, "o", 1L);
	do_fio(&c__1, "/", 1L);
	e_wsfe();
    }
    if (lsame_("/", "o")) {
	s_wsfe(&io___16);
	do_fio(&c__1, "/", 1L);
	do_fio(&c__1, "o", 1L);
	e_wsfe();
    }
    s_wsle(&io___17);
    do_lio(&c__9, &c__1, " Tests completed", 16L);
    e_wsle();

    return 0;
} /* MAIN__   

   Main program alias */ int test1_ () { MAIN__ (); return 0; }
