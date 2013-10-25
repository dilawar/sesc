/* 
 * Thanks to Greg Link from Penn State University
 * for his math acceleration engine. Where available,
 * the modified version of the engine found here, uses 
 * the fast, vendor-provided linear algebra routines 
 * from the BLAS and LAPACK packages in  lieu of
 * the vanilla C code present in the matrix functions 
 * of the previous versions of HotSpot. 
 */
#define MA_NONE		0 
#define MA_INTEL	1 
#define MA_AMD		2 
#define MA_APPLE	3 
#define MA_SUN		4 

#if (MATHACCEL == MA_INTEL)
	#include <mkl_cblas.h>
	#include <mkl_lapack.h>
#elif (MATHACCEL == MA_AMD)
	#include <acml.h>
#elif (MATHACCEL == MA_APPLE)
	#include <vecLib/cblas.h>
	#include <vecLib/clapack.h>
#elif (MATHACCEL == MA_SUN)
	#include <sunperf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "temperature.h"
#include "flp.h"
#include "util.h"

/* get the thermal resistance values. 'k' is the thermal conductivity
 * of the actual material (silicon or copper). Lb and Wb are bulk length
 * and width respectively. Ws is the source width. 't' is the thickness
 * The calculations are from "Heat Spreading and Conduction in Compressed 
 * Heatsinks", Jaana Behm and Jari Huttunen, in proceedings of the 10th
 * International Flotherm User Conference, May 2001.
 */

double getr(double k, double Wb, double Lb, double Ws, double t)
{
/*
	return (Lb/2)/(k*Wb*t);
*/	
	double theta, Bi, As, Ab, epsilon, tau, a, b, lamda, phi, psi, Rsprd;
	theta = (Lb/2)/(k*Wb*t);
	As = Ws*t;
	Ab = Wb*t;
	a = sqrt(As/M_PI);
	b = sqrt(Ab/M_PI);
	epsilon = a/b;
	lamda = M_PI+1/(sqrt(M_PI)*epsilon);
	Bi = 1/(M_PI*k*b*theta);
	tau = Lb/(2*b);
	phi = (tanh(lamda*tau)+lamda/Bi)/(1+(lamda/Bi)*tanh(lamda*tau));
	psi = (epsilon*tau+fabs(1-epsilon)*phi)/sqrt(M_PI);
	Rsprd = psi/(sqrt(M_PI)*k*a);
	return Rsprd;
}

/*
 * LUP decomposition from the pseudocode given in the CLR 
 * 'Introduction to Algorithms' textbook. The matrix 'a' is
 * transformed into an in-place lower/upper triangular matrix
 * and the vector'p' carries the permutation vector such that
 * Pa = lu, where 'P' is the matrix form of 'p'. The 'spd' flag 
 * indicates that 'a' is symmetric and positive definite 
 */
 
void lupdcmp(double**a, int32_t n, int32_t *p, int32_t spd)
{
	#if(MATHACCEL == MA_INTEL)
	int32_t info = 0;
	if (!spd)
		dgetrf(&n, &n, a[0], &n, p, &info);
	else	
		dpotrf("U", &n, a[0], &n, &info);
	assert(info == 0);	
	#elif(MATHACCEL == MA_AMD)
	int32_t info = 0;
	if (!spd)
		dgetrf_(&n, &n, a[0], &n, p, &info);
	else	
		dpotrf_("U", &n, a[0], &n, &info, 1);
	assert(info == 0);	
	#elif(MATHACCEL == MA_APPLE)
	int32_t info = 0;
	if (!spd)
		dgetrf_((__CLPK_integer *)&n, (__CLPK_integer *)&n, a[0], \
				(__CLPK_integer *)&n, (__CLPK_integer *)p, \
				(__CLPK_integer *)&info);
	else	
		dpotrf_("U", (__CLPK_integer *)&n, a[0], (__CLPK_integer *)&n, \
				(__CLPK_integer *)&info);
	assert(info == 0);	
	#elif(MATHACCEL == MA_SUN)
	int32_t info = 0;
	if (!spd)
		dgetrf_(&n, &n, a[0], &n, p, &info);
	else	
		dpotrf_("U", &n, a[0], &n, &info);
	assert(info == 0);	
	#else
	int32_t i, j, k, pivot=0;
	double max = 0;

	/* start with identity permutation	*/
	for (i=0; i < n; i++)
		p[i] = i;

	for (k=0; k < n-1; k++)	 {
		max = 0;
		for (i = k; i < n; i++)	{
			if (fabs(a[i][k]) > max) {
				max = fabs(a[i][k]);
				pivot = i;
			}
		}	
		if (eq (max, 0))
			fatal ("singular matrix in lupdcmp\n");

		/* bring pivot element to position	*/
		swap_ival (&p[k], &p[pivot]);
		for (i=0; i < n; i++)
			swap_dval (&a[k][i], &a[pivot][i]);

		for (i=k+1; i < n; i++) {
			a[i][k] /= a[k][k];
			for (j=k+1; j < n; j++)
				a[i][j] -= a[i][k] * a[k][j];
		}
	}
	#endif
}

