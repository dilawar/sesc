/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Milos Prvulovic

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

#if !(defined MIPS_EMUL)

#include <string.h>

#include "icode.h"
#endif
#include "ThreadContext.h"
#if !(defined MIPS_EMUL)
#include "globals.h"
#include "opcodes.h"
#else
#include "FileSys.h"
#endif

#ifdef TASKSCALAR
#include "MemBuffer.h"
#include "TaskContext.h"
#endif

#include "Events.h"

#if (defined TLS)
#include "Epoch.h"
#endif

ThreadContext::ContextVector ThreadContext::pid2context;

#if !(defined MIPS_EMUL)
extern icode_ptr Idone1;         /* calls terminator1() */
// void malloc_share(thread_ptr child, thread_ptr parent);

Pid_t ThreadContext::baseActualPid;
Pid_t ThreadContext::nextActualPid;
ThreadContext::ContextVector ThreadContext::actualPool;

Pid_t ThreadContext::baseClonedPid;
Pid_t ThreadContext::nextClonedPid;
ThreadContext::ContextVector ThreadContext::clonedPool;

size_t ThreadContext::nThreads;

ThreadContext *ThreadContext::mainThreadContext;

void ThreadContext::staticConstructor(void){
  nextActualPid=baseActualPid=0;
  nextClonedPid=baseClonedPid=1024;
  mainThreadContext=newActual();
  nThreads = 1;
}

void ThreadContext::copy(const ThreadContext *src)
{
  parent=src->parent;
  youngest=src->youngest;
  sibling=src->sibling;
  for(size_t i=0;i<32;i++) {
    reg[i]=src->reg[i];
    fp[i]=src->fp[i];
  }

  lo=src->lo;
  hi=src->hi;

  fcr31=src->fcr31;
  picode=src->picode;
  target=src->target;

  // Copy address mapping info
  virtToRealOffset=src->virtToRealOffset;

  // Copy virtual address ranges
  dataVAddrLb=src->dataVAddrLb;
  dataVAddrUb=src->dataVAddrUb;
  allStacksAddrLb=src->allStacksAddrLb;
  allStacksAddrUb=src->allStacksAddrUb;
  myStackAddrLb=src->myStackAddrLb;
  myStackAddrUb=src->myStackAddrUb;
}
#endif //!(defined MIPS_EMUL)

ThreadContext *ThreadContext::getContext(Pid_t pid)
{
  I(pid>=0);
  I((size_t)pid<pid2context.size());
  return pid2context[pid];
}

#if !(defined MIPS_EMUL)
size_t ThreadContext::size() 
{
  return nThreads;
}

ThreadContext *ThreadContext::getMainThreadContext(void)
{
  I(mainThreadContext);
  I(mainThreadContext->pid==0);
  I(pid2context.size()>0);
  I(pid2context[0]==mainThreadContext);
  return mainThreadContext;
}

ThreadContext *ThreadContext::newActual(void)
{
  ThreadContext *context;
  if(actualPool.empty()) {
    context=static_cast<ThreadContext *>(calloc(1,sizeof(ThreadContext)));
    // Initialize the actual context for the first time
    context->pid=nextActualPid++;
    //    context->pll_lock=&LL_lock[context->pid];
    context->fd=(char *)malloc(MAX_FDNUM*sizeof(char));
    I(context->fd);
    if(pid2context.size()<(size_t)nextActualPid)
      pid2context.resize(nextActualPid,0);
  }else{
    context=actualPool.back();
    actualPool.pop_back();
  }
  pid2context[context->pid]=context;
#if (defined TLS)
  context->myEpoch=0;
#endif
  nThreads++;
  return context;
}

ThreadContext *ThreadContext::newActual(Pid_t pid)
{
  I(pid2context[pid]==0);
  ContextVector::iterator poolIt=actualPool.begin();
  while((*poolIt)->pid!=pid) {
    I(poolIt!=actualPool.end());
    poolIt++;
  }
  ThreadContext *context=*poolIt;
  *poolIt=actualPool.back();
  actualPool.pop_back();
  I(context->pid==pid);
  pid2context[pid]=context;
#if (defined TLS)
  context->myEpoch=0;
#endif
  nThreads++;
  return context;
}

ThreadContext *ThreadContext::newCloned(void)
{
  ThreadContext *context;
  if(clonedPool.empty()) {
    context=static_cast<ThreadContext *>(calloc(1,sizeof(ThreadContext)));
    context->pid=nextClonedPid++;
    pid2context.resize(nextClonedPid,0);
  }else{
    context=clonedPool.back();
    clonedPool.pop_back();
  }
  pid2context[context->pid]=context;
#if (defined TLS)
  context->myEpoch=0;
#endif
  nThreads++;
  return context;
}  

