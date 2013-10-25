#include "AddressSpace.h"

AddressSpace::AddressSpace(void)
  // Allocate the entire page table and zero it out  
  : pageTable((1<<2)*((1<<(8*sizeof(VAddr)-2))/getPageSize()),0){
}  

AddressSpace::~AddressSpace(void){
  // Free function name strings  
  for(AddrToNameMap::iterator funcIt=funcAddrToName.begin();funcIt!=funcAddrToName.end();funcIt++)  
    free(funcIt->second);  
  // TODO: Free real memory that is still mapped, or each thread must free its own mappings
}

void AddressSpace::addReference(void){
}

void AddressSpace::delReference(void){
}

void AddressSpace::newRMem(VAddr begVAddr, VAddr endVAddr){
  begVAddr=alignDown(begVAddr,getPageSize());
  endVAddr=alignUp(endVAddr,getPageSize());
  void *realMem;
  if(posix_memalign(&realMem,getPageSize(),endVAddr-begVAddr))
    fatal("AddressSpace::newRMem could not allocate memory\n");
  for(size_t pageNum=getVPage(begVAddr);pageNum!=getVPage(endVAddr);pageNum++){
    if(pageTable[pageNum])
      fatal("AddressSpace::newRMem region overlaps with existing memory");
    pageTable[pageNum]=(RAddr)realMem+(pageNum*getPageSize()-begVAddr);
  }
}

void AddressSpace::delRMem(VAddr begVAddr, VAddr endVAddr){
  begVAddr=alignDown(begVAddr,getPageSize());
  endVAddr=alignUp(endVAddr,getPageSize());
  RAddr begRAddr=pageTable[getVPage(begVAddr)];
  free((void *)begRAddr);
  for(VAddr pageNum=getVPage(begVAddr);pageNum!=getVPage(endVAddr);pageNum++){
    if(pageTable[pageNum]!=begRAddr+(pageNum*getPageSize()-begVAddr))
      fatal("AddressSpace::delRMem region not allocated contiguously");
    pageTable[pageNum]=0;
  }
}

VAddr AddressSpace::findVMemLow(size_t memSize){
  size_t needPages=alignUp(memSize,getPageSize())/getPageSize();
  size_t foundPages=0;
  // Skip the first (zero) page, to avoid making null pointers valid
  size_t pageNum=1;
  while(foundPages<needPages){
    if(pageTable[pageNum])
      foundPages=0;
    else
      foundPages++;
    pageNum++;
    if(pageNum==pageTable.size())
      fatal("AddressSpace::findVMemLow not enough available virtual memory\n");
  }
  return (pageNum-needPages)*getPageSize();
}

VAddr AddressSpace::findVMemHigh(size_t memSize){
  size_t needPages=alignUp(memSize,getPageSize())/getPageSize();
  size_t foundPages=0;
  // Skip the last page, it creates addressing problems
  // becasue its upper-bound address is 0 due to wrap-around
  size_t pageNum=pageTable.size()-1;
  while(foundPages<needPages){
    pageNum--;
    // Can not use page zero because that would make the null pointer valid
    if(pageNum==0)
      fatal("AddressSpace::findVMemLow not enough available virtual memory\n");
    if(pageTable[pageNum]){
      foundPages=0;
    }else{
      foundPages++;
    }
  }
  return pageNum*getPageSize();
}

// Add a new function name-address mapping
void AddressSpace::addFuncName(const char *name, VAddr addr){
  char *myName=strdup(name);
  if(!myName)
    fatal("AddressSpace::addFuncName couldn't copy function name %s\n",name);
  if(funcNameToAddr.find(myName)!=funcNameToAddr.end())
    fatal("AddressSpace::addFuncName called with duplicate function name %s\n",name);
  funcNameToAddr.insert(NameToAddrMap::value_type(myName,addr));
  funcAddrToName.insert(AddrToNameMap::value_type(addr,myName));
}
