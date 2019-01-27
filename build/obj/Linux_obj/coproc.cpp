/*
 * Macros for generating multiple versions of functions to simulate an
 * instruction.
 *
 * Copyright (C) 1993 by Jack E. Veenstra (veenstra@cs.rochester.edu)
 * 
 * This file is part of MINT, a MIPS code interpreter and event generator
 * for parallel programs.
 * 
 * MINT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * MINT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MINT; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* If you think m4 source is hard to read, you should try *writing* it! */

/* If this is C code you are reading, this file was machine generated
 * using m4 macros. See the corresponding file.m4 for the source.
 */

/* Do not use a comma within a C comment inside an m4 macro!!! */

/* Change the quote character so the input file looks more like C code. */


/* This macro is used repeatedly to generate two versions of a function.
 * The first version is called for an instruction that is not in the branch
 * delay slot of the previously executed instruction. The second version
 * is called for an instruction that is in the branch delay slot of the
 * previously executed instruction.
 */


/* Byte READ */


/* Word READ */




/* floating point read */


/* double floating point read */


/* The following macro creates code to check that an address does not
 * have an active ll (load-linked) operation pending on it. It generates
 * this check on EVERY WRITE instruction.
 */


/* The following macro creates code to check that an address does not
 * have an active ll (load-linked) operation pending on it. It generates
 * this check only for sc (store-conditional) instructions.
 */


/* arg 5 is the value to write on successful sc; arg 6 is the value to
 * write on failed sc. When Verify_protocol is set we need to use the value
 * setup by the back-end.
 */




/* Define normal version only; no branch delay slot version */




/* Local Variables: */
/* mode: c */
/* End: */
/*
 * Routines for implementing coprocessor instructions.
 *
 * Copyright (C) 1993 by Jack E. Veenstra (veenstra@cs.rochester.edu)
 * 
 * This file is part of MINT, a MIPS code interpreter and event generator
 * for parallel programs.
 * 
 * MINT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * MINT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MINT; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <math.h>
#include "icode.h"
#include "ThreadContext.h"
#include "globals.h"
#include "opcodes.h"
#include "non_mips.h"

#ifdef SUNOS
#define trunc(a) ((int)(a))
#define truncf(a) ((int)(a))
#elif AIX
#define truncf(a) (trunc(a))
#endif

#ifndef isunordered
#define isunordered(u, v) ( isnan(u) || isnan(v) )
#endif

#ifndef isless
#define isless(x, y) (!isunordered (x, y) && (x) < (y) )
#endif

#ifndef islessequal
#define islessequal(x, y) (!isunordered (x, y) && (x) <= (y) )
#endif

/* Macros for declaring function prototypes. To make this less non-portable,
 * "float *" is used instead of "float" since otherwise some compilers might
 * promote a float to a double at the call site.
 */
#define FOP1(X) float X(float *, int *)
#define DOP1(X) double X(double, int *)
#define FOP2(X) float X(float *, float *, int *)
#define DOP2(X) double X(double, double, int *)

FOP1(s_abs_s); DOP1(s_abs_d);
FOP2(s_add_s); DOP2(s_add_d);
FOP2(s_div_s); DOP2(s_div_d);
FOP2(s_mul_s); DOP2(s_mul_d);
FOP1(s_neg_s); DOP1(s_neg_d);
FOP2(s_sub_s); DOP2(s_sub_d);
FOP1(s_trunc_w_s); DOP1(s_trunc_w_d); /* VK */
FOP1(s_sqrt_s); DOP1(s_sqrt_d);       /* VK */
double s_cvt_d_s(float *, int *);
double s_cvt_d_w(int *, int *);
float s_cvt_s_d(double, int *);
float s_cvt_s_w(int *, int *);
int s_cvt_w_d(double, int *);
int s_cvt_w_s(float *, int *);

