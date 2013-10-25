
/************************************************************************************/
/*                                                                                  */
/* Generative Matrix Package  -  File "Equal.h"                                     */
/*                                                                                  */
/*                                                                                  */
/* Category:   Helper classes                                                       */
/*                                                                                  */
/* Meta-Functions:                                                                  */
/* - EQUAL                                                                          */
/*                                                                                  */
/*                                                                                  */
/* EQUAL checks two values for equality at compile time. This function is necessary */
/* because a simple expression 'a == b' doesn't work in every metafunction          */
/* (e.g. does not in IF).                                                           */
/*                                                                                  */
/*                                                                                  */
/* (c) Tobias Neubert, Krzysztof Czarnecki, and Ulrich Eisenecker 1998              */
/*                                                                                  */
/************************************************************************************/

// EQUAL

#ifndef _META_EQUAL_
#define _META_EQUAL_


template<int n1, int n2>
struct EQUAL
{
	enum {RET= n1==n2 ? 1:0};
};

#endif