/* 
 * the matrix a is an in-place lower/upper triangular matrix
 * the following macros split them into their constituents
 */

#define LOWER(a, i, j)		((i > j) ? a[i][j] : 0)
#define UPPER(a, i, j)		((i <= j) ? a[i][j] : 0)

/* 
 * LU forward and backward substitution from the pseudocode given
 * in the CLR 'Introduction to Algorithms' textbook. It solves ax = b
 * where, 'a' is an in-place lower/upper triangular matrix. The vector
 * 'x' carries the solution vector. 'p' is the permutation vector. The
 * 'spd' flag indicates that 'a' is symmetric and positive definite
 */

void lusolve(double **a, int32_t n, int32_t *p, double *b, double *x, int32_t spd)
{
	#if(MATHACCEL == MA_INTEL)
	int32_t one = 1, info = 0;
	cblas_dcopy(n, b, 1, x, 1);
	if (!spd)
		dgetrs("T", &n, &one, a[0], &n, p, x, &n, &info);
	else	
		dpotrs("U", &n, &one, a[0], &n, x, &n, &info);
	assert(info == 0);	
	#elif(MATHACCEL == MA_AMD)
	int32_t one = 1, info = 0;
	dcopy(n, b, 1, x, 1);
	if (!spd)
		dgetrs_("T", &n, &one, a[0], &n, p, x, &n, &info, 1);
	else	
		dpotrs_("U", &n, &one, a[0], &n, x, &n, &info, 1);
	assert(info == 0);	
	#elif(MATHACCEL == MA_APPLE)
	int32_t one = 1, info = 0;
	cblas_dcopy(n, b, 1, x, 1);
	if (!spd)
		dgetrs_("T", (__CLPK_integer *)&n, (__CLPK_integer *)&one, a[0], \
				(__CLPK_integer *)&n, (__CLPK_integer *)p, x, \
				(__CLPK_integer *)&n, (__CLPK_integer *)&info);
	else	
		dpotrs_("U", (__CLPK_integer *)&n, (__CLPK_integer *)&one, a[0], \
				(__CLPK_integer *)&n, x, (__CLPK_integer *)&n, \
				(__CLPK_integer *)&info);
	assert(info == 0);	
	#elif(MATHACCEL == MA_SUN)
	int32_t one = 1, info = 0;
	dcopy(n, b, 1, x, 1);
	if (!spd)
		dgetrs_("T", &n, &one, a[0], &n, p, x, &n, &info);
	else	
		dpotrs_("U", &n, &one, a[0], &n, x, &n, &info);
	assert(info == 0);	
	#else
	int32_t i, j;
	double *y = dvector (n);
	double sum;

	/* forward substitution	- solves ly = pb	*/
	for (i=0; i < n; i++) {
		for (j=0, sum=0; j < i; j++)
			sum += y[j] * LOWER(a, i, j);
		y[i] = b[p[i]] - sum;
	}

	/* backward substitution - solves ux = y	*/
	for (i=n-1; i >= 0; i--) {
		for (j=i+1, sum=0; j < n; j++)
			sum += x[j] * UPPER(a, i, j);
		x[i] = (y[i] - sum) / UPPER(a, i, i);
	}

	free_dvector(y);
	#endif
}