void ThreadContext::free(void)
{
  // The main context has PID zero and should not be freed until the end
  // If it is freed it can be reused, and no child process should have
  // pid zero because it can not be distinguished from the parent in
  // fork-like function calls (which return child's pid in the parent and
  // zero in the child process).
  I((this!=mainThreadContext)||(nThreads==1));
  I(pid>=0);
  I((size_t)pid<pid2context.size());
  I(pid2context[pid]==this);
  // Stupid hack needed to prevent the main thread's address space from
  // ever being freed. When the last thread is freed, some memory requests are
  // still pending and check for validity of their addresses using the
  // main thread context. This needs to be fixed: 1) the thread should die
  // only when its last instruction retires, and 2) a dinst should know
  // its thread, becasue with multiple address spaces it is not enough to
  // know the vaddr and use the main thread to check vaddr validity
  if(this==mainThreadContext)
    return;
  pid2context[pid]=0;
  heapManager->delReference();
  heapManager=0;
  if(isCloned()) {
    clonedPool.push_back(this);
  }else{
    actualPool.push_back(this);
  }
  nThreads--;
}

void ThreadContext::initAddressing(VAddr dataVAddrLb2, VAddr dataVAddrUb2,
				   MINTAddrType rMap, MINTAddrType mMap, MINTAddrType sTop)
{
  dataVAddrLb=dataVAddrLb2;
  dataVAddrUb=dataVAddrUb2;
  allStacksAddrLb=Stack_start;
  allStacksAddrUb=Stack_end;
  myStackAddrLb = sTop;
  myStackAddrUb = myStackAddrLb + Stack_size;
  // Segments order: (Rdata|Data)...Bss...Heap...Stacks
  if(Data_start <= Rdata_start) {
    virtToRealOffset = mMap;
  }else{
    virtToRealOffset = rMap;
  }
    
  // Rdata should be in the static data area
  I(Rdata_start >= dataVAddrLb &&
    Rdata_end <= heapManager->getHeapAddrLb());
  // Data should be in the static data area
  I(Data_start >=  dataVAddrLb &&
    Data_end <= heapManager->getHeapAddrLb());
  // BSS should be in the static data area
  I(Bss_start >= dataVAddrLb &&
    (Bss_start + Bss_size) <= heapManager->getHeapAddrLb());
  // Heap should be between static and stack areas
  I(allStacksAddrLb>=heapManager->getHeapAddrUb());
  // Stack should be in the data area, too
  I(allStacksAddrUb<=dataVAddrUb);
  
  MSG("static[0x%x-0x%x] heap[0x%x-0x%x] stack[0x%x-0x%x] -> [%p-%p]"
      ,(VAddr)dataVAddrLb,(VAddr)(Bss_start+Bss_size)
      ,(VAddr)(heapManager->getHeapAddrLb())
      ,(VAddr)(heapManager->getHeapAddrUb())
      ,(VAddr)allStacksAddrLb
      ,(VAddr)allStacksAddrUb
      ,(void*)(dataVAddrLb+virtToRealOffset)
      ,(void*)(allStacksAddrUb+virtToRealOffset)
      );
}

void ThreadContext::dump()
{
  int32_t i, j;

  printf("thread 0x%p:\n", this);
  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i++)
      printf("  r%d: %s 0x%08x", i, i < 10 ? " " : "", reg[i]);
    printf("\n");
  }
  printf("  lo:   0x%08x  hi:   0x%08x\n", lo, hi);

  /* print out floats and doubles */
  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i++)
      printf("  $f%d:%s %10.6f", i, i < 10 ? " " : "", getFPNUM(i));
    printf("\n");
  }

  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i += 2)
      printf("  $d%d:%s %10.6f", i, i < 10 ? " " : "", getDPNUM(i));
    printf("\n");
  }

  for (i = 0; i < 32; ) {
    for (j = 0; j < 4; j++, i++)
      printf("  $w%d:%s 0x%08x", i, i < 10 ? " " : "", getWFPNUM(i));
    printf("\n");
  }

  printf("  fcr0 = 0x%x, fcr31 = 0x%x\n", fcr0, fcr31);

  printf("  target = 0x%p, pid = %d\n", target, pid);

  printf("  parent = 0x%p, youngest = 0x%p, sibling = 0x%p\n",
         parent, youngest,  sibling);
}

/* dump the stack for stksize number of words */
void ThreadContext::dumpStack()
{
  int32_t stksize = Stack_size;
  VAddr sp;
  VAddr i;
  int32_t j;
  uint8_t c;

  sp = getREGNUM(29);
  printf("sp = 0x%08x\n", (unsigned) sp);
  for (i = sp + (stksize - 1) * 4; i >= sp; i -= 4) {
    printf("0x%08x (+%d): 0x%x (%f)  ",
           i,  (i - sp), *(int32_t *) this->virt2real(i), *(float *) this->virt2real(i));
    for (j = 0; j < 4; j++) {
      c = *((char *) this->virt2real(i) + j);
      if (c < ' ')
        c += 'A' - 1;
      else if (c >= 0x7f)
        c = '?';
      printf("^%c", c);
    }
    printf("\n");
  }
}

