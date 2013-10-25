#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int zlarzt_(char *direct, char *storev, integer *n, integer *
	k, doublecomplex *v, integer *ldv, doublecomplex *tau, doublecomplex *
	t, integer *ldt)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    ZLARZT forms the triangular factor T of a complex block reflector   
    H of order > n, which is defined as a product of k elementary   
    reflectors.   

    If DIRECT = 'F', H = H(1) H(2) . . . H(k) and T is upper triangular;   

    If DIRECT = 'B', H = H(k) . . . H(2) H(1) and T is lower triangular.   

    If STOREV = 'C', the vector which defines the elementary reflector   
    H(i) is stored in the i-th column of the array V, and   

       H  =  I - V * T * V'   

    If STOREV = 'R', the vector which defines the elementary reflector   
    H(i) is stored in the i-th row of the array V, and   

       H  =  I - V' * T * V   

    Currently, only STOREV = 'R' and DIRECT = 'B' are supported.   

    Arguments   
    =========   

    DIRECT  (input) CHARACTER*1   
            Specifies the order in which the elementary reflectors are   
            multiplied to form the block reflector:   
            = 'F': H = H(1) H(2) . . . H(k) (Forward, not supported yet)   
            = 'B': H = H(k) . . . H(2) H(1) (Backward)   

    STOREV  (input) CHARACTER*1   
            Specifies how the vectors which define the elementary   
            reflectors are stored (see also Further Details):   
            = 'C': columnwise                        (not supported yet)   
            = 'R': rowwise   

    N       (input) INTEGER   
            The order of the block reflector H. N >= 0.   

    K       (input) INTEGER   
            The order of the triangular factor T (= the number of   
            elementary reflectors). K >= 1.   

    V       (input/output) COMPLEX*16 array, dimension   
                                 (LDV,K) if STOREV = 'C'   
                                 (LDV,N) if STOREV = 'R'   
            The matrix V. See further details.   

    LDV     (input) INTEGER   
            The leading dimension of the array V.   
            If STOREV = 'C', LDV >= max(1,N); if STOREV = 'R', LDV >= K.   

    TAU     (input) COMPLEX*16 array, dimension (K)   
            TAU(i) must contain the scalar factor of the elementary   
            reflector H(i).   

    T       (output) COMPLEX*16 array, dimension (LDT,K)   
            The k by k triangular factor T of the block reflector.   
            If DIRECT = 'F', T is upper triangular; if DIRECT = 'B', T is   
            lower triangular. The rest of the array is not used.   

    LDT     (input) INTEGER   
            The leading dimension of the array T. LDT >= K.   

    Further Details   
    ===============   

    Based on contributions by   
      A. Petitet, Computer Science Dept., Univ. of Tenn., Knoxville, USA   

    The shape of the matrix V and the storage of the vectors which define   
    the H(i) is best illustrated by the following example with n = 5 and   
    k = 3. The elements equal to 1 are not stored; the corresponding   
    array elements are modified but restored on exit. The rest of the   
    array is not used.   

    DIRECT = 'F' and STOREV = 'C':         DIRECT = 'F' and STOREV = 'R':   

                                                ______V_____   
           ( v1 v2 v3 )                        /            \   
           ( v1 v2 v3 )                      ( v1 v1 v1 v1 v1 . . . . 1 )   
       V = ( v1 v2 v3 )                      ( v2 v2 v2 v2 v2 . . . 1   )   
           ( v1 v2 v3 )                      ( v3 v3 v3 v3 v3 . . 1     )   
           ( v1 v2 v3 )   
              .  .  .   
              .  .  .   
              1  .  .   
                 1  .   
                    1   

    DIRECT = 'B' and STOREV = 'C':         DIRECT = 'B' and STOREV = 'R':   

                                                          ______V_____   
              1                                          /            \   
              .  1                           ( 1 . . . . v1 v1 v1 v1 v1 )   
              .  .  1                        ( . 1 . . . v2 v2 v2 v2 v2 )   
              .  .  .                        ( . . 1 . . v3 v3 v3 v3 v3 )   
              .  .  .   
           ( v1 v2 v3 )   
           ( v1 v2 v3 )   
       V = ( v1 v2 v3 )   
           ( v1 v2 v3 )   
           ( v1 v2 v3 )   

    =====================================================================   


       Check for currently supported options   

       Parameter adjustments */
    /* Table of constant values */
    static doublecomplex c_b1 = {0.,0.};
    static integer c__1 = 1;
    
    /* System generated locals */
    integer t_dim1, t_offset, v_dim1, v_offset, i__1, i__2;
    doublecomplex z__1;
    /* Local variables */
    static integer info, i__, j;
    extern logical lsame_(char *, char *);
    extern /* Subroutine */ int zgemv_(char *, integer *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, doublecomplex *, integer *), 
	    ztrmv_(char *, char *, char *, integer *, doublecomplex *, 
	    integer *, doublecomplex *, integer *), 
	    xerbla_(char *, integer *), zlacgv_(integer *, 
	    doublecomplex *, integer *);