/* core of the 4th order Runge-Kutta method, where the Euler step
 * (y(n+1) = y(n) + k1 where k1 = h * dydx(n)) is provided as an input.
 * Given values for y, and k1, advances the solution over an interval h, 
 * and returns the solution in yout. For details, see the discussion in 
 * "Numerical Recipes in C", Chapter 16, from 
 * http://www.library.cornell.edu/nr/bookcpdf/c16-0.pdf
 */
void rk4_core(double **c, double *y, double *k1, double *pow, int32_t n, double h, double *yout)
{
	int32_t i,j;
	double *t, *k2, *k3, *k4;
	k2 = dvector(n);
	k3 = dvector(n);
	k4 = dvector(n);
	#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
	/* k2 = k1	*/
	cblas_dcopy(n, k1, 1, k2, 1);
	/* k3 = k1	*/
	cblas_dcopy(n, k1, 1, k3, 1);
	/* k4 = k1	*/
	cblas_dcopy(n, k1, 1, k4, 1);
	/* k2 = k2 - h/2*c*k1 = k1 - h/2*c*k1 */
	cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, -h/2.0, c[0], \
				n, k1, 1, 1.0, k2, 1);
	/* k3 = k3 - h/2*c*k2	 = k1 - h/2*c*k2 */
	cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, -h/2.0, c[0], \
				n, k2, 1, 1.0, k3, 1);
	/* k4 = k4 - h*c*k3	 = k1 - h*c*k3 */
	cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, -h, c[0], \
				n, k3, 1, 1.0, k4, 1);
	/* yout = y	*/			
	cblas_dcopy(n, y, 1, yout, 1);
	/* yout = yout + k1/6	*/
	cblas_daxpy(n, 1.0/6.0, k1, 1, yout, 1);
	/* yout = yout + k2/3	*/
	cblas_daxpy(n, 1.0/3.0, k2, 1, yout, 1);
	/* yout = yout + k3/3	*/
	cblas_daxpy(n, 1.0/3.0, k3, 1, yout, 1);
	/* yout = yout + k4/6	*/
	cblas_daxpy(n, 1.0/6.0, k4, 1, yout, 1);
	#elif (MATHACCEL == MA_AMD || MATHACCEL == MA_SUN)
	/* k2 = k1	*/
	dcopy(n, k1, 1, k2, 1);
	/* k3 = k1	*/
	dcopy(n, k1, 1, k3, 1);
	/* k4 = k1	*/
	dcopy(n, k1, 1, k4, 1);
	/* k2 = k2 - h/2*c*k1 = k1 - h/2*c*k1 */
	dgemv('T', n, n, -h/2.0, c[0], n, k1, 1, 1.0, k2, 1);
	/* k3 = k3 - h/2*c*k2	 = k1 - h/2*c*k2 */
	dgemv('T', n, n, -h/2.0, c[0], n, k2, 1, 1.0, k3, 1);
	/* k4 = k4 - h*c*k3	 = k1 - h*c*k3 */
	dgemv('T', n, n, -h, c[0], n, k3, 1, 1.0, k4, 1);
	/* yout = y	*/			
	dcopy(n, y, 1, yout, 1);
	/* yout = yout + k1/6	*/
	daxpy(n, 1.0/6.0, k1, 1, yout, 1);
	/* yout = yout + k2/3	*/
	daxpy(n, 1.0/3.0, k2, 1, yout, 1);
	/* yout = yout + k3/3	*/
	daxpy(n, 1.0/3.0, k3, 1, yout, 1);
	/* yout = yout + k4/6	*/
	daxpy(n, 1.0/6.0, k4, 1, yout, 1);
	#else
	t = dvector(n);

	/* k2 = k1 - h/2*c*k1	*/
	for (i = 0; i < n; i++) {
		t[i] = 0.0;
		for (j = 0; j < n; j++) 
			t[i] += h/2.0*c[i][j]*k1[j];
		k2[i] = k1[i] - t[i];
	}

	/* k3 = k1 - h/2*c*k2	*/
	for (i = 0; i < n; i++) {
		t[i] = 0.0;
		for (j = 0; j < n; j++) 
			t[i] += h/2.0*c[i][j]*k2[j];
		k3[i] = k1[i] - t[i];
	}

	/* k4 = k1 - h*c*k3	*/
	for (i =0; i < n; i++) {
		t[i]=0.0;
		for (j = 0; j < n; j++) 
			t[i] += h*c[i][j]*k3[j];
		k4[i] = k1[i] - t[i];
	}

	/* yout = y + k1/6 + k2/3 + k3/3 + k4/6	*/
	for (i =0; i < n; i++) 
		yout[i] = y[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i])/6.0;

	free_dvector(t);
	#endif

	free_dvector(k2);
	free_dvector(k3);
	free_dvector(k4);
}

