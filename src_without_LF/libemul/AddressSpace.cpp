#include <sys/mman.h>
#include "AddressSpace.h"
#include "nanassert.h"

namespace MemSys{

  PAddr FrameDesc::nextPAddr=AddrSpacPageSize;

  FrameDesc::PAddrSet FrameDesc::freePAddrs;

  FrameDesc::FileToFrame FrameDesc::fileToFrame;
  
  FrameDesc *FrameDesc::create(FileSys::SeekableDescription *fdesc, off_t offs){
    FileMapKey key(fdesc,offs);
    FileToFrame::iterator it=fileToFrame.find(key);
    if(it!=fileToFrame.end())
      return it->second;
    FrameDesc *ptr=new FrameDesc(fdesc,offs);
    fileToFrame[key]=ptr;
    return ptr;
  }
  void FrameDesc::sync(void){
    if(dirty){
      if(fileDesc)
	fileDesc->msync(data,AddrSpacPageSize,fileOff);
      dirty=false;
    }
  }
  FrameDesc::FrameDesc() : GCObject(), basePAddr(newPAddr()), shared(false), dirty(false), fileDesc(0){
    memset(data,0,AddrSpacPageSize);
  }
  FrameDesc::FrameDesc(FileSys::SeekableDescription *fdesc, off_t offs)
    : GCObject()
    , basePAddr(newPAddr())
    , shared(true)
    , dirty(false)
    , fileDesc(fdesc)
    , fileOff(offs){
    if(offs%AddrSpacPageSize)
      fail("FrameDesc file mapping offset is not page-aligned\n");
    fileDesc->mmap(data,AddrSpacPageSize,fileOff);
#if (defined HAS_MEM_STATE)
    for(size_t s=0;s<AddrSpacPageSize/MemState::Granularity;s++)
      state[s]=src.state[s];
#endif
  }
  FrameDesc::FrameDesc(FrameDesc &src)
    : GCObject()
    ,basePAddr(newPAddr())
    ,shared(false)
    ,dirty(false)
  {
    memcpy(data,src.data,AddrSpacPageSize);
#if (defined HAS_MEM_STATE)
    for(size_t s=0;s<AddrSpacPageSize/MemState::Granularity;s++)
      state[s]=src.state[s];
#endif
  }
  FrameDesc::~FrameDesc(){
    sync();
    if(fileDesc)
      fileToFrame.erase(FileMapKey(fileDesc,fileOff));
    I(freePAddrs.find(basePAddr)==freePAddrs.end());
    freePAddrs.insert(basePAddr);
    memset(data,0xCC,AddrSpacPageSize);
  }
  void FrameDesc::save(ChkWriter &out) const{
    out.write(reinterpret_cast<const char *>(data),AddrSpacPageSize);
#if (defined HAS_MEM_STATE)
    for(size_t s=0;s<AddrSpacPageSize/MemState::Granularity;s++)
      state[s].save(out);
#endif
    out<<endl;
  }
  FrameDesc::FrameDesc(ChkReader &in){
    in.read(reinterpret_cast<char *>(data),AddrSpacPageSize);
#if (defined HAS_MEM_STATE)
    for(size_t s=0;s<AddrSpacPageSize/MemState::Granularity;s++)
      state[s]=MemState(in);
#endif
    in>>endl;
  }

}

