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

#ifndef MTL_ALGO_H
#define MTL_ALGO_H

#include "not_at.h"
#include "mtl_config.h"
#include "fast.h"

namespace mtl_algo {


// Section 25.2.1.1 -- Copy

template <class InputIterator, class OutputIterator>
inline OutputIterator
copy(InputIterator f, InputIterator l, OutputIterator r)
{
  InputIterator first = f;
  InputIterator last = l;
  OutputIterator result = r;

  while (mtl::not_at(first, last))
    *result++ = *first++;
  return result;
}

// Section 25.2.1.2 -- Copy backward

template <class BidirectionalIterator1, class BidirectionalIterator2>
inline BidirectionalIterator2
copy_backward(BidirectionalIterator1 first,
              BidirectionalIterator1 last,
              BidirectionalIterator2 result)
{
  while (mtl::not_at(first, last))
    *--result = *--last;
  return result;
}

// Section 25.2.2.1 -- swap

template <class T>
inline void
swap(T& a, T& b)
{
  T tmp = a;
  a = b;
  b = tmp;
}

// Required for implementing swap_ranges and other functions

template <class ForwardIterator1, class ForwardIterator2, class T>
inline void
__iter_swap(ForwardIterator1 a, ForwardIterator2 b, T*)
{
  T tmp = *a;
  *a = *b;
  *b = tmp;
}

template <class ForwardIterator1, class ForwardIterator2>
inline void
iter_swap(ForwardIterator1 a, ForwardIterator2 b)
{
  typedef typename std::iterator_traits <ForwardIterator1>::value_type
    Value;
  mtl_algo::__iter_swap(a, b, (Value*)0);
}

// Section 25.2.5 -- Fill

template <class ForwardIterator, class T>
inline void
fill(ForwardIterator first, ForwardIterator last, const T& value)
{
  while (mtl::not_at(first, last))
    *first++ = value;
}

template <class OutputIterator, class Size, class T>
inline OutputIterator
fill_n(OutputIterator first, Size n, const T& value)
{
  while (n-- > 0)
    *first++ = value;
  return first;
}


// Section 25.2.2.2 -- Swap ranges

template <class ForwardIterator1, class ForwardIterator2>
inline ForwardIterator2
swap_ranges(ForwardIterator1 f1, ForwardIterator1 l1,
            ForwardIterator2 f2)
{
  ForwardIterator1 first1 = f1;
  ForwardIterator1 last1 = l1;
  ForwardIterator2 first2 = f2;

  while (mtl::not_at(first1, last1))
    mtl_algo::swap(*first1++, *first2++);

  return first2;
}


// Section 26.4 -- Generic numeric operations

// Section 26.4.1 -- Accumulate

template <class InputIterator, class T>
inline T
accumulate(InputIterator f, InputIterator l, T init)
{
  InputIterator first = f;
  InputIterator last = l;

  while (mtl::not_at(first, last))
    init = init + *first++;

  return init;
}

template <class InputIterator, class T, class BinaryOperation>
inline T
accumulate(InputIterator f, InputIterator l, T init,
           BinaryOperation binary_op)
{
  InputIterator first = f;
  InputIterator last = l;

  while (mtl::not_at(first, last)) {
    init = binary_op (init, *first++);
  }
  return init;
}

// Section 26.4.2 -- inner product

template <class InputIterator1, class InputIterator2, class T>
inline T
inner_product(InputIterator1 f1, InputIterator1 l1,
              InputIterator2 f2, T init)
{
  InputIterator1 first1 = f1;
  InputIterator1 last1 = l1;
  InputIterator2 first2 = f2;

  while (mtl::not_at(first1, last1))
    init += (*first1++ * *first2++);

  return init;
}

template <class InputIterator1, class InputIterator2, class T,
          class BinaryOperation1, class BinaryOperation2>
inline T
inner_product(InputIterator1 f1, InputIterator1 l1,
              InputIterator2 f2, T init,
              BinaryOperation1 binary_op1,
              BinaryOperation2 binary_op2)
{
  InputIterator1 first1 = f1;
  InputIterator1 last1 = l1;
  InputIterator2 first2 = f2;

  while (mtl::not_at(first1, last1))
    init = binary_op1 (init, binary_op2 (*first1++, *first2++));

  return init;
}

// Section 26.4.3 -- Partial Sum

template <class InputIterator, class OutputIterator>
OutputIterator
partial_sum(InputIterator first, InputIterator last,
            OutputIterator result)
{
  typedef typename std::iterator_traits<InputIterator>::value_type sum_type;
    if( mtl::not_at(first, last) ) {
        sum_type sum1( *first );
        for(;;) {
            *result = sum1;
            ++result;
            if( ++first==last ) break;
            sum_type sum2( sum1 + *first );
            *result = sum2;
            ++result;
            if( ++first==last ) break;
            sum1.~sum_type();
            new((void*)&sum1) sum_type( sum2 + *first );
        }
    }
    return result;
}

template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator
partial_sum (InputIterator first, InputIterator last,
             OutputIterator result, BinaryOperation op)
{
  typedef typename  std::iterator_traits<InputIterator>::value_type sum_type;
    if( mtl::not_at(first, last) ) {
        sum_type sum1( *first );
        for(;;) {
            *result = sum1;
            ++result;
            if( ++first==last ) break;
            sum_type sum2( op( sum1, *first ) );
            *result = sum2;
            ++result;
            if( ++first==last ) break;
            sum1.~sum_type();
            new((void*)&sum1) sum_type( op( sum2, *first ) );
        }
    }
    return result;
}

// Section 26.4.4 -- Adjacent Difference

template <class InputIterator, class OutputIterator>
OutputIterator
adjacent_difference(InputIterator first, InputIterator last,
                    OutputIterator result)
{
  typedef typename std::iterator_traits <InputIterator>::value_type
                     value_type;
    if (mtl::not_at(first, last)) {
        value_type value0( *first );
        *result = value0;
        while( ++result, mtl::not_at(++first, last) ) {
            value_type value1(*first);
            *result = value1 - value0;
            value0.~value_type();
            new((void*)&value0) value_type( value1 );
        }
    }
    return result;
}

template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator
adjacent_difference(InputIterator first, InputIterator last,
                    OutputIterator result,
                    BinaryOperation binary_op)
{
  typedef typename std::iterator_traits <InputIterator>::value_type
                     value_type;
    if (mtl::not_at(first, last)) {
        value_type value0( *first );
        *result = value0;
        while( ++result, mtl::not_at(++first, last) ) {
            value_type value1(*first);
            *result = binary_op(value1,value0);
            value0.~value_type();
            new((void*)&value0) value_type( value1 );
        }
    }
    return result;
}

// Subclause 25.1 -- Non-modifying sequence operations

// Section 25.1.1 -- For each

template <class InputIterator, class Function>
inline Function
for_each(InputIterator f, InputIterator l, Function func)
{
  InputIterator first = f;
  InputIterator last = l;

  for( ; mtl::not_at(first, last); ++first)
    func(*first);
  return func;
}


// Section 25.2.3 -- Transform

template <class InputIterator, class OutputIterator, class UnaryOperation>
inline
OutputIterator
transform(InputIterator f, InputIterator l,
          OutputIterator r, UnaryOperation op)
{
  InputIterator first = f;
  InputIterator last = l;
  OutputIterator result = r;

  while (mtl::not_at(first, last))
    *result++ = op (*first++);

  return result;
}

template <class InputIterator, class OutputIterator>
inline
OutputIterator
transform_add(InputIterator f, InputIterator l,
	      OutputIterator r)
{
  InputIterator first = f;
  InputIterator last = l;
  OutputIterator result = r;

  while (mtl::not_at(first, last)) {
    *result += *first++;
    ++result;
  }
  return result;
}

#if 0
template <class InputIterator1, class InputIterator2, class OutputIterator,
          class BinaryOperation>
inline
OutputIterator
__transform(InputIterator1 f1, InputIterator1 l1,
            InputIterator2 f2, OutputIterator r,
            BinaryOperation binary_op,
            std::random_access_iterator_tag)
{
  InputIterator1 first1 = f1;
  InputIterator1 last1 = l1;
  InputIterator2 first2 = f2;
  OutputIterator result = r;

  typedef typename std::iterator_traits<InputIterator1>::difference_type
    diff_t;

  diff_t n = l1 - f1;

  const int unroll = 8;

  diff_t mod = n % unroll;
  diff_t i;

  for (i = 0; i < mod; ++i)
    result[i] = binary_op (first1[i], first2[i]);

  for (; i < n; i += unroll)
    fast::transform(first1 + i, fast::count<unroll>(),
                    first2 + i, result + i, binary_op);

  return result + n;
}
#endif

template <class InputIterator1, class InputIterator2, class OutputIterator,
          class BinaryOperation, class Tag>
inline
OutputIterator
__transform(InputIterator1 f1, InputIterator1 l1,
            InputIterator2 f2, OutputIterator r,
            BinaryOperation binary_op, Tag)
{
  InputIterator1 first1 = f1;
  InputIterator1 last1 = l1;
  InputIterator2 first2 = f2;
  OutputIterator result = r;

  while (mtl::not_at(first1, last1)) {
    *result = binary_op (*first1, *first2);
    ++first1; ++first2; ++result;
  }
  return result;
}

template <class InputIterator1, class InputIterator2, class OutputIterator,
          class BinaryOperation>
inline
OutputIterator
transform(InputIterator1 f1, InputIterator1 l1,
          InputIterator2 f2, OutputIterator r,
          BinaryOperation binary_op)
{
  typedef typename std::iterator_traits<InputIterator1>::iterator_category Cat;
  return __transform(f1, l1, f2, r, binary_op, Cat());
}


template <class ForwardIterator>
inline ForwardIterator
max_element(ForwardIterator f, ForwardIterator l)
{
  ForwardIterator first = f;
  ForwardIterator last = l;

  if (first == last) return first;
  ForwardIterator result = first;

  while (mtl::not_at(++first, last))
    if (*result < *first)
      result = first;

  return result;
}

template <class ForwardIterator, class Compare>
inline ForwardIterator
max_element(ForwardIterator f, ForwardIterator l,
            Compare comp)
{
  ForwardIterator first = f;
  ForwardIterator last = l;

  if (first == last) return first;
  ForwardIterator result = first;
  while (mtl::not_at(++first, last)) {
    if (comp (*result, *first))
      result = first;
  }
  return result;
}

template <class ForwardIterator>
inline ForwardIterator
min_element(ForwardIterator f, ForwardIterator l)
{
  ForwardIterator first = f;
  ForwardIterator last = l;

  if (first == last) return first;
  ForwardIterator result = first;

  while (mtl::not_at(++first, last))
    if (*first < *result)
      result = first;

  return result;
}

template <class ForwardIterator, class Compare>
inline ForwardIterator
min_element(ForwardIterator f, ForwardIterator l,
            Compare comp)
{
  ForwardIterator first = f;
  ForwardIterator last = l;

  if (first == last) return first;
  ForwardIterator result = first;

  while (mtl::not_at(++first, last))
    if (comp (*first, *result))
      result = first;

  return result;
}

} /* namespace mtl_algo */


#endif
