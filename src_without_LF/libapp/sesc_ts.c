#include <stdio.h>
#include <stdlib.h>
#include "sescapi.h"

// Note: TASKSCALAR only a part of the VERSIONMEM interface. Mainly,
// sesc_fork_successor and sesc_become_safe

void sesc_begin_versioning(void)
{
  // Do nothing
}

int sesc_fork_successor(void)
{
  fprintf(stderr,
	  "Versioning simulator is needed to execute sesc_fork_successor\n");
  return 0;
}

int sesc_prof_fork_successor(int id)
{
  fprintf(stderr,
	  "Versioning simulator is needed to execute sesc_prof_fork_successor\n");
  return 0;
}

void sesc_become_safe(void)
{
  fprintf(stderr,
	  "Versioning simulator is needed to execute sesc_become_safe\n");
}

void sesc_end_versioning(void) 
{
}

int sesc_prof_commit(int id)
{
  sesc_exit(0);
  return 1;
}

int sesc_commit() 
{
  sesc_exit(0);
  return 1;
}

int sesc_is_safe(int pid) 
{
  return 1;
}

void sesc_terminate(void)
{
  sesc_exit(0);
}


#ifdef SESC_LOCKPROFILE
void sesc_startlock() { }
void sesc_endlock() { }
void sesc_startlock2() { }
void sesc_endlock2() { }
#endif

