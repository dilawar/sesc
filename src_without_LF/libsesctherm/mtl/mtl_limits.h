/** -*- C++ -*-
 **  KAI C++ Compiler
 **
 **  Copyright (C) 1996-1997, Kuck & Associates, Inc. All rights reserved.
 **  This is an almost complete rewrite of Modena version.
 **/
/**
 ** Lib++     : The Modena C++ Standard Library,
 **             Version 2.4, August 27th 1997
 **
 ** Copyright (c) 1994-1997 Modena Software Inc.
 **/
#ifndef MTL_LIMIT_H
#define MTL_LIMIT_H

#include "mtl_config.h"

#ifdef MTL_CMPLR_HAS_LIMITS
#include <limits>
#else

#define MTL_LIMITS_THROW

#if 0
#include <mcompile.h>

#include <cfloat>
#include <climits>
#endif

#include <float.h>
#include <limits.h>

namespace std {

enum float_round_style {
     round_indeterminate       = -1,
     round_toward_zero         =  0,
     round_to_nearest          =  1,
     round_toward_infinity     =  2,
     round_toward_neg_infinity =  3
};

template <class T>
class numeric_limits {
public:
    static const bool is_specialized = false;
    static T min() throw() {return T();}
    static T max() throw() {return T();}
    static const int  digits = 0;
    static const int  digits10 = 0;
    static const bool is_signed = false;
    static const bool is_integer = false;
    static const bool is_exact = false;
    static const int  radix = 0;
    static T epsilon() throw() {return T();}
    static T round_error() throw() {return T();}

    static const int  min_exponent = 0;
    static const int  min_exponent10 = 0;
    static const int  max_exponent = 0;
    static const int  max_exponent10 = 0;

    static const bool has_infinity = false;
    static const bool has_quiet_NaN = false;
    static const bool has_signaling_NaN = false;
    static const bool has_denorm = false;
    static const bool has_denorm_loss = false;

    // Sept. 1996 draft is vague on how these can be guaranteed to return
    // value without throwing an exception.
    static T infinity() throw() {return T();};
    static T quiet_NaN() throw() {return T();}
    static T signaling_NaN() throw() {return T();}
    static T denorm_min() throw() {return T();}

    static const bool is_iec559 = false;
    static const bool is_bounded = false;
    static const bool is_modulo = false;

