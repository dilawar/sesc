/*
 * Routines for reading and parsing the text section and managing the
 * memory for an address space.
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

#include "alloca.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(__svr4__) || defined(_SVR4_SOURCE)
#include <sys/times.h>
#endif
#include <limits.h>

#define MAIN

#include "icode.h"
#include "ThreadContext.h"
#include "opcodes.h"
#include "globals.h"
#include "symtab.h"
#include "mendian.h"

#include "nanassert.h"

#ifdef DARWIN
#include <fenv.h>
#endif

#include <assert.h>

/* Note: the debugger dbx gets line numbers confused because of embedded
 * newlines inside the double quotes in the following macro.
 */
#define USAGE \
"\nUsage: %s [mint options] [-- simulator options] objfile [objfile options]\n\
\n\
mint options:\n\
	[-h heap_size]		heap size in bytes, default: %d (0x%x)\n\
	[-k stack_size]		stack size in bytes, default: %d (0x%x)\n\
	[-n nice_value]	        \'nice\' MINT process, default: 0 if 'nice' used, 4 if not\n\
	[-p procs]		number of per-process regions, default: %d\n"

FILE *Fobj=0;		/* file descriptor for object file */
static int32_t Nicode=0;	/* number of free icodes left */

/* imported functions */
void init_main_thread();
void upshot_init(char *fname);
void subst_init();
void subst_functions();

/* exported functions */
void mint_init(int32_t argc, char **argv, char **envp);
void allocate_fixed(int32_t addr, int32_t nbytes);

/* private functions */
static void parse_args(int32_t argc, char **argv);
static void copy_argv(int32_t argc, char **argv, char **envp);
static void mint_stats();
#if !(defined ADDRESS_SPACES)
static void create_addr_space();
#endif // !(defined ADDRESS_SPACES)

static void read_text();
static void usage();

#if (defined ADDRESS_SPACES)
#include "ElfObject.h"
#endif // (defined ADDRESS_SPACES)

void mint_init(int32_t argc, char **argv, char **envp)
{
  extern int32_t optind;
  int32_t next_arg;
  FILE *fd;
  
  Mint_output = stderr;
  Simname = argv[0];
  parse_args(argc, argv);
  ThreadContext::staticConstructor();

  next_arg = optind;
  Objname = argv[next_arg];
  
#if (defined ADDRESS_SPACES)
  ThreadContext *mainThread=ThreadContext::getMainThreadContext();
  AddressSpace  *addrSpace=new AddressSpace();
  mainThread->setAddressSpace(addrSpace);
  loadElfObject(Objname,mainThread);
  size_t heapSize=Heap_size;
  VAddr  heapStart=addrSpace->findVMemLow(heapSize);
  addrSpace->newRMem(heapStart,heapStart+heapSize);
  mainThread->setHeapManager(HeapManager::create(heapStart,heapSize));
  size_t stackSize=Stack_size;
  VAddr  stackStart=addrSpace->findVMemHigh(stackSize);
  addrSpace->newRMem(stackStart,stackStart+stackSize);
  mainThread->setStack(stackStart,stackStart+stackSize);
  mainThread->setStkPtr(stackStart+stackSize);
#endif // (defined ADDRESS_SPACES)

  read_hdrs(Objname);
  subst_init();
  read_text();
#if !(defined ADDRESS_SPACES)
  create_addr_space();
#endif // !(defined ADDRESS_SPACES)
  close_object();
  copy_argv(argc-next_arg,argv+next_arg,envp);
  subst_functions();

  ThreadContext::initMainThread();
  
#ifdef DARWIN
  feclearexcept(FE_ALL_EXCEPT);
#endif

  close_object();
}

static void usage()
{
  fprintf(stderr, USAGE, Simname,
	  HEAP_SIZE, HEAP_SIZE,
	  STACK_SIZE, STACK_SIZE,
	  MAXPROC-1);
}