#define t_subscr(a_1,a_2) (a_2)*t_dim1 + a_1
#define t_ref(a_1,a_2) t[t_subscr(a_1,a_2)]
#define v_subscr(a_1,a_2) (a_2)*v_dim1 + a_1
#define v_ref(a_1,a_2) v[v_subscr(a_1,a_2)]


    v_dim1 = *ldv;
    v_offset = 1 + v_dim1 * 1;
    v -= v_offset;
    --tau;
    t_dim1 = *ldt;
    t_offset = 1 + t_dim1 * 1;
    t -= t_offset;

    /* Function Body */
    info = 0;
    if (! lsame_(direct, "B")) {
	info = -1;
    } else if (! lsame_(storev, "R")) {
	info = -2;
    }
    if (info != 0) {
	i__1 = -info;
	xerbla_("ZLARZT", &i__1);
	return 0;
    }

    for (i__ = *k; i__ >= 1; --i__) {
	i__1 = i__;
	if (tau[i__1].r == 0. && tau[i__1].i == 0.) {

/*           H(i)  =  I */

	    i__1 = *k;
	    for (j = i__; j <= i__1; ++j) {
		i__2 = t_subscr(j, i__);
		t[i__2].r = 0., t[i__2].i = 0.;
/* L10: */
	    }
	} else {

/*           general case */

	    if (i__ < *k) {

/*              T(i+1:k,i) = - tau(i) * V(i+1:k,1:n) * V(i,1:n)' */

		zlacgv_(n, &v_ref(i__, 1), ldv);
		i__1 = *k - i__;
		i__2 = i__;
		z__1.r = -tau[i__2].r, z__1.i = -tau[i__2].i;
		zgemv_("No transpose", &i__1, n, &z__1, &v_ref(i__ + 1, 1), 
			ldv, &v_ref(i__, 1), ldv, &c_b1, &t_ref(i__ + 1, i__),
			 &c__1);
		zlacgv_(n, &v_ref(i__, 1), ldv);

/*              T(i+1:k,i) = T(i+1:k,i+1:k) * T(i+1:k,i) */

		i__1 = *k - i__;
		ztrmv_("Lower", "No transpose", "Non-unit", &i__1, &t_ref(i__ 
			+ 1, i__ + 1), ldt, &t_ref(i__ + 1, i__), &c__1);
	    }
	    i__1 = t_subscr(i__, i__);
	    i__2 = i__;
	    t[i__1].r = tau[i__2].r, t[i__1].i = tau[i__2].i;
	}
/* L20: */
    }
    return 0;

/*     End of ZLARZT */

} /* zlarzt_ */

#undef v_ref
#undef v_subscr
#undef t_ref
#undef t_subscr