    static const bool traps = false;
    static const bool tinyness_before = false;
    static const float_round_style round_style = round_toward_zero;
};
template<class T> const bool numeric_limits<T>::is_specialized;
template<class T> const int  numeric_limits<T>::digits;
template<class T> const int  numeric_limits<T>::digits10;
template<class T> const bool numeric_limits<T>::is_signed;
template<class T> const bool numeric_limits<T>::is_integer;
template<class T> const bool numeric_limits<T>::is_exact;
template<class T> const int  numeric_limits<T>::radix;
template<class T> const int  numeric_limits<T>::min_exponent;
template<class T> const int  numeric_limits<T>::min_exponent10;
template<class T> const int  numeric_limits<T>::max_exponent;
template<class T> const int  numeric_limits<T>::max_exponent10;
template<class T> const bool numeric_limits<T>::has_infinity;
template<class T> const bool numeric_limits<T>::has_quiet_NaN;
template<class T> const bool numeric_limits<T>::has_signaling_NaN;
template<class T> const bool numeric_limits<T>::has_denorm;
template<class T> const bool numeric_limits<T>::has_denorm_loss;
template<class T> const bool numeric_limits<T>::is_iec559;
template<class T> const bool numeric_limits<T>::is_bounded;
template<class T> const bool numeric_limits<T>::is_modulo;
template<class T> const bool numeric_limits<T>::traps;
template<class T> const bool numeric_limits<T>::tinyness_before;
template<class T> const float_round_style numeric_limits<T>::round_style;

// The specializations for floating-point types use the following macro
// to factor out commonality.  They presume IEEE arithmetic.
#define __KAI_NUMERIC_LIMITS_FLOAT(T)		\
    static const bool is_specialized = true;	\
    static const int  radix = 2;		\
						\
    static const bool is_signed = true;		\
    static const bool is_integer = false;	\
    static const bool is_exact = false;		\
						\
    static const bool has_infinity = true;	\
    static const bool has_quiet_NaN = true;	\
    static const bool has_signaling_NaN = true;	\
    static const bool has_denorm = false;	\
    static const bool has_denorm_loss = false;	\
						\
    static const bool is_iec559 = sizeof(T)<=8;	\
    static const bool is_bounded = true;	\
    static const bool is_modulo = false;	\
    static const bool traps = true;		\
    static const bool tinyness_before = true;	\
 						\
    static T round_error ()   MTL_LIMITS_THROW    { return (T)0.5F; }    	\
    static const float_round_style round_style = round_to_nearest;	\
    static T infinity ()      MTL_LIMITS_THROW {return *(T*)(void*)data.value[0];}\
    static T quiet_NaN ()     MTL_LIMITS_THROW {return *(T*)(void*)data.value[1];}\
    static T signaling_NaN () MTL_LIMITS_THROW {return *(T*)(void*)data.value[2];}\
private:					\
    static const struct data_t {		\
	T align;				\
	int value[3][sizeof(T)/sizeof(int)];	\
    } data;					\
public:						\

template<>
class numeric_limits <float> {
public:
    static const int  digits = FLT_MANT_DIG;
    static const int  digits10 = FLT_DIG;
    static const int  min_exponent = FLT_MIN_EXP;
    static const int  max_exponent = FLT_MAX_EXP;
    static const int  min_exponent10 = FLT_MIN_10_EXP;
    static const int  max_exponent10 = FLT_MAX_10_EXP;

    __KAI_NUMERIC_LIMITS_FLOAT(float)

    static float min ()          MTL_LIMITS_THROW    { return FLT_MIN; }
    static float max ()          MTL_LIMITS_THROW    { return FLT_MAX; }
    static float epsilon ()      MTL_LIMITS_THROW    { return FLT_EPSILON; }
    static float denorm_min ()   MTL_LIMITS_THROW    { return FLT_MIN; }
};

template<>
class numeric_limits <double> {
public:
    static const int  digits = DBL_MANT_DIG;
    static const int  digits10 = DBL_DIG;
    static const int  min_exponent = DBL_MIN_EXP;
    static const int  max_exponent = DBL_MAX_EXP;
    static const int  min_exponent10 = DBL_MIN_10_EXP;
    static const int  max_exponent10 = DBL_MAX_10_EXP;

    __KAI_NUMERIC_LIMITS_FLOAT(double)

    static double min ()          MTL_LIMITS_THROW    { return DBL_MIN; }
    static double max ()          MTL_LIMITS_THROW    { return DBL_MAX; }
    static double epsilon ()      MTL_LIMITS_THROW    { return DBL_EPSILON; }
    static double denorm_min ()   MTL_LIMITS_THROW    { return min (); }
};

template<>
class numeric_limits <long double> {
public:
    static const int  digits = LDBL_MANT_DIG;
    static const int  digits10 = LDBL_DIG;
    static const int  min_exponent = LDBL_MIN_EXP;
    static const int  max_exponent = LDBL_MAX_EXP;
    static const int  min_exponent10 = LDBL_MIN_10_EXP;
    static const int  max_exponent10 = LDBL_MAX_10_EXP;

    __KAI_NUMERIC_LIMITS_FLOAT(long double)

