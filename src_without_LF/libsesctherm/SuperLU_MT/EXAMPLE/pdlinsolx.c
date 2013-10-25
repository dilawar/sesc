/*
 * -- SuperLU MT routine (version 1.0) --
 * Univ. of California Berkeley, Xerox Palo Alto Research Center,
 * and Lawrence Berkeley National Lab.
 * August 15, 1997
 *
 */
#include "pdsp_defs.h"
#include "util.h"

main(int argc, char *argv[])
{
    SuperMatrix A, L, U;
    SuperMatrix B, X;
    NCformat    *Astore;
    SCPformat   *Lstore;
    NCPformat   *Ustore;
    int         nprocs;
    fact_t      fact;
    trans_t     trans;
    yes_no_t    refact, usepr;
    equed_t     equed;
    double      *a;
    int         *asub, *xa;
    int         *perm_c; /* column permutation vector */
    int         *perm_r; /* row permutations from partial pivoting */
    void        *work;
    pdgstrf_options_t pdgstrf_options;
    int         info, lwork, nrhs, ldx, panel_size, relax;
    int         m, n, nnz, permc_spec;
    int         i, firstfact;
    double      *rhsb, *rhsx, *xact;
    double      *R, *C;
    double      *ferr, *berr;
    double      u, drop_tol, rpg, rcond;
    superlu_memusage_t superlu_memusage;
    void parse_command_line();

    /* Default parameters to control factorization. */
    nprocs = 1;
    fact  = DOFACT;
    trans = NOTRANS;
    equed = NOEQUIL;
    refact= NO;
    panel_size = sp_ienv(1);
    relax = sp_ienv(2);
    u     = 1.0;
    usepr = NO;
    drop_tol = 0.0;
    lwork = 0;
    nrhs  = 1;

    /* Command line options to modify default behavior. */
    parse_command_line(argc, argv, &nprocs, &lwork, &panel_size, &relax, 
		       &u, &fact, &trans, &refact, &equed);

    if ( lwork > 0 ) {
	work = SUPERLU_MALLOC(lwork);
	printf("Use work space of size LWORK = %d bytes\n", lwork);
	if ( !work ) {
	    ABORT("DLINSOLX: cannot allocate work[]");
	}
    }

#if ( PRNTlevel==1 )
    cpp_defs();
#endif

#define HB
#if defined( DEN )
    m = n;
    nnz = n * n;
    dband(n, n, nnz, &a, &asub, &xa);
#elif defined( BAND )
    m = n;
    nnz = (2*b+1) * n;
    dband(n, b, nnz, &a, &asub, &xa);
#elif defined( BD )
    nb = 5;
    bs = 200;
    m = n = bs * nb;
    nnz = bs * bs * nb;
    dblockdiag(nb, bs, nnz, &a, &asub, &xa);
#elif defined( HB )
    dreadhb(&m, &n, &nnz, &a, &asub, &xa);
#else    
    dreadmt(&m, &n, &nnz, &a, &asub, &xa);
#endif

    firstfact = (fact == FACTORED || refact == YES);

    dCreate_CompCol_Matrix(&A, m, n, nnz, a, asub, xa, SLU_NC, SLU_D, SLU_GE);
    Astore = A.Store;
    printf("Dimension %dx%d; # nonzeros %d\n", A.nrow, A.ncol, Astore->nnz);
    
    if ( !(rhsb = doubleMalloc(m * nrhs)) ) ABORT("Malloc fails for rhsb[].");
    if ( !(rhsx = doubleMalloc(m * nrhs)) ) ABORT("Malloc fails for rhsx[].");
    dCreate_Dense_Matrix(&B, m, nrhs, rhsb, m, SLU_DN, SLU_D, SLU_GE);
    dCreate_Dense_Matrix(&X, m, nrhs, rhsx, m, SLU_DN, SLU_D, SLU_GE);
    xact = doubleMalloc(n * nrhs);
    ldx = n;
    dGenXtrue(n, nrhs, xact, ldx);
    dFillRHS(trans, nrhs, xact, ldx, &A, &B);
    
    if ( !(perm_r = intMalloc(m)) ) ABORT("Malloc fails for perm_r[].");
    if ( !(perm_c = intMalloc(n)) ) ABORT("Malloc fails for perm_c[].");
    if ( !(R = (double *) SUPERLU_MALLOC(A.nrow * sizeof(double))) ) 
        ABORT("SUPERLU_MALLOC fails for R[].");
    if ( !(C = (double *) SUPERLU_MALLOC(A.ncol * sizeof(double))) )
        ABORT("SUPERLU_MALLOC fails for C[].");
    if ( !(ferr = (double *) SUPERLU_MALLOC(nrhs * sizeof(double))) )
        ABORT("SUPERLU_MALLOC fails for ferr[].");
    if ( !(berr = (double *) SUPERLU_MALLOC(nrhs * sizeof(double))) ) 
        ABORT("SUPERLU_MALLOC fails for berr[].");

    /*
     * Get column permutation vector perm_c[], according to permc_spec:
     *   permc_spec = 0: natural ordering 
     *   permc_spec = 1: minimum degree ordering on structure of A'*A
     *   permc_spec = 2: minimum degree ordering on structure of A'+A
     *   permc_spec = 3: approximate minimum degree for unsymmetric matrices
     */    	
    permc_spec = 1;
    get_perm_c(permc_spec, &A, perm_c);

    pdgstrf_options.nprocs = nprocs;
    pdgstrf_options.fact = fact;
    pdgstrf_options.trans = trans;
    pdgstrf_options.refact = refact;
    pdgstrf_options.panel_size = panel_size;
    pdgstrf_options.relax = relax;
    pdgstrf_options.diag_pivot_thresh = u;
    pdgstrf_options.usepr = usepr;
    pdgstrf_options.drop_tol = drop_tol;
    pdgstrf_options.perm_c = perm_c;
    pdgstrf_options.perm_r = perm_r;
    pdgstrf_options.work = work;
    pdgstrf_options.lwork = lwork;
    
    /* 
     * Solve the system and compute the condition number
     * and error bounds using pdgssvx.
     */
    pdgssvx(nprocs, &pdgstrf_options, &A, perm_c, perm_r,
	    &equed, R, C, &L, &U, &B, &X, &rpg, &rcond,
	    ferr, berr, &superlu_memusage, &info);

    printf("pdgssvx(): info %d\n", info);

    if ( info == 0 || info == n+1 ) {

	printf("Recip. pivot growth = %e\n", rpg);
	printf("Recip. condition number = %e\n", rcond);
	printf("%8s%16s%16s\n", "rhs", "FERR", "BERR");
	for (i = 0; i < nrhs; ++i) {
	    printf("%8d%16e%16e\n", i+1, ferr[i], berr[i]);
	}
	       
        Lstore = (SCPformat *) L.Store;
        Ustore = (NCPformat *) U.Store;
	printf("No of nonzeros in factor L = %d\n", Lstore->nnz);
    	printf("No of nonzeros in factor U = %d\n", Ustore->nnz);
    	printf("No of nonzeros in L+U = %d\n", Lstore->nnz + Ustore->nnz - n);
	printf("L\\U MB %.3f\ttotal MB needed %.3f\texpansions %d\n",
	       superlu_memusage.for_lu/1e6, superlu_memusage.total_needed/1e6,
	       superlu_memusage.expansions);
	     
	fflush(stdout);

    } else if ( info > 0 && lwork == -1 ) {
        printf("** Estimated memory: %d bytes\n", info - n);
    }

    SUPERLU_FREE (rhsb);
    SUPERLU_FREE (rhsx);
    SUPERLU_FREE (xact);
    SUPERLU_FREE (perm_r);
    SUPERLU_FREE (perm_c);
    SUPERLU_FREE (R);
    SUPERLU_FREE (C);
    SUPERLU_FREE (ferr);
    SUPERLU_FREE (berr);
    Destroy_CompCol_Matrix(&A);
    Destroy_SuperMatrix_Store(&B);
    Destroy_SuperMatrix_Store(&X);
    if ( lwork >= 0 ) {
        Destroy_SuperNode_SCP(&L);
        Destroy_CompCol_NCP(&U);
    }
}

