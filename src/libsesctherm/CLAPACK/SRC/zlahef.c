#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zlahef_(char *uplo, integer *n, integer *nb, integer *kb,
	 doublecomplex *a, integer *lda, integer *ipiv, doublecomplex *w, 
	integer *ldw, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZLAHEF computes a partial factorization of a complex Hermitian   
    matrix A using the Bunch-Kaufman diagonal pivoting method. The   
    partial factorization has the form:   

    A  =  ( I  U12 ) ( A11  0  ) (  I    0   )  if UPLO = 'U', or:   
          ( 0  U22 ) (  0   D  ) ( U12' U22' )   

    A  =  ( L11  0 ) (  D   0  ) ( L11' L21' )  if UPLO = 'L'   
          ( L21  I ) (  0  A22 ) (  0    I   )   

    where the order of D is at most NB. The actual order is returned in   
    the argument KB, and is either NB or NB-1, or N if N <= NB.   
    Note that U' denotes the conjugate transpose of U.   

    ZLAHEF is an auxiliary routine called by ZHETRF. It uses blocked code   
    (calling Level 3 BLAS) to update the submatrix A11 (if UPLO = 'U') or   
    A22 (if UPLO = 'L').   

    Arguments   
    =========   

    UPLO    (input) CHARACTER*1   
            Specifies whether the upper or lower triangular part of the   
            Hermitian matrix A is stored:   
            = 'U':  Upper triangular   
            = 'L':  Lower triangular   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    NB      (input) INTEGER   
            The maximum number of columns of the matrix A that should be   
            factored.  NB should be at least 2 to allow for 2-by-2 pivot   
            blocks.   

    KB      (output) INTEGER   
            The number of columns of A that were actually factored.   
            KB is either NB-1 or NB, or N if N <= NB.   

    A       (input/output) COMPLEX*16 array, dimension (LDA,N)   
            On entry, the Hermitian matrix A.  If UPLO = 'U', the leading   
            n-by-n upper triangular part of A contains the upper   
            triangular part of the matrix A, and the strictly lower   
            triangular part of A is not referenced.  If UPLO = 'L', the   
            leading n-by-n lower triangular part of A contains the lower   
            triangular part of the matrix A, and the strictly upper   
            triangular part of A is not referenced.   
            On exit, A contains details of the partial factorization.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,N).   

    IPIV    (output) INTEGER array, dimension (N)   
            Details of the interchanges and the block structure of D.   
            If UPLO = 'U', only the last KB elements of IPIV are set;   
            if UPLO = 'L', only the first KB elements are set.   

            If IPIV(k) > 0, then rows and columns k and IPIV(k) were   
            interchanged and D(k,k) is a 1-by-1 diagonal block.   
            If UPLO = 'U' and IPIV(k) = IPIV(k-1) < 0, then rows and   
            columns k-1 and -IPIV(k) were interchanged and D(k-1:k,k-1:k)   
            is a 2-by-2 diagonal block.  If UPLO = 'L' and IPIV(k) =   
            IPIV(k+1) < 0, then rows and columns k+1 and -IPIV(k) were   
            interchanged and D(k:k+1,k:k+1) is a 2-by-2 diagonal block.   

    W       (workspace) COMPLEX*16 array, dimension (LDW,NB)   

    LDW     (input) INTEGER   
            The leading dimension of the array W.  LDW >= max(1,N).   

    INFO    (output) INTEGER   
            = 0: successful exit   
            > 0: if INFO = k, D(k,k) is exactly zero.  The factorization   
                 has been completed, but the block diagonal matrix D is   
                 exactly singular.   

    =====================================================================   


       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {1.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer a_dim1, a_offset, w_dim1, w_offset, i__1, i__2, i__3, i__4, i__5;
    doublereal d__1, d__2, d__3, d__4;
    doublecomplex z__1, z__2, z__3, z__4;
    /* Builtin functions */
    double sqrt(doublereal), d_imag(doublecomplex *);
    void d_cnjg(doublecomplex *, doublecomplex *), z_div(doublecomplex *, 
	    doublecomplex *, doublecomplex *);
    /* Local variables */
    static integer imax, jmax, j, k;
    static doublereal t, alpha;
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int zgemm_(char *, char *, integer *, integer *, 
	    integer *, doublecomplex *, doublecomplex *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, doublecomplex *, 
	    integer *);
    static integer kstep;
    extern /* Subroutine */ int zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *);
    static doublereal r1;
    extern /* Subroutine */ int zcopy_(integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zswap_(integer *, doublecomplex *, 
	    integer *, doublecomplex *, integer *);
    static doublecomplex d11, d21, d22;
    static integer jb, jj, kk, jp, kp;
    static doublereal absakk;
    static integer kw;
    extern /* Subroutine */ int zdscal_(integer *, doublereal *, 
	    doublecomplex *, integer *);
    static doublereal colmax;
    extern /* Subroutine */ int zlacgv_(integer *, doublecomplex *, integer *)
	    ;
    extern integer izamax_(integer *, doublecomplex *, integer *);
    static doublereal rowmax;
    static integer kkw;
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]
#define w_subscr(a_1,a_2) (a_2)*w_dim1 + a_1
#define w_ref(a_1,a_2) w[w_subscr(a_1,a_2)]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --ipiv;
    w_dim1 = *ldw;
    w_offset = 1 + w_dim1 * 1;
    w -= w_offset;

    /* Function Body */
    *info = 0;