/* normal version */
OP(c1_abs_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  if (isless(pthread->getFP(picode, ICODEFS),(float)0))
    pthread->setFP(picode, ICODEFD, -pthread->getFP(picode, ICODEFS));
  else
    pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_abs_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  if (isless(pthread->getFP(picode, ICODEFS),(float)0))
    pthread->setFP(picode, ICODEFD, -pthread->getFP(picode, ICODEFS));
  else
    pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_abs_s[] = { c1_abs_s_0, c1_abs_s_1 };


/* normal version */
OP(c1_abs_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  if (isless(pthread->getDP(picode, ICODEFS),(double)0))
    pthread->setDP(picode, ICODEFD, -pthread->getDP(picode, ICODEFS));
  else
    pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_abs_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  if (isless(pthread->getDP(picode, ICODEFS),(double)0))
    pthread->setDP(picode, ICODEFD, -pthread->getDP(picode, ICODEFS));
  else
    pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_abs_d[] = { c1_abs_d_0, c1_abs_d_1 };


/* normal version */
OP(c1_add_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) + pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_add_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) + pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_add_s[] = { c1_add_s_0, c1_add_s_1 };


/* normal version */
OP(c1_add_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) + pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_add_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) + pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_add_d[] = { c1_add_d_0, c1_add_d_1 };