/* 
 * 4th order Runge Kutta solver	with adaptive step sizing.
 * It integrates and solves the ODE dy + cy = p between
 * t and t+h. It returns the correct step size to be used 
 * next time.
 */
#define RK4_SAFETY		0.95
#define RK4_MAXUP		5.0
#define RK4_MAXDOWN		10.0
#define RK4_PRECISION	0.01
double rk4(double **c, double *y, double *p, int32_t n, double h, double *yout)
{
	int32_t i;
	double *k1, *t1, *t2, *ytemp, max, new_h = h;

	k1 = dvector(n);
	t1 = dvector(n);
	t2 = dvector(n);
	ytemp = dvector(n); 

	/* 
	 * find k1 in the Euler step: y(n+1) = y(n) + h * dy
	 * where, k1 = h * dy which, for our given equation 
	 * is h*(p-cy)
	 */
	#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
	/* k1 = p	*/
	cblas_dcopy(n, p, 1, k1, 1);
	/* k1 = h*k1 - h*c*y = h*p - h*c*y */
	cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, -h, c[0], \
				n, y, 1, h, k1, 1);
	#elif (MATHACCEL == MA_AMD || MATHACCEL == MA_SUN)
	/* k1 = p	*/
	dcopy(n, p, 1, k1, 1);
	/* k1 = h*k1 - h*c*y = h*p - h*c*y */
	dgemv('T', n, n, -h, c[0], n, y, 1, h, k1, 1);
	#else
	matvectmult(t2, c, y, n);
	for (i = 0; i < n; i++)
		k1[i] = h*(p[i]-t2[i]);
	#endif

	/* try until accuracy is achieved	*/
	do {
		h = new_h;

		/* try RK4 once with normal step size	*/
		rk4_core(c, y, k1, p, n, h, ytemp);

		/* repeat it with two half-steps	*/
		rk4_core(c, y, k1, p, n, h/2.0, t1);

		/* y after 1st half-step is in t1. re-evaluate k1 for this	*/
		#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
		/* k1 = p	*/
		cblas_dcopy(n, p, 1, k1, 1);
		/* k1 = h*k1 - h*c*t1 = h*p - h*c*t1 */
		cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, -h, c[0], \
					n, t1, 1, h, k1, 1);
		#elif (MATHACCEL == MA_AMD || MATHACCEL == MA_SUN)
		/* k1 = p	*/
		dcopy(n, p, 1, k1, 1);
		/* k1 = h*k1 - h*c*t1 = h*p - h*c*t1 */
		dgemv('T', n, n, -h, c[0], n, t1, 1, h, k1, 1);
		#else
		matvectmult(t2, c, t1, n);
		for (i = 0; i < n; i++)
			k1[i] = h*(p[i]-t2[i]);
		#endif

		/* get output of the second half-step in t2	*/	
		rk4_core(c, t1, k1, p, n, h/2.0, t2);

		/* find the max diff between these two results:
		 * use t1 to store the diff
		 */
		#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
		/* t1 = ytemp	*/
		cblas_dcopy(n, ytemp, 1, t1, 1);
		/* t1 = -1 * t2 + t1 = ytemp - t2	*/
		cblas_daxpy(n, -1.0, t2, 1, t1, 1);
		/* max = |t1[max_abs_index]|	*/
		max = fabs(t1[cblas_idamax(n, t1, 1)]);
		#elif (MATHACCEL == MA_AMD || MATHACCEL == MA_SUN)
		/* t1 = ytemp	*/
		dcopy(n, ytemp, 1, t1, 1);
		/* t1 = -1 * t2 + t1 = ytemp - t2	*/
		daxpy(n, -1.0, t2, 1, t1, 1);
		/* 
		 * max = |t1[max_abs_index]| note: FORTRAN BLAS
		 * indices start from 1 as opposed to CBLAS where
		 * indices start from 0
		 */
		max = fabs(t1[idamax(n, t1, 1)-1]);
		#else
		for(i=0; i < n; i++)
			t1[i] = fabs(ytemp[i] - t2[i]);
		max = t1[0];
		for(i=1; i < n; i++)
			if (max < t1[i])
				max = t1[i];
		#endif

		/* 
		 * compute the correct step size: see equation 
		 * 16.2.10  in chapter 16 of "Numerical Recipes
		 * in C"
		 */
		/* accuracy OK. increase step size	*/
		if (max <= RK4_PRECISION) {
			new_h = RK4_SAFETY * h * pow(fabs(RK4_PRECISION/max), 0.2);
			if (new_h > RK4_MAXUP * h)
				new_h = RK4_MAXUP * h;
		/* inaccuracy error. decrease step size	and compute again */
		} else {
			new_h = RK4_SAFETY * h * pow(fabs(RK4_PRECISION/max), 0.25);
			if (new_h < h / RK4_MAXDOWN)
				new_h = h / RK4_MAXDOWN;
		}

	} while (new_h < h);

	/* commit ytemp to yout	*/
	#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
	cblas_dcopy(n, ytemp, 1, yout, 1);
	#elif (MATHACCEL == MA_AMD || MATHACCEL == MA_SUN)
	dcopy(n, ytemp, 1, yout, 1);
	#else
	copy_dvector(yout, ytemp, n);
	#endif

	/* clean up */
	free_dvector(k1);
	free_dvector(t1);
	free_dvector(t2);
	free_dvector(ytemp);

	/* return the step-size	*/
	return new_h;
}

