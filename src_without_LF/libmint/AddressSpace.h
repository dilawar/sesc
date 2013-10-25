#if !(defined ADDRESS_SPACE_H)
#define ADDRESS_SPACE_H

#include <vector>
#include <map>
#include "Addressing.h"
#include "common.h"

class AddressSpace{
  static inline size_t getPageSize(void){ return 1<<16; }
  static inline size_t getVPage(VAddr vaddr){ return vaddr/getPageSize(); }
  // Mapping of virtual to real addresses
  typedef std::vector<RAddr> PageTable;
  PageTable pageTable;
  // Mapping of function names to code addresses
  typedef std::map<char *,VAddr> NameToAddrMap;
  typedef std::multimap<VAddr,char *> AddrToNameMap;
  NameToAddrMap funcNameToAddr;
  AddrToNameMap funcAddrToName;
 public:
  AddressSpace(void);
  ~AddressSpace(void);
  void addReference(void);
  void delReference(void);
  // Creates real memory for a range of virtual memory addresses
  void newRMem(VAddr begVAddr, VAddr endVAddr);
  // Deletes real memory for a range of virtual memory addresses
  void delRMem(VAddr begVAddr, VAddr endVAddr);
  // Finds a unmapped virtual memory range of a given size,
  // starting at the beginning of the address space
  VAddr findVMemLow(size_t memSize);
  // Finds a unmapped virtual memory range of a given size, 
  // starting at the end of the address space
  VAddr findVMemHigh(size_t memSize);
  // Maps a virtual address to a real address
  RAddr virtToReal(VAddr vaddr) const{
    size_t vPage=getVPage(vaddr);
    RAddr  rPageBase=pageTable[vPage];
    if(!rPageBase)
      return 0;
    return rPageBase+(vaddr-vPage*getPageSize());
  }
  // Maps a real address to a virtual address (if any) in this address space
  VAddr realToVirt(RAddr raddr) const{
    return 0;
  }
  // Add a new function name-address mapping
  void addFuncName(const char *name, VAddr addr);
};

#endif
