#ifndef SESCAPI_H
#define SESCAPI_H

#ifdef SIMICS
#include "simicsapi.h"
#endif


/*
 * Flags specification:
 *
 * flags are used in sesc_spawn and sesc_sysconf. Both cases share the
 * same flags structure, but some flag parameters are valid in some
 * cases.
 *
 * Since the pid has only 16 bits and flags are 32 bits, the lower 16
 * bits are a parameter to the flags.
 */

/* The created thread is not a allowed to migrate between CPUs on
 * context switchs.
 */
#define SESC_FLAG_NOMIGRATE  0x10000

/* The tid parameter (the lower 16 bits) indicate in which CPU this
 * thread is going to be mapped. If the flag SESC_SPAWN_NOMIGRATE is
 * also provided, the thread would always be executed in the same
 * CPU. (unless sesc_sysconf is modifies the flags status)
 */
#define SESC_FLAG_MAP        0x20000

#define SESC_FLAG_FMASK      0x8fff0000
#define SESC_FLAG_DMASK      0xffff

/* Example of utilization:
 *
 * tid = sesc_spawn(func,0,SESC_FLAG_NOMIGRATE|SESC_FLAG_MAP|7);
 * This would map the thread in the processor 7 for ever.
 *
 * sesc_sysconf(tid,SESC_FLAG_NOMIGRATE|SESC_FLAG_MAP|2);
 * Moves the previous thread (tid) to the processor 2
 *
 * sesc_sysconf(tid,SESC_FLAG_MAP|2); 
 * Keeps the thread tid in the same processor, but it is allowed to
 * migrate in the next context switch.
 *
 * tid = sesc_spawn(func,0,0);
 * Creates a thread and maps it to the processor an iddle processor if
 * possible.
 *
 * tid = sesc_spawn(func,0,SESC_FLAG_NOMIGRATE);
 * The same that the previous, but once assigned to a processor, it
 * never migrates.
 *
 */

enum FetchOpType {
  FetchIncOp = 0,
  FetchDecOp,
  FetchSwapOp
};

#define LOCKED    1
#define UNLOCKED  0

#define MAXLOCKWAITING 1023

/* #define  NOSPIN_DOSUSPEND 1 */

#if defined(SESCAPI_NATIVE_IRIX) && !defined(SESCAPI_NATIVE)
#define SESCAPI_NATIVE 1
#endif

#ifdef SESCAPI_NATIVE
#include <pthread.h>

#define slock_t pthread_mutex_t

#define sesc_lock_init(x) pthread_mutex_init(x,0)
#define sesc_lock(x)      pthread_mutex_lock(x)
#define sesc_unlock(x)    pthread_mutex_unlock(x)

#ifdef SESCAPI_NATIVE_IRIX
#include <sys/pmo.h>
#include <fetchop.h>
#endif

#else
typedef struct {
  volatile int32_t spin;            /* lock spins */
  volatile int32_t dummy;
#ifdef NOSPIN_DOSUSPEND
  volatile int32_t waitSpin;
  volatile int32_t waitingPos;
  int32_t waiting[MAXLOCKWAITING+1];
#endif
} slock_t;
#endif  /* !SESCAPI_NATIVE */


typedef struct {
  volatile int32_t gsense;
#ifdef SESCAPI_NATIVE_IRIX
  atomic_var_t *count;
#else /* !SESCAPI_NATIVE_IRIX */
  volatile int32_t count;    /* the count of entered processors */
#ifdef NOSPIN_DOSUSPEND
  /* Only for the enter phase */
  volatile int32_t waitingPos;
  int32_t waiting[MAXLOCKWAITING+1];
#endif
#endif /* SESCAPI_NATIVE_IRIX */
} sbarrier_t;

typedef struct {
  int32_t count;                   /* shared object, the number of arrived processors */
  volatile int32_t gsen;            /* shared object, the global sense */
} sgbarr_t;

typedef struct {
  int32_t lsen;                     /* local object, the local sense */
} slbarr_t;

typedef struct {
  volatile int32_t count;          /* the number of resource available to the semaphore */
} ssema_t;

/* A flag, implemented an in the original ANL macros for Splash-2 */
typedef struct{
  int32_t     flag;
  int32_t     count;
  slock_t lock;
  ssema_t queue;
} sflag_t;

