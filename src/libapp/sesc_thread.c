#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>

#include "sescapi.h"

#if ((defined(_MIPS_ISA) && (_MIPS_ISA == _MIPS_ISA_MIPS2)) || (defined XCC_MIPSEB) || (defined XCC_SPARC))
/* compiled with -32 */

void sesc_init(void)
{
}

int sesc_get_num_cpus(void)
{
  fprintf(stderr,"sesc_get_num_cpus shouldn't be called without a simulator. This is a simulator only code\n");
  exit(-1);
  return 1;
}

int sesc_spawn(void (*start_routine) (void *), void *arg, int flags)
{
  fprintf(stderr,"sesc_spawn shouldn't be called without a simulator. This is a simulator only code\n");
  exit(-1);
}


int  sesc_self(void){
  fprintf(stderr,"sesc_self shouldn't be called without a simulator. This is a simulator only code\n");
  exit(-1);
  return 0;
}

int  sesc_suspend(int tid){
  fprintf(stderr,"sesc_suspend shouldn't be called without a simulator\n");
  exit(-1);
  return 0;
}

int  sesc_resume(int tid){
  fprintf(stderr,"sesc_resume shouldn't be called without a simulator\n");
  exit(-1);
  return 0;
}

int  sesc_yield(int tid){
  fprintf(stderr,"sesc_yield shouldn't be called without a simulator\n");
  exit(-1);
  return 0;
}

void sesc_exit(int err){
  fprintf(stderr,"sesc_exit shouldn't be called without a simulator\n");
  exit(-1);
}


void sesc_wait(void){
  fprintf(stderr,"sesc_wait shouldn't be called without a simulator\n");
  exit(-1);
}

void sesc_pseudoreset(void){
}

int sesc_fetch_op(enum FetchOpType op, volatile int *addr, int val){
  fprintf(stderr,"sesc_fetch_op shouldn't be called without a simulator\n");
  exit(-1);
  return 0l;
}

void sesc_unlock_op(volatile int *addr, int val)
{
  fprintf(stderr,"sesc_unlock_op shouldn't be called without a simulator\n");
  exit(-1);
}

#else
/* NOT compiled with -32 */

#include <pthread.h>
/* hash Table lock */
static pthread_mutex_t hash_lock    = PTHREAD_MUTEX_INITIALIZER;

/* addWaiting && awake && nConcurrentThreads lock */
static pthread_mutex_t wait_lock    = PTHREAD_MUTEX_INITIALIZER;

/* sesc_fetch_op lock */
static pthread_mutex_t fetchop_lock = PTHREAD_MUTEX_INITIALIZER;

static volatile int nConcurrentThreads=1;

/* sescapi mapping between native pid (nPid) to internal pid (iPid)
 */

struct TableField 
{
  char inUse;
  char doWait;
  char doKill;
  signed short nWaiting;
  int  nPid;
  int  iPid;
  pthread_mutex_t locked;

  /* Spawn parameters */
  void (*routine) (void *);
  void *arg;
  pthread_t thr;
  int ppid;
};

#define SESC_MAXTHREADS (2*MAXLOCKWAITING)
#define SESC_TABLESIZE  (2*SESC_MAXTHREADS-1)

static struct TableField table[SESC_TABLESIZE];

#define TABLE_HASHFUNC(x)  ((x < 0 ? (-1*x) : x) % SESC_TABLESIZE)

static struct TableField *TableFindEntry(int nPid) 
{
  int pos = TABLE_HASHFUNC(nPid);
  assert(pos > 0);
  int i;

  for(i=0;i<SESC_TABLESIZE;i++ ) {
    if( table[pos].nPid == nPid && table[pos].inUse ) {
      return &table[pos];
    }
    pos= (pos +1) % SESC_TABLESIZE;
  }

  return 0;  // Not found?
}

static struct TableField *TableNewEntry(int nPid)
{
  int pos = TABLE_HASHFUNC(nPid);
  assert(pos > 0);

  // assert(nConcurrentThreads < SESC_TABLESIZE);

  pthread_mutex_lock(&hash_lock);
  
  while( table[pos].inUse ) {
    pos= (pos +1) % SESC_TABLESIZE;
  }
  
  table[pos].inUse    = 1;
  table[pos].doWait   = 0;
  table[pos].doKill   = 0;
  table[pos].nWaiting = 0;
  table[pos].nPid     = nPid;
  table[pos].iPid     = pos; // Stupid but easier to understand

  pthread_mutex_init(&table[pos].locked,0);
  pthread_mutex_lock(&table[pos].locked); // Ready to lock

  pthread_mutex_unlock(&hash_lock);

  return &table[pos];
}

static void TableFreeEntry(struct TableField *tf)
{
  pthread_mutex_lock(&hash_lock);

  tf->inUse = 0;

  pthread_mutex_destroy(&tf->locked);
  
  pthread_mutex_unlock(&hash_lock);
}

int rootPid;

#ifdef SESCAPI_NATIVE_IRIX
atomic_reservoir_t atomic_reservoir;
#endif
void sesc_init(void)
{
  int i;
  
  for(i=0;i<SESC_TABLESIZE;i++) {
    table[i].inUse = 0;
  }

  rootPid = pthread_self();
  TableNewEntry(rootPid);

#ifdef SESCAPI_NATIVE_IRIX
  atomic_reservoir = atomic_alloc_reservoir(USE_DEFAULT_PM,10, NULL);
#endif
}

