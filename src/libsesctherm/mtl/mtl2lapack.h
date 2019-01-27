/* -*- c++ -*- */
//
// Software License for MTL
//
// Copyright (c) 2001-2005 The Trustees of Indiana University. All rights reserved.
// Copyright (c) 1998-2001 University of Notre Dame. All rights reserved.
// Authors: Andrew Lumsdaine, Jeremy G. Siek, Lie-Quan Lee
//
// This file is part of the Matrix Template Library
//
// See also license.mtl.txt in the distribution.

#ifndef MTL_LAPACK_H
#define MTL_LAPACK_H

#include <iostream>

#include "mtl_complex.h"
#include "lapack_interface.h"
#include "matrix.h"

#define MTL_FCALL(x) x##_

namespace mtl_lapack_dispatch {

/*
 *  Dispatch routines used by the mtl2lapack functions
 *  (mtl2lapack functions are below)
 */

  inline void gecon (const char & norm, const int & n,
		     const double da[], const int & lda,
		     const double & danorm, double & drcond,
		     int & info)
  {
    double* dwork  = new double[4*n];
    int*    iwork2 = new int[n];
    MTL_FCALL(dgecon)(norm, n, da, lda, danorm, drcond, dwork, iwork2, info);
    delete [] dwork;
    delete [] iwork2;
  }

  inline void gecon (const char & norm, const int & n,
		     const float sa[], const int & lda,
		     const float & sanorm, float & srcond,
		     int & info)
  {
    float* swork  = new float[4*n];
    int*   iwork2 = new int[n];
    MTL_FCALL(sgecon)(norm, n, sa, lda, sanorm, srcond, swork, iwork2, info);
    delete [] swork;
    delete [] iwork2;
  }

  inline void gecon(const char & norm, const int & n,
		    const std::complex<double> za[], const int & lda,
		    const double & danorm, double & drcond,
		    int & info)
  {
    std::complex<double>* zwork  = new std::complex<double>[2*n];
    double*          dwork2 = new double[2*n];
    MTL_FCALL(zgecon)(norm, n, za, lda, danorm, drcond, zwork, dwork2, info);
    delete [] zwork;
    delete [] dwork2;
  }

  inline void gecon (const char & norm, const int & n,
		     const std::complex<float> ca[], const int & lda,
		     const float & canorm, float & crcond, int & info)
  {
    std::complex<float>* cwork  = new std::complex<float>[2*n];
    float*          cwork2 = new float[2*n];
    MTL_FCALL(cgecon)(norm, n, ca, lda, canorm, crcond, cwork, cwork2, info);
    delete [] cwork;
    delete [] cwork2;
  }

  inline void geev(const char& jobvl, const char& jobvr, const int&  n,
		   double da[], const int& lda, std::complex<double> zw[],
		   double dvl[], const int& ldvl, double dvr[],
		   const int& ldvr, int& info)
  {
    int     ldwork = 4*n+1;
    double* dwork  = new double[ldwork];
    double* dwr    = new double[n];
    double* dwi    = new double[n];
    MTL_FCALL(dgeev)(jobvl, jobvr, n, da, lda, dwr, dwi, dvl, ldvl, dvr, ldvr, dwork,
	   ldwork, info);
    /*assemble dwr and dwi into zw */
    for (int i = 0; i < n; i++)
      zw[i] = std::complex<double>(dwr[i],dwi[i]);
    delete [] dwork;
    delete [] dwr;
    delete [] dwi;
  }

  inline void geev(const char& jobvl, const char& jobvr, const int&  n,
		   float sa[], const int& lda, std::complex<float> cw[],
		   float svl[], const int& ldvl, float svr[],
		   const int& ldvr, int& info)
  {
    int     ldwork = 4*n+1;
    float*  swork  = new float[ldwork];
    float*  swr    = new float[n];
    float*  swi    = new float[n];

    MTL_FCALL(sgeev)(jobvl, jobvr, n, sa, lda, swr, swi, svl, ldvl, svr, ldvr,
	   swork, ldwork, info);
    /* assemble dwr and dwi into zw */
    for (int i = 0; i < n; i++)
      cw[i] = std::complex<float>(swr[i],swi[i]);
    delete [] swork;
    delete [] swr;
    delete [] swi;
  }

  inline void geev(const char& jobvl, const char& jobvr, const int&  n,
		   std::complex<double> za[], const int& lda,
		   std::complex<double> zw[], std::complex<double> zvl[],
		   const int& ldvl, std::complex<double> zvr[],
		   const int& ldvr, int& info)
  {
    int              ldwork  = 1+2*n;
    std::complex<double>* zwork   = new std::complex<double>[ldwork];
    double*          dwork2  = new double[2*n];
    MTL_FCALL(zgeev)(jobvl, jobvr, n, za, lda, zw, zvl, ldvl, zvr, ldvr, zwork, ldwork,
	   dwork2, info);
    delete [] zwork;
    delete [] dwork2;
  }

  inline void geev(const char& jobvl, const char& jobvr, const int&  n,
		   std::complex<float> ca[], const int& lda,
		   std::complex<float> cw[], std::complex<float> cvl[],
		   const int& ldvl, std::complex<float> cvr[],
		   const int& ldvr, int& info)
  {
    int              ldwork  = 1+2*n;
    std::complex<float>*  cwork   = new std::complex<float>[ldwork];
    float*           swork2  = new float[2*n];
    MTL_FCALL(cgeev)(jobvl, jobvr, n, ca, lda, cw, cvl, ldvl, cvr, ldvr, cwork, ldwork,
	   swork2, info);
    delete [] cwork;
    delete [] swork2;
  }

