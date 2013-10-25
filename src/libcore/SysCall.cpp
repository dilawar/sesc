#include "SysCall.h"

// Memory management system calls

// For definition of ENOMEM
#include <errno.h>

SysCallMalloc::SysCallMalloc(void)
  : SysCall(){
  ID(type="SysCallMalloc");
}

void SysCallMalloc::exec(ThreadContext *context, icode_ptr picode){
  MintFuncArgs args(context, picode);
  size_t size=args.getInt32();
  I((!executed)||(mySize==size));
  ID(mySize=size);
  I((!executed)||(myPid==context->getThreadPid()));
  myPid=context->getThreadPid();
  if(!executed)
    myAddr=context->getHeapManager()->allocate(size);
  if(myAddr){
    // Map real address to logical address space
    context->setRetVal(context->real2virt(myAddr));
  }else{
    // Return 0 and set errno to be POSIX compliant
    context->setErrno(ENOMEM);
    context->setRetVal(0);
  }
  /*printf("%1d: exec(%d) SysCallMalloc %8x at
   * %8x\n",myPid,executed?1:0,mySize,myAddr);*/
  executed=true;
}
void SysCallMalloc::undo(bool expectRedo){
  I(executed);
  if((!expectRedo)&&myAddr){
    I(ThreadContext::getContext(myPid));
    size_t size=ThreadContext::getContext(myPid)->getHeapManager()->deallocate(myAddr);
    I(size==mySize);
  }
/*  printf("%1d: undo(%d) SysCallMalloc %8x at
 *  %8x\n",myPid,executed?1:0,mySize,myAddr);*/
}

SysCallFree::SysCallFree(void)
  : SysCall(){
   ID(type="SysCallFree");
}
 

void SysCallFree::exec(ThreadContext *context, icode_ptr picode){
  MintFuncArgs args(context, picode);
  VAddr addr=args.getVAddr();
  I(addr);
  I((!executed)||((Address)(context->virt2real(addr))==myAddr));
  myAddr=context->virt2real(addr);
  I((!executed)||(myPid==context->getThreadPid()));
  myPid=context->getThreadPid();
  if(!executed)
    mySize=context->getHeapManager()->deallocate(myAddr);
  /*printf("%1d: exec(%d) SysCallFree   %8x at
   * %8x\n",myPid,executed?1:0,mySize,myAddr);*/
  executed=true;
}
void SysCallFree::undo(bool expectRedo){
  I(executed);
  I(ThreadContext::getContext(myPid));
  /*printf("%1d: undo(%d) SysCallFree   %8x at
   * %8x\n",myPid,executed?1:0,mySize,myAddr);*/
  if(!expectRedo){
    Address addr=ThreadContext::getContext(myPid)->getHeapManager()->allocate(myAddr,mySize);
    I(addr==myAddr);
  }
}
void SysCallMmap::exec(ThreadContext *context, icode_ptr picode){
  MintFuncArgs args(context, picode);
  // Prefered address for mmap. This is ignored in SESC.
  VAddr addr=args.getVAddr();
  // Size of block to mmap
  size_t size=args.getInt32();
  // Protection flags for mmap
  int32_t prot=args.getInt32();
  // PROT_READ and PROT_WRITE should be set, and nothing else
  I(prot==0x3);
  // Flags for mmap
  int32_t flag=args.getInt32();
  // MAP_ANONYMOUS and MAP_PRIVATE should be set, and nothing else
  I(flag==0x802);
  I((!executed)||(mySize==size));
  ID(mySize=size);
  I((!executed)||(myPid==context->getThreadPid()));
  if(!executed){
    myAddr=context->getHeapManager()->allocate(size);
    if(myAddr==0)
      myAddr=-1;
  }
  if(myAddr!=-1){
    // Map real address to logical address space
    context->setRetVal(context->real2virt(myAddr));
  }else{
    // Set errno to be POSIX compliant
    context->setErrno(ENOMEM);
    context->setRetVal(myAddr);
  }
  executed=true;
}
void SysCallMmap::undo(bool expectRedo){
  I(executed);
  if((!expectRedo)&&(myAddr!=-1)){
    I(ThreadContext::getContext(myPid));
    size_t size=ThreadContext::getContext(myPid)->getHeapManager()->deallocate(myAddr);
    I(size==mySize);
  }
}
void SysCallMunmap::exec(ThreadContext *context, icode_ptr picode){
  MintFuncArgs args(context, picode);
  // Starting address of the block
  VAddr addr=args.getInt32();
  // Size of block to munmap
  size_t wantSize=args.getInt32();
  I(addr);
  I((!executed)||((Address)(context->virt2real(addr))==myAddr));
  myAddr=context->virt2real(addr);
  I(wantSize);
  I((!executed)||(myPid==context->getThreadPid()));
  myPid=context->getThreadPid();
  if(!executed)
    mySize=context->getHeapManager()->deallocate(myAddr);
  I(mySize==wantSize);
  executed=true;
}
void SysCallMunmap::undo(bool expectRedo){
  I(executed);
  I(ThreadContext::getContext(myPid));
  if(!expectRedo){
    Address addr=ThreadContext::getContext(myPid)->getHeapManager()->allocate(myAddr,mySize);
    I(addr==myAddr);
  }
}

