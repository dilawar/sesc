
/* C functions to emulate MIPS instructions on machines without MIPS
 * processors.
 */


#include <stdint.h>
#include <mendian.h>
#include <stdio.h>
#include "icode.h"
#include "globals.h"

#if defined(__i386__) || defined(__x86_64__) 
#include <fpu_control.h>
/* Sets __i386__ FPU rounding flags according to the rouding bits in
   MIPS. MIPS uses the first two bits (bit1,bit0). I387 uses 11 and 10
   bits.
   
   MIPS I387         Meaning
   00     00 (0x000)  Nearest
   01     11 (0xC00)  Zero
   10     10 (0x800)  Plus Inf
   11     01 (0x400)  Minus Inf
*/
NativeFPUControlType changeFPUControl(uint32_t mipsFlag) {
  NativeFPUControlType oldValue, newValue;
  _FPU_GETCW(oldValue);
  /* Change the lowest 2 bits to correspond to the i387 rounding */ 
  mipsFlag^=2*mipsFlag;
  /* Get only the lowest 2 bits of the MIPS rounding control */ 
  mipsFlag&=3;
  /* Now move the bits to where the i387 wants them */
  mipsFlag*=0x400;
  /* Change the rounding control bits */
  newValue=(oldValue&0xF3FF)|mipsFlag;
   _FPU_SETCW(oldValue);
  return oldValue;
}

void setFPUControl(uint32_t mipsFlag)
{
  changeFPUControl(mipsFlag & 0x3);
}

void restoreFPUControl(NativeFPUControlType i387Flag) {
  _FPU_SETCW(i387Flag);
}

#elif defined(DARWIN)
#include <fenv.h>

static uint32_t FloatValue [] ={FE_TONEAREST,  /* 00 nearest   */
                                    FE_TOWARDZERO, /* 01 zero      */
                                    FE_UPWARD,     /* 10 plus inf  */
                                    FE_DOWNWARD    /* 11 minus inf */
};
void setFPUControl(uint32_t mipsFlag)
{
  fesetround(FloatValue[mipsFlag & 0x3]);
}

NativeFPUControlType changeFPUControl(uint32_t mipsFlag)
{
  NativeFPUControlType oldValue;

  oldValue = fegetround();

  setFPUControl(mipsFlag);

  return oldValue;
}

void restoreFPUControl(NativeFPUControlType nativeFlag)
{
  fesetround(nativeFlag & 0x3);
}
#elif defined(sparc) || defined(__x86)
// SPARC machine or SUNCC/x86
// sun machine hopefully
#include <ieeefp.h>
#include <math.h>

static fp_rnd FloatValue [] ={FP_RN, /* nearest */
                              FP_RM, /* --zero */
                              FP_RP, /* plus Inf */
                              FP_RZ  /* --minus inf */
};

void setFPUControl(uint32_t mipsFlag) {
  fpsetround(FloatValue[mipsFlag & 0x3]);
}

NativeFPUControlType changeFPUControl(uint32_t mipsFlag) {
  return fpsetround(FloatValue[mipsFlag & 0x3]);
}

void restoreFPUControl(NativeFPUControlType nativeFlag) {
  fpsetround((fp_rnd)nativeFlag );
}

#else

#warning "setFPUControl not implemented for this architecture"

void setFPUControl(uint32_t mipsFlag) {
  fesetround(FloatValue[mipsFlag & 0x3]);
}

NativeFPUControlType changeFPUControl(uint32_t mipsFlag) {
  return fesetround(FloatValue[mipsFlag & 0x3]);
}

static inline void restoreFPUControl(NativeFPUControlType nativeFlag) {
  fpsetround((fp_rnd)nativeFlag );
}
#endif

uint32_t mips_div(int32_t a, int32_t b, int32_t *lo, int32_t *hi)
{
  *lo = a / b;
  *hi = a % b;
  return 0;
}

uint32_t mips_divu(uint32_t a, uint32_t b, int32_t *lo, int32_t *hi)
{
  *lo = a / b;
  *hi = a % b;
  return 0;
}

