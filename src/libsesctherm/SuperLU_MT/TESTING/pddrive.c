#include <string.h>
#include "pdsp_defs.h"

#define NTESTS    5    /* Number of test types */
#define NTYPES    11   /* Number of matrix types */
#define NTRAN     2    
#define THRESH    40.0
#define FMT1      "%10s:n=%d, test(%d)=%12.5g\n"
#define	FMT2      "%10s:fact=%d, trans=%d, refact=%d, equed=%d, n=%d, imat=%d, test(%d)=%12.5g\n"
#define FMT3      "%10s:info=%d, izero=%d, n=%d, nrhs=%d, imat=%d, nfail=%d\n"


main(int argc, char *argv[])
{
/*
 * -- SuperLU MT routine (version 1.0) --
 * Univ. of California Berkeley, Xerox Palo Alto Research Center,
 * and Lawrence Berkeley National Lab.
 * August 15, 1997
 *
 * Purpose
 * =======
 *
 * pddrive() is the main test program for the DOUBLE linear 
 * equation driver routines pdgssv() and pdgssvx().
 * 
 * The program is invoked by a shell script file -- pdtest.csh.
 * The output from the tests are written into a file -- pdtest.out.
 *
 * =====================================================================
 */
    double         *a, *a_save;
    int            *asub, *asub_save;
    int            *xa, *xa_save;
    SuperMatrix    A, B, X, L, U;
    SuperMatrix    ASAV, AC;
    int            *perm_r; /* row permutation from partial pivoting */
    int            *perm_c, *pc_save; /* column permutation */
    int            *etree;
    double         zero = 0.0;
    double         *R, *C;
    double         *ferr, *berr;
    double         *rwork;
    double	   *wwork;
    double         diag_pivot_thresh, drop_tol;
    void           *work;
    int            info, lwork, nrhs, panel_size, relax;
    int            nprocs, m, n, nnz;
    double         *xact;
    double         *rhsb, *solx, *bsav;
    int            ldb, ldx;
    double         rpg, rcond;
    int            i, j, k1;
    double         rowcnd, colcnd, amax;
    int            maxsuper, rowblk, colblk;
    int            prefact, dofact, equil, iequed, norefact;
    int            nt, nrun, nfail, nerrs, imat, fimat, nimat;
    int            nfact, ifact, nrefact, irefact, nusepr, iusepr, itran;
    int            kl, ku, mode, lda;
    int            zerot, izero, ioff;
    double         anorm, cndnum;
    double         *Afull;
    double         result[NTESTS];
    Gstat_t        Gstat;
    pdgstrf_options_t pdgstrf_options;
    superlu_memusage_t superlu_memusage;
    char matrix_type[8];
    char path[3], sym[1], dist[1];
    fact_t fact;
    trans_t trans;
    equed_t equed;
    yes_no_t refact, usepr;
    void parse_command_line();

    /* Fixed set of parameters */
    int      iseed[]  = {1994, 1995, 1996, 1997};
    equed_t  equeds[] = {NOEQUIL, ROW, COL, BOTH};
    fact_t   facts[]  = {FACTORED, DOFACT, EQUILIBRATE};
    yes_no_t refacts[]= {YES, NO};
    trans_t  transs[] = {NOTRANS, TRANS};
    yes_no_t useprs[] = {YES, NO};

    /* Some function prototypes */
    extern int pdgst01(int, int, SuperMatrix *, SuperMatrix *, 
		       SuperMatrix *, int *, double *);
    extern int pdgst02(trans_t, int, int, int, SuperMatrix *, double *,
		       int, double *, int, double *resid);
    extern int pdgst04(int, int, double *, int, 
		       double *, int, double rcond, double *resid);
    extern int pdgst07(trans_t, int, int, SuperMatrix *, double *, int,
		       double *, int, double *, int, 
		       double *, double *, double *);
    extern int dlatb4_(char *, int *, int *, int *, char *, int *, int *, 
	               double *, int *, double *, char *);
    extern int dlatms_(int *, int *, char *, int *, char *, double *d,
                       int *, double *, double *, int *, int *,
                       char *, double *, int *, double *, int *);
    extern int sp_dconvert(int, int, double *, int, int, int,
	                   double *a, int *, int *, int *);


    /* Executable statements */

    strcpy(path, "DGE");
    nrun  = 0;
    nfail = 0;
    nerrs = 0;

    /* Defaults */
    nprocs     = 1;
    n          = 1;
    nrhs       = 1;
    panel_size = sp_ienv(1);
    relax      = sp_ienv(2);
    diag_pivot_thresh = 1.0;
    usepr = NO;
    drop_tol = 0.0;
    work     = NULL;
    lwork    = 0;
    strcpy(matrix_type, "LA");
    parse_command_line(argc, argv, matrix_type, &nprocs, &n,
		       &panel_size, &relax, &nrhs, &maxsuper,
		       &rowblk, &colblk, &lwork);
    if ( lwork > 0 ) {
	work = SUPERLU_MALLOC(lwork);
	if ( !work ) {
	    fprintf(stderr, "expert: cannot allocate %d bytes\n", lwork);
	    exit (-1);
	}
	bzero(work, lwork);
    }

#if ( DEBUGlevel>=1 )
    printf("n = %4d, nprocs = %4d, w = %4d, relax = %4d\n",
	   n, nprocs, panel_size, relax);
#endif

    if ( strcmp(matrix_type, "LA") == 0 ) {
	/* Test LAPACK matrix suite. */
	m = n;
	lda = MAX(n, 1);
	nnz = n * n;        /* upper bound */
	fimat = 1;
	nimat = NTYPES;
	Afull = doubleCalloc(lda * n);
	dallocateA(n, nnz, &a, &asub, &xa);
    } else {
	/* Read a sparse matrix */
	fimat = nimat = 0;
	dreadhb(&m, &n, &nnz, &a, &asub, &xa);
    }

    dallocateA(n, nnz, &a_save, &asub_save, &xa_save);
    rhsb = doubleMalloc(m * nrhs);
    bsav = doubleMalloc(m * nrhs);
    solx = doubleMalloc(n * nrhs);
    ldb  = m;
    ldx  = n;
    dCreate_Dense_Matrix(&B, m, nrhs, rhsb, ldb, SLU_DN, SLU_D, SLU_GE);
    dCreate_Dense_Matrix(&X, n, nrhs, solx, ldx, SLU_DN, SLU_D, SLU_GE);
    xact = doubleMalloc(n * nrhs);
    etree   = intMalloc(n);
    perm_c  = intMalloc(n);
    perm_r  = intMalloc(n);
    pc_save = intMalloc(n);
    R       = (double *) SUPERLU_MALLOC(m*sizeof(double));
    C       = (double *) SUPERLU_MALLOC(n*sizeof(double));
    ferr    = (double *) SUPERLU_MALLOC(nrhs*sizeof(double));
    berr    = (double *) SUPERLU_MALLOC(nrhs*sizeof(double));
    j = MAX(m,n) * MAX(4,nrhs);    
    rwork   = (double *) SUPERLU_MALLOC(j*sizeof(double));
    for (i = 0; i < j; ++i) rwork[i] = 0.;
    if ( !R ) ABORT("SUPERLU_MALLOC fails for R");
    if ( !C ) ABORT("SUPERLU_MALLOC fails for C");
    if ( !ferr ) ABORT("SUPERLU_MALLOC fails for ferr");
    if ( !berr ) ABORT("SUPERLU_MALLOC fails for berr");
    if ( !rwork ) ABORT("SUPERLU_MALLOC fails for rwork");
    wwork = doubleCalloc( MAX(m,n) * MAX(4,nrhs) );

    /* Fill in options used by the factorization routine. */
    pdgstrf_options.panel_size = panel_size;
    pdgstrf_options.relax = relax;
    pdgstrf_options.diag_pivot_thresh = diag_pivot_thresh;
    pdgstrf_options.drop_tol = drop_tol;
    pdgstrf_options.perm_c = perm_c;
    pdgstrf_options.perm_r = perm_r;
    pdgstrf_options.work = work;
    pdgstrf_options.lwork = lwork;

    for (i = 0; i < n; ++i) perm_c[i] = pc_save[i] = i;

    /* All matrix types */
    for (imat = fimat; imat <= nimat; ++imat) {
	
	if ( imat ) {

	    /* Skip types 5, 6, or 7 if the matrix size is too small. */
	    zerot = (imat >= 5 && imat <= 7);
	    if ( zerot && n < imat-4 )
		continue;
	    
	    /* Set up parameters with DLATB4 and generate a test matrix
	       with DLATMS.  */
	    dlatb4_(path, &imat, &n, &n, sym, &kl, &ku, &anorm, &mode,
		    &cndnum, dist);

	    dlatms_(&n, &n, dist, iseed, sym, &rwork[0], &mode, &cndnum,
		    &anorm, &kl, &ku, "No packing", Afull, &lda,
		    &wwork[0], &info);

	    if ( info ) {
		printf(FMT3, "DLATMS", info, izero, n, nrhs, imat, nfail);
		continue;
	    }

	    /* For types 5-7, zero one or more columns of the matrix
	       to test that INFO is returned correctly.   */
	    if ( zerot ) {
		if ( imat == 5 ) izero = 1;
		else if ( imat == 6 ) izero = n;
		else izero = n / 2 + 1;
		ioff = (izero - 1) * lda;
		if ( imat < 7 ) {
		    for (i = 0; i < n; ++i) Afull[ioff + i] = zero;
		} else {
		    for (j = 0; j < n - izero + 1; ++j)
			for (i = 0; i < n; ++i)
			    Afull[ioff + i + j*lda] = zero;
		}
	    } else {
		izero = 0;
	    }

	    /* Convert to sparse representation. */
	    sp_dconvert(n, n, Afull, lda, kl, ku, a, asub, xa, &nnz);

	} else {
	    izero = 0;
	    zerot = 0;
	}
	
	dCreate_CompCol_Matrix(&A, m, n, nnz, a, asub, xa, SLU_NC, SLU_D, SLU_GE);
	/*Print_CompCol_NC(&A);*/

	/* Save a copy of matrix A in ASAV */
	dCreate_CompCol_Matrix(&ASAV, m, n, nnz, a_save, asub_save, xa_save,
			       SLU_NC, SLU_D, SLU_GE);
	dCopy_CompCol_Matrix(&A, &ASAV);
	
	/* Form exact solution. */
	dGenXtrue(n, nrhs, xact, ldx);
	
	for (iequed = 0; iequed < 4; ++iequed) {
	    equed = equeds[iequed];
	    if (iequed == 0) nfact = 3;
	    else nfact = 1;

	    for (ifact = 0; ifact < nfact; ++ifact) {
		fact = facts[ifact];
		if (ifact == 0) nrefact = nusepr = 1;
		else nrefact = nusepr = 2;

		for (irefact = 0; irefact < nrefact; ++irefact) {
		    refact = refacts[irefact];
		    for (iusepr = 0; iusepr < nusepr; ++iusepr) {
		        usepr = useprs[iusepr];
			norefact  = ( refact == NO );
			prefact   = ( fact == FACTORED || refact == YES 
				      || usepr == YES );
			dofact    = ( fact == DOFACT );
			equil     = ( fact == EQUILIBRATE );

			/* Restore the matrix A. */
			dCopy_CompCol_Matrix(&ASAV, &A);
			
#if ( DEBUGlevel>=1 )
    printf("imat=%2d, equed=%2d, fact=%2d, refact=%2d, usepr=%2d\n",
	   imat, equed, fact, refact, usepr);
    fflush(stdout);
#endif
			if ( zerot ) {
			    if ( prefact ) continue;
			} else if ( ! dofact ) {
			    if ( equil || iequed ) {
			      /* Compute row and column scale factors to
				 equilibrate matrix A.    */
			        dgsequ(&A, R, C, &rowcnd, &colcnd,&amax,&info);

				/* Force equilibration. */
				if ( !info && n > 0 ) {
				    if ( equed == ROW ) {
					rowcnd = 0.;
					colcnd = 1.;
				    } else if ( equed == COL ) {
					rowcnd = 1.;
					colcnd = 0.;
				    } else if ( equed == BOTH ) {
					rowcnd = 0.;
					colcnd = 0.;
				    }
				}
			
				/* Equilibrate the matrix. */
				dlaqgs(&A, R, C, rowcnd, colcnd, amax, &equed);
			    }
			}
		    
			if ( prefact ) {	/* First time factor */
			
			    StatAlloc(n, nprocs, panel_size, relax, &Gstat);
			    StatInit(n, nprocs, &Gstat);
			
			    /* Initialize options, rreorder the matrix, 
			       obtain the column etree. */
			    if ( usepr ) {
			      pdgstrf_init(nprocs, NO, panel_size, relax,
					   0.1, NO, drop_tol,
					   perm_c, perm_r, work, lwork,
					   &A, &AC, &pdgstrf_options, &Gstat);
			    } else {
			      pdgstrf_init(nprocs, NO, panel_size, relax,
					   diag_pivot_thresh, NO, drop_tol,
					   perm_c, perm_r, work, lwork,
					   &A, &AC, &pdgstrf_options, &Gstat);
			    }

#if ( DEBUGlevel>=1 )
			    printf("Test PDGSTRF\n");
#endif
			    /* Factor the matrix AC. */
			    pdgstrf(&pdgstrf_options, &AC, perm_r, &L, &U, 
				    &Gstat, &info);

			    /* Restore parameters for subsequent factors. */
			    pdgstrf_options.refact = refact;
			    pdgstrf_options.usepr = usepr;
			    pdgstrf_options.diag_pivot_thresh=diag_pivot_thresh;

			    if ( info ) { 
			        printf("** First factor: info %d, equed %d\n",
				       info, equed);
				if ( lwork == -1 ) {
				  printf("** Estimated memory: %d bytes\n",
					 info - n);
				  exit(0);
				}
			    }
	
			    /* Deallocate storage. */
			    Destroy_CompCol_Permuted(&AC);
			    StatFree(&Gstat);
			
			} /* if .. first time factor */
		    
			for (itran = 0; itran < NTRAN; ++itran) {
			    trans = transs[itran];

			    /* Restore the matrix A. */
			    dCopy_CompCol_Matrix(&ASAV, &A);
			
			    /* Set the right hand side. */
			    dFillRHS(trans, nrhs, xact, ldx, &A, &B);
			    dCopy_Dense_Matrix(m, nrhs, rhsb, ldb, bsav, ldb);

			    /*----------------
			     * Test pdgssv
			     *----------------*/
			    if ( dofact && norefact && itran == 0) {
			        /* Not yet factored, and untransposed */
			        dCopy_Dense_Matrix(m, nrhs, rhsb, ldb,
						   solx, ldx);

				pdgssv(nprocs, &A, perm_c, perm_r, &L, &U, 
				       &X, &info);

#if ( DEBUGlevel>=1 )
			    printf("Test PDGSSV: info = %d\n", info);
#endif
				if ( info && info != izero ) {
                                    printf(FMT3, "pdgssv",
					   info, izero, n, nrhs, imat, nfail);
				} else {
                                    /* Reconstruct matrix from factors and
				       compute residual. */
                                    pdgst01(m, n, &A, &L, &U, perm_r,
					    &result[0]);

				    nt = 1;
				    if ( izero == 0 ) {
				        /* Compute residual of the computed
					   solution. */
				        dCopy_Dense_Matrix(m, nrhs, rhsb, ldb,
							   wwork, ldb);
					pdgst02(trans, m, n, nrhs, &A, solx,
						ldx, wwork,ldb, &result[1]);
					nt = 2;
				    }
				
				    /* Print information about the tests that
				       did not pass the threshold.      */
				    for (i = 0; i < nt; ++i) {
				        if ( result[i] >= THRESH ) {
					    printf(FMT1, "pdgssv", n, i,
						   result[i]);
					    ++nfail;
					}
				    }
				    nrun += nt;
				} /* else .. info == 0 */

				/* Restore perm_c. */
				for (i = 0; i < n; ++i) perm_c[i] = pc_save[i];

				if ( lwork == 0 ) {
				  Destroy_SuperNode_SCP(&L);
				  Destroy_CompCol_NCP(&U);
				}
				
			    } /* if .. end of testing pdgssv */

			    /*----------------
			     * Test pdgssvx
			     *----------------*/
			    
			    /* Equilibrate the matrix if fact = FACTORED and
			       equed = ROW, COL or BOTH.   */
			    if ( iequed > 0 && n > 0 ) {
			        dlaqgs(&A, R, C, rowcnd, colcnd, amax, &equed);
			    }
			
			    /* Initialize more options that are changed from
			       iteration to iteration. */
			    pdgstrf_options.nprocs = nprocs;
			    pdgstrf_options.fact = fact;
			    pdgstrf_options.trans = trans;
			    pdgstrf_options.refact = refact;
			    pdgstrf_options.usepr = usepr;

			    /* Solve the system and compute the condition 
			       number and error bounds using pdgssvx.   */
			    pdgssvx(nprocs, &pdgstrf_options, &A, perm_c, 
				    perm_r, &equed, R, C, &L, &U, &B, &X,
				    &rpg, &rcond, ferr, berr, 
				    &superlu_memusage, &info);

#if ( DEBUGlevel>=1 )
			    printf("Test PDGSSVX: info %d\n", info);
#endif
			    if ( info && info != izero ) {
			        printf(FMT3, "pdgssvx",
				       info, izero, n, nrhs, imat, nfail);
				if ( lwork == -1 ) {
				    printf("** Estimated memory: %.0f bytes\n",
					   superlu_memusage.total_needed);
				    exit(0);
				}
			    } else {
			        if ( !prefact ) {
				    /* Reconstruct matrix from factors and
				       compute residual. */
				    pdgst01(m, n, &A, &L, &U, perm_r, 
					    &result[0]);
				    k1 = 0;
				} else {
				  k1 = 1;
				}

				if ( !info ) {
				    /* Compute residual of the computed 
				       solution.*/
				    dCopy_Dense_Matrix(m, nrhs, bsav, ldb,
						       wwork, ldb);
				    pdgst02(trans, m, n, nrhs, &ASAV, solx,
					    ldx, wwork, ldb, &result[1]);

				    /* Check solution from generated exact
				       solution. */
				    pdgst04(n, nrhs, solx, ldx, xact, ldx,
					    rcond, &result[2]);

				    /* Check the error bounds from iterative
				       refinement. */
				    pdgst07(trans, n, nrhs, &ASAV, bsav, ldb,
					    solx, ldx, xact, ldx, ferr, berr,
					    &result[3]);

				    /* Print information about the tests that
				       did not pass the threshold.    */
				    for (i = k1; i < NTESTS; ++i) {
				        if ( result[i] >= THRESH ) {
					  printf(FMT2, "pdgssvx",
						 fact, trans, refact, equed,
						 n, imat, i, result[i]);
					  ++nfail;
					}
				    }
				    nrun += NTESTS;
				} /* if .. info == 0 */
			    } /* else .. end of testing pdgssvx */

			    if ( !prefact ) {
				if ( lwork == 0 ) {
				    Destroy_SuperNode_SCP(&L);
				    Destroy_CompCol_NCP(&U);
				} else if ( lwork > 0 ) {
				    bzero(work, lwork);
				    /*for (i = 0; i < lwork; ++i) 
				        ((char*)work)[i] = 0;*/
				}
			    }
			} /* for itran ... */

			if ( prefact ) {
			    if ( lwork == 0 ) {
			        Destroy_SuperNode_SCP(&L);
				Destroy_CompCol_NCP(&U);
			    } else if ( lwork > 0 ) {
			        bzero(work,lwork);
			        /*for (i = 0; i < lwork; ++i)
				   ((char*)work)[i] = 0;*/
			    }
			}
			if ( refact == YES ) {
			    SUPERLU_FREE(pdgstrf_options.etree);
			    SUPERLU_FREE(pdgstrf_options.colcnt_h);
			    SUPERLU_FREE(pdgstrf_options.part_super_h);
			}
		    } /* for iusepr ... */
		} /* for irefact ... */
	    } /* for ifact ... */
	} /* for iequed ... */
#if 0    
    if ( !info ) {
	PrintPerf(&L, &U, &superlu_memusage, rpg, rcond, ferr, berr, equed);
    }
#endif    
	/* Deallocate some storage to prepare the test of next matrix. */
	Destroy_SuperMatrix_Store(&A);
	Destroy_SuperMatrix_Store(&ASAV);

    } /* for imat ... */

    /* Print a summary of the results. */
    PrintSumm("DGE", nfail, nrun, nerrs);

    SUPERLU_FREE (a);
    SUPERLU_FREE (asub);
    SUPERLU_FREE (xa);
    SUPERLU_FREE (a_save);
    SUPERLU_FREE (asub_save);
    SUPERLU_FREE (xa_save);
    if ( strcmp(matrix_type, "LA") == 0 ) SUPERLU_FREE (Afull);
    SUPERLU_FREE (rhsb);
    SUPERLU_FREE (bsav);
    SUPERLU_FREE (solx);    
    SUPERLU_FREE (xact);
    SUPERLU_FREE (etree);
    SUPERLU_FREE (perm_r);
    SUPERLU_FREE (perm_c);
    SUPERLU_FREE (pc_save);
    SUPERLU_FREE (R);
    SUPERLU_FREE (C);
    SUPERLU_FREE (ferr);
    SUPERLU_FREE (berr);
    SUPERLU_FREE (rwork);
    SUPERLU_FREE (wwork);
    Destroy_SuperMatrix_Store(&B);
    Destroy_SuperMatrix_Store(&X);
    if ( lwork > 0 ) {
	SUPERLU_FREE (work);
	Destroy_SuperMatrix_Store(&L);
	Destroy_SuperMatrix_Store(&U);
    }

    return 0;
}