  inline void geqpf(const int & m, const int & n,
		    double da[], const int & lda,
		    int jpivot[], double dtau[],
		    int & info)
  {
    double* dwork = new double[3*n];
    MTL_FCALL(dgeqpf)(m, n, da, lda, jpivot, dtau, dwork, info);
    delete [] dwork;
  }

  inline void geqpf(const int & m, const int & n,
		    float sa[], const int & lda,
		    int jpivot[], float stau[],
		    int & info)
  {
    float* swork = new float[3*n];
    MTL_FCALL(sgeqpf)(m, n, sa, lda, jpivot, stau, swork, info);
    delete [] swork;
  }

  inline void geqpf(const int & m, const int & n,
		    std::complex<double> za[], const int & lda,
		    int jpivot[], std::complex<double> ztau[],
		    int & info)
  {
    std::complex<double>* zwork = new std::complex<double>[n];
    double * dwork2 = new double[2*n];
    MTL_FCALL(zgeqpf)(m, n, za, lda, jpivot, ztau, zwork, dwork2, info);
    delete [] zwork;
    delete [] dwork2;
  }

  inline void geqpf(const int & m, const int & n,
		    std::complex<float> ca[], const int & lda,
		    int jpivot[], std::complex<float> ctau[],
		    int & info)
  {
    std::complex<float>* cwork = new std::complex<float>[n];
    float * swork2 = new float[2*n];
    MTL_FCALL(cgeqpf)(m, n, ca, lda, jpivot, ctau, cwork, swork2, info);
    delete [] swork2;
  }


  inline void geqrf(const int & m, const int & n,
		    double da[], const int & lda,
		    double dtau[], int & info)
  {
    int ldwork = n*n;
    double* dwork   = new double[ldwork];
    MTL_FCALL(dgeqrf)(m, n, da, lda, dtau, dwork, ldwork, info);
    delete [] dwork;
  }

  inline void geqrf(const int & m, const int & n,
		    float sa[], const int & lda,
		    float stau[], int & info)
  {
    int ldwork = 4*n;
    float* swork = new float[ldwork];
    MTL_FCALL(sgeqrf)(m, n, sa, lda, stau, swork, ldwork, info);
    delete [] swork;
  }

  inline void geqrf(const int & m, const int & n,
		    std::complex<double> za[], const int & lda,
		    std::complex<double> ztau[], int & info)
  {
    int ldwork = 4*n;
    std::complex<double>* zwork = new std::complex<double>[ldwork];
    MTL_FCALL(zgeqrf)(m, n, za, lda, ztau, zwork, ldwork, info);
    delete [] zwork;
  }

  inline void geqrf(const int & m, const int & n,
		    std::complex<float> ca[], const int & lda,
		    std::complex<float> ctau[], int & info)
  {
    int ldwork = 4*n;
    std::complex<float>* cwork = new std::complex<float>[ldwork];
    MTL_FCALL(cgeqrf)(m, n, ca, lda, ctau, cwork, ldwork, info);
    delete [] cwork;
  }

  inline void gesv(const int & n, const int & nrhs,
		   double da[], const int & lda, int ipivot[],
		   double db[], const int & ldb, int & info)
  {
    MTL_FCALL(dgesv)(n, nrhs, da, lda, ipivot, db, ldb, info);
  }

  inline void gesv(const int & n, const int & nrhs,
		   float sa[], const int & lda,
		   int ipivot[], float sb[],
		   const int & ldb, int & info)
  {
    MTL_FCALL(sgesv)(n, nrhs, sa, lda, ipivot, sb, ldb, info);
  }

  inline void gesv(const int & n, const int & nrhs,
		   std::complex<double> za[], const int & lda,
		   int ipivot[], std::complex<double> zb[],
		   const int & ldb, int & info)
  {
    MTL_FCALL(zgesv)(n, nrhs, za, lda, ipivot, zb, ldb, info);
  }

  inline void gesv(const int & n, const int & nrhs,
		   std::complex<float> ca[], const int & lda,
		   int ipivot[], std::complex<float> cb[],
		   const int & ldb, int & info)
  {
    MTL_FCALL(cgesv)(n, nrhs, ca, lda, ipivot, cb, ldb, info);
  }


  inline void getrf (const int& m, const int& n,
		     double da[], const int& lda,
		     int ipivot[], int& info)
  {
    MTL_FCALL(dgetrf)(m, n, da, lda, ipivot, info);
  }

  inline void getrf (const int& m, const int& n,
		     float sa[], const int& lda,
		     int ipivot[], int& info)
  {
    MTL_FCALL(sgetrf)(m, n, sa, lda, ipivot, info);
  }

  inline void getrf (const int& m, const int& n,
		     std::complex<double> za[], const int& lda,
		     int ipivot[], int& info)
  {
    MTL_FCALL(zgetrf)(m, n, za, lda, ipivot, info);
  }

  inline void getrf (const int& m, const int& n,
		     std::complex<float> ca[], const int& lda,
		     int ipivot[], int& info)
  {
    MTL_FCALL(cgetrf)(m, n, ca, lda, ipivot, info);
  }