// File I/O system calls

// For open, close 
#include <fcntl.h>
// For read, write, stat
#include <unistd.h>

struct GlibcStat64{
  unsigned long st_dev;
  unsigned long st_pad0[3];     /* Reserved for st_dev expansion  */
  unsigned long long    st_ino;
  uint32_t  st_mode;
  int32_t           st_nlink;
  int32_t           st_uid;
  int32_t           st_gid;
  unsigned long st_rdev;
  unsigned long st_pad1[3];     /* Reserved for st_rdev expansion  */
  long long     st_size;
  /*
   * Actually this should be timestruc_t st_atime, st_mtime and st_ctime
   * but we don't have it under Linux.
   */
  long          st_atim;
  unsigned long reserved0;      /* Reserved for st_atime expansion  */
  long          st_mtim;
  unsigned long reserved1;      /* Reserved for st_mtime expansion  */
  long          st_ctim;
  unsigned long reserved2;      /* Reserved for st_ctime expansion  */
  unsigned long st_blksize;
  unsigned long st_pad2;
  long long     st_blocks;
};
  
void statToGlibcStat64(const struct stat *statptr, GlibcStat64 *glibcStat64ptr){
  glibcStat64ptr->st_dev     = SWAP_WORD(statptr->st_dev);
  glibcStat64ptr->st_ino     = SWAP_LONG((unsigned long long)statptr->st_ino);
  glibcStat64ptr->st_mode    = SWAP_WORD(statptr->st_mode);
  glibcStat64ptr->st_nlink   = SWAP_WORD(statptr->st_nlink);
  glibcStat64ptr->st_uid     = SWAP_WORD(statptr->st_uid);
  glibcStat64ptr->st_gid     = SWAP_WORD(statptr->st_gid);
  glibcStat64ptr->st_rdev    = SWAP_WORD(statptr->st_rdev);
  glibcStat64ptr->st_size    = SWAP_LONG((unsigned long long)statptr->st_size);
  glibcStat64ptr->st_atim    = SWAP_WORD(statptr->st_atime);
  glibcStat64ptr->st_mtim    = SWAP_WORD(statptr->st_mtime);
  glibcStat64ptr->st_ctim    = SWAP_WORD(statptr->st_ctime);
  glibcStat64ptr->st_blksize = SWAP_WORD(statptr->st_blksize);
  glibcStat64ptr->st_blocks  = SWAP_LONG((unsigned long long)statptr->st_blocks);
}

SysCallFileIO::OpenFileVector SysCallFileIO::openFiles;

#include "ReportGen.h"