AddressSpace::PageTable::Cache::Cache(void){
  for(size_t i=0;i<AddrSpaceCacheSize;i++)
    cache[i].pageNum=0;  
}
void AddressSpace::PageTable::Cache::unmap(PageNum pageNumLb, PageNum pageNumUb){
  if(AddrSpaceCacheSize<(pageNumUb-pageNumLb)){
    for(size_t i=0;i<AddrSpaceCacheSize;i++){
      if((cache[i].pageNum>=pageNumLb)&&(cache[i].pageNum<pageNumUb))
	cache[i].pageNum=0;
    }
  }else{
    for(PageNum pageNum=pageNumLb;pageNum<pageNumUb;pageNum++){
      Entry &entry=cache[pageNum%AddrSpaceCacheSize];
      if(entry.pageNum==pageNum)
	entry.pageNum=0;
    }
  }
}
AddressSpace::PageTable::PageTable(void)
  : pageMap(), cache(){
}
AddressSpace::PageTable::PageTable(PageTable &src)
  : pageMap(), cache(){
  for(PageMap::iterator it=src.pageMap.begin();it!=src.pageMap.end();it++)
    pageMap[it->first]=it->second;
}
void AddressSpace::PageTable::map(PageNum pageNumLb, PageNum pageNumUb,
                                  bool r, bool w, bool x, bool s,
                                  FileSys::SeekableDescription *fdesc, off_t offs){
  for(PageNum pageNum=pageNumLb;pageNum<pageNumUb;pageNum++){
    I(pageMap.find(pageNum)==pageMap.end());
    pageMap[pageNum].map(r,w,x,s,fdesc,offs+AddrSpacPageSize*(pageNum-pageNumLb));
  }
}
void AddressSpace::PageTable::unmap(PageNum pageNumLb, PageNum pageNumUb){
  pageMap.erase(pageMap.lower_bound(pageNumLb),pageMap.lower_bound(pageNumUb));
  cache.unmap(pageNumLb,pageNumUb);
}

AddressSpace::FrameTable AddressSpace::frameTable;

void AddressSpace::SegmentDesc::save(ChkWriter &out) const{
  out << "Addr " << addr << " Len " << len;
  out << "R" << canRead;
  out << "W" << canWrite;
  out << "X" << canExec;
  out << "G" << autoGrow;
  out << "D" << growDown;
  out << "S" << shared;
//  fileStatus->save(out);
  out << "Offs " << fileOffset;
  out << endl;
}
ChkReader &AddressSpace::SegmentDesc::operator=(ChkReader &in){
  in >> "Addr " >> addr >> " Len " >> len;
  in >> "R" >> canRead;
  in >> "W" >> canWrite;
  in >> "X" >> canExec;
  in >> "G" >> autoGrow;
  in >> "D" >> growDown;  
  in >> "S" >> shared;
//  fileStatus=new FileSys::FileStatus(in);
  in >> "Offs " >> fileOffset;
  in >> endl;
  return in;
}

AddressSpace::PageDesc::PageDesc(void)
  : flags(static_cast<Flags>(0)),
    frame(0){
}
AddressSpace::PageDesc::PageDesc(PageDesc &src){
  fail("Copy constructor called\n");
}
AddressSpace::PageDesc::PageDesc(const PageDesc &src)
  : flags(src.flags),
    frame(src.frame)
{
  if(frame||flags)
    fail("Const copy constructor called!\n");
}
AddressSpace::PageDesc::~PageDesc(void){
  if(frame)
    frameTable.erase(FrameTableEntry(frame,this));
}
void AddressSpace::PageDesc::copyFrame(void){
//  if(frame->getRefCount()<=1)
//    fail("Copying uniquely-mapped frame\n");
  frameTable.erase(FrameTableEntry(frame,this));
  frame=new MemSys::FrameDesc(*frame);
  frameTable.insert(FrameTableEntry(frame,this));
}
void AddressSpace::PageDesc::doWrCopy(void){
  flags=static_cast<Flags>(flags&~WrCopy);
  if(frame->isShared())
    return copyFrame();
  FrameTable::iterator it=frameTable.lower_bound(FrameTableEntry(frame,0));
  if(it->page!=this)
    return copyFrame();
  it++;
  if((it!=frameTable.end())&&(it->frame==frame))
    return copyFrame();
}
AddressSpace::PageDesc::PageDesc &AddressSpace::PageDesc::operator=(PageDesc &src){
  flags=src.flags;
  if(frame)
    fail("PageDesc::operator= dst already has a frame!\n");
  frame=src.frame;
  if(!frame)
    fail("PageDesc::operator= src has no frame!\n");
  if(frame)
    frameTable.insert(FrameTableEntry(frame,this));
  if(!(flags&Shared)){
    FrameTable::iterator it=frameTable.lower_bound(FrameTableEntry(frame,0));
    while((it!=frameTable.end())&&(it->frame==frame)){
      PageDesc *pg=it->page;
      if(!(pg->flags&Shared))
	pg->flags=static_cast<Flags>(pg->flags|WrCopy);
      it++;
    }
  }
  return *this;
}
void AddressSpace::PageDesc::save(ChkWriter &out) const{
//  out << "S" << shared << " C" << copyOnWrite << " ";
  out.writeobj(getFrame());
}
ChkReader &AddressSpace::PageDesc::operator=(ChkReader &in){
//  insts=0;
//  canRead=0;
//  canWrite=0;
//  canExec=0;
//  in >> "S" >> shared >> " C" >> copyOnWrite >> " ";
  frame=in.readobj<MemSys::FrameDesc>();
  return in;
}