/* parse the command line arguments */
static void parse_args(int32_t argc, char **argv)
{
  int32_t errflag;
  int32_t c;
  extern char *optarg;

  /* Value of command line option -n, -321 means 'not specified' */
  int32_t NiceValue=-321;
  
  /* set up the default values */
  Stack_size = STACK_SIZE;
  Heap_size = HEAP_SIZE;
  Max_nprocs = MAXPROC-1;
  
  errflag = 0;
  while((c=getopt(argc, argv, "n:h:k:P:p:s:W"))!=-1){
    switch (c) {
    case 'n':
      NiceValue=strtol(optarg,NULL,0);
      break;
    case 'h':
      Heap_size = strtol(optarg, NULL, 0);
      break;
    case 'k':
      Stack_size = strtol(optarg, NULL, 0);
      break;
    case 'p':
      Max_nprocs = strtol(optarg, NULL, 0);
      if (Max_nprocs < 1 || Max_nprocs >= MAXPROC) {
	errflag = 1;
	fprintf(stderr, "Number of processes must be in the range: 1 to %d\n",MAXPROC);
      }
      break;
    default:
      errflag = 1;
      break;
    }
  }

  /* If nice value is set using the -n command line option, use it */
  if(NiceValue!=-321){
    if(nice(NiceValue)==-1){
      error("Invalid nice value (option -n): %d\n",NiceValue);
      exit(1);
    }
  }

  if (errflag) {
    usage();
    exit(1);

  }
}

/* Copy application args onto the simulated stack for the main process */
static void copy_argv(int32_t argc, char **argv, char **envp){
  ThreadContext *context=ThreadContext::getMainThreadContext();
  // Count the environment variables
  int32_t envc=0;
  while(envp[envc])
    envc++;
  // We will need to know where on the simulated stack
  // did we put the arg and env strings
  VAddr *argVAddrs = (VAddr *)alloca(sizeof(VAddr)*argc);
  VAddr *envVAddrs = (VAddr *)alloca(sizeof(VAddr)*envc);
  // Put the env strings on the stack
  if(envc){
    for(int32_t envIdx=envc-1;envIdx>=0;envIdx--){
      size_t strSize=alignUp(strlen(envp[envIdx])+1,32);
      context->setStkPtr(context->getStkPtr()-strSize);
      strcpy((char *)(context->virt2real(context->getStkPtr())),envp[envIdx]);
      envVAddrs[envIdx]=context->getStkPtr();
    }
  }
  // Put the arg string on the stack
  if(argc){
    for(int32_t argIdx=argc-1;argIdx>=0;argIdx--){
      size_t strSize=alignUp(strlen(argv[argIdx])+1,32);
      context->setStkPtr(context->getStkPtr()-strSize);
      strcpy((char *)(context->virt2real(context->getStkPtr())),argv[argIdx]);
      argVAddrs[argIdx]=context->getStkPtr();
    }
  }
  // Put the envp array (with NULL at the end) on the stack
  context->setStkPtr(context->getStkPtr()-sizeof(VAddr));
  *((VAddr *)(context->virt2real(context->getStkPtr())))=
    SWAP_WORD((VAddr)0);
  if(envc){
    for(int32_t envIdx=envc-1;envIdx>=0;envIdx--){
      context->setStkPtr(context->getStkPtr()-sizeof(VAddr));
      *((VAddr *)(context->virt2real(context->getStkPtr())))=
	SWAP_WORD(envVAddrs[envIdx]);
    }
  }
  // Put the argv array (with NULL at the end) on the stack
  context->setStkPtr(context->getStkPtr()-sizeof(VAddr));
  *((VAddr *)(context->virt2real(context->getStkPtr())))=
    SWAP_WORD((VAddr)0);
  if(argc){
    for(int32_t argIdx=argc-1;argIdx>=0;argIdx--){
      context->setStkPtr(context->getStkPtr()-sizeof(VAddr));
      *((VAddr *)(context->virt2real(context->getStkPtr())))=
	SWAP_WORD(argVAddrs[argIdx]);
    }
  }
  // Put the argc on the stack
  context->setStkPtr(context->getStkPtr()-sizeof(VAddr));
  *((IntRegValue *)(context->virt2real(context->getStkPtr())))=
    SWAP_WORD((IntRegValue)argc);
}