/* normal version */
OP(c1_c_eq_s_0)
{

  if( pthread->getFP(picode, ICODEFS) == pthread->getFP(picode, ICODEFT) && !isunordered(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_eq_s_1)
{

  if( pthread->getFP(picode, ICODEFS) == pthread->getFP(picode, ICODEFT) && !isunordered(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_eq_s[] = { c1_c_eq_s_0, c1_c_eq_s_1 };

/* normal version */
OP(c1_c_eq_d_0)
{

  if( pthread->getDP(picode, ICODEFS) == pthread->getDP(picode, ICODEFT) && !isunordered(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_eq_d_1)
{

  if( pthread->getDP(picode, ICODEFS) == pthread->getDP(picode, ICODEFT) && !isunordered(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_eq_d[] = { c1_c_eq_d_0, c1_c_eq_d_1 };


/* normal version */
OP(c1_c_f_s_0)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_f_s_1)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_f_s[] = { c1_c_f_s_0, c1_c_f_s_1 };

/* normal version */
OP(c1_c_f_d_0)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_f_d_1)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_f_d[] = { c1_c_f_d_0, c1_c_f_d_1 };


/* normal version */
OP(c1_c_le_s_0)
{

  if( islessequal(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_le_s_1)
{

  if( islessequal(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_le_s[] = { c1_c_le_s_0, c1_c_le_s_1 };

/* normal version */
OP(c1_c_le_d_0)
{

  if( islessequal(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_le_d_1)
{

  if( islessequal(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_le_d[] = { c1_c_le_d_0, c1_c_le_d_1 };


/* normal version */
OP(c1_c_lt_s_0)
{

  if( isless(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_lt_s_1)
{

  if( isless(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_lt_s[] = { c1_c_lt_s_0, c1_c_lt_s_1 };

/* normal version */
OP(c1_c_lt_d_0)
{

  if( isless(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_lt_d_1)
{

  if( isless(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_lt_d[] = { c1_c_lt_d_0, c1_c_lt_d_1 };


/* single precision normal version */
OP(c1_c_nge_s_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_nge_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* single precision version for branch delay slots */
OP(c1_c_nge_s_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_nge_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

/* double precision normal version */
OP(c1_c_nge_d_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_nge_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* double precision version for branch delay slots */
OP(c1_c_nge_d_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_nge_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

PFPI c1_c_nge_s[] = { c1_c_nge_s_0, c1_c_nge_s_1 };
PFPI c1_c_nge_d[] = { c1_c_nge_d_0, c1_c_nge_d_1 };

/* single precision normal version */
OP(c1_c_ngl_s_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngl_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* single precision version for branch delay slots */
OP(c1_c_ngl_s_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngl_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

/* double precision normal version */
OP(c1_c_ngl_d_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngl_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* double precision version for branch delay slots */
OP(c1_c_ngl_d_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngl_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

PFPI c1_c_ngl_s[] = { c1_c_ngl_s_0, c1_c_ngl_s_1 };
PFPI c1_c_ngl_d[] = { c1_c_ngl_d_0, c1_c_ngl_d_1 };

/* single precision normal version */
OP(c1_c_ngle_s_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngle_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* single precision version for branch delay slots */
OP(c1_c_ngle_s_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngle_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

/* double precision normal version */
OP(c1_c_ngle_d_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngle_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* double precision version for branch delay slots */
OP(c1_c_ngle_d_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngle_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

PFPI c1_c_ngle_s[] = { c1_c_ngle_s_0, c1_c_ngle_s_1 };
PFPI c1_c_ngle_d[] = { c1_c_ngle_d_0, c1_c_ngle_d_1 };

/* single precision normal version */
OP(c1_c_ngt_s_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngt_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* single precision version for branch delay slots */
OP(c1_c_ngt_s_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngt_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

/* double precision normal version */
OP(c1_c_ngt_d_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngt_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* double precision version for branch delay slots */
OP(c1_c_ngt_d_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_ngt_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

PFPI c1_c_ngt_s[] = { c1_c_ngt_s_0, c1_c_ngt_s_1 };
PFPI c1_c_ngt_d[] = { c1_c_ngt_d_0, c1_c_ngt_d_1 };


/* normal version */
OP(c1_c_ole_s_0)
{

  if( islessequal(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ole_s_1)
{

  if( islessequal(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ole_s[] = { c1_c_ole_s_0, c1_c_ole_s_1 };

/* normal version */
OP(c1_c_ole_d_0)
{

  if( islessequal(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ole_d_1)
{

  if( islessequal(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ole_d[] = { c1_c_ole_d_0, c1_c_ole_d_1 };


/* normal version */
OP(c1_c_olt_s_0)
{

  if( isless(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_olt_s_1)
{

  if( isless(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_olt_s[] = { c1_c_olt_s_0, c1_c_olt_s_1 };

/* normal version */
OP(c1_c_olt_d_0)
{

  if( isless(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_olt_d_1)
{

  if( isless(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_olt_d[] = { c1_c_olt_d_0, c1_c_olt_d_1 };


/* single precision normal version */
OP(c1_c_seq_s_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_seq_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* single precision version for branch delay slots */
OP(c1_c_seq_s_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_seq_s (&FP(ICODEFS), &FP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

/* double precision normal version */
OP(c1_c_seq_d_0)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_seq_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return picode->next;
}

/* double precision version for branch delay slots */
OP(c1_c_seq_d_1)
{
#ifdef MIPS2_FNATIVE
    pthread->fcr31 = s_c1_c_seq_d (DP(ICODEFS), DP(ICODEFT), pthread->fcr31);
#else
    unimplemented_op(picode, pthread);
#endif
    return pthread->getTarget();
}

PFPI c1_c_seq_s[] = { c1_c_seq_s_0, c1_c_seq_s_1 };
PFPI c1_c_seq_d[] = { c1_c_seq_d_0, c1_c_seq_d_1 };


/* normal version */
OP(c1_c_sf_s_0)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_sf_s_1)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_sf_s[] = { c1_c_sf_s_0, c1_c_sf_s_1 };

/* normal version */
OP(c1_c_sf_d_0)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_sf_d_1)
{

  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_sf_d[] = { c1_c_sf_d_0, c1_c_sf_d_1 };


/* normal version */
OP(c1_c_ueq_s_0)
{

  if( pthread->getFP(picode, ICODEFS) == pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ueq_s_1)
{

  if( pthread->getFP(picode, ICODEFS) == pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ueq_s[] = { c1_c_ueq_s_0, c1_c_ueq_s_1 };

/* normal version */
OP(c1_c_ueq_d_0)
{

  if( pthread->getDP(picode, ICODEFS) == pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ueq_d_1)
{

  if( pthread->getDP(picode, ICODEFS) == pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ueq_d[] = { c1_c_ueq_d_0, c1_c_ueq_d_1 };


/* normal version */
OP(c1_c_ule_s_0)
{

  if( pthread->getFP(picode, ICODEFS) <= pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ule_s_1)
{

  if( pthread->getFP(picode, ICODEFS) <= pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ule_s[] = { c1_c_ule_s_0, c1_c_ule_s_1 };

/* normal version */
OP(c1_c_ule_d_0)
{

  if( pthread->getDP(picode, ICODEFS) <= pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ule_d_1)
{

  if( pthread->getDP(picode, ICODEFS) <= pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ule_d[] = { c1_c_ule_d_0, c1_c_ule_d_1 };


/* normal version */
OP(c1_c_ult_s_0)
{

  if( pthread->getFP(picode, ICODEFS) < pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ult_s_1)
{

  if( pthread->getFP(picode, ICODEFS) < pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ult_s[] = { c1_c_ult_s_0, c1_c_ult_s_1 };

/* normal version */
OP(c1_c_ult_d_0)
{

  if( pthread->getDP(picode, ICODEFS) < pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_ult_d_1)
{

  if( pthread->getDP(picode, ICODEFS) < pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_ult_d[] = { c1_c_ult_d_0, c1_c_ult_d_1 };


/* normal version */
OP(c1_c_un_s_0)
{

  if( isunordered(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_un_s_1)
{

  if( isunordered(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_un_s[] = { c1_c_un_s_0, c1_c_un_s_1 };

/* normal version */
OP(c1_c_un_d_0)
{

  if( isunordered(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return picode->next;
}

/* branch delay slot version */
OP(c1_c_un_d_1)
{

  if( isunordered(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);

    return pthread->getTarget();
}

PFPI c1_c_un_d[] = { c1_c_un_d_0, c1_c_un_d_1 };


/* normal version */
OP(c1_ceil_w_s_0)
{

    unimplemented_op(picode, pthread);

    return picode->next;
}

/* branch delay slot version */
OP(c1_ceil_w_s_1)
{

    unimplemented_op(picode, pthread);

    return pthread->getTarget();
}

PFPI c1_ceil_w_s[] = { c1_ceil_w_s_0, c1_ceil_w_s_1 };


/* normal version */
OP(c1_ceil_w_d_0)
{

    unimplemented_op(picode, pthread);

    return picode->next;
}

/* branch delay slot version */
OP(c1_ceil_w_d_1)
{

    unimplemented_op(picode, pthread);

    return pthread->getTarget();
}

PFPI c1_ceil_w_d[] = { c1_ceil_w_d_0, c1_ceil_w_d_1 };


/* normal version */
OP(c1_cvt_d_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, (double)pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_cvt_d_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, (double)pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_cvt_d_s[] = { c1_cvt_d_s_0, c1_cvt_d_s_1 };


/* normal version */
OP(c1_cvt_d_w_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, (double)pthread->getWFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_cvt_d_w_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, (double)pthread->getWFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_cvt_d_w[] = { c1_cvt_d_w_0, c1_cvt_d_w_1 };


/* normal version */
OP(c1_cvt_s_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, (float)pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_cvt_s_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, (float)pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_cvt_s_d[] = { c1_cvt_s_d_0, c1_cvt_s_d_1 };


/* normal version */
OP(c1_cvt_s_w_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, (float)pthread->getWFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_cvt_s_w_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, (float)pthread->getWFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_cvt_s_w[] = { c1_cvt_s_w_0, c1_cvt_s_w_1 };


/* normal version */
OP(c1_cvt_w_s_0)
{

  pthread->setWFP(picode, ICODEFD, mips_cvt_w_s(pthread->getFP(picode, ICODEFS), pthread->getFPUControl31()));

    return picode->next;
}

/* branch delay slot version */
OP(c1_cvt_w_s_1)
{

  pthread->setWFP(picode, ICODEFD, mips_cvt_w_s(pthread->getFP(picode, ICODEFS), pthread->getFPUControl31()));

    return pthread->getTarget();
}

PFPI c1_cvt_w_s[] = { c1_cvt_w_s_0, c1_cvt_w_s_1 };


/* normal version */
OP(c1_cvt_w_d_0)
{

  pthread->setWFP(picode, ICODEFD, mips_cvt_w_d(pthread->getDP(picode, ICODEFS), pthread->getFPUControl31()));

    return picode->next;
}

/* branch delay slot version */
OP(c1_cvt_w_d_1)
{

  pthread->setWFP(picode, ICODEFD, mips_cvt_w_d(pthread->getDP(picode, ICODEFS), pthread->getFPUControl31()));

    return pthread->getTarget();
}

PFPI c1_cvt_w_d[] = { c1_cvt_w_d_0, c1_cvt_w_d_1 };


/* normal version */
OP(c1_div_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) / pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_div_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) / pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_div_s[] = { c1_div_s_0, c1_div_s_1 };


/* normal version */
OP(c1_div_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif

  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) / pthread->getDP(picode, ICODEFT));

#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_div_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif

  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) / pthread->getDP(picode, ICODEFT));

#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_div_d[] = { c1_div_d_0, c1_div_d_1 };


/* normal version */
OP(c1_floor_w_s_0)
{

  unimplemented_op(picode, pthread);

    return picode->next;
}

/* branch delay slot version */
OP(c1_floor_w_s_1)
{

  unimplemented_op(picode, pthread);

    return pthread->getTarget();
}

PFPI c1_floor_w_s[] = { c1_floor_w_s_0, c1_floor_w_s_1 };


/* normal version */
OP(c1_floor_w_d_0)
{

  unimplemented_op(picode, pthread);

    return picode->next;
}

/* branch delay slot version */
OP(c1_floor_w_d_1)
{

  unimplemented_op(picode, pthread);

    return pthread->getTarget();
}

PFPI c1_floor_w_d[] = { c1_floor_w_d_0, c1_floor_w_d_1 };


/* normal version */
OP(c1_mov_s_0)
{

  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS));

    return picode->next;
}

/* branch delay slot version */
OP(c1_mov_s_1)
{

  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS));

    return pthread->getTarget();
}

PFPI c1_mov_s[] = { c1_mov_s_0, c1_mov_s_1 };


/* normal version */
OP(c1_mov_d_0)
{

  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS));

    return picode->next;
}

/* branch delay slot version */
OP(c1_mov_d_1)
{

  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS));

    return pthread->getTarget();
}

PFPI c1_mov_d[] = { c1_mov_d_0, c1_mov_d_1 };


/* normal version */
OP(c1_mul_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) * pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_mul_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) * pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_mul_s[] = { c1_mul_s_0, c1_mul_s_1 };


/* normal version */
OP(c1_mul_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) * pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_mul_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) * pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_mul_d[] = { c1_mul_d_0, c1_mul_d_1 };


/* normal version */
OP(c1_neg_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, -pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_neg_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, -pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_neg_s[] = { c1_neg_s_0, c1_neg_s_1 };


/* normal version */
OP(c1_neg_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, -pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_neg_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, -pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_neg_d[] = { c1_neg_d_0, c1_neg_d_1 };


/* normal version */
OP(c1_round_w_s_0)
{

  unimplemented_op(picode, pthread);

    return picode->next;
}

/* branch delay slot version */
OP(c1_round_w_s_1)
{

  unimplemented_op(picode, pthread);

    return pthread->getTarget();
}

PFPI c1_round_w_s[] = { c1_round_w_s_0, c1_round_w_s_1 };


/* normal version */
OP(c1_round_w_d_0)
{

  unimplemented_op(picode, pthread);

    return picode->next;
}

/* branch delay slot version */
OP(c1_round_w_d_1)
{

  unimplemented_op(picode, pthread);

    return pthread->getTarget();
}

PFPI c1_round_w_d[] = { c1_round_w_d_0, c1_round_w_d_1 };


/* normal version */
OP(c1_sqrt_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
#ifdef host_mips
  pthread->setFP(picode, ICODEFD, _sqrt_s(pthread->getFP(picode, ICODEFS)));
#else
  pthread->setFP(picode, ICODEFD, sqrt(pthread->getFP(picode, ICODEFS)));
#endif
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_sqrt_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
#ifdef host_mips
  pthread->setFP(picode, ICODEFD, _sqrt_s(pthread->getFP(picode, ICODEFS)));
#else
  pthread->setFP(picode, ICODEFD, sqrt(pthread->getFP(picode, ICODEFS)));
#endif
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_sqrt_s[] = { c1_sqrt_s_0, c1_sqrt_s_1 };


/* normal version */
OP(c1_sqrt_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
#ifdef host_mips
  pthread->setDP(picode, ICODEFD, _sqrt_d(pthread->getDP(picode, ICODEFS)));
#else
  pthread->setDP(picode, ICODEFD, sqrt(pthread->getDP(picode, ICODEFS)));
#endif
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_sqrt_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
#ifdef host_mips
  pthread->setDP(picode, ICODEFD, _sqrt_d(pthread->getDP(picode, ICODEFS)));
#else
  pthread->setDP(picode, ICODEFD, sqrt(pthread->getDP(picode, ICODEFS)));
#endif
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_sqrt_d[] = { c1_sqrt_d_0, c1_sqrt_d_1 };


/* normal version */
OP(c1_sub_s_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) - pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_sub_s_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) - pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_sub_s[] = { c1_sub_s_0, c1_sub_s_1 };


/* normal version */
OP(c1_sub_d_0)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) - pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return picode->next;
}

/* branch delay slot version */
OP(c1_sub_d_1)
{

#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) - pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif

    return pthread->getTarget();
}

PFPI c1_sub_d[] = { c1_sub_d_0, c1_sub_d_1 };


/* normal version */
OP(c1_trunc_w_s_0)
{

  int pp = (int)(truncf(pthread->getFP(picode, ICODEFS)));
  pthread->setFP(picode, ICODEFD, *(float *)&pp);

    return picode->next;
}

/* branch delay slot version */
OP(c1_trunc_w_s_1)
{

  int pp = (int)(truncf(pthread->getFP(picode, ICODEFS)));
  pthread->setFP(picode, ICODEFD, *(float *)&pp);

    return pthread->getTarget();
}

PFPI c1_trunc_w_s[] = { c1_trunc_w_s_0, c1_trunc_w_s_1 };



/* normal version */
OP(c1_trunc_w_d_0)
{

  long long pp = (long long)(trunc(pthread->getDP(picode, ICODEFS)));
  pthread->setDP(picode, ICODEFD, *(double *)&pp);

    return picode->next;
}

/* branch delay slot version */
OP(c1_trunc_w_d_1)
{

  long long pp = (long long)(trunc(pthread->getDP(picode, ICODEFS)));
  pthread->setDP(picode, ICODEFD, *(double *)&pp);

    return pthread->getTarget();
}

PFPI c1_trunc_w_d[] = { c1_trunc_w_d_0, c1_trunc_w_d_1 };



/* normal version */
OP(cop0_op_0)
{

  fatal("cop0: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cop0_op_1)
{

  fatal("cop0: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cop0_op[] = { cop0_op_0, cop0_op_1 };


/* normal version */
OP(cop1_op_0)
{

  fatal("cop1: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cop1_op_1)
{

  fatal("cop1: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cop1_op[] = { cop1_op_0, cop1_op_1 };


/* normal version */
OP(cop2_op_0)
{

  fatal("cop2: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cop2_op_1)
{

  fatal("cop2: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cop2_op[] = { cop2_op_0, cop2_op_1 };


/* normal version */
OP(cop3_op_0)
{

  fatal("cop3: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cop3_op_1)
{

  fatal("cop3: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cop3_op[] = { cop3_op_0, cop3_op_1 };


/* normal version */
OP(mfc0_op_0)
{

  fatal("mfc0: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(mfc0_op_1)
{

  fatal("mfc0: not yet implemented\n");

    return pthread->getTarget();
}

PFPI mfc0_op[] = { mfc0_op_0, mfc0_op_1 };


/* normal version */
OP(mfc1_op_0)
{

  pthread->setREG(picode, RT, pthread->getWFP(picode, ICODEFS));

    return picode->next;
}

/* branch delay slot version */
OP(mfc1_op_1)
{

  pthread->setREG(picode, RT, pthread->getWFP(picode, ICODEFS));

    return pthread->getTarget();
}

PFPI mfc1_op[] = { mfc1_op_0, mfc1_op_1 };


/* normal version */
OP(mfc2_op_0)
{

  fatal("mfc2: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(mfc2_op_1)
{

  fatal("mfc2: not yet implemented\n");

    return pthread->getTarget();
}

PFPI mfc2_op[] = { mfc2_op_0, mfc2_op_1 };


/* normal version */
OP(mfc3_op_0)
{

  fatal("mfc3: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(mfc3_op_1)
{

  fatal("mfc3: not yet implemented\n");

    return pthread->getTarget();
}

PFPI mfc3_op[] = { mfc3_op_0, mfc3_op_1 };


/* normal version */
OP(mtc0_op_0)
{

  fatal("mtc0: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(mtc0_op_1)
{

  fatal("mtc0: not yet implemented\n");

    return pthread->getTarget();
}

PFPI mtc0_op[] = { mtc0_op_0, mtc0_op_1 };


/* normal version */
OP(mtc1_op_0)
{

  pthread->setWFP(picode, ICODEFS, pthread->getREG(picode, RT));

    return picode->next;
}

/* branch delay slot version */
OP(mtc1_op_1)
{

  pthread->setWFP(picode, ICODEFS, pthread->getREG(picode, RT));

    return pthread->getTarget();
}

PFPI mtc1_op[] = { mtc1_op_0, mtc1_op_1 };


/* normal version */
OP(mtc2_op_0)
{

  fatal("mtc2: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(mtc2_op_1)
{

  fatal("mtc2: not yet implemented\n");

    return pthread->getTarget();
}

PFPI mtc2_op[] = { mtc2_op_0, mtc2_op_1 };


/* normal version */
OP(mtc3_op_0)
{

  fatal("mtc3: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(mtc3_op_1)
{

  fatal("mtc3: not yet implemented\n");

    return pthread->getTarget();
}

PFPI mtc3_op[] = { mtc3_op_0, mtc3_op_1 };


/* normal version */
OP(cfc0_op_0)
{

  fatal("cfc0: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cfc0_op_1)
{

  fatal("cfc0: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cfc0_op[] = { cfc0_op_0, cfc0_op_1 };


/* normal version */
OP(cfc1_op_0)
{

  if (picode->args[ICODEFS] == 0)
    pthread->setREG(picode, RT, pthread->getFPUControl0());
  else
    pthread->setREG(picode, RT, pthread->getFPUControl31());

    return picode->next;
}

/* branch delay slot version */
OP(cfc1_op_1)
{

  if (picode->args[ICODEFS] == 0)
    pthread->setREG(picode, RT, pthread->getFPUControl0());
  else
    pthread->setREG(picode, RT, pthread->getFPUControl31());

    return pthread->getTarget();
}

PFPI cfc1_op[] = { cfc1_op_0, cfc1_op_1 };


/* normal version */
OP(cfc2_op_0)
{

  fatal("cfc2: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cfc2_op_1)
{

  fatal("cfc2: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cfc2_op[] = { cfc2_op_0, cfc2_op_1 };


/* normal version */
OP(cfc3_op_0)
{

  fatal("cfc3: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(cfc3_op_1)
{

  fatal("cfc3: not yet implemented\n");

    return pthread->getTarget();
}

PFPI cfc3_op[] = { cfc3_op_0, cfc3_op_1 };


/* normal version */
OP(ctc0_op_0)
{

  fatal("ctc0: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(ctc0_op_1)
{

  fatal("ctc0: not yet implemented\n");

    return pthread->getTarget();
}

PFPI ctc0_op[] = { ctc0_op_0, ctc0_op_1 };


/* normal version */
OP(ctc1_op_0)
{

  if (picode->args[ICODEFS] == 0)
    pthread->setFPUControl0(pthread->getREG(picode, RT));
  else{
    pthread->setFPUControl31(pthread->getREG(picode, RT));
#if !((defined TLS) || (defined TASKSCALAR))
    // setFPUControl31(pthread->getFPUControl31());
#endif /* !(defined TLS) || (defined TASKSCALAR) */
  }

    return picode->next;
}

/* branch delay slot version */
OP(ctc1_op_1)
{

  if (picode->args[ICODEFS] == 0)
    pthread->setFPUControl0(pthread->getREG(picode, RT));
  else{
    pthread->setFPUControl31(pthread->getREG(picode, RT));
#if !((defined TLS) || (defined TASKSCALAR))
    // setFPUControl31(pthread->getFPUControl31());
#endif /* !(defined TLS) || (defined TASKSCALAR) */
  }

    return pthread->getTarget();
}

PFPI ctc1_op[] = { ctc1_op_0, ctc1_op_1 };


/* normal version */
OP(ctc2_op_0)
{

  fatal("ctc2: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(ctc2_op_1)
{

  fatal("ctc2: not yet implemented\n");

    return pthread->getTarget();
}

PFPI ctc2_op[] = { ctc2_op_0, ctc2_op_1 };


/* normal version */
OP(ctc3_op_0)
{

  fatal("ctc3: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(ctc3_op_1)
{

  fatal("ctc3: not yet implemented\n");

    return pthread->getTarget();
}

PFPI ctc3_op[] = { ctc3_op_0, ctc3_op_1 };


/* normal version */
OP(bc0f_op_0)
{

  fatal("bc0f: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc0f_op_1)
{

  fatal("bc0f: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc0f_op[] = { bc0f_op_0, bc0f_op_1 };


/* normal version */
OP(bc0t_op_0)
{

  fatal("bc0t: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc0t_op_1)
{

  fatal("bc0t: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc0t_op[] = { bc0t_op_0, bc0t_op_1 };


/* normal version */
OP(bc0fl_op_0)
{

  fatal("bc0fl: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc0fl_op_1)
{

  fatal("bc0fl: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc0fl_op[] = { bc0fl_op_0, bc0fl_op_1 };


/* normal version */
OP(bc0tl_op_0)
{

  fatal("bc0tl: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc0tl_op_1)
{

  fatal("bc0tl: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc0tl_op[] = { bc0tl_op_0, bc0tl_op_1 };


/* normal version */
OP(bc1f_op_0)
{

  /* if condition is false, then branch */
  if ((pthread->getFPUControl31() & 0x00800000) == 0)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bc1f_op_1)
{

  /* if condition is false, then branch */
  if ((pthread->getFPUControl31() & 0x00800000) == 0)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bc1f_op[] = { bc1f_op_0, bc1f_op_1 };


/* normal version */
OP(bc1t_op_0)
{

  /* if condition is true, then branch */
  if (pthread->getFPUControl31() & 0x00800000)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return picode->next;
}

/* branch delay slot version */
OP(bc1t_op_1)
{

  /* if condition is true, then branch */
  if (pthread->getFPUControl31() & 0x00800000)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);

    return pthread->getTarget();
}

PFPI bc1t_op[] = { bc1t_op_0, bc1t_op_1 };


/* normal version */
OP(bc1fl_op_0)
{

    /* if condition is false, then branch */
  if ((pthread->getFPUControl31() & 0x00800000) == 0)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return picode->next;
}

/* branch delay slot version */
OP(bc1fl_op_1)
{

    /* if condition is false, then branch */
  if ((pthread->getFPUControl31() & 0x00800000) == 0)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return pthread->getTarget();
}

PFPI bc1fl_op[] = { bc1fl_op_0, bc1fl_op_1 };


/* normal version */
OP(bc1tl_op_0)
{

    /* if condition is true, then branch */
  if (pthread->getFPUControl31() & 0x00800000)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return picode->next;
}

/* branch delay slot version */
OP(bc1tl_op_1)
{

    /* if condition is true, then branch */
  if (pthread->getFPUControl31() & 0x00800000)
    pthread->setTarget(picode->target);
  else {
    /* The conditional branch is not taken, so nullify the branch delay
     * slot instruction, but still count one cycle for the cost.
     */
    return picode->not_taken;
  }

    return pthread->getTarget();
}

PFPI bc1tl_op[] = { bc1tl_op_0, bc1tl_op_1 };


/* normal version */
OP(bc2f_op_0)
{

    fatal("bc2f: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc2f_op_1)
{

    fatal("bc2f: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc2f_op[] = { bc2f_op_0, bc2f_op_1 };


/* normal version */
OP(bc2t_op_0)
{

    fatal("bc2t: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc2t_op_1)
{

    fatal("bc2t: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc2t_op[] = { bc2t_op_0, bc2t_op_1 };


/* normal version */
OP(bc2fl_op_0)
{

    fatal("bc2fl: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc2fl_op_1)
{

    fatal("bc2fl: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc2fl_op[] = { bc2fl_op_0, bc2fl_op_1 };


/* normal version */
OP(bc2tl_op_0)
{

    fatal("bc2tl: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc2tl_op_1)
{

    fatal("bc2tl: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc2tl_op[] = { bc2tl_op_0, bc2tl_op_1 };


/* normal version */
OP(bc3f_op_0)
{

    fatal("bc3f: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc3f_op_1)
{

    fatal("bc3f: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc3f_op[] = { bc3f_op_0, bc3f_op_1 };


/* normal version */
OP(bc3t_op_0)
{

    fatal("bc3t: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc3t_op_1)
{

    fatal("bc3t: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc3t_op[] = { bc3t_op_0, bc3t_op_1 };


/* normal version */
OP(bc3fl_op_0)
{

    fatal("bc3fl: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc3fl_op_1)
{

    fatal("bc3fl: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc3fl_op[] = { bc3fl_op_0, bc3fl_op_1 };


/* normal version */
OP(bc3tl_op_0)
{

    fatal("bc3tl: not yet implemented\n");

    return picode->next;
}

/* branch delay slot version */
OP(bc3tl_op_1)
{

    fatal("bc3tl: not yet implemented\n");

    return pthread->getTarget();
}

PFPI bc3tl_op[] = { bc3tl_op_0, bc3tl_op_1 };


/* Local Variables: */
/* mode: c */
/* End: */