AddressSpace::InstTable::Cache::Cache(void){
  for(size_t i=0;i<AddrSpaceCacheSize;i++)
    cache[i].instAddr=0;
}
void AddressSpace::InstTable::Cache::unmap(VAddr instAddrLb, VAddr instAddrUb){
  if(AddrSpaceCacheSize<(instAddrUb-instAddrLb)){
    for(size_t i=0;i<AddrSpaceCacheSize;i++){
      if((cache[i].instAddr>=instAddrLb)&&(cache[i].instAddr<instAddrUb))
	cache[i].instAddr=0;
    }
  }else{
    for(VAddr instAddr=instAddrLb;instAddr<instAddrUb;instAddr++){
      Entry &entry=cache[instAddr%AddrSpaceCacheSize];
      if(entry.instAddr==instAddr)
	entry.instAddr=0;
    }
  }
}
AddressSpace::InstTable::InstTable(void)
  : instMap(), cache(){
}
void AddressSpace::InstTable::unmap(VAddr instAddrLb, VAddr instAddrUb){
  InstMap::iterator instItLb=instMap.lower_bound(instAddrLb);
  InstMap::iterator instItUb=instMap.lower_bound(instAddrUb);
  instMap.erase(instItLb,instItUb);
  cache.unmap(instAddrLb,instAddrUb);
}
void AddressSpace::createTrace(ThreadContext *context, VAddr addr){
  VAddr segAddr=getSegmentAddr(addr);
  VAddr segSize=getSegmentSize(segAddr);
  VAddr funcAddr=getFuncAddr(addr);
  VAddr funcSize;
  if(funcAddr){
    funcSize=getFuncSize(funcAddr);
    if((addr<funcAddr)||(addr>=funcAddr+funcSize))
      fail("createTrace: addr not within its function\n");
    if((segAddr>funcAddr)||(segAddr+segSize<funcAddr+funcSize))
      fail("createTrace: func not within its segment\n");
  }else{
    funcAddr=addr;
    funcSize=0;
  }
  if((addr<segAddr)||(addr>=segAddr+segSize))
    fail("createTrace: addr not within its segment\n");
  decodeTrace(context,funcAddr,funcSize);
}
void AddressSpace::mapTrace(InstDesc *binst, InstDesc *einst, VAddr baddr, VAddr eaddr){
  I((traceMap.upper_bound(eaddr)==traceMap.end())||(traceMap.upper_bound(eaddr)->second.eaddr<=baddr));
  TraceDesc &tdesc=traceMap[baddr];
  tdesc.binst=binst;
  tdesc.einst=einst;
  tdesc.baddr=baddr;
  tdesc.eaddr=eaddr;
}
void AddressSpace::delInsts(VAddr begAddr, VAddr endAddr){
  // We erase all traces that overlap and extend bounds to cover these traces
  TraceMap::iterator trcItLb=traceMap.upper_bound(endAddr);
  if(trcItLb==traceMap.end())
    return;
  if(trcItLb->second.eaddr<=begAddr)
    return;
  if(trcItLb->second.eaddr>endAddr)
    endAddr=trcItLb->second.eaddr;
  TraceMap::iterator trcItUb=trcItLb;
  while((trcItUb!=traceMap.end())&&(trcItUb->second.eaddr>begAddr)){
    if(trcItUb->second.baddr<begAddr)
      begAddr=trcItUb->second.baddr;
    delete [] trcItUb->second.binst;
    trcItUb++;
  }
  traceMap.erase(trcItLb,trcItUb);
  // TODO: Check if any thread is pointing to one of these insts (should never happen, but we should check)
  // Delete mapped instructions from traces we erased
  instTable.unmap(begAddr,endAddr);
}

