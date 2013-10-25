#if !(defined ADDRESS_SPACE_H)
#define ADDRESS_SPACE_H

#include <unistd.h>
#include <vector>
#include <set>
#include <map>
#include <list>
#include "Addressing.h"
//#include "CvtEndian.h"
#include "InstDesc.h"
#include "common.h"
#include "nanassert.h"
#include "GCObject.h"
#include "DbgObject.h"
#include "Checkpoint.h"
#include "FileSys.h"
#include "MemState.h"

typedef size_t PageNum;

#define AddrSpacPageOffsBits (12)
#define AddrSpacPageSize     (1<<AddrSpacPageOffsBits)
#define AddrSpacPageOffsMask (AddrSpacPageSize-1)

typedef uint64_t MemAlignType;

namespace MemSys{

  // Information about a page of physical memory
  class FrameDesc : public GCObject{
  public:
    typedef SmartPtr<FrameDesc> pointer;
  private:
    typedef std::set<PAddr> PAddrSet;
    static PAddr    nextPAddr;
    static PAddrSet freePAddrs;
    static inline PAddr newPAddr(void){
      PAddr retVal;
      if(nextPAddr){
	retVal=nextPAddr;
	nextPAddr+=AddrSpacPageSize;
      }else{
	PAddrSet::iterator it=freePAddrs.begin();
	if(it==freePAddrs.end())
	  fail("FrameDesc::newPAddr ran out of physical address space\n");
	retVal=*it;
	freePAddrs.erase(it);
      }
      return retVal;
    }
    PAddr    basePAddr;
    bool     shared;
    bool     dirty;
    FileSys::SeekableDescription::pointer  fileDesc;
    off_t                          fileOff;
    struct FileMapKey{
      FileSys::SeekableDescription *fileDesc;
      off_t                fileOff;
      FileMapKey(FileSys::SeekableDescription *fileDesc, off_t fileOff) : fileDesc(fileDesc), fileOff(fileOff){
      }
      inline bool operator<(const FileMapKey &other) const{
	if(fileDesc<other.fileDesc)
	  return true;
	if(fileDesc>other.fileDesc)
	  return false;
	return (fileOff<other.fileOff);
      }
    };
    typedef  std::map<FileMapKey,FrameDesc *> FileToFrame;
    static FileToFrame fileToFrame;
    // Private constructor, used by the public create(fs,offs) method
    FrameDesc(FileSys::SeekableDescription *fdesc, off_t offs);
    
    MemAlignType data[AddrSpacPageSize/sizeof(MemAlignType)];
#if (defined HAS_MEM_STATE)
    MemState state[AddrSpacPageSize/MemState::Granularity];
#endif
  public:
    template<class T>
    inline T read(VAddr addr) const{
      size_t offs=(addr&AddrSpacPageOffsMask);
      return *(reinterpret_cast<const T *>(&(reinterpret_cast<const int8_t *>(data)[offs])));
    }
    template<class T>
    inline void write(VAddr addr, T val){
      dirty=true;
      size_t offs=(addr&AddrSpacPageOffsMask);
      *(reinterpret_cast<T *>(&(reinterpret_cast<int8_t *>(data)[offs])))=val;
    }
    inline bool isShared(void) const{
      return shared;
    }
    FrameDesc();
    FrameDesc(FrameDesc &src);
    ~FrameDesc();
    int8_t *getData(VAddr addr){
      return reinterpret_cast<int8_t *>(data)+(addr&AddrSpacPageOffsMask);
    }
    PAddr getPAddr(VAddr addr) const{
      return basePAddr+(addr&AddrSpacPageOffsMask);
    }
#if (defined HAS_MEM_STATE)
    MemState &getState(VAddr addr){
      size_t offs=(addr&AddrSpacPageOffsMask)/MemState::Granularity;
      I(offs*sizeof(MemState)<sizeof(state));
      return state[offs];
    }
    const MemState &getState(VAddr addr) const{
      size_t offs=(addr&AddrSpacPageOffsMask)/MemState::Granularity;
      I(offs*sizeof(MemState)<sizeof(state));
      return state[offs];
    }
#endif
    void save(ChkWriter &out) const;
    FrameDesc(ChkReader &in);
    // Returns a frame that has a shared mapping to the given file and offset 
    static FrameDesc *create(FileSys::SeekableDescription *fdesc, off_t offs);
    void sync(void);
  };

  
}