void SysCallFileIO::staticConstructor(void){
  I(openFiles.empty());
  openFiles.resize(3,0);
  openFiles[0]=new OpenFileInfo("",0,O_RDONLY,0,0);
  char outName[]="stdout.XXXXXX";
  int32_t outFile=mkstemp(outName);
  I(outFile!=-1);
  Report::field("SysCall::stdout=%s",outName);
  I(!lseek(outFile,0,SEEK_END));
  openFiles[1]=new OpenFileInfo(outName,outFile,O_WRONLY,0,0);
  char errName[]="stderr.XXXXXX";
  int32_t errFile=mkstemp(errName);
  I(errFile!=-1);
  Report::field("SysCall::stderr=%s",errName);
  I(!lseek(errFile,0,SEEK_END));
  openFiles[2]=new OpenFileInfo(errName,errFile,O_WRONLY,0,0);
}

void SysCallFileIO::execFXStat64(ThreadContext *context,icode_ptr picode){
  MintFuncArgs args(context, picode);
  // We will completely ignore the glibc stat_ver parameter
  long statVer=args.getInt32();
  int32_t myFd=args.getInt32();
  VAddr addr=args.getVAddr();
  I(addr);
  struct stat statNative;
  fstat(myFd,&statNative);
  int32_t retVal=fstat(openFiles[myFd]->fdesc,&statNative);
  context->setRetVal(retVal);
  if(retVal==-1){   
    context->setErrno(errno);
  }else{
    if((myFd>0)&&(myFd<=2)){
      I(statNative.st_mode&S_IFREG);
      I(!(statNative.st_mode&S_IFCHR));
      statNative.st_mode&=~S_IFREG;
      statNative.st_mode|=S_IFCHR;
      I(statNative.st_rdev==0);
      statNative.st_rdev=0x8800;
    }
    GlibcStat64 statSimulated;
    statToGlibcStat64(&statNative, &statSimulated);
    rsesc_OS_write_block(context->getPid(),picode->addr,addr,
                         &statSimulated,sizeof(GlibcStat64));
  }
}

void SysCallOpen::exec(ThreadContext *context,icode_ptr picode){
  MintFuncArgs args(context, picode);
  VAddr pathnameAddr=args.getVAddr();
  int32_t flags=conv_flags_to_native(args.getInt32());
  mode_t mode=(mode_t)args.getInt32();
  // Get the file name from versioned memory
  char pathname[MAX_FILENAME_LENGTH];
  rsesc_OS_read_string(context->getPid(), picode->addr, pathname, 
		       pathnameAddr, MAX_FILENAME_LENGTH);
  int32_t realFd=open(pathname,flags,mode);
  if(realFd==-1){
    // Open failed, simulated fd is 0 and errno is set
    myFd=-1;
    context->setErrno(errno);
  }else{
    if(!executed){
      // Find first available simulated file descriptor
      for(myFd=0;((size_t)myFd<openFiles.size())&&(openFiles[myFd]);myFd++);
      if((size_t)myFd==openFiles.size())
        openFiles.resize(myFd+1,0);
    }
    I(!openFiles[myFd]);
    I(lseek(realFd,0,SEEK_CUR)==0);
    openFiles[myFd]=new OpenFileInfo(pathname,realFd,flags,mode,0);
  }
  context->setRetVal(myFd);
  executed=true;
}

void SysCallOpen::undo(bool expectRedo){
  I(executed);
  if(myFd!=-1){
    // Close file and remove from vector of open files
    int32_t err=close(openFiles[myFd]->fdesc);
    I(!err);
    delete openFiles[myFd];
    openFiles[myFd]=0;
  }
}

void SysCallClose::exec(ThreadContext *context, icode_ptr picode){
  MintFuncArgs args(context, picode);
  int32_t fd=args.getInt32();
  I((!executed)||(fd==myFd));
  myFd=fd;
  if((myFd<=2)||(openFiles.size()<=(size_t)myFd)||!openFiles[myFd]){
    // Descriptor is not that of an open file
    I((!executed)||(myInfo==0));
    myInfo=0;
    context->setErrno(EBADF);
  }else{  
    int32_t err=close(openFiles[myFd]->fdesc);
    if(err==0){
      if(executed){
	I(strcmp(myInfo->pathname,openFiles[myFd]->pathname)==0);
	I(myInfo->flags==openFiles[myFd]->flags);
	I(myInfo->mode==openFiles[myFd]->mode);
	ID(delete myInfo);
      }
      // Store info needed to reopen file in undo
      myInfo=openFiles[myFd];
      // Remove from vector of open files
      openFiles[myFd]=0;
    }else{
      I(err==-1);
      I((!executed)||(myInfo==0));
      myInfo=0;
      context->setErrno(errno);
    }
  }
  context->setRetVal(myInfo?0:-1);
  executed=true;
}