AddressSpace::AddressSpace(void) :
  GCObject(),
  brkBase(0)
{
}

AddressSpace::AddressSpace(AddressSpace &src) :
  GCObject(),
  segmentMap(src.segmentMap),
  pageTable(src.pageTable),
  brkBase(src.brkBase)
{
  for(NamesByAddr::const_iterator it=src.namesByAddr.begin();it!=src.namesByAddr.end();it++)
    addFuncName(it->addr,it->func,it->file);
}

AddressSpace::~AddressSpace(void){
  for(TraceMap::iterator traceit=traceMap.begin();traceit!=traceMap.end();traceit++)
    delete [] traceit->second.binst;
}

void AddressSpace::save(ChkWriter &out) const{
  out << "BrkBase " << brkBase <<endl;
  // Page dump, dumps only pages that have any physical mapping
//   for(size_t _pageNum=0;_pageNum<AddrSpacPageNumCount;_pageNum++){
//     if(!pageMap[_pageNum].frame)
//       continue;
//     out << _pageNum << " ";
//     pageMap[_pageNum].save(out);
//   }
//   // Page number zero signals end of page dump
//   out << 0 << endl;
  // Segment dump
  out << "Segments " << segmentMap.size() << endl;
  for(SegmentMap::const_iterator segIt=segmentMap.begin();segIt!=segmentMap.end();segIt++)
    segIt->second.save(out);
  // Dump function name mappings
//   out << "FuncNames " << funcAddrToName.size() << endl;
//   for(AddrToNameMap::const_iterator addrIt=funcAddrToName.begin();
//       addrIt!=funcAddrToName.end();addrIt++){
//     const std::string &file=getFuncFile(addrIt->first);
//     I(file!="");
//     out << addrIt->first << 
//            " -> " << strlen(addrIt->second) << " " << addrIt->second <<
//       " : " << file.length() << " " << file << endl;
//   }
}

AddressSpace::AddressSpace(ChkReader &in){
  in >> "BrkBase " >> brkBase >> endl;
  while(true){
    size_t _pageNum;
    in >> _pageNum;
    if(!_pageNum)
      break;
    in >> " ";
    pageTable[_pageNum]=in;
  }
  in >> endl;
  size_t _segCount;
  in >> "Segments " >> _segCount >> endl;
  for(size_t i=0;i<_segCount;i++){
    SegmentDesc seg;
    seg=in;
    newSegment(seg.addr,seg.len,seg.canRead,seg.canWrite,seg.canExec,seg.shared,seg.fileDesc,seg.fileOffset);
    setGrowth(seg.addr,seg.autoGrow,seg.growDown);
  }
  // Load function name mappings
//   size_t _funcAddrToName;
//   in >> "FuncNames " >> _funcAddrToName >> endl;
//   for(size_t i=0;i<_funcAddrToName;i++){
//     VAddr _addr; size_t _strlen;
//     in >> _addr >> " -> " >> _strlen >> " ";
//     char func[_strlen+1];
//     in >> func >> " : " >> _strlen >> " ";
//     char file[_strlen+1];
//     in >> file >> endl;
//     addFuncName(_addr,func,file);
//   }
}

