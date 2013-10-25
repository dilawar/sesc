#if !(defined MINTAPI_H)
#define MINTAPI_H

#include "Snippets.h"
#include "icode.h"

// Returns the number of CPUs the system has
int32_t rsesc_get_num_cpus(void);
ThreadContext*rsesc_get_thread_context(Pid_t pid);

void rsesc_spawn(Pid_t ppid, Pid_t cpid, int32_t flags);

int32_t  rsesc_exit(int32_t pid, int32_t err);
void rsesc_simulation_mark(int32_t pid);
void rsesc_simulation_mark_id(int32_t pid,int32_t id);
void rsesc_fast_sim_begin(int32_t pid);
void rsesc_fast_sim_end(int32_t pid);
int32_t  rsesc_suspend(int32_t pid, int32_t tid);
int32_t  rsesc_resume(int32_t pid, int32_t tid);
int32_t  rsesc_yield(int32_t pid, int32_t tid);
void rsesc_preevent(int32_t pid, int32_t vaddr, int32_t type, void *sptr);
void rsesc_postevent(int32_t pid, int32_t vaddr, int32_t type, void *sptr);
void rsesc_memfence(int32_t pid, int32_t vaddr);
void rsesc_acquire(int32_t pid , int32_t vaddr);
void rsesc_release(int32_t pid , int32_t vaddr);

#if (defined TLS)
icode_ptr rsesc_get_instruction_pointer(int32_t pid);
void rsesc_set_instruction_pointer(int32_t pid, icode_ptr picode);
int32_t rsesc_replay_system_call(int32_t pid);
void rsesc_record_system_call(int32_t pid, uint32_t retVal);

void rsesc_begin_epochs(int32_t pid);
void rsesc_future_epoch(int32_t pid);
void rsesc_future_epoch_jump(int32_t pid, icode_ptr jump_icode);
void rsesc_acquire_begin(int32_t pid);
void rsesc_acquire_retry(int32_t pid);
void rsesc_acquire_end(int32_t pid);
void rsesc_release_begin(int32_t pid);
void rsesc_release_end(int32_t pid);
void rsesc_commit_epoch(int32_t pid);
void rsesc_change_epoch(int32_t pid);
void rsesc_end_epochs(int32_t pid);

int32_t  rsesc_syscall_replay(int32_t pid);
void rsesc_syscall_new(int32_t pid);
void rsesc_syscall_retval(int32_t pid, int32_t value);
void rsesc_syscall_errno(int32_t pid, int32_t value);
void rsesc_syscall_perform(int32_t pid);
#endif

#ifdef TASKSCALAR
void  rsesc_exception(int32_t pid);
void  rsesc_spawn(int32_t ppid, int32_t pid, int32_t flags);
int32_t   rsesc_exit(int32_t cpid, int32_t err);
int32_t   rsesc_version_can_exit(int32_t pid);
void  rsesc_prof_commit(int32_t pid, int32_t tid);
void  rsesc_fork_successor(int32_t ppid, int32_t where, int32_t tid);
void  rsesc_become_safe(int32_t pid);
bool  rsesc_is_safe(int32_t pid);
void *rsesc_OS_prewrite(int32_t pid , int32_t iAddr, VAddr vaddr, int32_t flags);
void  rsesc_OS_postwrite(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags);
int32_t   rsesc_is_versioned(int32_t pid);
#endif

#if (defined TLS) || (defined TASKSCALAR)
void *rsesc_OS_read(int32_t pid, int32_t iAddr, VAddr vaddr, int32_t flags);
// Reads a string from simulated virtual address srcStart into simulator's address dstStart,
// copying at most maxSize-1 characters bytes in the process and null-terminating the string.
// Returns true if the entire string fits in dstStart, false if string was truncated to fit
bool rsesc_OS_read_string(int32_t pid, int32_t iAddr  , RAddr rdstStart, VAddr srcStart, size_t maxSize);
bool rsesc_OS_read_string(int32_t pid, VAddr iAddr, void  *dstStart, VAddr srcStart, size_t size);
// Reads a block from simulated virtual address srcStart into simulator's address dstStart
void rsesc_OS_read_block(int32_t pid, int32_t iAddr, RAddr rdstStart, VAddr srcStart, size_t size);
void rsesc_OS_read_block(int32_t pid, int32_t iAddr, void  *dstStart, VAddr srcStart, size_t size);

// Writes a block to the simulated virtual address dstStart from simulator's address srcStart
void rsesc_OS_write_block(int32_t pid, int32_t iAddr, VAddr dstStart, RAddr rsrcStart     , size_t size);
void rsesc_OS_write_block(int32_t pid, int32_t iAddr, VAddr dstStart, const void *srcStart, size_t size);
#endif

icode_ptr mint_exit(icode_ptr picode, thread_ptr pthread);

// Functions exported by mint for use in sesc

int32_t mint_sesc_create_clone(thread_ptr pthread);
void mint_sesc_die(thread_ptr pthread);

#endif /* !(defined MINTAPI_H) */