  inline void getrs (const char& transa, const int& n,
		     const int& nrhs, const double da[],
		     const int& lda, int ipivot[],
		     double db[], const int& ldb,
		     int& info)

  {
    MTL_FCALL(dgetrs)(transa, n, nrhs, da, lda, ipivot, db, ldb, info);
  }

  inline void getrs (const char& transa, const int& n,
		     const int& nrhs, const float  sa[],
		     const int& lda, int ipivot[],
		     float sb[], const int& ldb,
		     int& info)

  {
    MTL_FCALL(sgetrs)(transa, n, nrhs, sa, lda, ipivot, sb, ldb, info);
  }

  inline void getrs (const char& transa, const int& n,
		     const int& nrhs, const std::complex<double> za[],
		     const int& lda, int ipivot[],
		     std::complex<double> zb[], const int& ldb,
		     int& info)

  {
    MTL_FCALL(zgetrs)(transa, n, nrhs, za, lda, ipivot, zb, ldb, info);
  }

  inline void getrs (const char& transa, const int& n,
		     const int& nrhs, const std::complex<float> ca[],
		     const int& lda, int ipivot[],
		     std::complex<float> cb[], const int& ldb,
		     int& info)

  {
    MTL_FCALL(cgetrs)(transa, n, nrhs, ca, lda, ipivot, cb, ldb, info);
  }


  inline void geequ (const int& m, const int& n,
		     const double da[], const int& lda,
		     double r[], double c[],
		     double& rowcnd, double& colcnd,
		     double& amax, int& info)
  {
    MTL_FCALL(dgeequ)(m, n, da, lda, r, c, rowcnd, colcnd, amax, info);
  }

  inline void geequ(const int& m, const int& n,
		    const float da[], const int& lda,
		    float r[], float c[], float& rowcnd,
		    float& colcnd,
		    float& amax, int& info)
  {
    MTL_FCALL(sgeequ)(m, n, da, lda, r,  c, rowcnd, colcnd, amax, info);
  }

  inline void geequ(const int& m, const int& n,
		    const std::complex<float> da[],
		    const int& lda, std::complex<float> r[],
		    std::complex<float> c[],
		    float& rowcnd, float& colcnd,
		    float& amax, int& info)
  {
    MTL_FCALL(cgeequ)(m, n, da, lda, r,  c, rowcnd, colcnd, amax, info);
  }

  inline void geequ(const int& m, const int& n,
		    const std::complex<double> da[],
		    const int& lda, std::complex<double> r[],
		    std::complex<double> c[],
		    double& rowcnd, double& colcnd,
		    double& amax, int& info)
  {
    MTL_FCALL(zgeequ)(m, n, da, lda, r,  c, rowcnd, colcnd, amax, info);
  }



  inline void gelqf(const int& m, const int& n,
		    double da[], const int& lda,
		    double dtau[], int& info)
  {
    int ldwork = 2*m;
    double* work = new double[ldwork];
    MTL_FCALL(dgelqf)(m, n, da, lda, dtau, work, ldwork, info);
    delete [] work;
  }

  inline void gelqf(const int& m, const int& n,
		    float da[], const int& lda,
		    float dtau[], int& info)
  {
    int ldwork = 2*m;
    float* work = new float[ldwork];
    MTL_FCALL(sgelqf)(m, n, da, lda, dtau, work, ldwork, info);
    delete [] work;
  }

  inline void gelqf(const int& m, const int& n,
		    std::complex<float> da[], const int& lda,
		    std::complex<float> dtau[], int& info)
  {
    int ldwork = 2*m;
    std::complex<float>* work = new std::complex<float>[ldwork];
    MTL_FCALL(cgelqf)(m, n, da, lda, dtau, work, ldwork, info);
    delete [] work;
  }

  inline void gelqf(const int& m, const int& n,
		    std::complex<double> da[], const int& lda,
		    std::complex<double> dtau[], int& info)
  {
    int ldwork = 2*m;
    std::complex<double>* work = new std::complex<double>[ldwork];
    MTL_FCALL(zgelqf)(m, n, da, lda, work, dtau, ldwork, info);
    delete [] work;
  }


  inline void gelss(const int& m, const int& n, const int& nrhs,
		    double da[], const int& lda, double b[], const int& ldb,
		    const double& rcond, int& rank, int& info)
  {
    int ldwork = 5*MTL_MAX(m,n);
    double* work = new double[ldwork];
    double* s = new double[ldwork];
    MTL_FCALL(dgelss)(m, n, nrhs, da, lda, b, ldb, s, rcond, rank, work, ldwork, info);
    delete [] work;
    delete [] s;
  }

  inline void gelss(const int& m, const int& n, const int& nrhs,
		    float da[], const int& lda, float b[], const int& ldb,
		    const float& rcond, int& rank, int& info)
  {
    int ldwork = 5*MTL_MAX(m,n);
    float* work = new float[ldwork];
    float* s = new float[ldwork];
    MTL_FCALL(sgelss)(m, n, nrhs, da, lda, b, ldb, s, rcond, rank, work, ldwork, info);
    delete [] work;
    delete [] s;
  }