void ThreadContext::useSameAddrSpace(thread_ptr pthread)
{
  setHeapManager(pthread->getHeapManager());
  
  lo = pthread->lo;
  hi = pthread->hi;
  for (int32_t i = 0; i < 32; i++)
    reg[i] = pthread->reg[i];


  fcr0 = pthread->fcr0;
  fcr31 = pthread->fcr31;
  for (int32_t i = 0; i < 32; i++)
    fp[i] = pthread->fp[i];

  dataVAddrLb=pthread->dataVAddrLb;
  dataVAddrUb=pthread->dataVAddrUb;
  allStacksAddrLb=pthread->allStacksAddrLb;
  allStacksAddrUb=pthread->allStacksAddrUb;

  myStackAddrLb=pthread->myStackAddrLb;
  myStackAddrUb=pthread->myStackAddrUb;

  virtToRealOffset=pthread->virtToRealOffset;

#if (defined TLS) || (defined TASKSCALAR)
  fd = pthread->fd;
#endif
}

void ThreadContext::shareAddrSpace(thread_ptr pthread, int32_t share_all, int32_t copy_stack)
{
  // The address space has already been allocated

  I(share_all); // share_all is the only supported

  setHeapManager(pthread->getHeapManager());

  /* copy all the registers */
  for (int32_t i = 0; i < 32; i++)
    reg[i] = pthread->reg[i];
  lo = pthread->lo;
  hi = pthread->hi;
  for (int32_t i = 0; i < 32; i++)
    fp[i] = pthread->fp[i];
  fcr0 = pthread->fcr0;
  fcr31 = pthread->fcr31;

  dataVAddrLb=pthread->dataVAddrLb;
  dataVAddrUb=pthread->dataVAddrUb;
  allStacksAddrLb=pthread->allStacksAddrLb;
  allStacksAddrUb=pthread->allStacksAddrUb;

  myStackAddrLb=allStacksAddrLb+pid*Stack_size;
  myStackAddrUb=myStackAddrLb+Stack_size;
  I(myStackAddrUb<=allStacksAddrUb);

  virtToRealOffset=pthread->virtToRealOffset;

  if (copy_stack) {
#if (defined TLS)
    // Need to use TLS read/write methods to properly copy the stack
    I(!copy_stack);
#endif
    // Compute the child's stack pointer and copy the used portion of the stack
    VAddr srcStkPtr=pthread->getStkPtr();
    I(pthread->isLocalStackData(srcStkPtr));
    VAddr dstStkPtr=myStackAddrLb+(srcStkPtr-pthread->myStackAddrLb);
    setStkPtr(dstStkPtr);
    I(isLocalStackData(dstStkPtr));
    memcpy((void *)(virt2real(dstStkPtr)),
	   (void *)(pthread->virt2real(srcStkPtr)),
	   myStackAddrUb-dstStkPtr);
  } else {
    // Point32_t to the top of the stack (but leave some empty space there)
    setStkPtr(myStackAddrUb-FRAME_SIZE);
    I(isLocalStackData(getStkPtr()));
  }
}

#ifdef TASKSCALAR
void ThreadContext::badSpecThread(VAddr addr, short opflags) const
{
  I(pid != -1);

  GLOG(pid == -1, "failed Addressing Invalid thread");

  if(!rsesc_is_safe(pid) && rsesc_is_versioned(pid)) {
    LOG("speculative thread using bad pointer. stopping thread. pid = %d. Iaddr=0x%08x"
        , pid, osSim->eventGetInstructionPointer(pid)->addr);
    rsesc_exception(pid);
    return;
  }

  I(0); // warning, bad code?

  PAddr spawnAddr = 0xdeadbeef; // not valid address
  spawnAddr = TaskContext::getTaskContext(pid)->getSpawnAddr();

  bool badAddr  = !isValidDataVAddr(addr);
  bool badAlign = (
                   MemBufferEntry::calcAccessMask(opflags
                                                  ,MemBufferEntry::calcChunkOffset(addr))
                   ) == 0;

  GMSG(badAlign,"(failed) bad aligment 0x%3x [flags=0x%x] in safe thread. pid=%d. pc=0x%08x  (R31=0x%08x) (SA=0x%08x)"
       , (int)addr, (int)opflags, pid, osSim->eventGetInstructionPointer(pid)->addr, osSim->getContextRegister(pid, 31),
       spawnAddr);


  GMSG(badAddr, "(failed) bad pointer 0x%3x [flags=0x%x] in safe thread. pid=%d. pc=0x%08x  (R31=0x%08x) (SA=0x%08x)" 
       , (int)addr, (int)opflags, pid, osSim->eventGetInstructionPointer(pid)->addr, osSim->getContextRegister(pid, 31),
       spawnAddr);

  if(!badAddr)  // temporary ISCA03 hack
    return;

  mint_termination(pid);
}
#endif

void ThreadContext::init()
{
  int32_t i;

  reg[0]    = 0;
  rerrno    = 0;
  youngest  = NULL;

  /* map in the global errno */
  if (Errno_addr)
    perrno = (int32_t *) virt2real(Errno_addr);
  else
    perrno = &rerrno;

  /* init all file descriptors to "closed" */
  for (i = 0; i < MAX_FDNUM; i++)
    fd[i] = 0;
}

