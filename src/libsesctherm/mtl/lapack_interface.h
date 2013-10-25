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

#ifndef LAPACK_INTERFACE_H
#define LAPACK_INTERFACE_H

#include "mtl_complex.h"



extern "C" {

  void dgecon_ (const char & norm, const int & n, const double da[],
		const int & lda, const double & danorm,
		double & drcond, double dwork[],
		int iwork2[], int & info);

  void sgecon_ (const char & norm, const int & n,
		const float sa[], const int & lda,
		const float & sanorm, float & srcond,
		float swork[], int iwork2[],
		int & info);

  void zgecon_ (const char & norm, const int & n,
		const std::complex<double> za[], const int & lda,
		const double & danorm, double & drcond,
		std::complex<double> zwork[], double dwork2[],
		int & info);

  void cgecon_ (const char& norm, const int& n,
		const std::complex<float> sa[], const int& lda,
		const float & danorm, float& srcond,
		std::complex<float> cwork[],	float swork2[],
		int& info);


  void dgeev_(const char& jobvl, const char& jobvr,
	      const int& n, double da[],
	      const int& lda, double dwr[],
	      double dwi[], double dvl[],
	      const int& ldvl, double dvr[],
	      const int& ldvr, double dwork[],
	      const int& ldwork, int& info);

  void sgeev_(const char& jobvl, const char& jobvr,
	      const int&  n, float sa[],
	      const int& lda, float swr[],
	      float swi[], float svl[],
	      const int& ldvl, float svr[],
	      const int& ldvr, float swork[],
	      const int& ldwork, int& info);

  void zgeev_(const char& jobvl, const char& jobvr,
	      const int&  n, std::complex<double> za[],
	      const int& lda, std::complex<double> zw[],
	      std::complex<double> zvl[], const int& ldvl,
	      std::complex<double> zvr[], const int& ldvr,
	      std::complex<double> zwork[], const int& ldwork,
	      double dwork2[], int& info);

  void cgeev_(const char& jobvl, const char& jobvr,
	      const int&  n, std::complex<float> ca[],
	      const int& lda, std::complex<float> cw[],
	      std::complex<float> cvl[], const int& ldvl,
	      std::complex<float> cvr[], const int& ldvr,
	      std::complex<float> cwork[], const int& ldwork,
	      float swork2[], int& info);

  void dgeqpf_(const int & m, const int & n,
	       double da[], const int & lda,
	       int jpivot[], double dtau[],
	       double dwork[], int & info);

  void sgeqpf_(const int & m, const int & n,
	       float sa[], const int & lda,
	       int jpivot[], float stau[],
	       float swork[], int & info);

  void zgeqpf_(const int & m,
	       const int & n,
	       std::complex<double> za[],
	       const int & lda,
	       int jpivot[],
	       std::complex<double> ztau[],
	       std::complex<double> zwork[],
	       double dwork2[],
	       int & info);

  void cgeqpf_(const int & m,
	       const int & n,
	       std::complex<float> ca[],
	       const int & lda,
	       int jpivot[],
	       std::complex<float> ctau[],
	       std::complex<float> cwork[],
	       float swork2[],
	       int& info);

  void dgeqrf_(const int & m,
	       const int & n,
	       double da[],
	       const int & lda,
	       double dtau[],
	       double dwork[],
	       const int& ldwork,
	       int& info);

  void sgeqrf_(const int & m,
	       const int & n,
	       float sa[],
	       const int & lda,
	       float stau[],
	       float swork[],
	       const int& ldwork,
	       int& info);

  void zgeqrf_(const int & m,
	       const int & n,
	       std::complex<double> za[],
	       const int & lda,
	       std::complex<double> ztau[],
	       std::complex<double> zwork[],
	       const int& ldwork,
	       int & info);

  void cgeqrf_(const int & m,
	       const int & n,
	       std::complex<float> ca[],
	       const int & lda,
	       std::complex<float> ctau[],
	       std::complex<float> cwork[],
	       const int& ldwork,
	       int & info);

  void dgesv_(const int & n,
	      const int & nrhs,
	      double da[],
	      const int & lda,
	      int ipivot[],
	      double db[],
	      const int & ldb,
	      int & info);

  void sgesv_(const int & n,
	      const int & nrhs,
	      float sa[],
	      const int & lda,
	      int ipivot[],
	      float sb[],
	      const int & ldb,
	      int & info);

  void zgesv_(const int & n,
	      const int & nrhs,
	      std::complex<double> za[],
	      const int & lda,
	      int ipivot[],
	      std::complex<double> zb[],
	      const int & ldb,
	      int & info);

  void cgesv_(const int & n,
	      const int & nrhs,
	      std::complex<float> ca[],
	      const int & lda,
	      int ipivot[],
	      std::complex<float> cb[],
	      const int & ldb,
	      int & info);

 void dgetrf_ (const int& m,
	       const int& n,
	       double da[],
	       const int& lda,
	       int ipivot[],
	       int& info);

 void sgetrf_ (const int& m,
	       const int& n,
	       float sa[],
	       const int& lda,
	       int ipivot[],
	       int& info);

 void zgetrf_ (const int& m,
	       const int& n,
	       std::complex<double> za[],
	       const int& lda,
	       int ipivot[],
	       int& info);

 void cgetrf_ (const int& m,
	       const int& n,
	       std::complex<float> ca[],
	       const int& lda,
	       int ipivot[],
	       int& info);


 void dgetrs_ (const char& transa,
	       const int& n,
	       const int& nrhs,
	       const double da[],
	       const int& lda,
	       int ipivot[],
	       double db[],
	       const int& ldb,
	       int& info);

 void sgetrs_ (const char& transa,
	       const int& n,
	       const int& nrhs,
	       const float  sa[],
	       const int& lda,
	       int ipivot[],
	       float sb[],
	       const int& ldb,
	       int& info);

 void zgetrs_ (const char& transa,
	       const int& n,
	       const int& nrhs,
	       const std::complex<double> za[],
	       const int& lda,
	       int ipivot[],
	       std::complex<double> zb[],
	       const int& ldb,
	       int& info);

void cgetrs_ (const char& transa,
	      const int& n,
	      const int& nrhs,
	      const std::complex<float> ca[],
	      const int& lda,
	      int ipivot[],
	      std::complex<float> cb[],
	      const int& ldb,
	      int& info);

void dgeequ_ (const int& m, const int& n, const double da[], const int& lda,
	      double r[], double c[], double& rowcnd, double& colcnd,
	      double& amax, int& info);

void sgeequ_ (const int& m, const int& n, const float da[], const int& lda,
	      float r[], float c[], float& rowcnd, float& colcnd,
	      float& amax, int& info);

void cgeequ_ (const int& m, const int& n, const std::complex<float> da[],
	      const int& lda, std::complex<float> r[], std::complex<float> c[],
	      float& rowcnd, float& colcnd,
	      float& amax, int& info);

void zgeequ_ (const int& m, const int& n, const std::complex<double> da[],
	      const int& lda, std::complex<double> r[], std::complex<double> c[],
	      double& rowcnd, double& colcnd,
	      double& amax, int& info);



void dgelqf_ (const int& m, const int& n, double da[], const int& lda,
	      double dtau[], double work[], const int& ldwork, int& info);

void sgelqf_ (const int& m, const int& n, float da[], const int& lda,
	      float dtau[], float work[], const int& ldwork, int& info);

void cgelqf_ (const int& m, const int& n, std::complex<float> da[], const int& lda,
	      std::complex<float> dtau[], std::complex<float> work[],
	      const int& ldwork, int& info);

void zgelqf_ (const int& m, const int& n, std::complex<double> da[], const int& lda,
	      std::complex<double> dtau[], std::complex<double> work[],
	      const int& ldwork, int& info);


void dgelss_ (const int& m, const int& n, const int& nrhs,
	      double da[], const int& lda, double b[], const int& ldb,
	      double s[], const double& rcond, int& rank,
	      double work[], const int& ldwork, int& info);

void sgelss_ (const int& m, const int& n, const int& nrhs,
	      float da[], const int& lda, float b[], const int& ldb,
	      float s[], const float& rcond, int& rank,
	      float work[], const int& ldwork, int& info);

void zgelss_ (const int& m, const int& n, const int& nrhs,
	      std::complex<double> da[], const int& lda,
	      std::complex<double> b[], const int& ldb,
	      std::complex<double> s[], const double& rcond, int& rank,
	      std::complex<double> work[], const int& ldwork, int& info);

void cgelss_ (const int& m, const int& n, const int& nrhs,
	      std::complex<float> da[], const int& lda,
	      std::complex<float> b[], const int& ldb,
	      std::complex<float> s[], const float& rcond, int& rank,
	      std::complex<float> work[], const int& ldwork, int& info);


void dorglq_(const int& m, const int& n, const int& k,
	     double da[], int& lda, double dtau[],
	     double dwork[], const int& ldwork, int& info);

void sorglq_(const int& m, const int& n, const int& k,
	     float da[], int& lda, float dtau[],
	     float dwork[], const int& ldwork, int& info);


void dorgqr_(const int& m, const int& n, const int& k,
	     double da[], const int& lda, double dtau[],
	     double dwork[], const int& ldwork, int& info);

void sorgqr_(const int& m, const int& n, const int& k,
	     float da[], const int& lda, float dtau[],
	     float dwork[], const int& ldwork, int& info);

void dgesvd_(const char& jobu, const char& jobvt, const int& m, const int& n,
             double da[], const int& lda, double ds[], double du[],
	     const int& ldu, double dvt[], const int& ldvt,
	     double work[], const int& lwork, int& info);

void sgesvd_(const char& jobu, const char& jobvt, const int& m, const int& n,
             float da[], const int& lda, float ds[], float du[],
	     const int& ldu, float dvt[], const int& ldvt,
	     float work[], const int& lwork, int& info);

void zgesvd_(const char& jobu, const char& jobvt, const int& m, const int& n,
             std::complex<double> da[], const int& lda, double ds[],
	     std::complex<double> du[],
	     const int& ldu, std::complex<double> dvt[], const int& ldvt,
	     std::complex<double> work[], const int& lwork,
	     double rwork[], int& info);

void cgesvd_(const char& jobu, const char& jobvt, const int& m, const int& n,
             std::complex<float> da[], const int& lda, float ds[],
	     std::complex<float> du[],
	     const int& ldu, std::complex<float> dvt[], const int& ldvt,
	     std::complex<float> work[], const int& lwork,
	     float rwork[], int& info);


 void dgeevx_ (const char & balanc, //IN
	       const char & jobvl,  //IN
	       const char & jobvr,  //IN
	       const char & sense,  //IN
	       const int & n,       //IN
	       double da[],         //IN
	       const int & lda,     //IN
	       double dwr[],        //OUT
	       double dwi[],        //OUT
	       double dvl[],        //OUT
	       const int & ldvl,    //IN
	       double dvr[],        //OUT
	       const int & ldvr,    //IN
	       int & ilo,           //OUT
	       int & ihi,           //OUT
	       double dscale[],     //OUT
	       double & dabnrm,     //OUT
	       double drcone[],     //OUT
	       double drconv[],     //OUT
	       double dwork[],      //WORKSPACE
	       const int & ldwork,  //IN
	       int iwork2[],        //WORKSPACE
	       int & info);


 void dgemm_(const char *, const char*,
	     const int& cols, const int& rows, const int& mids,
	     const double& a, const double* B, const int& ldb,
	     const double* A, const int& lda, const double& b,
	     double* C, const int& ldc);


}

#endif