  inline void gelss(const int& m, const int& n, const int& nrhs,
		    std::complex<float> da[], const int& lda,
		    std::complex<float> b[], const int& ldb,
		    const float& rcond, int& rank, int& info)
  {
    int ldwork = 5*MTL_MAX(m,n);
    std::complex<float>* work = new std::complex<float>[ldwork];
    std::complex<float>* s = new std::complex<float>[ldwork];
    MTL_FCALL(cgelss)(m, n, nrhs, da, lda, b, ldb, s, rcond, rank, work, ldwork, info);
    delete [] work;
    delete [] s;
  }


  inline void gelss(const int& m, const int& n, const int& nrhs,
		    std::complex<double> da[], const int& lda,
		    std::complex<double> b[], const int& ldb,
		    const double& rcond, int& rank, int& info)
  {
    int ldwork = 5*MTL_MAX(m,n);
    std::complex<double>* work = new std::complex<double>[ldwork];
    std::complex<double>* s = new std::complex<double>[ldwork];
    MTL_FCALL(zgelss)(m, n, nrhs, da, lda, b, ldb, s, rcond, rank, work, ldwork, info);
    delete [] work;
    delete [] s;
  }


  inline void orglq(const int& m, const int& n, const int& k,
		    double da[], int& lda, double dtau[], int& info)
  {
    int ldwork = 2*m;
    double* work = new double[ldwork];
    MTL_FCALL(dorglq)(m, n, k, da, lda, dtau, work, ldwork, info);
    delete [] work;
  }

  inline void orglq(const int& m, const int& n, const int& k,
		    float da[], int& lda, float dtau[], int& info)
  {
    int ldwork = 2*m;
    float* work = new float[ldwork];
    MTL_FCALL(sorglq)(m, n, k, da, lda, dtau, work, ldwork, info);
    delete [] work;
  }

  inline void orgqr(const int& m, const int& n, const int& k,
		    double da[], int& lda, double dtau[], int& info)
  {
    int ldwork = 2*m;
    double* work = new double[ldwork];
    MTL_FCALL(dorgqr)(m, n, k, da, lda, dtau, work, ldwork, info);
    delete [] work;
  }

  inline void orgqr(const int& m, const int& n, const int& k,
		    float da[], int& lda, float dtau[], int& info)
  {
    int ldwork = 2*m;
    float* work = new float[ldwork];
    MTL_FCALL(sorgqr)(m, n, k, da, lda, dtau, work, ldwork, info);
    delete [] work;
  }

  inline void gesvd(const char& jobu, const char& jobvt, const int& m,
		    const int& n, double da[], const int& lda, double ds[],
		    double du[], const int& ldu, double dvt[],
		    const int& ldvt, int& info)
  {
    //allocate work space
    int lwork = MTL_MAX(3*MTL_MIN(m,n)+MTL_MAX(m,n),5*MTL_MIN(m,n));
    double* work = new double[lwork];

    MTL_FCALL(dgesvd)(jobu, jobvt, m, n, da, lda, ds, du, ldu, dvt, ldvt, work, lwork, info);

    delete [] work;
  }


  inline void gesvd(const char& jobu, const char& jobvt, const int& m,
		    const int& n, float da[], const int& lda, float ds[],
		    float du[], const int& ldu, float dvt[],
		    const int& ldvt, int& info)
  {
    //allocate work space
    int lwork = MTL_MAX(3*MTL_MIN(m,n)+MTL_MAX(m,n),5*MTL_MIN(m,n));
    float* work = new float[lwork];

    MTL_FCALL(sgesvd)(jobu, jobvt, m, n, da, lda, ds, du, ldu, dvt, ldvt, work, lwork, info);

    delete [] work;
  }

  inline void gesvd(const char& jobu, const char& jobvt, const int& m,
		    const int& n, std::complex<double> da[], const int& lda,
		    double ds[],
		    std::complex<double> du[], const int& ldu,
		    std::complex<double> dvt[], const int& ldvt, int& info)
  {
    //allocate work space
    int lwork = 2*MTL_MIN(m,n)+MTL_MAX(m,n);
    std::complex<double>* work = new std::complex<double>[lwork];
    int lrwork = 5*MTL_MIN(m,n);
    double* rwork = new double[lrwork];

    MTL_FCALL(zgesvd)(jobu, jobvt, m, n, da, lda, ds, du, ldu, dvt, ldvt, work, lwork, rwork, info);

    delete [] work;
    delete [] rwork;
  }

  inline void gesvd(const char& jobu, const char& jobvt, const int& m,
		    const int& n, std::complex<float> da[], const int& lda,
		    float ds[],
		    std::complex<float> du[], const int& ldu,
		    std::complex<float> dvt[], const int& ldvt, int& info)
  {
    //allocate work space
    int lwork = 2*MTL_MIN(m,n)+MTL_MAX(m,n);
    std::complex<float>* work = new std::complex<float>[lwork];
    int lrwork = 5*MTL_MIN(m,n);
    float* rwork = new float[lrwork];

    MTL_FCALL(cgesvd)(jobu, jobvt, m, n, da, lda, ds, du, ldu, dvt, ldvt, work, lwork, rwork, info);

    delete [] work;
    delete [] rwork;
  }


} /* namespace mtl lapack dispatch */

namespace mtl2lapack {

  using namespace mtl;

  /* MTL to Lapack Interface*/


