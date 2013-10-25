#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zgbtrf_(integer *m, integer *n, integer *kl, integer *ku,
	 doublecomplex *ab, integer *ldab, integer *ipiv, integer *info)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    ZGBTRF computes an LU factorization of a complex m-by-n band matrix A   
    using partial pivoting with row interchanges.   

    This is the blocked version of the algorithm, calling Level 3 BLAS.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    KL      (input) INTEGER   
            The number of subdiagonals within the band of A.  KL >= 0.   

    KU      (input) INTEGER   
            The number of superdiagonals within the band of A.  KU >= 0.   

    AB      (input/output) COMPLEX*16 array, dimension (LDAB,N)   
            On entry, the matrix A in band storage, in rows KL+1 to   
            2*KL+KU+1; rows 1 to KL of the array need not be set.   
            The j-th column of A is stored in the j-th column of the   
            array AB as follows:   
            AB(kl+ku+1+i-j,j) = A(i,j) for max(1,j-ku)<=i<=min(m,j+kl)   

            On exit, details of the factorization: U is stored as an   
            upper triangular band matrix with KL+KU superdiagonals in   
            rows 1 to KL+KU+1, and the multipliers used during the   
            factorization are stored in rows KL+KU+2 to 2*KL+KU+1.   
            See below for further details.   

    LDAB    (input) INTEGER   
            The leading dimension of the array AB.  LDAB >= 2*KL+KU+1.   

    IPIV    (output) INTEGER array, dimension (min(M,N))   
            The pivot indices; for 1 <= i <= min(M,N), row i of the   
            matrix was interchanged with row IPIV(i).   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   
            > 0: if INFO = +i, U(i,i) is exactly zero. The factorization   
                 has been completed, but the factor U is exactly   
                 singular, and division by zero will occur if it is used   
                 to solve a system of equations.   

    Further Details   
    ===============   

    The band storage scheme is illustrated by the following example, when   
    M = N = 6, KL = 2, KU = 1:   

    On entry:                       On exit:   

        *    *    *    +    +    +       *    *    *   u14  u25  u36   
        *    *    +    +    +    +       *    *   u13  u24  u35  u46   
        *   a12  a23  a34  a45  a56      *   u12  u23  u34  u45  u56   
       a11  a22  a33  a44  a55  a66     u11  u22  u33  u44  u55  u66   
       a21  a32  a43  a54  a65   *      m21  m32  m43  m54  m65   *   
       a31  a42  a53  a64   *    *      m31  m42  m53  m64   *    *   

    Array elements marked * are not used by the routine; elements marked   
    + need not be set on entry, but are required by the routine to store   
    elements of U because of fill-in resulting from the row interchanges.   

    =====================================================================   


       KV is the number of superdiagonals in the factor U, allowing for   
       fill-in   

       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {1.,0.};
    static integer c__1 = 1;
    static integer c__65 = 65;
    
    /* System generated locals */
    integer ab_dim1, ab_offset, i__1, i__2, i__3, i__4, i__5, i__6;
    doublecomplex z__1;
    /* Builtin functions */
    void z_div(doublecomplex *, doublecomplex *, doublecomplex *);
    /* Local variables */
    static doublecomplex temp;
    static integer i__, j;
    extern /* Subroutine */ int zscal_(integer *, doublecomplex *, 
	    doublecomplex *, integer *), zgemm_(char *, char *, integer *, 
	    integer *, integer *, doublecomplex *, doublecomplex *, integer *,
	     doublecomplex *, integer *, doublecomplex *, doublecomplex *, 
	    integer *);
    static doublecomplex work13[4160]	/* was [65][64] */, work31[4160]	
	    /* was [65][64] */;
    static integer i2, i3, j2, j3, k2;
    extern /* Subroutine */ int zgeru_(integer *, integer *, doublecomplex *, 
	    doublecomplex *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, integer *), zcopy_(integer *, doublecomplex *, 
	    integer *, doublecomplex *, integer *), zswap_(integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *), ztrsm_(
	    char *, char *, char *, char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *), zgbtf2_(integer *, 
	    integer *, integer *, integer *, doublecomplex *, integer *, 
	    integer *, integer *);
    static integer jb, nb, ii, jj, jm, ip, jp, km, ju, kv, nw;
    extern /* Subroutine */ int xerbla_(char *, integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen), izamax_(integer *, 
	    doublecomplex *, integer *);
    extern /* Subroutine */ int zlaswp_(integer *, doublecomplex *, integer *,
	     integer *, integer *, integer *, integer *);