/* matmult: C = AB, A, B are n x n square matrices	*/
void matmult(double **c, double **a, double **b, int32_t n) 
{
	#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, \
				n, n, n, 1.0, a[0], n, b[0], n, 0.0, c[0], n);
	#elif (MATHACCEL == MA_AMD  || MATHACCEL == MA_SUN)
	/* B^T * A^T = (A * B)^T	*/
	dgemm('N', 'N', n, n, n, 1.0, b[0], n, a[0], n, 0.0, c[0], n);
	#else
	int32_t i, j, k;

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++) {
			c[i][j] = 0;
			for (k = 0; k < n; k++)
				c[i][j] += a[i][k] * b[k][j];
		}
	#endif	
}

/* same as above but 'a' is a diagonal matrix stored as a 1-d array	*/
void diagmatmult(double **c, double *a, double **b, int32_t n) 
{
	int32_t i;
	#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
	zero_dmatrix(c, n, n);
	for(i=0; i < n; i++)
		cblas_daxpy(n, a[i], b[i], 1, c[i], 1);
	#elif (MATHACCEL == MA_AMD  || MATHACCEL == MA_SUN)
	zero_dmatrix(c, n, n);
	for(i=0; i < n; i++)
		daxpy(n, a[i], b[i], 1, c[i], 1);
	#else
	int32_t j;

	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			c[i][j] = a[i] * b[i][j];
	#endif		
}

/* mult of an n x n matrix and an n x 1 column vector	*/
void matvectmult(double *vout, double **m, double *vin, int32_t n)
{
	#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
	cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, 1.0, m[0], \
				n, vin, 1, 0.0, vout, 1);
	#elif (MATHACCEL == MA_AMD  || MATHACCEL == MA_SUN)
	dgemv('T', n, n, 1.0, m[0], n, vin, 1, 0.0, vout, 1);
	#else
	int32_t i, j;

	for (i = 0; i < n; i++) {
		vout[i] = 0;
		for (j = 0; j < n; j++)
			vout[i] += m[i][j] * vin[j];
	}
	#endif
}

