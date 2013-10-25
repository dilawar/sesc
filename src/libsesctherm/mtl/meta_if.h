/*****************************************************************************/
/*                                                                           */
/* Generative Matrix Package  -  File "IF.h"                                 */
/*                                                                           */
/*                                                                           */
/* Category:   Helper classes                                                */
/*                                                                           */
/* Meta-Functions:                                                           */
/* - IF                                                                      */
/*                                                                           */
/*                                                                           */
/* IF provides an if-then-else metafunction which works with VC++5.0. Some   */
/* additional classes are needed to work around some compiler problems.      */
/*                                                                           */
/*                                                                           */
/* (c) Tobias Neubert, Krzysztof Czarnecki, and Ulrich Eisenecker 1998       */
/*                                                                           */
/*****************************************************************************/

#ifndef _IF_
#define _IF_

#include "mtl_config.h"

namespace mtl {

  //sun workshop 6 update 2 did not handle partial specialized IF
#if (MTL_PARTIAL_SPEC) && (!defined  __SUNPRO_CC)

  template <int cond, class A, class B>
  struct IF { };
  template <class A, class B>
  struct IF<0, A, B> { typedef B RET; };
  template <class A, class B>
  struct IF<1, A, B> { typedef A RET; };

#else

template<int condition, class A, class B>struct IF;
template<int condition>struct SlectSelector;
struct SelectFirstType;
struct SelectSecondType;


struct SelectFirstType
{
	template<class A, class B>
	struct Template
	{
		typedef A RET;
	};
};

struct SelectSecondType
{
	template<class A, class B>
	struct Template
	{
		typedef B RET;
	};
};


template<int condition>
struct SlectSelector
{
	typedef SelectFirstType RET;
};


template <>
struct SlectSelector<1>
{

	typedef SelectFirstType RET;
};


template <>
struct SlectSelector<0>
{

	typedef SelectSecondType RET;
};


template<int condition, class A, class B>
struct IF
{
	typedef typename SlectSelector<condition>::RET selector;
	typedef typename selector:: MTL_TEMPLATE Template<A, B>::RET RET;
};

#endif

} /* namespace mtl */

#endif /* _IF_ */

