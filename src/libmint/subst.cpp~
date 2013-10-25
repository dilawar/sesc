/*
 * Includes all the functions that are substituted for their
 * counterparts in the traced object file.
 *
 * Copyright (C) 1993 by Jack E. Veenstra (veenstra@cs.rochester.edu)
 * 
 * This file is part of MINT, a MIPS code interpreter and event generator
 * for parallel programs.
 * 
 * MINT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * MINT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MINT; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#ifdef CYGWIN
#include <sys/dirent.h>
#else
#if (defined LINUX) && (defined __i386)
#include <ulimit.h>
#include <asm/types.h>
#include <asm/posix_types.h>
#include <linux/dirent.h>
#else
#include <ulimit.h>
#include <dirent.h>
#endif
#endif

/* #define DEBUG_VERBOSE 1 */

#if (defined DARWIN) || (defined CYGWIN)
typedef off_t off64_t;
#else
#include <stropts.h>
#endif

#if (defined LINUX) && (defined __i386)
#include <sys/syscall.h>
#include <linux/types.h>
#include <linux/unistd.h>
typedef __off64_t off64_t;

//#ifdef SYS_getdents
#define getdents(fd,dirp,count) syscall(SYS_getdents,fd,dirp,count)
//#else
//static _syscall3(int32_t, getdents, uint32_t, fd, struct dirent *, dirp, uint32_t, count);
//int32_t getdents(uint32_t fd, struct dirent *dirp, uint32_t count);
//#endif

#endif

#include "alloca.h"
#include "icode.h"
#include "ThreadContext.h"
#include "globals.h"
#include "opcodes.h"
#include "symtab.h"
#include "mendian.h"
#include "prctl.h"
#include "mintapi.h"
#include "OSSim.h"

#if (defined TLS)
#include "Epoch.h"
#endif

struct glibc_stat64 {
  targULong     st_dev;
  targLong      st_pad0[3]; /* Reserved for st_dev expansion  */
  targULongLong st_ino;

  targUInt      st_mode;
  targInt       st_nlink;

  targInt       st_uid;
  targInt       st_gid;

  targULong     st_rdev;
  targLong      st_pad1[3];     /* Reserved for st_rdev expansion  */

  targLongLong  st_size;

  /*
   * Actually this should be timestruc_t st_atime, st_mtime and st_ctime
   * but we don't have it under Linux.
   */
  targInt       st_atim;
  targLong      reserved0;      /* Reserved for st_atime expansion  */

  targInt       st_mtim;
  targLong      reserved1;      /* Reserved for st_mtime expansion  */

  targInt       st_ctim;
  targLong      reserved2;      /* Reserved for st_ctime expansion  */

  targUInt      st_blksize;
  targLong      st_pad2;

  targLongLong  st_blocks;
  targLong      st_pad3[14];
};

/* defines to survive the 64 bits mix */
struct glibc_stat32 {
  targUInt      st_dev;
  targInt       st_pad1[3];             /* Reserved for network id */
  targUInt      st_ino;
  targUInt      st_mode;
  targInt       st_nlink;
  targInt       st_uid;
  targInt       st_gid;
  targULong     st_rdev;
  targLong      st_pad2[2];
  targInt       st_size;
  targLong      st_pad3;
  /*
   * Actually this should be timestruc_t st_atime, st_mtime and st_ctime
   * but we don't have it under Linux.
   */
  targInt       st_atim;
  targLong      reserved0;
  targInt       st_mtim;
  targLong      reserved1;
  targInt       st_ctim;
  targLong      reserved2;
  targInt       st_blksize;
  targInt       st_blocks;
  targLong      st_pad4[14];
};

void conv_stat32_native2glibc(const struct stat *statp, struct glibc_stat32 *stat32p)
{
  stat32p->st_dev   = SWAP_WORD(statp->st_dev);
  stat32p->st_ino   = SWAP_WORD(statp->st_ino); 
  stat32p->st_mode  = SWAP_WORD(statp->st_mode);
  stat32p->st_nlink = SWAP_WORD(statp->st_nlink);
  stat32p->st_uid   = SWAP_WORD(statp->st_uid);
  stat32p->st_gid   = SWAP_WORD(statp->st_gid);
  stat32p->st_rdev  = SWAP_WORD(statp->st_rdev); 
  stat32p->st_size  = SWAP_WORD(statp->st_size); 

  stat32p->st_atim = SWAP_WORD(statp->st_atime);
  stat32p->st_mtim = SWAP_WORD(statp->st_mtime);
  stat32p->st_ctim = SWAP_WORD(statp->st_ctime);

  stat32p->st_blksize = SWAP_WORD(statp->st_blksize);
  stat32p->st_blocks  = SWAP_WORD(statp->st_blocks);
}

void conv_stat64_native2glibc(const struct stat *statp, struct glibc_stat64 *stat64p)
{
  stat64p->st_dev   = SWAP_WORD(statp->st_dev);
  stat64p->st_ino   = SWAP_LONG((unsigned long long)statp->st_ino); 
  stat64p->st_mode  = SWAP_WORD(statp->st_mode);
  stat64p->st_nlink = SWAP_WORD(statp->st_nlink);
  stat64p->st_uid   = SWAP_WORD(statp->st_uid);
  stat64p->st_gid   = SWAP_WORD(statp->st_gid);
  stat64p->st_rdev  = SWAP_WORD(statp->st_rdev); 
  stat64p->st_size  = SWAP_LONG((unsigned long long)statp->st_size); 

  stat64p->st_atim = SWAP_WORD(statp->st_atime);
  stat64p->st_mtim = SWAP_WORD(statp->st_mtime);
  stat64p->st_ctim = SWAP_WORD(statp->st_ctime);

  stat64p->st_blksize = SWAP_WORD(statp->st_blksize);
  stat64p->st_blocks  = SWAP_LONG((unsigned long long)statp->st_blocks);
}

#if (defined TASKSCALAR) || (defined TLS)
/* Exit icode, used from SESC with Taskscalar to force a thread to terminate */
icode_t TerminatePicode;
#endif
/* Invalid icode, used as the "next icode" for contexts without any thread to execute */ 
icode_t invalidIcode;
icode_ptr Idone1;               /* calls terminator1() */
static icode_t Iterminator1;

extern int32_t errno;

/* substitute functions */
OP(mint_ulimit);
OP(mint_sysmp);
OP(mint_getdents);
OP(mint_readv);OP(mint_writev);
OP(mint_execl); OP(mint_execle); OP(mint_execlp);
OP(mint_execv); OP(mint_execve); OP(mint_execvp);
OP(mint_mmap); OP(mint_munmap);
OP(mint_open); OP(mint_close); OP(mint_read); OP(mint_write);
OP(mint_creat); OP(mint_link); OP(mint_unlink); OP(mint_chdir);
OP(mint_rename);
OP(mint_chmod); OP(mint_fchmod); OP(mint_chown); OP(mint_fchown);
OP(mint_lseek); OP(mint_lseek64); OP(mint_access);
OP(mint_stat); OP(mint_lstat); OP(mint_fstat); 
OP(mint_stat64); OP(mint_lstat64); OP(mint_fstat64); 
OP(mint_fxstat64); 
OP(mint_dup); OP(mint_pipe); OP(mint_xstat); OP(mint_xstat64);
OP(mint_symlink); OP(mint_readlink); OP(mint_umask);
OP(mint_getuid); OP(mint_geteuid); OP(mint_getgid); OP(mint_getegid);
OP(mint_getdomainname); OP(mint_setdomainname);
OP(mint_gethostname); OP(mint_socket); OP(mint_connect);
OP(mint_send); OP(mint_sendto); OP(mint_sendmsg);
OP(mint_recv); OP(mint_recvfrom); OP(mint_recvmsg);
OP(mint_getsockopt); OP(mint_setsockopt);
OP(mint_select);
OP(mint_oserror); OP(mint_setoserror); OP(mint_perror);
OP(mint_times); OP(mint_getdtablesize);
OP(mint_sigprocmask);
OP(mint_syssgi); OP(mint_time);
OP(mint_prctl);

OP(mint_malloc); OP(mint_calloc); OP(mint_free); OP(mint_realloc);
OP(mint_usinit); OP(mint_usconfig); OP(mint_usdetach);

OP(mint_getpid); OP(mint_getppid); 
OP(mint_clock);
OP(mint_gettimeofday);
OP(mint_isatty); OP(mint_ioctl); OP(mint_fcntl);
OP(mint_cerror); OP(mint_null_func);
OP(return1); OP(mint_exit); 

OP(mint_finish); 
OP(mint_printf);        /* not used */
OP(mint_sesc_get_num_cpus);
#ifdef TLS
//OP(mint_sesc_begin_epochs);
OP(mint_sesc_future_epoch);
OP(mint_sesc_future_epoch_jump);
OP(mint_sesc_commit_epoch);
OP(mint_sesc_change_epoch);
//OP(mint_sesc_end_epochs);
/* OP(mint_sesc_barrier_init); */
/* OP(mint_sesc_barrier); */
/* OP(mint_sesc_lock_init); */
/* OP(mint_sesc_lock); */
/* OP(mint_sesc_unlock); */
/* OP(mint_sesc_flag_init); */
/* OP(mint_sesc_flag_set); */
/* OP(mint_sesc_flag_trywait); */
/* OP(mint_sesc_flag_wait); */
/* OP(mint_sesc_flag_clear); */
#endif
OP(mint_do_nothing);
#ifdef VALUEPRED
OP(mint_sesc_get_last_value);
OP(mint_sesc_put_last_value);
OP(mint_sesc_get_stride_value);
OP(mint_sesc_put_stride_value);
OP(mint_sesc_get_incr_value);
OP(mint_sesc_put_incr_value);
OP(mint_sesc_verify_value);
#endif
#ifdef TASKSCALAR
OP(mint_rexit);
OP(mint_sesc_become_safe);
OP(mint_sesc_is_safe);
OP(mint_sesc_prof_commit);
OP(mint_sesc_prof_fork_successor);
#endif


#ifdef SESC_LOCKPROFILE
OP(mint_sesc_startlock);
OP(mint_sesc_endlock);
OP(mint_sesc_startlock2);
OP(mint_sesc_endlock2);
#endif

OP(mint_uname);
OP(mint_getrlimit);
OP(mint_getrusage);
OP(mint_getcwd);
OP(mint_notimplemented);
OP(mint_assert_fail);
OP(mint_rmdir); 

/* Calls to these functions execute a Mint32_t function instead of the
 * native function.
 */

/* NOTE:
 *
 * It is NOT necessary to add underscores (_+) or libc_ at the
 * beginning of the function name. To avoid replication, noun_strcmp
 * eats all the underscores and libc_ that it finds.
 *
 * Ex: mmap would match for mmap, _mmap, __mmap, __libc_mmap ....
 *
 */

