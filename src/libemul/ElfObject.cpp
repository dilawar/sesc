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

#include "ElfObject.h"

#include <elf.h>
#include <fcntl.h>
#include "ThreadContext.h"
//#include "CvtEndian.h"
//#include "MipsRegs.h"
// To get definition of fail()
#include "EmulInit.h"
// To get endian conversion
#include "EndianDefs.h"

template<ExecMode mode>
void cvtEndianEhdr(typename ElfDefs<mode>::Elf_Ehdr &ehdr){
  // Array e_ident is all chars and need no endian conversion
  EndianDefs<mode>::cvtEndian(ehdr.e_type);
  EndianDefs<mode>::cvtEndian(ehdr.e_machine);
  EndianDefs<mode>::cvtEndian(ehdr.e_version);
  EndianDefs<mode>::cvtEndian(ehdr.e_entry);
  EndianDefs<mode>::cvtEndian(ehdr.e_phoff);
  EndianDefs<mode>::cvtEndian(ehdr.e_shoff);
  EndianDefs<mode>::cvtEndian(ehdr.e_flags);
  EndianDefs<mode>::cvtEndian(ehdr.e_ehsize);
  EndianDefs<mode>::cvtEndian(ehdr.e_phentsize);
  EndianDefs<mode>::cvtEndian(ehdr.e_phnum);
  EndianDefs<mode>::cvtEndian(ehdr.e_shentsize);
  EndianDefs<mode>::cvtEndian(ehdr.e_shnum);
  EndianDefs<mode>::cvtEndian(ehdr.e_shstrndx);
}

template<ExecMode mode>
void cvtEndianPhdr(typename ElfDefs<mode>::Elf_Phdr &phdr){
  EndianDefs<mode>::cvtEndian(phdr.p_type);
  EndianDefs<mode>::cvtEndian(phdr.p_offset);
  EndianDefs<mode>::cvtEndian(phdr.p_vaddr);
  EndianDefs<mode>::cvtEndian(phdr.p_paddr);
  EndianDefs<mode>::cvtEndian(phdr.p_filesz);
  EndianDefs<mode>::cvtEndian(phdr.p_memsz);
  EndianDefs<mode>::cvtEndian(phdr.p_flags);
  EndianDefs<mode>::cvtEndian(phdr.p_align);
}

template<ExecMode mode>
void cvtEndianShdr(typename ElfDefs<mode>::Elf_Shdr &shdr){
  EndianDefs<mode>::cvtEndian(shdr.sh_name);
  EndianDefs<mode>::cvtEndian(shdr.sh_type);
  EndianDefs<mode>::cvtEndian(shdr.sh_flags);
  EndianDefs<mode>::cvtEndian(shdr.sh_addr);
  EndianDefs<mode>::cvtEndian(shdr.sh_offset);
  EndianDefs<mode>::cvtEndian(shdr.sh_size);
  EndianDefs<mode>::cvtEndian(shdr.sh_link);
  EndianDefs<mode>::cvtEndian(shdr.sh_info);
  EndianDefs<mode>::cvtEndian(shdr.sh_addralign);
  EndianDefs<mode>::cvtEndian(shdr.sh_entsize);
}

template<ExecMode mode>
void cvtEndianSym(typename ElfDefs<mode>::Elf_Sym &sym){
  EndianDefs<mode>::cvtEndian(sym.st_name);
  EndianDefs<mode>::cvtEndian(sym.st_value);
  EndianDefs<mode>::cvtEndian(sym.st_size);
  EndianDefs<mode>::cvtEndian(sym.st_info);
  EndianDefs<mode>::cvtEndian(sym.st_other);
  EndianDefs<mode>::cvtEndian(sym.st_shndx);
}

