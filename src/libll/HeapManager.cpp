#include "HeapManager.h"
#include "ReportGen.h"

HeapManager::HeapManager(VAddr base, size_t size)
  : refCount(0),
    heapAddrLb(base), heapAddrUb(base+size),
    usedAddrLb(base), usedAddrUb(base){
  // The base and the size need to be non-zero multiples of MinBlockSize
  I(base&&!(base&MinBlockMask));
  I(size&&!(size&MinBlockMask));
  BlockInfo *blockInfo=new BlockInfo(base,size);
  freeByAddr.insert(BlockInfo(base,size));
  freeBySize.insert(BlockInfo(base,size));
}

HeapManager::~HeapManager(void){
  Report::field("HeapManager:maxHeapSize=0x%08lx",usedAddrUb-usedAddrLb);         
}

VAddr HeapManager::allocate(size_t size)
{
  size_t blockSize=roundUp(size);
  BlocksBySize::iterator sizeIt=freeBySize.lower_bound(BlockInfo(0,blockSize));
  if(sizeIt==freeBySize.end())
    return 0;
  VAddr blockBase=sizeIt->addr;
  size_t  foundSize=sizeIt->size;
  I(foundSize>=blockSize);
  BlocksByAddr::iterator addrIt=freeByAddr.find(BlockInfo(blockBase,foundSize));
  freeBySize.erase(sizeIt);
  freeByAddr.erase(addrIt);
  busyByAddr.insert(BlockInfo(blockBase,size));
  if(foundSize>blockSize){
    freeBySize.insert(BlockInfo(blockBase+blockSize,foundSize-blockSize));
    freeByAddr.insert(BlockInfo(blockBase+blockSize,foundSize-blockSize));
  }
  if(blockBase+blockSize>(size_t)usedAddrUb)
    usedAddrUb=blockBase+blockSize;
  return blockBase;
}

VAddr HeapManager::allocate(VAddr addr, size_t size)
{
  size_t blockSize=roundUp(size);
  // Find block with next strictly higher address, then go to block before that
  BlocksByAddr::iterator addrIt=freeByAddr.upper_bound(BlockInfo(addr,blockSize));
  if(addrIt==freeByAddr.begin()){
    // All available blocks are at higher addresses than what we want
    return allocate(size);
  }
  addrIt--;
  VAddr foundAddr=addrIt->addr;
  // Start of the block should be no higher than what we want
  I(foundAddr<=addr);
  size_t foundSize=addrIt->size;
  // End of the block should be no lower than what we want
  if(foundAddr+foundSize<addr+blockSize){
    // Block at requested location is not long enough
    return allocate(size);
  }
  BlocksBySize::iterator sizeIt=freeBySize.find(*addrIt);
  I(sizeIt!=freeBySize.end());
  I((sizeIt->addr==foundAddr)&&(sizeIt->size==foundSize));
  freeByAddr.erase(addrIt);
  freeBySize.erase(sizeIt);
  if(foundAddr<addr){
    size_t frontChop=addr-foundAddr;
    freeBySize.insert(BlockInfo(foundAddr,frontChop));
    freeByAddr.insert(BlockInfo(foundAddr,frontChop));
    foundSize-=frontChop;
    foundAddr+=frontChop;
    I(foundAddr==addr);
  }
  I(foundSize>=blockSize);
  if(foundSize>blockSize){
    size_t backChop=foundSize-blockSize;
    freeBySize.insert(BlockInfo(foundAddr+blockSize,backChop));
    freeByAddr.insert(BlockInfo(foundAddr+blockSize,backChop));
    foundSize-=backChop;
    I(foundSize==blockSize);
  }
  busyByAddr.insert(BlockInfo(addr,blockSize));
  if(addr+blockSize>(size_t)usedAddrUb)
    usedAddrUb=addr+blockSize;
  return addr;
}

size_t HeapManager::deallocate(VAddr addr)
{
  // Find block in the busy set and remove it
  BlocksByAddr::iterator busyIt=busyByAddr.find(BlockInfo(addr,0));
  I(busyIt!=busyByAddr.end());
  VAddr blockAddr=busyIt->addr;
  size_t  oldBlockSize=busyIt->size;
  size_t  blockSize=roundUp(oldBlockSize);
  I(blockAddr==addr);
  busyByAddr.erase(busyIt);
  BlocksByAddr::iterator addrIt=freeByAddr.upper_bound(BlockInfo(blockAddr,0));
  I((addrIt==freeByAddr.end())||(blockAddr+(VAddr)blockSize<=addrIt->addr));
  // Try to merge with the next free block
  if((addrIt!=freeByAddr.end())&&(blockAddr+(VAddr)blockSize==addrIt->addr)){
    blockSize+=addrIt->size;
    freeBySize.erase(*addrIt);
    freeByAddr.erase(addrIt);
    // Erasing from a set invalidates iterators, so reinitialize addrIt
    addrIt=freeByAddr.upper_bound(BlockInfo(blockAddr,0));
    I((addrIt==freeByAddr.end())||(blockAddr+(VAddr)blockSize<addrIt->addr));
  }
  // Try to merge with the previous free block
  if(addrIt!=freeByAddr.begin()){
    addrIt--;
    I(addrIt->addr+(VAddr)(addrIt->size)<=blockAddr);
    if(addrIt->addr+(VAddr)(addrIt->size)==blockAddr){
      blockAddr=addrIt->addr;
      blockSize+=addrIt->size;
      freeBySize.erase(*addrIt);
      freeByAddr.erase(addrIt);
    }
  }
  freeBySize.insert(BlockInfo(blockAddr,blockSize));
  freeByAddr.insert(BlockInfo(blockAddr,blockSize));
  return oldBlockSize;
}