  //: Lapack Matrix
  //
  // Use this matrix type constructor to create the type of matrix to
  // use in conjunction with the mtl2lapack functions.
  //
  // <p>The vector type you use with mtl2lapack functions must be
  // contiguous in memory, and have a function data() defined which
  // returns a pointer to that memory, and a function size() with
  // gives the length.
  //
  //!tparam: T - the matrix element type, either float, double, std::complex< float >, or std::complex< double >
  //!tparam: External - Memory managed by MTL? - internal
  //!category: mtl2lapack, generators
  //!component: type
  //!example: getrf.cc, geequ.cc, gecon.cc, geev.cc,
  template <class T, int External=mtl::internal>
  struct lapack_matrix {
    typedef typename matrix<T, rectangle<>,
                            dense<External>, column_major>::type type;
  };


  //: Estimate the reciprocal of the condition number of a general matrix.
  //
  //   Estimate the reciprocal of the condition number of a general matrix
  //   A, with either the one-norm or the infinity-norm. GECON uses the LU
  //   factorization computed by GETRF. Currently MTL only handles column
  //   oriented matrices for this function.
  //
  // <UL>
  // <LI>norm    (IN - <tt>char</tt>) Specifies whether to do one-norm or infinity norm. One of the following is required:
  //
  //   '1' the 1-norm condition number
  //   'I' the infinity-norm condition number
  //
  //   <LI> a       (IN - matrix(M,N)) The coefficient matrix A.
  //
  //   <LI> anorm   (IN - Real number) The one or infinity norm of matrix A, which is of the same numerical type as A.
  //
  //
  //   <LI> rcond  (OUT - Real number) The estimated reciprocal condition of matrix A.
  //
  //
  //   <LI> info   (OUT - <tt>int</tt>)
  //   0     : function completed normally
  //   < 0   : The ith argument, where i = abs(return value) had an illegal value.
  // </UL>
  //
  //!component: function
  //!category: mtl2lapack
  template <class LapackMatA, class Real>
  int gecon(char  _norm,
	    const LapackMatA& a,
	    const Real& _anorm,
	    Real& rcond)
  {
    int             _lda = a.minor();
    int             _n = a.nrows();
    int             _info;

    mtl_lapack_dispatch::gecon(_norm, _n, a.data(), _lda,
			       _anorm, rcond, _info);

    return _info;
  }


enum GEEV_JOBV {GEEV_CALC_LEFT,	GEEV_CALC_RIGHT, GEEV_CALC_BOTH, GEEV_CALC_NONE};


//: Compute the eigenvalues.
//   Compute for an N-by-N non-symmetric matrix A, the eigenvalues, and,  optionally, the left and/or right eigenvectors (simple driver).
// <UL>
//   <LI> jobv    (IN - <tt>int</tt>) Specifies which eigenvectors to solve for (left, right, both, or neither). One of four values:
//   GEEV_CALC_LEFT - function computes left eigenvectors.
//   GEEV_CALC_RIGHT - function computes right eigenvectors.
//   GEEV_CALC_BOTH - function computes both left and right eigenvectors.
//   GEEV_CALC_NONE - function computes neither left nor right eigenvectors.
//
//   <LI> a       (IN/OUT - matrix(M,N)) The coefficient matrix A on entry, and is overwritten on exit.
//
//   <LI> w       (OUT - vector(N))  Std::Complex Vector with same base precision as A. The computed real and imaginary parts of the eigenvectors.
//
//   <LI> vl      (OUT - matrix(N,N)) Matrix of same numerical type as A. Used to store left eigenvectors if they are computed.
//
//   <LI> vr      (OUT - matrix(N,N)) Matrix of same numerical type as A. Used to store right eigenvectors if they are computed.
//
//   <LI> info    (OUT - <tt>int</tt>)
//   0   : function completed normally
//   < 0 : The ith argument, where i = abs(return value) had an illegal value.
//   > 0 : The QR algorithm failed to compute all the eigenvalues and no eigenvectors have been computed.
// </UL>
//
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatA, class LapackMatVL,
            class LapackMatVR, class VectorComplex>
  int geev(int jobv,
	   LapackMatA& a,        /* N x N */
	   VectorComplex& w,     /* N x N */
	   LapackMatVL& vl,  /* LDVL x N */
	   LapackMatVR& vr)  /* LDVR x N */
  {
    char _jobvr, _jobvl;

    int  _info;

    /*determine _jobvr and _jobvl */
    switch (jobv) {
    case 0: /* GEEV_CALC_LEFT: */
      _jobvr = 'N';
      _jobvl = 'V';
      break;
    case 1: /* GEEV_CALC_RIGHT: */
      _jobvr = 'V';
      _jobvl = 'N';
      break;
    case 2:  /* GEEV_CALC_BOTH: */
      _jobvr = 'V';
      _jobvl = 'V';
      break;
    case 3:  /* GEEV_CALC_NONE:*/
      _jobvr = 'N';
      _jobvl = 'N';
      break;
    default:
      assert(0);
    }

    int             _lda = a.minor();
    int             a_n = a.major();
    int             _ldvl = vl.minor();
    int             _ldvr = vr.minor();

    /* make sure matrix is square */
    if (a.nrows() != a.ncols())
      return -100;

    /* make sure result eigenvalue vector is long enough */
    if (int(w.size()) != a_n)
      return -104;

    if (_jobvl == 'V')
      if (int(vl.nrows()) < a_n || int(vl.ncols()) < a_n)
        return -105;

    if (_jobvr == 'V')
      if (int(vr.nrows()) < a_n || int(vr.ncols()) < a_n )
	return -106;

    /* call dispatcher */
    mtl_lapack_dispatch::geev(_jobvl, _jobvr, a_n, a.data(), _lda, w.data(),
                              vl.data(), _ldvl, vr.data(), _ldvr, _info);

    return _info;

  }