void ThreadContext::initMainThread()
{
  int32_t i, entry_index;
  thread_ptr pthread=getMainThreadContext();

  pthread->parent = NULL;
  pthread->sibling = NULL;

  Maxpid = 0;

  pthread->init();

#ifdef MIPS2_FNATIVE
  pthread->fcr31 = s_get_fcr31();
#else
  pthread->fcr31 = 0;
#endif

  /* Set the picode to the first executable instruction. */
  if (Text_entry == 0)
    Text_entry = Text_start;
  entry_index = (Text_entry - Text_start) / sizeof(int);
  pthread->picode = Itext[entry_index];

  /* initialize the icodes for the terminator functions */
  /* set up this picode so that terminator1() gets called */
  Idone1 = Itext[Text_size + DONE_ICODE];
  Idone1->func = terminator1;
  Idone1->opnum = terminate_opn;

  /* Set up the return address so that terminator1() gets called.
   * This probably isn't necessary since exit() gets called instead */
  pthread->reg[31]=(int)icode2addr(Idone1);
}

void ThreadContext::newChild(ThreadContext *child)
{
  child->parent = this;
 
  child->sibling = youngest;
  youngest = child;
}

#if (defined TLS)
Pid_t ThreadContext::getThreadPid(void) const{
  I(getEpoch());
  return getEpoch()->getTid();
}
#endif

#endif // For !(defined MIPS_EMUL)

#if !(defined MIPS_EMUL)
uint64_t ThreadContext::getMemValue(RAddr p, unsigned dsize) {
  uint64_t value = 0;
  switch(dsize) {
  case 1:
    value = *((uint8_t *) p);
    break;
  case 2:
    value = *((uint16_t *) p);
#ifdef LENDIAN
    value = SWAP_SHORT(value);
#endif      
    break;
  case 4:
    value = *((uint32_t *) p);
#ifdef LENDIAN
    value = SWAP_WORD(value);
#endif      
    break;
  case 8:
    value = *((uint64_t *) p);
#ifdef LENDIAN
    value = SWAP_LONG(value);
#endif      
    break;
  default:
    MSG("ThreadContext:warning, getMemValue with bad (%d) data size.", dsize);
    value = 0;
    break;
  }
  
  return value;
}

int32_t MintFuncArgs::getInt32(void) {
  int32_t retVal; 
  I(sizeof(retVal)==4);
  I((curPos % 4)==0);
  if(curPos<16){
    I(curPos % 4==0);
    retVal=myContext->getIntReg((IntRegName)(4+curPos/4));
  }else{
    RAddr addr=myContext->virt2real(myContext->getStkPtr())+curPos;
#if (defined TASKSCALAR) || (defined TLS)
    VAddr vaddr = myContext->real2virt(addr);
    int32_t *ptr =(int32_t *)(rsesc_OS_read(myContext->getPid(),myIcode->addr,vaddr,E_WORD));
#else
    int32_t *ptr=(int32_t *)addr;
#endif
    retVal=SWAP_WORD(*ptr);
  }                                     
  curPos+=4;
  return retVal;
}           

int64_t  MintFuncArgs::getInt64(void){
  int64_t retVal;
  I(sizeof(retVal)==8);
  I((curPos%4)==0);
  // Align current position                  
  if(curPos%8!=0)
    curPos+=4;               
  I(curPos%8==0);
  if(curPos<16){
    retVal=myContext->getIntReg((IntRegName)(4+curPos/4));
    retVal=(retVal<<32)&0xFFFFFFFF00000000llu;
    retVal|=myContext->getIntReg((IntRegName)(4+curPos/4+1))&0xFFFFFFFFllu;
  }else{
    RAddr addr=myContext->virt2real(myContext->getStkPtr())+curPos;
#if (defined TASKSCALAR) || (defined TLS)
    int64_t *ptr =
      (int64_t *)(rsesc_OS_read(myContext->getPid(),myIcode->addr,addr,E_DWORD));
#else
    int64_t *ptr=(int64_t *)addr;
#endif
    retVal=SWAP_LONG(*ptr);
  }
  curPos+=8;
  return retVal;
}
#endif // !(defined MIPS_EMUL)

#if (defined MIPS_EMUL)

void ThreadContext::setMode(ExecMode mode){
  execMode=mode;
  if(mySystem)
    delete mySystem;
  mySystem=LinuxSys::create(execMode);
}