class AddressSpace : public GCObject{
 public:
  typedef SmartPtr<AddressSpace> pointer;
 private:
  class SegmentDesc{
  public:
    // Starting adress and length of the segment
    VAddr  addr;
    size_t len;
    // Protections for this segment
    bool   canRead;
    bool   canWrite;
    bool   canExec;
    // Does the segment autogrow and in which direction
    bool autoGrow;
    bool growDown;
    // Does this segment correspond to a shared mapping
    bool shared;
    // Points to the file from which this segment is mapped
    FileSys::SeekableDescription::pointer fileDesc;
    // If mapped from a file, this is the offset in the file from which the mapping came
    off_t fileOffset;
    SegmentDesc &operator=(const SegmentDesc &src){
      addr=src.addr;
      len=src.len;
      canRead =src.canRead;
      canWrite=src.canWrite;
      canExec =src.canExec;
      autoGrow=src.autoGrow;
      growDown=src.growDown;
      shared=src.shared;
      fileDesc=src.fileDesc;
      fileOffset=src.fileOffset;
      return *this;
    }
    // Page number (not address) of the first page that overlaps with this segment
    size_t pageNumLb(void){
      return (addr>>AddrSpacPageOffsBits);
    }
    // Page number (not address) of the first page after this segment with no overlap with it
    size_t pageNumUb(void){
      I(len);
      return ((addr+len+AddrSpacPageSize-1)>>AddrSpacPageOffsBits);
    }
    void save(ChkWriter &out) const;
    ChkReader &operator=(ChkReader &in);
  };
  typedef std::map<VAddr, SegmentDesc, std::greater<VAddr> > SegmentMap;
  SegmentMap segmentMap;
  // Information about pages of virtual memory (in each AddressSpace)
  class PageDesc
#if (defined DEBUG_PageDesc)
 : public DbgObject<PageDesc>
#endif
  {
    typedef enum{
      CanRead =  1,
      CanWrite=  2,
      CanExec =  4,
      Shared  =  8,
      // WrCopy is set if a copy-on-write check is needed before we write
      WrCopy  = 16
    } Flags;
    Flags flags;
    void copyFrame(void);
    void doWrCopy(void);
  public:
    MemSys::FrameDesc::pointer frame;
    template<class T>
    inline T read(VAddr addr) const{
      if(!(flags&CanRead))
        fail("PageDesc::read from non-readable page\n");
      return frame->read<T>(addr);
    }
    template<class T>
    inline void write(VAddr addr, T val){
      if(!(flags&CanWrite))
        fail("PageDesc::write from non-writeable page\n");
      if(flags&WrCopy)
	doWrCopy();
      return frame->write<T>(addr,val);
    }
    template<class T>
    inline T fetch(VAddr addr) const{
      if(!(flags&CanExec))
        fail("PageDesc::read from non-readable page\n");
      return frame->read<T>(addr);
    }
    void map(bool r, bool w, bool x, bool s, FileSys::SeekableDescription *fdesc, off_t offs){
      I(!frame);
      I(!flags);
      frame=fdesc?(MemSys::FrameDesc::create(fdesc,offs)):(new MemSys::FrameDesc());
      frameTable.insert(FrameTableEntry(frame,this));
      flags=static_cast<Flags>((r?CanRead:0)|(w?CanWrite:0)|(x?CanExec:0)|(s?Shared:(frame->isShared()?WrCopy:0)));
    }
    void protect(bool r, bool w, bool x){
      flags=static_cast<Flags>((r?CanRead:0)|(w?CanWrite:0)|(x?CanExec:0)|(flags&~(CanRead|CanWrite|CanExec)));
    }
    bool canRead(void) const{ return (flags&CanRead); }
    bool canWrite(void) const{ return (flags&CanWrite); }
    bool canExec(void) const{ return (flags&CanExec); }
    PageDesc(void);
    PageDesc(PageDesc &src);
    PageDesc(const PageDesc &src);
    PageDesc &operator=(PageDesc &src);
    ~PageDesc(void);
    MemSys::FrameDesc *getFrame(void) const{
      return frame;
    }
    void save(ChkWriter &out) const;
    ChkReader &operator=(ChkReader &in);
  };
  class PageTable{
    typedef std::map<size_t,PageDesc> PageMap;
    PageMap pageMap;
    class Cache{
    public:
      struct Entry{
	PageNum   pageNum;
	PageDesc *pageDesc;
	Entry(void) : pageNum(0), pageDesc(0){
	}
      };
    private:
      static const PageNum AddrSpaceCacheSize=(1<<16);
      Entry cache[AddrSpaceCacheSize];
    public:
      Cache(void);
      inline Entry &operator[](PageNum pageNum){
	return cache[pageNum%AddrSpaceCacheSize];
      }
      void unmap(PageNum pageNumLb, PageNum pageNumUb);
    };
    Cache cache;
  public:
    PageTable(void);
    PageTable(PageTable &src);
    inline PageDesc &operator[](PageNum pageNum){
      Cache::Entry &centry=cache[pageNum];
      if(centry.pageNum==pageNum)
	return *(centry.pageDesc);
      PageDesc &entry=pageMap[pageNum];
      centry.pageNum=pageNum;
      centry.pageDesc=&entry;
      return entry;
    }
    inline bool isMapped(PageNum pageNum) const{
      return (pageMap.find(pageNum)!=pageMap.end());
    }
    inline const PageDesc &operator[](PageNum pageNum) const{
      I(isMapped(pageNum));
      PageMap::const_iterator it=pageMap.find(pageNum);
      return it->second;
    }
    void map(PageNum pageNumLb, PageNum pageNumUb,
             bool r, bool w, bool x, bool s,
             FileSys::SeekableDescription *fdesc, off_t offs);
    void unmap(PageNum pageNumLb, PageNum pageNumUb);
  };
  PageTable pageTable;
  // For each frame, the frame table says which pages map to it
  struct FrameTableEntry{
    MemSys::FrameDesc *frame;
    PageDesc          *page;
    FrameTableEntry(MemSys::FrameDesc *frame, PageDesc *page) : frame(frame), page(page){
    }
    inline bool operator<(const FrameTableEntry &other) const{
      if(frame<other.frame)
        return true;
      if(frame>other.frame)
        return false;
      return (page<other.page);
    }
  };
  typedef std::set<FrameTableEntry> FrameTable;
  static FrameTable frameTable;