// Helper function for getExecMode
template<ExecMode mode>
ExecMode _getExecMode(FileSys::SeekableDescription *fdesc){
  typedef typename ElfDefs<mode>::Elf_Ehdr Elf_Ehdr;
  typedef typename ElfDefs<mode>::Elf_Phdr Elf_Phdr;
  typedef typename ElfDefs<mode>::Elf_Shdr Elf_Shdr;
  Elf_Ehdr ehdr;
  if(fdesc->pread(&ehdr,sizeof(Elf_Ehdr),(off_t)0)!=sizeof(Elf_Ehdr))
    return ExecModeNone;
  cvtEndianEhdr<mode>(ehdr);
  if(ehdr.e_version!=EV_CURRENT)
    return ExecModeNone;
  if(ehdr.e_ehsize!=sizeof(Elf_Ehdr))
    return ExecModeNone;
  if(ehdr.e_phentsize!=sizeof(Elf_Phdr))
    return ExecModeNone;
  if(ehdr.e_shentsize!=sizeof(Elf_Shdr))
    return ExecModeNone;
  switch(ehdr.e_type){
  case ET_EXEC:
  case ET_DYN:
    break;
  default: fail("e_type is not ET_EXEC or ET_DYN\n");
  }
  ExecMode mmode;
  switch(ehdr.e_machine){
  case EM_MIPS: {
    if((ehdr.e_flags&EF_MIPS_ABI)==EF_MIPS_ABI_O32){
      if((ehdr.e_flags&EF_MIPS_32BITMODE)==0)
        fail("EF_MIPS_32BITMODE not set for EF_MIPS_ABI_O32\n");
      if((ehdr.e_flags&EF_MIPS_ABI2)!=0)
        fail("EF_MIPS_ABI2 is set for EF_MIPS_ABI_O32\n");
      mmode=ExecModeArchMips;
      if((mode&ExecModeBitsMask)!=ExecModeBits32)
	fail("We have EF_MIPS_ABI_O32 but ExecModeBits32\n");
    }else if((ehdr.e_flags&EF_MIPS_ABI)==0){
      if((ehdr.e_flags&EF_MIPS_32BITMODE)!=0)
        fail("EF_MIPS_32BITMODE is set but EF_MIPS_ABI_O32 is not\n");
      if(((ehdr.e_flags&EF_MIPS_ABI2)==0)&&((mode&ExecModeBitsMask)==ExecModeBits32))
        fail("EF_MIPS_ABI2 is not set for 32-bit executable under ArchMips64\n");
      if(((ehdr.e_flags&EF_MIPS_ABI2)!=0)&&((mode&ExecModeBitsMask)==ExecModeBits64))
        fail("EF_MIPS_ABI2 is set for 64-bit executable under ArchMips64\n");
      mmode=ExecModeArchMips64;
    }else{
      fail("Unknown EF_MIPS_ABI\n");
    }
    switch(ehdr.e_flags&EF_MIPS_ARCH){
    case EF_MIPS_ARCH_1:
    case EF_MIPS_ARCH_2:
    case EF_MIPS_ARCH_3:
    case EF_MIPS_ARCH_4:
      break;
    default:
      fail("EF_MIPS_ARCH above EF_MIPS_ARCH4 was used\n");
    }
    if(ehdr.e_flags&~(EF_MIPS_ARCH|EF_MIPS_ABI|EF_MIPS_ABI2|EF_MIPS_NOREORDER|EF_MIPS_PIC|EF_MIPS_CPIC|EF_MIPS_32BITMODE))
      fail("Unknown e_machine (0x%08x) for EM_MIPS\n",ehdr.e_flags);
  } break;
  default: fail("e_machine is %d not EM_MIPS for %s\n",ehdr.e_machine,fdesc->getName().c_str());
  }
  return ExecMode(mode|mmode);
}

