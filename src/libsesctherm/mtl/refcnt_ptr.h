// -*- c++ -*-
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

#ifndef MTL_REFCNT_PTR_H
#define MTL_REFCNT_PTR_H

#include "mtl_config.h"
#include "mtl_exception.h"

namespace mtl {


template <class Object>
class refcnt_ptr {
  typedef refcnt_ptr<Object> self;
public:

  inline refcnt_ptr() : object(0), count(0) { }

  inline refcnt_ptr(Object* c)
    : object(c), count(0) {
    if ( object )
      count = new int(1);
  }
    //: object(c), count(new int(1)) { }

  inline refcnt_ptr(const self& x) : object(x.object), count(x.count) {
    inc();
  }

  inline ~refcnt_ptr() { dec(); }

  inline self& operator=(Object* c) {
    if (object) dec();
    object = c;
    count = new int(1);
    return *this;
  }

  inline self& operator=(const self& x) {
    if (this == &x)
      return *this;
    if (object) dec();
    object = x.object;
    count = x.count;
    inc();
    return *this;
  }

  inline Object& operator*() MTL_THROW_ASSERTION {
    MTL_ASSERT(count != 0, "refcnt_ptr::operator*()");
    MTL_ASSERT(*count >= 0, "refcnt_ptr::operator*()");
    return *object;
  }

  inline const Object& operator*() const MTL_THROW_ASSERTION {
    MTL_ASSERT(count != 0, "const refcnt_ptr::operator*()");
    MTL_ASSERT(*count >= 0, "const refcnt_ptr::operator*()");
    return *object;
  }

  inline Object* operator->() MTL_THROW_ASSERTION {
    MTL_ASSERT(count != 0, "refcnt_ptr::operator->()");
    MTL_ASSERT(*count >= 0, "refcnt_ptr::operator->()");
    return object;
  }

  inline const Object* operator->() const MTL_THROW_ASSERTION {
    MTL_ASSERT(count != 0, "const refcnt_ptr::operator->()");
    MTL_ASSERT(*count >= 0, "const refcnt_ptr::operator->()");
    return object;
  }

  inline void inc() { if (count) (*count)++; }

  inline void dec() {
    if (count) {
      (*count)--;
      if (*count <= 0) {
	delete object;
	delete count;
      }
    }
  }

protected:

  Object* object;
  int* count;
};

} /* namespace mtl */

#endif