void ThreadContext::save(ChkWriter &out) const{
  out << "ThreadContext pid " << pid;
  out << "Mode " << execMode << " exited " << exited << endl;
  out << "AddressSpace ";
  out.writeobj(getAddressSpace());
  out << "Parent " << parentID << " exitSig " << exitSig << " clear_child_tid " << clear_child_tid << endl;
  out << "robust_list " << robust_list << endl;
  if(exited){
    I(!iDesc);
    I(!getSignalTable());
    out << "Exit " << exitCode << endl;
    return;
  }
  I(getSignalTable());
  out << "SignalTable ";
  out.writeobj(getSignalTable());
  I(getOpenFiles());
  out << "OpenFiles ";
  out.writeobj(getOpenFiles());
  out << "SigMask " << sigMask << endl;
  out << "ReadySig " << readySig.size() << endl;
  for(size_t i=0;i<readySig.size();i++)
    out << *(readySig[i]) << endl;
  out << "MaskedSig " << maskedSig.size() << endl;
  for(size_t i=0;i<maskedSig.size();i++)
    out << *(maskedSig[i]) << endl;
  out << "SuspSig " << (suspSig?'+':'-') << endl;  
  I(iDesc);
  out << "Children " << childIDs.size() << ":";
  for(IntSet::iterator childIt=childIDs.begin();childIt!=childIDs.end();childIt++)
    out << " " << *childIt;
  out << endl;
  out << "Stack " << hex << myStackAddrLb << " to " << myStackAddrUb << endl;
  out << "Regs" << endl;
  for(size_t r=0;r<NumOfRegs;r++){
    if(r%4==0)
      out << r << ":";
    out << " ";
    out.writehex(regs[r]);
    if(r%4==3)
      out << endl;
  }
  out << "PC " << getIAddr() << " / " << getIDesc()-getAddressSpace()->virtToInst(getIAddr()) << dec << endl;
}

ThreadContext::ThreadContext(ChkReader &in) : nDInsts(0) {
  in >> "ThreadContext pid " >> pid;
  pid2context[pid]=this;
  size_t _execMode;
  in >> "Mode " >> _execMode >> " exited " >> exited >> endl;
  setMode(static_cast<ExecMode>(_execMode));
  size_t _addressSpace;
  in >> "AddressSpace ";
  setAddressSpace(in.readobj<AddressSpace>());
  size_t _exitSig;
  in >> "Parent " >> parentID >> " exitSig " >> _exitSig >> " clear_child_tid " >> clear_child_tid >> endl;
  exitSig=static_cast<SignalID>(_exitSig);
  in >> "robust_list " >> robust_list >> endl;
  if(exited){
    setIAddr(0);
    in >> "Exit " >> exitCode >> endl;
    return;
  }
  size_t _signalTable;
  in >> "SignalTable " >> _signalTable >> endl;
  if(!in.hasObject(_signalTable)){
    sigTable=new SignalTable();
    in.newObject(getSignalTable(),_signalTable);
    in >> *(getSignalTable());
  }else{
    sigTable=static_cast<SignalTable *>(in.getObject(_signalTable));
  }
  I(getSignalTable());
  size_t _openFiles;
  in >> "OpenFiles " >> _openFiles >> endl;
  if(!in.hasObject(_openFiles)){
    in.newObject(_openFiles);
    openFiles=new FileSys::OpenFiles(in);
    in.setObject(_openFiles,openFiles);
  }else{
    openFiles=static_cast<FileSys::OpenFiles *>(in.getObject(_openFiles));
  }
  I(getOpenFiles());
  in >> "SigMask " >> sigMask >> endl;
  size_t _ready;
  in >> "ReadySig " >> _ready >> endl;
  for(size_t i=0;i<_ready;i++){
    readySig.push_back(new SigInfo());
    in >> *(readySig[i]) >> endl;
  }
  size_t _masked;
  in >> "MaskedSig " >> _masked >> endl;
  for(size_t i=0;i<_masked;i++){
    maskedSig.push_back(new SigInfo());
    in >> *(maskedSig[i]) >> endl;
  }
  char _susp;
  in >> "SuspSig " >> _susp >> endl;
  suspSig=(_susp=='+');

  size_t childCnt;
  in >> "Children " >> childCnt >> ":";
  while(childCnt){
    size_t childNum;
    in >> " " >> childNum;
    childIDs.insert(childNum);
    childCnt--;
  }
  in >> endl;
  in >> "Stack " >> hex >> myStackAddrLb >> " to " >> myStackAddrUb >> endl;
  in >> "Regs" >> endl;
  for(size_t r=0;r<NumOfRegs;r++){
    if(r%4==0){
      size_t tmp;
      in >> tmp >> ":";
    }
    in >> " ";
    in.readhex(regs[r]);
    if(r%4==3)
      in >> endl;
  }
  VAddr pc; ssize_t upc;
  in >> "PC " >> pc >> " / " >> upc >> dec >> endl;
  setIAddr(pc);
  updIDesc(upc);
}

ThreadContext::ThreadContext(FileSys::FileSys *fileSys)
  :
  myStackAddrLb(0),
  myStackAddrUb(0),
  execMode(ExecModeNone),
  iAddr(0),
  iDesc(InvalidInstDesc),
  dAddr(0),
  nDInsts(0),
  fileSys(fileSys),
  openFiles(new FileSys::OpenFiles()),
  sigTable(new SignalTable()),
  sigMask(),
  maskedSig(),
  readySig(),
  suspSig(false),
  mySystem(0),
  parentID(-1),
  childIDs(),
  exitSig(SigNone),
  clear_child_tid(0),
  robust_list(0),
  exited(false),
  exitCode(0),
  killSignal(SigNone),
  callStack()
{
  for(tid=0;(tid<int(pid2context.size()))&&pid2context[tid];tid++);
  if(tid==int(pid2context.size()))
    pid2context.resize(pid2context.size()+1);
  pid2context[tid]=this;
  pid=tid;
  tgid=tid;
  pgid=tid;

  memset(regs,0,sizeof(regs));
  setAddressSpace(new AddressSpace());
}