#if !(defined ADDRESS_SPACES)
/*
 * logbase2() returns the log (base 2) of its argument, rounded up.
 * It also rounds up its argument to the next higher power of 2.
 */
static int
logbase2(int32_t *pnum)
{
    uint32_t logsize;
    uint32_t exp;

    for (logsize = 0, exp = 1; exp < *pnum; logsize++)
        exp *= 2;
    
    /* round pnum up to nearest power of 2 */
    *pnum = exp;

    return logsize;
}

void *allocate2(int32_t nbytes)
{
  void *ptr;
  int32_t size2;
  int32_t status = 0;
  /* round nbytes up to the next power of 2 */
  size2 = nbytes;
  logbase2(&size2);
#ifdef SUNOS
  ptr = memalign(0x1000000,nbytes);
  status = (ptr == NULL);
#elif POSIX_MEMALIGN
  /* MCH: memalign is obsolete, but on iacoma3 posix_memalign is not yet supported */
  // Milos: align to a 16-megabyte boundary so small
  // changes to the simulator are unlikely to change the
  // starting address of the allocated block of memory
  status = posix_memalign(&ptr,0x1000000,nbytes);
#else
  ptr = malloc(size2);
  status = (ptr == NULL);
#endif

  if(status) {
    fprintf(stderr, "allocate2: cannot allocate 0x%x bytes rounded to 0x%x bytes (status %d).\n",
	    (unsigned) nbytes, (unsigned) size2, status);
    exit(-1);
  }
  /* Slow as hell  bzero(ptr, size2); */

  return ptr;
}

/* Allocate memory for the shared memory region and all the per-process
 * private memory regions and initialize the address space for the main
 * process.
 */