    static long double min ()          MTL_LIMITS_THROW    { return LDBL_MIN; }
    static long double max ()          MTL_LIMITS_THROW    { return LDBL_MAX; }
    static long double epsilon ()      MTL_LIMITS_THROW    { return LDBL_EPSILON; }
    static long double denorm_min ()   MTL_LIMITS_THROW    { return min (); }
};

// The specializations for integral types use three macros to factor out
// commonality.
//
// 	__KAI_NUMERIC_LIMITS_INTEGRAL declares members of numeric_limits<T>
// 	whose value does not depend on the signdness of T.
//
//	__KAI_NUMERIC_LIMITS_SIGNED(T) declares members dependent on
//	knowing that T is signed.
//
//	__KAI_NUMERIC_LIMITS_UNSIGNED(T) declares members dependent on
//	knowing that T is unsigned.
//
// We could have been real cutesy and come up with definitions that would
// work for both signed and unsigned types, but doing so does not seem
// to be worth the additional obfuscation and overhead for constant folding.
//
// The definitions are not intended to be universally portable.
// They are designed with KAI C++ targets in mind. -ADR

#define __KAI_NUMERIC_LIMITS_INTEGRAL(T)			\
    static const bool is_specialized = true;			\
								\
    static const int radix = 2;					\
    static const int min_exponent = 0;				\
    static const int max_exponent = 0;				\
    static const int min_exponent10 = 0;			\
    static const int max_exponent10 = 0;			\
								\
    static const bool is_integer = true;			\
    static const bool is_exact = true;				\
								\
    static const bool has_infinity = false;			\
    static const bool has_quiet_NaN = false;			\
    static const bool has_signaling_NaN = false;		\
    static const bool has_denorm = false;			\
    static const bool has_denorm_loss = false;			\
								\
    static const bool is_iec559 = false;			\
    static const bool is_bounded = true;			\
    static const bool is_modulo = true;				\
    static const bool traps = false;				\
    static const bool tinyness_before = false;			\
								\
    static T infinity ()       MTL_LIMITS_THROW { return 0; }	\
    static T quiet_NaN ()      MTL_LIMITS_THROW { return 0; }	\
    static T signaling_NaN () MTL_LIMITS_THROW { return 0; }		\
    static T epsilon ()     MTL_LIMITS_THROW { return 1; }		\
    static T denorm_min ()  MTL_LIMITS_THROW { return min (); }	\
    static T round_error () MTL_LIMITS_THROW { return 0; }		\
								\
    static const float_round_style round_style = round_toward_zero;

#define __KAI_NUMERIC_LIMITS_SIGNED(T) 				\
    static const int digits = 8*sizeof(T)-1;			\
    /* Following presumes 8, 16, 32, or 64-bit T. */		\
    static const int digits10 = 7*sizeof(T)/3; 			\
    static const bool is_signed = true;

#define __KAI_NUMERIC_LIMITS_UNSIGNED(T) 			\
    static const int digits = 8*sizeof(T);			\
    /* Following presumes 8, 16, 32, or 64-bit T. */		\
    static const int digits10 = 12*sizeof(T)/5;  		\
    static const bool is_signed = false;

template<>
class numeric_limits <int> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(int)
    __KAI_NUMERIC_LIMITS_SIGNED(int)
    static int min() MTL_LIMITS_THROW { return INT_MIN; }
    static int max() MTL_LIMITS_THROW { return INT_MAX; }
};

template<>
class numeric_limits <unsigned int> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(unsigned int)
    __KAI_NUMERIC_LIMITS_UNSIGNED(unsigned int)
    static unsigned int min() MTL_LIMITS_THROW { return 0; }
    static unsigned int max() MTL_LIMITS_THROW { return UINT_MAX; }
};

template<>
class numeric_limits <long> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(long)
    __KAI_NUMERIC_LIMITS_SIGNED(long)
    static long min() MTL_LIMITS_THROW { return LONG_MIN; }
    static long max() MTL_LIMITS_THROW { return LONG_MAX; }
};

template<>
class numeric_limits <unsigned long> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(unsigned long)
    __KAI_NUMERIC_LIMITS_UNSIGNED(unsigned long)
    static unsigned long min() MTL_LIMITS_THROW { return 0; }
    static unsigned long max() MTL_LIMITS_THROW { return ULONG_MAX; }
};