/* same as above but 'm' is a diagonal matrix stored as a 1-d array	*/
void diagmatvectmult(double *vout, double *m, double *vin, int32_t n)
{
	#if (MATHACCEL == MA_INTEL || MATHACCEL == MA_APPLE)
	cblas_dsbmv(CblasRowMajor, CblasUpper, n, 0, 1.0, m, 1, vin, \
				1, 0.0, vout, 1);
	#elif (MATHACCEL == MA_AMD  || MATHACCEL == MA_SUN)
	dsbmv('U', n, 0, 1.0, m, 1, vin, 1, 0.0, vout, 1);
	#else
	int32_t i;

	for (i = 0; i < n; i++)
		vout[i] = m[i] * vin[i];
	#endif
}

/* 
 * inv = m^-1, inv, m are n by n matrices.
 * the spd flag indicates that m is symmetric 
 * and positive definite 
 */
#define BLOCK_SIZE		256
void matinv(double **inv, double **m, int32_t n, int32_t spd)
{
	int32_t *p, info = 0, lwork;
	double *work;
	p = ivector(n);
	lwork = n * BLOCK_SIZE;
	work = dvector(lwork);

	#if (MATHACCEL == MA_INTEL)
	cblas_dcopy(n*n, m[0], 1, inv[0], 1);
	if (!spd) {
		dgetrf(&n, &n, inv[0], &n, p, &info);
		assert(info == 0);
		dgetri(&n, inv[0], &n, p, work, &lwork, &info);
		assert(info == 0);
	} else {
		dpotrf("U", &n, inv[0], &n, &info);
		assert(info == 0);
		dpotri("U", &n, inv[0], &n, &info);
		assert(info == 0);
		mirror_dmatrix(inv, n);
	}	
	#elif(MATHACCEL == MA_AMD)
	dcopy(n*n, m[0], 1, inv[0], 1);
	if (!spd) {
		dgetrf_(&n, &n, inv[0], &n, p, &info);
		assert(info == 0);
		dgetri_(&n, inv[0], &n, p, work, &lwork, &info);
		assert(info == 0);
	} else {
		dpotrf_("U", &n, inv[0], &n, &info, 1);
		assert(info == 0);
		dpotri_("U", &n, inv[0], &n, &info, 1);
		assert(info == 0);
		mirror_dmatrix(inv, n);
	}	
	#elif (MATHACCEL == MA_APPLE)
	cblas_dcopy(n*n, m[0], 1, inv[0], 1);
	if (!spd) {
		dgetrf_((__CLPK_integer *)&n, (__CLPK_integer *)&n, \
				inv[0], (__CLPK_integer *)&n, (__CLPK_integer *)p, \
				(__CLPK_integer *)&info);
		assert(info == 0);
		dgetri_((__CLPK_integer *)&n, inv[0], (__CLPK_integer *)&n, \
				(__CLPK_integer *)p, work, (__CLPK_integer *)&lwork, \
				(__CLPK_integer *)&info);
		assert(info == 0);
	} else {
		dpotrf_("U", (__CLPK_integer *)&n, inv[0], (__CLPK_integer *)&n, \
				(__CLPK_integer *)&info);
		assert(info == 0);
		dpotri_("U", (__CLPK_integer *)&n, inv[0], (__CLPK_integer *)&n, \
				(__CLPK_integer *)&info);
		assert(info == 0);
		mirror_dmatrix(inv, n);
	}
	#elif(MATHACCEL == MA_SUN)
	dcopy(n*n, m[0], 1, inv[0], 1);
	if (!spd) {
		dgetrf_(&n, &n, inv[0], &n, p, &info);
		assert(info == 0);
		dgetri_(&n, inv[0], &n, p, work, &lwork, &info);
		assert(info == 0);
	} else {
		dpotrf_("U", &n, inv[0], &n, &info);
		assert(info == 0);
		dpotri_("U", &n, inv[0], &n, &info);
		assert(info == 0);
		mirror_dmatrix(inv, n);
	}
	#else
	int32_t i, j;
	double *col;

	col = dvector(n);

	lupdcmp(m, n, p, spd);

	for (j = 0; j < n; j++) {
		for (i = 0; i < n; i++) col[i]=0.0;
		col[j] = 1.0;
		lusolve(m, n, p, col, work, spd);
		for (i = 0; i < n; i++) inv[i][j]=work[i];
	}

	free_dvector(col);
	#endif

	free_ivector(p);
	free_dvector(work);
}