#define work13_subscr(a_1,a_2) (a_2)*65 + a_1 - 66
#define work13_ref(a_1,a_2) work13[work13_subscr(a_1,a_2)]
#define work31_subscr(a_1,a_2) (a_2)*65 + a_1 - 66
#define work31_ref(a_1,a_2) work31[work31_subscr(a_1,a_2)]
#define ab_subscr(a_1,a_2) (a_2)*ab_dim1 + a_1
#define ab_ref(a_1,a_2) ab[ab_subscr(a_1,a_2)]


    ab_dim1 = *ldab;
    ab_offset = 1 + ab_dim1 * 1;
    ab -= ab_offset;
    --ipiv;

    /* Function Body */
    kv = *ku + *kl;

/*     Test the input parameters. */

    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*kl < 0) {
	*info = -3;
    } else if (*ku < 0) {
	*info = -4;
    } else if (*ldab < *kl + kv + 1) {
	*info = -6;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZGBTRF", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*m == 0 || *n == 0) {
	return 0;
    }

/*     Determine the block size for this environment */

    nb = ilaenv_(&c__1, "ZGBTRF", " ", m, n, kl, ku, (ftnlen)6, (ftnlen)1);

/*     The block size must not exceed the limit set by the size of the   
       local arrays WORK13 and WORK31. */

    nb = min(nb,64);

    if (nb <= 1 || nb > *kl) {

/*        Use unblocked code */

	zgbtf2_(m, n, kl, ku, &ab[ab_offset], ldab, &ipiv[1], info);
    } else {

/*        Use blocked code   

          Zero the superdiagonal elements of the work array WORK13 */

	i__1 = nb;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = j - 1;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		i__3 = work13_subscr(i__, j);
		work13[i__3].r = 0., work13[i__3].i = 0.;
/* L10: */
	    }
/* L20: */
	}

/*        Zero the subdiagonal elements of the work array WORK31 */

	i__1 = nb;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = nb;
	    for (i__ = j + 1; i__ <= i__2; ++i__) {
		i__3 = work31_subscr(i__, j);
		work31[i__3].r = 0., work31[i__3].i = 0.;
/* L30: */
	    }
/* L40: */
	}

/*        Gaussian elimination with partial pivoting   

          Set fill-in elements in columns KU+2 to KV to zero */

	i__1 = min(kv,*n);
	for (j = *ku + 2; j <= i__1; ++j) {
	    i__2 = *kl;
	    for (i__ = kv - j + 2; i__ <= i__2; ++i__) {
		i__3 = ab_subscr(i__, j);
		ab[i__3].r = 0., ab[i__3].i = 0.;
/* L50: */
	    }
/* L60: */
	}