  //:  QR Factorization with Column Pivoting.
  //
  //   QR Factorization with Column Pivoting of a MxN General Matrix A.
  // <UL>
  //   <LI> a       (IN/OUT - matrix(M,N)) On entry, the coefficient matrix A. On exit, its upper triangle is the min(M,N)-by-N upper triangular matrix R.  The lower triangle, together with the tau vector, is the orthogonal matrix Q as a product of min(M,N) elementary reflectors.
  //
  //   <LI> jpivot  (IN/OUT - vector(N)) Integer vector. On entry, if JPVT(i) != 0, the i-th column of A is permuted to the front of A*P. If JPVT(i) = 0, the i-th column of A is a free column. On exit, if jpivot(i) = k, then the i-th column of A*P was the k-th column of A.
  //
  //   <LI> tau     (OUT - vector (min(M,N))) Vector of same numerical type as A. The scalar factors of the elementary reflectors.
  //
  //   <LI> info    (OUT - <tt>int</tt>)
  //   0   : function completed normally
  //   < 0 : The ith argument, where i = abs(return value) had an illegal value.
  // </UL>
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatA, class VectorInt, class VectorT>
  int geqpf (LapackMatA & a,
	     VectorInt & jpivot,
	     VectorT & tau)
  {
    typename LapackMatA::value_type * _a = a.get_contiguous();
    int              _m = a.nrows();
    int              _n = a.ncols();
    int              _lda = a.minor();
    int              _info;

    /*make_sure tau's size is greater than or equal to min(m,n)*/
    if (int(tau.size()) < MTL_MIN(_m,_n))
      return -105;

    mtl_lapack_dispatch::geqpf(_m, _n, _a, _lda, jpivot.data(), tau.data(),
                               _info);

    a.set_contiguous(_a);

    return _info;
  }


  //: QR Factorization of a General Matrix
  //
  //   QR Factorization of a MxN General Matrix A.
  // <UL>
  //   <LI> a       (IN/OUT - matrix(M,N)) On entry, the coefficient matrix A. On exit , the upper triangle and diagonal is the min(M,N) by N upper triangular matrix R.  The lower triangle, together with the tau vector, is the orthogonal matrix Q as a product of min(M,N) elementary reflectors.
  //
  //   <LI> tau     (OUT - vector (min(M,N))) Vector of the same numerical type as A. The scalar factors of the elementary reflectors.
  //
  //   <LI> info    (OUT - <tt>int</tt>)
  //   0   : function completed normally
  //   < 0 : The ith argument, where i = abs(return value) had an illegal value.
  // </UL>
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatA,class VectorT>
  int geqrf (LapackMatA& a,
	     VectorT & tau)
  {
    int              _m = a.nrows();
    int              _n = a.ncols();
    int              _lda = a.minor() ;
    int              _info;

    /*make_sure tau's size is greater than or equal to min(m,n)*/
    if (int(tau.size()) < MTL_MIN(_n, _m))
      return -104;

    /*call dispatcher*/
    mtl_lapack_dispatch::geqrf(_m, _n, a.data(), _lda, tau.data(), _info);

    return _info;
  }


  //: Solution to a linear system in a general matrix.
  //
  //   Computes the solution to a real system of linear equations A*X=B
  //   (simple driver).  LU decomposition with partial pivoting and row
  //   interchanges is used to solve the system.
  // <UL>
  //   <LI> a       (IN/OUT - matrix(M,N)) On entry, the coefficient matrix A, and the factors L and U from the factorization A = P*L*U on exit.
  //
  //   <LI> ipivot  (OUT - vector(N)) Integer vector. The row i of A was interchanged with row IPIV(i).
  //
  //   <LI> b        (IN/OUT - matrix(ldb,NRHS)) Matrix of same numerical type as A. On entry, the NxNRHS matrix of the right hand side matrix B. On a successful exit, it is the NxNRHS solution matrix X.
  //
  //   <LI> info     (OUT - <tt>int</tt>)
  //   0     : function completed normally
  //   < 0   : The ith argument, where i = abs(return value) had an illegal value.
  //   > 0   : U(i,i), where i = return value, is exactly zero and U is therefore singular. The LU factorization has been completed, but the solution could not be computed.
  // </UL>
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatA, class LapackMatB, class VectorInt>
  int gesv (LapackMatA& a,
	    VectorInt & ipivot,
	    LapackMatB& b)
  {
    int              _lda = a.minor();
    int              _n = a.ncols();
    int              _nrhs = b.ncols();

    int              _ldb = b.nrows();
    int              _info;

    /* make sure matrices are same size */
    if (int(a.nrows()) != int(b.nrows()))
      return -100;

    /* make sure ipivot is big enough */
    if (int(ipivot.size()) < _n)
      return -104;

    /* call dispatcher */
    mtl_lapack_dispatch::gesv(_n, _nrhs, a.data(), _lda, ipivot.data(),
			      b.data(), _ldb, _info);

    return _info;
  }