void SysCallClose::undo(bool expectRedo){
  I(executed);
  if(myInfo){
    // Reopen the file and update things accordingly 
    int32_t realFd=open(myInfo->pathname,myInfo->flags&~(O_TRUNC|O_APPEND),myInfo->mode);
    myInfo->fdesc=realFd;
    I((openFiles.size()>(size_t)myFd)&&(!openFiles[myFd]));
    openFiles[myFd]=myInfo;
    ID(if(expectRedo) myInfo=new OpenFileInfo(*(openFiles[myFd])));
    if(openFiles[myFd]->offset>0){
      off_t undoOffs=lseek(openFiles[myFd]->fdesc,openFiles[myFd]->offset,SEEK_SET);
      I(undoOffs==openFiles[myFd]->offset);
    }
  }
}

void SysCallClose::done(){
  if(myInfo)
    delete myInfo;
}

void SysCallRead::exec(ThreadContext *context,icode_ptr picode){
  MintFuncArgs args(context, picode);
  int32_t fd=args.getInt32();
  VAddr buf=args.getVAddr();
  size_t count=args.getInt32();
  I((!executed)||(fd==myFd));
  myFd=fd;
  I((openFiles.size()>(size_t)myFd)&&openFiles[myFd]);
  I((!executed)||(myBuf==buf));
  ID(myBuf=buf);
  I((!executed)||(myCount==count));
  ID(myCount=count);
  void *tempbuff=alloca(count);
  I(tempbuff);
  I((!executed)||(oldOffs==lseek(openFiles[myFd]->fdesc,0,SEEK_CUR)));
  ID(oldOffs=lseek(openFiles[myFd]->fdesc,0,SEEK_CUR));
  ssize_t nowBytesRead=read(openFiles[myFd]->fdesc,tempbuff,executed?bytesRead:count);
  I((!executed)||(nowBytesRead==bytesRead));
  bytesRead=nowBytesRead;
  context->setRetVal(bytesRead);
  if(bytesRead==-1){
    context->setErrno(errno);
    I(lseek(openFiles[myFd]->fdesc,0,SEEK_CUR)==oldOffs);
  }else{
    I(lseek(openFiles[myFd]->fdesc,0,SEEK_CUR)==oldOffs+bytesRead);
    openFiles[myFd]->offset+=bytesRead;
    rsesc_OS_write_block(context->getPid(),picode->addr,
			 buf,tempbuff,(size_t)bytesRead);
  }
  executed=true;
}
void SysCallRead::undo(bool expectRedo){
  I(executed);
  if(bytesRead!=-1){
    I((openFiles.size()>(size_t)myFd)&&openFiles[myFd]);
    off_t undoOffs=lseek(openFiles[myFd]->fdesc,-bytesRead,SEEK_CUR);
    I(undoOffs==oldOffs);
    I(lseek(openFiles[myFd]->fdesc,0,SEEK_CUR)==oldOffs);
  }
}

