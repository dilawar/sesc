#include <stdio.h>

#include "sescapi.h"

#ifndef SESCAPI_NATIVE
/*
 * Implementation of spin lock with "kind-of" back-off
 */
void sesc_lock_init(slock_t * lock)
{
  lock->spin = UNLOCKED;
#ifdef NOSPIN_DOSUSPEND
  lock->waitSpin = UNLOCKED;
  lock->waitingPos = 0;
#endif
}

void sesc_lock(slock_t * lock)
{
#ifdef SESC_LOCKPROFILE
  sesc_startlock();
  sesc_startlock2();
#endif
  lock->dummy=5;
  while(sesc_fetch_op(FetchSwapOp, &lock->spin, LOCKED) == LOCKED) {
    lock->dummy=5;
    int i=0;
    
#ifdef NOSPIN_DOSUSPEND
    int pos;

    while(sesc_fetch_op(FetchSwapOp, &lock->waitSpin, LOCKED) == LOCKED)
      ;

    pos = sesc_fetch_op(FetchIncOp,&lock->waitingPos,0);
    if( pos < MAXLOCKWAITING ) {
      lock->waiting[pos]=sesc_self();
      sesc_fetch_op(FetchSwapOp, &lock->waitSpin, UNLOCKED);
      sesc_suspend(sesc_self());
    }else{
      sesc_fetch_op(FetchDecOp,&lock->waitingPos,0); /* Do not saturate counter */
      sesc_fetch_op(FetchSwapOp, &lock->waitSpin, UNLOCKED);
    }
#endif

    while(lock->spin == LOCKED) {
      i--;
      if( i < 0 ) {
        sesc_yield(-1);
        i=1000;
      }
    }
  };

  sesc_memfence((int)lock);
#ifdef SESC_LOCKPROFILE
  sesc_endlock();
#endif
}

void sesc_unlock(slock_t * lock)
{
#ifdef SESC_LOCKPROFILE
  sesc_startlock();
#endif
  sesc_memfence((int)lock);

  lock->dummy = 5;
  sesc_unlock_op(&lock->spin, UNLOCKED);
  /*  sesc_fetch_op(FetchSwapOp, &lock->spin, UNLOCKED);*/
#ifdef NOSPIN_DOSUSPEND
  while(sesc_fetch_op(FetchSwapOp, &lock->waitSpin, LOCKED) == LOCKED)
      lock->dummy = 5;

  {
  int pos = lock->waitingPos;

  if( pos ) {
    int i;
    if( pos >= MAXLOCKWAITING )
      pos = MAXLOCKWAITING;
 
    sesc_resume(lock->waiting[0]);
    for(i=1;i<pos;i++)
      lock->waiting[i-1]= lock->waiting[i];

    lock->waitingPos= pos-1;
  }
  
  sesc_fetch_op(FetchSwapOp, &lock->waitSpin, UNLOCKED);
  }
#endif
#ifdef SESC_LOCKPROFILE
  sesc_endlock();
  sesc_endlock2();
#endif
}
#endif /* SESCAPI_NATIVE */

#ifdef SESCAPI_NATIVE_IRIX
extern atomic_reservoir_t atomic_reservoir;
#endif

/*
 * Implementation of two phase barrier
 */
void sesc_barrier_init(sbarrier_t * barr)
{
#ifdef SESCAPI_NATIVE_IRIX
  barr->count = atomic_alloc_variable(atomic_reservoir, NULL);
  atomic_clear(barr->count);
#else /* !SESCAPI_NATIVE_IRIX */
  barr->count = 0;
#ifdef NOSPIN_DOSUSPEND
  barr->waitingPos = 0;
#endif /* NOSPIN_DOSUSPEND */
#endif /* SESCAPI_NATIVE_IRIX */
  barr->gsense = 0;
}

void sesc_barrier(sbarrier_t *barr, int num_proc)
{
#ifdef NOSPIN_DOSUSPEND
  int pos;
#endif
  int lsense;
  int i=1000;

  lsense = !barr->gsense;
#ifdef SESCAPI_NATIVE_IRIX
  if((atomic_fetch_and_increment(barr->count)) == num_proc-1) {
    atomic_clear(barr->count);
#else
  if(sesc_fetch_op(FetchIncOp, &barr->count, 0) == num_proc-1) {    /*count = 0;*/
    barr->count = 0;
#endif
    barr->gsense = lsense;
  } else {

#ifdef NOSPIN_DOSUSPEND
    /* IF Joe: I left this in for you!   */
    /* ELSE: DO NOT ACTIVATE THIS CODE   */
    /* Leaving this code in for updating */
    pos = sesc_fetch_op(FetchIncOp,&barr->waitingPos,0);
    if( pos < MAXLOCKWAITING ) {
      barr->waiting[pos]=sesc_self();
      sesc_suspend(sesc_self());
    }
#endif
    while(lsense != barr->gsense){
      i--;
      if( i < 0 ) {
	sesc_yield(-1);
	i=1000;
      }
    };
    
  }
  sesc_memfence((int)barr);
}

/*
 * Implementation of semaphore
 */
void sesc_sema_init(ssema_t *sema, int initValue)
{
  sema->count = initValue;
}

void sesc_psema(ssema_t *sema)
{
  int i=0;
  /* DOWN, wait() */
  while(sema->count <= 0) {
    i--;
    if( i < 0 ) {
      sesc_yield(-1);
      i=1000;
    }
  };

  sesc_fetch_op(FetchDecOp, (int *)&(sema->count), 0);

  sesc_memfence((int)sema);
}

void sesc_vsema(ssema_t * sema)
{
  /* UP, signal() */
  sesc_fetch_op(FetchIncOp, (int *)&(sema->count), 0);

  sesc_memfence((int)sema);
}

void sesc_flag_init(sflag_t *flag){
  flag->flag=0;
  flag->count=0;
  sesc_sema_init(&(flag->queue),0);
  sesc_lock_init(&(flag->lock));
}

void sesc_flag_wait(sflag_t *flag){
  sesc_lock(&(flag->lock));
  if(!flag->flag){
    flag->count++;
    sesc_unlock(&(flag->lock));
    sesc_psema(&(flag->queue));
  }else{
    sesc_unlock(&(flag->lock));
  }
}

void sesc_flag_set(sflag_t *flag){
  sesc_lock(&(flag->lock));
  flag->flag=1;
  while(flag->count){
    flag->count--;
    sesc_vsema(&(flag->queue));
  }
  sesc_unlock(&(flag->lock));
}

void sesc_flag_clear(sflag_t *flag){
  sesc_lock(&(flag->lock));
  flag->flag=0;
  while(flag->count){
    flag->count--;
    sesc_vsema(&(flag->queue));
  }
  sesc_unlock(&(flag->lock));
}