  static inline size_t getPageNum(VAddr addr){
    return (addr>>AddrSpacPageOffsBits);
  }
  static inline size_t getPageOff(VAddr addr){
    return (addr&AddrSpacPageOffsMask);
  }
  static inline size_t getPageNumLb(VAddr addr){
    return (addr>>AddrSpacPageOffsBits);
  }
  static inline size_t getPageNumUb(VAddr addr){
    return ((addr+AddrSpacPageSize-1)>>AddrSpacPageOffsBits);
  }
  VAddr brkBase;
 public:
  static inline size_t getPageSize(void){
    return AddrSpacPageSize;
  }
  static inline VAddr pageAlignDown(VAddr addr){
    return alignDown(addr,AddrSpacPageSize);    
  }
  static inline VAddr pageAlignUp(VAddr addr){
    return alignUp(addr,AddrSpacPageSize);    
  }
  // Returns true iff the specified block does not ovelap with any allocated segment
  // and can be used to allocate a new segment or extend an existing one
  bool isNoSegment(VAddr addr, size_t len) const{
    I(addr>0);
    if(addr+len<addr)
      return false;
    SegmentMap::const_iterator segIt=segmentMap.upper_bound(addr+len);
    if(segIt==segmentMap.end())
      return true;
    const SegmentDesc &segDesc=segIt->second;
    return (segDesc.addr+segDesc.len<=addr);
  }
  // Returns true iff the specified block is entirely within the same allocated segment
  bool isInSegment(VAddr addr, size_t len) const{
    SegmentMap::const_iterator segIt=segmentMap.lower_bound(addr);
    return (segIt!=segmentMap.end())&&(segIt->second.addr+segIt->second.len>=addr+len);
  }
  // Returns true iff the specified block exactly matches an allocated segment
  bool isSegment(VAddr addr, size_t len) const{
    SegmentMap::const_iterator segIt=segmentMap.find(addr);
    return (segIt!=segmentMap.end())&&(segIt->second.len==len);
  }
  void setBrkBase(VAddr addr){
    brkBase=addr;
  }
  VAddr getBrkBase(void) const{
    I(brkBase);
    return brkBase;
  }
  void setGrowth(VAddr addr, bool autoGrow, bool growDown){
    I(segmentMap.find(addr)!=segmentMap.end());
    if(autoGrow){
      I(segmentMap[addr].addr==alignDown(segmentMap[addr].addr,AddrSpacPageSize));
      I(segmentMap[addr].len==alignDown(segmentMap[addr].len,AddrSpacPageSize));
    }
    segmentMap[addr].autoGrow=autoGrow;
    segmentMap[addr].growDown=growDown;
  }
  // Splits a segment into two, one that ends at the pivot and one that begins there
  // The pivot must be within an existing segment
  void splitSegment(VAddr pivot);
  void newSegment(VAddr addr, size_t len, bool canRead, bool canWrite, bool canExec, bool shared=false,
		  FileSys::SeekableDescription *fdesc=0, off_t offs=0);
  void protectSegment(VAddr addr, size_t len, bool canRead, bool canWrite, bool canExec);
  VAddr newSegmentAddr(size_t len);
  void deleteSegment(VAddr addr, size_t len);
  void resizeSegment(VAddr addr, size_t len);
  void growSegmentDown(VAddr oldaddr, VAddr newaddr);
  void moveSegment(VAddr oldaddr, VAddr newaddr);
  VAddr getSegmentAddr(VAddr addr) const{
    SegmentMap::const_iterator segIt=segmentMap.lower_bound(addr);
    I(segIt!=segmentMap.end());
    I(segIt->second.addr+segIt->second.len>addr);
    return segIt->second.addr;
  }
  size_t getSegmentSize(VAddr addr) const{
    SegmentMap::const_iterator segIt=segmentMap.find(addr);
    I(segIt!=segmentMap.end());
    return segIt->second.len;
  }
  // Returns true iff the entire specified block is mapped
  bool isMapped(VAddr addr, size_t len) const{
    for(PageNum pageNum=getPageNumLb(addr);pageNum<getPageNumUb(addr+len);pageNum++)
      if(!pageTable.isMapped(pageNum))
        return false;
    return true;
  }
  // Returns true iff the entire specified block is readable
  bool canRead(VAddr addr, size_t len) const{
    for(PageNum pageNum=getPageNumLb(addr);pageNum<getPageNumUb(addr+len);pageNum++)
      if((!pageTable.isMapped(pageNum))||(!pageTable[pageNum].canRead()))
        return false;
    return true;
  }
  // Returns true iff the entire specified block is writeable
  bool canWrite(VAddr addr, size_t len) const{
    for(PageNum pageNum=getPageNumLb(addr);pageNum<getPageNumUb(addr+len);pageNum++)
      if((!pageTable.isMapped(pageNum))||(!pageTable[pageNum].canWrite()))
        return false;
    return true;
  }
  // Returns true iff the entire specified block is executable
  bool canExec(VAddr addr, size_t len) const{
    for(PageNum pageNum=getPageNumLb(addr);pageNum<getPageNumUb(addr+len);pageNum++)
      if((!pageTable.isMapped(pageNum))||(!pageTable[pageNum].canExec()))
        return false;
    return true;
  }
private:
  // Information about decoded instructions for a page of virtual memory
  class TraceDesc{
  public:
    InstDesc *binst;
    InstDesc *einst;
    VAddr     baddr;
    VAddr     eaddr;
  };
  typedef std::map<VAddr, TraceDesc, std::greater<VAddr> > TraceMap;
  TraceMap traceMap;
  class InstTable{
    typedef std::map<VAddr, InstDesc *> InstMap;
    InstMap  instMap;
    class Cache{
    public:
      struct Entry{
	VAddr     instAddr;
	InstDesc *instDesc;
	Entry(void) : instAddr(0), instDesc(0){
	}
      };
    private:
      static const size_t AddrSpaceCacheSize=(1<<16);
      Entry cache[AddrSpaceCacheSize];
    public:
      Cache(void);
      inline Entry &operator[](VAddr instAddr){
	return cache[instAddr%AddrSpaceCacheSize];
      }
      void unmap(VAddr instAddrLb, VAddr instAddrUb);
    };
    Cache cache;
  public:
    InstTable(void);
    inline InstDesc *operator[](VAddr instAddr){
      Cache::Entry &centry=cache[instAddr];
      if(centry.instAddr==instAddr)
	return centry.instDesc;
      InstMap::iterator it=instMap.find(instAddr);
      if(it==instMap.end())
	return 0;
      centry.instAddr=instAddr;
      centry.instDesc=it->second;
      return it->second;
    }
    inline void map(VAddr instAddr, InstDesc *instDesc){
      I(instMap.find(instAddr)==instMap.end());
      instMap[instAddr]=instDesc;
    }
    void unmap(VAddr instAddrLb, VAddr instAddrUb);
  };
  InstTable instTable;
  // The InstMap is sorted by virtual address from lowest to highest
  typedef std::map<VAddr, InstDesc *> InstMap;
  InstMap  instMap;
public:
  void createTrace(ThreadContext *context, VAddr addr);
  void mapTrace(InstDesc *binst, InstDesc *einst, VAddr baddr, VAddr eaddr);
  void delInsts(VAddr begAddr, VAddr endAddr);
  inline void mapInst(VAddr addr,InstDesc *inst){
    instTable.map(addr,inst);
  }
  inline InstDesc *virtToInst(VAddr addr){
    return instTable[addr];
  }
 public:
  AddressSpace(void);
  AddressSpace(AddressSpace &src);
  ~AddressSpace(void);
  // Saves this address space to a stream
  void save(ChkWriter &out) const;
  AddressSpace(ChkReader &in);
  template<class T>
  inline T read(VAddr addr){
    I(pageTable[getPageNum(addr)].canRead());
    I(canRead(addr,sizeof(T)));
    return pageTable[getPageNum(addr)].read<T>(addr);
  }
  template<class T>
  inline void write(VAddr addr, T val){
    I(pageTable[getPageNum(addr)].canWrite());
    I(canWrite(addr,sizeof(T)));
    return pageTable[getPageNum(addr)].write<T>(addr,val);
  }
  template<class T>
  inline T fetch(VAddr addr){
    I(pageTable[getPageNum(addr)].canExec());
    I(canExec(addr,sizeof(T)));
    return pageTable[getPageNum(addr)].fetch<T>(addr);
  }
//  template<class T>
//  inline T readMemRaw(VAddr addr){
//    size_t pageNum=getPageNum(addr);
//    I(addr+sizeof(T)<=(pageNum+1)*AddrSpacPageSize);
//    PageDesc &myPage=pageTable[pageNum];
//    I(myPage.canRead);
//#if (defined EMUL_VALGRIND)
//    if(*(reinterpret_cast<T *>(myPage.frame->getData(addr)))==0)
//      if(addr==0)
//	printf("Never\n");
//#endif
//    return *(reinterpret_cast<T *>(myPage.frame->getData(addr)));
//  }
//  template<class T>
//  inline bool readMemRaw(VAddr addr, T &val){
//    size_t pageNum=getPageNum(addr);
//    I(addr+sizeof(T)<=(pageNum+1)*AddrSpacPageSize);
//    PageDesc &myPage=pageTable[pageNum];
//    if(!myPage.canRead)
//      return false;
//#if (defined EMUL_VALGRIND)
//    if(*(reinterpret_cast<T *>(myPage.frame->getData(addr)))==0)
//      if(addr==0)
//	printf("Never\n");
//#endif
//    val=*(reinterpret_cast<T *>(myPage.frame->getData(addr)));
//    return true;
//  }
//  template<class T>
//  inline bool writeMemRaw(VAddr addr, T val){
//    size_t pageNum=getPageNum(addr);
//    I(addr+sizeof(T)<=(pageNum+1)*AddrSpacPageSize);
//    PageDesc &myPage=pageTable[pageNum];
//    if(!myPage.canWrite)
//      return false;
//    if(myPage.copyOnWrite){
//      myPage.copyOnWrite=false;
//      if(myPage.frame->getRefCount()>1){
//        MemSys::FrameDesc *frame=new MemSys::FrameDesc(*myPage.frame);
//        frameTable.erase(FrameTableEntry(myPage.frame,&myPage));
//	myPage.frame=frame;
//        frameTable.insert(FrameTableEntry(frame,&myPage));
//      }
//    }
//#if (defined EMUL_VALGRIND)
//    if(val==0)
//      if(addr==0)
//	printf("Never\n");
//#endif
//    *(reinterpret_cast<T *>(myPage.frame->getData(addr)))=val;
//    return true;
//  }
#if (defined HAS_MEM_STATE)
/*   inline const MemState &getState(VAddr addr) const{ */
/*     size_t pageNum=getPageNum(addr); */
/*     I(addr<(pageNum+1)*AddrSpacPageSize); */
/*     const PageDesc &myPage=getPageDesc(pageNum); */
/*     return myPage.frame->getState(addr); */
/*   } */
  inline MemState &getState(VAddr addr){
    size_t pageNum=getPageNum(addr);             
    I(addr<(pageNum+1)*AddrSpacPageSize);                         
    PageDesc &myPage=getPageDesc(pageNum);
    if(myPage.copyOnWrite){
      myPage.copyOnWrite=false;
      if(myPage.frame->getRefCount()>1)
	myPage.frame=new MemSys::FrameDesc(*myPage.frame);
    }
    return myPage.frame->getState(addr);
  }
#endif
  //
  // Mapping of function names to code addresses
  //
 private:
  struct NameEntry{
    VAddr       addr;
    std::string func;
    std::string file;
    NameEntry(VAddr addr) : addr(addr), func(), file(){
    }
    NameEntry(VAddr addr, const std::string &func, const std::string &file) : addr(addr), func(func), file(file){
    }
    struct ByAddr{
      bool operator()(const NameEntry &e1, const NameEntry &e2) const{
	if(e1.addr!=e2.addr)
	  return (e1.addr>e2.addr);
	if(e1.func!=e2.func)
	  return (e1.func<e2.func);
	return (e1.file<e2.file);
      }
    };
    struct ByName{
      bool operator()(const NameEntry *e1, const NameEntry *e2) const{
	if(e1->func!=e2->func)
	  return (e1->func<e2->func);
	if(e1->file!=e2->file)
	  return (e1->file<e2->file);
	return (e1->addr<e2->addr);
      }
    };
  };
  typedef std::set< NameEntry, NameEntry::ByAddr >   NamesByAddr;
  NamesByAddr namesByAddr;
  typedef std::set< const NameEntry *, NameEntry::ByName > NamesByName;
  NamesByName namesByName;
 public:
  // Add a new function name-address mapping
  void addFuncName(VAddr addr, const std::string &func, const std::string &file);
  // Removes all existing function name mappings in a given address range
  void delFuncNames(VAddr begAddr, VAddr endAddr);
  // Return name of the function with given entry point
  const std::string &getFuncName(VAddr addr) const;
  // Return name of the ELF file in which the function is
  const std::string &getFuncFile(VAddr addr) const;
  // Given the name, return where the function begins
  VAddr getFuncAddr(const std::string &name) const;
  // Given a code address, return where the function begins (best guess)
  VAddr getFuncAddr(VAddr addr) const;
  // Given a code address, return the function size (best guess)
  size_t getFuncSize(VAddr addr) const;
  // Print name(s) of function(s) with given entry point
  void printFuncName(VAddr addr) const;
  //
  // Interception of function calls
  //
 public:
  typedef std::list<EmulFunc *> HandlerSet;
  //  typedef std::set<EmulFunc *> HandlerSet;
 private:
/*   struct ltstr{ */
/*     bool operator()(const char *s1, const char *s2) const{ */
/*       return (strcmp(s1,s2)<0); */
/*     } */
/*   }; */
/*   typedef std::multimap<const char *,EmulFunc *,ltstr> NameToFuncMap; */
  typedef std::map<std::string,HandlerSet> NameToFuncMap;
  static NameToFuncMap nameToCallHandler;
  static NameToFuncMap nameToRetHandler;
  static void addHandler(const std::string &name, NameToFuncMap &map, EmulFunc *func);
  static void delHandler(const std::string &name, NameToFuncMap &map, EmulFunc *func);
  static bool getHandlers(const std::string &name, const NameToFuncMap &map, HandlerSet &set);
 public:
  static void addCallHandler(const std::string &name,EmulFunc *func);
  static void addRetHandler(const std::string &name,EmulFunc *func);
  static void delCallHandler(const std::string &name,EmulFunc *func);
  static void delRetHandler(const std::string &name,EmulFunc *func);
  bool getCallHandlers(VAddr addr, HandlerSet &set) const;
  bool getRetHandlers(VAddr addr, HandlerSet &set) const;
};
#endif