ExecMode getExecMode(FileSys::SeekableDescription *fdesc){
  // Read the e_ident part of the ELF header
  unsigned char e_ident[EI_NIDENT];
  if(fdesc->pread(e_ident,EI_NIDENT,(off_t)0)!=EI_NIDENT)
    return ExecModeNone;
  if((e_ident[0]!=ELFMAG0)||(e_ident[1]!=ELFMAG1)||
     (e_ident[2]!=ELFMAG2)||(e_ident[3]!=ELFMAG3))
    return ExecModeNone;
  if(e_ident[EI_VERSION]!=EV_CURRENT)
    return ExecModeNone;
  switch(e_ident[EI_OSABI]){
  case ELFOSABI_NONE:
    break;
  default:
    return ExecModeNone;
  }
  if(e_ident[EI_ABIVERSION]!=0)
    return ExecModeNone;
  ExecMode wmode=ExecModeNone;
  switch(e_ident[EI_CLASS]){
  case ELFCLASS32: wmode=ExecModeBits32; break;
  case ELFCLASS64: wmode=ExecModeBits64; break;
  default: return ExecModeNone;
  }
  ExecMode emode=ExecModeNone;
  switch(e_ident[EI_DATA]){
  case ELFDATA2LSB: emode=ExecModeEndianLittle; break;
  case ELFDATA2MSB: emode=ExecModeEndianBig; break;
  default: return ExecModeNone;
  }
  if(wmode==ExecModeBits32)
    if(emode==ExecModeEndianLittle)
      return _getExecMode<ExecMode(ExecModeBits32|ExecModeEndianLittle)>(fdesc);
    else
      return _getExecMode<ExecMode(ExecModeBits32|ExecModeEndianBig)>(fdesc);
  else
    if(emode==ExecModeEndianLittle)
      return _getExecMode<ExecMode(ExecModeBits64|ExecModeEndianLittle)>(fdesc);
    else
      return _getExecMode<ExecMode(ExecModeBits64|ExecModeEndianBig)>(fdesc);
}

template<ExecMode mode>
void _mapFuncNames(ThreadContext *context, FileSys::SeekableDescription *fdesc, VAddr addr, size_t len, off_t off){
  typedef typename ElfDefs<mode>::Elf_Ehdr Elf_Ehdr;
  typedef typename ElfDefs<mode>::Elf_Shdr Elf_Shdr;
  typedef typename ElfDefs<mode>::Elf_Sym  Elf_Sym;
  // Read in the ELF header
  Elf_Ehdr ehdr;
  ssize_t ehdrSiz=fdesc->pread(&ehdr,sizeof(Elf_Ehdr),0);
  I(ehdrSiz==sizeof(Elf_Ehdr));
  cvtEndianEhdr<mode>(ehdr);
  // Read in section headers
  Elf_Shdr shdrs[ehdr.e_shnum];
  ssize_t shdrsSiz=fdesc->pread(shdrs,sizeof(Elf_Shdr)*ehdr.e_shnum,ehdr.e_shoff);
  I(shdrsSiz==(ssize_t)(sizeof(Elf_Shdr)*ehdr.e_shnum));
  I(shdrsSiz==(ssize_t)(sizeof(shdrs)));
  for(size_t sec=0;sec<ehdr.e_shnum;sec++)
    cvtEndianShdr<mode>(shdrs[sec]);
  // Read in section name strings
  I((ehdr.e_shstrndx>0)&&(ehdr.e_shstrndx<ehdr.e_shnum));
  char secStrTab[shdrs[ehdr.e_shstrndx].sh_size];
  ssize_t secStrTabSiz=fdesc->pread(secStrTab,shdrs[ehdr.e_shstrndx].sh_size,shdrs[ehdr.e_shstrndx].sh_offset);
  I(secStrTabSiz==(ssize_t)(shdrs[ehdr.e_shstrndx].sh_size));
  // Iterate over all sections
  for(size_t sec=0;sec<ehdr.e_shnum;sec++){
    switch(shdrs[sec].sh_type){
    case SHT_PROGBITS: {
      if(!(shdrs[sec].sh_flags&SHF_EXECINSTR))
        break;
      ssize_t loadBias=(addr-shdrs[sec].sh_addr)+(shdrs[sec].sh_offset-off);
      char  *secNam=secStrTab+shdrs[sec].sh_name;
      size_t secNamLen=strlen(secNam);
      if((off_t(shdrs[sec].sh_offset)>=off)&&(shdrs[sec].sh_offset<off+len)){
        char  *begNam="SecBeg";
        size_t begNamLen=strlen(begNam);
        char symNam[secNamLen+begNamLen+1];
        strcpy(symNam,begNam);
        strcpy(symNam+begNamLen,secNam);
        VAddr symAddr=shdrs[sec].sh_addr+loadBias;
        context->getAddressSpace()->addFuncName(symAddr,symNam,fdesc->getName());
      }
      if((off_t(shdrs[sec].sh_offset+shdrs[sec].sh_size)>off)&&
         (shdrs[sec].sh_offset+shdrs[sec].sh_size<=off+len)){
        char  *endNam="SecEnd";
        size_t endNamLen=strlen(endNam);
        char symNam[secNamLen+endNamLen+1];
        strcpy(symNam,endNam);
        strcpy(symNam+endNamLen,secNam);
        VAddr symAddr=shdrs[sec].sh_addr+shdrs[sec].sh_size+loadBias;
        context->getAddressSpace()->addFuncName(symAddr,symNam,fdesc->getName());
      }
    } break;
    case SHT_SYMTAB:
    case SHT_DYNSYM: {
      I(shdrs[sec].sh_entsize==sizeof(Elf_Sym));
      // Read in the symbols
      size_t symnum=shdrs[sec].sh_size/sizeof(Elf_Sym);
      Elf_Sym syms[symnum];
      ssize_t symsSiz=fdesc->pread(syms,shdrs[sec].sh_size,shdrs[sec].sh_offset);
      I(symsSiz==(ssize_t)(sizeof(Elf_Sym)*symnum));
      I(symsSiz==(ssize_t)(sizeof(syms)));
      for(size_t sym=0;sym<symnum;sym++)
        cvtEndianSym<mode>(syms[sym]);
      // Read in the symbol name strings
      char strTab[shdrs[shdrs[sec].sh_link].sh_size];
      ssize_t strTabSiz=fdesc->pread(strTab,shdrs[shdrs[sec].sh_link].sh_size,shdrs[shdrs[sec].sh_link].sh_offset);
      I(strTabSiz==(ssize_t)(shdrs[shdrs[sec].sh_link].sh_size));
      for(size_t sym=0;sym<symnum;sym++){
        I(ELF32_ST_TYPE(syms[sym].st_info)==ELF64_ST_TYPE(syms[sym].st_info));
        switch(ELF64_ST_TYPE(syms[sym].st_info)){
        case STT_FUNC: {
          if(!syms[sym].st_shndx)
            break;
          ssize_t loadBias=(addr-shdrs[syms[sym].st_shndx].sh_addr)+
                           (shdrs[syms[sym].st_shndx].sh_offset-off);
//          printf("sh_type %s st_name %s st_value 0x%08x st_size 0x%08x st_shndx 0x%08x\n",
//                 shdrs[sec].sh_type==SHT_SYMTAB?"SYMTAB":"DYNSYM",
//                 strTab+syms[sym].st_name,
//                 (int)(syms[sym].st_value),(int)(syms[sym].st_size),(int)(syms[sym].st_shndx));
          char *symNam=strTab+syms[sym].st_name;
          VAddr symAddr=syms[sym].st_value+loadBias;
          if((symAddr<addr)||(symAddr>=addr+len))
            break;
	  context->getAddressSpace()->addFuncName(symAddr,symNam,fdesc->getName());
        }  break;
        }
      }
    }  break;
    }
  }
}