uint32_t mips_lwlBE(int32_t value, char *addr)
{
  uint32_t rval;
  char *pval;
        
  rval = value;
  pval = (char *) &rval;
  *pval++ = *addr++;
  while ((intptr_t) addr & 0x3)
    *pval++ = *addr++;
  return rval;
}

uint32_t mips_lwlLE(int32_t value, char *addr)
{
  uint32_t rval;
  uint32_t offset;
  char *pval;

  offset = ((uintptr_t)addr) & 0x3;
  rval = value;
  pval = (char *) &rval+3;
        
  do{
    *pval-- = *addr++;
  }while(offset++<3);
        
  return rval;
}

int32_t mips_lwrBE(int32_t value, char *addr)
{
  int32_t rval;
  char *pval;

  rval = value;
  pval = (char *) &rval + 3;
  *pval-- = *addr--;
  while (((intptr_t) addr & 0x3) != 0x3)
    *pval-- = *addr--;
  return rval;
}

int32_t mips_lwrLE(int32_t value, char *addr)
{
  int32_t rval;
  char *pval;

  RAddr offset = ((RAddr)addr)&0x3;
  addr = (char *)(((RAddr)addr)-offset);
        
  rval = value;
  pval = (char *) &rval + offset;
        
  do{
    *pval-- = *addr++;
  }while(offset-->0);
        
  return rval;
}

/* This routine computes the signed 64-bit product of the signed 32-bit
 * parameters "a" and "b" and stores the low 32 bits of the result in the
 * location pointed to by "lo" and stores the high 32 bits of the result
 * in the location pointed to by "hi".
 * 
 * There must be a more efficient way to do signed 32-bit multiplication.
 */
int32_t mips_mult(int32_t a, int32_t b, int32_t *lo, int32_t *hi)
{
  uint32_t ahi, alo, bhi, blo;
  uint32_t part1, part2, part3, part4;
  uint32_t p2lo, p2hi, p3lo, p3hi;
  uint32_t ms1, ms2, ms3, sum, carry;
  uint32_t extra;

  ahi = (uint32_t) a >> 16;
  alo = a & 0xffff;
  bhi = (uint32_t) b >> 16;
  blo = b & 0xffff;

  /* compute the partial products */
  part4 = ahi * bhi;
  part3 = ahi * blo;
  part2 = alo * bhi;
  part1 = alo * blo;

  p2lo = part2 << 16;
  p2hi = part2 >> 16;
  p3lo = part3 << 16;
  p3hi = part3 >> 16;

  /* compute the carry bit by checking the high-order bits of the addends */
  carry = 0;
  sum = part1 + p2lo;
  ms1 = part1 & 0x80000000;
  ms2 = p2lo & 0x80000000;
  ms3 = sum & 0x80000000;
  if ((ms1 & ms2) || ((ms1 | ms2) & ~ms3))
    carry++;
  ms1 = p3lo & 0x80000000;
  sum += p3lo;
  ms2 = sum & 0x80000000;
  if ((ms1 & ms3) || ((ms1 | ms3) & ~ms2))
    carry++;

  *lo = sum;

  extra = 0;
  if (a < 0)
    extra = b;
  if (b < 0)
    extra += a;
    
  *hi = part4 + p2hi + p3hi + carry - extra;
  return 0;
}

/* This routine computes the unsigned 64-bit product of the unsigned 32-bit
 * parameters "a" and "b" and stores the low 32 bits of the result in the
 * location pointed to by "lo" and stores the high 32 bits of the result
 * in the location pointed to by "hi".
 */