func_desc_t Func_subst[] = {
  // char *name               PFPI func;
  {"ulimit",                  mint_ulimit,                     1, OpExposed},
  {"execl",                   mint_execl,                      1, OpExposed},
  {"execle",                  mint_execle,                     1, OpExposed},
  {"execlp",                  mint_execlp,                     1, OpExposed},
  {"execv",                   mint_execv,                      1, OpExposed},
  {"execve",                  mint_execve,                     1, OpExposed},
  {"execvp",                  mint_execvp,                     1, OpExposed},
  {"mmap",                    mint_mmap,                       1, OpUndoable},
  {"open64",                  mint_open,                       1, OpUndoable}, 
  {"open",                    mint_open,                       1, OpUndoable},
  {"close",                   mint_close,                      1, OpUndoable},
  {"read",                    mint_read,                       1, OpUndoable},
  {"write",                   mint_write,                      1, OpUndoable},
  {"readv",                   mint_readv,                      1, OpExposed},
  {"writev",                  mint_writev,                     1, OpExposed},
  {"creat",                   mint_creat,                      1, OpExposed},
  {"link",                    mint_link,                       1, OpExposed},
  {"unlink",                  mint_unlink,                     1, OpExposed},
  {"rename",                  mint_rename,                     1, OpExposed},
  {"chdir",                   mint_chdir,                      1, OpExposed},
  {"chmod",                   mint_chmod,                      1, OpExposed},
  {"fchmod",                  mint_fchmod,                     1, OpExposed},
  {"chown",                   mint_chown,                      1, OpExposed},
  {"fchown",                  mint_fchown,                     1, OpExposed},
  {"lseek64",                 mint_lseek64,                    1, OpExposed},
  {"lseek",                   mint_lseek,                      1, OpExposed},
  {"access",                  mint_access,                     1, OpInternal},
  {"stat64",                  mint_stat64,                     1, OpExposed},
  {"lstat64",                 mint_lstat64,                    1, OpExposed},
  {"fstat64",                 mint_fstat64,                    1, OpExposed},
  {"xstat64",                 mint_xstat64,                    1, OpExposed},
  {"fxstat64",                mint_fxstat64,                   1, OpInternal},
  {"stat",                    mint_stat,                       1, OpExposed},
  {"lstat",                   mint_lstat,                      1, OpExposed},
  {"fstat",                   mint_fstat,                      1, OpExposed},
  {"xstat",                   mint_xstat,                      1, OpExposed},
  {"dup",                     mint_dup,                        1, OpExposed},
  {"pipe",                    mint_pipe,                       1, OpExposed},
  {"symlink",                 mint_symlink,                    1, OpExposed},
  {"readlink",                mint_readlink,                   1, OpExposed},
  {"umask",                   mint_umask,                      1, OpExposed},
  {"getuid",                  mint_getuid,                     0, OpInternal},
  {"geteuid",                 mint_geteuid,                    0, OpInternal},
  {"getgid",                  mint_getgid,                     0, OpInternal},
  {"getegid",                 mint_getegid,                    0, OpInternal},
  {"gethostname",             mint_gethostname,                1, OpExposed},
  {"getdomainname",           mint_getdomainname,              1, OpExposed},
  {"setdomainname",           mint_setdomainname,              1, OpExposed},
  {"socket",                  mint_socket,                     1, OpExposed},
  {"connect",                 mint_connect,                    1, OpExposed},
  {"send",                    mint_send,                       1, OpExposed},
  {"sendto",                  mint_sendto,                     1, OpExposed},
  {"sendmsg",                 mint_sendmsg,                    1, OpExposed},
  {"recv",                    mint_recv,                       1, OpExposed},
  {"recvfrom",                mint_recvfrom,                   1, OpExposed},
  {"recvmsg",                 mint_recvmsg,                    1, OpExposed},
  {"getsockopt",              mint_getsockopt,                 1, OpExposed},
  {"setsockopt",              mint_setsockopt,                 1, OpExposed},
  {"select",                  mint_select,                     1, OpExposed},
  {"cachectl",                mint_null_func,                  1, OpExposed},
  {"oserror",                 mint_oserror,                    0, OpExposed},
  {"setoserror",              mint_setoserror,                 0, OpExposed},
  {"perror",                  mint_perror,                     1, OpExposed},
  {"times",                   mint_times,                      1, OpUndoable},
  {"getdtablesize",           mint_getdtablesize,              1, OpExposed},
  {"syssgi",                  mint_syssgi,                     1, OpExposed},
  {"time",                    mint_time,                       1, OpExposed},
  {"munmap",                  mint_munmap,                     1, OpUndoable},
  {"malloc",                  mint_malloc,                     1, OpUndoable},
  {"calloc",                  mint_calloc,                     1, OpUndoable},
  {"free",                    mint_free,                       1, OpUndoable},
  {"cfree",                   mint_free,                       1, OpUndoable},
  {"realloc",                 mint_realloc,                    1, OpExposed},
  {"getpid",                  mint_getpid,                     1, OpInternal},
  {"getppid",                 mint_getppid,                    1, OpExposed},
  {"clock",                   mint_clock,                      1, OpExposed},
  {"gettimeofday",            mint_gettimeofday,               1, OpExposed},
  {"sysmp",                   mint_sysmp,                      1, OpExposed},
  {"getdents",                mint_getdents,                   1, OpExposed},
  {"_getdents",               mint_getdents,                   1, OpExposed},
  {"sproc",                   mint_notimplemented,             1, OpExposed},
  {"sprocsp",                 mint_notimplemented,             1, OpExposed},
  {"m_fork",                  mint_notimplemented,             1, OpExposed},
  {"m_next",                  mint_notimplemented,             1, OpExposed},
  {"m_set_procs",             mint_notimplemented,             1, OpExposed},
  {"m_get_numprocs",          mint_notimplemented,             1, OpExposed},
  {"m_get_myid",              mint_notimplemented,             1, OpExposed},
  {"m_kill_procs",            mint_notimplemented,             1, OpExposed},
  {"m_parc_procs",            mint_notimplemented,             1, OpExposed},
  {"m_rele_procs",            mint_notimplemented,             1, OpExposed},
  {"m_sync",                  mint_notimplemented,             1, OpExposed},
  {"m_lock",                  mint_notimplemented,             1, OpExposed},
  {"m_unlock",                mint_notimplemented,             1, OpExposed},
  {"fork",                    mint_notimplemented,             1, OpExposed},
  {"wait",                    mint_notimplemented,             1, OpExposed},
  {"wait3",                   mint_notimplemented,             1, OpExposed},
  {"waitpid",                 mint_notimplemented,             1, OpExposed},
  {"pthread_create",          mint_notimplemented,             1, OpExposed},
  {"pthread_lock",            mint_notimplemented,             1, OpExposed},
  /* wait must generate a yield because it might block */
  {"isatty",                  mint_isatty,                     1, OpInternal},
  {"ioctl",                   mint_ioctl,                      1, OpExposed},
  {"prctl",                   mint_prctl,                      1, OpExposed},
  {"fcntl64",                 mint_fcntl,                      1, OpInternal},
  {"fcntl",                   mint_fcntl,                      1, OpInternal},
  {"cerror64",                mint_cerror,                     1, OpExposed},
  {"cerror",                  mint_cerror,                     1, OpExposed},
#if (defined TASKSCALAR) & (! defined ATOMIC)
  {"abort",                   mint_rexit,                      1, OpExposed},
  {"exit",                    mint_rexit,                      1, OpExposed},
#else
  {"abort",                   mint_exit,                       1, OpExposed},
/* {"exit",                    mint_exit,                       1, OpClass(OpUndoable,OpAtStart)}, */
#endif
  {"sesc_fetch_op",           mint_sesc_fetch_op,              1, OpExposed},
  {"sesc_unlock_op",          mint_sesc_unlock_op,             1, OpExposed},
  {"sesc_spawn",              mint_sesc_spawn,                 1, OpNoReplay},
  {"sesc_spawn_",             mint_sesc_spawn,                 1, OpNoReplay},
  {"sesc_sysconf",            mint_sesc_sysconf,               1, OpExposed},
  {"sesc_sysconf_",           mint_sesc_sysconf,               1, OpExposed},
  {"sesc_self",               mint_getpid,                     1, OpInternal},
  {"sesc_self_",              mint_getpid,                     1, OpInternal},
  {"sesc_exit",               mint_exit,                       1, OpExposed},
  {"sesc_exit_",              mint_exit,                       1, OpExposed},
  {"sesc_finish",             mint_finish,                     1, OpExposed},
  {"sesc_yield",              mint_sesc_yield,                 1, OpExposed},
  {"sesc_yield_",             mint_sesc_yield,                 1, OpExposed},
  {"sesc_suspend",            mint_sesc_suspend,               1, OpExposed},
  {"sesc_suspend_",           mint_sesc_suspend,               1, OpExposed},
  {"sesc_resume",             mint_sesc_resume,                1, OpExposed},
  {"sesc_resume_",            mint_sesc_resume,                1, OpExposed},
  {"sesc_simulation_mark",    mint_sesc_simulation_mark,       1, OpExposed},
  {"sesc_simulation_mark_id", mint_sesc_simulation_mark_id,    1, OpExposed},
  {"sesc_fast_sim_begin",     mint_sesc_fast_sim_begin,        1, OpExposed},
  {"sesc_fast_sim_begin_",    mint_sesc_fast_sim_begin,        1, OpExposed},
  {"sesc_fast_sim_end",       mint_sesc_fast_sim_end,          1, OpExposed},
  {"sesc_fast_sim_end_",      mint_sesc_fast_sim_end,          1, OpExposed},
  {"sesc_preevent",           mint_sesc_preevent,              1, OpExposed},
  {"sesc_preevent_",          mint_sesc_preevent,              1, OpExposed},
  {"sesc_postevent",          mint_sesc_postevent,             1, OpExposed},
  {"sesc_postevent_",         mint_sesc_postevent,             1, OpExposed},
  {"sesc_memfence",           mint_sesc_memfence,              1, OpExposed},
  {"sesc_memfence_",          mint_sesc_memfence,              1, OpExposed},
  {"sesc_acquire",            mint_sesc_acquire,               1, OpExposed},
  {"sesc_acquire_",           mint_sesc_acquire,               1, OpExposed},
  {"sesc_release",            mint_sesc_release,               1, OpExposed},
  {"sesc_release_",           mint_sesc_release,               1, OpExposed},
  {"sesc_wait",               mint_sesc_wait,                  1, OpClass(OpUndoable,OpAtStart)},
  {"sesc_pseudoreset",        mint_sesc_pseudoreset,           1, OpExposed},
  //  {"printf",                      mint_printf,                     1, OpExposed},
  //  {"IO_printf",                   mint_printf,                     1, OpExposed},
  {"sesc_get_num_cpus",       mint_sesc_get_num_cpus,          0, OpInternal},
#if (defined TLS)
  //  {"sesc_begin_epochs",       mint_sesc_begin_epochs,          0, OpExposed},
  {"sesc_future_epoch",       mint_sesc_future_epoch,          0, OpExposed},
  {"sesc_future_epoch_jump",  mint_sesc_future_epoch_jump,     0, OpExposed},
  {"sesc_commit_epoch",       mint_sesc_commit_epoch,          0, OpExposed},
  {"sesc_change_epoch",       mint_sesc_change_epoch,          0, OpInternal},
  //  {"sesc_end_epochs",         mint_sesc_end_epochs,            0, OpExposed},
#endif
#ifdef TASKSCALAR
  {"sesc_fork_successor",     mint_sesc_fork_successor,        0, OpExposed},
  {"sesc_prof_fork_successor",mint_sesc_prof_fork_successor,   0, OpExposed},
  {"sesc_commit",             mint_sesc_commit,                0, OpExposed},
  {"sesc_prof_commit",        mint_sesc_prof_commit,           0, OpExposed},
  {"sesc_become_safe",        mint_sesc_become_safe,           0, OpExposed},
  {"sesc_is_safe",            mint_sesc_is_safe,               0, OpExposed},
  {"sesc_is_versioned",       mint_do_nothing,                 0, OpExposed},
  {"sesc_begin_versioning",   mint_do_nothing,                 0, OpExposed},
#endif
#ifdef SESC_LOCKPROFILE
  {"sesc_startlock",          mint_sesc_startlock,             0, OpExposed},
  {"sesc_endlock",            mint_sesc_endlock,               0, OpExposed},
  {"sesc_startlock2",         mint_sesc_startlock2,            0, OpExposed},
  {"sesc_endlock2",           mint_sesc_endlock2,              0, OpExposed},
#endif
#ifdef VALUEPRED
  {"sesc_get_last_value",     mint_sesc_get_last_value,        0, OpExposed}, 
  {"sesc_put_last_value",     mint_sesc_put_last_value,        0, OpExposed}, 
  {"sesc_get_stride_value",   mint_sesc_get_stride_value,      0, OpExposed}, 
  {"sesc_put_stride_value",   mint_sesc_put_stride_value,      0, OpExposed}, 
  {"sesc_get_incr_value",     mint_sesc_get_incr_value,        0, OpExposed}, 
  {"sesc_put_incr_value",     mint_sesc_put_incr_value,        0, OpExposed}, 
  {"sesc_verify_value",       mint_sesc_verify_value,          0, OpExposed}, 
#endif
  {"uname",                   mint_uname,                      1, OpInternal},
  {"getrlimit",               mint_getrlimit,                  1, OpExposed},
  {"setrlimit",               mint_do_nothing,                 1, OpExposed},
  {"getrusage",               mint_getrusage,                  1, OpExposed},
  {"syscall_fstat64",         mint_fstat64,                    1, OpExposed},
  {"syscall_fstat",           mint_fstat,                      1, OpExposed},
  {"syscall_stat64",          mint_stat64,                     1, OpExposed},
  {"syscall_stat",            mint_stat,                       1, OpExposed},
  {"syscall_lstat64",         mint_lstat64,                    1, OpExposed},
  {"syscall_lstat",           mint_lstat,                      1, OpExposed},
  {"syscall_getcwd",          mint_getcwd,                     1, OpExposed},
  {"assert_fail",             mint_assert_fail,                1, OpExposed},
  {"sigaction",               mint_do_nothing,                 1, OpInternal},
  {"ftruncate64",             mint_do_nothing,                 1, OpExposed},
  {"rmdir",                   mint_rmdir,                      1, OpExposed},
  {"fxstat",                  mint_fxstat64,                   1, OpExposed},
  { NULL,                     NULL,                            1, OpExposed}
};

static struct namelist *Func_nlist;

int32_t Nfuncs = sizeof(Func_subst) / sizeof (struct func_desc_t);

void subst_init()
{
  int32_t i;
  int32_t notfound;
  struct namelist *pnlist, *psym;
  struct namelist errno_nlist[7];
  FILE *fdummy;

  Iterminator1.func = terminator1;
  Iterminator1.opnum = 0;
  Iterminator1.next = NULL;
  Iterminator1.instID = 0;

  invalidIcode.addr = 0;
  invalidIcode.func = NULL;
  invalidIcode.opnum = 0;
  invalidIcode.next = NULL;
  invalidIcode.instID = 8;

#ifdef TASKSCALAR
  TerminatePicode.func   = mint_exit;
  TerminatePicode.addr   = 8; /* Invalid addr */
  TerminatePicode.opnum  = 0;
  TerminatePicode.next   = NULL;
  TerminatePicode.instID = 0;
#endif

  bzero(errno_nlist,sizeof(struct namelist)*7);

  errno_nlist[0].n_name = "__errnoaddr";
  /* Original: errno_nlist[0].n_name = "errno"; */
  errno_nlist[1].n_name = "sesc_spawn";
  errno_nlist[2].n_name = "exit";
  errno_nlist[3].n_name = "environ";
  errno_nlist[4].n_name = "m_fork";
  errno_nlist[5].n_name = "sprocsp";
  errno_nlist[6].n_name = NULL;

  /* check that object exists and is readable */
  fdummy = fopen(Objname, "r");
  if (fdummy == NULL) {
    perror(Objname);
    exit(1);
  }
  fclose(fdummy);
  notfound = namelist(Objname, errno_nlist);
  if (notfound == -1)
    fatal("nlist() failed on %s\n", Objname);
  Errno_addr = errno_nlist[0].n_value;
  if( Errno_addr == 4 ) {
#ifdef DEBUG
    printf("Errno_addr is pointing to who knows where\n");
#endif
    Errno_addr = 0;
  }
    
  Environ_addr = errno_nlist[3].n_value;
  Exit_addr = errno_nlist[2].n_value;
  if (errno_nlist[2].n_type == 0 || errno_nlist[2].n_value == 0)
    fatal("subst_init: cannot find address of exit()\n");

  pnlist = (struct namelist *) malloc(sizeof(struct namelist) * Nfuncs);
  if (pnlist == NULL)
    fatal("subst_init: cannot allocate 0x%x bytes for function nlist.\n",
          sizeof(struct namelist) * Nfuncs);
  for (i = 0; i < Nfuncs; i++)
    pnlist[i].n_name = Func_subst[i].name;

  notfound = namelist(Objname, pnlist);
  if (notfound == -1)
    fatal("nlist() failed on %s\n", Objname);
  Func_nlist = pnlist;

  /* Find minimum address in name list. Anything past this address
   * can be assumed to be in library code.
   */
  Min_lib_value = 0xffffffff;
  for (psym = pnlist; psym->n_name; psym++) {
    if (psym->n_type == 0)
      continue;
    if (strcmp(psym->n_name, "sesc_preevent") == 0)
      continue;
    if (strcmp(psym->n_name, "sesc_preevent_") == 0)
      continue;

    if (strcmp(psym->n_name, "sesc_postevent") == 0)
      continue;
    if (strcmp(psym->n_name, "sesc_postevent_") == 0)
      continue;

    if (strcmp(psym->n_name, "sesc_memfence") == 0)
      continue;
    if (strcmp(psym->n_name, "sesc_memfence_") == 0)
      continue;
                
    if (strcmp(psym->n_name, "sesc_acquire") == 0)
      continue;
    if (strcmp(psym->n_name, "sesc_acquire_") == 0)
      continue;

    if (strcmp(psym->n_name, "sesc_release") == 0)
      continue;
    if (strcmp(psym->n_name, "sesc_relase_") == 0)
      continue;
    if (Min_lib_value > psym->n_value)
      Min_lib_value = psym->n_value;
  }
}

void subst_functions()
{
  int32_t i;
  int32_t notfound;
  icode_ptr picode, pcopy;
  func_desc_ptr pfname;
  struct namelist *pnlist, *psym;
  int32_t base;
  
  pnlist = Func_nlist;
  pfname = Func_subst;
  for(psym=pnlist;psym->n_name;psym++,pfname++){
    if(psym->n_type==0)
      continue;
    if(psym->n_value<(unsigned)Text_start||psym->n_value>=(unsigned)Text_end)
      continue;
    
#ifdef DEBUG_VERBOSE
    printf("func name = %s, value = 0x%x\n", psym->n_name, psym->n_value);
#endif
    picode = addr2icode(psym->n_value);
    /* replace the first instruction in this routine with my function */
    picode->func = pfname->func;
#if (defined TASKSCALAR)
    if (pfname->no_spec == 1)
      picode->opflags = E_NO_SPEC;
    if (pfname->no_spec == 2)
      picode->opflags = E_LIMIT_SPEC;
#endif
#if (defined TLS)
    picode->setClass(pfname->opClass);
#endif
    picode->opnum = 0;
    
    /* Set the next field to return to the same routine. If we are
     * generating events, then this field will be changed to point to
     * the event function.
     */
    picode->next = picode;
  }
  free(pnlist);
  
  base = Text_size;
  Idone1 = Itext[base + DONE_ICODE];
}

OP(mint_assert_fail)
{
  printf("assertion failed in simulated program: (%s) %s:%d\n",
         (const char *)pthread->virt2real(pthread->getIntReg(IntArg1Reg)),
         (const char *)pthread->virt2real(pthread->getIntReg(IntArg2Reg)),
         pthread->getIntReg(IntArg3Reg)
         ); 
  
  mint_exit(0,pthread);

  return 0;
}