template<>
class numeric_limits <short> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(short)
    __KAI_NUMERIC_LIMITS_SIGNED(short)
    static short min () MTL_LIMITS_THROW { return SHRT_MIN; }
    static short max () MTL_LIMITS_THROW { return SHRT_MAX; }
};

template<>
class numeric_limits <unsigned short> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(unsigned short)
    __KAI_NUMERIC_LIMITS_UNSIGNED(unsigned short)
    static unsigned short min () MTL_LIMITS_THROW { return 0; }
    static unsigned short max () MTL_LIMITS_THROW { return USHRT_MAX; }
};

template<>
class numeric_limits <char> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(char)
    static const int digits = CHAR_MIN<0 ? 7 : 8;
    static const int digits10 = 2;
    static const bool is_signed = CHAR_MIN<0;
    static char min () MTL_LIMITS_THROW { return CHAR_MIN; }
    static char max () MTL_LIMITS_THROW { return CHAR_MAX; }
};

template<>
class numeric_limits <signed char> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(signed char)
    __KAI_NUMERIC_LIMITS_SIGNED(signed char)
    static signed char min ()         MTL_LIMITS_THROW { return SCHAR_MIN; }
    static signed char max ()         MTL_LIMITS_THROW { return SCHAR_MAX; }
};

template<>
class numeric_limits <unsigned char> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(unsigned char)
    __KAI_NUMERIC_LIMITS_UNSIGNED(unsigned char)
    static unsigned char min ()         MTL_LIMITS_THROW { return 0; }
    static unsigned char max ()         MTL_LIMITS_THROW { return UCHAR_MAX; }
};

#if 0
#ifdef MTL_WCHART

template<>
class numeric_limits <wchar_t> {
public:
    __KAI_NUMERIC_LIMITS_INTEGRAL(wchar_t)
    static const bool is_signed = (wchar_t)-1<0;
    static const int digits = 8*sizeof(wchar_t) - is_signed;
    // Following assumes that wchar_t is 8, 16, or 32-bit,
    // either signed or unsigned.
    static const int digits10 = 7*sizeof(T)/3;
    static char min () MTL_LIMITS_THROW { return CHAR_MIN; }
    static char max () MTL_LIMITS_THROW { return CHAR_MAX; }
};

#endif /* MTL_WCHART */

#ifdef _BOOL
template<>
class numeric_limits <bool> {
public:
    static const bool is_specialized = true;

    static const int radix = 2;
    static const int min_exponent = 0;
    static const int max_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent10 = 0;

    static const bool is_integer = false;
    static const bool is_exact = true;

    static const bool has_infinity = false;
    static const bool has_quiet_NaN = false;
    static const bool has_signaling_NaN = false;
    static const bool has_denorm = false;
    static const bool has_denorm_loss = false;

    static const bool is_iec559 = false;
    static const bool is_bounded = true;
    static const bool is_modulo = false;
    static const bool traps = false;
    static const bool tinyness_before = false;

    static bool infinity ()       MTL_LIMITS_THROW { return false; }
    static bool quiet_NaN ()      MTL_LIMITS_THROW { return false; }
    static bool signaling_NaN () MTL_LIMITS_THROW { return false; }
    static bool epsilon ()     MTL_LIMITS_THROW { return false; }
    static bool denorm_min ()  MTL_LIMITS_THROW { return min (); }
    static bool round_error () MTL_LIMITS_THROW { return false; }

    static const float_round_style round_style = round_toward_zero;

    static const int digits = 1;
    static const int digits10 = 0;
    static const bool is_signed = false;

    static bool min ()         MTL_LIMITS_THROW { return false; }
    static bool max ()         MTL_LIMITS_THROW { return true; }
};
#endif	/* _BOOL */
#endif

} /* namespace std */

#endif /* MTL_CMPLR_HAS_LIMITS */

#endif /* MTL_LIMIT_H */