  //: LU factorization of a general matrix A.
  //    Computes an LU factorization of a general M-by-N matrix A using
  //    partial pivoting with row interchanges. Factorization has the form
  //    A = P*L*U.
  // <UL>
  //   <LI> a       (IN/OUT - matrix(M,N)) On entry, the coefficient matrix A to be factored. On exit, the factors L and U from the factorization A = P*L*U.
  //
  //   <LI> ipivot  (OUT - vector(min(M,N))) Integer vector. The row i of A was interchanged with row IPIV(i).
  //
  //   <LI> info    (OUT - <tt>int</tt>)
  //   0   :  successful exit
  //   < 0 :  If INFO = -i, then the i-th argument had an illegal value.
  //   > 0 :  If INFO = i, then U(i,i) is exactly zero. The  factorization has been completed, but the factor U is exactly singular, and division by zero will occur if it is used to solve a system of equations.
  //
  // </UL>
  //
  //!component: function
  //!category: mtl2lapack
  template <class LapackMatrix, class VectorInt>
  int getrf (LapackMatrix& a,
	     VectorInt& ipivot)
  {
    typename LapackMatrix::value_type* _a = a.data();
    int _lda = a.minor();
    int _n = a.ncols();
    int _m = a.nrows();
    int _info;

    mtl_lapack_dispatch::getrf (_m, _n,	_a, _lda, ipivot.data(), _info);

    return _info;
  }

  //:  Solution to a system using LU factorization
  //   Solves a system of linear equations A*X = B with a general NxN
  //   matrix A using the LU factorization computed by GETRF.
  // <UL>
  //   <LI> transa  (IN - char)  'T' for the transpose of A, 'N' otherwise.
  //
  //   <LI> a       (IN - matrix(M,N)) The factors L and U from the factorization A = P*L*U as computed by GETRF.
  //
  //   <LI> ipivot  (IN - vector(min(M,N))) Integer vector. The pivot indices from GETRF; row i of A was interchanged with row IPIV(i).
  //
  //   <LI> b       (IN/OUT - matrix(ldb,NRHS)) Matrix of same numerical type as A. On entry, the right hand side matrix B. On exit, the solution matrix X.
  //
  //   <LI> info    (OUT - <tt>int</tt>)
  //   0   : function completed normally
  //   < 0 : The ith argument, where i = abs(return value) had an illegal value.
  //   > 0 : if INFO =  i,  U(i,i)  is  exactly  zero;  the  matrix is singular and its inverse could not be computed.
  // </UL>
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatrixA, class LapackMatrixB, class VectorInt>
  int getrs (char transa, LapackMatrixA& a,
	     VectorInt& ipivot, LapackMatrixB& b)
  {
    typename LapackMatrixA::value_type* _a = a.data();
    int _lda = a.minor();
    int a_n = a.nrows();
    int b_n = b.nrows();
    int p_n = ipivot.size();

    typename LapackMatrixB::value_type* _b = b.data();
    int _ldb = b.minor();
    int _nrhs = b.ncols(); /* B's ncols is the # of vectors on rhs */

    if (a_n != b_n) /*Test to see if AX=B has correct dimensions */
      return -101;
    if (p_n < a_n)     /*Check to see if ipivot is big enough */
      return -102;

    int _info;
    mtl_lapack_dispatch::getrs (transa, a_n, _nrhs, _a,	_lda, ipivot.data(),
				_b, _ldb, _info);

    return _info;
  }


  inline
  void error_message(int ret_val)
  {
    if (ret_val==0)
      std::cerr << "No error" << std::endl;
    else if (ret_val > 0)
      std::cerr << "Computational error." << std::endl;
    else if (ret_val > -101)
      std::cerr << "Argument #"
		<< -ret_val
		<< " of lapack subroutine had an illegal value."
		<< std::endl;
    else
      std::cerr << "Argument #"
		<< -(ret_val+100)
		<< "of mtl_lapack subroutine had an illegal value."
		<< std::endl;
  }

  //: Equilibrate and reduce condition number.
  //  Compute row and column scaling inteded to equilibrate an M-by-N
  //  matrix A and reduce its condition number.
  // <UL>
  //   <LI> a       (IN - matrix(M,N)) The M-by-N matrix whose equilibration factors are to be computed.
  //
  //   <LI> r       (OUT - vector(M)) Real vector. If INFO = 0 or INFO > M, R contains  the  row  scale factors for A.
  //
  //   <LI> c       (OUT - vector(N)) Real vector. If INFO = 0,  C contains the  column  scale  factors for A.
  //
  //   <LI> row_cond (OUT - Real number) If INFO = 0 or INFO > M, ROWCND contains  the  ratio of the smallest R(i) to the largest R(i).  If ROWCND >= 0.1 and AMAX is neither too large nor too  small, it is not worth scaling by R.
  //
  //   <LI> col_cond (OUT - Real number) If INFO = 0, COLCND contains the ratio of the  smallest C(i) to the largest C(i).  If COLCND >= 0.1, it is not worth scaling by C.
  //
  //   <LI> amax    (OUT - Real number) Absolute value of largest matrix element. If  AMAX is  very  close  to overflow or very close to underflow, the matrix should be scaled.
  //
  // </UL>
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatA, class VectorReal, class Real>
  int geequ(const LapackMatA& a, VectorReal& r, VectorReal& c,
		   Real& row_cond, Real& col_cond, Real& amax)
  {
    int lda = a.minor();
    int m = a.nrows();
    int n = a.ncols();
    int info;
    mtl_lapack_dispatch::geequ(m, n, a.data(), lda,  r.data(), c.data(),
			       row_cond, col_cond, amax, info);
    return info;
  }


