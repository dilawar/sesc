#ifndef SIMICSAPI_H
#define SIMICSAPI_H

#define SIMICS_MAGICW_SYNC   0x00c0ffee

#define SIMICS_MAGICW_BEGIN_LOCK 0x0000b10c
#define SIMICS_MAGICW_LOCKED     0x00010ced
#define SIMICS_MAGICW_UNLOCK     0x0000010c

#define SIMICS_MAGICW_ENTER_BARRIER 0x00000eba
#define SIMICS_MAGICW_LEAVE_BARRIER 0x000001ba

#define __SIMICS_MAGIC_CASSERT(p) do {                \
    typedef int32_t __check_magic_argument[(p) ? 1 : -1]; \
  } while (0)

#define SIMICS_MAGIC(n) do {			       \
    __SIMICS_MAGIC_CASSERT(!(n));		       \
    __asm__ __volatile__ ("xchg %bx,%bx");	       \
  } while (0)

#define SIMICS_MAGIC_PARM(n) do {					\
    __asm__ __volatile__ ("movl %0, %%eax" :: "g" (n) : "eax");		\
    SIMICS_MAGIC(0);							\
  } while (0)

#define SIMICS_BEGIN_LOCK(_lock_addr) do {	       \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_SYNC);	       \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_BEGIN_LOCK);       \
    SIMICS_MAGIC_PARM(_lock_addr);		       \
  } while (0) 

#define SIMICS_LOCKED(_lock_addr) do {		  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_SYNC);	  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_LOCKED);	  \
    SIMICS_MAGIC_PARM(_lock_addr);		  \
  } while (0)

#define SIMICS_UNLOCK(_lock_addr) do {			\
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_SYNC);		\
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_UNLOCK);		\
    SIMICS_MAGIC_PARM(_lock_addr);			\
  } while (0)

#define SIMICS_ENTER_BARRIER(_barrier_addr, _nproc)  do {	  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_SYNC);			  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_ENTER_BARRIER);		  \
    SIMICS_MAGIC_PARM(_barrier_addr);				  \
    SIMICS_MAGIC_PARM(_nproc);					  \
  } while(0) 

#define SIMICS_LEAVE_BARRIER(_barrier_addr)  do {		  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_SYNC);			  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_LEAVE_BARRIER);		  \
    SIMICS_MAGIC_PARM(_barrier_addr);				  \
  } while(0) 

#define SIMICS_MAGICW_SESCBEGIN 0x5e5c000b
#define SIMICS_MAGICW_SESCEND   0x5e5c000e

#define SIMICS_SESC_BEGIN() do {		  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_SESCBEGIN);	  \
  } while (0)

#define SIMICS_SESC_END() do {			  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_SESCEND);	  \
  } while (0)

#define SIMICS_MAGICW_START_IGNORE  0x0000b0b0
#define SIMICS_MAGICW_STOP_IGNORE   0x0000b1b1

#define SIMICS_START_IGNORE() do {			  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_START_IGNORE);	  \
  } while (0)

#define SIMICS_STOP_IGNORE() do {			  \
    SIMICS_MAGIC_PARM(SIMICS_MAGICW_STOP_IGNORE);	  \
  } while (0)

#endif