// Add a new function name-address mapping
void AddressSpace::addFuncName(VAddr addr, const std::string &func, const std::string &file){
  std::pair<NamesByAddr::iterator,bool> ins=namesByAddr.insert(NameEntry(addr,func,file));
  if(ins.second)
    namesByName.insert(&(*(ins.first)));
}

// Removes all existing function name mappings in a given address range
void AddressSpace::delFuncNames(VAddr begAddr, VAddr endAddr){
  NamesByAddr::iterator begIt=namesByAddr.upper_bound(endAddr);
  NamesByAddr::iterator endIt=begIt;
  while(endIt->addr>=begAddr){
    namesByName.erase(&(*endIt));
    endIt++;
  }
  namesByAddr.erase(begIt,endIt);
}


// Return name of the function with given entry point
const std::string &AddressSpace::getFuncName(VAddr addr) const{
  NamesByAddr::const_iterator nameIt=namesByAddr.lower_bound(addr);
  if(nameIt==namesByAddr.end())
    fail("");
  if(nameIt->addr!=addr)
    fail("");
  return nameIt->func;
}

// Return name of the ELF file in which the function is, given the entry point
const std::string &AddressSpace::getFuncFile(VAddr addr) const{
  NamesByAddr::const_iterator nameIt=namesByAddr.lower_bound(addr);
  if(nameIt==namesByAddr.end())
    fail("");
  if(nameIt->addr!=addr)
    fail("");
  return nameIt->func;
}

// Given the name, return where the function begins
VAddr AddressSpace::getFuncAddr(const std::string &name) const{
  NameEntry key(0,name,"");
  NamesByName::const_iterator nameIt=namesByName.lower_bound(&key);
  if(nameIt==namesByName.end())
    return 0;
  if((*nameIt)->func!=name)
    return 0;
  return (*nameIt)->addr;
}

// Given a code address, return where the function begins (best guess)
VAddr AddressSpace::getFuncAddr(VAddr addr) const{
  NamesByAddr::const_iterator nameIt=namesByAddr.lower_bound(addr);
  if(nameIt==namesByAddr.end())
    fail("");
  return nameIt->addr;
}

// Given a code address, return the function size (best guess)
size_t AddressSpace::getFuncSize(VAddr addr) const{
  NamesByAddr::const_iterator nameIt=namesByAddr.lower_bound(addr);
  I(nameIt!=namesByAddr.end());
  VAddr abeg=nameIt->addr;
  do{
    nameIt--;
    I(nameIt!=namesByAddr.end());
  }while(nameIt->addr==abeg);
  VAddr aend=nameIt->addr;
  return aend-abeg;
}

// Print name(s) of function(s) with given entry point
void AddressSpace::printFuncName(VAddr addr) const{
  NamesByAddr::const_iterator it=namesByAddr.lower_bound(addr);
  bool first=true;
  while((it!=namesByAddr.end())&&(it->addr==addr)){
    std::cout << (first?"":", ") << it->file << ":" << it->func;
    first=false;
    it++;
  }
}

