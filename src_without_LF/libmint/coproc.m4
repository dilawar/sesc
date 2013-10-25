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

M4_IN(c1_abs_s,
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
})

M4_IN(c1_abs_d,
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
})

M4_IN(c1_add_s,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) + pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_add_d,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) + pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_c_eq_s,
{
  if( pthread->getFP(picode, ICODEFS) == pthread->getFP(picode, ICODEFT) && !isunordered(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_eq_d,
{
  if( pthread->getDP(picode, ICODEFS) == pthread->getDP(picode, ICODEFT) && !isunordered(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_f_s,
{
  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_f_d,
{
  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_le_s,
{
  if( islessequal(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_le_d,
{
  if( islessequal(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_lt_s,
{
  if( isless(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_lt_d,
{
  if( isless(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_COND_FMT(c1_c_nge)
M4_COND_FMT(c1_c_ngl)
M4_COND_FMT(c1_c_ngle)
M4_COND_FMT(c1_c_ngt)

M4_IN(c1_c_ole_s,
{
  if( islessequal(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_ole_d,
{
  if( islessequal(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_olt_s,
{
  if( isless(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_olt_d,
{
  if( isless(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_COND_FMT(c1_c_seq)

M4_IN(c1_c_sf_s,
{
  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_sf_d,
{
  pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_ueq_s,
{
  if( pthread->getFP(picode, ICODEFS) == pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_ueq_d,
{
  if( pthread->getDP(picode, ICODEFS) == pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_ule_s,
{
  if( pthread->getFP(picode, ICODEFS) <= pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_ule_d,
{
  if( pthread->getDP(picode, ICODEFS) <= pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_ult_s,
{
  if( pthread->getFP(picode, ICODEFS) < pthread->getFP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_ult_d,
{
  if( pthread->getDP(picode, ICODEFS) < pthread->getDP(picode, ICODEFT) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_c_un_s,
{
  if( isunordered(pthread->getFP(picode, ICODEFS),pthread->getFP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})
M4_IN(c1_c_un_d,
{
  if( isunordered(pthread->getDP(picode, ICODEFS),pthread->getDP(picode, ICODEFT)) )
    pthread->setFPUControl31(pthread->getFPUControl31() | 0x00800000);
  else
    pthread->setFPUControl31(pthread->getFPUControl31() & 0xFF7FFFFF);
})

M4_IN(c1_ceil_w_s,
{
    unimplemented_op(picode, pthread);
})

M4_IN(c1_ceil_w_d,
{
    unimplemented_op(picode, pthread);
})

M4_IN(c1_cvt_d_s,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, (double)pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_cvt_d_w,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, (double)pthread->getWFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_cvt_s_d,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, (float)pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_cvt_s_w,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, (float)pthread->getWFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_cvt_w_s,
{
  pthread->setWFP(picode, ICODEFD, mips_cvt_w_s(pthread->getFP(picode, ICODEFS), pthread->getFPUControl31()));
})

M4_IN(c1_cvt_w_d,
{
  pthread->setWFP(picode, ICODEFD, mips_cvt_w_d(pthread->getDP(picode, ICODEFS), pthread->getFPUControl31()));
})

M4_IN(c1_div_s,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) / pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_div_d,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif

  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) / pthread->getDP(picode, ICODEFT));

#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_floor_w_s,
{
  unimplemented_op(picode, pthread);
})

M4_IN(c1_floor_w_d,
{
  unimplemented_op(picode, pthread);
})

M4_IN(c1_mov_s,
{
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS));
})

M4_IN(c1_mov_d,
{
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS));
})

M4_IN(c1_mul_s,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) * pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_mul_d,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) * pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_neg_s,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, -pthread->getFP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_neg_d,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, -pthread->getDP(picode, ICODEFS));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_round_w_s,
{
  unimplemented_op(picode, pthread);
})

M4_IN(c1_round_w_d,
{
  unimplemented_op(picode, pthread);
})

M4_IN(c1_sqrt_s,
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
})

M4_IN(c1_sqrt_d,
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
})

M4_IN(c1_sub_s,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setFP(picode, ICODEFD, pthread->getFP(picode, ICODEFS) - pthread->getFP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_sub_d,
{
#if (defined TLS) || (defined TASKSCALAR)
  NativeFPUControlType fpuCtrl=changeFPUControl(pthread->getFPUControl31());
#endif
  pthread->setDP(picode, ICODEFD, pthread->getDP(picode, ICODEFS) - pthread->getDP(picode, ICODEFT));
#if (defined TLS) || (defined TASKSCALAR)
  restoreFPUControl(fpuCtrl);
#endif
})

M4_IN(c1_trunc_w_s,
{
  int pp = (int)(truncf(pthread->getFP(picode, ICODEFS)));
  pthread->setFP(picode, ICODEFD, *(float *)&pp);
})


M4_IN(c1_trunc_w_d,
{
  long long pp = (long long)(trunc(pthread->getDP(picode, ICODEFS)));
  pthread->setDP(picode, ICODEFD, *(double *)&pp);
})


M4_IN(cop0_op,
{
  fatal("cop0: not yet implemented\n");
})

M4_IN(cop1_op,
{
  fatal("cop1: not yet implemented\n");
})

M4_IN(cop2_op,
{
  fatal("cop2: not yet implemented\n");
})

M4_IN(cop3_op,
{
  fatal("cop3: not yet implemented\n");
})

M4_IN(mfc0_op,
{
  fatal("mfc0: not yet implemented\n");
})

M4_IN(mfc1_op,
{
  pthread->setREG(picode, RT, pthread->getWFP(picode, ICODEFS));
})

M4_IN(mfc2_op,
{
  fatal("mfc2: not yet implemented\n");
})

M4_IN(mfc3_op,
{
  fatal("mfc3: not yet implemented\n");
})

M4_IN(mtc0_op,
{
  fatal("mtc0: not yet implemented\n");
})

M4_IN(mtc1_op,
{
  pthread->setWFP(picode, ICODEFS, pthread->getREG(picode, RT));
})

M4_IN(mtc2_op,
{
  fatal("mtc2: not yet implemented\n");
})

M4_IN(mtc3_op,
{
  fatal("mtc3: not yet implemented\n");
})

M4_IN(cfc0_op,
{
  fatal("cfc0: not yet implemented\n");
})

M4_IN(cfc1_op,
{
  if (picode->args[ICODEFS] == 0)
    pthread->setREG(picode, RT, pthread->getFPUControl0());
  else
    pthread->setREG(picode, RT, pthread->getFPUControl31());
})

M4_IN(cfc2_op,
{
  fatal("cfc2: not yet implemented\n");
})

M4_IN(cfc3_op,
{
  fatal("cfc3: not yet implemented\n");
})

M4_IN(ctc0_op,
{
  fatal("ctc0: not yet implemented\n");
})

M4_IN(ctc1_op,
{
  if (picode->args[ICODEFS] == 0)
    pthread->setFPUControl0(pthread->getREG(picode, RT));
  else{
    pthread->setFPUControl31(pthread->getREG(picode, RT));
#if !((defined TLS) || (defined TASKSCALAR))
    // setFPUControl31(pthread->getFPUControl31());
#endif /* !(defined TLS) || (defined TASKSCALAR) */
  }
})

M4_IN(ctc2_op,
{
  fatal("ctc2: not yet implemented\n");
})

M4_IN(ctc3_op,
{
  fatal("ctc3: not yet implemented\n");
})

M4_IN(bc0f_op,
{
  fatal("bc0f: not yet implemented\n");
})

M4_IN(bc0t_op,
{
  fatal("bc0t: not yet implemented\n");
})

M4_IN(bc0fl_op,
{
  fatal("bc0fl: not yet implemented\n");
})

M4_IN(bc0tl_op,
{
  fatal("bc0tl: not yet implemented\n");
})

M4_IN(bc1f_op,
{
  /* if condition is false, then branch */
  if ((pthread->getFPUControl31() & 0x00800000) == 0)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);
})

M4_IN(bc1t_op,
{
  /* if condition is true, then branch */
  if (pthread->getFPUControl31() & 0x00800000)
    pthread->setTarget(picode->target);
  else
    pthread->setTarget(picode->not_taken);
})

M4_IN(bc1fl_op,
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
})

M4_IN(bc1tl_op,
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
})

M4_IN(bc2f_op,
{
    fatal("bc2f: not yet implemented\n");
})

M4_IN(bc2t_op,
{
    fatal("bc2t: not yet implemented\n");
})

M4_IN(bc2fl_op,
{
    fatal("bc2fl: not yet implemented\n");
})

M4_IN(bc2tl_op,
{
    fatal("bc2tl: not yet implemented\n");
})

M4_IN(bc3f_op,
{
    fatal("bc3f: not yet implemented\n");
})

M4_IN(bc3t_op,
{
    fatal("bc3t: not yet implemented\n");
})

M4_IN(bc3fl_op,
{
    fatal("bc3fl: not yet implemented\n");
})

M4_IN(bc3tl_op,
{
    fatal("bc3tl: not yet implemented\n");
})

/* Local Variables: */
/* mode: c */
/* End: */