/*        JU is the index of the last column affected by the current   
          stage of the factorization */

	ju = 1;

	i__1 = min(*m,*n);
	i__2 = nb;
	for (j = 1; i__2 < 0 ? j >= i__1 : j <= i__1; j += i__2) {
/* Computing MIN */
	    i__3 = nb, i__4 = min(*m,*n) - j + 1;
	    jb = min(i__3,i__4);

/*           The active part of the matrix is partitioned   

                A11   A12   A13   
                A21   A22   A23   
                A31   A32   A33   

             Here A11, A21 and A31 denote the current block of JB columns   
             which is about to be factorized. The number of rows in the   
             partitioning are JB, I2, I3 respectively, and the numbers   
             of columns are JB, J2, J3. The superdiagonal elements of A13   
             and the subdiagonal elements of A31 lie outside the band.   

   Computing MIN */
	    i__3 = *kl - jb, i__4 = *m - j - jb + 1;
	    i2 = min(i__3,i__4);
/* Computing MIN */
	    i__3 = jb, i__4 = *m - j - *kl + 1;
	    i3 = min(i__3,i__4);

/*           J2 and J3 are computed after JU has been updated.   

             Factorize the current block of JB columns */

	    i__3 = j + jb - 1;
	    for (jj = j; jj <= i__3; ++jj) {

/*              Set fill-in elements in column JJ+KV to zero */

		if (jj + kv <= *n) {
		    i__4 = *kl;
		    for (i__ = 1; i__ <= i__4; ++i__) {
			i__5 = ab_subscr(i__, jj + kv);
			ab[i__5].r = 0., ab[i__5].i = 0.;
/* L70: */
		    }
		}

/*              Find pivot and test for singularity. KM is the number of   
                subdiagonal elements in the current column.   

   Computing MIN */
		i__4 = *kl, i__5 = *m - jj;
		km = min(i__4,i__5);
		i__4 = km + 1;
		jp = izamax_(&i__4, &ab_ref(kv + 1, jj), &c__1);
		ipiv[jj] = jp + jj - j;
		i__4 = ab_subscr(kv + jp, jj);
		if (ab[i__4].r != 0. || ab[i__4].i != 0.) {
/* Computing MAX   
   Computing MIN */
		    i__6 = jj + *ku + jp - 1;
		    i__4 = ju, i__5 = min(i__6,*n);
		    ju = max(i__4,i__5);
		    if (jp != 1) {

/*                    Apply interchange to columns J to J+JB-1 */

			if (jp + jj - 1 < j + *kl) {

			    i__4 = *ldab - 1;
			    i__5 = *ldab - 1;
			    zswap_(&jb, &ab_ref(kv + 1 + jj - j, j), &i__4, &
				    ab_ref(kv + jp + jj - j, j), &i__5);
			} else {

/*                       The interchange affects columns J to JJ-1 of A31   
                         which are stored in the work array WORK31 */

			    i__4 = jj - j;
			    i__5 = *ldab - 1;
			    zswap_(&i__4, &ab_ref(kv + 1 + jj - j, j), &i__5, 
				    &work31_ref(jp + jj - j - *kl, 1), &c__65)
				    ;
			    i__4 = j + jb - jj;
			    i__5 = *ldab - 1;
			    i__6 = *ldab - 1;
			    zswap_(&i__4, &ab_ref(kv + 1, jj), &i__5, &ab_ref(
				    kv + jp, jj), &i__6);
			}
		    }

/*                 Compute multipliers */

		    z_div(&z__1, &c_b1, &ab_ref(kv + 1, jj));
		    zscal_(&km, &z__1, &ab_ref(kv + 2, jj), &c__1);

/*                 Update trailing submatrix within the band and within   
                   the current block. JM is the index of the last column   
                   which needs to be updated.   

   Computing MIN */
		    i__4 = ju, i__5 = j + jb - 1;
		    jm = min(i__4,i__5);
		    if (jm > jj) {
			i__4 = jm - jj;
			z__1.r = -1., z__1.i = 0.;
			i__5 = *ldab - 1;
			i__6 = *ldab - 1;
			zgeru_(&km, &i__4, &z__1, &ab_ref(kv + 2, jj), &c__1, 
				&ab_ref(kv, jj + 1), &i__5, &ab_ref(kv + 1, 
				jj + 1), &i__6);
		    }
		} else {

/*                 If pivot is zero, set INFO to the index of the pivot   
                   unless a zero pivot has already been found. */

		    if (*info == 0) {
			*info = jj;
		    }
		}

/*              Copy current column of A31 into the work array WORK31   

   Computing MIN */
		i__4 = jj - j + 1;
		nw = min(i__4,i3);
		if (nw > 0) {
		    zcopy_(&nw, &ab_ref(kv + *kl + 1 - jj + j, jj), &c__1, &
			    work31_ref(1, jj - j + 1), &c__1);
		}
/* L80: */
	    }
	    if (j + jb <= *n) {

/*              Apply the row interchanges to the other blocks.   

   Computing MIN */
		i__3 = ju - j + 1;
		j2 = min(i__3,kv) - jb;
/* Computing MAX */
		i__3 = 0, i__4 = ju - j - kv + 1;
		j3 = max(i__3,i__4);

/*              Use ZLASWP to apply the row interchanges to A12, A22, and   
                A32. */

		i__3 = *ldab - 1;
		zlaswp_(&j2, &ab_ref(kv + 1 - jb, j + jb), &i__3, &c__1, &jb, 
			&ipiv[j], &c__1);

/*              Adjust the pivot indices. */

		i__3 = j + jb - 1;
		for (i__ = j; i__ <= i__3; ++i__) {
		    ipiv[i__] = ipiv[i__] + j - 1;
/* L90: */
		}

/*              Apply the row interchanges to A13, A23, and A33   
                columnwise. */

		k2 = j - 1 + jb + j2;
		i__3 = j3;
		for (i__ = 1; i__ <= i__3; ++i__) {
		    jj = k2 + i__;
		    i__4 = j + jb - 1;
		    for (ii = j + i__ - 1; ii <= i__4; ++ii) {
			ip = ipiv[ii];
			if (ip != ii) {
			    i__5 = ab_subscr(kv + 1 + ii - jj, jj);
			    temp.r = ab[i__5].r, temp.i = ab[i__5].i;
			    i__5 = ab_subscr(kv + 1 + ii - jj, jj);
			    i__6 = ab_subscr(kv + 1 + ip - jj, jj);
			    ab[i__5].r = ab[i__6].r, ab[i__5].i = ab[i__6].i;
			    i__5 = ab_subscr(kv + 1 + ip - jj, jj);
			    ab[i__5].r = temp.r, ab[i__5].i = temp.i;
			}
/* L100: */
		    }
/* L110: */
		}

/*              Update the relevant part of the trailing submatrix */

		if (j2 > 0) {

/*                 Update A12 */

		    i__3 = *ldab - 1;
		    i__4 = *ldab - 1;
		    ztrsm_("Left", "Lower", "No transpose", "Unit", &jb, &j2, 
			    &c_b1, &ab_ref(kv + 1, j), &i__3, &ab_ref(kv + 1 
			    - jb, j + jb), &i__4);

		    if (i2 > 0) {

/*                    Update A22 */

			z__1.r = -1., z__1.i = 0.;
			i__3 = *ldab - 1;
			i__4 = *ldab - 1;
			i__5 = *ldab - 1;
			zgemm_("No transpose", "No transpose", &i2, &j2, &jb, 
				&z__1, &ab_ref(kv + 1 + jb, j), &i__3, &
				ab_ref(kv + 1 - jb, j + jb), &i__4, &c_b1, &
				ab_ref(kv + 1, j + jb), &i__5);
		    }

		    if (i3 > 0) {

/*                    Update A32 */

			z__1.r = -1., z__1.i = 0.;
			i__3 = *ldab - 1;
			i__4 = *ldab - 1;
			zgemm_("No transpose", "No transpose", &i3, &j2, &jb, 
				&z__1, work31, &c__65, &ab_ref(kv + 1 - jb, j 
				+ jb), &i__3, &c_b1, &ab_ref(kv + *kl + 1 - 
				jb, j + jb), &i__4);
		    }
		}

		if (j3 > 0) {

/*                 Copy the lower triangle of A13 into the work array   
                   WORK13 */

		    i__3 = j3;
		    for (jj = 1; jj <= i__3; ++jj) {
			i__4 = jb;
			for (ii = jj; ii <= i__4; ++ii) {
			    i__5 = work13_subscr(ii, jj);
			    i__6 = ab_subscr(ii - jj + 1, jj + j + kv - 1);
			    work13[i__5].r = ab[i__6].r, work13[i__5].i = ab[
				    i__6].i;
/* L120: */
			}
/* L130: */
		    }

/*                 Update A13 in the work array */

		    i__3 = *ldab - 1;
		    ztrsm_("Left", "Lower", "No transpose", "Unit", &jb, &j3, 
			    &c_b1, &ab_ref(kv + 1, j), &i__3, work13, &c__65);

		    if (i2 > 0) {

/*                    Update A23 */

			z__1.r = -1., z__1.i = 0.;
			i__3 = *ldab - 1;
			i__4 = *ldab - 1;
			zgemm_("No transpose", "No transpose", &i2, &j3, &jb, 
				&z__1, &ab_ref(kv + 1 + jb, j), &i__3, work13,
				 &c__65, &c_b1, &ab_ref(jb + 1, j + kv), &
				i__4);
		    }

		    if (i3 > 0) {

/*                    Update A33 */

			z__1.r = -1., z__1.i = 0.;
			i__3 = *ldab - 1;
			zgemm_("No transpose", "No transpose", &i3, &j3, &jb, 
				&z__1, work31, &c__65, work13, &c__65, &c_b1, 
				&ab_ref(*kl + 1, j + kv), &i__3);
		    }

/*                 Copy the lower triangle of A13 back into place */

		    i__3 = j3;
		    for (jj = 1; jj <= i__3; ++jj) {
			i__4 = jb;
			for (ii = jj; ii <= i__4; ++ii) {
			    i__5 = ab_subscr(ii - jj + 1, jj + j + kv - 1);
			    i__6 = work13_subscr(ii, jj);
			    ab[i__5].r = work13[i__6].r, ab[i__5].i = work13[
				    i__6].i;
/* L140: */
			}
/* L150: */
		    }
		}
	    } else {

/*              Adjust the pivot indices. */

		i__3 = j + jb - 1;
		for (i__ = j; i__ <= i__3; ++i__) {
		    ipiv[i__] = ipiv[i__] + j - 1;
/* L160: */
		}
	    }

/*           Partially undo the interchanges in the current block to   
             restore the upper triangular form of A31 and copy the upper   
             triangle of A31 back into place */

	    i__3 = j;
	    for (jj = j + jb - 1; jj >= i__3; --jj) {
		jp = ipiv[jj] - jj + 1;
		if (jp != 1) {

/*                 Apply interchange to columns J to JJ-1 */

		    if (jp + jj - 1 < j + *kl) {

/*                    The interchange does not affect A31 */

			i__4 = jj - j;
			i__5 = *ldab - 1;
			i__6 = *ldab - 1;
			zswap_(&i__4, &ab_ref(kv + 1 + jj - j, j), &i__5, &
				ab_ref(kv + jp + jj - j, j), &i__6);
		    } else {

/*                    The interchange does affect A31 */

			i__4 = jj - j;
			i__5 = *ldab - 1;
			zswap_(&i__4, &ab_ref(kv + 1 + jj - j, j), &i__5, &
				work31_ref(jp + jj - j - *kl, 1), &c__65);
		    }
		}

/*              Copy the current column of A31 back into place   

   Computing MIN */
		i__4 = i3, i__5 = jj - j + 1;
		nw = min(i__4,i__5);
		if (nw > 0) {
		    zcopy_(&nw, &work31_ref(1, jj - j + 1), &c__1, &ab_ref(kv 
			    + *kl + 1 - jj + j, jj), &c__1);
		}
/* L170: */
	    }
/* L180: */
	}
    }

    return 0;

/*     End of ZGBTRF */

} /* zgbtrf_ */

#undef ab_ref
#undef ab_subscr
#undef work31_ref
#undef work31_subscr
#undef work13_ref
#undef work13_subscr