void mapFuncNames(ThreadContext *context, FileSys::SeekableDescription *fdesc,
		  ExecMode mode, VAddr addr, size_t len, off_t off){
  if((mode&ExecModeBitsMask)==ExecModeBits32)
    if((mode&ExecModeEndianMask)==ExecModeEndianLittle)
      return _mapFuncNames<ExecMode(ExecModeBits32|ExecModeEndianLittle)>(context,fdesc,addr,len,off);
    else
      return _mapFuncNames<ExecMode(ExecModeBits32|ExecModeEndianBig)>(context,fdesc,addr,len,off);
  else
    if((mode&ExecModeEndianMask)==ExecModeEndianLittle)
      return _mapFuncNames<ExecMode(ExecModeBits64|ExecModeEndianLittle)>(context,fdesc,addr,len,off);
    else
      return _mapFuncNames<ExecMode(ExecModeBits64|ExecModeEndianBig)>(context,fdesc,addr,len,off);
}

template<ExecMode mode>
VAddr _loadElfObject(ThreadContext *context, FileSys::SeekableDescription *fdesc,
                     VAddr addr, bool isInterpreter){
  typedef typename ElfDefs<mode>::Elf_Ehdr Elf_Ehdr;
  typedef typename ElfDefs<mode>::Elf_Phdr Elf_Phdr;
  // Set if this is a dynamically linked executable and we found the interpreter
  bool hasInterpreter=false;
  // Read in the ELF header
  Elf_Ehdr ehdr;
  ssize_t ehdrSiz=fdesc->pread(&ehdr,sizeof(Elf_Ehdr),0);
  cvtEndianEhdr<mode>(ehdr);
  I(ehdrSiz==sizeof(Elf_Ehdr));
  // Clear all the registers (this is actually needed for correct operation)
  context->clearRegs();
  // Set the ExecMode of the processor to match executable
  // TODO: Do this only for executable, for interpreter check if it matches
  if(isInterpreter&&(context->getMode()!=mode))
    fail("loadElfObject: executable with mode %x has interpreter with mode %x\n",
         context->getMode(),mode);
  context->setMode(mode);
  // Read in program (segment) headers
  Elf_Phdr phdrs[ehdr.e_phnum];
  ssize_t phdrsSiz=fdesc->pread(phdrs,sizeof(Elf_Phdr)*ehdr.e_phnum,ehdr.e_phoff);
  I(phdrsSiz==(ssize_t)(sizeof(Elf_Phdr)*ehdr.e_phnum));
  I(phdrsSiz==(ssize_t)(sizeof(phdrs)));
  for(size_t seg=0;seg<ehdr.e_phnum;seg++)
    cvtEndianPhdr<mode>(phdrs[seg]);
  // Iterate over all segments to load interpreter and find top and bottom of address range
  VAddr  loReqAddr=0;
  VAddr  hiReqAddr=0;
  for(size_t seg=0;seg<ehdr.e_phnum;seg++){
    switch(phdrs[seg].p_type){
    case PT_INTERP: {
      char interpName[phdrs[seg].p_filesz];
      ssize_t interpNameSiz=fdesc->pread(interpName,phdrs[seg].p_filesz,phdrs[seg].p_offset);
      I(interpNameSiz==ssize_t(phdrs[seg].p_filesz));
      const std::string exeLinkName(context->getFileSys()->toHost(interpName));
      const std::string exeRealName(FileSys::Node::resolve(exeLinkName));
      if(exeRealName.empty())
        fail("loadElfObject: Link loop when executable %s\n",exeLinkName.c_str());
      FileSys::Node *node=FileSys::Node::lookup(exeRealName);
      if(!node)
        fail("loadElfObject: Executable %s does not exist\n",exeLinkName.c_str());
      FileSys::FileNode *fnode=dynamic_cast<FileSys::FileNode *>(node);
      if(!fnode)
        fail("loadElfObject: Executable %s is not a regular file\n",exeLinkName.c_str());
      FileSys::FileDescription *fdesc=new FileSys::FileDescription(fnode,O_RDONLY);
      FileSys::Description::pointer pdesc(fdesc);
      addr=_loadElfObject<mode>(context,fdesc,addr,true);
      hasInterpreter=true;
    }  break;
    case PT_LOAD:
      if(loReqAddr==hiReqAddr){
	loReqAddr=phdrs[seg].p_vaddr;
	hiReqAddr=phdrs[seg].p_vaddr+phdrs[seg].p_memsz;
      }
      if(phdrs[seg].p_vaddr<loReqAddr)
	loReqAddr=phdrs[seg].p_vaddr;
      if(phdrs[seg].p_vaddr+phdrs[seg].p_memsz>hiReqAddr)
	hiReqAddr=phdrs[seg].p_vaddr+phdrs[seg].p_memsz;
      break;
    }
  }
  VAddr loadAddr=addr+context->getAddressSpace()->getPageSize()-1;
  loadAddr=loadAddr-(loadAddr%context->getAddressSpace()->getPageSize());
  switch(ehdr.e_type){
  case ET_EXEC: loadAddr=loReqAddr; break;
  case ET_DYN:  break;
  }
  I(context->getAddressSpace()->isNoSegment(loadAddr,hiReqAddr-loReqAddr));
  for(size_t seg=0;seg<ehdr.e_phnum;seg++){
    if(phdrs[seg].p_type!=PT_LOAD)
      continue;
    VAddr  segFilAddr=loadAddr+(phdrs[seg].p_vaddr-loReqAddr);
    size_t segFilLen=phdrs[seg].p_filesz;
    VAddr  segMapAddr=context->getAddressSpace()->pageAlignDown(segFilAddr);
    size_t segMapLen=context->getAddressSpace()->pageAlignUp((segFilAddr-segMapAddr)+phdrs[seg].p_memsz);
    // Map segment into address space
    if(!context->getAddressSpace()->isNoSegment(segMapAddr,segMapLen))
      fail("Segment overlap is loadElfObject\n");
    context->getAddressSpace()->newSegment(segMapAddr,segMapLen,false,true,false,false,fdesc,
                                           context->getAddressSpace()->pageAlignDown(phdrs[seg].p_offset));
    context->writeMemWithByte(segMapAddr,segFilAddr-segMapAddr,0);
    context->writeMemWithByte(segFilAddr+segFilLen,(segMapAddr+segMapLen)-(segFilAddr+segFilLen),0);
    context->getAddressSpace()->protectSegment(segMapAddr,segMapLen,
					       phdrs[seg].p_flags&PF_R,
					       phdrs[seg].p_flags&PF_W,
					       phdrs[seg].p_flags&PF_X);
    _mapFuncNames<mode>(context,fdesc,segFilAddr,segFilLen,phdrs[seg].p_offset);
    if((!isInterpreter)&&(phdrs[seg].p_offset<=ehdr.e_phoff)&&
       (phdrs[seg].p_offset+phdrs[seg].p_filesz>=ehdr.e_phoff+ehdr.e_phnum*sizeof(Elf_Phdr))){
      context->getAddressSpace()->addFuncName(segFilAddr+ehdr.e_phoff-phdrs[seg].p_offset,"PrgHdrAddr","");
      context->getAddressSpace()->addFuncName(sizeof(Elf_Phdr),"PrgHdrEnt" ,"");
      context->getAddressSpace()->addFuncName(ehdr.e_phnum,"PrgHdrNum","");
    }
  }
  context->getAddressSpace()->setBrkBase(loadAddr+(hiReqAddr-loReqAddr));
  VAddr entryAddr=loadAddr+(ehdr.e_entry-loReqAddr);
  // This is the entry point of the program (not the interpreter)
  if(!isInterpreter)
    context->getAddressSpace()->addFuncName(entryAddr,"UserEntry","");
  // If there is an interpreter, we enter there, otherwise we enter the program
  if(isInterpreter||!hasInterpreter)
    context->setIAddr(entryAddr);
  return loadAddr+(hiReqAddr-loReqAddr);
}