ThreadContext::ThreadContext(ThreadContext &parent,
			     bool cloneParent, bool cloneFileSys, bool newNameSpace,
			     bool cloneFiles, bool cloneSighand,
			     bool cloneVm, bool cloneThread,
			     SignalID sig, VAddr clearChildTid)
  :
  myStackAddrLb(parent.myStackAddrLb),
  myStackAddrUb(parent.myStackAddrUb),
  dAddr(0),
  nDInsts(0),
  fileSys(cloneFileSys?((FileSys::FileSys *)(parent.fileSys)):(new FileSys::FileSys(*(parent.fileSys),newNameSpace))),
  openFiles(cloneFiles?((FileSys::OpenFiles *)(parent.openFiles)):(new FileSys::OpenFiles(*(parent.openFiles)))),
  sigTable(cloneSighand?((SignalTable *)(parent.sigTable)):(new SignalTable(*(parent.sigTable)))),
  sigMask(),
  maskedSig(),
  readySig(),
  suspSig(false),
  mySystem(0),
  parentID(cloneParent?parent.parentID:parent.pid),
  childIDs(),
  exitSig(sig),
  clear_child_tid(0),
  robust_list(0),
  exited(false),
  exitCode(0),
  killSignal(SigNone),
  callStack(parent.callStack)
{
  I((!newNameSpace)||(!cloneFileSys));
  setMode(parent.execMode);
  for(tid=0;(tid<int(pid2context.size()))&&pid2context[tid];tid++);
  if(tid==int(pid2context.size()))
    pid2context.resize(pid2context.size()+1);
  pid2context[tid]=this;
  pid=tid;
  if(cloneThread){
    tgid=parent.tgid;
    I(tgid!=-1);
    I(pid2context[tgid]);
    pid2context[tgid]->tgtids.insert(tid);
  }else{
    tgid=tid;
  }
  pgid=parent.pgid;
  if(parentID!=-1)
    pid2context[parentID]->childIDs.insert(pid);
  memcpy(regs,parent.regs,sizeof(regs));
  // Copy address space and instruction pointer
  if(cloneVm){
    setAddressSpace(parent.getAddressSpace());
    iAddr=parent.iAddr;
    iDesc=parent.iDesc;
  }else{
    setAddressSpace(new AddressSpace(*(parent.getAddressSpace())));
    iAddr=parent.iAddr;
    iDesc=virt2inst(iAddr);
  }
  // This must be after setAddressSpace (it resets clear_child_tid)
  clear_child_tid=clearChildTid;
}

ThreadContext::~ThreadContext(void){
  I(!nDInsts);
  while(!maskedSig.empty()){
    delete maskedSig.back();
    maskedSig.pop_back();
  }
  while(!readySig.empty()){
    delete readySig.back();
    readySig.pop_back();
  }
  if(getAddressSpace())
    setAddressSpace(0);
  if(mySystem)
    delete mySystem;
}

void ThreadContext::setAddressSpace(AddressSpace *newAddressSpace){
  if(addressSpace)
    getSystem()->clearChildTid(this,clear_child_tid);
  addressSpace=newAddressSpace;
}

#include "OSSim.h"

int32_t ThreadContext::findZombieChild(void) const{
  for(IntSet::iterator childIt=childIDs.begin();childIt!=childIDs.end();childIt++){
    ThreadContext *childContext=getContext(*childIt);
    if(childContext->isExited()||childContext->isKilled())
      return *childIt;
  }
  return 0;
}

void ThreadContext::suspend(void){
  I(!isSuspended());
  I(!isExited());
  suspSig=true;
  osSim->eventSuspend(pid,pid);
}

void ThreadContext::signal(SigInfo *sigInfo){
  I(!isExited());
  SignalID sig=sigInfo->signo;
  if(sigMask.test(sig)){
    maskedSig.push_back(sigInfo);
  }else{
    readySig.push_back(sigInfo);
    if(suspSig)
      resume();
  }
}

void ThreadContext::resume(void){
  I(suspSig);
  I(!exited);
  suspSig=false;
  osSim->eventResume(pid,pid);
}