/*     Initialize ALPHA for use in choosing pivot block size. */

    alpha = (sqrt(17.) + 1.) / 8.;

    if (lsame_(uplo, "U")) {

/*        Factorize the trailing columns of A using the upper triangle   
          of A and working backwards, and compute the matrix W = U12*D   
          for use in updating A11 (note that conjg(W) is actually stored)   

          K is the main loop index, decreasing from N in steps of 1 or 2   

          KW is the column of W which corresponds to column K of A */

	k = *n;
L10:
	kw = *nb + k - *n;

/*        Exit from loop */

	if (k <= *n - *nb + 1 && *nb < *n || k < 1) {
	    goto L30;
	}

/*        Copy column K of A to column KW of W and update it */

	i__1 = k - 1;
	zcopy_(&i__1, &a_ref(1, k), &c__1, &w_ref(1, kw), &c__1);
	i__1 = w_subscr(k, kw);
	i__2 = a_subscr(k, k);
	d__1 = a[i__2].r;
	w[i__1].r = d__1, w[i__1].i = 0.;
	if (k < *n) {
	    i__1 = *n - k;
	    z__1.r = -1., z__1.i = 0.;
	    zgemv_("No transpose", &k, &i__1, &z__1, &a_ref(1, k + 1), lda, &
		    w_ref(k, kw + 1), ldw, &c_b1, &w_ref(1, kw), &c__1);
	    i__1 = w_subscr(k, kw);
	    i__2 = w_subscr(k, kw);
	    d__1 = w[i__2].r;
	    w[i__1].r = d__1, w[i__1].i = 0.;
	}

	kstep = 1;

/*        Determine rows and columns to be interchanged and whether   
          a 1-by-1 or 2-by-2 pivot block will be used */

	i__1 = w_subscr(k, kw);
	absakk = (d__1 = w[i__1].r, abs(d__1));

/*        IMAX is the row-index of the largest off-diagonal element in   
          column K, and COLMAX is its absolute value */

	if (k > 1) {
	    i__1 = k - 1;
	    imax = izamax_(&i__1, &w_ref(1, kw), &c__1);
	    i__1 = w_subscr(imax, kw);
	    colmax = (d__1 = w[i__1].r, abs(d__1)) + (d__2 = d_imag(&w_ref(
		    imax, kw)), abs(d__2));
	} else {
	    colmax = 0.;
	}

	if (max(absakk,colmax) == 0.) {

/*           Column K is zero: set INFO and continue */

	    if (*info == 0) {
		*info = k;
	    }
	    kp = k;
	    i__1 = a_subscr(k, k);
	    i__2 = a_subscr(k, k);
	    d__1 = a[i__2].r;
	    a[i__1].r = d__1, a[i__1].i = 0.;
	} else {
	    if (absakk >= alpha * colmax) {

/*              no interchange, use 1-by-1 pivot block */

		kp = k;
	    } else {

/*              Copy column IMAX to column KW-1 of W and update it */

		i__1 = imax - 1;
		zcopy_(&i__1, &a_ref(1, imax), &c__1, &w_ref(1, kw - 1), &
			c__1);
		i__1 = w_subscr(imax, kw - 1);
		i__2 = a_subscr(imax, imax);
		d__1 = a[i__2].r;
		w[i__1].r = d__1, w[i__1].i = 0.;
		i__1 = k - imax;
		zcopy_(&i__1, &a_ref(imax, imax + 1), lda, &w_ref(imax + 1, 
			kw - 1), &c__1);
		i__1 = k - imax;
		zlacgv_(&i__1, &w_ref(imax + 1, kw - 1), &c__1);
		if (k < *n) {
		    i__1 = *n - k;
		    z__1.r = -1., z__1.i = 0.;
		    zgemv_("No transpose", &k, &i__1, &z__1, &a_ref(1, k + 1),
			     lda, &w_ref(imax, kw + 1), ldw, &c_b1, &w_ref(1, 
			    kw - 1), &c__1);
		    i__1 = w_subscr(imax, kw - 1);
		    i__2 = w_subscr(imax, kw - 1);
		    d__1 = w[i__2].r;
		    w[i__1].r = d__1, w[i__1].i = 0.;
		}

/*              JMAX is the column-index of the largest off-diagonal   
                element in row IMAX, and ROWMAX is its absolute value */

		i__1 = k - imax;
		jmax = imax + izamax_(&i__1, &w_ref(imax + 1, kw - 1), &c__1);
		i__1 = w_subscr(jmax, kw - 1);
		rowmax = (d__1 = w[i__1].r, abs(d__1)) + (d__2 = d_imag(&
			w_ref(jmax, kw - 1)), abs(d__2));
		if (imax > 1) {
		    i__1 = imax - 1;
		    jmax = izamax_(&i__1, &w_ref(1, kw - 1), &c__1);
/* Computing MAX */
		    i__1 = w_subscr(jmax, kw - 1);
		    d__3 = rowmax, d__4 = (d__1 = w[i__1].r, abs(d__1)) + (
			    d__2 = d_imag(&w_ref(jmax, kw - 1)), abs(d__2));
		    rowmax = max(d__3,d__4);
		}

		if (absakk >= alpha * colmax * (colmax / rowmax)) {

/*                 no interchange, use 1-by-1 pivot block */

		    kp = k;
		} else /* if(complicated condition) */ {
		    i__1 = w_subscr(imax, kw - 1);
		    if ((d__1 = w[i__1].r, abs(d__1)) >= alpha * rowmax) {

/*                 interchange rows and columns K and IMAX, use 1-by-1   
                   pivot block */

			kp = imax;

/*                 copy column KW-1 of W to column KW */

			zcopy_(&k, &w_ref(1, kw - 1), &c__1, &w_ref(1, kw), &
				c__1);
		    } else {

/*                 interchange rows and columns K-1 and IMAX, use 2-by-2   
                   pivot block */

			kp = imax;
			kstep = 2;
		    }
		}
	    }

	    kk = k - kstep + 1;
	    kkw = *nb + kk - *n;

/*           Updated column KP is already stored in column KKW of W */

	    if (kp != kk) {

/*              Copy non-updated column KK to column KP */

		i__1 = a_subscr(kp, kp);
		i__2 = a_subscr(kk, kk);
		d__1 = a[i__2].r;
		a[i__1].r = d__1, a[i__1].i = 0.;
		i__1 = kk - 1 - kp;
		zcopy_(&i__1, &a_ref(kp + 1, kk), &c__1, &a_ref(kp, kp + 1), 
			lda);
		i__1 = kk - 1 - kp;
		zlacgv_(&i__1, &a_ref(kp, kp + 1), lda);
		i__1 = kp - 1;
		zcopy_(&i__1, &a_ref(1, kk), &c__1, &a_ref(1, kp), &c__1);

/*              Interchange rows KK and KP in last KK columns of A and W */

		if (kk < *n) {
		    i__1 = *n - kk;
		    zswap_(&i__1, &a_ref(kk, kk + 1), lda, &a_ref(kp, kk + 1),
			     lda);
		}
		i__1 = *n - kk + 1;
		zswap_(&i__1, &w_ref(kk, kkw), ldw, &w_ref(kp, kkw), ldw);
	    }

	    if (kstep == 1) {

/*              1-by-1 pivot block D(k): column KW of W now holds   

                W(k) = U(k)*D(k)   

                where U(k) is the k-th column of U   

                Store U(k) in column k of A */

		zcopy_(&k, &w_ref(1, kw), &c__1, &a_ref(1, k), &c__1);
		i__1 = a_subscr(k, k);
		r1 = 1. / a[i__1].r;
		i__1 = k - 1;
		zdscal_(&i__1, &r1, &a_ref(1, k), &c__1);

/*              Conjugate W(k) */

		i__1 = k - 1;
		zlacgv_(&i__1, &w_ref(1, kw), &c__1);
	    } else {

/*              2-by-2 pivot block D(k): columns KW and KW-1 of W now   
                hold   

                ( W(k-1) W(k) ) = ( U(k-1) U(k) )*D(k)   

                where U(k) and U(k-1) are the k-th and (k-1)-th columns   
                of U */

		if (k > 2) {

/*                 Store U(k) and U(k-1) in columns k and k-1 of A */

		    i__1 = w_subscr(k - 1, kw);
		    d21.r = w[i__1].r, d21.i = w[i__1].i;
		    d_cnjg(&z__2, &d21);
		    z_div(&z__1, &w_ref(k, kw), &z__2);
		    d11.r = z__1.r, d11.i = z__1.i;
		    z_div(&z__1, &w_ref(k - 1, kw - 1), &d21);
		    d22.r = z__1.r, d22.i = z__1.i;
		    z__1.r = d11.r * d22.r - d11.i * d22.i, z__1.i = d11.r * 
			    d22.i + d11.i * d22.r;
		    t = 1. / (z__1.r - 1.);
		    z__2.r = t, z__2.i = 0.;
		    z_div(&z__1, &z__2, &d21);
		    d21.r = z__1.r, d21.i = z__1.i;
		    i__1 = k - 2;
		    for (j = 1; j <= i__1; ++j) {
			i__2 = a_subscr(j, k - 1);
			i__3 = w_subscr(j, kw - 1);
			z__3.r = d11.r * w[i__3].r - d11.i * w[i__3].i, 
				z__3.i = d11.r * w[i__3].i + d11.i * w[i__3]
				.r;
			i__4 = w_subscr(j, kw);
			z__2.r = z__3.r - w[i__4].r, z__2.i = z__3.i - w[i__4]
				.i;
			z__1.r = d21.r * z__2.r - d21.i * z__2.i, z__1.i = 
				d21.r * z__2.i + d21.i * z__2.r;
			a[i__2].r = z__1.r, a[i__2].i = z__1.i;
			i__2 = a_subscr(j, k);
			d_cnjg(&z__2, &d21);
			i__3 = w_subscr(j, kw);
			z__4.r = d22.r * w[i__3].r - d22.i * w[i__3].i, 
				z__4.i = d22.r * w[i__3].i + d22.i * w[i__3]
				.r;
			i__4 = w_subscr(j, kw - 1);
			z__3.r = z__4.r - w[i__4].r, z__3.i = z__4.i - w[i__4]
				.i;
			z__1.r = z__2.r * z__3.r - z__2.i * z__3.i, z__1.i = 
				z__2.r * z__3.i + z__2.i * z__3.r;
			a[i__2].r = z__1.r, a[i__2].i = z__1.i;
/* L20: */
		    }
		}

/*              Copy D(k) to A */

		i__1 = a_subscr(k - 1, k - 1);
		i__2 = w_subscr(k - 1, kw - 1);
		a[i__1].r = w[i__2].r, a[i__1].i = w[i__2].i;
		i__1 = a_subscr(k - 1, k);
		i__2 = w_subscr(k - 1, kw);
		a[i__1].r = w[i__2].r, a[i__1].i = w[i__2].i;
		i__1 = a_subscr(k, k);
		i__2 = w_subscr(k, kw);
		a[i__1].r = w[i__2].r, a[i__1].i = w[i__2].i;

/*              Conjugate W(k) and W(k-1) */

		i__1 = k - 1;
		zlacgv_(&i__1, &w_ref(1, kw), &c__1);
		i__1 = k - 2;
		zlacgv_(&i__1, &w_ref(1, kw - 1), &c__1);
	    }
	}

/*        Store details of the interchanges in IPIV */

	if (kstep == 1) {
	    ipiv[k] = kp;
	} else {
	    ipiv[k] = -kp;
	    ipiv[k - 1] = -kp;
	}

/*        Decrease K and return to the start of the main loop */

	k -= kstep;
	goto L10;

L30:

/*        Update the upper triangle of A11 (= A(1:k,1:k)) as   

          A11 := A11 - U12*D*U12' = A11 - U12*W'   

          computing blocks of NB columns at a time (note that conjg(W) is   
          actually stored) */

	i__1 = -(*nb);
	for (j = (k - 1) / *nb * *nb + 1; i__1 < 0 ? j >= 1 : j <= 1; j += 
		i__1) {
/* Computing MIN */
	    i__2 = *nb, i__3 = k - j + 1;
	    jb = min(i__2,i__3);

/*           Update the upper triangle of the diagonal block */

	    i__2 = j + jb - 1;
	    for (jj = j; jj <= i__2; ++jj) {
		i__3 = a_subscr(jj, jj);
		i__4 = a_subscr(jj, jj);
		d__1 = a[i__4].r;
		a[i__3].r = d__1, a[i__3].i = 0.;
		i__3 = jj - j + 1;
		i__4 = *n - k;
		z__1.r = -1., z__1.i = 0.;
		zgemv_("No transpose", &i__3, &i__4, &z__1, &a_ref(j, k + 1), 
			lda, &w_ref(jj, kw + 1), ldw, &c_b1, &a_ref(j, jj), &
			c__1);
		i__3 = a_subscr(jj, jj);
		i__4 = a_subscr(jj, jj);
		d__1 = a[i__4].r;
		a[i__3].r = d__1, a[i__3].i = 0.;
/* L40: */
	    }

/*           Update the rectangular superdiagonal block */

	    i__2 = j - 1;
	    i__3 = *n - k;
	    z__1.r = -1., z__1.i = 0.;
	    zgemm_("No transpose", "Transpose", &i__2, &jb, &i__3, &z__1, &
		    a_ref(1, k + 1), lda, &w_ref(j, kw + 1), ldw, &c_b1, &
		    a_ref(1, j), lda);
/* L50: */
	}

/*        Put U12 in standard form by partially undoing the interchanges   
          in columns k+1:n */

	j = k + 1;
L60:
	jj = j;
	jp = ipiv[j];
	if (jp < 0) {
	    jp = -jp;
	    ++j;
	}
	++j;
	if (jp != jj && j <= *n) {
	    i__1 = *n - j + 1;
	    zswap_(&i__1, &a_ref(jp, j), lda, &a_ref(jj, j), lda);
	}
	if (j <= *n) {
	    goto L60;
	}

/*        Set KB to the number of columns factorized */

	*kb = *n - k;

    } else {

/*        Factorize the leading columns of A using the lower triangle   
          of A and working forwards, and compute the matrix W = L21*D   
          for use in updating A22 (note that conjg(W) is actually stored)   

          K is the main loop index, increasing from 1 in steps of 1 or 2 */

	k = 1;
L70:

/*        Exit from loop */

	if (k >= *nb && *nb < *n || k > *n) {
	    goto L90;
	}

/*        Copy column K of A to column K of W and update it */

	i__1 = w_subscr(k, k);
	i__2 = a_subscr(k, k);
	d__1 = a[i__2].r;
	w[i__1].r = d__1, w[i__1].i = 0.;
	if (k < *n) {
	    i__1 = *n - k;
	    zcopy_(&i__1, &a_ref(k + 1, k), &c__1, &w_ref(k + 1, k), &c__1);
	}
	i__1 = *n - k + 1;
	i__2 = k - 1;
	z__1.r = -1., z__1.i = 0.;
	zgemv_("No transpose", &i__1, &i__2, &z__1, &a_ref(k, 1), lda, &w_ref(
		k, 1), ldw, &c_b1, &w_ref(k, k), &c__1);
	i__1 = w_subscr(k, k);
	i__2 = w_subscr(k, k);
	d__1 = w[i__2].r;
	w[i__1].r = d__1, w[i__1].i = 0.;

	kstep = 1;

/*        Determine rows and columns to be interchanged and whether   
          a 1-by-1 or 2-by-2 pivot block will be used */

	i__1 = w_subscr(k, k);
	absakk = (d__1 = w[i__1].r, abs(d__1));

/*        IMAX is the row-index of the largest off-diagonal element in   
          column K, and COLMAX is its absolute value */

	if (k < *n) {
	    i__1 = *n - k;
	    imax = k + izamax_(&i__1, &w_ref(k + 1, k), &c__1);
	    i__1 = w_subscr(imax, k);
	    colmax = (d__1 = w[i__1].r, abs(d__1)) + (d__2 = d_imag(&w_ref(
		    imax, k)), abs(d__2));
	} else {
	    colmax = 0.;
	}

	if (max(absakk,colmax) == 0.) {

/*           Column K is zero: set INFO and continue */

	    if (*info == 0) {
		*info = k;
	    }
	    kp = k;
	    i__1 = a_subscr(k, k);
	    i__2 = a_subscr(k, k);
	    d__1 = a[i__2].r;
	    a[i__1].r = d__1, a[i__1].i = 0.;
	} else {
	    if (absakk >= alpha * colmax) {

/*              no interchange, use 1-by-1 pivot block */

		kp = k;
	    } else {

/*              Copy column IMAX to column K+1 of W and update it */

		i__1 = imax - k;
		zcopy_(&i__1, &a_ref(imax, k), lda, &w_ref(k, k + 1), &c__1);
		i__1 = imax - k;
		zlacgv_(&i__1, &w_ref(k, k + 1), &c__1);
		i__1 = w_subscr(imax, k + 1);
		i__2 = a_subscr(imax, imax);
		d__1 = a[i__2].r;
		w[i__1].r = d__1, w[i__1].i = 0.;
		if (imax < *n) {
		    i__1 = *n - imax;
		    zcopy_(&i__1, &a_ref(imax + 1, imax), &c__1, &w_ref(imax 
			    + 1, k + 1), &c__1);
		}
		i__1 = *n - k + 1;
		i__2 = k - 1;
		z__1.r = -1., z__1.i = 0.;
		zgemv_("No transpose", &i__1, &i__2, &z__1, &a_ref(k, 1), lda,
			 &w_ref(imax, 1), ldw, &c_b1, &w_ref(k, k + 1), &c__1);
		i__1 = w_subscr(imax, k + 1);
		i__2 = w_subscr(imax, k + 1);
		d__1 = w[i__2].r;
		w[i__1].r = d__1, w[i__1].i = 0.;

/*              JMAX is the column-index of the largest off-diagonal   
                element in row IMAX, and ROWMAX is its absolute value */

		i__1 = imax - k;
		jmax = k - 1 + izamax_(&i__1, &w_ref(k, k + 1), &c__1);
		i__1 = w_subscr(jmax, k + 1);
		rowmax = (d__1 = w[i__1].r, abs(d__1)) + (d__2 = d_imag(&
			w_ref(jmax, k + 1)), abs(d__2));
		if (imax < *n) {
		    i__1 = *n - imax;
		    jmax = imax + izamax_(&i__1, &w_ref(imax + 1, k + 1), &
			    c__1);
/* Computing MAX */
		    i__1 = w_subscr(jmax, k + 1);
		    d__3 = rowmax, d__4 = (d__1 = w[i__1].r, abs(d__1)) + (
			    d__2 = d_imag(&w_ref(jmax, k + 1)), abs(d__2));
		    rowmax = max(d__3,d__4);
		}

		if (absakk >= alpha * colmax * (colmax / rowmax)) {

/*                 no interchange, use 1-by-1 pivot block */

		    kp = k;
		} else /* if(complicated condition) */ {
		    i__1 = w_subscr(imax, k + 1);
		    if ((d__1 = w[i__1].r, abs(d__1)) >= alpha * rowmax) {

/*                 interchange rows and columns K and IMAX, use 1-by-1   
                   pivot block */

			kp = imax;

/*                 copy column K+1 of W to column K */

			i__1 = *n - k + 1;
			zcopy_(&i__1, &w_ref(k, k + 1), &c__1, &w_ref(k, k), &
				c__1);
		    } else {

/*                 interchange rows and columns K+1 and IMAX, use 2-by-2   
                   pivot block */

			kp = imax;
			kstep = 2;
		    }
		}
	    }

	    kk = k + kstep - 1;

/*           Updated column KP is already stored in column KK of W */

	    if (kp != kk) {

/*              Copy non-updated column KK to column KP */

		i__1 = a_subscr(kp, kp);
		i__2 = a_subscr(kk, kk);
		d__1 = a[i__2].r;
		a[i__1].r = d__1, a[i__1].i = 0.;
		i__1 = kp - kk - 1;
		zcopy_(&i__1, &a_ref(kk + 1, kk), &c__1, &a_ref(kp, kk + 1), 
			lda);
		i__1 = kp - kk - 1;
		zlacgv_(&i__1, &a_ref(kp, kk + 1), lda);
		if (kp < *n) {
		    i__1 = *n - kp;
		    zcopy_(&i__1, &a_ref(kp + 1, kk), &c__1, &a_ref(kp + 1, 
			    kp), &c__1);
		}

/*              Interchange rows KK and KP in first KK columns of A and W */

		i__1 = kk - 1;
		zswap_(&i__1, &a_ref(kk, 1), lda, &a_ref(kp, 1), lda);
		zswap_(&kk, &w_ref(kk, 1), ldw, &w_ref(kp, 1), ldw);
	    }

	    if (kstep == 1) {

/*              1-by-1 pivot block D(k): column k of W now holds   

                W(k) = L(k)*D(k)   

                where L(k) is the k-th column of L   

                Store L(k) in column k of A */

		i__1 = *n - k + 1;
		zcopy_(&i__1, &w_ref(k, k), &c__1, &a_ref(k, k), &c__1);
		if (k < *n) {
		    i__1 = a_subscr(k, k);
		    r1 = 1. / a[i__1].r;
		    i__1 = *n - k;
		    zdscal_(&i__1, &r1, &a_ref(k + 1, k), &c__1);

/*                 Conjugate W(k) */

		    i__1 = *n - k;
		    zlacgv_(&i__1, &w_ref(k + 1, k), &c__1);
		}
	    } else {

/*              2-by-2 pivot block D(k): columns k and k+1 of W now hold   

                ( W(k) W(k+1) ) = ( L(k) L(k+1) )*D(k)   

                where L(k) and L(k+1) are the k-th and (k+1)-th columns   
                of L */

		if (k < *n - 1) {

/*                 Store L(k) and L(k+1) in columns k and k+1 of A */

		    i__1 = w_subscr(k + 1, k);
		    d21.r = w[i__1].r, d21.i = w[i__1].i;
		    z_div(&z__1, &w_ref(k + 1, k + 1), &d21);
		    d11.r = z__1.r, d11.i = z__1.i;
		    d_cnjg(&z__2, &d21);
		    z_div(&z__1, &w_ref(k, k), &z__2);
		    d22.r = z__1.r, d22.i = z__1.i;
		    z__1.r = d11.r * d22.r - d11.i * d22.i, z__1.i = d11.r * 
			    d22.i + d11.i * d22.r;
		    t = 1. / (z__1.r - 1.);
		    z__2.r = t, z__2.i = 0.;
		    z_div(&z__1, &z__2, &d21);
		    d21.r = z__1.r, d21.i = z__1.i;
		    i__1 = *n;
		    for (j = k + 2; j <= i__1; ++j) {
			i__2 = a_subscr(j, k);
			d_cnjg(&z__2, &d21);
			i__3 = w_subscr(j, k);
			z__4.r = d11.r * w[i__3].r - d11.i * w[i__3].i, 
				z__4.i = d11.r * w[i__3].i + d11.i * w[i__3]
				.r;
			i__4 = w_subscr(j, k + 1);
			z__3.r = z__4.r - w[i__4].r, z__3.i = z__4.i - w[i__4]
				.i;
			z__1.r = z__2.r * z__3.r - z__2.i * z__3.i, z__1.i = 
				z__2.r * z__3.i + z__2.i * z__3.r;
			a[i__2].r = z__1.r, a[i__2].i = z__1.i;
			i__2 = a_subscr(j, k + 1);
			i__3 = w_subscr(j, k + 1);
			z__3.r = d22.r * w[i__3].r - d22.i * w[i__3].i, 
				z__3.i = d22.r * w[i__3].i + d22.i * w[i__3]
				.r;
			i__4 = w_subscr(j, k);
			z__2.r = z__3.r - w[i__4].r, z__2.i = z__3.i - w[i__4]
				.i;
			z__1.r = d21.r * z__2.r - d21.i * z__2.i, z__1.i = 
				d21.r * z__2.i + d21.i * z__2.r;
			a[i__2].r = z__1.r, a[i__2].i = z__1.i;
/* L80: */
		    }
		}

/*              Copy D(k) to A */

		i__1 = a_subscr(k, k);
		i__2 = w_subscr(k, k);
		a[i__1].r = w[i__2].r, a[i__1].i = w[i__2].i;
		i__1 = a_subscr(k + 1, k);
		i__2 = w_subscr(k + 1, k);
		a[i__1].r = w[i__2].r, a[i__1].i = w[i__2].i;
		i__1 = a_subscr(k + 1, k + 1);
		i__2 = w_subscr(k + 1, k + 1);
		a[i__1].r = w[i__2].r, a[i__1].i = w[i__2].i;

/*              Conjugate W(k) and W(k+1) */

		i__1 = *n - k;
		zlacgv_(&i__1, &w_ref(k + 1, k), &c__1);
		i__1 = *n - k - 1;
		zlacgv_(&i__1, &w_ref(k + 2, k + 1), &c__1);
	    }
	}

/*        Store details of the interchanges in IPIV */

	if (kstep == 1) {
	    ipiv[k] = kp;
	} else {
	    ipiv[k] = -kp;
	    ipiv[k + 1] = -kp;
	}

/*        Increase K and return to the start of the main loop */

	k += kstep;
	goto L70;

L90:

/*        Update the lower triangle of A22 (= A(k:n,k:n)) as   

          A22 := A22 - L21*D*L21' = A22 - L21*W'   

          computing blocks of NB columns at a time (note that conjg(W) is   
          actually stored) */

	i__1 = *n;
	i__2 = *nb;
	for (j = k; i__2 < 0 ? j >= i__1 : j <= i__1; j += i__2) {
/* Computing MIN */
	    i__3 = *nb, i__4 = *n - j + 1;
	    jb = min(i__3,i__4);

/*           Update the lower triangle of the diagonal block */

	    i__3 = j + jb - 1;
	    for (jj = j; jj <= i__3; ++jj) {
		i__4 = a_subscr(jj, jj);
		i__5 = a_subscr(jj, jj);
		d__1 = a[i__5].r;
		a[i__4].r = d__1, a[i__4].i = 0.;
		i__4 = j + jb - jj;
		i__5 = k - 1;
		z__1.r = -1., z__1.i = 0.;
		zgemv_("No transpose", &i__4, &i__5, &z__1, &a_ref(jj, 1), 
			lda, &w_ref(jj, 1), ldw, &c_b1, &a_ref(jj, jj), &c__1);
		i__4 = a_subscr(jj, jj);
		i__5 = a_subscr(jj, jj);
		d__1 = a[i__5].r;
		a[i__4].r = d__1, a[i__4].i = 0.;
/* L100: */
	    }

/*           Update the rectangular subdiagonal block */

	    if (j + jb <= *n) {
		i__3 = *n - j - jb + 1;
		i__4 = k - 1;
		z__1.r = -1., z__1.i = 0.;
		zgemm_("No transpose", "Transpose", &i__3, &jb, &i__4, &z__1, 
			&a_ref(j + jb, 1), lda, &w_ref(j, 1), ldw, &c_b1, &
			a_ref(j + jb, j), lda);
	    }
/* L110: */
	}

/*        Put L21 in standard form by partially undoing the interchanges   
          in columns 1:k-1 */

	j = k - 1;
L120:
	jj = j;
	jp = ipiv[j];
	if (jp < 0) {
	    jp = -jp;
	    --j;
	}
	--j;
	if (jp != jj && j >= 1) {
	    zswap_(&j, &a_ref(jp, 1), lda, &a_ref(jj, 1), lda);
	}
	if (j >= 1) {
	    goto L120;
	}

/*        Set KB to the number of columns factorized */

	*kb = k - 1;

    }
    return 0;

/*     End of ZLAHEF */

} /* zlahef_ */

#undef w_ref
#undef w_subscr
#undef a_ref
#undef a_subscr


