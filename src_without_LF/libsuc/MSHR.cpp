/*
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by Jose Renau
                  Luis Ceze
                  James Tuck

   This file is part of SESC.

   SESC is free software; you can redistribute it and/or modify it under the terms
   of the GNU General Public License as published by the Free Software Foundation;
   either version 2, or (at your option) any later version.

   SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE.  See the GNU General Public License for more details.21

   You should  have received a copy of  the GNU General  Public License along with
   SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
   Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#define MSHR_CPP

#include "Snippets.h"
#include "MSHR.h"

// TODO: Add energy model

//
// basic MSHR class
//

template<class Addr_t, class Cache_t>
MSHR<Addr_t, Cache_t> *MSHR<Addr_t, Cache_t>::create(const char *name,
                                                     const char *type,
                                                     int32_t size,
                                                     int32_t lineSize,
                                                     int32_t nrd,
                                                     int32_t nwr,
                                                     int32_t aPolicy)
{
  MSHR *mshr;

  if(strcmp(type, "none") == 0) {
    mshr = new NoMSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy);
  } else if(strcmp(type, "nodeps") == 0) {
    mshr = new NoDepsMSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy);
  } else if(strcmp(type, "full") == 0) {
    mshr = new FullMSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy);
  } else if(strcmp(type, "single") == 0) {
    mshr = new SingleMSHR<Addr_t, Cache_t>(name, size, lineSize, nrd, nwr, aPolicy);
  } else {
    MSG("WARNING:MSHR: type \"%s\" unknown, defaulting to \"none\"", type);
    mshr = new NoMSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy);
  }

  return mshr;
}

template<class Addr_t, class Cache_t>
MSHR<Addr_t, Cache_t> *MSHR<Addr_t, Cache_t>::create(const char *name, const char *section)
{
  MSHR *mshr;
  int32_t nrd = 16;
  int32_t nwr = 0;
  int32_t aPolicy = SPECIAL;

  const char *type = SescConf->getCharPtr(section, "type");
  int32_t size = SescConf->getInt(section, "size");
  int32_t lineSize = SescConf->getInt(section, "bsize");

  const char *alloc;
  if(SescConf->checkCharPtr(section,"alloc")) {
    alloc = SescConf->getCharPtr(section,"alloc");

    if(strcmp(alloc,"conventional")==0) {
      aPolicy = CONVENTIONAL;
    }
  }

  if(SescConf->checkInt(section, "rpl"))
    nrd = SescConf->getInt(section, "rpl");

  if(SescConf->checkInt(section, "maxWrites"))
    nwr = SescConf->getInt(section, "maxWrites");

  if(strcmp(type, "banked") == 0) {
    int32_t nb = SescConf->getInt(section, "banks");
    mshr = new BankedMSHR<Addr_t, Cache_t>(name, size, lineSize, nb, nrd, nwr,
                                           aPolicy);
  } else {
    mshr = MSHR<Addr_t,Cache_t>::create(name, type, size, lineSize, nrd, nwr,
                                        aPolicy);
  }

  return mshr;
}

template<class Addr_t, class Cache_t>
MSHR<Addr_t,Cache_t> *MSHR<Addr_t, Cache_t>::attach(const char *name,
                                                    const char *section,
                                                    MSHR<Addr_t,Cache_t> *mshr)
{
  MSHR *newmshr;
  int32_t aPolicy = SPECIAL;

    {
      newmshr = MSHR<Addr_t,Cache_t>::create(name,section);
    }

  // Share occupancy stats
  newmshr->attach(mshr);

  return newmshr;
}

template<class Addr_t, class Cache_t>
MSHR<Addr_t, Cache_t>::MSHR(const char *name, int32_t size, int32_t lineSize, int32_t aPolicy)
  :nEntries(size)
  ,Log2LineSize(log2i(lineSize))
  ,nUse("%s_MSHR:nUse", name)
  ,nUseReads("%s_MSHR:nUseReads", name)
  ,nUseWrites("%s_MSHR:nUseWrites", name)
  ,nOverflows("%s_MSHR:nOverflows", name)
  ,maxUsedEntries("%s_MSHR_maxUsedEntries", name)
  ,nCanAccept("%s_MSHR:nCanAccept", name)
  ,nCanNotAccept("%s_MSHR:nCanNotAccept", name)
  ,nCanNotAcceptConv("%s_MSHR:nCanNotAcceptConv", name)
  ,blockingCycles("%s_MSHR:blockingCycles",name)
  ,allocPolicy(aPolicy)
  ,occStatsAttached(false)
  ,lowerCache(NULL)
{
  I(size>0 && size<1024*32);

#ifdef MSHR_BASICOCCSTATS
  occStats = new MSHRStats<Addr_t,Cache_t>(name);
  occStats->attach(this);
#endif

  nFreeEntries = size;
}

template<class Addr_t, class Cache_t>
void MSHR<Addr_t, Cache_t>::attach(MSHR<Addr_t, Cache_t> *mshr)
{
#ifdef MSHR_BASICOCCSTATS

  if(!occStatsAttached)
    delete occStats;

  occStatsAttached = true;

  occStats = mshr->occStats;
  occStats->attach(this);
#endif
}

template<class Addr_t, class Cache_t>
bool MSHR<Addr_t, Cache_t>::canAcceptRequest(Addr_t paddr, MemOperation mo)
{
  if(allocPolicy==CONVENTIONAL) {
    if(!canAllocateEntry()) {
      nCanNotAccept.inc();
      nCanNotAcceptConv.inc();

      if(readSEntryFull())
	blockingCycles.sample(2);
      else if(writeSEntryFull())
	blockingCycles.sample(3);
      else
	blockingCycles.sample(1);

      return false;
    }
  }

  bool canAcceptSpecial = canAcceptRequestSpecial(paddr,mo);
  //GI(allocPolicy == CONVENTIONAL, canAcceptSpecial);
  //this is not true, the MSHR might be overflowing

  blockingCycles.sample((canAcceptSpecial)? 0 : 1);

  return canAcceptSpecial;
}

template<class Addr_t, class Cache_t>
MSHRentry<Addr_t>* MSHR<Addr_t, Cache_t>::selectEntryToDrop(Addr_t paddr)
{
  MSG("MSHR::selectEntryToDrop not defined. May not have intended meaning.");
  return NULL;
}

template<class Addr_t, class Cache_t>
void MSHR<Addr_t, Cache_t>::dropEntry(Addr_t lineAddr)
{
  MSG("MSHR::dropEntry not defined. May not have intended meaning.");
}

template<class Addr_t, class Cache_t>
MSHRentry<Addr_t>* MSHR<Addr_t, Cache_t>::getEntry(Addr_t paddr)
{
  MSG("MSHR::getEntry not defined. May not have intended meaning.");
  return NULL;
}

template<class Addr_t, class Cache_t>
void MSHR<Addr_t, Cache_t>::putEntry(MSHRentry<Addr_t> &me)
{
  MSG("MSHR::putEntry not defined. May not have intended use.");
}

template<class Addr_t, class Cache_t>
void MSHR<Addr_t, Cache_t>::updateOccHistogram()
{
  occStats->sampleEntryOcc(this,nEntries-nFreeEntries);
}

//
// NoMSHR class
//

template<class Addr_t, class Cache_t>
NoMSHR<Addr_t, Cache_t>::NoMSHR(const char *name, int32_t size, int32_t lineSize, int32_t aPolicy)
  : MSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy)
{
  //nothing to do
}


//
// NoDepsMSHR class
//

template<class Addr_t, class Cache_t>
NoDepsMSHR<Addr_t, Cache_t>::NoDepsMSHR(const char *name, int32_t size, int32_t lineSize,
                                        int32_t aPolicy)
  : MSHR<Addr_t, Cache_t>(name, size, lineSize,aPolicy)
{
  //nothing to do
}

template<class Addr_t, class Cache_t>
bool NoDepsMSHR<Addr_t, Cache_t>::issue(Addr_t paddr, MemOperation mo)
{
  nUse.inc();

  nFreeEntries--;

  if(!overflow.empty() || nFreeEntries < 0) {
    return false;
  }

  return true;
}


template<class Addr_t, class Cache_t>
void NoDepsMSHR<Addr_t, Cache_t>::addEntry(Addr_t paddr, CallbackBase *c,
                                           CallbackBase *ovflwc, MemOperation mo)
{
  // by definition, calling addEntry in the NoDepsMSHR is overflowing
  OverflowField f;
  f.paddr   = paddr;
  f.cb      = c;
  f.ovflwcb = ovflwc;
  f.mo      = mo;
  overflow.push_back(f);

  if(nFreeEntries <= 0)
    nOverflows.inc();

  return;
}

template<class Addr_t, class Cache_t>
bool NoDepsMSHR<Addr_t, Cache_t>::retire(Addr_t paddr)
{
  maxUsedEntries.sample(nEntries - nFreeEntries);

  nFreeEntries++;

  if(!overflow.empty()) {
    OverflowField f = overflow.front();
    overflow.pop_front();

    if(f.ovflwcb) {
      f.ovflwcb->call();
      f.cb->destroy(); // the accessQueuedCallback will bever be called.
    } else
      f.cb->call();  // temporary until vmem uses the ovflw callback FIXME
  }

  return true;
}

template<class Addr_t, class Cache_t>
bool NoDepsMSHR<Addr_t, Cache_t>::hasLineReq(Addr_t paddr)
{
  return false;
}

//
// FullMSHR class
//

template<class Addr_t, class Cache_t>
FullMSHR<Addr_t, Cache_t>::FullMSHR(const char *name, int32_t size, int32_t lineSize, int32_t aPolicy)
  : MSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy)
  ,nStallConflict("%s_MSHR:nStallConflict", name)
  ,MSHRSize(roundUpPower2(size)*4)
  ,MSHRMask(MSHRSize-1)

{
  I(lineSize>=0 && Log2LineSize<(8*sizeof(Addr_t)-1));
  overflowing  = false;

  entry = new EntryType[MSHRSize];

  for(int32_t i=0;i<MSHRSize;i++) {
    entry[i].inUse = false;
    I(entry[i].cc.empty());
  }
}

template<class Addr_t, class Cache_t>
bool FullMSHR<Addr_t, Cache_t>::issue(Addr_t paddr, MemOperation mo)
{
  nUse.inc();

  if (overflowing || nFreeEntries == 0) {
    overflowing = true;
    return false;
  }

  nFreeEntries--;
  I(nFreeEntries>=0);

  int32_t pos = calcEntry(paddr);
  if (entry[pos].inUse)
    return false;

  entry[pos].inUse = true;
  I(entry[pos].cc.empty());

  return true;
}

template<class Addr_t, class Cache_t>
void FullMSHR<Addr_t, Cache_t>::addEntry(Addr_t paddr, CallbackBase *c, CallbackBase *ovflwc, MemOperation mo)
{
  I(nFreeEntries>=0);
  I(nFreeEntries <= nEntries);

  if (overflowing) {
    OverflowField f;
    f.paddr = paddr;
    f.cb    = c;
    f.ovflwcb = ovflwc;
    f.mo    = mo;
    overflow.push_back(f);

    nOverflows.inc();
    return;
  }

  if(ovflwc)
    ovflwc->destroy();

  nStallConflict.inc();

  int32_t pos = calcEntry(paddr);

  I(entry[pos].inUse);

  entry[pos].cc.add(c);
}

template<class Addr_t, class Cache_t>
bool FullMSHR<Addr_t, Cache_t>::retire(Addr_t paddr)
{
  maxUsedEntries.sample((nEntries - nFreeEntries) + overflow.size());

  if (overflowing) {
    I(!overflow.empty());
    OverflowField f = overflow.front();
    overflow.pop_front();
    overflowing = !overflow.empty();

    int32_t opos = calcEntry(f.paddr);
    if (entry[opos].inUse) {
      entry[opos].cc.add(f.cb);
      // we did not need the overflow callback here, since there was a
      // pending line already. but we need to destroy the callback to
      // avoid leaks.
      if(f.ovflwcb)
        f.ovflwcb->destroy();
    }else{
      entry[opos].inUse = true;
      if(f.ovflwcb) {
        f.ovflwcb->call();
        f.cb->destroy(); // the retire callback will never be called
      } else
        f.cb->call(); // temporary until vmem uses the ovflw callback FIXME
    }
  } else
    nFreeEntries++;

  I(nFreeEntries>=0);
  I(nFreeEntries <= nEntries);

  int32_t pos = calcEntry(paddr);

  I(entry[pos].inUse);

  if (!entry[pos].cc.empty()) {
    entry[pos].cc.callNext();
    return false;
  }

  entry[pos].inUse = false;
  return true;
}


template<class Addr_t, class Cache_t>
bool FullMSHR<Addr_t, Cache_t>::hasLineReq(Addr_t paddr)
{
  //not well defined for this MSHR
  return false;
}

//
// SingleMSHR
//

template<class Addr_t, class Cache_t>
SingleMSHR<Addr_t, Cache_t>::SingleMSHR(const char *name, int32_t size,
                                        int32_t lineSize, int32_t nrd, int32_t nwr,
                                        int32_t aPolicy)
  : MSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy),
    nReads(nrd),
    nWrites(nwr),
    bf(4, 8, 256, 6, 64, 6, 64, 6, 64),
    avgOverflowConsumptions("%s_MSHR_avgOverflowConsumptions", name),
    maxOutsReqs("%s_MSHR_maxOutsReqs", name),
    avgReqsPerLine("%s_MSHR_avgReqsPerLine", name),
    nIssuesNewEntry("%s_MSHR:nIssuesNewEntry", name),
    nCanNotAcceptSubEntryFull("%s_MSHR:nCanNotAcceptSubEntryFull", name),
    nCanNotAcceptTooManyWrites("%s_MSHR:nCanNotAcceptTooManyWrites", name),
    avgQueueSize("%s_MSHR_avgQueueSize", name),
    avgWritesPerLine("%s_MSHR_avgWritesPerLine", name),
    avgWritesPerLineComb("%s_MSHR_avgWritesPerLineComb", name),
    nOnlyWrites("%s_MSHR:nOnlyWrites", name),
    nRetiredEntries("%s_MSHR:nRetiredEntries", name),
    nRetiredEntriesWritten("%s_MSHR:nRetiredEntriesWritten", name)
{
  nFreeEntries = nEntries;
  nFullReadEntries = 0;
  nFullWriteEntries = 0;
  checkingOverflow = false;
  nOutsReqs = 0;
}

template<class Addr_t, class Cache_t>
bool SingleMSHR<Addr_t, Cache_t>::issue(Addr_t paddr, MemOperation mo)
{
  MSHRit it = ms.find(calcLineAddr(paddr));

  nUse.inc();
  if(mo == MemRead)
    nUseReads.inc();

  if(mo == MemWrite)
    nUseWrites.inc();

  if(!overflow.empty()) {
    return false;
  }

  I(nFreeEntries >= 0 && nFreeEntries <=nEntries);

  if(it == ms.end()) {
    if(nFreeEntries > 0) {
      ms[calcLineAddr(paddr)].firstRequest(paddr, calcLineAddr(paddr), 
					   nReads, nWrites, mo);
      bf.insert(calcLineAddr(paddr));
      nFreeEntries--;

#ifdef MSHR_BASICOCCSTATS
      updateOccHistogram();
#endif

      nOutsReqs++;

#ifdef MSHR_BASICOCCSTATS
      if(mo == MemRead)
	occStats->incRdReqs();
#endif

      nIssuesNewEntry.inc();
      avgQueueSize.sample(0);

      checkSubEntries(paddr, mo);

#ifdef MSHR_EXTRAOCCSTATS
      occStats->sampleEntry( calcLineAddr(paddr) );
#endif
      return true;
    }
  }

  return false;
}

template<class Addr_t, class Cache_t>
void SingleMSHR<Addr_t, Cache_t>::toOverflow(Addr_t paddr, CallbackBase *c,
                                             CallbackBase *ovflwc, MemOperation mo)
{
  OverflowField f;
  f.paddr = paddr;
  f.cb    = c;
  f.ovflwcb = ovflwc;
  f.mo      = mo;

  overflow.push_back(f);
  nOverflows.inc();
}

template<class Addr_t, class Cache_t>
void SingleMSHR<Addr_t, Cache_t>::checkSubEntries(Addr_t paddr, MemOperation mo)
{
  MSHRit it = ms.find(calcLineAddr(paddr));
  I(it != ms.end());

  if((*it).second.isRdWrSharing()) {
    if(!(*it).second.hasFreeReads() ||!(*it).second.hasFreeWrites()) {
      nFullReadEntries++;
      nFullWriteEntries++;
    }
  } else {
    if(!(*it).second.hasFreeReads() && mo == MemRead) {
      nFullReadEntries++;
    }
    if(!(*it).second.hasFreeWrites() && mo == MemWrite) {
      nFullWriteEntries++;
    }
  }
}

template<class Addr_t, class Cache_t>
void SingleMSHR<Addr_t, Cache_t>::checkOverflow()
{
  if(overflow.empty()) //nothing to do
    return;

  if(checkingOverflow) // i am already checking the overflow
    return;

  I(!overflow.empty());
  I(!checkingOverflow);

  checkingOverflow = true;

  int32_t nConsumed = 0;

  do {
    OverflowField f = overflow.front();
    MSHRit it = ms.find(calcLineAddr(f.paddr));

    if(it == ms.end()) {
      if(nFreeEntries > 0) {
        ms[calcLineAddr(f.paddr)].firstRequest(f.paddr, calcLineAddr(f.paddr),
                                               nReads, nWrites, f.mo);
	checkSubEntries(f.paddr, f.mo);
#ifdef MSHR_EXTRAOCCSTATS
        occStats->sampleEntry( calcLineAddr( f.paddr ) );
#endif
        bf.insert(calcLineAddr(f.paddr));
        nFreeEntries--;

#ifdef MSHR_BASICOCCSTATS
        updateOccHistogram();
#endif
        nConsumed++;
        nOutsReqs++;

#ifdef MSHR_BASICOCCSTATS
	if(f.mo == MemRead)
	  occStats->incRdReqs();
#endif

        f.ovflwcb->call();
        f.cb->destroy();
        overflow.pop_front();
        nIssuesNewEntry.inc();
        avgQueueSize.sample(0);
      } else {
        break;
      }
    } else { // just try to add the entry
      if((*it).second.addRequest(f.paddr, f.cb, f.mo)) {
        // succesfully accepted entry, but no need to call the callback
        // since there was already an entry pending for the same line
        avgQueueSize.sample((*it).second.getPendingReqs() - 1);
        f.ovflwcb->destroy();
        overflow.pop_front();
        nOutsReqs++;

#ifdef MSHR_BASICOCCSTATS
	if(f.mo == MemRead)
	  occStats->incRdReqs();
#endif

	checkSubEntries(f.paddr, f.mo);
        //MSG("[%llu] nFullRd=%d nFullWr=%d a:%lu",globalClock,
        //  nFullReadEntries,nFullWriteEntries,calcLineAddr(f.paddr));
      } else {
        break;
      }
    }
  } while(!overflow.empty());

  if(nConsumed)
    avgOverflowConsumptions.sample(nConsumed);

  checkingOverflow = false;
}

template<class Addr_t, class Cache_t>
void SingleMSHR<Addr_t, Cache_t>::addEntry(Addr_t paddr, CallbackBase *c,
                                           CallbackBase *ovflwc, MemOperation mo)
{
  MSHRit it = ms.find(calcLineAddr(paddr));
  I(ovflwc); // for single MSHR, overflow handler REQUIRED!

  if(!overflow.empty()) {
    toOverflow(paddr, c, ovflwc, mo);
    return;
  }

  if(it == ms.end())  {// we must be overflowing because the issue did not happen
    toOverflow(paddr, c, ovflwc, mo);
    return;
  }

  I(it != ms.end());

  if((*it).second.addRequest(paddr, c, mo)) {
    // ok, the addrequest succeeded, the request was added
    avgQueueSize.sample((*it).second.getPendingReqs() - 1);
    nOutsReqs++;

#ifdef MSHR_BASICOCCSTATS
    if(mo == MemRead)
      occStats->incRdReqs();
#endif

    // there was no overflow, so the callback needs to be destroyed
    ovflwc->destroy();
    // check to see if we have filled up the subentries
    checkSubEntries(paddr, mo);

    //MSG("[%llu] nFullRd=%d nFullWr=%d a:%lu",globalClock,
    //      nFullReadEntries,nFullWriteEntries, calcLineAddr(paddr));
    return;
  } else {
    // too many oustanding requests to the same line already. send to overflow
    toOverflow(paddr, c, ovflwc, mo);
    return;
  }
}

template<class Addr_t, class Cache_t>
bool SingleMSHR<Addr_t, Cache_t>::retire(Addr_t paddr)
{
  bool rmEntry = false;

  MSHRit it = ms.find(calcLineAddr(paddr));
  I(it != ms.end());
  I(calcLineAddr(paddr) == (*it).second.getLineAddr());

  maxOutsReqs.sample(nOutsReqs);
  nOutsReqs--;

  //MSG("[%llu] nFullSubE=%d a=%lu",globalClock,nFullReadEntries,calcLineAddr(paddr));

  rmEntry = (*it).second.retire();
  if(rmEntry) {
    // the last pending request for the MSHRentry was completed
    // recycle the entry
    nRetiredEntries.inc();
    avgReqsPerLine.sample((*it).second.getUsedReads() + (*it).second.getUsedWrites());
    maxUsedEntries.sample(nEntries - nFreeEntries);
    avgWritesPerLine.sample((*it).second.getUsedWrites());
    avgWritesPerLineComb.sample((*it).second.getNWrittenWords());

#ifdef MSHR_BASICOCCSTATS    
    occStats->decRdReqs((*it).second.getUsedReads());
#endif

    if((*it).second.getUsedWrites() > 0)
      nRetiredEntriesWritten.inc();

    if( ! (*it).second.hasFreeReads() ) {
      nFullReadEntries--;
      I(nFullReadEntries>=0);
    }

    if( ! (*it).second.hasFreeWrites() ) {
      nFullWriteEntries--;
      I(nFullWriteEntries>=0);
    }

#ifdef MSHR_BASICOCCSTATS
    if((*it).second.isL2Hit()) 
      occStats->avgReadSubentriesL2Hit.sample((*it).second.getUsedReads());
    else
      occStats->avgReadSubentriesL2Miss.sample((*it).second.getUsedReads());
#endif

#ifdef MSHR_EXTRAOCCSTATS
    // extra MSHR occ stats
    occStats->subEntriesHist.sample((*it).second.getUsedReads()+
				    (*it).second.getUsedWrites(), 1);

    occStats->subEntriesReadsHist.sample((*it).second.getUsedReads(), 1);

    occStats->subEntriesWritesHist.sample((*it).second.getUsedWrites(), 1);

    occStats->subEntriesWritesHistComb.sample((*it).second.getNWrittenWords(), 1);

    occStats->subEntriesHistComb.sample((*it).second.getUsedReads()+
					(*it).second.getNWrittenWords(), 1);
    
    if((*it).second.getUsedReads() == 0 &&
       (*it).second.getUsedWrites() > 0) {
      nOnlyWrites.inc();
      occStats->retireWrEntry((*it).second.getLineAddr());
    } else if( (*it).second.getUsedReads() > 0 &&
      (*it).second.getUsedWrites() == 0 ) {
      occStats->retireRdEntry((*it).second.getLineAddr());
    } else {
      I( (*it).second.getUsedWrites() > 0 );  
      I( (*it).second.getUsedReads() > 0 );
      occStats->retireRdWrEntry((*it).second.getLineAddr());  
    }
#endif
    
    nFreeEntries++;

#ifdef MSHR_BASICOCCSTATS
    updateOccHistogram();
#endif

    bf.remove((*it).second.getLineAddr());
    ms.erase(it);
  }

  checkOverflow();

  return rmEntry;
}


template<class Addr_t, class Cache_t>
bool SingleMSHR<Addr_t, Cache_t>::canAllocateEntry()
{
  return (nFreeEntries > 0) && (nFullReadEntries==0) && (nFullWriteEntries==0);
}

template<class Addr_t, class Cache_t>
bool SingleMSHR<Addr_t, Cache_t>::readSEntryFull()
{ 
  return (nFullReadEntries != 0);
}

template<class Addr_t, class Cache_t>
bool SingleMSHR<Addr_t, Cache_t>::writeSEntryFull() 
{ 
  return (nFullWriteEntries != 0);
}


template<class Addr_t, class Cache_t>
bool SingleMSHR<Addr_t, Cache_t>::canAcceptRequestSpecial(Addr_t paddr, MemOperation mo)
{
  if(!overflow.empty()) {
    nCanNotAccept.inc();
    return false;
  }

  const_MSHRit it = ms.find(calcLineAddr(paddr));
  I(nFreeEntries >= 0 && nFreeEntries <= nEntries);

  if(it == ms.end()) {
    if(nFreeEntries <= 0) {
      nCanNotAccept.inc();
      return false;
    }
    nCanAccept.inc();
    return true;
  }

  I(it != ms.end());

  bool canAccept = (*it).second.canAcceptRequest(mo);
  if(canAccept)
    nCanAccept.inc();
  else {
    nCanNotAccept.inc();

    if(mo == MemWrite && !(*it).second.hasFreeWrites())
      nCanNotAcceptTooManyWrites.inc();
    else
      nCanNotAcceptSubEntryFull.inc();
  }

  return canAccept;
}

template<class Addr_t, class Cache_t>
bool SingleMSHR<Addr_t, Cache_t>::isOnlyWrites(Addr_t paddr)
{
  const_MSHRit it = ms.find(calcLineAddr(paddr));
  I(it != ms.end());

  return ((*it).second.getUsedReads() == 0);
}

template<class Addr_t, class Cache_t>
MSHRentry<Addr_t>* SingleMSHR<Addr_t, Cache_t>::selectEntryToDrop(Addr_t paddr)
{
  MSHRentry<Addr_t> *me;
  MSHRit dispIt = ms.begin();

  me = &((*dispIt).second);

  I(!ms.empty());

  // choosing the oldest one
  for(MSHRit it = ms.begin(); it != ms.end(); it++) {
    Time_t ts = it->second.getWhenAllocated();
    if(ts < dispIt->second.getWhenAllocated()) {
      dispIt = it;
      me = &((*it).second);
    }
  }

  return me;
}

template<class Addr_t, class Cache_t>
void SingleMSHR<Addr_t, Cache_t>::dropEntry(Addr_t lineAddr)
{
  MSHRit it = ms.find(lineAddr);
  I(it != ms.end());
  (*it).second.displace();

  if( ! (*it).second.hasFreeReads() ) {
    nFullReadEntries--;
  }
  if( ! (*it).second.hasFreeWrites() ) {
    nFullWriteEntries--;
  }

  nOutsReqs -= ( (*it).second.getPendingReqs() );

  nFreeEntries++;

#ifdef MSHR_BASICOCCSTATS
  updateOccHistogram();
  occStats->decRdReqs((*it).second.getUsedReads());
#endif
#ifdef MSHR_EXTRAOCCSTATS
  if((*it).second.getUsedReads() == 0 &&
     (*it).second.getUsedWrites() > 0) {
    occStats->retireWrEntry((*it).second.getLineAddr());
  } else if( (*it).second.getUsedReads() > 0 &&
	     (*it).second.getUsedWrites() == 0 ) {
    occStats->retireRdEntry((*it).second.getLineAddr());
  } else {
    I( (*it).second.getUsedWrites() > 0 );  
    I( (*it).second.getUsedReads() > 0 );
    occStats->retireRdWrEntry((*it).second.getLineAddr());  
  }
#endif

  ms.erase(it);
  bf.remove(lineAddr);
}

template<class Addr_t, class Cache_t>
MSHRentry<Addr_t>* SingleMSHR<Addr_t, Cache_t>::getEntry(Addr_t paddr)
{
  MSHRentry<Addr_t> *me = NULL;
  MSHRit dispIt = ms.find(calcLineAddr(paddr));

  if(dispIt!=ms.end())
    me = &((*dispIt).second);

  return me;
}

template<class Addr_t, class Cache_t>
void SingleMSHR<Addr_t, Cache_t>::putEntry(MSHRentry<Addr_t> &me)
{
  I(ms.find(me.getLineAddr()) == ms.end());

  I(nFreeEntries > 0);

  ms[me.getLineAddr()] = me;

  ms[me.getLineAddr()].adjustParameters(getnReads(), 
					getnWrites());

  MSHRentry<Addr_t> &pme = ms[me.getLineAddr()];

  bf.insert(pme.getLineAddr());
  nOutsReqs += pme.getPendingReqs();

  nFreeEntries--;

#ifdef MSHR_BASICOCCSTATS
  updateOccHistogram();
  occStats->incRdReqs(me.getUsedReads());
#endif
#ifdef MSHR_EXTRAOCCSTATS
  occStats->sampleEntry(me.getLineAddr());
#endif

  if( ! pme.hasFreeReads() ) {
    nFullReadEntries++;
  }
  if( ! pme.hasFreeWrites() ) {
    nFullWriteEntries++;
  }

  I(nFreeEntries >=0);
}

//
// BankedMSHR
//

template<class Addr_t, class Cache_t>
BankedMSHR<Addr_t, Cache_t>::BankedMSHR(const char *name, int32_t size, int32_t lineSize,
                                        int32_t nb, int32_t nrd, int32_t nwr, int32_t aPolicy)
  : MSHR<Addr_t, Cache_t>(name, size, lineSize, aPolicy),
    nBanks(nb),
    maxOutsReqs("%s_MSHR_maxOutsReqs", name),
    avgOverflowConsumptions("%s_MSHR_avgOverflowConsumptions", name)
{
  mshrBank = (SingleMSHR<Addr_t, Cache_t> **)
    malloc(sizeof(SingleMSHR<Addr_t, Cache_t> *) * nBanks);
  
  for(int32_t i = 0; i < nBanks; i++) {
    char mName[512];
    sprintf(mName, "%s_set%d", name, i);
    mshrBank[i] =
      new SingleMSHR<Addr_t, Cache_t>(mName, size, lineSize, nrd, nwr);
    mshrBank[i]->attach(this);
  }

  nOutsReqs = 0;
  checkingOverflow = false;
}

template<class Addr_t, class Cache_t>
bool BankedMSHR<Addr_t, Cache_t>::canAllocateEntry()
{
  bool can=true;
  for(int32_t i=0;i<nBanks;i++) {
    can = (can && mshrBank[i]->canAllocateEntry());
  }
  return can;
}

template<class Addr_t, class Cache_t>
bool BankedMSHR<Addr_t, Cache_t>::readSEntryFull()
{ 
  for(int32_t i=0;i<nBanks;i++) {
    if(mshrBank[i]->readSEntryFull())
      return true;
  }
  return false;
}

template<class Addr_t, class Cache_t>
bool BankedMSHR<Addr_t, Cache_t>::writeSEntryFull()
{ 
  for(int32_t i=0;i<nBanks;i++) {
    if(mshrBank[i]->writeSEntryFull())
      return true;
  }
  return false;
}


template<class Addr_t, class Cache_t>
bool BankedMSHR<Addr_t, Cache_t>::canAcceptRequestSpecial(Addr_t paddr,
                                                          MemOperation mo)
{
  if(!overflow.empty()) {
    nCanNotAccept.inc();
    return false;
  }

  bool canAccept = mshrBank[calcBankIndex(paddr)]->canAcceptRequest(paddr,mo);

  if(canAccept)
    nCanAccept.inc();
  else
    nCanNotAccept.inc();

  return canAccept;
}

template<class Addr_t, class Cache_t>
bool BankedMSHR<Addr_t, Cache_t>::issue(Addr_t paddr, MemOperation mo)
{
  nUse.inc();

  if(mo == MemRead)
    nUseReads.inc();

  if(mo == MemWrite)
    nUseWrites.inc();

  if(!overflow.empty())
    return false;

  bool issued = mshrBank[calcBankIndex(paddr)]->issue(paddr, mo);

  if(issued) {
    nOutsReqs++;
  }

  return issued;
}

template<class Addr_t, class Cache_t>
void BankedMSHR<Addr_t, Cache_t>::addEntry(Addr_t paddr, CallbackBase *c,
                                           CallbackBase *ovflwc, MemOperation mo)
{
  if(!overflow.empty()) {
    toOverflow(paddr, c, ovflwc, mo);
    return;
  }

  if(mshrBank[calcBankIndex(paddr)]->canAcceptRequest(paddr, mo)) {
    nOutsReqs++;
    mshrBank[calcBankIndex(paddr)]->addEntry(paddr, c, ovflwc, mo);
    I(!mshrBank[calcBankIndex(paddr)]->isOverflowing());
    return;
  }

  toOverflow(paddr, c, ovflwc, mo);
}

template<class Addr_t, class Cache_t>
bool BankedMSHR<Addr_t, Cache_t>::retire(Addr_t paddr)
{
  bool rmEntry;
  maxOutsReqs.sample(nOutsReqs);
  rmEntry = mshrBank[calcBankIndex(paddr)]->retire(paddr);
  nOutsReqs--;
  checkOverflow();
  return rmEntry;
}

template<class Addr_t, class Cache_t>
void BankedMSHR<Addr_t, Cache_t>::toOverflow(Addr_t paddr, CallbackBase *c,
                                             CallbackBase *ovflwc, MemOperation mo)
{
  OverflowField f;
  f.paddr = paddr;
  f.cb    = c;
  f.ovflwcb = ovflwc;
  f.mo      = mo;

  nOverflows.inc();
  overflow.push_back(f);
}

template<class Addr_t, class Cache_t>
void BankedMSHR<Addr_t, Cache_t>::checkOverflow()
{
  if(overflow.empty()) //nothing to do
    return;

  if(checkingOverflow) // i am already checking the overflow
    return;

  I(!overflow.empty());
  I(!checkingOverflow);

  checkingOverflow = true;

  int32_t nConsumed = 0;

  do {
    OverflowField f = overflow.front();
    SingleMSHR<Addr_t, Cache_t> *mb = mshrBank[calcBankIndex(f.paddr)];

    if(!mb->canAcceptRequest(f.paddr, f.mo))
      break;

    overflow.pop_front();
    nOutsReqs++;
    nConsumed++;

    if(mb->issue(f.paddr, f.mo)) {

      f.ovflwcb->call();
      f.cb->destroy();
      continue;
    }

    mb->addEntry(f.paddr, f.cb, f.ovflwcb, f.mo);
    I(!mb->isOverflowing());

  } while(!overflow.empty());

  if(nConsumed)
    avgOverflowConsumptions.sample(nConsumed);

  checkingOverflow = false;
}

template<class Addr_t, class Cache_t>
void BankedMSHR<Addr_t, Cache_t>::setLowerCache(Cache_t *lCache)
{
  for(int32_t i=0; i<nBanks; i++) {
    mshrBank[i]->setLowerCache(lCache);
  }
}

template<class Addr_t, class Cache_t>
bool BankedMSHR<Addr_t, Cache_t>::hasLineReq(Addr_t paddr)
{
  return mshrBank[calcBankIndex(paddr<<Log2LineSize)]->hasLineReq(paddr);
}

template<class Addr_t, class Cache_t>
MSHRentry<Addr_t>* BankedMSHR<Addr_t, Cache_t>::selectEntryToDrop(Addr_t paddr)
{
  MSHRentry<Addr_t> *me;

  me = mshrBank[calcBankIndex(paddr)]->selectEntryToDrop(paddr);

  return me;
}

template<class Addr_t, class Cache_t>
void BankedMSHR<Addr_t, Cache_t>::dropEntry(Addr_t lineAddr)
{
  I(mshrBank);
  I(mshrBank[calcBankIndex(lineAddr<<Log2LineSize)]);
  mshrBank[calcBankIndex(lineAddr<<Log2LineSize)]->dropEntry(lineAddr);
}

template<class Addr_t, class Cache_t>
MSHRentry<Addr_t>* BankedMSHR<Addr_t, Cache_t>::getEntry(Addr_t paddr)
{
  return mshrBank[calcBankIndex(paddr)]->getEntry(paddr);
}


template<class Addr_t, class Cache_t>
void BankedMSHR<Addr_t, Cache_t>::putEntry(MSHRentry<Addr_t> &me)
{
  I(mshrBank);
  I(mshrBank[calcBankIndex(me.getLineAddr()<<Log2LineSize)]);
  mshrBank[calcBankIndex(me.getLineAddr()<<Log2LineSize)]->putEntry(me);
}

template<class Addr_t, class Cache_t>
void BankedMSHR<Addr_t, Cache_t>::attach( MSHR<Addr_t,Cache_t> *mshr )
{
  MSHR<Addr_t,Cache_t>::attach(mshr);
  for(int32_t i=0;i<nBanks;i++) {
    mshrBank[i]->attach(mshr);
  }
}



//
// MSHRentry stuff
//

template<class Addr_t>
bool MSHRentry<Addr_t>::addRequest(Addr_t reqAddr, CallbackBase *rcb, MemOperation mo)
{
  I(nFreeReads >= 0);
  I(nFreeReads <= nReads);
  I(nFreeWrites >= 0);
  I(nFreeWrites <= nWrites);
  I(nFreeSEntries >= 0);

  if(nFreeSEntries == 0)
    return false;

  if(mo == MemRead && nFreeReads == 0)
    return false;

  if(mo == MemWrite && nFreeWrites == 0) 
    return false;

  // Writes/reads are counted separately
  if(mo == MemRead) {
    nFreeReads--;
  } else {
    I(mo == MemWrite);
    //hardcoded word mask for now. ugly, i know.
    writeSet.insert(reqAddr & 0xfffffffc);
    nFreeWrites--;
  }
  
  nFreeSEntries--;
  cc.add(rcb);
  return true;
}

template<class Addr_t>
bool MSHRentry<Addr_t>::retire()
{
  if(!cc.empty()) {
    cc.callNext();
    return false;
  }

  return true;
}