void AddressSpace::addHandler(const std::string &name, NameToFuncMap &map, EmulFunc *func){
  map[name].push_back(func);
  //  map.insert(NameToFuncMap::value_type(name.c_str(),func));
}
void AddressSpace::delHandler(const std::string &name, NameToFuncMap &map, EmulFunc *func){
  HandlerSet::iterator curIt=map[name].begin();
  HandlerSet::iterator endIt=map[name].end();
  while((curIt!=endIt)&&(*curIt!=func))
    curIt++;
  if(curIt==endIt)
    return;
  map[name].erase(curIt);
  if(!map.count(name))
    map.erase(name);
//   NameToFuncMap::iterator begIt=map.lower_bound(name.c_str());
//   NameToFuncMap::iterator endIt=map.upper_bound(name.c_str());
//   NameToFuncMap::iterator curIt=begIt;
//   while((curIt!=endIt)&&(curIt->second!=func))
//     curIt++;
//   if(curIt==endIt)
//     return;
//   map.erase(curIt);
}
bool AddressSpace::getHandlers(const std::string &name, const NameToFuncMap &map, HandlerSet &set){
  NameToFuncMap::const_iterator it=map.find(name);
  if(it==map.end())
    return false;
  const HandlerSet &srcSet=it->second;
  set.insert(set.end(),srcSet.begin(),srcSet.end());
//   NameToFuncMap::const_iterator handBeg=map.lower_bound(name.c_str());
//   NameToFuncMap::const_iterator handEnd=map.upper_bound(name.c_str());
//   for(NameToFuncMap::const_iterator handCur=handBeg;handCur!=handEnd;handCur++)
//     set.push_back(handCur->second);
  //    set.insert(handCur->second);
  return true;
}
AddressSpace::NameToFuncMap AddressSpace::nameToCallHandler;
AddressSpace::NameToFuncMap AddressSpace::nameToRetHandler;
void AddressSpace::addCallHandler(const std::string &name,EmulFunc *func){
  addHandler(name,nameToCallHandler,func);
}
void AddressSpace::addRetHandler(const std::string &name,EmulFunc *func){
  addHandler(name,nameToRetHandler,func);
}
void AddressSpace::delCallHandler(const std::string &name,EmulFunc *func){
  delHandler(name,nameToCallHandler,func);
}
void AddressSpace::delRetHandler(const std::string &name,EmulFunc *func){
  delHandler(name,nameToRetHandler,func);
}
bool AddressSpace::getCallHandlers(VAddr addr, HandlerSet &set) const{
  bool rv=false;
  for(NamesByAddr::iterator it=namesByAddr.lower_bound(addr);(it!=namesByAddr.end())&&(it->addr==addr);it++)
    rv=rv||getHandlers(it->func,nameToCallHandler,set);
  return rv||getHandlers("",nameToCallHandler,set);
}
bool AddressSpace::getRetHandlers(VAddr addr, HandlerSet &set) const{
  bool rv=false;
  for(NamesByAddr::iterator it=namesByAddr.lower_bound(addr);(it!=namesByAddr.end())&&(it->addr==addr);it++)
    rv=rv||getHandlers(it->func,nameToRetHandler,set);
  return rv||getHandlers("",nameToRetHandler,set);
}