OP(mint_notimplemented)
{
  fatal("System call @0x%lx not implemented. Use the appropiate API\n",
        picode->addr);

  return 0;
}

void rsesc_sysconf(int32_t pid, int32_t id, int32_t flags);
icode_ptr rsesc_get_instruction_pointer(int32_t pid);
void rsesc_set_instruction_pointer(int32_t pid, icode_ptr picode);
void rsesc_spawn_stopped(int32_t pid, int32_t id, int32_t flags);

OP(mint_sesc_sysconf)
{
  int32_t thePid = pthread->getPid();
  int32_t pid    = pthread->getIntArg1();
  int32_t flags  = pthread->getIntArg2();

  rsesc_sysconf(thePid,pid,flags);
  // There should be no context switch
  I(pthread->getPid()==thePid);
  // Return from the call
  return pthread->getRetIcode();
}

OP(mint_sesc_spawn){
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
#if (defined TLS)
  tls::Epoch *epoch=tls::Epoch::getEpoch(pthread->getPid());
  I(epoch);
  SysCallSescSpawn *sysCall=epoch->newSysCall<SysCallSescSpawn>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else
  // Process ID of the parent thread
  Pid_t ppid=pthread->getPid();
  // Arguments of the sesc_spawn call
  MintFuncArgs funcArgs(pthread,picode);
  VAddr entry = funcArgs.getVAddr();
  VAddr arg   = funcArgs.getVAddr();
  int32_t   flags = funcArgs.getInt32();
  
#ifdef DEBUG_VERBOSE
  printf("sesc_spawn( 0x%lx, 0x%lx, 0x%lx ) pid = %d\n",
         (unsigned long)entry,(unsigned long)arg,(unsigned long)flags,ppid);
#endif
  
  // Allocate a new thread for the child
  ThreadContext *child=ThreadContext::newActual();
  if(!child)
    fatal("sesc_spawn: exceeded process limit (%d)\n", Max_nprocs);

  // Process ID of the child thread
  Pid_t cpid=child->getPid();
  if (Maxpid < cpid)
    Maxpid = cpid;
  
  /* entry == 0 is more like a fork (with shared address space of
     course). In share_addr_space, the last parameter set to true
     means that the stack is copied
  */
  child->shareAddrSpace(pthread, PR_SADDR, !entry);
  
  child->init();

  pthread->newChild(child);
  
  if( entry ) {
    // The first instruction for the child is the entry point passed in
    child->setPCIcode(addr2icode(entry));
    child->setIntReg(IntArg1Reg,arg);
    child->setIntReg(IntArg2Reg,Stack_size);            /* for sprocsp() */
    // In position-independent code every function expects to find
    // its own entry address in register jp (also known as t9 and R25)
    child->setIntReg(JmpPtrReg,entry);
    // When the child returns from the 'entry' function,
    // it will go directly to the exit() function
    child->setIntReg(RetAddrReg,Exit_addr);
  }else{
    // If no entry point is supplied, we have fork-like behavior
    // The child will just return from sesc_spawn the same as parent
    child->setPCIcode(pthread->getPCIcode());
  }
  // The return value for the parent is the child's pid
  pthread->setRetVal(cpid);
  // The return value for the child is 0
  child->setRetVal(0);
  // Inform SESC of what we have done here
  osSim->eventSpawn(ppid,cpid,flags);
#endif // End (defined TLS) else block
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

int32_t mint_sesc_create_clone(ThreadContext *pthread){
  int32_t ppid=pthread->getPid();
  int32_t cpid;
  ThreadContext *cthread;
  int32_t i;
#if (defined TLS)
  cthread=ThreadContext::newCloned();
  assert(cthread!=0);
#else
#ifdef DEBUG_VERBOSE
  printf("mint_sesc_create_clone: pid=%d at iAddr %08x\n",
         ppid,icode2addr(pthread->getPCIcode()));
#endif
  cthread=ThreadContext::newActual();
#endif /* (defined TLS) */

  if(cthread==NULL) {
    fatal("mint_sesc_create_clone: exceeded process limit (%d)\n",Max_nprocs);
  }
  cpid=cthread->getPid();
  if(Maxpid<cpid)
    Maxpid=cpid;
  /* The child and the parent have the same address space (even the stack) */
  cthread->useSameAddrSpace(pthread);

  /* init_thread() must be called *after* use_same_addr_space() since
   * init_thread() uses the mapping fields set up by use_same_addr_space().
   */
  cthread->init();
  pthread->newChild(cthread);

  // The first instruction for the child is to return
  cthread->setPicode(pthread->getPCIcode());
  cthread->setTarget(pthread->getTarget());

  rsesc_spawn_stopped(ppid,cpid,0);
  return cpid;
}

void mint_sesc_die(thread_ptr pthread)
{
  pthread->setIntReg(IntArg1Reg,0);
  mint_exit(0,pthread);
}

#if (defined TLS)

OP(aspectReductionBegin_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->beginReduction(picode->addr);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectReductionBegin_op[] = { aspectReductionBegin_op_0, 0 };

OP(aspectReductionEnd_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->endReduction();
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectReductionEnd_op[]   = { aspectReductionEnd_op_0,   0 };

OP(aspectAtomicBegin_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->beginAtomic(picode->addr,false,false);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectAtomicBegin_op[]   = { aspectAtomicBegin_op_0,   0 };

OP(aspectAcquireBegin_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->beginAtomic(picode->addr,true,false);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectAcquireBegin_op[]   = { aspectAcquireBegin_op_0,   0 };

OP(aspectAcquireRetry_op_0){
  // No need to set up next instruction, because it never gets executed
  pthread->getEpoch()->retryAtomic();
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectAcquireRetry_op[]   = { aspectAcquireRetry_op_0,   0 };

OP(aspectAcquireExit_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->changeAtomic(true,false);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectAcquireExit_op[]   = { aspectAcquireExit_op_0,   0 };
OP(aspectAcquire2Release_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->changeAtomic(true,true);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectAcquire2Release_op[]   = { aspectAcquire2Release_op_0,   0 };

OP(aspectReleaseBegin_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->beginAtomic(picode->addr,false,true);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectReleaseBegin_op[]   = { aspectReleaseBegin_op_0,   0 };

OP(aspectReleaseEnter_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->changeAtomic(false,true);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectReleaseEnter_op[]   = { aspectReleaseEnter_op_0,   0 };

OP(aspectAtomicEnd_op_0){
  // Move instruction pointer to next instruction
  pthread->setPCIcode(picode->next);
  // Emulate the actual instruction
  pthread->getEpoch()->endAtomic();
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}
PFPI aspectAtomicEnd_op[]   = { aspectAtomicEnd_op_0,   0 };

//OP(mint_sesc_begin_epochs){
  // Set things up for the return from this call
//  pthread->setIP(addr2icode(pthread->getGPR(RetAddrGPR)));
  // Do the actual call
//   if(!pthread->getEpoch()){
//     tls::Epoch *iniEpoch=tls::Epoch::initialEpoch(static_cast<tls::ThreadID>(pthread->getPid()));
//     iniEpoch->run();
//   }
  // Note: not neccessarily running the same thread as before
//  return pthread->getIP();
//}

OP(mint_sesc_future_epoch){
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
  /* Do the actual call to create the successor */
  rsesc_future_epoch(pthread->getPid());
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

OP(mint_sesc_future_epoch_jump){
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
  /* The address where the successor is to start */
  int32_t successorAddr=pthread->getIntArg1();
  /* Do the actual call to create the successor */
  rsesc_future_epoch_jump(pthread->getPid(),addr2icode(successorAddr));
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

OP(mint_sesc_commit_epoch){
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
  /* Do the actual call to fork the successor */
  tls::Epoch *epoch=pthread->getEpoch();
  I(epoch==tls::Epoch::getEpoch(pthread->getPid()));
  if(epoch)
    epoch->complete();
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

OP(mint_sesc_change_epoch){
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
  // Do the actual call
  tls::Epoch *oldEpoch=pthread->getEpoch();
  I(oldEpoch==tls::Epoch::getEpoch(pthread->getPid()));
  if(oldEpoch)
    oldEpoch->changeEpoch();
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

#endif

#ifdef TASKSCALAR
void rsesc_fork_successor(int32_t ppid, int32_t where, int32_t tid);
OP(mint_sesc_fork_successor)
{
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
  // Do the actual call
  rsesc_fork_successor(pthread->getPid(),pthread->getIntReg(RetAddrReg),0);
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

OP(mint_sesc_prof_fork_successor)
{
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
  // Do the actual call
  rsesc_fork_successor(pthread->getPid(),pthread->getIntReg(RetAddrReg),pthread->getIntArg1());
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

OP(mint_sesc_become_safe)
{
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
  // Do the actual call
  rsesc_become_safe(pthread->getPid());
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

bool rsesc_is_safe(int32_t pid);
OP(mint_sesc_is_safe)
{
  // Do the call and set the return value
  pthread->setRetVal(rsesc_is_safe(pthread->getIntArg1())?1:0);
  // Return from the call
  return pthread->getRetIcode();
}
#endif



#ifdef VALUEPRED
int32_t rsesc_get_last_value(int32_t pid, int32_t index);
OP(mint_sesc_get_last_value)
{
  int32_t index = pthread->getIntArg1();
  pthread->setRetVal(rsesc_get_last_value(pthread->getPid(),index));
  // Return from the call
  return pthread->getRetIcode();
}

void rsesc_put_last_value(int32_t pid, int32_t index, int32_t val);
OP(mint_sesc_put_last_value)
{
  int32_t index = pthread->getIntArg1();
  int32_t val   = pthread->getIntArg2();
  rsesc_put_last_value(pthread->getPid(), index, val);
  // Return from the call
  return pthread->getRetIcode();
}

int32_t rsesc_get_stride_value(int32_t pid, int32_t index);
OP(mint_sesc_get_stride_value)
{
  int32_t index = pthread->getIntArg1();
  pthread->setRetVal(rsesc_get_stride_value(pthread->getPid(), index));
  // Return from the call
  return pthread->getRetIcode();
}

void rsesc_put_stride_value(int32_t pid, int32_t index, int32_t val);
OP(mint_sesc_put_stride_value)
{
  int32_t index = pthread->getIntArg1();
  int32_t val   = pthread->getIntArg2();
  rsesc_put_stride_value(pthread->getPid(), index, val);
  // Return from the call
  return pthread->getRetIcode();
}

int32_t rsesc_get_incr_value(int32_t pid, int32_t index, int32_t lval);
OP(mint_sesc_get_incr_value)
{
  int32_t index = pthread->getIntArg1();
  int32_t lval  = pthread->getIntArg2();
  pthread->setRetVal(rsesc_get_incr_value(pthread->getPid(), index, lval));
  // Return from the call
  return pthread->getRetIcode();
}

void rsesc_put_incr_value(int32_t pid, int32_t index, int32_t incr);
OP(mint_sesc_put_incr_value)
{
  int32_t  index = pthread->getIntArg1();
  int32_t  incr  = pthread->getIntArg2();
  rsesc_put_incr_value(pthread->getPid(), index, incr);
  // Return from the call
  return pthread->getRetIcode();
}

void rsesc_verify_value(int32_t pid, int32_t rval, int32_t pval);
OP(mint_sesc_verify_value)
{
  int32_t  rval = pthread->getIntArg1();
  int32_t  pval = pthread->getIntArg2();
  rsesc_verify_value(pthread->getPid(), rval, pval);
  // Return from the call
  return pthread->getRetIcode();
}
#endif

/* ARGSUSED */
OP(mint_sysmp)
{
  int32_t command = pthread->getIntArg1();
  switch(command){
  case MP_MUSTRUN:
  case MP_RUNANYWHERE:
  case MP_CLOCK:
  case MP_EMPOWER:
  case MP_RESTRICT:
  case MP_UNISOLATE:
  case MP_SCHED:
    pthread->setRetVal(0);
    break;
  case MP_NPROCS:
  case MP_NAPROCS:
    pthread->setRetVal(Max_nprocs);
    break;
  case MP_PGSIZE:
    pthread->setRetVal(getpagesize());
    break;
  default:
    fatal("mint_sysmp( 0x%x ) not supported yet.\n", command);
    break;
  }
  return pthread->getRetIcode();
}

#if 0
// FIXME: the following code is not ready for 64 bit architectures

/* Please, someone should port mint to execute mint3 and glibc from
 * gcc. glibc is a fully reentrant library, not like this crap!
 */
OP(mint_printf)
{
  int32_t addr;
  int32_t *sp;
  int32_t index;
  char *cp;
  int32_t args[100];
#if (defined TASKSCALAR) || (defined TLS)
  int32_t *wdata;
  uint8_t *tempbuff2 = (uint8_t *) alloca (8000);
#endif
 
#ifdef DEBUG_VERBOSE
  printf("mint_printf()\n");
#endif

#if (defined TASKSCALAR) || (defined TLS)
  sp = (int32_t *)pthread->getIntReg(StkPtrReg);
  rsesc_OS_read_string(pthread->getPid(), picode->addr, tempbuff2, pthread->getIntArg1() ,100);
  addr = (int)tempbuff2;
  tempbuff2 += 100;
#else
  addr = pthread->virt2real(pthread->getIntArg1());
  sp = (int32_t *)pthread->virt2real(pthread->getIntReg(StkPtrReg));
#endif
  args[0] = addr;
  args[1] = pthread->getIntArg2();
  args[2] = pthread->getIntArg3();
  args[3] = pthread->getIntArg4();

  index = 0;
  for(cp=(char *)addr;*cp;cp++){
    if(*cp!='%')
      continue;
    if(*++cp=='%')
      continue;
    index++;

    if(index>3){
#if (defined TASKSCALAR) || (defined TLS)
      wdata = (int32_t *)rsesc_OS_read(pthread->getPid(), picode->addr, ((RAddr)sp) + index, E_BYTE);
      args[index]=SWAP_WORD(*wdata);
#else
      args[index]=SWAP_WORD(*(sp+index));
#endif
    }
    while((*cp>='0')&&(*cp<='9')) {
      cp++;
    }
    if(*cp=='s'){
      if(args[index]) {
#if (defined TASKSCALAR) || (defined TLS)
        rsesc_OS_read_string(pthread->getPid(), picode->addr, tempbuff2, args[index], 100);
        args[index]=(int)tempbuff2;
        tempbuff2 += 100;
#else
        args[index]=pthread->virt2real(args[index]);
#endif
      }else
        args[index]=(int)"(nil)";
    }else if(*cp=='f'||*cp=='g') {
      /* found a double */
      /* need to fix this sometime; for now don't use both %f and %s */
    }
  }

#ifdef TASKSCALAR
  if(index>10){
    printf("mint_printf: too many args\n");
    return pthread->getRetIcode();
  }
  if (!rsesc_is_safe(pthread->getPid()))
    printf("SPEC[");
#else
  if(index>10)
    fatal("mint_printf: too many args\n");
#endif

  printf((char *)addr, args[1], args[2], args[3], args[4], args[5], args[6],
         args[7], args[8], args[9], args[10]);

#ifdef TASKSCALAR
  if (!rsesc_is_safe(pthread->getPid())) {
    printf("]");
    fflush(stdout);
  }
    
#endif

  fflush(stdout);

  return  pthread->getRetIcode();
}
#endif

// enum FetchOpType {
//   FetchIncOp  =0,
//   FetchDecOp,
//   FetchSwapOp
// };

#include "sescapi.h"

int32_t rsesc_fetch_op(int32_t pid, enum FetchOpType op, int32_t addr, int32_t *data, int32_t val);
OP(mint_sesc_fetch_op)
{
  int32_t  op   = pthread->getIntArg1();
  int32_t addr  = pthread->getIntArg2();
  int32_t val   = pthread->getIntArg3();
  int32_t *data = (int32_t *)pthread->virt2real(addr);

  pthread->setRetVal(rsesc_fetch_op(pthread->getPid(),(enum FetchOpType)op,addr,data,val));
  
  return pthread->getRetIcode();
}

void rsesc_unlock_op(int32_t pid, int32_t addr, int32_t *data, int32_t val);
OP(mint_sesc_unlock_op)
{
  int32_t addr  = pthread->getIntArg1();
  int32_t val   = pthread->getIntArg2();
  int32_t *data = (int32_t *)pthread->virt2real(addr);
  rsesc_unlock_op(pthread->getPid(), addr, data, val);
  return pthread->getRetIcode();
}

int32_t rsesc_exit(int32_t pid, int32_t err);

void rsesc_finish(int32_t pid);

OP(mint_finish)
{
  rsesc_finish(pthread->getPid());
  return &Iterminator1;
}

/* ARGSUSED */
OP(mint_exit){
#if (defined TASKSCALAR)
  if(rsesc_version_can_exit(pthread->getPid())==1)
    return pthread->getRetIcode();
#endif
#if (defined TLS)
  callSysCallExit(pthread,picode);
#else
  rsesc_exit(pthread->getPid(),pthread->getIntArg1());
#endif // (defined TLS)
  // Note: not running the same thread as before
  return pthread->getPCIcode();
}

#ifdef TASKSCALAR
OP(mint_sesc_commit)
{
#ifdef DEBUG_VERBOSE
  printf("mint_sesc_commit\n");
#endif  
  pthread->setIntReg(RetValReg,1);
  return mint_exit(picode, pthread);
}

void rsesc_prof_commit(int32_t pid, int32_t tid);
OP(mint_sesc_prof_commit)
{
  int32_t tid = pthread->getIntArg1();
  int32_t pid = pthread->getPid();

#ifdef DEBUG_VERBOSE
  printf("mint_sesc_prof_commit\n");
#endif  
  rsesc_prof_commit(pid, tid);

  pthread->setRetVal(1);
  return mint_exit(picode, pthread);
}
#endif


#ifdef TASKSCALAR
void mint_termination(int32_t pid);
OP(mint_rexit)
{
  fprintf(stderr,"mint_rexit called pid(%d) RA=0x%08x\n",
          pthread->getPid(), (uint32_t) pthread->getIntReg(RetAddrReg));
  mint_termination(pthread->getPid());

  return pthread->getPCIcode();
}
#endif

/* ARGSUSED */
OP(mint_isatty)
{
#ifdef DEBUG_VERBOSE
    printf("mint_isatty()\n");
#endif

  if(pthread->getIntArg1()==0)
    pthread->setRetVal(pthread->getIntArg1());
  else
    pthread->setRetVal(1);
/*  REGNUM(2) = isatty(REGNUM(4)); 
 *  REGNUM(2) = 1;
 *
 * This should be the real, but makes simulation harder because the
 * program behaves different if the output is redirected to the
 * standard output
 * MCH: we only fake out to be tty. Input is real checking because of crafty
 */

#ifdef DEBUG_VERBOSE
  printf("isatty(%d) returned %d\n",
         (int)pthread->getIntArg1(),(int)pthread->getIntReg(RetValReg));
#endif
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_ioctl)
{
#if (defined __LP64__)
  fatal("mint_ioctl: Not supported yet under __LP64__");
#else
  int32_t fd, cmd, arg;
  int32_t err;

#ifdef DEBUG_VERBOSE
  printf("mint_ioctl()\n");
#endif
  fd  = pthread->getIntArg1();
  cmd = pthread->getIntArg2();
  arg = pthread->getIntArg3();
  switch (cmd) {
#if 0
  case FIOCLEX:
  case FIONCLEX:
    break;
#endif
#if (!defined DARWIN) && (!defined CYGWIN)
  case I_FLUSH:
  case I_SETSIG:
  case I_SRDOPT:
  case I_SENDFD:
    break;
#endif
#if (!defined SUNOS) && (!defined CYGWIN)
  case FIONREAD:
  case FIONBIO:
  case FIOASYNC:
  case FIOSETOWN:
  case FIOGETOWN:
#endif
#if (!defined DARWIN) && (!defined CYGWIN)
  case I_PUSH:
  case I_POP:
  case I_LOOK:
  case I_GETSIG:
  case I_FIND:
  case I_GRDOPT:
  case I_NREAD:
#endif
#ifdef CYGWIN
  case WINDOWS_POST:
  case WINDOWS_SEND:
  case WINDOWS_HWND:
#endif
    if (arg)
      arg = pthread->virt2real(arg);
    break;
  default:
    fatal("ioctl command %d (0x%x) not supported.\n", cmd, cmd);
    break;
  }
  err = ioctl(fd, cmd, (char *) arg);
  pthread->setRetVal(err);
  if(err == -1)
    pthread->setperrno(errno);
#endif // !(defined __LP64__)
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_prctl)
{
  int32_t option;
  int32_t err = 0;
#ifdef DEBUG_VERBOSE
  printf("mint_prctl()\n");
#endif
  option = pthread->getIntArg1();
  switch (option) {
  case PR_MAXPROCS:
  case PR_MAXPPROCS:
    err = Max_nprocs;
    break;
  case PR_GETNSHARE:
    err = 0;
    break;
  case PR_SETEXITSIG:
    /* not really supported, but just fake it for now */
    err = 0;
    break;
  default:
    fatal("prctl option %d not supported yet.\n", option);
    break;
  }
  pthread->setRetVal(err);
  if(err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_fcntl)
{
  int32_t err;

#ifdef DEBUG_VERBOSE
  printf("mint_fcntl()\n");
#endif
  int32_t fd  = pthread->getIntArg1();
  int32_t cmd = pthread->getIntArg2();
  int32_t arg = pthread->getIntArg3();
  switch (cmd) {
  case F_DUPFD:
#if (defined TLS)
    fatal("mint_fcntl: F_DUPFD not supported.\n");
    break;
#endif
  case F_GETFD:
#if (defined TLS)
    pthread->setRetVal(0);
    return pthread->getRetIcode();
#endif
  case F_SETFD:
#if (defined TLS)
    fatal("mint_fcntl: F_SETFD not supported.\n");
    break;
#endif
  case F_GETFL:
  case F_SETFL:
#if (defined TLS)
    fatal("mint_fcntl: F_GETFL and F_SETFL not supported.\n");
#endif
    break;
  case F_GETLK:
  case F_SETLK:
#if (defined TLS) || (defined __LP64__)
    fatal("mint_fcntl: F_GETLK and F_SETLK not supported.\n");
#endif
    if (arg)
      arg = pthread->virt2real(arg);
    break;
  case F_SETLKW:
    fatal("mint_fcntl: system call fcntl cmd `F_SETLKW' not supported.\n");
    break;
  case F_GETOWN:
#if (defined TLS)
    fatal("mint_fcntl: F_GETOWN not supported.\n");
#endif
    break;
  case F_SETOWN:
#if (defined TLS)
    fatal("mint_fcntl: F_SETOWN not supported.\n");
#endif
    break;
  default:
    fatal("mint_fcntl: system call fcntl cmd (%d) not expected.\n",
          cmd);
  }
  err = fcntl(fd, cmd, arg);
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  else {
    if (cmd == F_DUPFD && err < MAX_FDNUM)
      pthread->setFD(err, 1);
  }
  return pthread->getRetIcode();
}

struct  mint_utsname {
  char    sysname[65];  /* Name of OS */
  char    nodename[65]; /* Name of this network node */
  char    release[65];  /* Release level */
  char    version[65];  /* Version level */
  char    machine[65];  /* Hardware type */
};

const char *sysnamestr="SescLinux";
const char *nodenamestr="sesc";
const char *releasestr="2.4.18";
const char *versionstr="#1 SMP Tue Jun 4 16:05:29 CDT 2002"; /* fake */
const char *machinestr="mips";

/* ARGSUSED */
OP(mint_uname)
{
#ifdef DEBUG_VERBOSE
  printf("mint_uname()\n");
#endif
  VAddr bufVAddr = pthread->getIntArg1();
  if(bufVAddr == 0) {
    fatal("uname called with a null pointer\n");
  }
#if (defined TLS)
  {
    struct mint_utsname *tp = (struct mint_utsname *)bufVAddr;
    rsesc_OS_write_block(pthread->getPid(),picode->addr,
			 (VAddr)(tp->sysname),  sysnamestr,  strlen(sysnamestr)+1);
    rsesc_OS_write_block(pthread->getPid(),picode->addr,
			 (VAddr)(tp->nodename), nodenamestr, strlen(nodenamestr)+1);
    rsesc_OS_write_block(pthread->getPid(),picode->addr,
			 (VAddr)(tp->release),  releasestr,  strlen(releasestr)+1);
    rsesc_OS_write_block(pthread->getPid(),picode->addr,
			 (VAddr)(tp->version),  versionstr,  strlen(versionstr)+1);
    rsesc_OS_write_block(pthread->getPid(),picode->addr,
			 (VAddr)(tp->machine),  machinestr,  strlen(machinestr)+1);
  }
#else
  {
    struct mint_utsname *tp = (struct mint_utsname *)pthread->virt2real(bufVAddr);
    strcpy(tp->sysname,sysnamestr);
    strcpy(tp->nodename,nodenamestr);
    strcpy(tp->release,releasestr);
    strcpy(tp->version,versionstr);
    strcpy(tp->machine,machinestr);
  }
#endif
  pthread->setRetVal(0);
  return pthread->getRetIcode();
}

struct targRlimit{
  targULong rlim_cur;
  targULong rlim_max;
};

/* ARGSUSED */
OP(mint_getrlimit)
{
  MintFuncArgs funcArgs(pthread,picode);
  targInt resource = funcArgs.getInt32();
  VAddr   rlim     = funcArgs.getVAddr();
  targInt retVal=0;
#ifdef DEBUG_VERBOSE
  printf("mint_getrlimit()\n");
#endif
  if(rlim==0) {
    fatal("mint_getrlimit called with a null pointer\n");
  }
  struct targRlimit *targRlimitPtr=(struct targRlimit *)(pthread->virt2real(rlim));
  switch(resource){
    case RLIMIT_STACK:
      targRlimitPtr->rlim_cur=SWAP_WORD(Stack_size);
      targRlimitPtr->rlim_max=SWAP_WORD(Stack_size);
      retVal=0;
      break;
    default:
      fatal("mint_getrlimit does not yet support resource %d\n",resource);
  }
  if(retVal == -1)
    pthread->setErrno(errno);
  pthread->setRetVal(retVal);
  return pthread->getRetIcode();
}

int32_t rsesc_usecs();

struct targTimeval{
  targULong tv_sec;
  targULong tv_usec;
};

struct targRusage{
  // Total amount of user time used.
  struct targTimeval ru_utime;
  // Total amount of system time used.
  struct targTimeval ru_stime;
  // Maximum resident set size (in kilobytes).
  targLong ru_maxrss;
  // Amount of sharing of text segment memory
  // with other processes (kilobyte-seconds).
  targLong ru_ixrss;
  // Amount of data segment memory used (kilobyte-seconds).
  targLong ru_idrss;
  // Amount of stack memory used (kilobyte-seconds).
  targLong ru_isrss;
  // Number of soft page faults (i.e. those serviced by reclaiming
  // a page from the list of pages awaiting reallocation.
  targLong ru_minflt;
  // Number of hard page faults (i.e. those that required I/O).
  targLong ru_majflt;
  // Number of times a process was swapped out of physical memory.
  targLong ru_nswap;
  // Number of input operations via the file system.  Note: This
  //  and `ru_oublock' do not include operations with the cache.
  targLong ru_inblock;
  // Number of output operations via the file system.
  targLong ru_oublock;
  // Number of IPC messages sent.
  targLong ru_msgsnd;
  // Number of IPC messages received.
  targLong ru_msgrcv;
  // Number of signals delivered.
  targLong ru_nsignals;
  // Number of voluntary context switches, i.e. because the process
  // gave up the process before it had to (usually to wait for some
  // resource to be available).
  targLong ru_nvcsw;
  // Number of involuntary context switches, i.e. a higher priority process
  // became runnable or the current process used up its time slice.
  targLong ru_nivcsw;
};

/* ARGSUSED */
OP(mint_getrusage)
{
  MintFuncArgs funcArgs(pthread,picode);
  targInt who   = funcArgs.getInt32();
  VAddr   usage = funcArgs.getVAddr();
  targInt retVal=0;
#ifdef DEBUG_VERBOSE
  printf("mint_getrusage()\n");
#endif
  if(usage == 0) {
    fatal("uname called with a null pointer\n");
  }
  struct targRusage *targRusagePtr=(struct targRusage *)(pthread->virt2real(usage));
  // For user time, get the microsecond count since the beginning of the simulation
  uint64_t sescUusecs=rsesc_usecs();
  // System time is simply 1% of user time
  uint64_t sescSusecs=sescUusecs/100;
  targRusagePtr->ru_utime.tv_sec  = SWAP_WORD(sescUusecs/1000000);
  targRusagePtr->ru_utime.tv_usec = SWAP_WORD(sescUusecs%1000000);
  targRusagePtr->ru_stime.tv_sec  = SWAP_WORD(sescSusecs/1000000);
  targRusagePtr->ru_stime.tv_usec = SWAP_WORD(sescSusecs%1000000);
  // Fake some values for the other stuff
  targRusagePtr->ru_maxrss   = SWAP_WORD(1);
  targRusagePtr->ru_ixrss    = SWAP_WORD(1);
  targRusagePtr->ru_idrss    = SWAP_WORD(50);
  targRusagePtr->ru_isrss    = SWAP_WORD(10);
  targRusagePtr->ru_minflt   = SWAP_WORD(10);
  targRusagePtr->ru_majflt   = SWAP_WORD(5);
  targRusagePtr->ru_nswap    = SWAP_WORD(1);
  targRusagePtr->ru_inblock  = SWAP_WORD(4);
  targRusagePtr->ru_oublock  = SWAP_WORD(2);
  targRusagePtr->ru_msgsnd   = SWAP_WORD(1);
  targRusagePtr->ru_msgrcv   = SWAP_WORD(1);
  targRusagePtr->ru_nsignals = SWAP_WORD(3);
  targRusagePtr->ru_nvcsw    = SWAP_WORD(2);
  targRusagePtr->ru_nivcsw   = SWAP_WORD(10);
  if(retVal == -1)
    pthread->setErrno(errno);
  pthread->setRetVal(retVal);
  return pthread->getRetIcode();
}


/* ARGSUSED */
OP(mint_getpid)
{
#ifdef DEBUG_VERBOSE
  printf("mint_getpid()\n");
#endif
  pthread->setRetVal(pthread->getPid());
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_getppid)
{
#ifdef DEBUG_VERBOSE
  printf("mint_getppid()\n");
#endif

  if (pthread->getParent())
    pthread->setRetVal(pthread->getParent()->getPid());
  else
    pthread->setRetVal(0);

  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_clock)
{
  static clock_t t=0;

  if (t==0) {
    t = clock();
  }
  /* 100 ticks per second is the standard */

  pthread->setRetVal(t+rsesc_usecs()/10000);

#ifdef DEBUG_VERBOSE
  printf("mint_clock() %d\n", pthread->getIntReg(RetValReg));
#endif

  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_null_func)
{
#ifdef DEBUG_VERBOSE
  printf("null func\n");
#endif

  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(return1)
{
#ifdef DEBUG_VERBOSE
  printf("return 1\n");
#endif

  pthread->setRetVal(1);
  return pthread->getRetIcode();
}

OP(mint_cerror)
{
  printf("mint_cerror\n");
#ifdef DEBUG_VERBOSE
  printf("mint_cerror\n");
#endif
  pthread->dump();
  picode->dump();
  pthread->setRetVal(-1);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_gettimeofday)
{
  // Prepare for parameter extraction
  MintFuncArgs funcArgs(pthread, picode);
  // First parameter is an address
  VAddr tvVAddr = funcArgs.getVAddr();
  // Note that we ignore the second (time zone) parameter

  // Check if the first parameter is a valid data address
  if (!pthread->isValidDataVAddr(tvVAddr)){
    pthread->setRetVal(-1);
    pthread->setErrno(EFAULT);
    return pthread->getRetIcode();
  }

  static uint64_t nativeMicroSeconds=0;
  static const uint64_t secs2usecs=(uint64_t)1000*(uint64_t)1000;
  if(!nativeMicroSeconds) {
    struct timeval tv;
    gettimeofday(&tv,0);
    nativeMicroSeconds = tv.tv_sec*secs2usecs + tv.tv_usec;
  }

  uint64_t usecs = nativeMicroSeconds+rsesc_usecs();

  struct targTimeval tv;
  tv.tv_sec  = SWAP_WORD((IntRegValue)(usecs/secs2usecs));
  tv.tv_usec = SWAP_WORD((IntRegValue)(usecs%secs2usecs));

#ifdef DEBUG_VERBOSE
  printf("mint_gettimeofday() %lld secs %lld usesc\n",usecs/secs2usecs,usecs%secs2usecs);
#endif

#ifdef TASKSCALAR
  rsesc_OS_write_block(pthread->getPid(),picode->addr,tvVAddr,&tv, sizeof(tv));
#else
  memcpy((void *)(pthread->virt2real(tvVAddr)),&tv,sizeof(tv));
#endif
  pthread->setRetVal(0);
  return pthread->getRetIcode();
}

void rsesc_wait(int32_t pid);

OP(mint_sesc_wait)
{
  // Set things up for the return to from this call
  pthread->setPCIcode(pthread->getRetIcode());
#if !(defined TLS)
  rsesc_wait(pthread->getPid());
#else
  SysCallWait *sysCall=pthread->getEpoch()->newSysCall<SysCallWait>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#endif
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

void rsesc_pseudoreset(int32_t pid);

OP(mint_sesc_pseudoreset)
{
  // Set things up for the return to from this call
  pthread->setPCIcode(pthread->getRetIcode());
  rsesc_pseudoreset(pthread->getPid());
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

/* It's "system calls" all the way down. */

/* ARGSUSED */
OP(mint_ulimit)
{
#if (defined __LP64__)
  fatal("mint_ulimit: Not working yet for __LP64__");
#endif
#ifdef DEBUG_VERBOSE
  printf("mint_ulimit()\n");
#endif
  int32_t cmd      = pthread->getIntArg1();
  int32_t newlimit = pthread->getIntArg2();
  
#ifdef CYGWIN
  //fatal("ulimit not supported in cygwin") ;
  int32_t err=-1;
#else
  int32_t err = ulimit(cmd, newlimit);
#endif
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_execl)
{
#ifdef DEBUG_VERBOSE
    printf("mint_execl()\n");
#endif
    fatal("execl() system call not yet implemented\n");
    return NULL;
}

/* ARGSUSED */
OP(mint_execle)
{
#ifdef DEBUG_VERBOSE
    printf("mint_execle()\n");
#endif
    fatal("execle() system call not yet implemented\n");
    return NULL;
}

/* ARGSUSED */
OP(mint_execlp)
{
#ifdef DEBUG_VERBOSE
    printf("mint_execlp()\n");
#endif
    fatal("execlp() system call not yet implemented\n");
    return NULL;
}

/* ARGSUSED */
OP(mint_execv)
{
#ifdef DEBUG_VERBOSE
    printf("mint_execv()\n");
#endif
    fatal("execv() system call not yet implemented\n");
    return NULL;
}

/* ARGSUSED */
OP(mint_execve)
{
#ifdef DEBUG_VERBOSE
    printf("mint_execve()\n");
#endif
    fatal("execve() system call not yet implemented\n");
    return NULL;
}

/* ARGSUSED */
OP(mint_execvp)
{
#ifdef DEBUG_VERBOSE
    printf("mint_execvp()\n");
#endif
    fatal("execvp() system call not yet implemented\n");
    return NULL;
}

OP(mint_munmap)
{
  ID(Pid_t thePid=pthread->getPid());
  // Get size parameter
  int32_t size=pthread->getIntArg2();
  if(size){
#if (defined TLS)
    tls::Epoch *epoch=tls::Epoch::getEpoch(pthread->getPid());
    I(epoch);
    SysCallMunmap *sysCall=epoch->newSysCall<SysCallMunmap>();
    I(sysCall);
    sysCall->exec(pthread,picode);
#else
    VAddr startVAddr=pthread->getIntArg1();
#ifdef DEBUG_VERBOSE
    printf("mint_unmmap(%d)\n", (int) r4);
#endif
    if(startVAddr)
      mint_free(picode,pthread);
    // Munmap fails only due to page-alignment problems, which we don't have
    pthread->setRetVal(0);
#endif
  }else{
    // Zero-size munmap is not an error
    pthread->setRetVal(0);
  }
  // Return from the call (and check for context switch)
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}


/* ARGSUSED */
OP(mint_mmap)
{
  ID(Pid_t thePid=pthread->getPid());
#if (defined TLS)
  tls::Epoch *epoch=tls::Epoch::getEpoch(pthread->getPid());
  I(epoch);
  SysCallMmap *sysCall=epoch->newSysCall<SysCallMmap>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else
  // Prefered address for mmap
  VAddr addr=pthread->getIntArg1();
  // Size of block to mmap
  size_t size=pthread->getIntArg2();
#ifdef DEBUG_VERBOSE
  printf("mint_mmap(%d)\n", size);
#endif
  
  // Milos: Note that the address given to mmap is only a preference,
  // so mmap can ignore it and does not need to fail when addr!=0
  
  if(addr) {
    /* We can not force to map a specific address */
    fatal("mmap called with start different than zero\n");
  }
  pthread->setIntReg(IntArg1Reg,1);
  mint_calloc(picode,pthread);
  pthread->setIntReg(IntArg1Reg,addr);
  if(pthread->getIntReg(RetValReg)==0){
    pthread->setRetVal(-1);
    pthread->setperrno(errno);
  }
#endif
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}

int32_t conv_flags_to_native(int32_t flags)
{
  int32_t ret = flags & 07;

  if (flags & 0x0008) ret |= O_APPEND;
  /* Ignore O_SYNC Flag because it would be sync (atomic inside mint)
   * if (flags & 0x0010) ret |= O_SYNC;
   * if (flags & 0x1000) ret |= O_ASYNC;
  */
  if (flags & 0x0080) ret |= O_NONBLOCK;
  if (flags & 0x0100) ret |= O_CREAT;
  if (flags & 0x0200) ret |= O_TRUNC;
  if (flags & 0x0400) ret |= O_EXCL;
  if (flags & 0x0800) ret |= O_NOCTTY;

  /* get those from host's bits/fcntl.h */
  if (flags & 0x2000) ret |= 0100000;
  if (flags & 0x8000) ret |= 040000;
  if (flags & 0x10000) ret |= 0200000;
  if (flags & 0x20000) ret |= 0400000;

#if 0
  // Milos: it is perfectly OK for the flag to be zero
  // That means that an existing file is opened for read-only!
  if (flags == 0) ret |= O_CREAT;  /* for some reason, sometimes the flag is zero */
#endif
  
  return ret;
}

/* ARGSUSED */
OP(mint_open)
{
  ID(Pid_t thePid=pthread->getPid());
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallOpen *sysCall=pthread->getEpoch()->newSysCall<SysCallOpen>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else // (defined TLS) not true
  long r4, r5, r6;
  int32_t err;

  VAddr   pathname  = pthread->getIntArg1();
  targInt targFlags = pthread->getIntArg2();
  targInt mode      = pthread->getIntArg3();
    
  int32_t flags = conv_flags_to_native(targFlags);

#ifdef TASKSCALAR
  {
    char cadena[100];

    rsesc_OS_read_string(pthread->getPid(), picode->addr, cadena, pathname, 100);
 
   err = open((const char *) cadena, flags, mode);
    if( err == -1 ) {
      fprintf(stderr,"original flag = %ld, and conv_flag = %ld\n", pthread->getIntArg2(), flags);
      fprintf(stderr,"mint_open(%s,%ld,%ld) (failed) ret=%d\n",(const char *) cadena, flags, mode, err);
    }  


#ifdef DEBUG_VERBOSE
    fprintf(stderr,"mint_open(%s,%ld,%ld) ret=%d\n",(const char *) cadena, r5, r6, err);
#endif
  }
#else
  err = open((const char *)(pthread->virt2real(pathname)), flags, mode);

#ifdef DEBUG_VERBOSE
  fprintf(stderr,"mint_open(%s,%ld,%ld) ret=%d\n",(const char *) pthread->virt2real(pathname), flags, mode, err);
#endif
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  else {
    if (err < MAX_FDNUM)
      pthread->setFD(err, 1);
  }
#endif // End (defined TLS) else block
  // Return from the call (and check for context switch)
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_close)
{
  ID(Pid_t thePid=pthread->getPid());
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallClose *sysCall=pthread->getEpoch()->newSysCall<SysCallClose>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else // Begin (defined TLS) else block
#ifdef DEBUG_VERBOSE
  printf("mint_close()\n");
#endif
  int32_t fd = pthread->getIntArg1();
  /* don't close our file descriptors */
  if (fd < MAX_FDNUM && pthread->getFD(fd)) {
    pthread->setFD(fd,0);
    int32_t err=close(fd);
    pthread->setRetVal(err);
    if (err == -1)
      pthread->setperrno(errno);
  }else{
    pthread->setRetVal(0);
  }
#endif // End (defined TLS) else block
  // Return from the call (and check for context switch)
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_read)
{
  ID(Pid_t thePid=pthread->getPid());
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallRead *sysCall=pthread->getEpoch()->newSysCall<SysCallRead>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else // Begin (defined TLS) else block
    int32_t r4, r5, r6;
    int32_t err;

    r4 = pthread->getIntArg1();
    VAddr buf = pthread->getIntArg2();
    r6 = pthread->getIntArg3();

#ifdef DEBUG_VERBOSE
    printf("mint_read(%d, 0x%08x, %d), RA=0x%08x\n", (int) r4, (unsigned) r5, 
           (int) r6, (unsigned) pthread->getIntReg(RetAddrReg));
#endif

#ifdef TASKSCALAR
    {
      void *tempbuff = alloca(r6);

      err = read(r4, tempbuff, r6);
      if (err > 0)
        rsesc_OS_write_block(pthread->getPid(), picode->addr, buf, tempbuff, err);
    }
#else
    err = read(r4, (void *)(pthread->virt2real(buf)), r6);
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
#endif // End (defined TLS) else block
  // Return from the call (and check for context switch)
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_write){
  ID(Pid_t thePid=pthread->getPid());
  // Set things up for the return from this call
  pthread->setPCIcode(pthread->getRetIcode());
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallWrite *sysCall=pthread->getEpoch()->newSysCall<SysCallWrite>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else // Begin (defined TLS) else block
  // Arguments of the write call
  MintFuncArgs funcArgs(pthread,picode);
  int32_t    fd    = funcArgs.getInt32();
  VAddr  buf   = funcArgs.getVAddr();
  size_t count = funcArgs.getInt32();

  int32_t err;

  int32_t pid=pthread->getPid();

#ifdef DEBUG_VERBOSE
  printf("mint_write(%d, 0x%08x, %d)\n",(int)fd,(unsigned)buf ,(int)count);
#endif

#if (defined TASKSCALAR) || (defined TLS)
  {
    void *tempbuff=alloca(count);
    rsesc_OS_read_block(pthread->getPid(),picode->addr, tempbuff, buf, count);
    err=write(fd,tempbuff,count);
  }
#else
  err=write(fd,(const void *)(pthread->virt2real(buf)),count);
#endif
  pthread->setRetVal(err);
  if(err==-1)
    pthread->setErrno(errno);
#endif // End (defined TLS) else block
  // Note: not neccessarily running the same thread as before
  return pthread->getPCIcode();
}

/* ARGSUSED */
OP(mint_readv)
{
#if (defined __LP64__)
  fatal("mint_readv: Not working yet for __LP64__");
#endif
    int32_t r4, r5, r6;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_readv()\n");
#endif
    r4 = pthread->getIntArg1();
    r5 = pthread->getIntArg2();
    r6 = pthread->getIntArg3();

    // r5 = pthread->virt2real(r5);

#ifdef TASKSCALAR
    {
      void *tempbuff = alloca(r6);
      
      err = readv(r4, (const iovec *)tempbuff, r6);
      if (err > 0)
        rsesc_OS_write_block(pthread->getPid(), picode->addr, pthread->getIntArg2(), tempbuff, err);
    }
#else
    err = readv(r4, (const iovec *) pthread->virt2real(r5), r6);
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_writev)
{
#if (defined __LP64__)
  fatal("mint_writev: Not working yet for __LP64__");
#endif
         int32_t r4, r5, r6;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_writev()\n");
#endif
    r4 = pthread->getIntArg1();
    r5 = pthread->getIntArg2();
    r6 = pthread->getIntArg3();

    // r5 = pthread->virt2real(r5);

#ifdef TASKSCALAR
    {
      void *tempbuff =  alloca (r6);
      rsesc_OS_read_block(pthread->getPid(), picode->addr, tempbuff, pthread->getIntArg2(), r6);

      err = writev(r4, (const iovec *)tempbuff, r6);
    }
#else
    err = writev(r4, (const iovec *) pthread->virt2real(r5), r6);
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_creat)
{
#if (defined __LP64__)
  fatal("mint_creat: Not working yet for __LP64__");
#endif
    int32_t r4, r5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_creat()\n");
#endif
    r4 = pthread->getIntArg1();
    r5 = pthread->getIntArg2();

    // r4 = pthread->virt2real(r4);

#ifdef TASKSCALAR
    {
      char cadena[100];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cadena, pthread->getIntArg1(), 100);
      
      err = open((const char *) cadena, r5);
    }
#else
    err = creat((const char *) pthread->virt2real(r4), r5);
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_link)
{
#if (defined __LP64__)
  fatal("mint_link: Not working yet for __LP64__");
#endif
    int32_t r4, r5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_link()\n");
#endif
    r4 = pthread->getIntArg1();
    r5 = pthread->getIntArg2();

#ifdef TASKSCALAR
    {
      char cad1[100];
      char cad2[100];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 100);
      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad2, REGNUM(5), 100);
      
      err = link((const char *) cad1, cad2);
    }
#else
    // r4 = pthread->virt2real(r4);
    // r5 = pthread->virt2real(r5);

    err = link((const char *) pthread->virt2real(r4), (const char *) pthread->virt2real(r5));
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_unlink)
{
#if (defined __LP64__)
  fatal("mint_unlink: Not working yet for __LP64__");
#endif
    int32_t r4;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_unlink()\n");
#endif
    r4 = pthread->getIntArg1();

    // r4 = pthread->virt2real(r4);

#ifdef TASKSCALAR
    {
      char cad1[100];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 100);
      
      err = unlink((const char *) cad1);
    }
#else
    err = unlink((const char *) pthread->virt2real(r4));
#endif
    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_rename)
{
#if (defined __LP64__)
  fatal("mint_rename: Not working yet for __LP64__");
#endif
    int32_t r4, r5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_rename()\n");
#endif
    r4 = pthread->getIntArg1();
    r5 = pthread->getIntArg2();

#ifdef TASKSCALAR
    {
      char cad1[100];
      char cad2[100];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 100);
      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad2, REGNUM(5), 100);
      
      err = rename((const char *) cad1, (const char *)cad2);
    }
#else
    // r4 = pthread->virt2real(r4);
    // r5 = pthread->virt2real(r5);

    err = rename((const char *) pthread->virt2real(r4), (const char *) pthread->virt2real(r5));
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_chdir)
{
#if (defined __LP64__)
  fatal("mint_chdir: Not working yet for __LP64__");
#endif
    int32_t r4;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_chdir()\n");
#endif
    r4 = pthread->getIntArg1();

    // r4 = pthread->virt2real(r4);

#ifdef TASKSCALAR
    {
      char cad1[100];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 100);
      
      err = chdir((const char *) cad1);
    }
#else
    err = chdir((const char *) pthread->virt2real(r4));
#endif
    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_chmod)
{
#if (defined __LP64__)
  fatal("mint_chmod: Not working yet for __LP64__");
#endif
    int32_t r4, r5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_chmod()\n");
#endif
    r4 = pthread->getIntArg1();
    r5 = pthread->getIntArg2();

    // r4 = pthread->virt2real(r4);

#ifdef TASKSCALAR
    {
      char cad1[100];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 100);
      
      err = chmod(cad1, r5);
    }
#else
    err = chmod((char *) pthread->virt2real(r4), r5);
#endif
    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_fchmod)
{
#if (defined __LP64__)
  fatal("mint_fchmod: Not working yet for __LP64__");
#endif
    int32_t r4, r5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_fchmod()\n");
#endif
    r4 = pthread->getIntArg1();
    r5 = pthread->getIntArg2();

    err = fchmod(r4, r5);
    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_chown)
{
#if (defined __LP64__)
  fatal("mint_chown: Not working yet for __LP64__");
#endif
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_chown()\n");
#endif
    int32_t pathVAddr = pthread->getIntArg1();
    int32_t ownerUID  = pthread->getIntArg2();
    int32_t groupGID  = pthread->getIntArg3();

#ifdef TASKSCALAR
    {
      char cad1[100];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, pathVAddr, 100);
      
      err = chown((const char *) cad1, ownerUID, groupGID);
    }
#else
    err = chown((const char *) pthread->virt2real(pathVAddr), ownerUID, groupGID);
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_fchown)
{
#if (defined __LP64__)
  fatal("mint_fchown: Not working yet for __LP64__");
#endif
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_chown()\n");
#endif
    int32_t fd       = pthread->getIntArg1();
    int32_t ownerUID = pthread->getIntArg2();
    int32_t groupGID = pthread->getIntArg3();

    err = fchown(fd, ownerUID, groupGID);
    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_lseek64)
{
  // Prepare for parameter extraction
  MintFuncArgs funcArgs(pthread, picode);
  // First parameter is a 32-bit int
  int32_t fildes = funcArgs.getInt32();
  // Second parameter is a 64-bit int
  off_t offset = funcArgs.getInt64();
  // Third parameter is a 32-bit int
  int32_t whence = funcArgs.getInt32();
  // Now do the actual call with these parameters
  off_t retVal = lseek(fildes,offset,whence);
#ifdef DEBUG_VERBOSE
  printf("mint_lseek64(%d,%d,%d)=%d\n",fildes,offset,whence,retVal);
#endif
  // Return value is a 64-bit number
  pthread->setRetVal64(retVal);
  if(retVal == (off_t)-1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_lseek)
{
  int32_t r4 = REGNUM(4);
  int32_t r5 = REGNUM(5);
  int32_t r6 = REGNUM(6);
  int32_t err;

#ifdef DEBUG_VERBOSE
  printf("mint_lseek()\n");
#endif

  err = (int) lseek((int) r4, (off64_t) r5, (int) r6);
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);

  return pthread->getRetIcode();
}


/* ARGSUSED */
OP(mint_access)
{
  MintFuncArgs funcArgs(pthread, picode);
  VAddr namePtr = funcArgs.getVAddr();
  int32_t   amode   = funcArgs.getInt32();
  int32_t err;
  
#ifdef DEBUG_VERBOSE
  printf("mint_access()\n");
#endif
  
#if (defined TLS) || (defined TASKSCALAR)
  {
    char tmpBuf[2048];
    rsesc_OS_read_string(pthread->getPid(), picode->addr,
			 tmpBuf, namePtr, sizeof(tmpBuf));
    err = access(tmpBuf,amode);
  }
#else
  err = access((const char *)(pthread->virt2real(namePtr)),amode);
#endif
  
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_stat)
{
  MintFuncArgs funcArgs(pthread, picode);
  VAddr namePtr = funcArgs.getVAddr();
  VAddr statPtr = funcArgs.getVAddr();
  int32_t  err;

#ifdef DEBUG_VERBOSE
  printf("mint_stat()\n");
#endif
  if(namePtr == 0 || statPtr == 0) {
    fatal("stat called with a null pointer\n");
  }

#if (defined TLS) || (defined TASKSCALAR)
  {
    struct glibc_stat32 stat32p;
    struct stat stat_native;
    char tmpBuf[2048];
      
    rsesc_OS_read_string(pthread->getPid(), picode->addr,
			 tmpBuf, namePtr, sizeof(tmpBuf));

    err = stat(tmpBuf, &stat_native);
    conv_stat32_native2glibc(&stat_native, &stat32p);
    
    rsesc_OS_write_block(pthread->getPid(), picode->addr, statPtr,
                         &stat32p, sizeof(struct glibc_stat32));
  }
#else
  {
    struct glibc_stat32 *stat32p = (struct glibc_stat32 *)(pthread->virt2real(statPtr));
    struct stat stat_native;
      
    err = stat((const char *)pthread->virt2real(namePtr), &stat_native);
      
    conv_stat32_native2glibc(&stat_native, stat32p);
  }
#endif

  pthread->setRetVal(err);
  if (err == -1)
      pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_lstat)
{
  MintFuncArgs funcArgs(pthread, picode);
  VAddr namePtr = funcArgs.getVAddr();
  VAddr statPtr = funcArgs.getVAddr();
  int32_t  err;

#ifdef DEBUG_VERBOSE
  printf("mint_stat()\n");
#endif
  if(namePtr == 0 || statPtr == 0) {
    fatal("stat called with a null pointer\n");
  }

#if (defined TLS) || (defined TASKSCALAR)
  {
    struct glibc_stat32 stat32p;
    struct stat stat_native;
    char tmpbuff[2048];
      
    rsesc_OS_read_string(pthread->getPid(), picode->addr, tmpbuff, namePtr, 2048);

    err = lstat(tmpbuff, &stat_native);
    conv_stat32_native2glibc(&stat_native, &stat32p);

    rsesc_OS_write_block(pthread->getPid(), picode->addr, 
                         statPtr,
                         &stat32p, sizeof(struct glibc_stat32));
  }
#else
  {
    struct glibc_stat32 *stat32p = (struct glibc_stat32 *)(pthread->virt2real(statPtr));
    struct stat stat_native;
      
    err = lstat((const char *)pthread->virt2real(namePtr), &stat_native);
      
    conv_stat32_native2glibc(&stat_native, stat32p);
  }
#endif

    pthread->setRetVal(err);
    if (err == -1)
        pthread->setperrno(errno);
    return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_fstat)
{
  targInt r4 = REGNUM(4);
  VAddr   r5 = REGNUM(5);
  int32_t  err;

#ifdef DEBUG_VERBOSE
  printf("mint_fstat(%ld, 0x%lx)\n", r4, r5);
#endif
  if(r5 == 0) {
    fatal("fstat called with a null pointer (ip=0x%x)\n", picode->addr);
  }

#if (defined TLS) || (defined TASKSCALAR)
  {
    struct glibc_stat64 stat64p;
    struct stat stat_native;
    char tmpbuff[2048];
      
    err = fstat(r4, &stat_native);
    conv_stat64_native2glibc(&stat_native, &stat64p);

    rsesc_OS_write_block(pthread->getPid(), picode->addr, r5, &stat64p, sizeof(struct glibc_stat64));
  }
#else
  {
    struct glibc_stat64 *stat64p = (struct glibc_stat64 *)(pthread->virt2real(r5));
    struct stat stat_native;
      
    err = fstat(r4, &stat_native);
      
    conv_stat64_native2glibc(&stat_native, stat64p);
  }
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_fstat64)
{
  return mint_fstat(picode, pthread);
}

/* ARGSUSED */
OP(mint_xstat)
{
  int32_t r4, r5, r6;

  r4 = REGNUM(4);
  r5 = REGNUM(5);
  r6 = REGNUM(6);

#ifdef DEBUG_VERBOSE
  printf("mint_xstat(0x%08lx,0x%08lx,0x%08lx)\n", r4, r5, r6);
#endif

  /* calling fstat ignoring the _stat_ver in glibc */
  REGNUM(4) = REGNUM(5);
  REGNUM(5) = REGNUM(6);
  return mint_stat(picode,pthread);
}

OP(mint_xstat64)
{
  int32_t r4, r5, r6;

  r4 = REGNUM(4);
  r5 = REGNUM(5);
  r6 = REGNUM(6);

#ifdef DEBUG_VERBOSE
  printf("mint_xstat64(0x%08lx,0x%08lx,0x%08lx)\n", r4, r5, r6);
#endif

  /* calling fstat ignoring the _stat_ver in glibc */
  REGNUM(4) = REGNUM(5);
  REGNUM(5) = REGNUM(6);
  return mint_stat(picode, pthread);
}

OP(mint_fxstat64)
{
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallFileIO::execFXStat64(pthread,picode);
#else // Begin (defined TLS) else block
  MintFuncArgs funcArgs(pthread, picode);
  /* We ignore the glibc stat_ver parameter (parameter 1) */
  targInt    statVer=funcArgs.getInt32();
  targInt    fd     =funcArgs.getInt32();
  VAddr      addr   =funcArgs.getVAddr();
  int32_t   retVal;
#ifdef DEBUG_VERBOSE
  printf("mint_fxstat64(0x%08lx,0x%08lx,0x%08lx)\n", statVer, fd, addr);
#endif
  if(addr==0)
    fatal("fstat called with a null pointer (ip=0x%x)\n", picode->addr);
  struct stat stat_native;
  retVal=fstat(fd,&stat_native);
  pthread->setRetVal(retVal);
  if(retVal==-1)
    pthread->setErrno(errno);
  struct glibc_stat64 stat64p;     
  conv_stat64_native2glibc(&stat_native, &stat64p);
#if (defined TLS) || (defined TASKSCALAR)
  rsesc_OS_write_block(pthread->getPid(),picode->addr, addr, &stat64p,sizeof(struct glibc_stat64));
#else
  memcpy((struct glibc_stat64 *)(pthread->virt2real(addr)),&stat64p,sizeof(struct glibc_stat64));
#endif
#endif // End (defined TLS) else block
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_lstat64)
{
  int32_t r4 = REGNUM(4);
  int32_t r5 = REGNUM(5);
  int32_t  err;

#ifdef DEBUG_VERBOSE
  printf("mint_lstat64()\n");
#endif
  if(r4 == 0 || r5 == 0) {
    fatal("lstat64 called with a null pointer\n");
  }

#if (defined TLS) || (defined TASKSCALAR)
  {
    struct glibc_stat64 stat64p;
    struct stat stat_native;
    char tmpbuff[2048];
      
    rsesc_OS_read_string(pthread->getPid(), picode->addr, tmpbuff, r4, 2048);

    err = lstat(tmpbuff, &stat_native);
    conv_stat64_native2glibc(&stat_native, &stat64p);

    rsesc_OS_write_block(pthread->getPid(), picode->addr, r5, &stat64p, sizeof(struct glibc_stat64));
  }
#else
  {
    struct glibc_stat64 *stat64p = (struct glibc_stat64 *)pthread->virt2real(r5);
    struct stat stat_native;
      
    err = lstat((const char *)pthread->virt2real(r4), &stat_native);
      
    conv_stat64_native2glibc(&stat_native, stat64p);
  }
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_stat64)
{
  VAddr r4 = REGNUM(4);
  VAddr r5 = REGNUM(5);
  int32_t  err;
  
#ifdef DEBUG_VERBOSE
  printf("mint_stat64()\n");
#endif
  if(r4 == 0 || r5 == 0) {
    fatal("stat64 called with a null pointer\n");
  }

#if (defined TLS) || (defined TASKSCALAR)
  {
    struct glibc_stat64 stat64p;
    struct stat stat_native;
    char tmpbuff[2048];
      
    rsesc_OS_read_string(pthread->getPid(), picode->addr, tmpbuff, r4, 2048);

    err = stat(tmpbuff, &stat_native);
    conv_stat64_native2glibc(&stat_native, &stat64p);

    rsesc_OS_write_block(pthread->getPid(), picode->addr, r5, &stat64p, sizeof(struct glibc_stat64));
  }
#else
  {
    struct glibc_stat64 *stat64p = (struct glibc_stat64 *)(pthread->virt2real(r5));
    struct stat stat_native;
      
    err = stat((const char *)(pthread->virt2real(r4)), &stat_native);
      
    conv_stat64_native2glibc(&stat_native, stat64p);
  }
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_dup)
{
         int32_t r4;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_dup()\n");
#endif
    r4 = REGNUM(4);

    err = dup(r4);
    REGNUM(2) = err;
    if (err == -1)
        pthread->setperrno(errno);
    else {
        if (err < MAX_FDNUM)
          pthread->setFD(err, 1);
    }
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_getcwd)
{
#if (defined __LP64__)
  fatal("mint_getcwd: Not working yet for __LP64__");
#endif
  int32_t r4 = REGNUM(4);
  int32_t r5 = REGNUM(5);
  RAddr err;
  
#ifdef DEBUG_VERBOSE
  printf("mint_getcwd()");
#endif

  if(r4 == 0) {
    fatal("getcwd called with a null pointer\n");
  }

#if (defined TLS) || (defined TASKSCALAR)
  {
    char tmpbuf[4096];
    if (r5 >= 4096)
      fatal("getcwd(): please increase tmpbuf to %ld\n", r5);
    err = (RAddr)getcwd(tmpbuf, (size_t)r5);
    rsesc_OS_write_block(pthread->getPid(), picode->addr, REGNUM(4), (const void *)tmpbuf, r5);
  }
#else
  {
    err = (RAddr)getcwd((char *)pthread->virt2real(r4), (size_t)r5);
  }
#endif  

  if(err == 0) {
    pthread->setperrno(errno);
    REGNUM(2) = 0;
  }else{
    REGNUM(2) = pthread->real2virt(err);
  }
  

#ifdef DEBUG_VERBOSE
  printf("=%s\n", (char *)pthread->virt2real(REGNUM(2)));
#endif

  return addr2icode(REGNUM(31));
}


/* ARGSUSED */
OP(mint_pipe)
{
#if (defined __LP64__)
  fatal("mint_pipe: Not working yet with __LP64__");
#endif
         int32_t r4;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_pipe()\n");
#endif
    r4 = REGNUM(4);

    // r4 = pthread->virt2real(r4);

#ifdef TASKSCALAR
    {
      char cad1[100];

      rsesc_OS_read_block(pthread->getPid(), picode->addr, cad1, REGNUM(4), 2*4);

      err = pipe((int32_t *) cad1);
    }
#else
    err = pipe((int32_t *) pthread->virt2real(r4));
#endif
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_symlink)
{
    VAddr r4, r5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_symlink()\n");
#endif
    r4 = REGNUM(4);
    r5 = REGNUM(5);

#ifdef TASKSCALAR
    {
      char cad1[1024];
      char cad2[1024];

      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 1024);
      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad2, REGNUM(5), 1024);

      err = symlink((const char *) cad1, (const char *) cad2);
    }
#else
    err = symlink((const char *)(pthread->virt2real(r4)),(const char *)(pthread->virt2real(r5)));
#endif
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_readlink)
{
  IntRegValue retVal;
  IntRegValue errVal;
#ifdef DEBUG_VERBOSE
  printf("mint_readlink()\n");
#endif
  MintFuncArgs funcArgs(pthread, picode);
  VAddr  pathPtr=funcArgs.getVAddr();
  VAddr  bufPtr =funcArgs.getVAddr();
  size_t bufSiz =funcArgs.getInt32();

  // Check if buffer is valid before doing a real system call
  if((!pthread->isValidDataVAddr(bufPtr))||
     (!pthread->isValidDataVAddr(bufPtr+bufSiz-1))){
    pthread->setRetVal(-1);
    pthread->setErrno(EFAULT);
    return pthread->getRetIcode();
  }

#ifdef TASKSCALAR
  {
    char cad1[1024];
    char *cad2 = (char *)alloca(bufSiz);
    
    rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 1024);
    
    retVal=readlink(cad1, cad2, bufSiz);
    errVal=errno;

    if(bufPtr)
      rsesc_OS_write_block(pthread->getPid(), picode->addr, REGNUM(5), cad2,bufSiz);
  }
#else
  retVal=readlink((const char *)(pthread->virt2real(pathPtr)),
		  (char *)(pthread->virt2real(bufPtr)),bufSiz);
  errVal=errno;
#endif
  pthread->setRetVal(retVal);
  if(retVal==-1)
    pthread->setErrno(errVal);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_umask)
{
         int32_t r4;

#ifdef DEBUG_VERBOSE
    printf("mint_umask()\n");
#endif
    r4 = REGNUM(4);

         REGNUM(2) = (int) umask((mode_t)r4);
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_getuid)
{
#ifdef DEBUG_VERBOSE
    printf("mint_getuid()\n");
#endif

         REGNUM(2) = (int) getuid();
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_geteuid)
{
#ifdef DEBUG_VERBOSE
    printf("mint_geteuid()\n");
#endif

         REGNUM(2) = (int) geteuid();
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_getgid)
{
#ifdef DEBUG_VERBOSE
    printf("mint_getgid()\n");
#endif

         REGNUM(2) = (int) getgid();
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_getegid)
{
#ifdef DEBUG_VERBOSE
    printf("mint_getegid()\n");
#endif

         REGNUM(2) = (int) getegid();
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_getdomainname)
{
#if (defined __LP64__)
  fatal("mint_getdomainname: Not working yet with __LP64__");
#endif
  int32_t r4, r5;
  int32_t err;

#ifdef DEBUG_VERBOSE
  printf("mint_getdomainname()\n");
#endif

  r4 = REGNUM(4);
  r5 = REGNUM(5);

#if (defined SUNOS) || (defined AIX)
  fatal("getdomainname not supported\n");
  err = 0;
#else
#ifdef TASKSCALAR
  {
    char *cad1 = (char *)alloca(r5);
      
    err = getdomainname(cad1, r5);

    if (r4)
      rsesc_OS_write_block(pthread->getPid(), picode->addr, REGNUM(4), cad1, r5);
  }
#else
  // if (r4)
  //     r4 = pthread->virt2real(r4);

  err = getdomainname((char *) pthread->virt2real(r4), r5);
#endif
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_setdomainname)
{
#if (defined __LP64__)
  fatal("mint_setdomainname: Not working yet with __LP64__");
#endif
  int32_t r4, r5;
  int32_t err;

#ifdef DEBUG_VERBOSE
  printf("mint_setdomainname()\n");
#endif

  r4 = REGNUM(4);
  r5 = REGNUM(5);

#if (defined SUNOS) || (defined AIX) || (defined CYGWIN) 
  fatal("setdomainname not supported\n");
  err = 0;
#else
#ifdef TASKSCALAR
  {
    char *cad1 = (char *)alloca(r5);
      
    if (r4)
      rsesc_OS_read_block(pthread->getPid(), picode->addr, cad1, REGNUM(4), r5);
    else
      cad1 = 0;
#ifndef CYGWIN
    err = setdomainname(cad1, r5);
#else
    fatal("cygwin does not support setdomainname") ;
#endif
  }
#else
  // if (r4)
  //     r4 = pthread->virt2real(r4);
#ifndef CYGWIN
  err = setdomainname((char *) pthread->virt2real(r4), r5);
#else
  fatal("cygwin does not support setdomainname") ;
#endif

#endif
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_gethostname)
{
#if (defined __LP64__)
  fatal("mint_gethostname: Not working yet with __LP64__");
#endif
         int32_t r4, r5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_gethostname()\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);

#ifdef TASKSCALAR
    {
      char *cad1 = (char *)alloca(r5);
      
      err = gethostname(cad1, r5);

      if (r4)
        rsesc_OS_write_block(pthread->getPid(), picode->addr, REGNUM(4), cad1, r5);
    }
#else
    // if (r4)
    //     r4 = pthread->virt2real(r4);

    err = gethostname((char *) pthread->virt2real(r4), r5);
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_socket)
{
#if (defined __LP64__)
  fatal("mint_socket: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_socket()\n");
#endif
    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);

    err = socket(r4, r5, r6);
    REGNUM(2) = err;
    if (err == -1)
        pthread->setperrno(errno);
    else {
        if (err < MAX_FDNUM)
          pthread->setFD(err, 1);
    }
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_connect)
{
#if (defined __LP64__)
  fatal("mint_connect: Not working yet with __LP64__");
#endif
  int32_t r4, r5, r6;
  int32_t err;

#ifdef DEBUG_VERBOSE
  printf("mint_connect()\n");
#endif
  r4 = REGNUM(4);
  r5 = REGNUM(5);
  r6 = REGNUM(6);

#ifdef TASKSCALAR
  fatal("connect not implemented with TLS\n");
#endif
  // if (r5)
  //   r5 = pthread->virt2real(r5);

#ifdef sparc
  err = connect(r4, (struct sockaddr *) pthread->virt2real(r5), r6);
#else
  err = connect(r4, (const struct sockaddr *) pthread->virt2real(r5), r6);
#endif
  REGNUM(2) = err;
  if (err == -1)
    pthread->setperrno(errno);
  else {
    if (err < MAX_FDNUM)
      pthread->setFD(err, 1);
  }
  return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_send)
{
#if (defined __LP64__)
  fatal("mint_send: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6, r7;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_send()\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);
    r7 = REGNUM(7);

#ifdef TASKSCALAR
    {
      char *cad1 = (char *)alloca(r6);
      
      if (r5)
        rsesc_OS_read_block(pthread->getPid(), picode->addr, cad1, REGNUM(5), r6);
      else
        cad1 = 0;

      err = send(r4, (const void *) cad1, r6, r7);
    }
#else
    // if (r5)
    //     r5 = pthread->virt2real(r5);

    err = send(r4, (const void *) pthread->virt2real(r5), r6, r7);
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_sendto)
{
#if (defined __LP64__)
  fatal("mint_sendto: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6, r7;
         int32_t *sp, arg5, arg6;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_sendto()\n");
#endif

#ifdef TASKSCALAR
    fatal("sendto not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);
    r7 = REGNUM(7);
         sp = (int32_t *) pthread->virt2real(REGNUM(29));
    arg5=SWAP_WORD(sp[4]);
    arg6=SWAP_WORD(sp[5]);

    // if (r5)
    //     r5 = pthread->virt2real(r5);
    // if (arg5)
    //     arg5 = pthread->virt2real(arg5);

  err = sendto(r4, (const void *) pthread->virt2real(r5), r6, r7, (struct sockaddr *) pthread->virt2real(arg5), arg6);
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_sendmsg)
{
#if (defined __LP64__)
  fatal("mint_sendmsg: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6;
    int32_t err;
    uint32_t i;
    struct msghdr *msg;
    struct iovec *iovp;

#ifdef DEBUG_VERBOSE
    printf("mint_sendmsg()\n");
#endif

#ifdef TASKSCALAR
    fatal("sendmsg not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);

    // if (r5) {
        // r5 = pthread->virt2real(r5);
        // msg = (struct msghdr *) pthread->virt2real(r5);
    //     if (msg->msg_name)
          // msg->msg_name = (char *) pthread->real2virt((VAddr) msg->msg_name);
        // iovp = msg->msg_iov;
        // if (iovp) {
                                // iovp = (struct iovec *) pthread->virt2real(iovp);
        //     msg->msg_iov = iovp;
        //     for (i = 0; i < msg->msg_iovlen; i++, iovp++)
        //         if (iovp->iov_base)
                                //                iovp->iov_base = (char *) pthread->virt2real((VAddr) iovp->iov_base);
        // }
// #ifdef IRIX64
//      /* This is BSB4.3 neither implemented in Linux nor AIX */
//         if (msg->msg_accrights)
//             msg->msg_accrights = (char *) pthread->virt2real(msg->msg_accrights);
// #endif
    // }

#ifdef sparc
    err = sendmsg(r4, (struct msghdr *) pthread->virt2real(r5), r6);
#else
    err = sendmsg(r4, (const struct msghdr *) pthread->virt2real(r5), r6);
#endif

    // if (r5) {
    //     msg = (struct msghdr *) pthread->virt2real(r5);
    //     if (msg->msg_name)
          // msg->msg_name = (char *) pthread->real2virt((VAddr) msg->msg_name);
        // iovp = msg->msg_iov;
//         if (iovp) {
//             for (i = 0; i < msg->msg_iovlen; i++, iovp++)
//                 if (iovp->iov_base)
//                iovp->iov_base = (char *) pthread->real2virt((VAddr) iovp->iov_base);
//                              msg->msg_iov = (struct iovec *) pthread->real2virt((VAddr) msg->msg_iov);
//         }
// #ifdef IRIX64
//      /* This is BSB4.3 neither implemented in Linux nor AIX */
//         if (msg->msg_accrights)
//             msg->msg_accrights = (char *) pthread->real2virt(msg->msg_accrights);
// #endif
    // }

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_recv)
{
#if (defined __LP64__)
  fatal("mint_recv: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6, r7;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_recv()\n");
#endif

#ifdef TASKSCALAR
    fatal("recv not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);
    r7 = REGNUM(7);

    // if (r5)
    //     r5 = pthread->virt2real(r5);

  err = recv(r4, (void *) pthread->virt2real(r5), r6, r7);
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_recvfrom)
{
#if (defined __LP64__)
  fatal("mint_recvfrom: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6, r7;
         int32_t *sp, arg5, arg6;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_recvfrom()\n");
#endif

#ifdef TASKSCALAR
    fatal("recvfrom not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);
    r7 = REGNUM(7);
         sp = (int32_t *) pthread->virt2real(REGNUM(29));
    arg5 = SWAP_WORD(sp[4]);
    arg6 = SWAP_WORD(sp[5]);

    // if (r5)
    //     r5 = pthread->virt2real(r5);
    // if (arg5)
    //     arg5 = pthread->virt2real(arg5);
    // if (arg6)
    //     arg6 = pthread->virt2real(arg6);

  err = recvfrom(r4, (void *) pthread->virt2real(r5), r6, r7, (struct sockaddr *) pthread->virt2real(arg5), (socklen_t *) pthread->virt2real(arg6));
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_recvmsg)
{
#if (defined __LP64__)
  fatal("mint_recvmsg: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6;
    int32_t err;
    uint32_t i;
    struct msghdr *msg;
    struct iovec *iovp;

#ifdef DEBUG_VERBOSE
    printf("mint_recvmsg()\n");
#endif

#ifdef TASKSCALAR
    fatal("recvmsg not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);

    // if (r5) {
        // r5 = pthread->virt2real(r5);
        // msg = (struct msghdr *) pthread->virt2real(r5);
        // if (msg->msg_name)
                                // msg->msg_name = (char *) pthread->virt2real((VAddr) msg->msg_name);
        // iovp = msg->msg_iov;
        // if (iovp) {
                                // iovp = (struct iovec *) pthread->virt2real((VAddr) iovp);
        //     msg->msg_iov = iovp;
        //     for (i = 0; i < msg->msg_iovlen; i++, iovp++)
        //         if (iovp->iov_base)
                                //                iovp->iov_base = (char *) pthread->virt2real((VAddr) iovp->iov_base);
        // }
// #ifdef IRIX64
//      /* This is BSB4.3 neither implemented in Linux nor AIX */
//         if (msg->msg_accrights)
//             msg->msg_accrights = (char *) pthread->virt2real(msg->msg_accrights);
// #endif
    // }

    err = recvmsg(r4, (struct msghdr *) pthread->virt2real(r5), r6);
    
    // if (r5) {
        // msg = (struct msghdr *) pthread->virt2real(r5);
        // if (msg->msg_name)
                                // msg->msg_name = (char *) pthread->real2virt((VAddr) msg->msg_name);
        // iovp = msg->msg_iov;
        // if (iovp) {
        //     for (i = 0; i < msg->msg_iovlen; i++, iovp++)
        //         if (iovp->iov_base)
                                //                iovp->iov_base = (char *) pthread->real2virt((VAddr) iovp->iov_base);
                                // msg->msg_iov = (struct iovec *) pthread->real2virt((VAddr) msg->msg_iov);
        // }
// #ifdef IRIX64
//      /* This is BSB4.3 neither implemented in Linux nor AIX */
//         if (msg->msg_accrights)
//             msg->msg_accrights = (char *) pthread->real2virt(msg->msg_accrights);
// #endif
    // }

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_select)
{
#if (defined __LP64__)
  fatal("mint_select: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6, r7;
         int32_t *sp, arg5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_select()\n");
#endif

#ifdef TASKSCALAR
    fatal("select not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);
    r7 = REGNUM(7);
         sp = (int32_t *) pthread->virt2real(REGNUM(29));
    arg5 = SWAP_WORD(sp[4]);

    // if (r5)
    //     r5 = pthread->virt2real(r5);
    // if (r6)
    //     r6 = pthread->virt2real(r6);
    // if (r7)
    //     r7 = pthread->virt2real(r7);
    // if (arg5)
    //     arg5 = pthread->virt2real(arg5);

  err = select(r4, (fd_set *) pthread->virt2real(r5), (fd_set *) pthread->virt2real(r6), (fd_set *) pthread->virt2real(r7), (struct timeval *) pthread->virt2real(arg5));

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_getsockopt)
{
#if (defined __LP64__)
  fatal("mint_getsockopt: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6, r7;
         int32_t *sp, arg5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_getsockopt()\n");
#endif

#ifdef TASKSCALAR
    fatal("getsockopt not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);
    r7 = REGNUM(7);
         sp = (int32_t *) pthread->virt2real(REGNUM(29));
    arg5 = SWAP_WORD(sp[4]);

    // if (r7)
    //     r7 = pthread->virt2real(r7);
    // if (arg5)
    //     arg5 = pthread->virt2real(arg5);

  err = getsockopt(r4, r5, r6, (void *) pthread->virt2real(r7), (socklen_t *) pthread->virt2real(arg5));
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_setsockopt)
{
#if (defined __LP64__)
  fatal("mint_setsockopt: Not working yet with __LP64__");
#endif
         int32_t r4, r5, r6, r7;
         int32_t *sp, arg5;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_setsockopt()\n");
#endif

#ifdef TASKSCALAR
    fatal("setsockopt not implemented with TLS\n");
#endif

    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);
    r7 = REGNUM(7);
         sp = (int32_t *) pthread->virt2real(REGNUM(29));
    arg5 = SWAP_WORD(sp[4]);

    // if (r7)
    //     r7 = pthread->virt2real(r7);

    err = setsockopt(r4, r5, r6, (const void *) pthread->virt2real(r7), arg5);
    REGNUM(2) = err;
    if (err == -1)
        pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_oserror)
{
#if (defined __LP64__)
  fatal("mint_oserror: Not working yet with __LP64__");
#endif
#ifdef DEBUG_VERBOSE
  printf("mint_oserror()\n");
#endif

  REGNUM(2) = pthread->getperrno();
  return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_setoserror)
{
#if (defined __LP64__)
  fatal("mint_setoserror: Not working yet with __LP64__");
#endif
         int32_t r4;

#ifdef DEBUG_VERBOSE
    printf("mint_oserror()\n");
#endif
    r4 = REGNUM(4);

    REGNUM(2) = pthread->getperrno();
    pthread->setperrno(r4);
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_perror)
{
#if (defined __LP64__)
  fatal("mint_perror: Not working yet with __LP64__");
#endif
         int32_t r4;

#ifdef DEBUG_VERBOSE
    printf("mint_oserror()\n");
#endif

    r4 = REGNUM(4);
    // r4 = pthread->virt2real(r4);

    errno = pthread->getperrno();
#ifdef TASKSCALAR
    {
      char cad1[100];
      
      rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, REGNUM(4), 100);

      perror((char *) cad1);
    }
#else
    perror((char *) pthread->virt2real(r4));
#endif

    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_times) {

  ID(Pid_t thePid=pthread->getPid());
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallTimes *sysCall=pthread->getEpoch()->newSysCall<SysCallTimes>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else // Begin (defined TLS) else block
    int32_t err;

    {
      char *cad1 = (char *)alloca(sizeof(struct tms));
      
                err = (int) times((struct tms *)cad1);

#ifdef TASKSCALAR
      rsesc_OS_write_block(pthread->getPid(), picode->addr, REGNUM(4), cad1 , sizeof(struct tms));
#else
    struct tms *t = (struct tms *)pthread->virt2real(REGNUM(4));
    // FIXME: convert from 64 to 32bit
    err = 0;
#endif
    }

#ifdef DEBUG_VERBOSE
    printf("mint_times()\n");
#endif

  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
#endif // End (defined TLS) else block
  // Return from the call (and check for context switch)
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_time)
{
#if (defined __LP64__)
  fatal("mint_time: Not working yet with __LP64__");
#endif
         int32_t r4;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_time()\n");
#endif
    r4 = REGNUM(4);

#ifdef TASKSCALAR
    {
      char *cad1 = (char *)alloca(sizeof(time_t));
      
                err = (int) time((time_t *)cad1);

      if (r4)
        rsesc_OS_write_block(pthread->getPid(), picode->addr, REGNUM(4), cad1, sizeof(time_t));
    }
#else
    // if (r4)
    //     r4 = pthread->virt2real(r4);

         err = (int) time((time_t *) pthread->virt2real(r4));
#endif
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_getdents)
{
#if (defined __LP64__)
  fatal("mint_getdents: Not working yet with __LP64__");
#endif

         int32_t r4, r5, r6;
    int32_t err;

#ifdef DEBUG_VERBOSE
    printf("mint_getdents()\n");
#endif
    r4 = REGNUM(4);
    r5 = REGNUM(5);
    r6 = REGNUM(6);

#ifdef DEBUG_VERBOSE
    //fprintf(stderr,"mint_getdents r4=%d r5=%d r6=%d\n",r4,r5,r6);
#endif

#if (defined LINUX) && (defined __i386__)
#ifdef TASKSCALAR
    {
      char *cad1 = (char *)alloca(r6);      
      err = (int) getdents(r4, (struct dirent *)cad1, r6);  
      rsesc_OS_write_block(pthread->getPid(), picode->addr, REGNUM(5), cad1, err);
    }
#else
    r5 = pthread->virt2real(r5);
    err = (int) getdents(r4, (struct dirent *)r5, r6);
#endif
#else
    err = -1;
    fatal("getdents is only implemented in Linux i386.");
#endif

#if 0
  fprintf(stderr,"mint_getdents %d\n",err);
  
  int32_t tot=0;
  char *p = (char*) pthread->virt2real(REGNUM(5));
  int32_t i=0;
  struct dirent *d = (struct dirent *)p;

  while(tot<err) {
      tot += d->d_reclen;
      fprintf(stderr,"tot=%d i=%d name=%s d_off=%d d_reclen=%d\n",tot,i,(d->d_name),(d->d_off),(d->d_reclen));
      i++;
      d = (struct dirent*) ( (char *)d + (long)d->d_reclen );
  }  
#endif

  pthread->setRetVal(err);
  if (err == -1) 
    pthread->setperrno(errno);
  return pthread->getRetIcode();
}

/* ARGSUSED */
OP(mint_getdtablesize)
{
#if (defined __LP64__)
  fatal("mint_getdtablesize: Not working yet with __LP64__");
#endif
#ifdef DEBUG_VERBOSE
    printf("mint_getdtablesize()\n");
#endif
    REGNUM(2) = MAX_FDNUM; 
    return addr2icode(REGNUM(31));
}

/* ARGSUSED */
OP(mint_syssgi)
{
#if (defined __LP64__)
  fatal("mint_syssgi: Not working yet with __LP64__");
#endif
  int32_t r4, r5, r6;

#ifdef DEBUG_VERBOSE
  printf("mint_syssgi()\n");
#endif
  r4 = REGNUM(4);
  switch(r4) {
  case SGI_SYSID:
  case SGI_RDNAME:
    /* case SGI_PROCSZ: */
  case SGI_TUNE:
  case SGI_IDBG:
  case SGI_SETNVRAM:
  case SGI_GETNVRAM:
    fatal("mint_syssgi: command 0x%x not supported yet.\n", r4);
    break;
  case SGI_RUSAGE:
  case SGI_SSYNC:
  case SGI_BDFLUSHCNT:
  case SGI_QUERY_CYCLECNTR:
  case SGI_SPROFIL:
  case SGI_SETTIMETRIM:
  case SGI_GETTIMETRIM:
    fatal("mint_syssgi: command 0x%x not supported yet.\n", r4);
    break;
  case 103:
    /* What does arg 103 do? It occurs in startup code on some SGI
     * machines. For now, just do nothing.
     */
    REGNUM(2) = 0;
    break;
  case SGI_INVENT:
    r5 = REGNUM(5);
    if (r5 == SGI_INV_SIZEOF)
      /* return a non-zero size */
      REGNUM(2) = 20;
    else if (r5 == SGI_INV_READ) {
      /* for now, just do nothing */
      r6 = REGNUM(6);
      if (r6)
        r6 = pthread->virt2real(r6);
      REGNUM(2) = 0;
    } else
      fatal("mint_syssgi: SGI_INVENT: arg 0x%x not expected.\n", r5);
    break;
  case SGI_USE_FP_BCOPY:
    REGNUM(2) = 0;
    break;
  default:
    fatal("mint_syssgi: command 0x%x not expected.\n", r4);
    break;
  }
  return addr2icode(REGNUM(31));
}

OP(mint_sesc_get_num_cpus)
{
  // Do the actual call (should not context-switch)
  ID(Pid_t thePid=pthread->getPid());
  IntRegValue retVal=rsesc_get_num_cpus();
  I(pthread->getPid()==thePid);
  // Set the return value
  pthread->setIntReg(RetValReg,retVal);
  // Return from the call
  return pthread->getRetIcode();
}

OP(mint_do_nothing)
{
  // Set the return value
  pthread->setIntReg(RetValReg,0);
  // Just return from the call
  return pthread->getRetIcode();
}

// Heap related functions

OP(mint_malloc)
{
  // This call should not context-switch
  ID(Pid_t thePid=pthread->getPid());
  // Get size parameter
  size_t size=pthread->getIntArg1();
  if(!size) size++;
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallMalloc *sysCall=pthread->getEpoch()->newSysCall<SysCallMalloc>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else // Begin (defined TLS) else block
  VAddr addr=pthread->getHeapManager()->allocate(size);
  // Set errno on error to be POSIX compliant
  if(!addr)
    pthread->setErrno(ENOMEM);
  pthread->setIntReg(RetValReg,addr);
#endif
  // Return from the call (and check for context switch)
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}

OP(mint_calloc)
{
  // This call should not context-switch
  ID(Pid_t thePid=pthread->getPid());
  // Get parameters
  size_t nmemb=pthread->getIntReg(IntArg1Reg);
  size_t size =pthread->getIntReg(IntArg2Reg);
  // The total size of the allocation
  size_t totalSize=nmemb*size;
  if(!totalSize)
    totalSize++;
  pthread->setIntReg(IntArg1Reg,totalSize);
#if (defined TLS)
  I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
  SysCallMalloc *sysCall=pthread->getEpoch()->newSysCall<SysCallMalloc>();
  I(sysCall);
  sysCall->exec(pthread,picode);
#else // Begin (defined TLS) else block
  VAddr addr=pthread->getHeapManager()->allocate(totalSize);
  // Set errno on error to be POSIX compliant
  if(addr)
    pthread->setErrno(ENOMEM);
  pthread->setRetVal(addr);
#endif // End (defined TLS) else block
  // Clear the allocated region
  VAddr blockPtr=pthread->getIntReg(RetValReg);
  if(blockPtr){
    size_t blockSize=nmemb*size;
#if (defined TLS)
    unsigned long long zero=0ull;
    I((blockPtr&(sizeof(zero)-1))==0);
    while(blockSize>=sizeof(zero)){
      rsesc_OS_write_block(pthread->getPid(),picode->addr, blockPtr, &zero,sizeof(zero));
      blockPtr+=sizeof(zero);
      blockSize-=sizeof(zero);
    }
    if(blockSize){
      rsesc_OS_write_block(pthread->getPid(),picode->addr, blockPtr, &zero,blockSize);
    }
#else
    memset((void *)(pthread->virt2real(blockPtr)),0,blockSize);
#endif
  }
  // There should be no context switch
  I(pthread->getPid()==thePid);
  // Return from the call
  return pthread->getRetIcode();
}

OP(mint_free)
{
  // This call should not context-switch
  ID(Pid_t thePid=pthread->getPid());
  // Get address parameter
  VAddr addr=pthread->getIntReg(IntArg1Reg);
  if(addr){
#if (defined TLS)
    I(pthread->getEpoch()&&(pthread->getEpoch()==tls::Epoch::getEpoch(pthread->getPid())));
    SysCallFree *sysCall=pthread->getEpoch()->newSysCall<SysCallFree>();
    I(sysCall);
    sysCall->exec(pthread,picode);
#else
    pthread->getHeapManager()->deallocate(addr);
#endif
  }
  // There should be no context switch
  I(pthread->getPid()==thePid);
  // Return from the call
  return pthread->getRetIcode();
}

OP(mint_realloc)
{
  // This call should not context-switch
  ID(Pid_t thePid=pthread->getPid());
  // Get parameters
  VAddr  oldAddr=pthread->getIntReg(IntArg1Reg);
  size_t newSize=pthread->getIntReg(IntArg2Reg);
  I(oldAddr||newSize);
  if(!oldAddr){
    // Equivalent to malloc
    pthread->setIntReg(IntArg1Reg,newSize);
    return mint_malloc(picode,pthread);
  }else if(newSize==0){
    // Equivalent to free, but returns NULL
    mint_free(picode,pthread);
    pthread->setIntReg(RetValReg,0);
  }else{
    size_t oldSize=pthread->getHeapManager()->deallocate(oldAddr);
    VAddr  newAddr=pthread->getHeapManager()->allocate(oldAddr,newSize);
    if(newAddr!=oldAddr){
#if (defined TLS)
      fatal("mint_realloc: Not working with TLS yet!");
#else
      // Block could not grow in place, must copy old data
      I(newSize>oldSize);
      memmove((void *)(pthread->virt2real(newAddr)),
	      (void *)(pthread->virt2real(oldAddr)),
	      oldSize);
#endif
    }
    pthread->setIntReg(RetValReg,newAddr);
  }
  // There should be no context switch
  I(pthread->getPid()==thePid);
  // Return from the call
  return pthread->getRetIcode();
}


#ifdef SESC_LOCKPROFILE
void rsesc_startlock(int32_t pid);
void rsesc_endlock(int32_t pid);
void rsesc_startlock2(int32_t pid);
void rsesc_endlock2(int32_t pid);

OP(mint_sesc_startlock)
{
  rsesc_startlock(pthread->getPid());
  return addr2icode(REGNUM(31));
}

OP(mint_sesc_endlock)
{
  rsesc_endlock(pthread->getPid());
  return addr2icode(REGNUM(31));
}

OP(mint_sesc_startlock2)
{
  rsesc_startlock2(pthread->getPid());
  return addr2icode(REGNUM(31));
}

OP(mint_sesc_endlock2)
{
  rsesc_endlock2(pthread->getPid());
  return addr2icode(REGNUM(31));
}
#endif

/* ARGSUSED */
OP(mint_rmdir)
{
  ID(Pid_t thePid=pthread->getPid());

#if (defined TLS)
#error TLS not supported
#endif

  int err;

  MintFuncArgs funcArgs(pthread,picode);
  VAddr pathname = funcArgs.getVAddr();

#ifdef TASKSCALAR
  {
    char cad1[100];
    rsesc_OS_read_string(pthread->getPid(), picode->addr, cad1, pathname, 100);
    err = rmdir((const char *) cad1);
  }
#else
  RAddr path = pthread->virt2real(pathname);
  err = rmdir((const char *) path);
#endif
#ifdef DEBUG_VERBOSE
  fprintf(stderr,"mint_rmdir(%s) ret=%d\n",(const char *) pthread->virt2real(pathname), err);
#endif
  //pthread->setRetVal(err);
  pthread->setRetVal(err);
  if (err == -1)
    pthread->setperrno(errno);
  I(pthread->getPid()==thePid);
  return pthread->getRetIcode();
}