static void create_addr_space()
{
  size_t size, multiples;
  unsigned i;
  unsigned dwords;
  uint32_t *addr;
  thread_ptr pthread;
  size_t min_seek, total_size;
  MINTAddrType heap_start;
  size_t heap_size;

  VAddr dataAddrLb;
  VAddr dataAddrUb;

  /* this assumes that the bss is located higher in memory than the data */

  /* Segments order:
   *
   * (Rdata|Data)...Bss...Heap...Stacks
   *
   */

  if (Rdata_start < Data_start) {
    Rsize_round = Rdata_size;
    if (Rsize_round != (Data_start - Rdata_start)) {
      fatal("Rdata is not contiguous with Data");
    }
    dataAddrLb=Rdata_start;
  }else{
    // Data section includes rdata section (if it exists)
    Rsize_round = 0;
    dataAddrLb=Data_start;
  }

  /* DB_size points to the beginning of Heap */
  DB_size  = Rsize_round + Bss_start + Bss_size - Data_start;

  Mem_size = DB_size;
  // Page align
  Mem_size = ((Mem_size + dataAddrLb + M_ALIGN - 1) & ~(M_ALIGN - 1)) - dataAddrLb;
  size_t Heap_start_rel = Mem_size;

  Mem_size = Mem_size+Heap_size;
  // Page align
  Mem_size = ((Mem_size + dataAddrLb + M_ALIGN - 1) & ~(M_ALIGN - 1)) - dataAddrLb;
  size_t Stack_start_rel = Mem_size;

  Stack_size = (Stack_size + M_ALIGN - 1) & ~(M_ALIGN - 1); // Page align

  Mem_size = Mem_size+Stack_size*Max_nprocs;

  dataAddrUb=dataAddrLb+Mem_size;

  // Data_end includes Bss (may or may not include Rdata)
  Data_end  = Data_start  + Heap_start_rel;
  Rdata_end = Rdata_start + Rdata_size;

  Private_start = (MINTAddrType)allocate2(Mem_size);
  Private_end   = Private_start + Mem_size;

  if (((MINTAddrType)Private_end > (MINTAddrType)Data_start  && (MINTAddrType)Private_end < (MINTAddrType)Data_end)
		|| ((MINTAddrType)Private_start > (MINTAddrType)Data_start  && (MINTAddrType)Private_start < (MINTAddrType)Data_end)) {
    RAddr oldSpace = Private_start;
    Private_start = (MINTAddrType)allocate2(Mem_size);
    fprintf(stderr,"Overlap: Shifting address space [0x%p] -> [0x%p]\n",(void*)oldSpace, (void*)Private_start);
    free((void *)oldSpace);
    Private_end   = Private_start + Mem_size;
  }

  pthread = ThreadContext::getMainThreadContext();

  MINTAddrType addrSpace = Private_start;
  MINTAddrType rdataMap  = Private_start - Rdata_start;

  addr = (uint32_t *) addrSpace;

  if (Rdata_start < Data_start) {
    /* Read in the .rdata section first since the .rdata section is not
     * contiguous with the rest of the data in the address space or object
     * file.
     */
    if (Rdata_seek != 0 && Rdata_size > 0) {
      fseek(Fobj, Rdata_seek, SEEK_SET);

      /* read in the .rdata section from the object file */
      if (fread(addr, sizeof(char), Rdata_size, Fobj) < Rdata_size)
	fatal("create_addr_space: end-of-file reading rdata section\n");

      /* move "addr" to prepare for reading in the data and bss */
      addr = (uint32_t *) (Private_start + Rsize_round);
      addrSpace = (MINTAddrType) addr;
    }
    rdataMap = Private_start - Data_start;
  }

  /* Figure out which data section comes first. The SGI COFF files have
   * .data first. DECstations have .rdata first except sometimes the
   * .rdata section is before the .text section.
   */
  min_seek = UINT_MAX;
  if (Rdata_start >= Data_start && Rdata_seek != 0)
    min_seek = Rdata_seek;
  if (Data_seek != 0 && Data_seek < min_seek)
    min_seek = Data_seek;
  if (Sdata_seek != 0 && Sdata_seek < min_seek)
    min_seek = Sdata_seek;
  if (min_seek >= UINT_MAX)
    fatal("create_addr_space: no .rdata or .data section\n");
        
  fseek(Fobj, min_seek, SEEK_SET);
  dwords = Data_size / 4;

  /* read in the initialized data from the object file */
  if (fread(addr, sizeof(int), dwords, Fobj) < dwords)
    fatal("create_addr_space: end-of-file reading data section\n");

  /* zero out the bss section */
  addr = (uint32_t *) (addrSpace + Bss_start - Data_start);
  dwords = Bss_size / 4;
  for (i = 0; i < dwords; i++)
    *addr++ = 0;

  heap_start = dataAddrLb + Heap_start_rel;
  heap_size = Stack_start_rel - Heap_start_rel; // Next to heap is the stack
  if (heap_size < HEAP_SIZE_MIN) {
    fprintf(stderr, "Not enough memory for private malloc: %u\n", heap_size);
    fprintf(stderr, "Try increasing it using the \"-h\" option.\n");
    usage();
    exit(1);
  }
  Heap_start = heap_start;
  Heap_end   = heap_start+heap_size;
  /* initialize the private memory allocator for this thread */
  pthread->setHeapManager(HeapManager::create(heap_start,heap_size));
  // malloc_init(pthread, heap_start, heap_size);

  /* point the sp to the top of the allocated space */
  /* (The stack grows down toward lower memory addresses.) */
  Stack_start = dataAddrLb + Stack_start_rel;
  Stack_end   = dataAddrUb;
  /* Change the sp so that mapping will work for sp-relative addresses. */

  pthread->initAddressing(dataAddrLb, // Start of virtual addresses for data 
			  dataAddrUb, // End of virtual addresses for data
			  rdataMap                // RDataMap
			  ,addrSpace - Data_start // memMap
			  ,Stack_start            // Stack Top
			  );

  pthread->setStkPtr(Stack_start+Stack_size);
}
#endif // !(defined ADDRESS_SPACES)