#ifdef __cplusplus
extern "C" {
#endif
  void sesc_preevent_(int32_t vaddr, int32_t type, void *sptr);
  void sesc_preevent(int32_t vaddr, int32_t type, void *sptr);
  void sesc_postevent_(int32_t vaddr, int32_t type, const void *sptr);
  void sesc_postevent(int32_t vaddr, int32_t type, const void *sptr);
  void sesc_memfence_(int32_t vaddr);
  void sesc_memfence(int32_t vaddr);
  void sesc_acquire_(int32_t vaddr);
  void sesc_acquire(int32_t vaddr);
  void sesc_release_(int32_t vaddr);
  void sesc_release(int32_t vaddr);

  void sesc_init(void);
  int32_t  sesc_get_num_cpus(void);
  void sesc_sysconf(int32_t tid, int32_t flags);
  int32_t  sesc_spawn(void (*start_routine) (void *), void *arg, int32_t flags);
  int32_t  sesc_self(void);
  int32_t  sesc_suspend(int32_t tid);
  int32_t  sesc_resume(int32_t tid);
  int32_t  sesc_yield(int32_t tid);
  void sesc_exit(int32_t err);
  void sesc_finish(void);  /* Finish the whole simulation */
  void sesc_wait(void);

  void sesc_pseudoreset(void); /* Reset/Capture some stats for parallel
                                        applications to subtract spawning
                                        overheads                            */

  int32_t sesc_fetch_op(enum FetchOpType op, volatile int32_t *addr, int32_t val); 
  void sesc_unlock_op(volatile int32_t *addr, int32_t val);

  void sesc_simulation_mark(void);
  void sesc_simulation_mark_(void);
#if (defined SESCAPI_NATIVE) || (defined SUNSTUDIO)
  void sesc_simulation_mark_id(int32_t id);
  void sesc_simulation_mark_id_(int32_t id);
#else
  void sesc_simulation_mark_id(int32_t id) __attribute__((noinline));
  void sesc_simulation_mark_id_(int32_t id) __attribute__((noinline));
#endif
  void sesc_fast_sim_begin(void);
  void sesc_fast_sim_begin_(void);
  void sesc_fast_sim_end(void);
  void sesc_fast_sim_end_(void);

#ifndef SESCAPI_NATIVE
  /*
   * LOCK/UNLOCK operation
   * a simple spin lock
   */
  void sesc_lock_init(slock_t * lock);
  void sesc_lock(slock_t * lock);
  void sesc_unlock(slock_t * lock);
#endif

  /*
   * Barrier 
   * a two-phase centralized barrier
   */
  void sesc_barrier_init(sbarrier_t *barr);
  void sesc_barrier(sbarrier_t *barr, int32_t num_proc);

  /*
   * Semaphore
   * busy-wait semaphore
   */
  void sesc_sema_init(ssema_t *sema, int32_t initValue);
  void sesc_psema(ssema_t *sema);
  void sesc_vsema(ssema_t *sema);

  /*
   * Flag
   * using a lock and a semaphore
   */
  void sesc_flag_init(sflag_t *flag);
  void sesc_flag_wait(sflag_t *flag);
  void sesc_flag_set(sflag_t *flag);
  void sesc_flag_clear(sflag_t *flag);

#ifdef VALUEPRED
  int32_t  sesc_get_last_value(int32_t id);
  void sesc_put_last_value(int32_t id, int32_t val);
  int32_t  sesc_get_stride_value(int32_t id);
  void sesc_put_stride_value(int32_t id, int32_t val);
  int32_t  sesc_get_incr_value(int32_t id, int32_t lval);
  void sesc_put_incr_value(int32_t id, int32_t incr);
  void sesc_verify_value(int32_t rval, int32_t pval);
#endif

#ifdef TLS
  /* The thread begins the first epoch in its sequence */
  void sesc_begin_epochs(void);

  /* Creates and begins a new epoch that is the sequential successor of the
     current epoch. Return value is similar to fork - 0 is returned to the
     newly created epoch, the ID of the new epoch is returned to the original
     epoch
  */
  int32_t  sesc_future_epoch(void);

  /* Creates and begins a new epoch that is the sequential successor of the
     current epoch. The successor begins executing instructions from the
     instruction address codeAddr.
  */
  volatile void  sesc_future_epoch_jump(void *codeAddr);

  /* Completes an epoch that has already created its future.  The epoch wait
     until it can commit, then commits.  This call does not return.
  */
  void sesc_commit_epoch(void);

  /* Completes the current epoch and begins its sequential successor.  This
     could be done using sesc_future_epoch followed by csesc_commit_epoch, but
     sesc_change_epoch is more efficient
  */ 
  void sesc_change_epoch(void);

  /* The thread ends the last epoch in its sequence */
  void sesc_end_epochs(void);
  
#if ( (defined mips) && (__GNUC__ >= 3) )

  static inline void aspectReductionBegin(void) __attribute__ ((always_inline));
  static inline void aspectReductionEnd(void) __attribute__ ((always_inline));

  static inline void aspectReductionBegin(void){
    asm volatile (".word 0x70000000":::"memory");
  }
  static inline void aspectReductionEnd(void){
    asm volatile (".word 0x70000001":::"memory");
  }
  static inline void aspectAtomicBegin(void) __attribute__ ((always_inline));
  static inline void aspectAcquireBegin(void) __attribute__ ((always_inline));
  static inline void aspectAcquireRetry(void) __attribute__ ((always_inline, noreturn));
  static inline void aspectAcquireExit(void) __attribute__ ((always_inline));
  static inline void aspectAcquire2Release(void) __attribute__ ((always_inline));
  static inline void aspectReleaseBegin(void) __attribute__ ((always_inline));
  static inline void aspectReleaseEnter(void) __attribute__ ((always_inline));
  static inline void aspectAtomicEnd(void) __attribute__ ((always_inline));

  static inline void aspectAtomicBegin(void){
    asm volatile (".word 0x70000002":::"memory");
  }
  static inline void aspectAcquireBegin(void){
    asm volatile (".word 0x70000003":::"memory");
  }
  static inline void aspectAcquireRetry(void){
    asm volatile (".word 0x70000004":::"memory");
  }
  static inline void aspectAcquireExit(void){
    asm volatile (".word 0x70000005":::"memory");
  }
  static inline void aspectAcquire2Release(void){
    asm volatile (".word 0x70000006":::"memory");
  }
  static inline void aspectReleaseBegin(void){
    asm volatile (".word 0x70000007":::"memory");
  }
  static inline void aspectReleaseEnter(void){
    asm volatile (".word 0x70000008":::"memory");
  }
  static inline void aspectAtomicEnd(void){
    asm volatile (".word 0x70000009":::"memory");
  }

  static inline void tls_begin_epochs(void) __attribute__ ((always_inline));
  static inline void tls_change_epoch(void) __attribute__ ((always_inline));
  static inline void tls_end_epochs(void) __attribute__ ((always_inline));

  static inline void tls_lock_init(slock_t *lock_ptr) __attribute__ ((always_inline));
  static inline void tls_lock(slock_t *lock_ptr) __attribute__ ((always_inline));
  static inline void tls_unlock(slock_t *lock_ptr) __attribute__ ((always_inline));

  static inline void tls_barrier_init(sbarrier_t *barr_ptr) __attribute__ ((always_inline));
  static inline void tls_barrier(sbarrier_t *barr_ptr, int32_t num_proc) __attribute__ ((always_inline));

  static inline void tls_begin_epochs(void){
    asm volatile ("jal sesc_begin_epochs"
                  :
                  :
                  : "cc", "memory", "a0", "a1", "a2", "a3", "v0", "v1",
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9",
                  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "ra",
                  "$f0",  "$f1",  "$f2",  "$f3",  "$f4",  "$f5",  "$f6",  "$f7",
                  "$f8",  "$f9",  "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
                  "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
                  "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31",
                  "$fcc0", "$fcc1", "$fcc2", "$fcc3", "$fcc4", "$fcc5", "$fcc6", "$fcc7");
  }
  static inline void tls_change_epoch(void){
    asm volatile ("jal sesc_change_epoch"
                  :
                  : 
                  : "cc", "memory", "a0", "a1", "a2", "a3", "v0", "v1",
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9",
                  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "ra",
                  "$f0",  "$f1",  "$f2",  "$f3",  "$f4",  "$f5",  "$f6",  "$f7",
                  "$f8",  "$f9",  "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
                  "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
                  "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31",
                  "$fcc0", "$fcc1", "$fcc2", "$fcc3", "$fcc4", "$fcc5", "$fcc6", "$fcc7");
  }
  static inline void tls_end_epochs(void){
    asm volatile ("jal sesc_end_epochs"
                  :
                  : 
                  : "cc", "memory", "a0", "a1", "a2", "a3", "v0", "v1",
                  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9",
                  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "ra",
                  "$f0",  "$f1",  "$f2",  "$f3",  "$f4",  "$f5",  "$f6",  "$f7",
                  "$f8",  "$f9",  "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
                  "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
                  "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31",
                  "$fcc0", "$fcc1", "$fcc2", "$fcc3", "$fcc4", "$fcc5", "$fcc6", "$fcc7");
  }

  static inline void tls_lock_init(slock_t *lock_ptr){
#if !(defined ASPECT)
    lock_ptr->spin=UNLOCKED;
#endif
  }
  static inline void tls_lock(slock_t *lock_ptr){
#if (defined ASPECT)
    aspectAtomicBegin();
#else
    typedef volatile int32_t volint32_t;
    register volint32_t *spinptr=&(lock_ptr->spin);
    aspectAcquireBegin();
    if(*spinptr!=UNLOCKED)
      aspectAcquireRetry();
    aspectAcquireExit();
    *spinptr=LOCKED;
    aspectAtomicEnd();
#endif
  }
  static inline void tls_unlock(slock_t *lock_ptr){
#if (defined ASPECT)
    aspectAtomicEnd();
#else
    typedef volatile int32_t volint32_t;
    register volint32_t *spinptr=&(lock_ptr->spin);
    aspectReleaseBegin();
    *spinptr=UNLOCKED;
    aspectAtomicEnd();
#endif
  }
  static inline void tls_barrier_init(sbarrier_t *barr_ptr){
    barr_ptr->count=0;
    barr_ptr->gsense=0;
  }
  static inline void tls_barrier(sbarrier_t *barr_ptr, int32_t num_proc){
    typedef volatile int32_t volint32_t;
    register volint32_t *gsenseptr=&(barr_ptr->gsense);
    register int32_t  lsense=!(*gsenseptr);
    register volint32_t *countptr=&(barr_ptr->count);
    register int32_t lcount;
    aspectAtomicBegin();
    lcount=(*countptr)++;
    if(lcount==num_proc-1){
      barr_ptr->count=0;
      aspectReleaseEnter();
      (*gsenseptr)=lsense;
      aspectAtomicEnd();
    }else{
      aspectAtomicEnd();
      aspectAcquireBegin();
      if((*gsenseptr)!=lsense)
        aspectAcquireRetry();
      aspectAtomicEnd();
    }
  }
  static inline void tls_flag_init(sflag_t *flag_ptr){
    flag_ptr->flag=LOCKED;
  }
  static inline void tls_flag_clear(sflag_t *flag_ptr){
    flag_ptr->flag=LOCKED;
  }
  static inline void tls_flag_set(sflag_t *flag_ptr){
    typedef volatile int32_t volint32_t;
    register volint32_t *flagptr=&(flag_ptr->flag);
    aspectReleaseBegin();
    *(flagptr)=UNLOCKED;
    aspectAtomicEnd();
  }
  static inline void tls_flag_wait(sflag_t *flag_ptr){
    typedef volatile int32_t volint32_t;
    register volint32_t *flagptr=&(flag_ptr->flag);
    aspectAcquireBegin();
    if((*flagptr)!=UNLOCKED)
      aspectAcquireRetry();
    aspectAtomicEnd();
  }

#endif
#endif

#ifdef TASKSCALAR
  void sesc_begin_versioning(void);
  int32_t  sesc_fork_successor(void);
  int32_t  sesc_prof_fork_successor(int32_t id);
  void sesc_become_safe(void);
  void sesc_end_versioning(void);
  int32_t  sesc_is_safe(int32_t pid);
  int32_t  sesc_commit();
  int32_t  sesc_prof_commit(int32_t id);
#endif

#ifdef TS_CKPSUPORT
  int32_t  sesc_ckp();
  void sesc_commit_ckp();
  void sesc_restart_ckp();
#endif


#ifdef SESC_LOCKPROFILE
  void sesc_startlock();
  void sesc_endlock();
  void sesc_startlock2();
  void sesc_endlock2();
#endif

#ifdef VALUEPROF
  void sesc_delinquent_load_begin(int32_t id);
  void sesc_delinquent_load_end();
#endif

#ifdef __cplusplus
}
#endif

#endif                          /* SESCAPI_H */