int32_t mips_multu(uint32_t a, uint32_t b, int32_t *lo, int32_t *hi)
{
  uint32_t ahi, alo, bhi, blo;
  uint32_t part1, part2, part3, part4;
  uint32_t p2lo, p2hi, p3lo, p3hi;
  uint32_t ms1, ms2, ms3, sum, carry;

  ahi = a >> 16;
  alo = a & 0xffff;
  bhi = b >> 16;
  blo = b & 0xffff;

  /* compute the partial products */
  part4 = ahi * bhi;
  part3 = ahi * blo;
  part2 = alo * bhi;
  part1 = alo * blo;

  p2lo = part2 << 16;
  p2hi = part2 >> 16;
  p3lo = part3 << 16;
  p3hi = part3 >> 16;

  /* compute the carry bit by checking the high-order bits of the addends */
  carry = 0;
  sum = part1 + p2lo;
  ms1 = part1 & 0x80000000;
  ms2 = p2lo & 0x80000000;
  ms3 = sum & 0x80000000;
  if ((ms1 & ms2) || ((ms1 | ms2) & ~ms3))
    carry++;
  ms1 = p3lo & 0x80000000;
  sum += p3lo;
  ms2 = sum & 0x80000000;
  if ((ms1 & ms3) || ((ms1 | ms3) & ~ms2))
    carry++;

  *lo = sum;
  *hi = part4 + p2hi + p3hi + carry;
  return 0;
}

void mips_swlBE(int32_t value, char *addr)
{
  char *pval;

  pval = (char *) &value;
  *addr++ = *pval++;
  while ((intptr_t) addr & 0x3)
    *addr++ = *pval++;
}

void mips_swlLE(int32_t value, char *addr)
{
  char *pval;
  uint32_t offset;

  offset = ((uintptr_t)addr) & 0x3;
  pval = (char *) &value+3 ;
        
  do{
    *addr++ = *pval--;
  }while(offset++<3);
}


void mips_swrBE(int32_t value, char *addr)
{
  char *pval;

  pval = (char *) &value + 3;
  *addr-- = *pval--;
  while (((intptr_t) addr & 0x3) != 0x3){
    *addr-- = *pval--;
  }
}

void mips_swrLE(int32_t value, char *addr)
{
  char *pval;
  uint32_t offset;

  offset = ((uintptr_t)addr) & 0x3;
  offset = 3 - offset;
  /*      addr = (char *)(((unsigned long )addr) & 0xFFFFFFFC); */
        
  pval = (char *) &value;
        
  do{
    *addr-- = *pval++;
  }while(offset++<3);
}


/* The conversion from float to int32_t on the SPARC ignores the rounding mode
 * bits, but the MIPS follows them faithfully. For example, when round-to-
 * nearest or round-to-plus-infinity is used, 1234.6 should be converted
 * to 1235, but the SPARC always truncates. So we have to do the conversion
 * explicitly. (Ugh!)
 */
int32_t mips_cvt_w_s(float pf, int32_t fsr)
{
  int32_t ival, rmode;

  ival = (int) pf;
  rmode = fsr & 3;
  if (ival < 0) {
    if (rmode == 0) {
      ival = (int) (pf - 0.5);
      /* -1.5 rounds down to -2, but -2.5 rounds up to -2 */
      if ((ival == pf - 0.5) && (ival & 1))
	ival++;
    } else {
      if (rmode == 3 && ival != pf)
	ival--;
    }
  } else {
    if (rmode == 0) {
      ival = (int) (pf + 0.5);
      /* 1.5 rounds up to 2, but 2.5 rounds down to 2 */
      if ((ival == pf + 0.5) && (ival & 1))
	ival--;
    } else {
      if (rmode == 2 && ival != pf)
	ival++;
    }
  }
  return ival;
}

int32_t mips_cvt_w_d(double dval, int32_t fsr)
{
  int32_t ival, rmode;

  rmode = fsr & 3;
  if (dval < 0) {
    if (rmode == 0) {
      ival = (int) (dval - 0.5);
      /* -1.5 rounds down to -2, but -2.5 rounds up to -2 */
      if ((ival == dval - 0.5) && (ival & 1))
	ival++;
    } else {
      ival = (int) dval;
      if (rmode == 3 && ival != dval)
	ival--;
    }
  } else {
    if (rmode == 0) {
      ival = (int) (dval + 0.5);
      /* 1.5 rounds up to 2, but 2.5 rounds down to 2 */
      if ((ival == dval + 0.5) && (ival & 1))
	ival--;
    } else {
      ival = (int) dval;
      if (rmode == 2 && ival != dval)
	ival++;
    }
  }
  return ival;
}