VAddr AddressSpace::newSegmentAddr(size_t len){
  VAddr retVal=alignDown((VAddr)0x7fffffff,AddrSpacPageSize);
  retVal=alignDown(retVal-len,AddrSpacPageSize);
  for(SegmentMap::const_iterator segIt=segmentMap.begin();segIt!=segmentMap.end();segIt++){
    const SegmentDesc &curSeg=segIt->second;
    if(retVal>=curSeg.addr+curSeg.len)
      break;
    if(alignUp(len+AddrSpacPageSize,AddrSpacPageSize)>curSeg.addr){
      retVal=0;
    }else{
      retVal=alignDown(curSeg.addr-len,AddrSpacPageSize);
    }
  }
  return retVal;
}
void AddressSpace::splitSegment(VAddr pivot){
  if(pivot%AddrSpacPageSize)
    fail("AddressSpace::splitSegment with non-aligned pivot\n");
  // Find segment that begins below pivot - this will be the one to split
  SegmentMap::iterator segIt=segmentMap.upper_bound(pivot);
  // Return if no such segment
  if(segIt==segmentMap.end())
    return;
  SegmentDesc &oldSeg=segIt->second;
  // If segment ends at or below pivot, no split needed
  if(oldSeg.addr+oldSeg.len<=pivot)
    return;
  I(!(pivot%AddrSpacPageSize));
  SegmentDesc &newSeg=segmentMap[pivot];
  newSeg=oldSeg;
  newSeg.addr=pivot;
  newSeg.len=oldSeg.addr+oldSeg.len-newSeg.addr;
  oldSeg.len=oldSeg.len-newSeg.len;
  if(oldSeg.fileDesc)
    newSeg.fileOffset=oldSeg.fileOffset+oldSeg.len;
  I((newSeg.pageNumLb()+1==oldSeg.pageNumUb())||(newSeg.pageNumLb()==oldSeg.pageNumUb()));
  if(newSeg.pageNumLb()!=oldSeg.pageNumUb())
    fail("AddressSpace::splitSegment pageNumLb and pageNumUb disagree\n");
}
void AddressSpace::growSegmentDown(VAddr oldaddr, VAddr newaddr){
  if(newaddr%AddrSpacPageSize)
    fail("AddressSpace::resizeSegment with non-aligned newlen\n");
  I(!(newaddr%AddrSpacPageSize));
  I(segmentMap.find(oldaddr)!=segmentMap.end());
  I(newaddr<oldaddr);
  SegmentDesc &oldSeg=segmentMap[oldaddr];
  if(oldSeg.fileDesc)
    fail("Growing a file-mapped segment down\n");
  I(isNoSegment(newaddr,oldaddr-newaddr));
  SegmentDesc &newSeg=segmentMap[newaddr];
  newSeg=oldSeg;
  newSeg.addr=newaddr;
  newSeg.len=oldaddr+oldSeg.len-newaddr;
  I(newSeg.pageNumUb()==oldSeg.pageNumUb());
  pageTable.map(newSeg.pageNumLb(),oldSeg.pageNumLb(),newSeg.canRead,newSeg.canWrite,newSeg.canExec,newSeg.shared,0,0);
  segmentMap.erase(oldaddr);
}
void AddressSpace::resizeSegment(VAddr addr, size_t newlen){
  if(newlen%AddrSpacPageSize)
    fail("AddressSpace::resizeSegment with non-aligned newlen\n");
  I(segmentMap.find(addr)!=segmentMap.end());
  I(newlen>0);
  SegmentDesc &mySeg=segmentMap[addr];
  if(mySeg.fileDesc)
    fail("Resizing a file-mapped segment\n");
  I(mySeg.addr==addr);
  size_t oldPageNumUb=mySeg.pageNumUb();
  mySeg.len=newlen;
  size_t newPageNumUb=mySeg.pageNumUb();
  if(oldPageNumUb<newPageNumUb){
    pageTable.map(oldPageNumUb,newPageNumUb,mySeg.canRead,mySeg.canWrite,mySeg.canExec,mySeg.shared,0,0);
  }else if(oldPageNumUb>newPageNumUb){
    pageTable.unmap(newPageNumUb,oldPageNumUb);
  }
}
void AddressSpace::moveSegment(VAddr oldaddr, VAddr newaddr){
  I(!(newaddr%AddrSpacPageSize));
  I(segmentMap.find(oldaddr)!=segmentMap.end());
  SegmentDesc &oldSeg=segmentMap[oldaddr];
  I(!oldSeg.fileDesc);
  I(isNoSegment(newaddr,oldSeg.len));
  SegmentDesc &newSeg=segmentMap[newaddr];
  newSeg=oldSeg;
  newSeg.addr=newaddr;
  size_t oldPg=oldSeg.pageNumLb();
  size_t newPg=newSeg.pageNumLb();
  while(oldPg<oldSeg.pageNumUb()){
    I(newPg<newSeg.pageNumUb());
    PageDesc &oldPageDesc=pageTable[oldPg];
    PageDesc &newPageDesc=pageTable[newPg];
    newPageDesc=oldPageDesc;
    oldPg++;
    newPg++;
  }
  I(newPg==newSeg.pageNumUb());
  pageTable.unmap(oldSeg.pageNumLb(),oldSeg.pageNumUb());
  segmentMap.erase(oldaddr);
}
void AddressSpace::deleteSegment(VAddr addr, size_t len){
  if(addr%AddrSpacPageSize)
    fail("AddressSpace::deleteSegment with non-aligned addr\n");
  if(len%AddrSpacPageSize)
    fail("AddressSpace::deleteSegment with non-aligned len\n");
  splitSegment(addr);
  splitSegment(addr+len);
  // Find the segment that begins before the end of deletion region
  SegmentMap::iterator begIt=segmentMap.upper_bound(addr+len);
  // If nothing begins before the end of deletion, nothing to do
  if(begIt==segmentMap.end())
    return;
  // This segment should not go past the end of deletion (we did the split)
  I(begIt->second.addr+begIt->second.len<=addr+len);
  // Find the segment that begins before the start of deletion region
  SegmentMap::iterator endIt=segmentMap.upper_bound(addr);
  // This segment should not go past the start of deletion (we did the split)
  I((endIt==segmentMap.end())||(endIt->second.addr+endIt->second.len<=addr));
  for(SegmentMap::iterator segIt=begIt;segIt!=endIt;segIt++){
    SegmentDesc &seg=segIt->second;
    I((seg.addr>=addr)&&(seg.addr+seg.len<=addr+len));
    pageTable.unmap(seg.pageNumLb(),seg.pageNumUb());
  }
  segmentMap.erase(begIt,endIt);
#if (defined DEBUG)
  for(SegmentMap::iterator segIt=segmentMap.begin();segIt!=segmentMap.end();segIt++){
    SegmentDesc &seg=segIt->second;    
    I((seg.addr+seg.len<=addr)||(seg.addr>=addr+len));
  }
#endif
  // Remove function name mappings for this segment
  delFuncNames(addr,addr+len);
}
void AddressSpace::newSegment(VAddr addr, size_t len, bool canRead, bool canWrite, bool canExec, bool shared,
			      FileSys::SeekableDescription *fdesc, off_t offs){
  if(addr%AddrSpacPageSize)
    fail("AddressSpace::newSegment with non-aligned addr\n");
  I(len>0);
  if(!isNoSegment(addr,alignUp(len,AddrSpacPageSize)))
    fail("AddressSpace::newSegment end-overlap with another segment\n");
  SegmentDesc &newSeg=segmentMap[addr];
  newSeg.addr=addr;
  newSeg.len=len;
  newSeg.canRead=canRead;
  newSeg.canWrite=canWrite;
  newSeg.canExec=canExec;
  newSeg.autoGrow=false;
  newSeg.growDown=false;
  newSeg.shared=shared;
  newSeg.fileDesc=shared?fdesc:0;
  newSeg.fileOffset=shared?offs:0;
  pageTable.map(newSeg.pageNumLb(),newSeg.pageNumUb(),canRead,canWrite,canExec,shared,fdesc,offs);
}
void AddressSpace::protectSegment(VAddr addr, size_t len, bool canRead, bool canWrite, bool canExec){
  splitSegment(addr);
  splitSegment(addr+len);
  I(isSegment(addr,len));
  SegmentDesc &mySeg=segmentMap[addr];
  I(mySeg.addr==addr);
  I(mySeg.len==len);
  I(!mySeg.fileDesc);
  for(size_t pageNum=mySeg.pageNumLb();pageNum<mySeg.pageNumUb();pageNum++)
    pageTable[pageNum].protect(canRead,canWrite,canExec);
  mySeg.canRead=canRead;
  mySeg.canWrite=canWrite;
  mySeg.canExec=canExec;
}