/*  
 * Parse command line options to get relaxed snode size, panel size, etc.
 */
void
parse_command_line(int argc, char *argv[], char *matrix_type,
		   int *nprocs, int *n, int *w, int *relax, int *nrhs,
		   int *maxsuper, int *rowblk, int *colblk, int *lwork)
{
    int c;
    extern char *optarg;

    while ( (c = getopt(argc, argv, "ht:n:p:w:r:s:m:b:c:l:")) != EOF ) {
	switch (c) {
	  case 'h':
	    printf("Options:\n");
	    printf("\t-t <char*> - matrix type (\"LA\" or \"SP\")\n");
	    printf("\t-p <int> - number of processes\n");
	    printf("\t-w <int> - panel size\n");
	    printf("\t-r <int> - granularity of relaxed supernodes\n");
	    printf("\t-s <int> - number of right-hand sides\n");
	    printf("\t-m <int> - maximum size of a supernode\n");
	    printf("\t-b <int> - row block size in 2D partition\n");
	    printf("\t-c <int> - column block size in 2D partition\n");
	    printf("\t-l <int> - length of work[] array\n");
	    exit(1);
	    break;
	  case 't': strcpy(matrix_type, optarg);
	            break;
	  case 'n': *n = atoi(optarg);
	            break;
	  case 'p': *nprocs = atoi(optarg);
	            break;
	  case 'w': *w = atoi(optarg);
	            break;
	  case 'r': *relax = atoi(optarg); 
	            break;
	  case 's': *nrhs = atoi(optarg); 
	            break;
	  case 'm': *maxsuper = atoi(optarg); 
	            break;
	  case 'b': *rowblk = atoi(optarg); 
	            break;
	  case 'c': *colblk = atoi(optarg); 
	            break;
	  case 'l': *lwork = atoi(optarg); 
	            break;
  	}
    }
}
