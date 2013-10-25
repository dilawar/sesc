#if !(defined ELF_OBJECT_H)
#define ELF_OBJECT_H

#include <elf.h>
#include "ThreadContext.h"

void loadElfObject(const char *fname, ThreadContext *threadContext){
  int32_t fd=open(fname,O_RDONLY);
  if(fd==-1)
      fatal("Could not open ELF file %s",fname);

  // Read the ELF header
  Elf32_Ehdr myElfHdr;
  if(read(fd,&myElfHdr,sizeof(myElfHdr))!=sizeof(myElfHdr))
    fatal("Could not read ELF header for file %s",fname);
  char elfMag[]= {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };
  if(memcmp(myElfHdr.e_ident,elfMag,sizeof(elfMag)))
    fatal("Not an ELF file: %s",fname);
  if((myElfHdr.e_ident[EI_VERSION]!=EV_CURRENT) ||
     (bigEndian(myElfHdr.e_version)!=EV_CURRENT))
    fatal("Wrong ELF version in file %s",fname);
  if(myElfHdr.e_ident[EI_CLASS]!=ELFCLASS32)
    fatal("Not a 32-bit ELF file: %s",fname);
  if(bigEndian(myElfHdr.e_ehsize)!=sizeof(myElfHdr))
    fatal("Wrong ELF header size in file %s\n",fname);
  if(myElfHdr.e_ident[EI_DATA]!=ELFDATA2MSB)
    fatal("Not a big-endian ELF file: %s",fname);
  if(bigEndian(myElfHdr.e_machine)!=EM_MIPS)
    fatal("Not a MIPS ELF file %s",fname);
  if(bigEndian(myElfHdr.e_type)!=ET_EXEC)
    fatal("Not an executable file %s",fname);
  if((bigEndian(myElfHdr.e_flags)&EF_MIPS_ARCH)>EF_MIPS_ARCH_2)
    fatal("Using architecture above MIPS2 in ELF file %s",fname);
  if(bigEndian(myElfHdr.e_flags)&(EF_MIPS_PIC|EF_MIPS_CPIC))
    fatal("File contains or uses PIC code: %s",fname);
  if(bigEndian(myElfHdr.e_phentsize)!=sizeof(Elf32_Phdr))
    fatal("Wrong ELF program table entry size in %s",fname);
  if(bigEndian(myElfHdr.e_shentsize)!=sizeof(Elf32_Shdr))
    fatal("Wrong ELF section table entry size in %s",fname);
  
  // Address space of the thread
  AddressSpace *addrSpace=threadContext->getAddressSpace();
  if(!addrSpace)
    fatal("loadElfObject: thread has no address space\n");

  // File offset to the program (segment table)
  off_t prgTabOff=bigEndian(myElfHdr.e_phoff);
  // Number of program (segment) table entries
  size_t prgTabCnt=bigEndian(myElfHdr.e_phnum);
  
  // Parse program (segment) headers first to find out where and how much
  // memory to allocate for static and bss data
  
  VAddr allocBegAddr=0;
  VAddr allocEndAddr=0;
  if(lseek(fd,prgTabOff,SEEK_SET)!=prgTabOff)
    fatal("Couldn't seek to ELF program table in %s",fname);
  else{
    for(int32_t prgTabIdx=0;prgTabIdx<prgTabCnt;prgTabIdx++){
      Elf32_Phdr myPrgHdr;
      if(read(fd,&myPrgHdr,sizeof(myPrgHdr))!=sizeof(myPrgHdr))
	fatal("Could not read ELF program table entry in %s",fname);
      if(bigEndian(myPrgHdr.p_type)==PT_LOAD){
	VAddr secBegAddr=bigEndian(myPrgHdr.p_vaddr);
	VAddr secEndAddr=secBegAddr+bigEndian(myPrgHdr.p_memsz);
	if((!allocBegAddr)||(secBegAddr<allocBegAddr))
	  allocBegAddr=secBegAddr;
	if((!allocEndAddr)||(secEndAddr>allocEndAddr))
	  allocEndAddr=secEndAddr;
      }
    }
  }
  
  
  // Create real memory for loadable segments in the thread's address space
  addrSpace->newRMem(allocBegAddr,allocEndAddr);
  
  // Now make another pass through the program (segment) table. This time:
  // 1) Load and initialize the actual data for PT_LOAD segments
  // 2) Find the initial value of the global pointer (GP) from the PT_MIPS_REGINFO segment
  // 3) Check if unsupported segment types are used

  {
    off_t currFilePtr=prgTabOff;
    for(int32_t prgTabIdx=0;prgTabIdx<prgTabCnt;prgTabIdx++){
      if(lseek(fd,currFilePtr,SEEK_SET)!=currFilePtr)
	fatal("Couldn't seek to current ELF program table position in %s",fname);
      Elf32_Phdr myPrgHdr;
      if(read(fd,&myPrgHdr,sizeof(myPrgHdr))!=sizeof(myPrgHdr))
	fatal("Could not read ELF program table entry in %s",fname);
      if((currFilePtr=lseek(fd,0,SEEK_CUR))==(off_t)-1)
	fatal("Could not get current file position in %s",fname);
      switch(bigEndian(myPrgHdr.p_type)){
      case PT_LOAD:
	{
	  VAddr  segMPos =bigEndian(myPrgHdr.p_vaddr);
	  size_t segMSize=bigEndian(myPrgHdr.p_memsz);
	  size_t segFSize=bigEndian(myPrgHdr.p_filesz);
	  off_t  segFPos =bigEndian(myPrgHdr.p_offset);
	  if(segFSize){
	    if(lseek(fd,segFPos,SEEK_SET)!=segFPos)
	      fatal("Couldn't seek to a PT_LOAD segment in ELF file %s",fname);
	    if(read(fd,(void *)(addrSpace->virtToReal(segMPos)),segFSize)!=segFSize)
	      fatal("Could not read a PT_LOAD segment in ELF file %s",fname);
	    segMPos+=segFSize;
	    segMSize-=segFSize;
	  }
	  if(segMSize){ 
	    memset((void *)(addrSpace->virtToReal(segMPos)),0,segMSize);
	  }
	}
	break;
      case PT_MIPS_REGINFO:
	{
	  if(bigEndian(myPrgHdr.p_filesz)!=sizeof(Elf32_RegInfo))
	    fatal("PT_MIPS_REGINFO section size mismatch in ELF file %s",fname);
	  Elf32_RegInfo myRegInfo;
	  off_t regInfoOff=bigEndian(myPrgHdr.p_offset);
	  if(lseek(fd,regInfoOff,SEEK_SET)!=regInfoOff)
	    fatal("Couldn't seek to ELF RegInfo segment in %s",fname);
	  if(read(fd,&myRegInfo,sizeof(myRegInfo))!=sizeof(myRegInfo))
	    fatal("Could not read ELF RegInfo segment in %s",fname);
	  if(bigEndian(myRegInfo.ri_cprmask[0]) || bigEndian(myRegInfo.ri_cprmask[2]) || bigEndian(myRegInfo.ri_cprmask[3]))
	    fatal("Unsupported coprocessor registers used in ELF file %s",fname);
	  threadContext->setGlobPtr(bigEndian(myRegInfo.ri_gp_value));
	}
	break;
      case PT_NULL: // This type of segment is ignored by definition
      case PT_NOTE: // Auxiliary info that is not needed for execution
	break;
      default:
	printf("Unsupported segment type 0x%lx ",bigEndian(myPrgHdr.p_type));
	printf("FOffs 0x%lx, VAddr 0x%lx, PAddr 0x%lx, FSize 0x%lx, MSize 0x%lx, Align 0x%lx, Flags %c%c%c + 0x%lx\n",
	       bigEndian(myPrgHdr.p_offset),
	       bigEndian(myPrgHdr.p_vaddr),
	       bigEndian(myPrgHdr.p_paddr),
	       bigEndian(myPrgHdr.p_filesz),
	       bigEndian(myPrgHdr.p_memsz),
	       bigEndian(myPrgHdr.p_align),
	       (bigEndian(myPrgHdr.p_flags)&PF_R)?'R':' ',
	       (bigEndian(myPrgHdr.p_flags)&PF_W)?'W':' ',
	       (bigEndian(myPrgHdr.p_flags)&PF_X)?'X':' ',
	       bigEndian(myPrgHdr.p_flags)^(bigEndian(myPrgHdr.p_flags)&(PF_R|PF_W|PF_X))
	       );
      }
    }
  }
  
  off_t  secTabOff=bigEndian(myElfHdr.e_shoff);
  size_t secTabCnt=bigEndian(myElfHdr.e_shnum);
  size_t secNamSec=bigEndian(myElfHdr.e_shstrndx);
  
  // Get the section name string table
  char *secNamTab=0;
  if(secNamSec==SHN_UNDEF)
    fatal("No section name string table in ELF file %s",fname);
  else{
    if(lseek(fd,secTabOff+secNamSec*sizeof(Elf32_Shdr),SEEK_SET)!=secTabOff+secNamSec*sizeof(Elf32_Shdr))
      fatal("Couldn't seek to section for section name string table in ELF file %s",fname);
    Elf32_Shdr mySecHdr;
    if(read(fd,&mySecHdr,sizeof(mySecHdr))!=sizeof(mySecHdr))
      fatal("Could not read ELF section table entry %d in %s",secNamSec,fname);
    if(bigEndian(mySecHdr.sh_type)!=SHT_STRTAB)
      fatal("Section table entry for section name string table is not of SHT_STRTAB type in ELF file %s",fname);
    off_t  secNameTabOffs=bigEndian(mySecHdr.sh_offset);
    if(lseek(fd,secNameTabOffs,SEEK_SET)!=secNameTabOffs)
      fatal("Could not seek to section name string table in ELF file %s",fname);
    size_t secNamTabSize=bigEndian(mySecHdr.sh_size);
    secNamTab=(char *)malloc(secNamTabSize);
    if(read(fd,secNamTab,secNamTabSize)!=secNamTabSize)
      fatal("Could not read the section name string table in ELF file %s",fname);
  }
    
  // Read symbol tables and their string tables
  {
    off_t secTabPos=secTabOff;
    for(int32_t secTabIdx=0;secTabIdx<secTabCnt;secTabIdx++){
      Elf32_Shdr mySecHdr;
      if(lseek(fd,secTabPos,SEEK_SET)!=secTabPos)
	fatal("Couldn't seek to current ELF section table position in %s",fname);
      if(read(fd,&mySecHdr,sizeof(mySecHdr))!=sizeof(mySecHdr))
	fatal("Could not read ELF section table entry in %s",fname);
      if((secTabPos=lseek(fd,0,SEEK_CUR))==(off_t)-1)
	fatal("Could not get current file position in %s",fname);
      if(bigEndian(mySecHdr.sh_type)==SHT_SYMTAB){
	if(bigEndian(mySecHdr.sh_entsize)!=sizeof(Elf32_Sym))
	  fatal("Symbol table has entries of wrong size in ELF file %s",fname);
	size_t symTabSize=bigEndian(mySecHdr.sh_size);
	size_t symTabOffs=bigEndian(mySecHdr.sh_offset);
	Elf32_Section strTabSec=bigEndian(mySecHdr.sh_link);
	if(lseek(fd,secTabOff+strTabSec*sizeof(mySecHdr),SEEK_SET)!=secTabOff+strTabSec*sizeof(mySecHdr))
	  fatal("Couldn't seek to string table section %d for symbol table section %d in ELF file %s",
		secTabIdx,strTabSec,fname);
	if(read(fd,&mySecHdr,sizeof(mySecHdr))!=sizeof(mySecHdr))
	  fatal("Could not read ELF section table entry %d in %s",fname,secTabIdx);
	if(bigEndian(mySecHdr.sh_type)!=SHT_STRTAB)
	  fatal("SYMTAB section %d links to non-STRTAB section %d in ELF file %s",secTabIdx,strTabSec,fname);
	size_t strTabSize=bigEndian(mySecHdr.sh_size);
	size_t strTabOffs=bigEndian(mySecHdr.sh_offset);
	if(lseek(fd,strTabOffs,SEEK_SET)!=strTabOffs)
	  fatal("Could not seek to string table for SYMTAB section %d in ELF file %s",secTabIdx,fname);
	char *strTab=(char *)malloc(strTabSize);
	if(read(fd,strTab,strTabSize)!=strTabSize)
	  fatal("Could not read string table for SYMTAB section %d in ELF file %s",secTabIdx,fname);
	if(lseek(fd,symTabOffs,SEEK_SET)!=symTabOffs)
	  fatal("Could not seek to symbol table for section %d in ELF file %s",secTabIdx,fname);
	for(int32_t symTabIdx=0;symTabIdx<symTabSize/sizeof(Elf32_Sym);symTabIdx++){
	  Elf32_Sym mySym;
	  if(read(fd,&mySym,sizeof(mySym))!=sizeof(mySym))
	    fatal("Could not read symbol in ELF file %s",fname);
	  switch(ELF32_ST_TYPE(bigEndian(mySym.st_info))){
	  case STT_FILE:   // Ignore file name symbols
	  case STT_OBJECT: // Ignore data symbols
	  case STT_SECTION:// Ignore section symbols
	  case STT_NOTYPE: // Ignore miscelaneous (no-type) symbols
	    break;
	  case STT_FUNC:   // Function entry point
	    char *funcName=strTab+bigEndian(mySym.st_name);
	    VAddr funcAddr=bigEndian(mySym.st_value);
	    if(!funcAddr)
	      fatal("Function %s has zero address in ELF file %s\n",funcName,fname);
	    addrSpace->addFuncName(funcName,funcAddr);
	    break;
	  default:
	    fatal("Unknown symbol type %d for symbol %s value %x\n",
		  ELF32_ST_TYPE(bigEndian(mySym.st_info)),
		  strTab+bigEndian(mySym.st_name),
		  bigEndian(mySym.st_value)
		  );
	  }
	}
	free(strTab);
      }
    }
  }
  
  // Decode instructions in executable sections
  // also, check for unsupported section types
  {
    off_t secTabPos=secTabOff;
    for(int32_t secTabIdx=0;secTabIdx<secTabCnt;secTabIdx++){
      Elf32_Shdr mySecHdr;
      if(lseek(fd,secTabPos,SEEK_SET)!=secTabPos)
	fatal("Couldn't seek to current ELF section table position in %s",fname);
      if(read(fd,&mySecHdr,sizeof(mySecHdr))!=sizeof(mySecHdr))
	fatal("Could not read ELF section table entry in %s",fname);
      if((secTabPos=lseek(fd,0,SEEK_CUR))==(off_t)-1)
	fatal("Could not get current file position in %s",fname);
      switch(bigEndian(mySecHdr.sh_type)){
      case SHT_NULL: // This section is unused by definition
      case SHT_NOTE:  // Auxiliary info that is not needed for execution
      case SHT_NOBITS: // BSS-type section, already initialized when we processed segments
      case SHT_MIPS_DWARF: // Debugging info, we ignore it
      case SHT_MIPS_REGINFO: // Register use info, already parsed when we processed segments
      case SHT_STRTAB: // String table, already parsed in previous section table pass
      case SHT_SYMTAB: // symbol table, already parsed in previous section table pass
	break;
      case SHT_PROGBITS:
	if(bigEndian(mySecHdr.sh_flags)&SHF_EXECINSTR){
	  // TODO: Decode instructions for this section
	}
	break;
      default:
	printf("Unsupported section type 0x%08x ",bigEndian(mySecHdr.sh_type));
	printf("FOffs 0x%08x VAddr 0x%08x Size 0x%08x Align 0x%lx Flags %c%c%c + 0x%lx",
	       bigEndian(mySecHdr.sh_offset),
	       bigEndian(mySecHdr.sh_addr),
	       bigEndian(mySecHdr.sh_size),
	       bigEndian(mySecHdr.sh_addralign),
	       (bigEndian(mySecHdr.sh_flags)&SHF_ALLOC)?'A':' ',
	       (bigEndian(mySecHdr.sh_flags)&SHF_WRITE)?'W':' ',
	       (bigEndian(mySecHdr.sh_flags)&SHF_EXECINSTR)?'X':' ',
	       bigEndian(mySecHdr.sh_flags)^(bigEndian(mySecHdr.sh_flags)&(SHF_ALLOC|SHF_WRITE|SHF_EXECINSTR))
	       );
	printf("\n");
      }
    }
  }
  
  close(fd);
  
  // TODO: Set instruction pointer to the program entry point
  VAddr entryIAddr=bigEndian(myElfHdr.e_entry);
  
}

#endif