template<ExecMode bmode>
VAddr _loadElfObjectE(ThreadContext *context, FileSys::SeekableDescription *fdesc,
		      VAddr addr, ExecMode mode){
  switch(mode&ExecModeArchMask){
  case ExecModeArchMips:    return _loadElfObject<ExecMode(bmode|ExecModeArchMips)>(context,fdesc,addr,false);
  case ExecModeArchMips64:  return _loadElfObject<ExecMode(bmode|ExecModeArchMips64)>(context,fdesc,addr,false);
  defualt: fail("loadElfObject: ExecModeEndian is not Little or Big\n");
  }
  return VAddr(-1);
}
template<ExecMode bmode>
VAddr _loadElfObjectB(ThreadContext *context, FileSys::SeekableDescription *fdesc,
		      VAddr addr, ExecMode mode){
  switch(mode&ExecModeEndianMask){
  case ExecModeEndianBig:    return _loadElfObjectE<ExecMode(bmode|ExecModeEndianBig)>(context,fdesc,addr,mode);
  case ExecModeEndianLittle: return _loadElfObjectE<ExecMode(bmode|ExecModeEndianLittle)>(context,fdesc,addr,mode);
  defualt: fail("loadElfObject: ExecModeEndian is not Little or Big\n");
  }
  return VAddr(-1);
}
VAddr loadElfObject(ThreadContext *context, FileSys::SeekableDescription *fdesc, VAddr addr){
  ExecMode mode=getExecMode(fdesc);
  switch(mode&ExecModeBitsMask){
  case ExecModeBits32: return _loadElfObjectB<ExecModeBits32>(context,fdesc,addr,mode);
  case ExecModeBits64: return _loadElfObjectB<ExecModeBits64>(context,fdesc,addr,mode);
  defualt: fail("loadElfObject: ExecModeBits is not 32 or 64\n");
  }
  return VAddr(-1);
}