void addWaiting(int pid)
{
  struct TableField *tf = TableFindEntry(pid);
  
  pthread_mutex_lock(&wait_lock);

  tf->nWaiting++;
  if( pid == (int)pthread_self() ) {
    int doWait = (tf->nWaiting > 0) && (nConcurrentThreads > 1);
    tf->doWait = doWait;
  
    pthread_mutex_unlock(&wait_lock);
    
    if( doWait ) {
      pthread_mutex_lock(&tf->locked);
    }
    
  }else{
    tf->doKill = (tf->nWaiting > 0) && (nConcurrentThreads > 1);
    if( tf->doKill > 0 )
      pthread_kill(pid,SIGSTOP);

    pthread_mutex_unlock(&wait_lock);
  }
}

int sesc_get_num_cpus(void){
  return 1;
}

int awakeWaiting(int ppid)
{
  struct TableField *tf = TableFindEntry(ppid);

  /* Already died check */
  if( tf == 0 )
    return 0;
  
  pthread_mutex_lock(&wait_lock);

  tf->nWaiting--;

  if( tf->doWait ) {
    assert(tf->nWaiting == 0);
    
    pthread_mutex_unlock(&tf->locked);
    tf->doWait = 0;
    assert(!tf->doKill);
  }else if( tf->doKill ) {
    assert(tf->nWaiting == 0);

    pthread_kill(ppid,SIGCONT);
    
    tf->doKill = 0;
  }
  

  pthread_mutex_unlock(&wait_lock);

  return 1;
}

void *sesc_spawn_wrapper(void *p)
{
  struct TableField *tempArg = (struct TableField *)p;
  struct TableField *tf;

  pthread_mutex_lock(&wait_lock);

  if( TableFindEntry(pthread_self()) == 0 ) {
    tf = TableNewEntry(pthread_self());

    tf->routine = tempArg->routine;
    tf->arg     = tempArg->arg;
    tf->ppid    = tempArg->ppid;
  }else{
    tf = TableFindEntry(pthread_self());
  }
  
  pthread_mutex_unlock(&wait_lock);
  
  /* Spawning thread can continue: tempArg can not be used because it
   * pointed to the stack of its parent.
   */

  TableFreeEntry((struct TableField *)tempArg);

  tf->routine(tf->arg);

  pthread_mutex_lock(&wait_lock);
  nConcurrentThreads--;
  pthread_mutex_unlock(&wait_lock);

  awakeWaiting(tf->ppid);

  TableFreeEntry(tf);
  
  sesc_exit(0);

  return 0;
}


int sesc_spawn(void (*routine) (void *), void *arg, int flags)
{
  struct TableField *tf;

  /* Find free entry */
  int fakePid= sesc_self()+12321022;

  if( routine == 0 ) {
    fprintf(stderr,"sesc_spawn fork like functionality not supported in native\n");
    exit(0);
  }

  do{
    fakePid++;
    tf = TableFindEntry(fakePid);
  }while(tf);
  
  tf = TableNewEntry(fakePid);
  assert(tf);

  tf->routine = routine;
  tf->arg     = arg;
  tf->ppid    = pthread_self();

  pthread_mutex_lock(&wait_lock);
  nConcurrentThreads++;
  pthread_mutex_unlock(&wait_lock);

  pthread_create( &tf->thr, 0, sesc_spawn_wrapper, tf);

  pthread_mutex_lock(&wait_lock);

  if( TableFindEntry((int)tf->thr) == 0 ) {
    struct TableField *rtf;
  
    rtf = TableNewEntry((int)tf->thr);
    rtf->routine = tf->routine;
    rtf->arg     = tf->arg;
    rtf->ppid    = tf->ppid;
  }
  
  pthread_mutex_unlock(&wait_lock);

  return (int)tf->thr;
}

int sesc_self(void)
{
  return pthread_self();
}

int sesc_suspend(int pid)
{
  addWaiting(pid);

  return 1;
}

int sesc_resume(int tid)
{
  awakeWaiting(tid);
  
  return 1;
}

int sesc_yield(int tid)
{
  /* In native the OS scheduler does it automaticaly for you */
  return 1;
}

void sesc_wait(void)
{
  addWaiting(pthread_self());
}

void sesc_pseudoreset(void)
{
  
}

void sesc_exit(int err)
{
  if( rootPid == (int)pthread_self() ) {
#ifdef SESCAPI_NATIVE_IRIX
    atomic_free_reservoir(atomic_reservoir);
#endif
    fprintf(stderr,"Main exiting...\n");
    TableFreeEntry(TableFindEntry(rootPid));
  }
  
  pthread_exit(0);
}

int sesc_fetch_op(enum FetchOpType op, volatile int *data, int val){
  assert(data);
  pthread_mutex_lock(&fetchop_lock);
  {
	 int odata;
    odata = *data;
    
    switch (op) {
    case FetchIncOp:
      (*data)++;
      break;
    case FetchDecOp:
      (*data)--;
      break;
    case FetchSwapOp:
      *data = val;
      break;
    default:
      fprintf(stderr, "sesc_fetch_op(%d,0x%p,%ld) has invalid Op\n", op, data, val);     
      exit(-1);
    };
    pthread_mutex_unlock(&fetchop_lock);
    return odata;
  }
}

void sesc_unlock_op(volatile int *addr, int val)
{
  *addr = val;
}
#endif

