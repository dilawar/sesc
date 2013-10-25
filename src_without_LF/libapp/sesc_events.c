
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "sescapi.h"

#ifdef SESC_OSX_CHUD
void chud_boot();
void chud_begin();
void chud_end();
#endif

static void notifyEvent(const char *ev, int vaddr, int type, const void *sptr)
{
#ifdef DEBUG
  fprintf(stderr, "%s(0x%x,%d,0x%x) invoked\n", ev, (unsigned)vaddr, (int)type,
	  (unsigned)sptr);
#endif
}

/***********************************/
void sesc_preevent_(int vaddr, int type, void *sptr)
{
  sesc_preevent(vaddr, type, sptr);
}

void sesc_preevent(int vaddr, int type, void *sptr)
{
  notifyEvent("sesc_preevent", vaddr, type, sptr);
}

/***********************************/
void sesc_postevent_(int vaddr, int type, const void *sptr)
{
  sesc_postevent(vaddr, type, sptr);
}

void sesc_postevent(int vaddr, int type, const void *sptr)
{
  notifyEvent("sesc_postevent", vaddr, type, sptr);
}

/***********************************/
void sesc_memfence_(int vaddr)
{
    sesc_memfence(vaddr);
}

void sesc_memfence(int vaddr)
{
  notifyEvent("sesc_memfence", vaddr, 0, 0);
}

void sesc_acquire_(int vaddr)
{
  sesc_acquire(vaddr);
}

void sesc_acquire(int vaddr)
{
  notifyEvent("sesc_acquire", vaddr, 0, 0);
}

void sesc_release_(int vaddr)
{
    sesc_release(vaddr);
}

void sesc_release(int vaddr)
{
  notifyEvent("sesc_release", vaddr, 0, 0);
}

/***********************************/

void sesc_simulation_mark_()
{
  sesc_simulation_mark();
}

void sesc_tvsub(struct timeval *tdiff, struct timeval *t1, struct timeval *t0)
{
  tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
  tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
  if (tdiff->tv_usec < 0 && tdiff->tv_sec > 0) {
    tdiff->tv_sec--;
    tdiff->tv_usec += 1000000;
  }
  
  /* time shouldn't go backwards!!! */
  if (tdiff->tv_usec < 0 || t1->tv_sec < t0->tv_sec) {
    tdiff->tv_sec = 0;
    tdiff->tv_usec = 0;
  }
}


void sesc_simulation_mark()
{
  static int mark=0;
  static int mark1=0;
  static int mark2=0;
  static struct timeval tv1;

  if (mark == 0) {
    gettimeofday(&tv1, 0); // just in case that mark1 is not set

    if (getenv("SESC_1"))
      mark1 = atoi(getenv("SESC_1"));
   
    if (getenv("SESC_2"))
      mark2 = atoi(getenv("SESC_2"));
    else
      mark2 = -1;

#ifdef SESC_OSX_CHUD
    chud_boot();
    chud_begin();
#endif
  }

  if (mark == mark1) {
    gettimeofday(&tv1, 0);
#ifdef SESC_OSX_CHUD
    chud_begin();
#endif
  }

  fprintf(stderr,"sesc_simulation_mark %d (native) (sim from %d to %d mark)\n"
	  ,mark, mark1, mark2);

  if (mark == mark2) {
#ifdef SESC_OSX_CHUD
    chud_end();
#endif

    struct timeval tv2;
    gettimeofday(&tv2, 0);

    struct timeval td;
    long long usecs;
    
    sesc_tvsub(&td, &tv2, &tv1);
    usecs = td.tv_sec;
    usecs *= 1000000;
    usecs += td.tv_usec;
    
    fprintf(stderr,"sesc_simulation_mark FINISH %lld usesc\n", usecs);
    exit(0);
  }

  mark++;
}

void sesc_simulation_mark_id_(int id)
{
  sesc_simulation_mark_id(id);
}

void sesc_simulation_mark_id(int id)
{
  static int marks[256];
  static int first=1;
  int i;

  if(first) {
    for(i=0;i<256;i++)
      marks[i]=0;
    first=0;
  }

  fprintf(stderr,"sesc_simulation_mark(%d) %d (native)", id, marks[id%256]);
  marks[id%256]++;
}

void sesc_finish()
{
  fprintf(stderr,"sesc_finish called (end of simulation in native)");
  exit(0);
}

void sesc_fast_sim_begin_()
{
  sesc_fast_sim_begin();
}

void sesc_fast_sim_begin()
{
  notifyEvent("sesc_fast_sim_begin", 0, 0, 0);

#ifdef SIMICS
  SIMICS_SESC_END();  
  fprintf(stderr, "simicsapi:sesc_end\n");
#endif
}

void sesc_fast_sim_end_()
{
  sesc_fast_sim_end();
}

void sesc_fast_sim_end()
{
  notifyEvent("sesc_fast_sim_end", 0, 0, 0);

#ifdef SIMICS
  SIMICS_SESC_BEGIN();
  fprintf(stderr, "simicsapi:sesc_begin\n");
#endif
}

void sesc_sysconf(int tid, int flags)
{
  notifyEvent("sesc_sysconf", 0, 0, 0);
}

/*************************************/

#ifdef VALUEPRED
int  sesc_get_last_value(int id)
{
  return 0;
}

void sesc_put_last_value(int id, int val)
{
}

int  sesc_get_stride_value(int id)
{
  return 0;
}

void sesc_put_stride_value(int id, int val)
{
}

int  sesc_get_incr_value(int id, int lval)
{
  return 0;
}

void sesc_put_incr_value(int id, int incr)
{
}

void sesc_verify_value(int rval, int pval)
{
}
#endif