  //:  Compute an LQ factorization.
  //  Compute an LQ factorization of a M-by-N matrix A.
  // <UL>
  //   <LI> a     (IN/OUT - matrix(M,N)) On entry, the M-by-N matrix A.  On  exit, the  elements on and below the diagonal of the array contain the m-by-min(m,n) lower trapezoidal matrix L  (L  is lower  triangular if m <= n); the elements above the diagonal, with the array TAU, represent the  unitary matrix  Q as a product of elementary reflectors.
  //
  //   <LI> tau   (OUT - vector(min(M,N)) The scalar factors of the elementary reflectors.
  // </UL>
  //!component: function
  //!category: mtl2lapack
  template <class LapackMatA, class VectorT>
  int gelqf(LapackMatA& a, VectorT& tau)
  {
    int lda = a.minor();
    int m = a.nrows();
    int n = a.ncols();

    if (int(tau.size()) < MTL_MIN(m,n))
      return -104;

    int info;
    mtl_lapack_dispatch::gelqf(m, n, a.data(), lda, tau.data(), info);
    return info;
  }


  //: Compute least squares solution using svd for Ax = b
  //  Compute least squares solution using svd for Ax = b
  //  A is m by n, x is length n, b is length m
  template <class LapackMatA, class VectorT>
  int gelss(LapackMatA& a,
	    VectorT& x,
	    VectorT& b,
	    double tol=1.e-6)
  {
    int lda = a.minor();
    int m   = a.nrows();
    int n   = a.ncols();
    int ldb = b.size();

    int info, rank;

    const int max_length = MTL_MAX(b.size(), x.size());

    typedef typename VectorT::value_type T;
    T *rhs_data = new T[max_length];

    for (int i = 0; i < b.size(); ++i)
      rhs_data[i] = b[i];

    mtl_lapack_dispatch::gelss(m, n, 1, a.data(), lda,
			       rhs_data, max_length, tol, rank, info);

    for (int i = 0; i < x.size(); ++i)
      x[i] = rhs_data[i];

    delete [] rhs_data;

    return info;
  }

  /*
  template <class LapackMatA, class VectorT>
  int gelss(LapackMatA& a, VectorT& b, double tol=1.e-6)
  {
    int lda = a.minor();
    int m = a.nrows();
    int n = a.ncols();
    int ldb = b.size();

    int info, rank;

    mtl_lapack_dispatch::gelss(m, n, 1, a.data(), lda,
			       b.data(), ldb, tol, rank, info);

    return info;
  }
  */



  //:  Generate a matrix Q with orthonormal rows.
  //  Generate an M-by-N real matrix Q with orthonormal rows.
  // <UL>
  //   <LI> a     (IN/OUT - matrix(M,N) On entry, the i-th row must contain the vector which defines the  elementary  reflector  H(i),  for  i  = 1,2,...,k, as returned by GELQF in the first k rows of its array argument A.  On exit, the M-by-N matrix Q.
  //
  //   <LI> tau   (IN - vector(K)) tau[i] must contain the scalar factor of the elementary reflector H(i), as returned by GELQF.
  // </UL>
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatA, class VectorT>
  int orglq(LapackMatA& a, const VectorT& tau)
  {
    int lda = a.minor();
    int m = a.nrows();
    int n = a.ncols();
    int info;
    mtl_lapack_dispatch::orglq(m, n, tau.size(), a.data(), lda,
			       tau.data(), info);
    return info;
  }


//: Generate a matrix Q with orthonormal columns.
  //  Generate an M-by-N real matrix Q with orthonormal columns.
  // <UL>
  //  <LI> a     (IN/OUT - matrix(M,N) On  entry,  the  i-th column must contain the vector which defines the elementary reflector H(i), for i = 1,2,...,k,  as  returned  by GEQRF  in the first k columns of its array argument A.  On  exit, the  M-by-N matrix Q.
  //
  //  <LI> tau   (IN - vector(K)) tau[i] must contain the scalar factor of the elementary reflector H(i), as returned by GEQRF.
  // </UL>
  //
  //!component: function
  //!category: mtl2lapack

  template <class LapackMatA, class VectorT>
  int orgqr(LapackMatA& a, const VectorT& tau)
  {
    int lda = a.minor();
    int m = a.nrows();
    int n = a.ncols();
    int info;
    mtl_lapack_dispatch::orgqr(m, n, tau.size(), a.data(), lda,
			       tau.data(), info);
    return info;
  }



  template <class LapackMatA, class VectorT, class LapackMatU, class LapackMatVT>
  int gesvd(const char& jobu, const char& jobvt, LapackMatA& a, VectorT& s,
	    LapackMatU& u, LapackMatVT& vt)
  {
    int info;
    const int lda = a.minor();
    const int m = a.nrows();
    const int n = a.ncols();
    const int ldu = u.minor();
    const int ldvt = vt.minor();

    mtl_lapack_dispatch::gesvd(jobu, jobvt, m, n, a.data(), lda, s.data(),
			       u.data(), ldu, vt.data(), ldvt, info);
    return info;
  }

} /* namespace mtl2lapack */

#endif