void SysCallWrite::exec(ThreadContext *context,icode_ptr picode){
  MintFuncArgs args(context, picode);
  int32_t fd=args.getInt32();
  VAddr buf=args.getVAddr();
  size_t count=args.getInt32();
  I((!executed)||(fd==myFd));
  myFd=fd;
  I((!executed)||(myBuf==buf));
  ID(myBuf=buf);
  I((!executed)||(myCount==count));
  ID(myCount=count);
  off_t currentOffset=lseek(openFiles[myFd]->fdesc,0,SEEK_CUR);
  if(currentOffset==-1){
    I(errno!=ESPIPE);
    if(errno==ESPIPE){
      // Non-seekable file, must buffer writes instead of undoing them
      // We can't yet handle open-ed files that are non-seekable
      I((openFiles.size()<=(size_t)myFd)||!openFiles[myFd]);
      if(executed){
	I(bytesWritten==(ssize_t)count);
	void *tempbuff=alloca(count);
	I(tempbuff);
	// Read data from versioned memory into a temporary buffer
	rsesc_OS_read_block(context->getPid(),picode->addr,
			    tempbuff,buf,count);
	I(bufData&&(memcmp(bufData,tempbuff,count)==0));
      }else{
	bufData=malloc(count);
	I(bufData);
	// Read data from versioned memory into a temporary buffer
	rsesc_OS_read_block(context->getPid(),picode->addr,
			    bufData,buf,count);
	bytesWritten=count;
      }
      context->setRetVal(bytesWritten);
    }else if(errno==EBADF){
      // Invalid file handle, fail with errno of EBADF
      context->setErrno(EBADF);
      bytesWritten=-1;
      context->setRetVal(-1);
    }else{
      I(0);
    }
  }else{
    // File is seekable so writes can be undone
    I((openFiles.size()>(size_t)myFd)&&openFiles[myFd]);
    I(!executed||!bufData);
    bufData=0;
    void *tempbuff=alloca(count);
    I(tempbuff);
    // Read data from versioned memory into a temporary buffer
    rsesc_OS_read_block(context->getPid(),picode->addr,
			tempbuff,buf,count);
    // Get current position and verify that we are in append mode
    ID(oldOffs=currentOffset);
    I(oldOffs==lseek(openFiles[myFd]->fdesc,0,SEEK_END));
    I(oldOffs==lseek(openFiles[myFd]->fdesc,0,SEEK_CUR));
    // Write to file and free the temporary buffer
    ssize_t nowBytesWritten=write(openFiles[myFd]->fdesc,tempbuff,executed?bytesWritten:count);
    I((!executed)||(nowBytesWritten==bytesWritten));
    bytesWritten=nowBytesWritten;
    context->setRetVal(bytesWritten);
    if(bytesWritten==-1){
      context->setErrno(errno);
      I(lseek(openFiles[myFd]->fdesc,0,SEEK_CUR)==oldOffs);
    }else{
      I(lseek(openFiles[myFd]->fdesc,0,SEEK_CUR)==oldOffs+bytesWritten);
      openFiles[myFd]->offset+=bytesWritten;
    }
  }
  executed=true;
}
void SysCallWrite::undo(bool expectRedo){
  I(executed);
  if(!bytesWritten){
    // Do nothing
  }else if(bufData){
    I(bytesWritten!=-1);
    if(!expectRedo)
      free(bufData);
  }else if(bytesWritten!=-1){
    I((openFiles.size()>(size_t)myFd)&&openFiles[myFd]);
    off_t currOffs=lseek(openFiles[myFd]->fdesc,-bytesWritten,SEEK_END);
    I(currOffs==oldOffs);
    int32_t err=ftruncate(openFiles[myFd]->fdesc,currOffs);
    I(!err);
    I(oldOffs==lseek(openFiles[myFd]->fdesc,0,SEEK_END));
    I(oldOffs==lseek(openFiles[myFd]->fdesc,0,SEEK_CUR));
  }
}
void SysCallWrite::done(void){
  I(!bufData);
  if(bufData){
    I(bytesWritten!=-1);    
    ssize_t nowBytesWritten=write(openFiles[myFd]->fdesc,bufData,bytesWritten);
    I(nowBytesWritten==bytesWritten);
    free(bufData);
  }
}

// Thread management system calls

#include "Epoch.h"