bool ThreadContext::exit(int32_t code){
  I(!isExited());
  I(!isKilled());
  I(!isSuspended());
  openFiles=0;
  sigTable=0;
  exited=true;
  exitCode=code;
  if(tgid!=tid){
    I(tgid!=-1);
    I(pid2context[tgid]);
    pid2context[tgid]->tgtids.erase(tid);
    tgid=-1;
  }
  if(pgid==tid){
    // TODO: Send SIGHUP to each process in the process group
  }
  osSim->eventExit(pid,exitCode);
  while(!childIDs.empty()){
    ThreadContext *childContext=getContext(*(childIDs.begin()));
    I(childContext->parentID==pid);
    childIDs.erase(childContext->pid);
    childContext->parentID=-1;
    if(childContext->exited)
      childContext->reap();
  }
  iAddr=0;
  iDesc=InvalidInstDesc;
  if(robust_list)
    getSystem()->exitRobustList(this,robust_list);
  if(parentID==-1){
    reap();
    return true;
  }
  ThreadContext *parent=getContext(parentID);
  I(parent->pid==parentID);
  I(parent->childIDs.count(pid));
  return false;
}
void ThreadContext::reap(){
  I(exited);
  if(parentID!=-1){
    ThreadContext *parent=getContext(parentID);
    I(parent);
    I(parent->pid==parentID);
    I(parent->childIDs.count(pid));
    parent->childIDs.erase(pid);
  }
  pid2context[pid]=0;
}

inline bool ThreadContext::skipInst(void){
  if(isSuspended())
    return false;
  if(isExited())
    return false;
#if (defined DEBUG_InstDesc)
  iDesc->debug();
#endif
  (*iDesc)(this);
  return true;
}

int64_t ThreadContext::skipInsts(int64_t skipCount){
    int64_t skipped=0;
    int nowPid=0;
    while(skipped<skipCount){
      nowPid=nextReady(nowPid);
      if(nowPid==-1)
        return skipped;
      ThreadContext::pointer context=pid2context[nowPid];
      I(context);
      I(!context->isSuspended());
      I(!context->isExited());
      int nowSkip=(skipCount-skipped<500)?(skipCount-skipped):500;
      while(nowSkip&&context->skipInst()){
        nowSkip--;
        skipped++;
      }
      nowPid++;
    }
    return skipped;
  }

void ThreadContext::writeMemFromBuf(VAddr addr, size_t len, const void *buf){
  I(canWrite(addr,len));
  const uint8_t *byteBuf=(uint8_t *)buf;
  while(len){
    if((addr&sizeof(uint8_t))||(len<sizeof(uint16_t))){
      writeMemRaw(addr,*((uint8_t *)byteBuf));
      addr+=sizeof(uint8_t);
      byteBuf+=sizeof(uint8_t);
      len-=sizeof(uint8_t);
    }else if((addr&sizeof(uint16_t))||(len<sizeof(uint32_t))){
      writeMemRaw(addr,*((uint16_t *)byteBuf));
      addr+=sizeof(uint16_t);
      byteBuf+=sizeof(uint16_t);
      len-=sizeof(uint16_t);
    }else if((addr&sizeof(uint32_t))||(len<sizeof(uint64_t))){
      writeMemRaw(addr,*((uint32_t *)byteBuf));
      addr+=sizeof(uint32_t);
      byteBuf+=sizeof(uint32_t);
      len-=sizeof(uint32_t);
    }else{
      I(!(addr%sizeof(uint64_t)));
      I(len>=sizeof(uint64_t));
      writeMemRaw(addr,*((uint64_t *)byteBuf));
      addr+=sizeof(uint64_t);
      byteBuf+=sizeof(uint64_t);
      len-=sizeof(uint64_t);
    }
  }
}
/*
ssize_t ThreadContext::writeMemFromFile(VAddr addr, size_t len, int32_t fd, bool natFile, bool usePread, off_t offs){
  I(canWrite(addr,len));
  ssize_t retVal=0;
  uint8_t buf[AddressSpace::getPageSize()];
  while(len){
    size_t ioSiz=AddressSpace::getPageSize()-(addr&(AddressSpace::getPageSize()-1));
    if(ioSiz>len)
      ioSiz=len;
    ssize_t nowRet;
    if(usePread){
      nowRet=(natFile?(pread(fd,buf,ioSiz,offs+retVal)):(openFiles->pread(fd,buf,ioSiz,offs+retVal)));
    }else{
      nowRet=(natFile?(read(fd,buf,ioSiz)):(openFiles->read(fd,buf,ioSiz)));
    }
    if(nowRet==-1)
      return nowRet;
    retVal+=nowRet;
    writeMemFromBuf(addr,nowRet,buf);
    addr+=nowRet;
    len-=nowRet;
    if(nowRet<(ssize_t)ioSiz)
      break;
  }
  return retVal;
}
*/
void ThreadContext::writeMemWithByte(VAddr addr, size_t len, uint8_t c){
  I(canWrite(addr,len));
  uint8_t buf[AddressSpace::getPageSize()];
  memset(buf,c,AddressSpace::getPageSize());
  while(len){
    size_t wrSiz=AddressSpace::getPageSize()-(addr&(AddressSpace::getPageSize()-1));
    if(wrSiz>len) wrSiz=len;
    writeMemFromBuf(addr,wrSiz,buf);
    addr+=wrSiz;
    len-=wrSiz;
  }
}
void ThreadContext::readMemToBuf(VAddr addr, size_t len, void *buf){
  I(canRead(addr,len));
  uint8_t *byteBuf=(uint8_t *)buf;
  while(len){
    if((addr&sizeof(uint8_t))||(len<sizeof(uint16_t))){
      *((uint8_t *)byteBuf)=readMemRaw<uint8_t>(addr);
      addr+=sizeof(uint8_t);
      byteBuf+=sizeof(uint8_t);
      len-=sizeof(uint8_t);
    }else if((addr&sizeof(uint16_t))||(len<sizeof(uint32_t))){
      *((uint16_t *)byteBuf)=readMemRaw<uint16_t>(addr);
      addr+=sizeof(uint16_t);
      byteBuf+=sizeof(uint16_t);
      len-=sizeof(uint16_t);
    }else if((addr&sizeof(uint32_t))||(len<sizeof(uint64_t))){
      *((uint32_t *)byteBuf)=readMemRaw<uint32_t>(addr);
      addr+=sizeof(uint32_t);
      byteBuf+=sizeof(uint32_t);
      len-=sizeof(uint32_t);
    }else{
      I(!(addr%sizeof(uint64_t)));
      I(len>=sizeof(uint64_t));
      *((uint64_t *)byteBuf)=readMemRaw<uint64_t>(addr);
      addr+=sizeof(uint64_t);
      byteBuf+=sizeof(uint64_t);
      len-=sizeof(uint64_t);
    }
  }
}
/*
ssize_t ThreadContext::readMemToFile(VAddr addr, size_t len, int32_t fd, bool natFile){
  I(canRead(addr,len));
  ssize_t retVal=0;
  uint8_t buf[AddressSpace::getPageSize()];
  while(len){
    size_t ioSiz=AddressSpace::getPageSize()-(addr&(AddressSpace::getPageSize()-1));
    if(ioSiz>len) ioSiz=len;
    readMemToBuf(addr,ioSiz,buf);
    ssize_t nowRet=-1;
    if(natFile)
      nowRet=write(fd,buf,ioSiz);
    else
      nowRet=openFiles->write(fd,buf,ioSiz);
    if(nowRet==-1)
      return nowRet;
    retVal+=nowRet;
    addr+=nowRet;
    len-=nowRet;
    if(nowRet<(ssize_t)ioSiz)
      break;
  }
  return retVal;
}
*/
ssize_t ThreadContext::readMemString(VAddr stringVAddr, size_t maxSize, char *dstStr){
  size_t i=0;
  while(true){
    if(!canRead(stringVAddr+i,sizeof(char)))
      return -1;
    char c=readMemRaw<char>(stringVAddr+i);
    if(i<maxSize)
      dstStr[i]=c;
    i++;
    if(c==(char)0)
      break;
  }
  return i;
}
#if (defined DEBUG_BENCH)
VAddr ThreadContext::readMemWord(VAddr addr){
  return readMemRaw<VAddr>(addr);
}
#endif

