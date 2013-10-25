/*
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Code based on Jose Martinez pool code (Thanks)

   Contributed by Jose Renau
                  Milos Prvulovic
                  James Tuck

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _POOL_H
#define _POOL_H

#include "nanassert.h"

// Recycle memory allocated from time to time. This is useful for adapting to
// the phases of the application

#include "Snippets.h"

#ifdef DEBUG
//#define POOL_TIMEOUT 1
#define POOL_SIZE_CHECK
#endif

#ifdef POOL_TIMEOUT
#define POOL_CHECK_CYCLE  12000
#endif


template<class Ttype, class Parameter1, bool noTimeCheck=false>
class pool1 {
protected:
  class Holder : public Ttype {
  public:
    Holder(Parameter1 p1) : Ttype(p1) { }
    Holder *holderNext;
#ifdef POOL_TIMEOUT
    Holder *allNext; // List of Holders when active
    Time_t outCycle; // Only valid if inPool is false
#endif
    ID(bool inPool;)
  };

#ifdef POOL_SIZE_CHECK
  unsigned long psize;
  unsigned long warn_psize;
#endif

  ID(bool deleted;)

#ifdef POOL_TIMEOUT
  Time_t need2cycle;
  Holder *allFirst; // List of Holders when active
#endif

  const int32_t Size; // Reproduction size

  Parameter1 p1;

  Holder *first;  // List of free nodes

  void reproduce() {
    I(first==0);

    for(int32_t i = 0; i < Size; i++) {
      Holder *h = ::new Holder(p1);
      h->holderNext = first;
      IS(h->inPool = true);
#ifdef POOL_TIMEOUT
      h->allNext = allFirst;
      allFirst = h;
#endif
      first   = h;
    }
  }

public:
  pool1(Parameter1 a1, int32_t s = 32)
    : Size(s)
      ,p1(a1) {
    I(Size > 0);
    IS(deleted=false);

#ifdef POOL_SIZE_CHECK
    psize=0;
    warn_psize=s*8;
#endif

    first  = 0;

#ifdef POOL_TIMEOUT
    allFirst = 0;
    need2cycle = globalClock + POOL_CHECK_CYCLE;
#endif

    if( first == 0 )
      reproduce();
  }

  ~pool1() {
    // The last pool whould delete all the crap
    while(first) {
      Holder *h = first;
      first = first->holderNext;
      ::delete h;
    }
    first = 0;
    IS(deleted=true);
  }

  void doChecks() {
#ifdef POOL_TIMEOUT
    if (noTimeCheck)
      return;
    if( need2cycle < globalClock ) {
      Holder *tmp = allFirst;
      while( tmp ) {
        GI(!tmp->inPool, (tmp->outCycle+POOL_CHECK_CYCLE)>need2cycle);
        tmp = tmp->allNext;
      }
      need2cycle = globalClock + POOL_CHECK_CYCLE;
    }
#endif // POOL_TIMEOUT
  }

  void in(Ttype *data) {
    I(!deleted);
    Holder *h = static_cast<Holder *>(data);

    I(!h->inPool);
    IS(h->inPool=true);

    h->holderNext = first;
    first = h;

#ifdef POOL_SIZE_CHECK
    psize--;
#endif

    doChecks();
  }

  Ttype *out() {
    I(!deleted);
    I(first);

    I(first->inPool);
    IS(first->inPool=false);
#ifdef POOL_TIMEOUT
    first->outCycle = globalClock;
    doChecks();
#endif

#ifdef POOL_SIZE_CHECK
    psize++;
    if (psize>=warn_psize) {
      I(0);
      MSG("Pool1 class size grew to %lu", psize);
      warn_psize=4*psize;
    }
#endif

    Ttype *h = static_cast<Ttype *>(first);
    first = first->holderNext;
    if( first == 0 )
      reproduce();

    return h;
  }
};

//*********************************************

template<class Ttype, bool noTimeCheck=false>
class pool {
protected:
  class Holder : public Ttype {
  public:
    Holder *holderNext;
#ifdef POOL_TIMEOUT
    Holder *allNext; // List of Holders when active
    Time_t outCycle; // Only valid if inPool is false
#endif
    ID(bool inPool;)
  };

#ifdef POOL_SIZE_CHECK
  unsigned long psize;
  unsigned long warn_psize;
#endif

  ID(bool deleted;)

#ifdef POOL_TIMEOUT
  Time_t need2cycle;
  Holder *allFirst; // List of Holders when active
#endif

  const int32_t Size; // Reproduction size
  const char *Name;

  Holder *first;  // List of free nodes

  void reproduce() {
    I(first==0);

    for(int32_t i = 0; i < Size; i++) {
      Holder *h = ::new Holder;
      h->holderNext = first;
      IS(h->inPool = true);
#ifdef POOL_TIMEOUT
      h->allNext = allFirst;
      allFirst = h;
#endif
      first   = h;
    }
  }

public:
    pool(int32_t s = 32, const char *n = "pool name not declared")
    : Size(s)
    , Name(n)
    {
    I(Size > 0);
    IS(deleted=false);

#ifdef POOL_SIZE_CHECK
    psize=0;
    warn_psize=s*8;
#endif

    first  = 0;

#ifdef POOL_TIMEOUT
    allFirst = 0;
    need2cycle = globalClock + POOL_CHECK_CYCLE;
#endif

    if( first == 0 )
      reproduce();
  }

  ~pool() {
    // The last pool whould delete all the crap
    while(first) {
      Holder *h = first;
      first = first->holderNext;
      ::delete h;
    }
    first = 0;
    IS(deleted=true);
  }

  void doChecks() {
#ifdef POOL_TIMEOUT
    if (noTimeCheck)
      return;
    if( need2cycle < globalClock ) {
      Holder *tmp = allFirst;
      while( tmp ) {
        GI(!tmp->inPool, (tmp->outCycle+POOL_CHECK_CYCLE)>need2cycle);
        tmp = tmp->allNext;
      }
      need2cycle = globalClock + POOL_CHECK_CYCLE;
    }
#endif // POOL_TIMEOUT
  }

  void in(Ttype *data) {
    I(!deleted);
    Holder *h = static_cast<Holder *>(data);

    I(!h->inPool);
    IS(h->inPool=true);

    h->holderNext = first;
    first = h;

#ifdef POOL_SIZE_CHECK
    psize--;
#endif

    doChecks();
  }

  Ttype *out() {
    I(!deleted);
    I(first);

    I(first->inPool);
    IS(first->inPool=false);
#ifdef POOL_TIMEOUT
    first->outCycle = globalClock;
    doChecks();
#endif

#ifdef POOL_SIZE_CHECK
    psize++;
    if (psize>=warn_psize) {
      #ifndef TLS
      	I(0);
      #endif
      MSG("%s:pool class size grew to %lu", Name, psize);
      warn_psize=4*psize;
    }
#endif

    Ttype *h = static_cast<Ttype *>(first);
    first = first->holderNext;
    if( first == 0 )
      reproduce();

    return h;
  }
};


//*********************************************

template<class Ttype, bool noTimeCheck=false>
class poolplus {
protected:
  class Holder : public Ttype {
  public:
    Holder *holderNext;
#ifdef POOL_TIMEOUT
    Holder *allNext; // List of Holders when active
    Time_t outCycle; // Only valid if inPool is false
#endif
    ID(bool inPool;)
  };

#ifdef POOL_SIZE_CHECK
  unsigned long psize;
  unsigned long warn_psize;
#endif

  ID(bool deleted;)

#ifdef POOL_TIMEOUT
  Time_t need2cycle;
  Holder *allFirst; // List of Holders when active
#endif

  const int32_t Size; // Reproduction size
  const char *Name;

  Holder *first;  // List of free nodes

  void reproduce() {
    I(first==0);

    for(int32_t i = 0; i < Size; i++) {
      Holder *h = ::new Holder;
      h->holderNext = first;
      IS(h->inPool = true);
#ifdef POOL_TIMEOUT
      h->allNext = allFirst;
      allFirst = h;
#endif
      first   = h;
    }
  }

public:
    poolplus(int32_t s = 32, const char *n = "poolplus name not declared")
    : Size(s)
    , Name(n)
    {
    I(Size > 0);
    IS(deleted=false);

#ifdef POOL_SIZE_CHECK
    psize=0;
    warn_psize=s*8;
#endif

    first  = 0;

#ifdef POOL_TIMEOUT
    allFirst = 0;
    need2cycle = globalClock + POOL_CHECK_CYCLE;
#endif

    if( first == 0 )
      reproduce();
  }

  ~poolplus() {
    // The last pool whould delete all the crap
    while(first) {
      Holder *h = first;
      first = first->holderNext;
      ::delete h;
    }
    first = 0;
    IS(deleted=true);
  }

  void doChecks() {
#ifdef POOL_TIMEOUT
    if (noTimeCheck)
      return;
    if( need2cycle < globalClock ) {
      Holder *tmp = allFirst;
      while( tmp ) {
        GI(!tmp->inPool, (tmp->outCycle+POOL_CHECK_CYCLE)>need2cycle);
        tmp = tmp->allNext;
      }
      need2cycle = globalClock + POOL_CHECK_CYCLE;
    }
#endif // POOL_TIMEOUT
  }

  void in(Ttype *data) {
    I(!deleted);
    Holder *h = static_cast<Holder *>(data);

    I(!h->inPool);
    IS(h->inPool=true);

    h->holderNext = first;
    first = h;

#ifdef POOL_SIZE_CHECK
    psize--;
#endif

    doChecks();
  }

  Ttype *out() {
    I(!deleted);
    I(first);

    I(first->inPool);
    IS(first->inPool=false);
#ifdef POOL_TIMEOUT
    first->outCycle = globalClock;
    doChecks();
#endif

#ifdef POOL_SIZE_CHECK
    psize++;
    if (psize>=warn_psize) {
      I(0);
      MSG("%s:pool class size grew to %lu", Name, psize);
      warn_psize=4*psize;
    }
#endif

    Ttype *h = static_cast<Ttype *>(first);
    first = first->holderNext;
    if( first == 0 )
      reproduce();

    h->Ttype::prepare();

    return h;
  }
};

//*********************************************

#endif  // _POOL_H