void SysCallSescSpawn::exec(ThreadContext *context,icode_ptr picode){
  // Arguments of the sesc_spawn call
  MintFuncArgs args(context, picode);
  VAddr entry = args.getVAddr();
  VAddr arg   = args.getVAddr();
  int32_t   flags = args.getInt32();
  // Get parent thread and spawning epoch
  tls::Epoch *oldEpoch=context->getEpoch();
  I(oldEpoch==tls::Epoch::getEpoch(context->getPid()));
  I(oldEpoch);
  Pid_t ppid=oldEpoch->getTid();
  I(entry);
  ThreadContext *childContext=0;
  tls::Thread   *childThread=0;
  tls::Epoch    *childEpoch=0;
  if(!executed){
    // Allocate a new thread for the child
    childContext=ThreadContext::newActual();
    // Process ID of the child thread
    childPid=childContext->getPid();
  }else{
    childThread=tls::Thread::getByID(static_cast<tls::ThreadID>(childPid));
    if(!childThread){
      childContext=ThreadContext::newActual(childPid);
    }else{
      childEpoch=childThread->getInitialEpoch();
    }
  }
  // The return value for the parent is the child's pid
  context->setRetVal(childPid);
  if(childContext){
    // Eerything is shared, stack is not copied
    childContext->shareAddrSpace(context,PR_SADDR,false);
    childContext->init();
    // The first instruction for the child is the entry point passed in
    childContext->setPCIcode(addr2icode(entry));
    childContext->setIntReg(IntArg1Reg,arg);
    childContext->setIntReg(IntArg2Reg,Stack_size); /* for sprocsp() */
    // In position-independent code every function expects to find
    // its own entry address in register jp (also known as t9 and R25)
    childContext->setIntReg(JmpPtrReg,entry);
    // When the child returns from the 'entry' function,
    // it will go directly to the exit() function
    childContext->setIntReg(RetAddrReg,Exit_addr);
    // The return value for the child is 0
    childContext->setRetVal(0);
    // Inform SESC of what we have done here
    osSim->eventSpawn(ppid,childPid,flags);
  }
  if(!childThread){
    // Create child's initial epoch
    childEpoch=tls::Epoch::initialEpoch(static_cast<tls::ThreadID>(childPid),oldEpoch);
  }else{
    oldEpoch->changeEpoch();
    if(!childEpoch){
      // Re-create child's initial epoch
      I(0);
    }
    childEpoch->run();
  }
  executed=true;
}

void SysCallSescSpawn::undo(bool expectRedo){
}

void SysCallSescSpawn::done(void){
}

void SysCallExit::exec(ThreadContext *context,icode_ptr picode){
  I(this);
  I((!executed)||(myThread==context->getEpoch()->getTid()));
  myThread=context->getEpoch()->getTid();
  context->getEpoch()->exitCalled();
  executed=true;
}

void SysCallExit::undo(bool expectRedo){
  I(executed);
  tls::Thread *thisThread=tls::Thread::getByID(myThread);
  I(thisThread);
  thisThread->undoExitCall();
}

void SysCallWait::exec(ThreadContext *context,icode_ptr picode){
  if(!executed){
    childThread=-1;
  }else if(childThread!=-1){
    tls::Thread *child=tls::Thread::getByID(childThread);
    if(!child)
      return;
  }
  context->getEpoch()->waitCalled();
  executed=true;
}

void SysCallWait::undo(bool expectRedo){
  I(executed);
  if(childThread==-1)
    return;
  tls::Thread *child=tls::Thread::getByID(childThread);
  if(!child)
    return;
  tls::ThreadID parentThread=child->getParentID();
  I(parentThread!=-1);
  tls::Thread *parent=tls::Thread::getByID(parentThread);
  I(parent);
  parent->undoWaitCall(child);
}

// Time-related system calls

void SysCallTimes::exec(ThreadContext *context,icode_ptr picode){
  if(!executed){
    retVal=globalClock/1024;
    tmsStruct.tms_utime=globalClock/1024;
    tmsStruct.tms_stime=0;
    tmsStruct.tms_cutime=0;
    tmsStruct.tms_cstime=0;
  }
  MintFuncArgs args(context,picode);
  VAddr buf=args.getVAddr();
  if(buf){
    rsesc_OS_write_block(context->getPid(),picode->addr,
			 buf,&tmsStruct,sizeof(tmsStruct));
  }
  context->setRetVal(retVal);
  executed=true;
}