void ThreadContext::execCall(VAddr entry, VAddr  ra, VAddr sp){
  I(entry!=0x418968);
  // Unwind stack if needed
  while(!callStack.empty()){
    if(sp<callStack.back().sp)
      break;
    if((sp==callStack.back().sp)&&(addressSpace->getFuncAddr(ra)==callStack.back().entry))
      break;
    callStack.pop_back();
  }
  bool tailr=(!callStack.empty())&&(sp==callStack.back().sp)&&(ra==callStack.back().ra);
  callStack.push_back(CallStackEntry(entry,ra,sp,tailr));
#ifdef DEBUG
  if(!callStack.empty()){
    CallStack::reverse_iterator it=callStack.rbegin();
    while(it->tailr){
      I(it!=callStack.rend());
      it++;
    }
    it++;
    I((it==callStack.rend())||(it->entry==addressSpace->getFuncAddr(ra))||(it->entry==addressSpace->getFuncAddr(ra-1)));
  }
#endif
}
void ThreadContext::execRet(VAddr entry, VAddr ra, VAddr sp){
  while(callStack.back().sp!=sp){
    I(callStack.back().sp<sp);
    callStack.pop_back();
  }
  while(callStack.back().tailr){
    I(sp==callStack.back().sp);
    I(ra==callStack.back().ra);
    callStack.pop_back();
  }
  I(sp==callStack.back().sp);
  I(ra==callStack.back().ra);
  callStack.pop_back();
}
void ThreadContext::dumpCallStack(void){
  printf("Call stack dump for thread %d begins\n",pid);
  for(size_t i=0;i<callStack.size();i++)
    printf("  Entry 0x%08llx from 0x%08llx with sp 0x%08llx tail %d Name %s File %s\n",
	   (unsigned long long)(callStack[i].entry),(unsigned long long)(callStack[i].ra),
	   (unsigned long long)(callStack[i].sp),callStack[i].tailr,
	   addressSpace->getFuncName(callStack[i].entry).c_str(),
	   addressSpace->getFuncFile(callStack[i].entry).c_str());
  printf("Call stack dump for thread %d ends\n",pid);
}

void ThreadContext::clearCallStack(void){
  printf("Clearing call stack for %d\n",pid);
  callStack.clear();
}
#endif // (define MIPS_EMUL)