/*  
 * Parse command line options.
 */
void
parse_command_line(int argc, char *argv[], int *nprocs, int *lwork, 
		   int *w, int *relax, double *u, fact_t *fact, 
		   trans_t *trans, yes_no_t *refact, equed_t *equed)
{
    int c;
    extern char *optarg;

    while ( (c = getopt(argc, argv, "hp:l:w:x:u:f:t:r:e:")) != EOF ) {
	switch (c) {
	  case 'h':
	    printf("Options:\n");
	    printf("\t-p <int> - number of processes\n");
	    printf("\t-l <int> - length of work[*] array\n");
	    printf("\t-w <int> - panel size\n");
	    printf("\t-x <int> - maximum size of relaxed supernodes\n");
	    printf("\t-u <int> - pivoting threshold\n");
	    printf("\t-f <FACTORED/DOFACT/EQUILIBRATE> - factor control\n");
	    printf("\t-t <NOTRANS/TRANS/CONJ> - transpose or not\n");
	    printf("\t-r <NO/YES> - refactor or not\n");
	    printf("\t-e <NOEQUIL/ROW/COL/BOTH> - equilibrate or not\n");
	    exit(1);
	    break;
	  case 'p': *nprocs = atoi(optarg);
	            break;
	  case 'l': *lwork = atoi(optarg);
	            break;
	  case 'w': *w = atoi(optarg);
	            break;
	  case 'x': *relax = atoi(optarg); 
	            break;
	  case 'u': *u = atof(optarg); 
	            break;
	  case 'f': *fact = (fact_t) atoi(optarg);
	            break;
	  case 't': *trans = (trans_t) atoi(optarg);
	            break;
	  case 'r': *refact = (yes_no_t) atoi(optarg);
	            break;
	  case 'e': *equed = (equed_t) atoi(optarg);
	            break;
	  default: fprintf(stderr, "Invalid command line option.\n");
		   break;
  	}
    }
}