/* Share the parent's address space with the child. This is used to
 * support the sproc() system call on the SGI. This also sets up
 * all the mapping fields in the child's thread structure.
 */

/* create a new copy of an icode */
icode_ptr newcopy_icode(icode_ptr picode){
  int32_t i;
  icode_ptr inew;
  static icode_ptr Icode_free;
  
  /* reduce calls to malloc and make more efficient use of space
   * by allocating several icodes at once
   */
  if (Nicode == 0) {
    Icode_free = (icode_ptr) malloc(1024 * sizeof(struct icode));
    if (Icode_free == NULL)
      fatal("newcopy_icode: out of memory\n");
    Nicode = 1024;
  }
  inew = &Icode_free[--Nicode];
  inew->func = picode->func;
  inew->addr = picode->addr;
  inew->instr = picode->instr;
  for (i = 0; i < 4; i++)
    inew->args[i] = picode->args[i];
  inew->immed = picode->immed;
  inew->next = picode->next;
  inew->target = picode->target;
  inew->not_taken = picode->not_taken;
  inew->is_target = picode->is_target;
  inew->opnum = picode->opnum;
  inew->opflags = picode->opflags;
  
  inew->instID = picode->instID;
#if (defined TLS)
  inew->setClass(picode->getClass());
#endif
  return inew;
}

icode_ptr icodeArray;
size_t    icodeArraySize;

/* Reads the text section of the object file and creates the linked list
 * of icode structures.
 */
static void
read_text()
{
  int32_t make_copy, voffset, err;
  unsigned i;
  unsigned num_pointers;
  int32_t instr, opflags, iflags;
  icode_ptr picode, prev_picode, dslot, pcopy, *pitext, pcopy2;
  struct op_desc *pdesc;
  uint32_t addr;
  int32_t opnum;
  int32_t regfield[4], target;
  int32_t prev_was_branch;
  signed short immed;	/* immed must be a signed short */
  PFPI *pfunc;
#ifdef DEBUG_READ_TEXT
  static int32_t Debug_addr = 0;
#endif
  int32_t ud_defined=0;
  int32_t ud_params[MAX_UD_PARAMS];
  int32_t ud_size=0;
  int32_t ud_i;
  uint32_t ud_addr;
  icode_ptr ud_picode;


  /* Allocate space for pointers to icode, plus the SGI function pointers
   * for the _lock routines, plus some for special function
   * pointers that have no other place to go, plus 1 for a NULL pointer.
   * Text_size is the number of instructions.
   */
  num_pointers = Text_size + EXTRA_ICODES;
  Itext = (icode_ptr *) calloc((num_pointers + 1), sizeof(icode_ptr));
  if (Itext == NULL)
    fatal("read_text: cannot allocate 0x%x bytes for Itext.\n",
	  (num_pointers + 1) * sizeof(icode_ptr));

  /* Allocate space for the icode structures */
  picode = (icode_ptr) calloc(num_pointers, sizeof(struct icode));
  icodeArray=picode;
  icodeArraySize=num_pointers;
  if (picode == NULL)
    fatal("read_text: cannot allocate 0x%x bytes for icode structs.\n",
	  num_pointers * sizeof(struct icode));

  /* Assign each pointer to its corresponding icode, and link each
   * icode to point to the next one in the array.
   */
  pitext = Itext;
  for (i = 0; i < num_pointers; i++) {
    *pitext++ = picode;
    picode->instID = i;
    picode->next = picode + 1;
    picode++;
  }
  *pitext = NULL;

  fseek(Fobj, Text_seek, SEEK_SET);
  /*    ldnsseek(Ldptr, ".text"); */

#ifdef notdef
  for (i = 0; i < max_opnum; i++)
    printf("%d %s\n", i, desc_table[i]);
#endif

  prev_was_branch = 0;
  prev_picode = NULL;
  picode = Itext[0];
  addr = Text_start;
  for (i = 0; i < Text_size; i++, addr += 4, picode++) {
	 err = fread(&instr, sizeof(int), 1, Fobj);
    if (err < 1)
      fatal("read_text: end-of-file reading text section\n");
    instr = SWAP_WORD((unsigned)instr);

    picode->addr = addr;
    regfield[RS] = (instr >> 21) & 0x1f;
    regfield[RT] = (instr >> 16) & 0x1f;
    regfield[RD] = (instr >> 11) & 0x1f;
    regfield[SA] = (instr >> 6) & 0x1f;
    immed = instr & 0xffff;

    ud_defined = ( ((instr >> 26) & 0x3f) == 60 );

    opnum = decode_instr(picode, instr);
    pdesc = &desc_table[opnum];
    opflags = pdesc->opflags;
    iflags = pdesc->iflags;
#if (defined TLS)
    // All instructions get the "normal" OpClass by default 
    picode->setClass(OpClass(OpInternal,OpAnywhere));
#endif
    picode->opflags = opflags;
    pfunc = pdesc->func;

    /* replace instructions that use r0 with faster equivalent ones */
    switch(opnum) {
    case beq_opn:
      if (picode->args[RT] == 0) {
	if (picode->args[RS] == 0) {
	  pfunc = b_op;
	  opnum = b_opn;
	} else
	  pfunc = beq0_op;
      }
      break;
    case bne_opn:
      if (picode->args[RT] == 0)
	pfunc = bne0_op;
      break;
    case addiu_opn:
      if (picode->args[RS] == 0) {
	pfunc = li_op;
	opnum = li_opn;
      } else if (picode->args[RS] == picode->args[RT]) {
	pfunc = addiu_xx_op;
      }
      break;
    case addu_opn:
      if (picode->args[RT] == 0) {
	if (picode->args[RS] == 0) {
	  pfunc = move0_op;
	  opnum = move_opn;
	} else {
	  pfunc = move_op;
	  opnum = move_opn;
	}
      }
      break;
#if (defined TLS)
    case aspectReductionBegin_opn:
    case aspectAtomicBegin_opn:
    case aspectAcquireBegin_opn:
    case aspectReleaseBegin_opn:
      picode->setClass(OpClass(OpInternal,OpAtStart));
      break;
    case aspectReductionEnd_opn:
    case aspectAtomicEnd_opn:
      picode->setClass(OpClass(OpInternal,OpCanEnd));
      break;
#endif // (defined TLS)
    }
    picode->opnum = opnum;

    /* Change instructions that modify the stack pointer to
     * include an overflow check. A stack underflow cannot happen.
     */
    if (opnum == addiu_opn && regfield[RT] == 29) {
      if (immed < 0) {
	/* zero the opnum so that the optimizer won't inline this
	 * instruction
	 */
	picode->opnum = 0;
	pfunc = sp_over_op;
      }
    }

    /* Precompute the branch and jump targets */
    if (iflags & IS_BRANCH) {
      if (immed == -1) {
	//	warning("branch to itself at addr 0x%x. jump to next instruction if executed\n",
	//		picode->addr);
	picode->target = picode + 1;
      } else {
	picode->target = picode + immed + 1;
      }
      picode->target->is_target = 1;
    } else if ((iflags & IS_JUMP) && !ud_defined ) {
      /* for jump instrns, the target address is: */
      target = ((instr & 0x03ffffff) << 2) | ((addr + 4) & 0xf0000000);
      /* target == 0 if it has not been relocated yet */
      if (target > 0) {
	/* if the target address is out of range, then this
	 * is probably a jump to a shared library function.
	 */
	if ((unsigned) target > Text_start + 4 * (Text_size + 10)) {
	  fatal("target address (0x%x) of jump instruction at addr 0x%x is past end of text.\n",
		target, picode->addr);
	}
	picode->target = addr2icode(target);
	picode->target->is_target = 1;
      }
    }
    picode->not_taken = picode + 2;

    /* Find user define opnums and handle them appropriately
       In particular, handle parameters so that decoder does not
       get confused */
    ud_size = 0;

    if ( ud_defined ) {

      ud_size = picode->args[RD];
      if(ud_size > MAX_UD_PARAMS)
	fatal("read_text: illegal user-defined opcode.\n");
      ud_addr = addr+4;
      ud_picode = picode+1;
	
      for(ud_i=0; ud_i < ud_size; ud_i++,ud_addr+=4,ud_picode++) {

	err = fread(&ud_params[ud_i], sizeof(int), 1, Fobj);
	if (err < 1)
	  fatal("read_text: end-of-file reading text section\n");
	ud_params[ud_i] = SWAP_WORD((unsigned)ud_params[ud_i]);
	ud_picode->instr = ud_params[ud_i];
	ud_picode->instID = i+ud_i+1;
	ud_picode->addr = ud_addr;
      }
      picode->next = ud_picode;

      switch((ud_class_t)picode->args[SA]) {
      case ud_call:
	/*FIXME: check for proper arguments*/
	if(ud_size > 0) {
	  picode->target = addr2icode(ud_params[0]);
	  picode->target->is_target = 1;
	}
	break;
      case ud_spawn:
	break;
      case ud_fork:
	break;
      case ud_regsynch:
	break;
      case ud_compute:
	break;
      }
    }

    voffset = 2;

    /* Either not tracing any memory references or
     * not a memory reference; use normal version.
     */
    if( opflags & E_MEM_REF )
      picode->func = pfunc[voffset];
    else
      picode->func = pfunc[0];
	
    dslot = NULL;
    if (prev_was_branch) {

      dslot = newcopy_icode(picode);
      /* link in the new copy to the "next" field of the previous icode */
      prev_picode->next = dslot;

      /* The "next" field of the dslot icode should never be used
       * since the current value of pthread->target is used instead.
       * So set the "next" field to NULL to catch bugs.
       */
      dslot->next = NULL;

      /* Either not tracing any memory references or
       * not a memory reference; use branch delay slot version.
       */
      if( opflags & E_MEM_REF )
	dslot->func = pfunc[voffset+1];
      else
	dslot->func = pfunc[1];
    }

    /* Set or clear the "prev_was_branch" flag so that the next instruction
     * knows which version of the function to call.
     */
    prev_was_branch = iflags & BRANCH_OR_JUMP;

    /* prev_picode is needed only for branch delay slot instructions */
    prev_picode = picode;

    /*  If ud_opnums were found, then adjust these variables
	accordingly 
    */
    if( ud_size > 0 ) {
      i+=ud_size;
      picode+=ud_size;
      addr+=4*ud_size;
    }

  }
}

/* Performs the work for decoding an instruction word, filling in the
 * fields of an icode structure, and replacing instructions that use
 * register r0 (always zero) with simpler, faster equivalent ones.
 * The register indices are pre-shifted here so that register value
 * lookups at execution time will be faster.
 *
 * Returns: opnum, the index into the opcode description table.
 */
int
decode_instr(icode_ptr picode, int32_t instr)
{
    int32_t iflags;
    struct op_desc *pdesc;
    int32_t j, opcode, bits31_28, bits20_16, bits5_0, fmt, opnum;
    int32_t coproc, cofun;
    int32_t regfield[4];
    signed short immed;	/* immed must be a signed short */
    
    picode->instr = instr;
    opcode = (instr >> 26) & 0x3f;
    bits31_28 = (instr >> 28) & 0xf;
    bits20_16 = (instr >> 16) & 0x1f;
    bits5_0 = instr & 0x3f;
    regfield[RS] = (instr >> 21) & 0x1f;
    regfield[RT] = (instr >> 16) & 0x1f;
    regfield[RD] = (instr >> 11) & 0x1f;
    regfield[SA] = (instr >> 6) & 0x1f;
    immed = instr & 0xffff;
    picode->immed = immed;
    
    if (opcode == 0) {
        /* special instruction */
        opnum = special_opnums[bits5_0];
    } else if (opcode == 1) {
        /* regimm instruction */
        opnum = regimm_opnums[bits20_16];
    } else if (opcode == 60) {
        /* user defined instructions */
        opnum = user_opnums[bits5_0];
#if (defined TLS)
    } else if (bits31_28 == 0x7){
      /* TLS user-defined instructions */
      opnum = tls_opnums[bits5_0];
#endif
    } else if (bits31_28 != 4) {
        /* normal operation */
        opnum = normal_opnums[opcode];
    } else {
        /* coprocessor instruction */
        coproc = (instr >> 26) & 0x3;
        
        fmt = regfield[FMT];
        if ((instr >> 25) & 1) {
            /* general coprocessor operation, uses cofun */
            if (coproc == 1) {
                cofun = instr & 0x03f;
                if (fmt == 16)		/* single precision format */
                    opnum = cop1func_opnums[0][cofun];
                else if (fmt == 17)		/* double precision format */
                    opnum = cop1func_opnums[1][cofun];
                else			/* fixed-point format */
                    opnum = cop1func_opnums[2][cofun];
                
            } else {
                /* coprocessor other than 1 */
                opnum = normal_opnums[opcode];
            }
        } else {
            switch (fmt) {
              case 0:
                /* mfc1, move register from coprocessor */
                opnum = mfc_opnums[coproc];
                break;
              case 2:
                /* cfc1, move control from coprocessor */
                opnum = cfc_opnums[coproc];
                break;
              case 4:
                /* mtc1, move register to coprocessor */
                opnum = mtc_opnums[coproc];
                break;
              case 6:
                /* ctc1, move control to coprocessor */
                opnum = ctc_opnums[coproc];
                break;
              case 8:
                /* coprocessor branch */
                if (regfield[ICODEFT] < 4)
                    opnum = bc_opnums[coproc][regfield[ICODEFT]];
                else
                    opnum = reserved_opn;
                break;
              default:
                opnum = reserved_opn;
                break;
            }
        }
    }
    pdesc = &desc_table[opnum];
#if 0
    opflags = pdesc->opflags;
#endif
    iflags = pdesc->iflags;

    /* for coprocessor instructions, the fmt field should not be shifted */
    picode->args[FMT] = regfield[FMT];
    
    /* 1. Pre-shift the register indices.
     * 2. Modify instructions that write to r0 so that they write
     *    to r32 instead (so that r0 remains zero).
     * 3. Modify instructions that access the floating point
     *    registers so that they use the correct index into the fp[] array.
     *    This requires flipping the low order bit.
     */
    for (j = 0; j < 4; j++) {
        if (pdesc->regflags[j] & MOD0)
	  if (regfield[j] == 0) {
                if (iflags & MOD0_IS_NOP) {
                    /* replace this instruction with a nop */
                    opnum = nop_opn;
                } else
                    regfield[j] = 32;
	  }
        if ((pdesc->regflags[j] & REG0) || (pdesc->regflags[j] & DREG1))
	  picode->args[j] = regfield[j];
        else if (pdesc->regflags[j] & REG1) {
	  /* ENDIANA TRASH */
#ifdef LENDIAN
	  picode->args[j] = regfield[j];
#else
	  /* flip the low order bit of single precision fp regs */
	  picode->args[j] = (regfield[j] ^ 1);
#endif
        } else
	  picode->args[j] = regfield[j];
	//       picode->args[j] = regfield[j];
    }
    return opnum;
}
