#include "Epoch.h"
#include "Checkpoint.h"
#include "SescConf.h"
#include "AdvancedStats.h"

void callSysCallExit(ThreadContext *context,icode_ptr picode){
  tls::Epoch *epoch=context->getEpoch();
  I(epoch);
  epoch->newSysCall<SysCallExit>()->exec(context,picode);
}
  
namespace tls{

  Stats::Group AllStats;

  // Statistics for all epochs
  Stats::Distribution NumRunClockAdj(&AllStats,"number of run-time clock adjustments");
  Stats::Distribution BufBlocksAdjusted(&AllStats,"number of buffer blocks per clock adjustment");
  Stats::Distribution EpochInstructions(&AllStats,"number of instructions per epoch");
  Stats::Distribution EpochBuffSpace(&AllStats,"bytes of buffer space per epoch");

  // Statistics for acquire/release
  Stats::Distribution AtomicCommitInstrs(&AllStats,"number of instructions committed in atomic");
  Stats::Distribution AtomicSquashInstrs(&AllStats,"number of instructions squashed in atomic");
  Stats::Distribution AtomicTotalBlocks(&AllStats,"number of blocks accessed in atomic");
  Stats::Distribution AtomicConsumedBlocks(&AllStats,"number of blocks consumed in atomic");
  Stats::Distribution AtomicProducedBlocks(&AllStats,"number of blocks produced in atomic");
  
  // Statistics for squashes
  Stats::Distribution SquashInstructions(&AllStats,"number of instructions squashed");

  // Statistics for system calls
  
  Stats::Distribution SysCallCount(&AllStats,"recorded system calls per epoch");

  ClockValue LClock::syncDelta;

  void LClock::staticConstructor(void){
    SescConf->isInt("TLS","LClockSyncDelta");
    SescConf->isGT("TLS","LClockSyncDelta",-1);
    syncDelta=SescConf->getInt("TLS","LClockSyncDelta");
  }

  uint32_t numBegThread=0;
  uint32_t numEndThread=0;

  InstrCount Epoch::limitEpochInstrCount;
  InstrCount Epoch::maxEpochInstrCount=0;

  Epoch::PendWb Epoch::pendWb;
  Epoch::WaitAcqMap Epoch::waitAcqMap;
  GStatsCntr Epoch::waitAcqCycles("%s: Total cycles spent waiting to be squashed","TLS");
  void Epoch::staticConstructor(void)
  {
    SysCallFileIO::staticConstructor();
    LClock::staticConstructor();
    Thread::staticConstructor();
    SescConf->isInt("TLS","limitEpochInstrCount");
    SescConf->isGT("TLS","limitEpochInstrCount",-1);
    limitEpochInstrCount=(InstrCount)(SescConf->getInt("TLS","limitEpochInstrCount"));
    SescConf->isBool("TLS","threadsSequential");
    threadsSequential=SescConf->getBool("TLS","threadsSequential");
    SescConf->isInt("TLS","limitThreadBlockVersions");
    SescConf->isGT("TLS","limitThreadBlockVersions",-1);
    limitThreadBlockVersions=(size_t)(SescConf->getInt("TLS","limitThreadBlockVersions"));
    SescConf->isBool("TLS","mustFindAllRaces");
    mustFindAllRaces=SescConf->getBool("TLS","mustFindAllRaces");
    //    SescConf->isBool("TLS","ignoreRaces");
    //    dataRacesIgnored=SescConf->getBool("TLS","ignoreRaces");
    dataRacesIgnored=false;
    numEpochs=0;
    numRaceMissEpochs=0;
    SescConf->isInt("TLS","epochBufferSizeLimit");
    SescConf->isGT("TLS","epochBufferSizeLimit",-1);
    long epochBufferSizeLimit=SescConf->getInt("TLS","epochBufferSizeLimit");
    I(epochBufferSizeLimit>=0);
    maxBlocksPerEpoch=(size_t)(epochBufferSizeLimit/blockSize);
    I(maxBlocksPerEpoch*blockSize==(size_t)epochBufferSizeLimit);
    SescConf->isBool("TLS","testRollback");
    testRollback=SescConf->getBool("TLS","testRollback");
    clockAtLastRollback=smallClockValue;
    squashToTestRollback=0;
    dynamicAtmOmmitCount=0;
    staticAtmOmmitCount=0;
    if(SescConf->getInt("TLS","atmOmmit")>0){
      dynamicAtmOmmitCount=SescConf->getInt("TLS","atmOmmit");
    }else{
      staticAtmOmmitCount=-SescConf->getInt("TLS","atmOmmit");
    }
    atmOmmitInstr=0;
    atmOmmitCount=0;
    Checkpoint::staticConstructor();
    char *Tlsname=new char[4];
    strcpy(Tlsname,"TLS");
    
  }

  bool Epoch::threadsSequential;
  size_t Epoch::maxAllBlockVersions=0;
  size_t Epoch::limitThreadBlockVersions;
  size_t Epoch::maxThreadBlockVersions=0;

  bool Epoch::mustFindAllRaces;
  bool Epoch::dataRacesIgnored;
  size_t Epoch::numEpochs;
  size_t Epoch::numRaceMissEpochs;
  size_t Epoch::maxBlocksPerEpoch;
  bool Epoch::testRollback;
  ClockValue Epoch::clockAtLastRollback;
  Epoch *Epoch::squashToTestRollback;

  void Epoch::report(void){
    allInstRaces.report();
    Report::field("TLS:numEpochs=%d",numEpochs);
    Report::field("TLS:maxEpochInstrCount=%lld",maxEpochInstrCount);
    Report::field("TLS:numRaceMissEpochs=%d",numRaceMissEpochs);
    Thread::report();
    Report::field("TLS:threadsBegun=%d",numBegThread);
    Report::field("TLS:threadsEnded=%d",numEndThread);
    size_t staticAtmSections=0;
    size_t dynamicAtmSections=0;
    for(AddrToCount::iterator atmIt=atomicEntryCount.begin();
	atmIt!=atomicEntryCount.end();atmIt++){
      I(atmIt->second>0);
      staticAtmSections++;
      dynamicAtmSections+=atmIt->second;
    }
    Report::field("TLS:staticAtmSections=%d",staticAtmSections);
    Report::field("TLS:dynamicAtmSections=%d",dynamicAtmSections);
    Report::field("TLS:atmOmmitInstr=%08x",atmOmmitInstr);
    Report::field("TLS:atmOmmitCount=%d",atmOmmitCount);
    Report::field("TLS::maxAllBlockVersions=%d",maxAllBlockVersions);
    Report::field("TLS::maxThreadBlockVersions=%d",maxThreadBlockVersions);
    AllStats.report();
  }

  void Epoch::staticDestructor(void){
    BlockVersions::staticDestructor();
    I(numBegThread==numEndThread);
    for(EpochList::reverse_iterator epochIt=allEpochs.rbegin();epochIt!=allEpochs.rend();epochIt++){
      delete (*epochIt);
      //I(0);
      //printf("Epoch %ld:%d still remains\n",(*epochIt)->myClock,(*epochIt)->getTid());
    }
    Checkpoint::staticDestructor();
    VClock::staticDestructor();
  }

  // VClock stuff begins here

  size_t VClock::vectorSize=0;
  VClock::VClockList VClock::statVClocks;
  VClock::VClockList VClock::usedVClocks;
  VClock::VClockList VClock::freeVClocks;
  VClock VClock::smallVClockValue;
  VClock VClock::largeVClockValue;
  VClock VClock::freeStart;

  VClock::VClock(void)
    : myThreadID(invalidThreadID), myVector(),
      myListPos(statVClocks.size()){
    I(vectorSize==0);
    I(usedVClocks.empty());
    I(freeVClocks.empty());
    statVClocks.push_back(this);
    I(statVClocks[myListPos]==this);
  }
  VClock::VClock(const VClock *srcClock)
    : myThreadID(srcClock->myThreadID),
      myVector(srcClock->myVector),
      myListPos(usedVClocks.size()){
    I(myVector.size()==vectorSize);
    I(freeVClocks.empty());
    usedVClocks.push_back(this);
    I(usedVClocks[myListPos]==this);
  }
  VClock::~VClock(void){
    I(myVector.size()==vectorSize);
    if((myListPos<statVClocks.size())&&(statVClocks[myListPos]==this)){
      VClock *last=statVClocks.back();
      if(last!=this){
	last->myListPos=myListPos;
	statVClocks[myListPos]=last;
      }
      statVClocks.pop_back();
    }else{
      I((myListPos<freeVClocks.size())&&(freeVClocks[myListPos]==this));
      VClock *last=freeVClocks.back();
      if(last!=this){
	last->myListPos=myListPos;
	freeVClocks[myListPos]=last;
      }
      freeVClocks.pop_back();
    }
  }
  void VClock::staticDestructor(void){
    I(usedVClocks.empty());
    while(freeVClocks.size()){
      delete freeVClocks.back();
      freeVClocks.pop_back();
    }
  }
  VClock *VClock::newVClock(const VClock *srcClock, bool incOwnValue){
    I((srcClock->myListPos>=freeVClocks.size())||(freeVClocks[srcClock->myListPos]!=srcClock));
    VClock *newClock;
    // Get a VClock from free list or allocate a new one
    if(freeVClocks.empty()){
      newClock=new VClock(srcClock);
    }else{
      newClock=freeVClocks.back();
      newClock->myVector=srcClock->myVector;
      newClock->myThreadID=srcClock->myThreadID;
      freeVClocks.pop_back();
      newClock->myListPos=usedVClocks.size();
      usedVClocks.push_back(newClock);
    }
    if(incOwnValue){
      I(srcClock->myThreadID!=invalidThreadID);
      I((size_t)(srcClock->myThreadID)<vectorSize);
      if(newClock->myVector[srcClock->myThreadID]==smallClockValue)
        newClock->myVector[srcClock->myThreadID]=startClockValue;
      else
        newClock->myVector[srcClock->myThreadID]++;
    }
    return newClock;
  }
  VClock *VClock::newVClock(const VClock *srcClock, ThreadID tid, bool incOwnValue){
    I(tid!=invalidThreadID);
    VClock *newClock=newVClock(srcClock,false);
    // Increase size of all vectors if new index too large
    if((size_t)tid>=vectorSize){
      vectorSize=(size_t)tid+1;
      // This VClock is extended with the large clock value,
      largeVClockValue.myVector.resize(vectorSize,largeClockValue);
      // Everything else is extended with the small clock value
      for(VClockList::const_iterator statIt=statVClocks.begin();
	  statIt!=statVClocks.end();statIt++)
	(*statIt)->myVector.resize(vectorSize,smallClockValue);
      for(VClockList::iterator usedIt=usedVClocks.begin();
	  usedIt!=usedVClocks.end();usedIt++)
	(*usedIt)->myVector.resize(vectorSize,smallClockValue);
    }
    newClock->myThreadID=tid;
    newClock->myVector[tid]=freeStart[tid];
    if(incOwnValue){
      I((size_t)tid<vectorSize);
      if(newClock->myVector[tid]==smallClockValue)
        newClock->myVector[tid]=startClockValue;
      else
        newClock->myVector[tid]++;
    }
    return newClock;
  }
  void VClock::freeVClock(VClock *oldClock){
    I((oldClock->myListPos<usedVClocks.size())&&(usedVClocks[oldClock->myListPos]==oldClock));
    ID(oldClock->myThreadID=invalidThreadID);
    VClock *lastClock=usedVClocks.back();
    lastClock->myListPos=oldClock->myListPos;
    usedVClocks[oldClock->myListPos]=lastClock;
    usedVClocks.pop_back();
    oldClock->myListPos=freeVClocks.size();
    freeVClocks.push_back(oldClock);
  }

  Thread::ThreadVector Thread::threadVector(0);
  ClockValue Thread::syncClockDelta;
  Thread *Thread::checkClockOwner=0;

  size_t Thread::totalThreads=0;
  size_t Thread::waitingThreads=0;

  InstrCount Thread::limitThreadInstrCount;
  InstrCount Thread::maxThreadInstrCount=0;
  size_t Thread::limitThreadOldVersions;
  size_t Thread::maxThreadOldVersions=0;
  size_t Thread::limitThreadAllVersions;
  size_t Thread::maxThreadAllVersions=0;

  void simExpired(int32_t sig){
    raise(SIGUSR1);
    raise(SIGKILL);
  }

  void Thread::staticConstructor(void){
    SescConf->isInt("TLS","limitSimulationMinutes");
    SescConf->isGT("TLS","limitSimulationMinutes",-1);
    long runMins=SescConf->getInt("TLS","limitSimulationMinutes");
    if(runMins){
      long runSecs=runMins*60;
      struct itimerval itv;
      itv.it_value.tv_sec=runSecs;
      itv.it_value.tv_usec=0;
      itv.it_interval.tv_sec=0;
      itv.it_interval.tv_usec=0;    
      int32_t timerRes=setitimer(ITIMER_VIRTUAL,&itv,0);
      I(timerRes==0);
      struct sigaction sac;
      sac.sa_handler=simExpired;
      int32_t setRes=sigemptyset(&(sac.sa_mask));
      I(setRes==0);
      sac.sa_flags=SA_ONESHOT|SA_NOMASK;
      int32_t actionRes=sigaction(SIGVTALRM,&sac,0);
      I(actionRes==0);
    }
    SescConf->isInt("TLS","limitThreadInstrCount");
    SescConf->isGT("TLS","limitThreadInstrCount",-1);
    limitThreadInstrCount=(InstrCount)(SescConf->getInt("TLS","limitThreadInstrCount"));
    SescConf->isInt("TLS","limitThreadOldVersions");
    SescConf->isGT("TLS","limitThreadOldVersions",-1);
    limitThreadOldVersions=(size_t)(SescConf->getInt("TLS","limitThreadOldVersions"));
    SescConf->isInt("TLS","limitThreadAllVersions");
    SescConf->isGT("TLS","limitThreadAllVersions",-1);
    limitThreadAllVersions=(size_t)(SescConf->getInt("TLS","limitThreadAllVersions"));
    I(limitThreadOldVersions<=limitThreadAllVersions);
    syncClockDelta=SescConf->getInt("TLS","syncClockDelta");
  }
  
  void Thread::report(void){
    Report::field("TLS:maxThreadInstrCount=%lld",maxThreadInstrCount);
    Report::field("TLS:maxThreadAllVersions=%d",maxThreadAllVersions);
    Report::field("TLS:maxThreadOldVersions=%d",maxThreadOldVersions);
  }

  Thread::Thread(ThreadID myID, ThreadID parentID, ClockValue spawnClock)
    : myID(myID), parentID(parentID), spawnClock(spawnClock),
      threadEpochs(), threadSafe(threadEpochs.end()),
      threadSafeClk(VClock::newVClock(VClock::getSmallValue(),myID,false)),
      noRacesMissed(VClock::newVClock(VClock::getSmallValue(),false)),
      raceFrontier(threadEpochs.end()),
      nextFrontierClk(smallClockValue),
      raceFrontierHolders(0),
      exitAtEnd(false), myExitCode(0), isWaiting(false),
      threadSafeAvailable(true),
      myAllVersionsCount(0), myOldVersionsCount(0), currentInstrCount(0)
  {
    ID(lastCommitEpoch=0);
    ID(lastFrontierCandidate=0);
    I(myID!=invalidThreadID);
    if(threadVector.size()<=(size_t)myID)
      threadVector.resize(myID+1,0);
    I(threadVector[myID]==0);
    threadVector[myID]=this;
    totalThreads++;
    // Add a child to parent thread
    if(parentID!=invalidThreadID){
      I(threadVector[parentID]);
      threadVector[parentID]->newChild(this);
    }
    // If possible, become owner of CheckClock
    if(!checkClockOwner)
      checkClockOwner=this;
  }
  
  Epoch *Thread::getSpawnEpoch(void){
    if(parentID==invalidThreadID)
      return 0;
    I(spawnClock!=smallClockValue);
    I(threadVector.size()>(size_t)parentID);
    Thread *parentThread=threadVector[parentID];
    if(!parentThread)
      return 0;
    EpochList::reverse_iterator threadRIt=parentThread->threadEpochs.rbegin();
    while((*threadRIt)->myClock<spawnClock)
      threadRIt++;
    I(threadRIt!=parentThread->threadEpochs.rend());
    if((*threadRIt)->myClock==spawnClock)
      return *threadRIt;
    I((*threadRIt)->myClock>spawnClock);
    return 0;
  }

  Thread::~Thread(void){
    I(!isWaiting);
    I(activeChildren.empty());
    I(zombieChildren.empty());
    I(!myAllVersionsCount);
    I(!myOldVersionsCount);
    totalThreads--;
    I((!waitingThreads)||(waitingThreads<totalThreads));
    I(threadVector[myID]==this);
    threadVector[myID]=0;
    // If we own the CheckClock, now it is free
    if(checkClockOwner==this)
      checkClockOwner=0;
    osSim->eventExit(myID,ThreadContext::getContext(myID)->getIntArg1());
    VClock::freeVClock(threadSafeClk);
    VClock::freeVClock(noRacesMissed);
  }
  
  void Thread::newChild(Thread *child){
    activeChildren.push_back(child);
  }
  void Thread::delChild(Thread *child){
    I(!isWaiting);
    ThreadList::iterator actIt;
    for(actIt=activeChildren.begin();*actIt!=child;actIt++)
      I(actIt!=activeChildren.end());
    activeChildren.erase(actIt);
  }
  bool Thread::endChild(Thread *child, Epoch *exitEpoch){
    ThreadList::iterator actIt;
    for(actIt=activeChildren.begin();*actIt!=child;actIt++)
      I(actIt!=activeChildren.end());
    activeChildren.erase(actIt);
    if(isWaiting&&((*threadSafe)->myState==Epoch::State::WaitChild)){
      I(dynamic_cast<SysCallWait *>(*(SysCallLog::reverse_iterator((*threadSafe)->sysCallLogPos))));
      SysCallWait *sysCallWait=static_cast<SysCallWait *>(*(SysCallLog::reverse_iterator((*threadSafe)->sysCallLogPos)));
      ThreadID zombieTid=sysCallWait->getChild();
      if((zombieTid==-1)||(zombieTid==child->myID)){
	(*threadSafe)->endWaitChild();
	sysCallWait->setChild(child->myID);
	(*threadSafe)->advanceClock(exitEpoch,true);
	if((*threadSafe)->myState==Epoch::State::Initial){
	  (*threadSafe)->myVClock->succeed(exitEpoch->myVClock);
	}else{
	  I(VClock::isOrder(exitEpoch->myVClock,(*threadSafe)->myVClock));
	}
	return true;
      }
    }
    zombieChildren.push_back(child);
    return false;
  }
  bool Thread::exitCalled(Epoch *exitEpoch){
    I(exitEpoch==threadEpochs.front());
    I(parentID!=invalidThreadID);
    I(threadVector[parentID]);
    Thread *parentThread=threadVector[parentID];
    if(parentThread->endChild(this,exitEpoch))
      return true;
    wait();
    return false;
  }
  Thread *Thread::harvestZombieThread(ThreadID tid){
    for(ThreadList::iterator zombieIt=zombieChildren.begin();
	zombieIt!=zombieChildren.end();zombieIt++){
      Thread *zombie=(*zombieIt);
      if((tid==invalidThreadID)||(zombie->myID==tid)){
	zombieChildren.erase(zombieIt);
	return zombie;
      }
    }
    return 0;
  }
  bool Thread::waitCalled(Epoch *waitEpoch){
    if(activeChildren.empty()&&zombieChildren.empty())
      return true;
    I(dynamic_cast<SysCallWait *>(*(SysCallLog::reverse_iterator(waitEpoch->sysCallLogPos))));
    SysCallWait *sysCallWait=static_cast<SysCallWait *>(*(SysCallLog::reverse_iterator(waitEpoch->sysCallLogPos)));
    ThreadID zombieTid=sysCallWait->getChild();
    Thread *zombieThread=harvestZombieThread(zombieTid);
    if(!zombieThread){
      wait();
      return false;
    }
    zombieTid=zombieThread->myID;
    I(zombieThread->isWaiting);
    I((zombieThread->isWaiting)||(waitEpoch->myState!=Epoch::State::Initial));
    if(zombieThread->isWaiting){
      Epoch *zombieEpoch=*(zombieThread->threadSafe);
      I(zombieEpoch->myState==Epoch::State::Waiting);
      I(zombieEpoch->myState==Epoch::State::WaitZombie);
      zombieEpoch->endWaitZombie();
      sysCallWait->setChild(zombieTid);
      waitEpoch->advanceClock(zombieEpoch,true);
      if(waitEpoch->myState==Epoch::State::Initial){
	waitEpoch->myVClock->succeed(zombieEpoch->myVClock);
      }else{
	I(VClock::isOrder(zombieEpoch->myVClock,waitEpoch->myVClock));
      }
    }
    return true;
  }
  void Thread::undoWaitCall(Thread *child){
    for(ThreadList::iterator actIt=activeChildren.begin();
	actIt!=activeChildren.end();actIt++){
      if(child==*actIt)
	return;
    }
    zombieChildren.push_front(child);
  }
  void Thread::undoExitCall(void){
    if(parentID==invalidThreadID)
      return;
    Thread *parent=threadVector[parentID];
    I(parent);
    I(parent->zombieChildren.back()==this);
    parent->zombieChildren.pop_back();
    parent->activeChildren.push_front(this);
  }
  Thread *Thread::getThreadForNewEpoch(ThreadID tid, Epoch *parentEpoch){
    if(!parentEpoch){
      return new Thread(tid,invalidThreadID,smallClockValue);
    }else if(tid!=parentEpoch->getTid()){
      return new Thread(tid,parentEpoch->getTid(),parentEpoch->myClock);
    }else{
      return parentEpoch->myThread;
    }
  }
  EpochList::iterator Thread::addEpoch(Epoch *epoch){
    // If this is the very first epoch in this thread,
    // it can have races with any epoch in each of the
    // other threads.
    if(threadEpochs.empty()){
      I(nextFrontierClk==smallClockValue);
      for(ThreadVector::iterator it=threadVector.begin();
	  it!=threadVector.end();it++){
	Thread *thread=*it;
	// If the other thread exists and has a frontier candidate
	// (which means there are frontier holders), now it has one
	// more holder (this thread)
	if(thread){
	  if(thread->raceFrontierHolders){
	    I(thread->noRacesMissed->getComponent(myID)==smallClockValue);
	    thread->raceFrontierHolders++;
	  }else{
	    threadSafeClk->setComponent(thread->myID,thread->nextFrontierClk);
	    thread->noRacesMissed->setComponent(myID,thread->nextFrontierClk);
	  }
	}
      }
    }
    // Find the position of the new epoch in threadEpochs
    EpochList::iterator listPos=threadEpochs.begin();
    while((listPos!=threadEpochs.end())&&(epoch->myClock<(*listPos)->myClock))
      listPos++;
    // Except for the very first epoch, there should be older epochs
    // in this thread already (the partent of the new epoch is one such epoch)
    I(threadEpochs.empty()||(listPos!=threadEpochs.end()));
    // We are either at the end, or in front of an older epoch
    I((listPos==threadEpochs.end())||(epoch->myClock>(*listPos)->myClock));
    // Insert and return the iterator to the inserted position
    return threadEpochs.insert(listPos,epoch);
  }

  bool Thread::removeEpoch(Epoch *epoch){
    EpochList::iterator listPos=epoch->myThreadEpochsPos;
    if(listPos==threadEpochs.end())
      return false;
    I(epoch->myState!=Epoch::State::ThreadSafe);
    if(listPos==threadSafe){
      // A ThreadSafe epoch can not be removed until it also commits
      I(threadSafeAvailable);
      I(epoch->myState>=Epoch::State::Committed);
      threadSafe++;
    }
    EpochList::reverse_iterator nextFrontier(raceFrontier);
    bool wasNextFrontier=false;
    if(raceFrontier!=threadEpochs.begin()){
      if((epoch==*nextFrontier)&&(epoch->myState>=Epoch::State::Committed))
        wasNextFrontier=true;
    }
    if(listPos==raceFrontier)
      raceFrontier++;
    threadEpochs.erase(listPos);
    epoch->myThreadEpochsPos=threadEpochs.end();
    ID(if(epoch==lastFrontierCandidate) lastFrontierCandidate=0);
    ID(if(epoch==lastCommitEpoch) lastCommitEpoch=0);
    // Is this the last epoch in this thread?
    if(threadEpochs.empty()){
      I(epoch->myState==Epoch::State::Committed);
      I(epoch->myState==Epoch::State::Completed);
      I(epoch->myState>=Epoch::State::LazyMerge);
      // If thread not exited already, update its context so it can proceed
      if(!exitAtEnd){
	// Transfer context of the epoch to the thread
	ThreadContext *epochContext=
	  osSim->getContext(epoch->myPid);
	ThreadContext *threadContext=
	  osSim->getContext(myID);
	// Update thread context with stuff from the last epoch's context
	threadContext->copy(epochContext);
      }
      return true;
    }
    if(wasNextFrontier){
      raceFrontierHolders=0;
      if((*nextFrontier)->myState>=Epoch::State::Committed)
        newFrontierCandidate();
    }
    return false;
  }
  
  void Thread::moveThreadSafe(void){
    I(threadSafe!=threadEpochs.begin());
    threadSafe--;
    I((*threadSafe)->myState==Epoch::State::ThreadSafe);
    changeThreadSafeVClock((*threadSafe)->myVClock);
  }

  void Thread::changeThreadSafeVClock(const VClock *newVClock){
    // Update the no-races-missed vector of each thread
    for(size_t i=0;i<VClock::size();i++){
      ClockValue newValue=newVClock->getComponent(i);
      ClockValue oldValue=threadSafeClk->getComponent(i);
      // Only update forward
      if(newValue<=oldValue)
	continue;
      threadSafeClk->setComponent(i,newValue);
      Thread *otherThread=threadVector[i];
      if(!otherThread)
        continue;
      // In the other thread, the epoch with a VClock value
      // of newValue has not missed any races with this thread
      I(otherThread->noRacesMissed->getComponent(myID)==oldValue);
      otherThread->noRacesMissed->setComponent(myID,newValue);
      // If the other thread has no waiting frontier candidate,
      // it's next-frontier is still not committed, so do nothing
      if(!otherThread->raceFrontierHolders)
        continue;
      // If the frontier candidate already could miss no
      // races with this thread, it is still a candidate for
      // some other reason, so we do nothing else now
      if(oldValue>=otherThread->nextFrontierClk)
	continue;
      // If the frontier candidate can still miss races
      // with this thread, we still can not move the frontier
      if(newValue<otherThread->nextFrontierClk)
	continue;
      // This thread is no longer holding the race frontier in
      // the other thread. If nothing else holds it, move it.
      otherThread->raceFrontierHolders--;
      if(!otherThread->raceFrontierHolders)
	otherThread->moveRaceFrontier();
    }
  }

  void Thread::commitEpoch(Epoch *epoch){
    I(epoch->myState>=Epoch::State::Committed);
    I(epoch!=lastCommitEpoch);
    currentInstrCount+=epoch->pendInstrCount;
    if(currentInstrCount>maxThreadInstrCount)
      maxThreadInstrCount=currentInstrCount;
    if(limitThreadInstrCount&&(maxThreadInstrCount>limitThreadInstrCount)){
      EpochList::iterator delEpochIt=Epoch::allEpochs.begin();
      I(delEpochIt!=Epoch::allEpochs.end());
      while(*delEpochIt!=epoch){
	Epoch *delEpoch=*delEpochIt;
	delEpochIt++;
	I(delEpochIt!=Epoch::allEpochs.end());
	if(delEpoch->myState<Epoch::State::ThreadSafe){
	  delEpoch->unspawn();
	}else if(delEpoch->canBeTruncated()){
	  delEpoch->complete();
	}
      }
    }
    if(epoch->myThreadEpochsPos==threadSafe){
      // When last thread commits, thread safe clock becomes too-large
      // because no running epoch in this thread can participate in a race
      I(epoch->myThreadEpochsPos==threadEpochs.begin());
      changeThreadSafeVClock(VClock::getLargeValue());
    }
    if(nextFrontierClk<epoch->myVClock->ownValue()){
      I(epoch!=lastFrontierCandidate);
      ID(lastCommitEpoch=epoch);
      EpochList::reverse_iterator nextFrontier(raceFrontier);
      if(epoch==*nextFrontier){
	I((epoch->myState>=Epoch::State::LazyMerge)||(!raceFrontierHolders));
	newFrontierCandidate();
      }
    }
  }

  void Thread::moveRaceFrontier(void){
    I(!raceFrontierHolders);
    I(raceFrontier!=threadEpochs.begin());
    raceFrontier--;
#if (defined DEBUG)
    I(nextFrontierClk==(*raceFrontier)->myVClock->ownValue());
    for(size_t i=0;i<VClock::size();i++){
      // If no thread with this index, do not count it
      if(!threadVector[i])
	continue;
      I(nextFrontierClk<=noRacesMissed->getComponent(i));
    }    
#endif
    EpochList::reverse_iterator nextFrontier(raceFrontier);
    if((nextFrontier!=threadEpochs.rend())&&((*nextFrontier)->myState>=Epoch::State::Committed))
      newFrontierCandidate();
    // If there are past-frontier epochs with RaceHold set, remove their hold
    if((raceFrontier!=threadEpochs.end())&&((*raceFrontier)->myState==Epoch::State::RaceHold)){
      // Find youngest epoch past race frontier without RaceHold
      EpochList::iterator holdIt=raceFrontier;
      while((holdIt!=threadEpochs.end())&&((*holdIt)->myState==Epoch::State::RaceHold))
	holdIt++;
      I(holdIt!=raceFrontier);
      // Removing merge holds may end up destroying the thread,
      // so we remember the thread ID to check its existence
      ThreadID threadID=myID;
      EpochList::reverse_iterator holdItR(holdIt);
      while(holdItR.base()!=raceFrontier){
	Epoch *holdEpoch=*holdItR;
	holdEpoch->removeMergeHold(Epoch::State::RaceHold);
	// If the thread is gone, we are done
	if(!threadVector[threadID])
	  break;
	// If no more epochs left in thread, we are done
	if(holdItR==threadEpochs.rend())
	  break;
	// If epoch still there after hold removal, skip it
	if(holdEpoch==(*holdItR))
	  holdItR++;
      }
    }
  }

  void Thread::newFrontierCandidate(void){
    EpochList::reverse_iterator nextFrontier(raceFrontier);
    I(((*nextFrontier)->myState>=Epoch::State::LazyMerge)||(!raceFrontierHolders));
    raceFrontierHolders=0;
    I(nextFrontier!=threadEpochs.rend());
    I((*nextFrontier)->myState>=Epoch::State::Committed);
    I(nextFrontierClk<(*nextFrontier)->myVClock->ownValue());
    nextFrontierClk=(*nextFrontier)->myVClock->ownValue();
    ID(lastFrontierCandidate=(*nextFrontier));
    for(size_t i=0;i<VClock::size();i++){
      // If no thread with this index, do not count it
      if(!threadVector[i])
	continue;
      // If next-frontier has no races missed with indexed thread,
      // the indexed thread is not a frontier holder
      if(nextFrontierClk<=noRacesMissed->getComponent(i))
	continue;
      raceFrontierHolders++;
    }
    if(!raceFrontierHolders)
      moveRaceFrontier();
  }

  bool Thread::canMissRaces(const Epoch *epoch) const{
    I(epoch&&(epoch->myThread==this));
    if(raceFrontier==threadEpochs.end())
      return true;
    return epoch->myClock>(*raceFrontier)->myClock;
  }
  
  void Thread::incAllVersionsCount(void){
    myAllVersionsCount++;
    if(limitThreadAllVersions&&(myAllVersionsCount>limitThreadAllVersions)){
      size_t removalCount=myAllVersionsCount-limitThreadAllVersions;
      EpochList::reverse_iterator epochIt=threadEpochs.rbegin();
      while(removalCount&&(epochIt!=threadEpochs.rend())){
	removalCount=(*epochIt)->requestAnyBlockRemoval(removalCount);
	epochIt++;
      }
    }
    if(myAllVersionsCount>maxThreadAllVersions)
      maxThreadAllVersions=myAllVersionsCount;
  }
  void Thread::incOldVersionsCount(void){
    myOldVersionsCount++;
    if(limitThreadOldVersions&&(myOldVersionsCount>limitThreadOldVersions)){
      size_t removalCount=myOldVersionsCount-limitThreadOldVersions;
      EpochList::reverse_iterator epochIt=threadEpochs.rbegin();
      while(removalCount&&(epochIt!=threadEpochs.rend())){
	removalCount=(*epochIt)->requestOldBlockRemoval(removalCount);
	epochIt++;
      }
    }
    if(myOldVersionsCount>maxThreadOldVersions)
      maxThreadOldVersions=myOldVersionsCount;
  }

  VClock Epoch::memVClock;
  LClock Epoch::memLClock;

  Epoch::CmpResult Epoch::compareClock(const Epoch *otherEpoch) const{
    if(myThread==otherEpoch->myThread){
      if(myClock<otherEpoch->myClock)
	return StrongAfter;
      if(myClock>otherEpoch->myClock)
	return StrongBefore;
      I(myClock!=otherEpoch->myClock);
    }
    if(myClock<otherEpoch->myClock-otherEpoch->getSyncClockDelta())
      return StrongAfter;
    if(myClock-getSyncClockDelta()>otherEpoch->myClock)
      return StrongBefore;
    if(myClock<otherEpoch->myClock)
      return WeakAfter;
    if(myClock>otherEpoch->myClock)
      return WeakBefore;
    return Unordered;
  }
  Epoch::CmpResult Epoch::compareCheckClock(const Epoch *otherEpoch) const{
    if(myThread==otherEpoch->myThread){
      if(myClock<otherEpoch->myClock){
	I(myCheckClock<=otherEpoch->myCheckClock);
	return StrongAfter;
      }
      if(myClock>otherEpoch->myClock){
	I(myCheckClock>=otherEpoch->myCheckClock);
	return StrongBefore;
      }
      I(myClock!=otherEpoch->myClock);
    }
    if(myClock<otherEpoch->myClock){
      if((myCheckClock<otherEpoch->myCheckClock)||
	 ((!myThread->isCheckClockOwner())&&(myCheckClock==otherEpoch->myCheckClock)))
	return StrongAfter;
      return WeakAfter;
    }
    if(myClock>otherEpoch->myClock){
      if((myCheckClock>otherEpoch->myCheckClock)||
	 ((!otherEpoch->myThread->isCheckClockOwner())&&(myCheckClock==otherEpoch->myCheckClock)))
	return StrongBefore;
      return WeakBefore;
    }
    return Unordered;
  }
  void Epoch::advanceClock(const Epoch *predEpoch,bool sync){
    // cout << "beg advanceClock " << myClock << ":" << getTid() << " ";
    //    printEpochs();
    I(myThread!=predEpoch->myThread);
    // Find the new clock value
    ClockValue newClock=predEpoch->myClock+(sync?(getSyncClockDelta()+1):1);
    // Advance the clock only if new value larger than old
    if(newClock>myClock){
      I(myState==State::Initial);
      I(myState<State::LazyMerge);
      if(myState!=State::NoSuccessors){
	I(myState==State::Atm);
	I(myClock>=predEpoch->myClock);
	unSucceed();
      }
      I(myState==State::NoSuccessors);
      I(myState!=State::GlobalSafe);
      if(pendInstrCount){
	runtimeClockAdjustments++;
	BufBlocksAdjusted.addSample(bufferBlockMap.size());
      }
      I(predEpoch!=this);
      myClock=newClock;
      I(myClock>predEpoch->myClock);
      // Advance all buffered blocks
      for(BufferBlockList::const_iterator blockIt=bufferBlockList.begin();
	  blockIt!=bufferBlockList.end();blockIt++)
	(*blockIt)->getVersions()->advance(*blockIt);
      // Find new place in my thread's threadEpochs list
      if(myThreadEpochsPos!=myThread->threadEpochs.begin()){
	ID(EpochList::reverse_iterator newPosRIt(myThreadEpochsPos));
	I((*newPosRIt)->myClock>myClock);
      }
      // Find new place in the allEpochs list
      if(myAllEpochsPos!=allEpochs.begin()){
	EpochList::reverse_iterator newPosRIt(myAllEpochsPos);
	while((newPosRIt!=allEpochs.rend())&&(myClock>(*newPosRIt)->myClock)){
	  // Should not advance past any epochs in the same thread
	  I(myThread!=(*newPosRIt)->myThread);
	  newPosRIt++;
	}
	// If new place not the same as old, splice into new place
	if(myAllEpochsPos!=newPosRIt.base()){
	  EpochList::iterator oldNextPos=myAllEpochsPos;
	  oldNextPos--;
	  I(myState!=State::GlobalSafe);
	  allEpochs.splice(newPosRIt.base(),allEpochs,myAllEpochsPos);
	  ID(EpochList::iterator newPos=newPosRIt.base());
	  ID(newPos--);
	  I(myAllEpochsPos==newPos);
	  (*oldNextPos)->tryGlobalSave();
	}
      }
    }
    // If sync, we also advance the check clock and the lamport clock
    if(sync){
      ClockValue newCheckClock=predEpoch->myCheckClock;
      if(predEpoch->myThread->isCheckClockOwner())
	newCheckClock++;
      if(newCheckClock>myCheckClock){
	I(myState==State::Initial);
	myCheckClock=newCheckClock;
      }
      I((myState==State::Initial)||(LClock::isOrder(predEpoch->myLClock,myLClock)));
      myLClock.succeed(predEpoch->myLClock);
    }
    // cout << "end advanceClock " << myClock << ":" << getTid() << " ";
    // printEpochs();
  }

  void Epoch::initMergeHold(void){
    // Epoch has a PredHold until it is GlobalSafe
    I(myState<State::GlobalSafe);
    myState+=State::PredHold;
    if(mustFindAllRaces)
      myState+=State::RaceHold;
  }

  void Epoch::removeMergeHold(State::HoldEnum hold){
    myState-=hold;
    // If a hold is still left, we are done
    if(myState!=State::NoHold)
      return;
    EpochList::iterator currIt=myAllEpochsPos;
    while(currIt!=allEpochs.begin()){
      EpochList::iterator nextIt=currIt; nextIt--;
      // Can not remove PredHold until GlobalSafe
      if((*nextIt)->myState<State::GlobalSafe)
	break;
      (*nextIt)->myState-=State::PredHold;
      // Can not go further if successor still has a hold
      if((*nextIt)->myState!=State::NoHold)
	break;
      // OK, successor has no holds left, allow merge
      // of current epoch and move on to successor 
      if((*currIt)->myState==State::ReqLazyMerge){
	(*currIt)->requestLazyMerge();
      }else if((*currIt)->myState==State::ReqEagerMerge){
	(*currIt)->requestEagerMerge();
      }
      currIt=nextIt;
    }
    // Last epoch without holds can merge now
    if((*currIt)->myState==State::ReqLazyMerge){
      (*currIt)->requestLazyMerge();
    }else if((*currIt)->myState==State::ReqEagerMerge){
      (*currIt)->requestEagerMerge();
    }
    // If no epochs are left, there is nothing else to do
    if(allEpochs.empty())
      return;
    // If the last epoch is committed and without holds, we must
    // initiate the final EagerMerge to cleanly end execution
    Epoch *lastEpoch=allEpochs.front();
    // If still not committed, it is not time yet
    if(lastEpoch->myState<State::GlobalSafe)
      return;
    // If still has a hold, it is not time yet
    if(lastEpoch->myState!=State::NoHold)
      return;
    lastEpoch->requestEagerMerge();
  }

  Epoch::Epoch(ThreadID tid, Epoch *parentEpoch)
    : myPid(-1),
      beginDecisionExe(0), endDecisionExe(0),
      beginCurrentExe(0), endCurrentExe(0),
      pendInstrCount(0), execInstrCount(0), doneInstrCount(0),
      maxInstrCount(limitEpochInstrCount),
      myThread(parentEpoch?
	       new Thread(tid,parentEpoch->getTid(),parentEpoch->myClock):
	       new Thread(tid,invalidThreadID,smallClockValue)),
      myVClock(VClock::newVClock(parentEpoch?parentEpoch->myVClock:VClock::getSmallValue(),tid,true)),
      originalVClock(VClock::newVClock(myVClock,false)),
      parentClock(parentEpoch?parentEpoch->myClock:smallClockValue), 
      myClock(parentEpoch?(parentEpoch->myClock+getSyncClockDelta()+1):getGlobalSafeClock()),
      runtimeClockAdjustments(0), myCheckClock(0),
      myCheckpoint(0),
      myAllEpochsPos(allEpochs.insert(findPos(myClock),this)),
      myThreadEpochsPos(myThread->addEpoch(this)),
      myState(), spawnSuccessorsCnt(0), atmNestLevel(0),
      sysCallLog(), sysCallLogPos(sysCallLog.begin()),
      consumedBufferBlocks(0), producedBufferBlocks(0)
      {
    initMergeHold();
    myAtomicSection=0;
    skipAtm=false;
    if(parentEpoch){
      I(parentEpoch->getTid()!=getTid());
      myCheckClock=parentEpoch->myCheckClock;
      if(parentEpoch->myThread->isCheckClockOwner())
	myCheckClock++;
      myLClock.succeed(parentEpoch->myLClock);
    }
    numBegThread++;
    // The original context of the thread is frozen while its epochs execute
    osSim->stop(static_cast<Pid_t>(tid));
    // Create my SESC context
    createContext(osSim->getContext(tid));
    // Initial epoch is always ThreadSafe
    threadSave();
  }
  
  Epoch::Epoch(Epoch *parentEpoch)
    : myPid(-1),
      beginDecisionExe(0), endDecisionExe(0),
      beginCurrentExe(0), endCurrentExe(0),
      pendInstrCount(0), execInstrCount(0), doneInstrCount(0),
      maxInstrCount(limitEpochInstrCount),
      myThread(parentEpoch->myThread),
      myVClock(VClock::newVClock(parentEpoch->myVClock,true)),
      originalVClock(VClock::newVClock(myVClock,false)),
      parentClock(parentEpoch->myClock), myClock(parentClock+1),
      runtimeClockAdjustments(0), myCheckClock(parentEpoch->myCheckClock), myLClock(parentEpoch->myLClock),
      myCheckpoint(0),
      myAllEpochsPos(allEpochs.insert(findPos(myClock),this)),
      myThreadEpochsPos(myThread->addEpoch(this)),
      myState(), spawnSuccessorsCnt(0), atmNestLevel(0),
      sysCallLog(), sysCallLogPos(sysCallLog.begin()),
      consumedBufferBlocks(0), producedBufferBlocks(0){
    initMergeHold();
    myAtomicSection=0;
    skipAtm=false;
    parentEpoch->myState+=State::SpawnSuccessors;
    parentEpoch->spawnSuccessorsCnt++;
    // Create my SESC context
    createContext(osSim->getContext(parentEpoch->myPid));
  }
  
  Epoch *Epoch::initialEpoch(ThreadID tid, Epoch *parentEpoch){
    if(parentEpoch){
      if(!parentEpoch->changeEpoch())
	return 0;
    }
    Epoch *newEpoch=new Epoch(tid,parentEpoch);
    I((!parentEpoch)||(parentEpoch->myState==State::Completed));
    newEpoch->run();
    return newEpoch;
  }

  void Epoch::run(void){
    I(myState==State::Spawning);
    // Current execution begins now
    beginCurrentExe=globalClock+1;
    I(sysCallLogPos==sysCallLog.begin());
    // ID(printf("Executing %ld:%d",myClock,getTid()));
    // ID(printf((myState==State::FullReplay)?" in replay\n":"\n"));
    if(myState==State::NoWait){
      // Epoch can begin to actually execute now
      myState=State::Running;
      osSim->unstop(myPid);
    }else{
      I(0);
      // Epoch needs to wait for another event before it actually runs
      myState=State::Waiting;
    }
  }

  Epoch *Epoch::spawnEpoch(void){
    I(myState==State::Running);
    I(myState!=State::Atm);
    I(!atmNestLevel);
    // Try to find a matching epoch already spawned in a previous execution
    if(myState==State::SpawnSuccessors){
      EpochList::reverse_iterator searchIt(myThreadEpochsPos);
      for(EpochList::reverse_iterator searchIt(myThreadEpochsPos);
	  searchIt!=myThread->threadEpochs.rend();searchIt++){
	I((*searchIt)->myThread==myThread);
	if((*searchIt)->parentClock!=myClock)
	  continue;
	if((*searchIt)->myState!=State::Spawning)
	  continue;
	Epoch *newEpoch=(*searchIt);
	if(newEpoch->myContext.getPCIcode()!=
	   osSim->eventGetInstructionPointer(myPid))
	  return 0;
	return newEpoch;
      }
    }
    // If no new epoch found, create it now
    return new Epoch(this);
  }
  
  Epoch::AddrToCount Epoch::atomicEntryCount;
  size_t Epoch::staticAtmOmmitCount;
  size_t Epoch::dynamicAtmOmmitCount;
  VAddr  Epoch::atmOmmitInstr;
  size_t Epoch::atmOmmitCount;
  
  void Epoch::beginReduction(VAddr iVAddr){
    beginAtomic(iVAddr,false,false);
  }

  void Epoch::endReduction(void){
    endAtomic();
  }

  void Epoch::beginAtomic(VAddr iVAddr, bool isAcq, bool isRel){
    // I(myState==State::NoAcq);
    I(myState==State::NoRel);
    I(atmNestLevel||(pendInstrCount==1));
    atmNestLevel++;
    if(myState==State::NoAtm){
      if(skipAtm)
	return;
      myAtomicSection=iVAddr;
      if(!(--dynamicAtmOmmitCount)){
	skipAtm=true;
	atmOmmitInstr=iVAddr;
	I(!atmOmmitCount);
	atmOmmitCount++;
	return;
      }
    }
    myState=State::Atm;
    if(isAcq){
      //I(atmNestLevel==1);
      //I(myState<State::GlobalSafe);
      myState=State::Acq;
    }
    if(isRel){
      // I(atmNestLevel==1);
      myState=State::Rel;
    }
  }

  void Epoch::retryAtomic(void){
    printf("Retry squash\n");
    // I(atmNestLevel==1);
    if(myState!=State::Atm){
      I(myAtomicSection==atmOmmitInstr);
      squash(false);
      return;
    }
    I(myState==State::Atm);
    I(myState==State::Acq);
    //I(myState<State::GlobalSafe);
    I((myState!=State::Initial)||(myState!=State::SpawnSuccessors));
    // I(myState!=State::FlowSuccessors);
    waitAcqRetry();
  }

  void Epoch::changeAtomic(bool endAcq, bool begRel){
    // I(atmNestLevel); 
   if(myState!=State::Atm){
      I(myAtomicSection==atmOmmitInstr);
      return;
    }
    I(myState==State::Atm);
    I(endAcq||begRel);
    if(endAcq){
      I(myState==State::Acq);
      myState=State::NoAcq;
      tryGlobalSave();      
    }
    if(begRel){
      I(myState==State::NoRel);
      myState=State::Rel;
    }
  }
  
  void Epoch::endAtomic(void){
    // I(atmNestLevel>0);
    atmNestLevel--;
    if(myState!=State::Atm){
      I(myAtomicSection==atmOmmitInstr);
      return;
    }
    if(!atmNestLevel){
      myState=State::NoAtm;
      // Update statistics for atomic sections
      AtomicCommitInstrs.addSample(pendInstrCount);
      AtomicTotalBlocks.addSample(bufferBlockMap.size());
      AtomicConsumedBlocks.addSample(consumedBufferBlocks);
      AtomicProducedBlocks.addSample(producedBufferBlocks);
    }
    // Handle acquire/release specifics
    if(myState==State::Acq){
      // I(!atmNestLevel);
      myState=State::NoAcq;
    }
    if(myState==State::Rel){
      // I(!atmNestLevel);
      myState=State::NoRel;
    }
    if(!atmNestLevel){
      Epoch *newEpoch=changeEpoch();
      if(!newEpoch)
	return;
      I(newEpoch->myCheckClock>=myCheckClock);
      if(myThread->isCheckClockOwner()){
	if(newEpoch->myCheckClock==myCheckClock)
	  newEpoch->myCheckClock++;
	I(newEpoch->myCheckClock==myCheckClock+1);
      }
      newEpoch->myLClock.advance();
    }
    // Being Atm prevents an epoch from becoming GlobalSafe,
    // so now we need to retry to become GlobalSafe
    tryGlobalSave();
  }

  void Epoch::unSucceed(void){
    I(myState!=State::NoSuccessors);
    // Squash, but skip self
    squash(true);
    // No longer succeeded
    I(!spawnSuccessorsCnt);
    myState=State::NoSuccessors;
  }

  void Epoch::threadSave(void){
    I(myState==State::Spec);
    I(myState!=State::WaitUnspawn);
    I(myThread->threadSafeAvailable==true);
    myState=State::ThreadSafe;
    myThread->threadSafeAvailable=false;
    myThread->moveThreadSafe();
    if(myState==State::WaitThreadSafe){
      myState=State::Running;
      myState=State::NoWait;
      osSim->unstop(myPid);
    }else if(myState==State::WaitAcqRetry){
      myThread->wait();
    }else if(myState==State::WaitChild){
      if(myThread->waitCalled(this)){
	I(!myThread->isWaiting);
	endWaitChild();
      }
    }else if(myState==State::WaitZombie){
      if(myThread->exitCalled(this)){
	I(!myThread->isWaiting);
	endWaitZombie();
      }
    }
    // Now try to become GlobalSafe
    if(globalSafeAvailable)
      tryGlobalSave();
  }
  
  void Epoch::complete(void){
    I(pendInstrCount);
    I(!atmNestLevel);
    I(myState==State::Running);
    ID(printf("Ending %ld:%d numInstr %lld\n",
	      myClock,getTid(),pendInstrCount));
    // Remember the end time
    endCurrentExe=globalClock+1;
    // Disable SESC execution of this epoch
    osSim->stop(myPid);
    I(sysCallLogPos==sysCallLog.end());
    // Change the epoch's execution state to completed
    myState=State::Completed;
    // Must not be in an atomic section when it ends
    I(myState==State::NoAtm);
    // If FullReplay, adjust event times in the trace
    if(myState==State::FullReplay){
      I(pendInstrCount==maxInstrCount);
      // Adjust the event times in the trace
      for(TraceList::iterator traceIt=myTrace.begin();
	  traceIt!=myTrace.end();traceIt++){
	TraceEvent *traceEvent=*traceIt;
	traceEvent->adjustTime(this);
      }
      for(EventSet::iterator eventIt=accessEvents.begin();
	  eventIt!=accessEvents.end();eventIt++){
	TraceAccessEvent *accessEvent=*eventIt;
	accessEvent->adjustTime(this);
      }
    }
    if(threadsSequential){
      EpochList::reverse_iterator threadItR(myThreadEpochsPos);
      if(threadItR!=myThread->threadEpochs.rend())
	(*threadItR)->run();
    }
    if(myState==State::GlobalSafe){
      tryCommit();
    }else{
      tryGlobalSave();
    }
  }
  
  void Epoch::advanceGlobalSafe(void){
    globalSafeAvailable=true;
    if(globalSafe!=allEpochs.begin()){
      EpochList::reverse_iterator globalSafeRIt(globalSafe);
      I(globalSafeRIt!=allEpochs.rend());
      (*globalSafeRIt)->tryGlobalSave();
    }
  }

  void Epoch::tryGlobalSave(void){
    I(myState!=State::WaitUnspawn);
    if(!globalSafeAvailable)
      return;
    if(myState!=State::ThreadSafe)
      return;
    if(!pendInstrCount)
      return;
    EpochList::iterator saveCheckIt=myAllEpochsPos;
    saveCheckIt++;
    I((saveCheckIt==globalSafe)==((saveCheckIt==allEpochs.end())||
       ((*saveCheckIt)->myState>=State::Committed)));
    if(saveCheckIt!=globalSafe)
      return;
    // If our thread is waiting, we can not proceed until some
    // event outside of this thread advances our clock
    // However, that event may require GlobalSafe status, and
    // if we stay here we are blocking GlobalSafe's advance.
    // To prevent deadlocks, we move the waiting epoch to come
    // after the oldest still-active epoch in allEpochs 
    if((myThread->isWaiting)&&(myState==State::Initial)){
      I((myState==State::WaitChild)||
	(myState==State::WaitZombie)||
	(myState==State::WaitAcqRetry));
      EpochList::iterator nextActiveIt=myAllEpochsPos;
      while(((*nextActiveIt)->myThread->isWaiting)||
            ((*nextActiveIt)->myState==State::Completed)){
        I(nextActiveIt!=allEpochs.begin());
        nextActiveIt--;
      }
      advanceClock(*nextActiveIt,false);
      // The advanceClock method advances the GlobalSafe status
      // so we need not advance it further... but we check that
      ID(EpochList::reverse_iterator globalSafeRIt(globalSafe));
      ID(if(globalSafeRIt!=allEpochs.rend()) (*globalSafeRIt)->tryGlobalSave());
      I((globalSafeRIt==allEpochs.rend())||((*globalSafeRIt)->myState<State::GlobalSafe));
      return;
    }
    // Atm can increment the clock, and a GlobalSafe epoch must not do that
    // Also, we must execute the first instruction to see if the epoch will enter Atm
    if((myState==State::Atm)||(!execInstrCount))
      return;
    globalSave();
  }

  void Epoch::globalSave(void){
    I(pendInstrCount);
    I(globalSafeAvailable);
    I(myState==State::ThreadSafe);
    I(myState==State::NoAcq);
    I(myState!=State::WaitChild);
    myState=State::GlobalSafe;
    globalSafeAvailable=false;
    // The globalSafe iterator is still pointing at the
    // immediate predecessor, which should be committed
    I((globalSafe==allEpochs.end())||((*globalSafe)->myState>=State::Committed));
    // If the predecessor can merge, we can remove our own PredHold 
    bool noPredHold=(globalSafe==allEpochs.end())||
                    ((*globalSafe)->myState==State::NoHold);
    globalSafe--;
    I((*globalSafe)==this);
    I(myState<State::LazyMerge);
    if(myState==State::WaitGlobalSafe){
      myState=State::Running;
      myState=State::NoWait;
      osSim->unstop(myPid);
    }
    // Remove our PredHold if possible
    if(noPredHold)
      removeMergeHold(State::PredHold);
    // Now try to also commit this epoch
    // Note: commit can destroy the epoch, so it must
    // be done as the last thing in this method
    tryCommit();
  }

  void Epoch::tryCommit(void){
     if(myState!=State::Committed)
  		  I(myState!=State::Committed);
    I((myState==State::Completed)||(myState==State::GlobalSafe));
    if(myState!=State::Completed)
      return;
    if(myState!=State::GlobalSafe)
      return;
    I(pendInstrCount>=execInstrCount);
    I(execInstrCount>=doneInstrCount);
    if(doneInstrCount!=pendInstrCount)
      return;
    commit();
  }

  void Epoch::commit(void){
    ID(printf("Committing %ld:%d numInstr %lld\n",myClock,getTid(),pendInstrCount));
    ID(Pid_t myPidTmp=myPid);
    I(myState==State::GlobalSafe);
    //If the epoch is already fully merged, it will be destroyed when commit is done
    bool destroyNow=((myState>=State::LazyMerge)&&bufferBlockList.empty());
    if(myState==State::FullReplay){
      I(pendInstrCount==maxInstrCount);
      I(beginDecisionExe);
      I(endDecisionExe);
    }else{
      I(!beginDecisionExe);
      I(!endDecisionExe);
      // Update Statistics
      NumRunClockAdj.addSample(runtimeClockAdjustments);
      // Update race detection info
      allInstRaces.add(myInstRaces);
      // Remember the start and end time
      beginDecisionExe=beginCurrentExe;
      endDecisionExe=endCurrentExe;
    }
    myInstRaces.clear();
    I(beginCurrentExe);
    I(beginCurrentExe<=endCurrentExe);
    // Remeber the instruction count
    maxInstrCount=pendInstrCount;
    // Transition into the Committed state
    myState=State::Committed;
    // We held the GlobalSafe and ThreadSafe status
    myThread->threadSafeAvailable=true;
    // Pass on the ThreadSafe status
    EpochList::reverse_iterator threadSaveRIt(myThreadEpochsPos);
    if(threadSaveRIt!=myThread->threadEpochs.rend())
      (*threadSaveRIt)->threadSave();
    // A threadSave() can squash the epoch for replay,
    // in which case nothing else should be done in commit()
    if(myState<State::Committed)
      return;
    myThread->commitEpoch(this);
    I((destroyNow&&(pidToEpoch[myPidTmp]==this))||
      ((!destroyNow)&&((pidToEpoch[myPidTmp]==0)||(myState<State::LazyMerge)||(!bufferBlockList.empty()))));
    //if(destroyNow)
    //Prevent block deletion, as backend pointers may still refer to the block
    //delete this;
    advanceGlobalSafe();
  }

  bool Epoch::forceEpochBeginning(void){
    I(myState==State::Running);
    if(myState==State::Atm)
      return false;
    if(!pendInstrCount)
      return false;
    changeEpoch();
    return true;
  }

  void Epoch::waitAcqRetry(void){
    //I(myState<State::GlobalSafe);
    I(myState==State::Running);
    I(myState==State::Acq);
    for(BufferBlockList::iterator blockIt=bufferBlockList.begin();
        blockIt!=bufferBlockList.end();blockIt++){
      if((*blockIt)->isStale()){
	squash(false);
	return;
      }
    }
    //Add to map of Epochs waiting to be squashed
    //Check epoch is not already in map
    I(waitAcqMap.find(this)==waitAcqMap.end());
    //Else add epoch and timestamp it
    std::pair <Epoch *,Time_t> pwait ((Epoch *)this,globalClock);
    waitAcqMap.insert(pwait);
    
    myState=State::Waiting;
    myState=State::WaitAcqRetry;
    if(myState==State::ThreadSafe)
      myThread->wait();
    osSim->stop(myPid);
  }
  
  void Epoch::waitChild(void){
    I(myState<State::GlobalSafe);
    I(myState!=State::Acq);
    if(myState==State::Running){
      myState=State::Waiting;
      osSim->stop(myPid);
    }else{
      I(threadsSequential&&(myState==State::Spawning));
    }
    myState=State::WaitChild;
    I((myState!=State::ThreadSafe)||myThread->isWaiting);
  }

  void Epoch::endWaitChild(void){
    I(myState==State::Waiting);
    I(myState==State::WaitChild);
    myState=State::NoWait;
    myState=State::Running;
    I(myState==State::ThreadSafe);
    if(myThread->isWaiting)
      myThread->proceed();
    osSim->unstop(myPid);
  }
  void Epoch::waitZombie(void){
    I(myState<State::GlobalSafe);
    I(myState!=State::Acq);
    if(myState==State::Running){
      myState=State::Waiting;
      osSim->stop(myPid);
    }else{
      I(threadsSequential&&(myState==State::Spawning));
    }
    myState=State::WaitZombie;
    I((myState!=State::ThreadSafe)||myThread->isWaiting);
  }
  void Epoch::endWaitZombie(void){
    I(myState==State::Waiting);
    I(myState==State::WaitZombie);
    myState=State::NoWait;
    myState=State::Running;
    I((myState==State::ThreadSafe)||(myState==State::GlobalSafe));
    if(myThread->isWaiting)
      myThread->proceed();
    osSim->unstop(myPid);
    complete();
  }
  
  void Epoch::waitCalled(void){
    I(pendInstrCount==1);
    I(myState<State::GlobalSafe);
    I(myState==State::Running);
    I(myState!=State::Acq);
    if((myState!=State::ThreadSafe)||!myThread->waitCalled(this))
      waitChild();
  }

  void Epoch::exitCalled(void){
    I(pendInstrCount==1);
    I(myState<State::GlobalSafe);
    I(myState==State::Running);
    I(myState!=State::Acq);
    if(myThread->parentID==invalidThreadID){
      complete();
      return;
    }
    if((myState!=State::ThreadSafe)||!myThread->exitCalled(this)){
      waitZombie();
    }else{
      complete();
    }
  }

  struct PredDescriptor{
    Thread *thread;
    ClockValue clock;
    Epoch *epoch;
    PredDescriptor(Thread *thread, ClockValue clock, Epoch *epoch)
      : thread(thread), clock(clock), epoch(epoch){
    }
    bool operator<(const PredDescriptor &other) const{
      if(thread<other.thread)
	return true;
      if(thread>other.thread)
	return false;
      return clock<other.clock;
    }
  };

  struct EpochDescriptor{
    Epoch *currentEpoch;
    Epoch *parentEpoch;
    EpochDescriptor(Epoch *currentEpoch, Epoch *parentEpoch)
      : currentEpoch(currentEpoch), parentEpoch(parentEpoch){
    }
    bool operator==(const Epoch &other) const{
      return currentEpoch==&other;
    }
  };

  void Epoch::squash(bool skipSelf){
    //I(myState<State::LazyMerge);
    I(!myCheckpoint);
    // Roll back the Spec state if needed
    if(myState>=State::GlobalSafe){
      EpochList::reverse_iterator currRIt(myAllEpochsPos);
      for(currRIt--;currRIt!=allEpochs.rend();currRIt++){
	Epoch *currEpoch=(*currRIt);
        bool moveGlobalSafe=false;
        bool moveThreadSafe=false;
	if(currEpoch->myState!=State::Spec){
          if(currEpoch->myState==State::Committed){
            currEpoch->myState=State::FullReplay;
            moveGlobalSafe=true;
            moveThreadSafe=true;
	  }else if(currEpoch->myState==State::GlobalSafe){
            currEpoch->myState=State::PartReplay;
            moveGlobalSafe=true;
            moveThreadSafe=true;
	  }else if(currEpoch->myState==State::ThreadSafe){
            moveThreadSafe=true;
	  }
	  currEpoch->myState=State::Spec;
          if(moveGlobalSafe){
            I(globalSafeAvailable||(globalSafe!=allEpochs.end()));
            if((globalSafe!=allEpochs.end())&&((*globalSafe)->myClock>=currEpoch->myClock)){
              globalSafeAvailable=true;
              globalSafe=currEpoch->myAllEpochsPos;
              globalSafe++;
              I((globalSafe==allEpochs.end())||((*globalSafe)->myState==State::Committed));
            }else{
              I(globalSafeAvailable);
              I((globalSafe==allEpochs.end())||((*globalSafe)->myState==State::Committed));
            }
          }
          if(moveThreadSafe){
            Thread *currThread=currEpoch->myThread;
            I(currThread->threadSafeAvailable||(currThread->threadSafe!=currThread->threadEpochs.end()));
            if((currThread->threadSafe!=currThread->threadEpochs.end())&&
               ((*(currThread->threadSafe))->myClock>=currEpoch->myClock)){
              currThread->threadSafeAvailable=true;
              currThread->threadSafe=currEpoch->myThreadEpochsPos;
              currThread->threadSafe++;
#if (defined DEBUG)
              if(currThread->threadSafe==currThread->threadEpochs.end()){
		ID(currThread->lastCommitEpoch=0);
	      }else{
		ID(currThread->lastCommitEpoch=*(currThread->threadSafe));
                I(currThread->lastCommitEpoch->myState==State::Committed);
	      }
#endif
	      if(currThread->isWaiting)
		currThread->proceed();
	    }
	    I(currThread->threadSafeAvailable);
	    I((currThread->threadSafe==currThread->threadEpochs.end())||
	      ((*(currThread->threadSafe))->myState==State::Committed));
	    I(!currThread->isWaiting);
          }
	}
      }
    }
    // If we have a SysCall, find if there are any system calls in succesor epochs
    bool hasSuccSysCalls=false;
    if(!sysCallLog.empty()){
      for(EpochList::iterator listIt=allEpochs.begin();
  	  listIt!=myAllEpochsPos;listIt++){
        if(!(*listIt)->sysCallLog.empty()){
	  hasSuccSysCalls=true;
	  break;
        }
      }
    }
    // Try to squash this epoch alone and avoid squashing successors
    while(true){
      if(skipSelf)
	break;
      if(myState==State::FlowSuccessors)
	break;
      if(myState==State::NoHold)
        break;
      if(hasSuccSysCalls)
	break;
      if((threadsSequential||(myState==State::Initial))&&(myState==State::SpawnSuccessors))
	break;
      undoSysCalls();
      squashLocal();
      return;
    }
    // Determine which epochs are unspawned and which are restarted
    typedef std::set<PredDescriptor> PredDescriptors;
    typedef list<EpochDescriptor> EpochDescriptors;
    PredDescriptors specSpawners;
    EpochDescriptors specSpawns;
    if(myState==State::Initial)
      specSpawners.insert(PredDescriptor(myThread,myClock,this));
    for(EpochList::reverse_iterator currRIt(myAllEpochsPos);
	currRIt!=allEpochs.rend();currRIt++){
      Epoch *currEpoch=(*currRIt);
      if(currEpoch->myClock==myClock)
	continue;
      PredDescriptor currDesc(currEpoch->myThread,currEpoch->myClock,currEpoch);
      PredDescriptor prevDesc(currEpoch->myThread,currEpoch->parentClock,currEpoch);
      if(currEpoch->myState==State::Initial){
	I(specSpawners.find(currDesc)==specSpawners.end());
	specSpawners.insert(currDesc);
      }
      PredDescriptors::iterator prevIt=specSpawners.find(prevDesc);
      if(prevIt!=specSpawners.end())
	specSpawns.push_front(EpochDescriptor(currEpoch,prevIt->epoch));
    }
    // Now we will undo SysCall entries for epochs with clocks
    // higher than "this" epoch, staring from most recent epoch
    for(EpochList::iterator squashIt=allEpochs.begin();
	(*squashIt)->myClock>myClock;squashIt++)
      (*squashIt)->undoSysCalls();
    if(!skipSelf)
      undoSysCalls();
    // Unspawn speculatively created epochs
    for(EpochDescriptors::iterator specSpawnIt=specSpawns.begin();
	specSpawnIt!=specSpawns.end();specSpawnIt++){
      Epoch *currentEpoch=specSpawnIt->currentEpoch;
      Epoch *parentEpoch=specSpawnIt->parentEpoch;
      I(currentEpoch->myState!=State::SpawnSuccessors);
      I(!currentEpoch->spawnSuccessorsCnt);
      I(currentEpoch->myState==State::Spec);
      I(parentEpoch->myState==State::SpawnSuccessors);
      I(parentEpoch->spawnSuccessorsCnt>0);
      I(parentEpoch->myState==State::Initial);
      currentEpoch->unspawn();
      parentEpoch->spawnSuccessorsCnt--;
      if(!parentEpoch->spawnSuccessorsCnt)
	parentEpoch->myState-=State::SpawnSuccessors;
    }
    if(!skipSelf)
      squashLocal();
    // Squash non-speculatively created epochs with
    // clocks greater than "this" epoch
    for(EpochList::reverse_iterator squashRIt(myAllEpochsPos);
	squashRIt!=allEpochs.rend();squashRIt++){
      Epoch *squashEpoch=*squashRIt;
      if(squashEpoch->myClock==myClock)
	continue;
      squashEpoch->squashLocal();
      I((squashEpoch->myState!=State::Initial)||
	(squashEpoch->myState==State::NoSuccessors));
    }
  }

  void Epoch::undoSysCalls(void){
    if(myState==State::Initial){
      I(sysCallLogPos==sysCallLog.end());
      // Undo system calls and destroy the log
      while(!sysCallLog.empty()){
	SysCall *sysCall=sysCallLog.back();
	sysCallLog.pop_back();
	sysCall->undo(false);
	delete sysCall;
      }
    }else{
      // Undo starting from the last entry that was exec-ed
      for(SysCallLog::reverse_iterator logRIt(sysCallLogPos);
	  logRIt!=sysCallLog.rend();logRIt++)
	(*logRIt)->undo(true);
      sysCallLogPos=sysCallLog.begin();
    }
  }

  void Epoch::squashLocal(void){
    // If this epoch did nothing, no need to squash it
    if(myState==State::Spawning)
      return;
    //See if epoch was waiting to be squashed
    if (waitAcqMap.find(this)!=waitAcqMap.end())
    {
    	waitAcqCycles.add((globalClock-waitAcqMap[this]));
    	waitAcqMap.erase(this);
    }	
    
    ID(printf("Squashing %ld:%d numInstr %lld\n",myClock,getTid(),pendInstrCount));
    I(myState<State::LazyMerge);
    initMergeHold();
    I(!myCheckpoint);
    I(!(myState==State::Committed));
    // Roll back the Exec state
    if((myState==State::ThreadSafe)&&myThread->isWaiting)
      myThread->proceed();
    if(myState==State::Running)
      osSim->stop(myPid);
    myState=State::Spawning;
    myState=State::NoWait;
    // Roll back the sync state
    if(myState==State::Atm){
      atmNestLevel=0;
      AtomicSquashInstrs.addSample(pendInstrCount);
    }
    myState=State::NoAtm;
    myState=State::NoAcq;
    myState=State::NoRel;
    // Spec state already rolled back in the global squash method
    I(myState!=State::Committed);
    // Erase buffered blocks and clear block removal requests
    eraseBuffer();
    pendingBlockRemovals.erase(this);
    // No buffered blocks -> no flow and name successors
    myState-=State::DataSuccessors;
    I((!spawnSuccessorsCnt)==(myState==State::NoSuccessors));
    // Restore initial SESC execution context
    ThreadContext *context=osSim->getContext(myPid);
    *context=myContext;

    SquashInstructions.addSample(pendInstrCount);
    doneInstrCount-=pendInstrCount;
    execInstrCount-=pendInstrCount;
    pendInstrCount=0;
    // Clear out epoch's race detection info
    myInstRaces.clear();

    beginCurrentExe=0;
    endCurrentExe=0;
    if(myState==State::Initial)
      myVClock->restore(originalVClock);
    // Try to restart execution and transition into less speculative states
    if(myThread->threadSafeAvailable){
      EpochList::iterator checkIt=myThreadEpochsPos;
      checkIt++;
      if((checkIt==myThread->threadEpochs.end())||((*checkIt)->myState==State::Committed))
	threadSave();
    }
    if(!threadsSequential){
      run();
    }else if(parentClock>myThread->spawnClock){
      EpochList::iterator threadIt=myThreadEpochsPos;
      threadIt++;
      if((threadIt==myThread->threadEpochs.end())||((*threadIt)->myState==State::Completed))
	run();
    }else{
      I(parentClock==myThread->spawnClock);
      Epoch *spawnEpoch=myThread->getSpawnEpoch();
      if((!spawnEpoch)||spawnEpoch->myState==State::Completed)
	run();
    }
#if (defined DEBUG)
    for(EpochList::iterator allChkIt=allEpochs.begin();allChkIt!=allEpochs.end();allChkIt++)
      I((*allChkIt)->myState!=State::WaitUnspawn);
    for(size_t i=0;i<Thread::threadVector.size();i++){
      if(!Thread::threadVector[i])
	continue;
      for(EpochList::iterator thrChkIt=Thread::threadVector[i]->threadEpochs.begin();
	  thrChkIt!=Thread::threadVector[i]->threadEpochs.end();thrChkIt++)
	I((*thrChkIt)->myState!=State::WaitUnspawn);
    }
#endif
  }

  void Epoch::unspawn(void){
    ID(printf("Unspawning %ld:%d numInstr %lld\n",myClock,getTid(),pendInstrCount));
    I(myState==State::Spec);
    I(myState==State::Initial);
    // Roll back the Exec state
    if(myState==State::Running)
      osSim->stop(myPid);
    // This epoch is waiting to be unspawned
    myState=State::Waiting;
    myState=State::WaitUnspawn;
    // Erase buffered blocks
    eraseBuffer();
    // Remove from pending-block-removal list
    pendingBlockRemovals.erase(this);
    // Undo system calls and destroy the log
    I(sysCallLog.empty());
    // Remove from list of all epochs
    I(myAllEpochsPos!=globalSafe);
    allEpochs.erase(myAllEpochsPos);
    myAllEpochsPos=allEpochs.end();
    // Remove from thread's list of epochs
    bool lastInThread=myThread->removeEpoch(this);
    I(!lastInThread);
    	//Prevent block deletion, as backend pointers may still refer to the block
    //Destroy the epoch if all instructions have gone through
    //if(doneInstrCount==pendInstrCount)
    //  delete this;
  }

  void Epoch::requestEagerMerge(void){
    I(this);
    // If already EagerMerge, do nothing
    if(myState==State::EagerMerge)
      return;
    // If the epoch is not GlobalSafe, its ordering may change and
    // it is still not safe to request eager merge from predecesors
    if(myState<State::GlobalSafe){
      // Remember the request to repeat it when we are GlobalSafe
      if(myState<State::ReqEagerMerge)
	myState=State::ReqEagerMerge;
      return;
    }
    // We can request eager merge from this epoch and its predecesors
    // First, we find the predecssors that need an eager merge request
    EpochList::iterator mergIt=myAllEpochsPos;
    for(mergIt++;mergIt!=allEpochs.end();mergIt++){
      // Need no request if already have one
      if((*mergIt)->myState==State::ReqEagerMerge)
	break;
      // Need n orequest if already in EagerMerge
      if((*mergIt)->myState==State::EagerMerge)
	break;
    }
    // Merging may actually delete "this", so we remember it now
    // and do not refer to "this" in the rest of this method
    Epoch *thisEpoch=this;
    EpochList::reverse_iterator mergItR(mergIt);
    while(true){
      Epoch *mergEpoch=*mergItR;
      if(mergEpoch->myState!=State::NoHold){
	I(mergEpoch->myState<State::LazyMerge);
	mergEpoch->myState=State::ReqEagerMerge;
      }else{
	if(mergEpoch->myState!=State::LazyMerge)
	  mergEpoch->beginLazyMerge();
	if(mergEpoch==*mergItR)
	  mergEpoch->beginEagerMerge();
      }
      if(mergEpoch==thisEpoch)
	break;
      I(mergItR!=allEpochs.rend());
      if(mergEpoch==(*mergItR))
	mergItR++;
    }
  }

  void Epoch::requestLazyMerge(void){
    I(this);
    // If already LazyMerge, do nothing
    if(myState>=State::LazyMerge)
      return;
    // To start merging this epoch, we must start merging
    // all its predecessors and get the GlobalSafe status
    // First, we seek to the youngest merging epoch
    EpochList::iterator mergIt=myAllEpochsPos;
    while((mergIt!=allEpochs.end())&&((*mergIt)->myState<State::LazyMerge))
      mergIt++;
    // If that epoch is not completed yet, it must also be dealt with
    if((mergIt!=allEpochs.end())&&((*mergIt)->myState!=State::Completed))
      mergIt++;
    // This points to the oldest epoch that is posing a problem
    EpochList::reverse_iterator mergItR(mergIt);
    // Deal with the predecessors
    while((*mergItR)!=this){
      Epoch *mergEpoch=*mergItR;
      // A non-committed predecessor stops advance of GlobalSafe
      if(mergEpoch->myState<State::Committed){
	// First see if we can move it out of the way
	if(((mergEpoch->myState<State::ReqLazyMerge)||(mergEpoch->myState>=State::WaitUnspawn))&&
	   mergEpoch->canEasilyAdvance()){
          Pid_t thePid=myPid;
	  mergEpoch->advanceClock(this,false);
          // Advancing a predecessor may commit and destroy "this" epoch,
          // in which case this method's function is done and we must return
          if(!pidToEpoch[thePid])
            return;
	  // The epoch should be moved from where it was
	  //I((*mergItR)!=mergEpoch);
	  // No need to increment mergItR (already points to next epoch)
	  continue;
	}
	// We can't advance it, can we truncate it?
	if(mergEpoch->canBeTruncated()){
	  Epoch *newEpoch=mergEpoch->changeEpoch();
	  // Move the new epoch out of the way
	  newEpoch->advanceClock(this,false);
	  // The old epoch should still be there and be Completed now
	  I((*mergItR)==mergEpoch);
	  I(mergEpoch->myState==State::Completed);
	  // Epoch can be committed and deleted when it completes,
	  // in which case mergItR already points to the next epoch
	  if((*mergItR)!=mergEpoch)
	    continue;
	}
	// Note: mergEpoch can still be uncommitted at this time because
	// 1) if it is GlobalSafe and Completed, it still can't commit
	//    until all of its pending and executed instructions are done
	// 2) some epochs can not be moved or truncated (e.g. during replay)
      }
      // I(mergEpoch->myState!=State::Waiting);
      if(mergEpoch->myState==State::NoHold){
	// If there is no hold left, epoch can merge (if not merging already)
	if(mergEpoch->myState<State::LazyMerge)
	  mergEpoch->beginLazyMerge();
      }else{
	// There is a hold, remember the request to retry it later
	if(mergEpoch->myState<State::ReqLazyMerge)
	  mergEpoch->myState=State::ReqLazyMerge;
      }
      if((*mergItR)==mergEpoch)
	mergItR++;
    }
    I((*mergItR)==this);
    if(myState==State::NoHold){
      // If there is no hold left, epoch can merge (if not merging already)
      if(myState<State::LazyMerge)
	beginLazyMerge();
    }else{
      // Can't merge now, but remember the request
      if(myState<State::ReqLazyMerge)
	myState=State::ReqLazyMerge;
    }
  }
//    // If we get here we are not yet Merging, but we can become
//    // In a "normal" situation, we just need to call beginMerge
//    if(!testRollback){
//      beginMerge(true);
//      return;
//    }
//    // However, if testing rollbacks, we need to do weird, weird things
//    if((myClock<=clockAtLastRollback)&&
//       ((!squashToTestRollback)||(myClock<squashToTestRollback->myClock))){
//      beginMerge(true);
//      return;
//    }
//    EpochList::reverse_iterator epochRIt=allEpochs.rbegin();
//    I(epochRIt!=allEpochs.rend());
//    while((*epochRIt)->myClock<=clockAtLastRollback){
//      epochRIt++;
//      I(epochRIt!=allEpochs.rend());
//    }
//    I((*epochRIt)->myState!=State::Merging);
//    clockAtLastRollback=myClock;
//    myState=State::PassiveMerge;
//    if((!squashToTestRollback)||(squashToTestRollback->myClock>(*epochRIt)->myClock))
//      squashToTestRollback=(*epochRIt);
//  }

  void Epoch::beginLazyMerge(void){
    ID(Pid_t myPidTmp=myPid);
    I(myState<State::LazyMerge);
    I(myState>=State::GlobalSafe);
    numEpochs++;
    if(myThread->canMissRaces(this)){
      I(!mustFindAllRaces);
      numRaceMissEpochs++;
    }
    // Predecessor should already be in a merge state
    ID(EpochList::iterator nextPosIt=myAllEpochsPos);
    ID(nextPosIt++);
    I((nextPosIt==allEpochs.end())||((*nextPosIt)->myState>=State::LazyMerge));
    // Update per-epoch statistics
    SysCallCount.addSample(sysCallLog.size());
    EpochInstructions.addSample(pendInstrCount);
    EpochBuffSpace.addSample(bufferBlockMap.size()*blockSize);
    // Update atomic section entry counts
    if(myAtomicSection){
      if(atomicEntryCount.find(myAtomicSection)==atomicEntryCount.end())
	atomicEntryCount[myAtomicSection]=0;
      atomicEntryCount[myAtomicSection]++;
    }
    // Now my state is LazyMerge
    myState=State::LazyMerge;
    // Find the checkpoint to merge into and start the merging process
    myCheckpoint=Checkpoint::mergeInit(this);
    I(pidToEpoch[myPidTmp]==this);
    if(blockRemovalEnabled)
      doBlockRemovals();
  }
  
  void Epoch::beginEagerMerge(void){
    I(myState>=State::GlobalSafe);
    I(myState==State::LazyMerge);
    // Next epoch should already be EagerMerging
    ID(EpochList::iterator nextPosIt=myAllEpochsPos);
    ID(nextPosIt++);
    I((nextPosIt==allEpochs.end())||((*nextPosIt)->myState==State::EagerMerge));
    myState=State::EagerMerge;
    if(blockRemovalEnabled){
      // For now we immediatelly merge and erase the entire buffer on EagerMerge
      mergeBuffer();
      eraseBuffer();
    }else{
      pendingBlockRemovals.insert(this);
    }
  }

  Epoch::~Epoch(void){
#if (defined DEBUG)
    if(myState!=State::WaitUnspawn)
      printf("Deleting %ld:%d numInstr %lld\n",myClock,getTid(),pendInstrCount);
#endif
    I((myState==State::WaitUnspawn)||blockRemovalEnabled);
    I(pendInstrCount||(myState==State::WaitUnspawn));
    I(execInstrCount==pendInstrCount);
    I(doneInstrCount==pendInstrCount);
    I(myState!=State::Running);
    I((myState>=State::LazyMerge)||(!myCheckpoint));
    if(myState>=State::LazyMerge){
      I(myCheckpoint);
      myCheckpoint->mergeDone(this);
    }
    // Remove epoch from the allEpochs list, if it's still there
    I(myState!=State::GlobalSafe);
    if(myAllEpochsPos!=allEpochs.end()){
      if(myAllEpochsPos==globalSafe)
	globalSafe++;
      allEpochs.erase(myAllEpochsPos);
    }else{
      // If already removed, should be Unspawn
      I(myState==State::WaitUnspawn);
    }
    // Remove epoch from its thread
    //, delete the thread
    // if this was the last epoch
    bool lastInThread=myThread->removeEpoch(this);
    // Remove pid-to-epoch mapping
    I(pidToEpoch[myPid]==this);
    pidToEpoch[myPid]=0;
    // The SESC thread for this epoch ends now
    osSim->eventExit(myPid,0);
    // Delete the entire thread if this was the last epoch
    if(lastInThread){
      delete myThread;
      numEndThread++;
    }
    VClock::freeVClock(myVClock);
    VClock::freeVClock(originalVClock);
    ID(myPid=-1);
  }
  
  Epoch::PidToEpoch Epoch::pidToEpoch;
  EpochList Epoch::allEpochs;

  Epoch::InstRaces Epoch::allInstRaces;

  bool Epoch::globalSafeAvailable=true;
  EpochList::iterator Epoch::globalSafe(allEpochs.end());

  Epoch::BlockVersions::BlockAddrToVersions Epoch::BlockVersions::blockAddrToVersions;

  void Epoch::BlockVersions::check(Address baseAddr){
    BlockAddrToVersions::iterator addrIt=blockAddrToVersions.find(baseAddr);
    I(addrIt!=blockAddrToVersions.end());
    I(addrIt->first==baseAddr);
    I(addrIt->second==this);
    BufferBlockList::iterator acIt=accessors.begin();
    BufferBlockList::iterator wrIt=writers.begin();
    ClockValue lastClock=largeClockValue;
    while(acIt!=accessors.end()){
      I((*acIt)->isAccessed());
      I((*acIt)->epoch->getClock()<=lastClock);
      I((*acIt)->myVersions==this);
      I((*acIt)->accessorsPos==acIt);
      I((*acIt)->writersPos==wrIt);
      I((*acIt)->baseAddr==baseAddr);
      I((*acIt)->epoch->bufferBlockMap.find(baseAddr)!=(*acIt)->epoch->bufferBlockMap.end());
      I((*acIt)->epoch->bufferBlockMap.find(baseAddr)->second==(*acIt));
      lastClock=(*acIt)->epoch->getClock();
      I((*acIt)->isWritten()==((wrIt!=writers.end())&&(*wrIt==*acIt)));
      if((*acIt)->isWritten())
	wrIt++;
      acIt++;
    }
    I(wrIt==writers.end());
    I(acIt==accessors.end());
  }
  
  void Epoch::BlockVersions::checkAll(void){
    for(BlockAddrToVersions::iterator addrIt=blockAddrToVersions.begin();
        addrIt!=blockAddrToVersions.end();addrIt++){
      Address baseAddr=addrIt->first;
      BlockVersions *blockVers=addrIt->second;
      blockVers->check(baseAddr);
    }
  }

  Epoch::BlockVersions::BlockPosition
  Epoch::BlockVersions::findBlockPosition(const BufferBlock *block){
    I(block->myVersions==this);
    //    ID(check(block->baseAddr));
    BlockPosition retVal=block->accessorsPos;
    // No need to search if block already correctly positioned
    if(retVal==accessors.end()){
      // Search starts from most recent writers
      retVal=accessors.begin();      
      // Use writer positions to jump forward while we can
      for(BufferBlockList::iterator wrPos=writers.begin();
	  wrPos!=writers.end();wrPos++){
	if((*wrPos)->epoch->myClock<=block->epoch->myClock)
	  break;
	retVal=(*wrPos)->accessorsPos;
      }
      // No more writer positions can be used, continue
      // search using accessors list only
      while(retVal!=accessors.end()){
	if((*retVal)->epoch->myClock<=block->epoch->myClock)
	  break;
	retVal++;
      }
    }
    return retVal;
  }
  
  Epoch::ChunkBitMask
  Epoch::BlockVersions::findReadConflicts(const BufferBlock *currBlock, size_t chunkIndx,
					  BlockPosition blockPos,
					  ChunkBitMask beforeMask, ChunkBitMask afterMask,
					  ConflictList &writesBefore, ConflictList &writesAfter){
    BufferBlockList::iterator wrPos=
      (blockPos==accessors.end())?writers.end():(*blockPos)->writersPos;
    if(afterMask){
      BufferBlockList::iterator afterIt=wrPos;
      while(afterIt!=writers.begin()){
	afterIt--;
	BufferBlock *confBlock=*afterIt;
	ChunkBitMask confMask=confBlock->wrMask[chunkIndx]&afterMask;
	if(confMask){
	  writesAfter.push_front(ConflictInfo(confBlock,confMask));
	  afterMask^=confMask;
	  if(!afterMask)
	    break;
	}
      }
    }
    ChunkBitMask memReadMask=beforeMask;
    if(beforeMask){
      BufferBlockList::iterator beforeIt=wrPos;
      if((beforeIt!=writers.end())&&(*beforeIt==currBlock))
	beforeIt++;
      while(beforeIt!=writers.end()){
	BufferBlock *confBlock=*beforeIt;
	ChunkBitMask confMask=confBlock->wrMask[chunkIndx]&beforeMask;
	if(confMask){
	  writesBefore.push_front(ConflictInfo(confBlock,confMask));
	  beforeMask^=confMask;
          // Merged blocks must be read from memory, not the block
          // because in-between blocks could be missing
          if(!confBlock->isMerged())
            memReadMask^=confMask;
	  if(!beforeMask)
	    break;
	}
	beforeIt++;
      }
    }
    return memReadMask;
  }

  void Epoch::BlockVersions::findWriteConflicts(const BufferBlock *currBlock, size_t chunkIndx,
						BlockPosition blockPos,
						ChunkBitMask beforeMask, ChunkBitMask afterMask,
						ConflictList &readsBefore, ConflictList &writesBefore,
						ConflictList &writesAfter, ConflictList &readsAfter){
    if(afterMask){
      BufferBlockList::iterator afterIt=blockPos;
      I(blockPos!=accessors.end());
      while(afterIt!=accessors.begin()){
	afterIt--;
	BufferBlock *confBlock=*afterIt;
	ChunkBitMask rdConfMask=confBlock->xpMask[chunkIndx]&afterMask;
	if(rdConfMask){
	  readsAfter.push_front(ConflictInfo(confBlock,rdConfMask));
	}
	ChunkBitMask wrConfMask=confBlock->wrMask[chunkIndx]&afterMask;
	if(wrConfMask){
	  writesAfter.push_front(ConflictInfo(confBlock,wrConfMask));
	  afterMask^=wrConfMask;
	  if(!afterMask)
	    break;
	}
      }
    }
    if(beforeMask){
      BufferBlockList::iterator beforeIt=blockPos;
      I((blockPos!=accessors.end())&&(*blockPos==currBlock));
      beforeIt++;
      while(beforeIt!=accessors.end()){
	BufferBlock *confBlock=*beforeIt;
	ChunkBitMask wrConfMask=confBlock->wrMask[chunkIndx]&beforeMask;
	if(wrConfMask){
	  writesBefore.push_front(ConflictInfo(confBlock,wrConfMask));
	  beforeMask^=wrConfMask;
	  if(!beforeMask)
	    break;
	}
	ChunkBitMask rdConfMask=confBlock->xpMask[chunkIndx]&beforeMask;
	if(rdConfMask){
	  readsBefore.push_front(ConflictInfo(confBlock,rdConfMask));
	}
	beforeIt++;
      }
    }
  }
  
  void Epoch::BlockVersions::advance(BufferBlock *block){
    I(block->isAccessed()==(block->accessorsPos!=accessors.end()));
    if(block->isAccessed()){
      BufferBlockList::iterator currAcPos=block->accessorsPos;
      BufferBlockList::iterator currWrPos=block->writersPos;
      BufferBlockList::iterator newAcPos=block->accessorsPos;
      BufferBlockList::iterator newWrPos=block->writersPos;
      // This is a write block. Because read blocks point to the
      // first predecessor writer block, when we advance this
      // block we must adjust the write position of read blocks
      // we pass. We do this until we either reach our new position
      // or we pass a write block. Once we pass a write block we
      // continue the advance (if needed) just like a read block
      if(block->isWritten()){
	BufferBlockList::iterator predWrPos=currWrPos; predWrPos++;
	while(newAcPos!=accessors.begin()){
	  newAcPos--;
	  if(((*newAcPos)->epoch->myClock>block->epoch->myClock)||
	     ((*newAcPos)->writersPos!=currWrPos)){
	    newAcPos++;
	    break;
	  }
	  (*newAcPos)->writersPos=predWrPos;
	}
      }
      // Advance faster using the writers list until we hit the
      // write that will become our new immediate successor
      while(newWrPos!=writers.begin()){
	newWrPos--;
	if((*newWrPos)->epoch->myClock>block->epoch->myClock){
	  newWrPos++;
	  break;
	}
	newAcPos=(*newWrPos)->accessorsPos;
      }
      // Now advance the rest of the way using the accessors list
      while(newAcPos!=accessors.begin()){
	newAcPos--;
	if((*newAcPos)->epoch->myClock>block->epoch->myClock){
	  newAcPos++;
	  break;
	}
      }
      // Insert the block into its new accessors position
      accessors.splice(newAcPos,accessors,currAcPos);
      I(*currAcPos==block);
      // If write block, insert into its new writers position and
      // adjust the write pointers of successor read blocks
      // If not a write block, adjust its own write pointer
      if(block->isWritten()){
	writers.splice(newWrPos,writers,currWrPos);
	I(*currWrPos==block);
	BufferBlockList::iterator succAcPos=currAcPos;
	while(succAcPos!=accessors.begin()){
	  succAcPos--;
	  if((*succAcPos)->writersPos!=newWrPos)
	    break;
	  (*succAcPos)->writersPos=currWrPos;
	}
      }else{
	block->writersPos=newWrPos;
      }
    }
    //    ID(check(block->baseAddr));
  }

  void Epoch::BlockVersions::access(bool isWrite, BufferBlock *currBlock, BlockPosition &blockPos){
    I(currBlock->myVersions==this);
    //    ID(check(currBlock->baseAddr));
    if(!currBlock->isAccessed()){
      I(currBlock->accessorsPos==accessors.end());
      I(currBlock->writersPos==writers.end());
      accessorsCount++;
      ThreadID currTid=currBlock->epoch->getTid();
      I(currTid>=0);
      if(threadAccessorsCount.size()<=(size_t)currTid)
        threadAccessorsCount.resize(currTid+1,0);
      bool hasOldVersion=(threadAccessorsCount[currTid]!=0);
      threadAccessorsCount[currTid]++;
      // If too many versions of this block in our thread, try to merge some
      if(Epoch::limitThreadBlockVersions&&
         (threadAccessorsCount[currTid]>Epoch::limitThreadBlockVersions)){
	I(!blockRemovalEnabled);
	size_t requestCount=threadAccessorsCount[currTid]-Epoch::limitThreadBlockVersions;
	// Request removal of enough versions to bring the count down
	for(BufferBlockList::reverse_iterator reqItR=accessors.rbegin();requestCount;reqItR++){
	  I(reqItR!=accessors.rend());
	  if((*reqItR)->epoch->getTid()==currTid){
	    I((*reqItR)!=currBlock);
	    (*reqItR)->epoch->requestBlockRemoval(currBlock->baseAddr);
	    requestCount--;
	  }
	}
      }
      // If not the first version in this thread, increment old version count
      // That call will try to merge old versions if there are too many
      if(hasOldVersion)
	currBlock->epoch->getThread()->incOldVersionsCount();
      // Increment the all-version count, and merge them if there are too many
      currBlock->epoch->getThread()->incAllVersionsCount();
      if(accessorsCount>maxAllBlockVersions)
	maxAllBlockVersions=accessorsCount;
      if(threadAccessorsCount[currTid]>maxThreadBlockVersions)
	maxThreadBlockVersions=threadAccessorsCount[currTid];
      if(blockPos==accessors.end()){
	currBlock->writersPos=writers.end();
      }else{
	currBlock->writersPos=(*blockPos)->writersPos;
      }
      blockPos=currBlock->accessorsPos=accessors.insert(blockPos,currBlock);
    }else{
      I(*(currBlock->accessorsPos)==currBlock);
      I(blockPos==currBlock->accessorsPos);
      I(!currBlock->isWritten()||(*(currBlock->writersPos)==currBlock));
    }
    if(isWrite){
      if(!currBlock->isWritten()){
	I((currBlock->writersPos==writers.end())||(*(currBlock->writersPos)!=currBlock));
	// Mark the block as written
	currBlock->becomeProducer();
	currBlock->writersPos=writers.insert(currBlock->writersPos,currBlock);
	BufferBlockList::iterator lastSuccAcPos;
	if(currBlock->writersPos==writers.begin()){
	  lastSuccAcPos=accessors.begin();
	}else{
	  lastSuccAcPos=currBlock->writersPos; lastSuccAcPos--;
	  lastSuccAcPos=(*lastSuccAcPos)->accessorsPos; lastSuccAcPos++;
	}
	for(BufferBlockList::iterator succAcPos=lastSuccAcPos;
	    succAcPos!=currBlock->accessorsPos;succAcPos++)
	  (*succAcPos)->writersPos=currBlock->writersPos;
      }
    }else{
      // Mark the block as exposed-read
      currBlock->becomeConsumer();
    }
    //    ID(check(currBlock->baseAddr));
  }
  
  void Epoch::BlockVersions::remove(BufferBlock *block){
    I(block->myVersions==this);
//    ID(check(block->baseAddr));
    if(block->isWritten()){
      I(*(block->writersPos)==block);
      if(block->accessorsPos!=accessors.begin()){
	BufferBlockList::iterator lastSuccAcPos;
	if(block->writersPos==writers.begin()){
	  lastSuccAcPos=accessors.begin();
	}else{
	  lastSuccAcPos=block->writersPos; lastSuccAcPos--;
	  lastSuccAcPos=(*lastSuccAcPos)->accessorsPos; lastSuccAcPos++;
	}
	BufferBlockList::iterator predWrPos=block->writersPos; predWrPos++;
	for(BufferBlockList::iterator succAcPos=lastSuccAcPos;
	    succAcPos!=block->accessorsPos;succAcPos++)
	  (*succAcPos)->writersPos=predWrPos;
      }
      writers.erase(block->writersPos);
      ID(block->writersPos=writers.end());
    }else{
      I((block->writersPos==writers.end())||(*(block->writersPos)!=block));
    }
    if(block->isAccessed()){
      I(block->accessorsPos!=accessors.end());
      I(*(block->accessorsPos)==block);
      accessors.erase(block->accessorsPos);
      accessorsCount--;
      threadAccessorsCount[block->epoch->getTid()]--;
      if(threadAccessorsCount[block->epoch->getTid()]!=0)
	block->epoch->getThread()->decOldVersionsCount();
      block->epoch->getThread()->decAllVersionsCount();
      ID(block->accessorsPos=accessors.end());
    }else{
      I(block->accessorsPos==accessors.end());
      I(block->writersPos==writers.end());
    }
    //    ID(check(block->baseAddr));
  }

  std::pair<size_t,size_t> Epoch::BlockVersions::getAgeInThread(BufferBlock *block){
    size_t vers=0;
    size_t blks=0;
    BufferBlockList::iterator myIt=block->accessorsPos;
    Thread *myThread=block->epoch->getThread();
    for(BufferBlockList::iterator it=accessors.begin();it!=myIt;it++){
      I(it!=accessors.end());
      if((*it)->epoch->getThread()==myThread){
	vers++;
	blks+=(*it)->epoch->bufferBlockMap.size();
      }
    }
    return std::pair<size_t,size_t>(vers,blks);
  }
  
  size_t Epoch::BufferBlock::blockCount=0;

#if ((E_RIGHT != 0x8) || (E_LEFT !=0x4) || (E_SIZE != 0x3))
#error "OpFlags does not have the proper structure!"
#endif

  // accessBitMask[OpFlags][Offset]
  //   Where OpFlags is 0..16 and Offset is 0..7
  //   The OpFlags represents for bits, from MSB (bit 3) to LSB (bit 0):
  //     E_RIGHT, E_LEFT, and 2 bits for E_SIZE
  //   The bits of BitMask represent which bytes the memory operation accesses
  //     (MSB is lowest byte, LSB is highest byte)
  const Epoch::ChunkBitMask Epoch::accessBitMask[16][chunkSize]={
    // E_RIGHT is 0, E_LEFT is 0, E_SIZE is 0..3
    {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01},
    {0xC0,0x00,0x30,0x00,0x0C,0x00,0x03,0x00},
    {0xF0,0x00,0x00,0x00,0x0F,0x00,0x00,0x00},
    {0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // E_RIGHT is 0, E_LEFT is 1, E_SIZE should be 0
    {0xF0,0x70,0x30,0x10,0x0F,0x07,0x03,0x01},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // E_RIGHT is 1, E_LEFT is 0, E_SIZE should be 0
    {0x80,0xC0,0xE0,0xF0,0x08,0x0C,0x0E,0x0F},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // E_RIGHT is 1, E_LEFT is 1, this should not happen
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
  };


  void Epoch::appendBuffer(const Epoch *succEpoch){
    I(myState>=State::GlobalSafe);
    I(myState==State::Committed);
    for(BufferBlockList::const_iterator succBlockIt=succEpoch->bufferBlockList.begin();
	succBlockIt!=succEpoch->bufferBlockList.end();succBlockIt++){
      const BufferBlock *succBlock=*succBlockIt;
      Address blockBase=succBlock->getBaseAddr();
      // Find my corresponding block, create anew one if needed
      BufferBlockMap::iterator myBlockIt=bufferBlockMap.find(blockBase);
      BufferBlock *myBlock;
      if(myBlockIt==bufferBlockMap.end()){
	myBlock=new BufferBlock(this,blockBase);
	bufferBlockMap.insert(BufferBlockMap::value_type(blockBase,myBlock));
	myBlock->bufferPos=bufferBlockList.insert(bufferBlockList.end(),myBlock);
      }else{
	myBlock=myBlockIt->second;
      }
      // Append successor block to my block
      myBlock->append(succBlock);
    }
  }

  void Epoch::cleanBuffer(void){
    I(myState==State::FullReplay);
    for(BufferBlockList::iterator blockIt=bufferBlockList.begin();
	blockIt!=bufferBlockList.end();blockIt++)
      (*blockIt)->clean();
  }

  void Epoch::eraseBlock(Address baseAddr){
    BufferBlock *block=bufferBlockMap[baseAddr];
    I(block);
    I(block->epoch==this);
    myBlockRemovalRequests.erase(baseAddr);
    bufferBlockList.erase(block->bufferPos);
    bufferBlockMap.erase(baseAddr);
    delete block;
    //BACKEND:If this was called by backend remove it from list

//   	printf("myBlockRemovalRequests count after erase %u\n",myBlockRemovalRequests.size());
    int32_t blkCount = Epoch::pendWb.count(baseAddr);
    if (blkCount>0)
    {
    	printf("Clear address %ul\n",baseAddr);
    	multimap<Address,CallbackBase *>::iterator iter;
     	multimap<Address,CallbackBase *>::iterator lower;
    	multimap<Address,CallbackBase *>::iterator upper;
    	lower=Epoch::pendWb.lower_bound(baseAddr);
    	upper=Epoch::pendWb.upper_bound(baseAddr);
	    for (iter=lower;iter!=upper;iter++)
	    {
	    	iter->second->call();
	    }
	    Epoch::pendWb.erase(baseAddr);
    }
    
    // If this was the last block in a committed epoch,
    // the epoch itself should also be destroyed
    if(bufferBlockList.empty()){
      if(myState==State::Committed){
	I(myState==State::NoWait);
	//Prevent block deletion, as backend pointers may still refer to the block
	//delete this;
      }else if(myState==State::WaitFullyMerged){
	becomeFullyMerged();
      }
    }
  }

  void Epoch::mergeBlock(Address baseAddr){
    I(myState>=State::LazyMerge);
    BufferBlockMap::iterator blockIt=bufferBlockMap.find(baseAddr);
    //I(blockIt!=bufferBlockMap.end());
    if(blockIt==bufferBlockMap.end())
    	I(blockIt!=bufferBlockMap.end());
    I(blockIt->first==baseAddr);
    BufferBlock *block=blockIt->second;
    // If block already merged, do nothing
    if(block->isMerged())
      return;
    I(block&&(block->epoch==this));
    // If predecessor blocks exist, they must merge as well
    BlockVersions *versions=block->getVersions();
    BufferBlockList::iterator mergIt=block->accessorsPos;
    while((mergIt!=versions->accessors.end())&&(!(*mergIt)->isMerged()))
      mergIt++;
    while(mergIt!=block->accessorsPos){
      mergIt--;
      BufferBlock *mergBlock=*mergIt;
      Epoch *mergEpoch=mergBlock->epoch;
      if(mergEpoch->myState<State::LazyMerge){
        I((mergEpoch!=this)&&(mergEpoch->myClock==myClock));
        mergIt++;
        versions->advance(mergBlock);
        ID(mergIt--);
        I(mergBlock!=*mergIt);
        ID(mergIt++);         
        continue;
      }
      I(mergEpoch->myCheckpoint==Checkpoint::getCheckpoint(mergEpoch->myClock));
      if(mergBlock->isProducer){
        mergEpoch->myCheckpoint->write(baseAddr);
        memVClock.succeed(mergEpoch->myVClock);
        memLClock.advance(mergEpoch->myLClock);
      }
      mergBlock->merge();
    }
    I(block->isMerged());
  }

  Epoch::EpochSet Epoch::pendingBlockRemovals;
  bool Epoch::blockRemovalEnabled=true;

  bool Epoch::requestBlockRemoval(Address blockAddr){
    if(bufferBlockMap.find(blockAddr)==bufferBlockMap.end())
    	return true;
   //I(bufferBlockMap.find(blockAddr)!=bufferBlockMap.end());
    I(myState!=State::ReqEagerMerge);
    if(blockRemovalEnabled){
      requestLazyMerge();
      if(myState>=State::LazyMerge){
        mergeBlock(blockAddr);
        eraseBlock(blockAddr);
        return true;
      }
    }
    myBlockRemovalRequests.insert(blockAddr);
    pendingBlockRemovals.insert(this);
    return false;
  }
  bool Epoch::requestBlockRemovalWB(Address blockAddr, CallbackBase *wcb){
    if(bufferBlockMap.find(blockAddr)==bufferBlockMap.end())
    	return true;
   //I(bufferBlockMap.find(blockAddr)!=bufferBlockMap.end());
    I(myState!=State::ReqEagerMerge);
    if(blockRemovalEnabled){
      requestLazyMerge();
      if(myState>=State::LazyMerge){
        mergeBlock(blockAddr);
        eraseBlock(blockAddr);
        return true;
      }
    }
    //BACKEND:Add to que to sending up request later
    //I(pendWb.find(mreq) == pendWb.end());

  	std::pair <Address,CallbackBase *> p1 (blockAddr,wcb);
  	//pendWb[blockAddr]=wcb;
    pendWb.insert(p1);
    //Add request only once
    if (myBlockRemovalRequests.find(blockAddr)==myBlockRemovalRequests.end())
    {
    	myBlockRemovalRequests.insert(blockAddr);
    	pendingBlockRemovals.insert(this);
    	//printf("myBlockRemovalRequests count %u\n",myBlockRemovalRequests.size());
    }
    return false;
  }
  
  
  size_t Epoch::requestAnyBlockRemoval(size_t count){
    if(bufferBlockList.empty())
      return count;
    BufferBlockList::iterator blockIt=bufferBlockList.begin();
    BufferBlockList::iterator blockItNext=blockIt;
    blockItNext++;
    while(count&&(blockItNext!=bufferBlockList.end())){
      requestBlockRemoval((*blockIt)->getBaseAddr());
      count--;
      blockIt=blockItNext;
      blockItNext++;
    }
    if(count){
      requestBlockRemoval((*blockIt)->getBaseAddr());
      count--;      
    }
    return count;
  }
  size_t Epoch::requestOldBlockRemoval(size_t count){
    return 0;
  }
  void Epoch::enableBlockRemoval(void){
    I(!blockRemovalEnabled);
    blockRemovalEnabled=true;
    // If no epochs with pending removals, we're done
    if(pendingBlockRemovals.empty())
      return;
    // Copy the elements of the set into a list because the epoch is removed
    // from the set on doBlockremovals and that invalidates set iterators
    EpochList pendList(pendingBlockRemovals.begin(),
		       pendingBlockRemovals.end());
    for(EpochList::iterator epochIt=pendList.begin();
	epochIt!=pendList.end();epochIt++){
      Epoch *epoch=*epochIt;
      // Ignore epochs that are already removed from the set
      if(pendingBlockRemovals.find(epoch)==pendingBlockRemovals.end())
        continue;
      if(epoch->myState>=State::LazyMerge){
	epoch->doBlockRemovals();
      }else{
	epoch->requestLazyMerge();
      }
    }
  }
  void Epoch::doBlockRemovals(void){
    I(blockRemovalEnabled);
    I(myState>=State::LazyMerge);
    ID(Pid_t thePid=myPid);
    pendingBlockRemovals.erase(this);
    if(bufferBlockList.empty()){
    //Prevent block deletion, as backend pointers may still refer to the block
       if(myState==State::Committed);
	//delete this;
      else if(myState==State::WaitFullyMerged)
	becomeFullyMerged();
    }else if(myState>=State::EagerMerge){
      mergeBuffer();
      eraseBuffer();
    }else{
      while(!myBlockRemovalRequests.empty()){
	Address blockAddr=*myBlockRemovalRequests.begin();
	mergeBlock(blockAddr);
	eraseBlock(blockAddr);
      }
    }
    I((!pidToEpoch[thePid])||(myBlockRemovalRequests.empty()&&(myState>=State::LazyMerge)));
  }

  void Epoch::eraseBuffer(void){
    if(bufferBlockList.empty()){
      //I(myState<State::Committed);
      return;
    }
    // Deletion of the last block can delete the epoch,
    // so we peel that last deletion off the loop
    BufferBlockList::iterator blockIt=bufferBlockList.begin();
    BufferBlockList::iterator blockItNext=blockIt;
    blockItNext++;
    while(blockItNext!=bufferBlockList.end()){
      I((*blockIt)->isAccessed());
      eraseBlock((*blockIt)->getBaseAddr());
      blockIt=blockItNext;
      blockItNext++;
    }
    I((*blockIt)->isAccessed());
    eraseBlock((*blockIt)->getBaseAddr());
  }

  void Epoch::mergeBuffer(void){
    I(myState==State::EagerMerge);
    for(BufferBlockList::iterator blockIt=bufferBlockList.begin();
	blockIt!=bufferBlockList.end();blockIt++){
      I((*blockIt)->isAccessed());
      mergeBlock((*blockIt)->getBaseAddr());
    }
  }

  RAddr Epoch::read(VAddr iVAddr, short iFlags,
		    VAddr dVAddr, Address dAddrR){
//#if (defined DEBUG)
//    {
//      EpochList::iterator listIt=allEpochs.begin();
//      EpochList::iterator nextListIt=listIt;
//      nextListIt++;
//      while(nextListIt!=allEpochs.end()){
//        I((*listIt)->myClock>=(*nextListIt)->myClock);
//        I(!VClock::isOrder((*listIt)->myVClock,(*nextListIt)->myVClock));
//        listIt=nextListIt;
//        nextListIt++;
//      }
//    }
//#endif
    I(iFlags&E_READ);
    I(myState!=State::Completed);
    if(!dAddrR)
      return 0;
    if((myState==State::FullReplay)&&traceDataAddresses.count(dVAddr)){
      traceCodeAddresses.insert(iVAddr);
      // Add access to the trace
      if(myTrace.size()>16){
	TraceAccessEvent *prevEvent=0;
	if(!myTrace.empty())
	  prevEvent=dynamic_cast<TraceAccessEvent *>(myTrace.back());
	TraceAccessEvent *myEvent;
	if(prevEvent){
	  myEvent=prevEvent->newAccess(this,dVAddr,TraceAccessEvent::Read);
	}else{
	  myEvent=new TraceAccessEvent(this,dVAddr,TraceAccessEvent::Read);
	  I(iVAddr==(VAddr)(osSim->eventGetInstructionPointer(myPid)->addr));
	}
	if(myEvent){
	  myTrace.push_back(myEvent);
	}
      }
      // Update the forward and backward race info of this access
      RaceByAddrEp::iterator forwRaceByAddrEpIt=
	myForwRacesByAddrEp.find(dVAddr);
      if(forwRaceByAddrEpIt!=myForwRacesByAddrEp.end())
	forwRaceByAddrEpIt->second->addReadAccess(this,dVAddr);
      RaceByAddrEp::iterator backRaceByAddrEpIt=
	myBackRacesByAddrEp.find(dVAddr);
      if(backRaceByAddrEpIt!=myBackRacesByAddrEp.end())
	backRaceByAddrEpIt->second->addReadAccess(this,dVAddr);
      I((forwRaceByAddrEpIt!=myForwRacesByAddrEp.end())||
	(backRaceByAddrEpIt!=myBackRacesByAddrEp.end()));
    }
    // Find block base address and offset for this access 
    size_t blockOffs=dAddrR&blockAddrMask;
    Address blockBase=dAddrR-blockOffs;
    // Find the buffer block to access
    BufferBlock *bufferBlock=getBufferBlock(blockBase);
    // No block => no access
    if(!bufferBlock)
      return 0;
    I(bufferBlock->baseAddr==blockBase);
    // Index of the chunk within the block and the offset within the chunk
    size_t chunkIndx=blockOffs>>logChunkSize;
    size_t chunkOffs=dAddrR&chunkAddrMask;
    // Access mask contains the bytes read by this access
    ChunkBitMask accMask=accessBitMask[iFlags&(E_RIGHT|E_LEFT|E_SIZE)][chunkOffs];
    // Back mask contains bytes we need to find from predecessors because we don't already have them
    ChunkBitMask backMask=accMask&(~((bufferBlock->wrMask[chunkIndx])|(bufferBlock->xpMask[chunkIndx])));
    // Forward mask contains bytes that may be obsolete.
    // If the data in the block is stale, everything may be obsolete
    // If the data already in the block is not stale, only newly accessed bytes may be obsolete
    ChunkBitMask forwMask=bufferBlock->isStale()?accMask:backMask;
    // If back or forward mask is nonempty, other versions must be looked at
    if(backMask||forwMask){
      BlockVersions *versions=bufferBlock->getVersions();
      // We'll attempt to advance past all successor writers
      // However, we can't always do that
      bool noSuccessorsLeft=false;
      // Must sync advance if in a sync operation
      bool syncAdvance=(myState==State::Atm);
      bool easyAdvance=canEasilyAdvance();
      // We don't truncate if we can easily advance
      bool canTruncate=(!easyAdvance)&&canBeTruncated();
      // True iff clock adjustment of the reader can be done without squashes
      bool simpleAdjust=easyAdvance||canTruncate;
      // It is only possible to adjust the logical clock if this not a full replay
      if(myState<State::FullReplay){
        // We'll try to advance past all successors, but we won't
        // do it if squashes are needed to advance this epoch
        noSuccessorsLeft=true;
	// Go through all written versions, starting with the most recent one
	for(BufferBlockList::iterator wrPos=versions->writers.begin();
	    wrPos!=versions->writers.end();wrPos++){
	  // Skip versions with non-overlapping accesses
	  if(!((*wrPos)->wrMask[chunkIndx]&forwMask))
	    continue;
	  // Skip versions from the same thread
	  if(myThread==(*wrPos)->epoch->myThread){
	    // Same-thread dependences should already be properly ordered
	    I(((*wrPos)->epoch==this)||(myClock>(*wrPos)->epoch->myClock));
	    continue;
	  }
	  // Current clocks of conflicting read and write can be:
	  // 1) Equal, in which case we must advance one of them, so we
	  //    advance the read because that avoids weird livelock problems
	  // 2) The read's clock is lower than the write's, in which case we advance
	  //    the read's clock if possible without squashes. If squashes are needed
	  //    to advance the read, we enforce the clocks to avoid livelock problems
	  // 3) The read's clock is greater than the write's, in which case
	  //    there is no need to do anything, except when the read is an acquire
	  //    and the clock difference is too small. In that case, we advance the
	  //    read to have the proper difference
	  // We check whether we have case 2) and what to do about it
	  if((!simpleAdjust)&&((*wrPos)->epoch->myClock>myClock)){
	    // We didn't advance past this successor, so we'll have to handle this later
	    noSuccessorsLeft=false;
	    continue;
	  }
	  // We check whether we have case 3) and what to do about it
	  // Note: with syncAdvance, we must look at all predecessors because
	  // (1) we must advance myClock to synchronize with preceding, but not
	  // yet synchronizaed successors. If this was the only condition, lowClock
	  // would be myClock-getSyncDelta(), but we must also ensure that
	  // (2) we must advance myCheckClock beyond any myClock predecessor not yet
	  // check-synchronized with myCheckClock, so we search through ALL predecessors
	  // Not that this second requirement is only there becasue adjustClock is
	  // also adjusting the checkClock... instead, checkClock can be handled later
	  if((!syncAdvance)&&((*wrPos)->epoch->myClock<myClock))
	    break;

	  // If we got here we have an actual read-write conflict between accesses
          // from different threads. We will adjust the clocks by advancing or truncating
	  ClockValue newClock=(*wrPos)->epoch->myClock+(syncAdvance?(getSyncClockDelta()+1):1);
          // If anomalies are found, they are between writers and this epoch, unless
          // we are about to truncate it, in which case the read is in the new epoch
	  Epoch *currEpoch=canTruncate?changeEpoch():this;
	  I(currEpoch);
	  if(!syncAdvance){
	    I(newClock>=myClock);
	    // Go thorough all conflicting writes not synchronized with this read, mark them as anomalies
	    for(BufferBlockList::iterator wrPosRace=wrPos;
		wrPosRace!=versions->writers.end();wrPosRace++){
	      // Only look through writers that are known to be not synchronized
	      if((*wrPosRace)->epoch->myClock<myClock-getSyncClockDelta())
		break;
	      // Skip blocks with non-overlapping accesses
	      if(!((*wrPosRace)->wrMask[chunkIndx]&forwMask))
		continue;
	      // Skip blocks from the same thread
	      if(myThread==(*wrPosRace)->epoch->myThread)
		continue;
	      I(newClock>(*wrPosRace)->epoch->myClock);
	      currEpoch->myInstRaces.addAnom(iVAddr,versions->getAgeInThread(*wrPosRace));
	    }
	  }
	  // If truncated, remove the allocated buffer block and advance the new epoch
	  if(currEpoch!=this){
	    I(canTruncate);
	    if(!bufferBlock->isAccessed())
	      eraseBlock(blockBase);
	    I((myState!=State::EagerMerge)||(bufferBlockList.empty()));
	    currEpoch->advanceClock((*wrPos)->epoch,syncAdvance);
	    return 0;
	  }
          // We will advance the clock of the current epoch
	  I(!canTruncate);
#if (defined DEBUG)
	  {
	    EpochList::iterator listIt=allEpochs.begin();
	    EpochList::iterator nextListIt=listIt;
	    nextListIt++;
	    while(nextListIt!=allEpochs.end()){
	      I((*listIt)->myClock>=(*nextListIt)->myClock);
	      I(!VClock::isOrder((*listIt)->myVClock,(*nextListIt)->myVClock));
	      listIt=nextListIt;
	      nextListIt++;
	    }
	  }
#endif
	  I(!VClock::isOrder(myVClock,(*wrPos)->epoch->myVClock));
	  advanceClock((*wrPos)->epoch,syncAdvance);
	  // The writer will have a flow successor now, and we must change
	  // the status now because the versions->access() call later may
	  // otherwise advance the writer in order to merge a buffer block
	  (*wrPos)->epoch->myState+=State::FlowSuccessors;
#if (defined DEBUG)
	  {
	    EpochList::iterator listIt=allEpochs.begin();
	    EpochList::iterator nextListIt=listIt;
	    nextListIt++;
	    while(nextListIt!=allEpochs.end()){
	      I((*listIt)->myClock>=(*nextListIt)->myClock);
	      I(!VClock::isOrder((*listIt)->myVClock,(*nextListIt)->myVClock));
	      listIt=nextListIt;
	      nextListIt++;
	    }
	  }
#endif
	  // First conflict determines the new clock, no need to look for more
	  break;
	}
	// If no successor writers are left, there are no forward conflicts remaining
	if(noSuccessorsLeft){
	  // But for debugging we leave the old mask to check
	  ID(ChunkBitMask oldForwMask=forwMask);
	  forwMask=0;
	  ID(forwMask=oldForwMask);
	}
//#if (defined DEBUG)
// 	if(isSync){
// 	  printf("Epochs in writers list for %ld:%d (%x):",myClock,getTid(),accMask);
// 	  BlockVersions::BufferBlockList::iterator wrPos;
// 	  for(wrPos=versions->writers.begin();wrPos!=versions->writers.end();wrPos++){
// 	    printf(" %ld:%d(%x)",
// 		   (*wrPos)->epoch->myClock,
// 		   (*wrPos)->epoch->getTid(),
// 		   ((*wrPos)->wrMask[chunkIndx])&accMask);
// 	  }
// 	  printf("\n");
// 	}
//#endif
      }
      // Now find the position of the block
      BlockVersions::BlockPosition blockPos=versions->findBlockPosition(bufferBlock);
      if(backMask){
	// Perform the read access to the block
	versions->access(false,bufferBlock,blockPos);
      }
      // Now find conflicting accesses for this read
      BlockVersions::ConflictList writesBefore,writesAfter;
      ChunkBitMask memMask=versions->findReadConflicts(bufferBlock,chunkIndx,blockPos,
						       backMask,forwMask,writesBefore,writesAfter);
      I((!noSuccessorsLeft)||writesAfter.empty());
      // If we could have advanced or truncated, we should have avoided forward conflicts
      I(!simpleAdjust||writesAfter.empty());
      // If we need to copy-in, we have to do it from somewhere
      I((!backMask)==((!memMask)&&writesBefore.empty()));
      // Process the writes that precede this read and do the copy-in
      if(backMask){
	// Destination address for data copy-in
	uint8_t *dstDataPtr=
	  (uint8_t *)(bufferBlock->wkData)+blockOffs-chunkOffs;	
	for(BlockVersions::ConflictList::const_iterator wrBeforeIt=writesBefore.begin();
	    wrBeforeIt!=writesBefore.end();wrBeforeIt++){
	  BufferBlock *wrBeforeBlock=wrBeforeIt->block;
	  ChunkBitMask wrBeforeMask=wrBeforeIt->mask;
	  // The predecesor block has sourced data for this flow dependence
	  wrBeforeBlock->becomeFlowSource();
          // If block is not merged, copy-in the bytes we need
          if(!wrBeforeBlock->isMerged()){
	    // Source data for copy-in is in the predecessor's buffer
	    uint8_t *srcDataPtr=
	      (uint8_t *)(wrBeforeBlock->wkData)+blockOffs-chunkOffs;
	    BufferBlock::maskedChunkCopy(srcDataPtr,dstDataPtr,wrBeforeMask);
	    // Add the bytes to exposed read mask
	    bufferBlock->xpMask[chunkIndx]|=wrBeforeMask;
          }
	  Epoch *wrBeforeEpoch=wrBeforeBlock->epoch;
	  // This epoch succeeds the predecessor write in a flow dependence
	  wrBeforeEpoch->myState+=State::FlowSuccessors;
	  // No check for races if writer epoch in the same thread
	  if(myThread==wrBeforeEpoch->myThread)
	    continue;
	  // In any clock scheme, wrBeforeEpoch should not be after "this" epoch
	  I(!VClock::isOrder(myVClock,wrBeforeEpoch->myVClock));
	  if(syncAdvance){
	    if(!VClock::isOrder(wrBeforeEpoch->myVClock,myVClock)){
	      myVClock->succeed(wrBeforeEpoch->myVClock);
	      if(myState>=State::ThreadSafe){
		I(myState!=State::Committed);
		I(!myThread->threadSafeAvailable);
		myThread->changeThreadSafeVClock(myVClock);
	      }
	    }
	    if(!LClock::isOrder((wrBeforeEpoch)->myLClock,myLClock))
	      myLClock.succeed(wrBeforeEpoch->myLClock);
	    continue;
	  }
	  // Data race on in-order raw?
	  if(compareClock(wrBeforeEpoch)!=StrongBefore){
	    // An anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(wrBeforeEpoch->myVClock,myVClock));
	    myInstRaces.addAnom(iVAddr,versions->getAgeInThread(wrBeforeBlock));
	  }
	  if(compareCheckClock(wrBeforeEpoch)!=StrongBefore){
	    // A check-clock anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(wrBeforeEpoch->myVClock,myVClock));
	    myInstRaces.addChka(iVAddr,versions->getAgeInThread(wrBeforeBlock));
	  }
	  // We have a running anomaly only if the predecessor writer is not Atm
	  // Note: We can't also report an anomaly if "this" in not Atm because
	  // our thread could have just synced after the predecessor, which still
	  // hasn't left the Atm section. In that case we are not Atm and both us
	  // and the predecessor are not Completed, but there is no anomaly
	  if((wrBeforeEpoch->myState!=State::Atm)&&
	     (wrBeforeEpoch->myState!=State::Completed)){
	    // A running anomaly should also be a data race (no VClock ordering)
	    // unless this is a case of an injected lacking-synchronization error
	    // I((!VClock::isOrder(wrBeforeEpoch->myVClock,myVClock))||
	    //  wrBeforeEpoch->atmNestLevel);
	    myInstRaces.addRuna(iVAddr,versions->getAgeInThread(wrBeforeBlock));
	  }
	  if(!VClock::isOrder(wrBeforeEpoch->myVClock,myVClock))
	    myInstRaces.addRace(iVAddr,versions->getAgeInThread(wrBeforeBlock));
	  if(!LClock::isOrder((wrBeforeEpoch)->myLClock,myLClock))
	    myInstRaces.addLama(iVAddr,versions->getAgeInThread(wrBeforeBlock));
	}
	if(memMask){
	  // Source data for copy-in is in main memory
	  uint8_t *srcDataPtr=(uint8_t *)(dAddrR-chunkOffs);
	  BufferBlock::maskedChunkCopy(srcDataPtr,dstDataPtr,memMask);
	  // Add the bytes to exposed read mask, unless we are in condition mode
	  bufferBlock->xpMask[chunkIndx]|=memMask;
          if(syncAdvance){
            if(!VClock::isOrder(&memVClock,myVClock)){
              myVClock->succeed(&memVClock);
              if(myState>=State::ThreadSafe){
                I(myState!=State::Committed);
                I(!myThread->threadSafeAvailable);
                myThread->changeThreadSafeVClock(myVClock);
              }
            }
            if(!LClock::isOrder(memLClock,myLClock))
              myLClock.succeed(memLClock);
          }
	}
      }
      // Process the writes that logically succeed this read
      if(!writesAfter.empty()){
	// Data block is stale because overwriters already exist
	bufferBlock->becomeStale();
	// Should not be reading stale data in sync
      //  I(myState==State::NoAcq);
	// Create ordering and detect races, if needed
	for(BlockVersions::ConflictList::const_iterator wrAfterIt=writesAfter.begin();
	    wrAfterIt!=writesAfter.end();wrAfterIt++){
	  Epoch *wrAfterEpoch=wrAfterIt->block->epoch;
	  myState+=State::NameSuccessors;
	  // No check for races if writer epoch in the same thread
	  if(myThread==wrAfterEpoch->myThread)
	    continue;
	  // In any clock scheme, wrAfterEpoch should not be before "this" epoch
	  I(!VClock::isOrder(wrAfterEpoch->myVClock,myVClock));
	  if(compareClock(wrAfterEpoch)!=StrongAfter){
	    // An anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(myVClock,wrAfterEpoch->myVClock));
 	    wrAfterEpoch->myInstRaces.addAnom(iVAddr,versions->getAgeInThread(wrAfterIt->block));
	  }
	  if(compareCheckClock(wrAfterEpoch)!=StrongAfter){
	    // A check-clock anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(myVClock,wrAfterEpoch->myVClock));
 	    wrAfterEpoch->myInstRaces.addChka(iVAddr,versions->getAgeInThread(wrAfterIt->block));
	  }
	  if(((myState!=State::Atm)||(wrAfterEpoch->myState!=State::Atm))
	     &&(wrAfterEpoch->myState!=State::Completed)){
	    // A running anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(myVClock,wrAfterEpoch->myVClock));
 	    wrAfterEpoch->myInstRaces.addRuna(iVAddr,versions->getAgeInThread(wrAfterIt->block));
	  }
	  if(!VClock::isOrder(myVClock,wrAfterEpoch->myVClock)){
	    // Clock adjustment prevents this from happening,
	    // except for atomic sections and replay executions
	 //   I((myState!=State::Initial)||
	 //     ((myState==State::Atm)&&(myState!=State::Acq)&&(myState!=State::Rel)));
 	    wrAfterEpoch->myInstRaces.addRace(iVAddr,versions->getAgeInThread(wrAfterIt->block));
	  }
	  if(!LClock::isOrder(myLClock,wrAfterEpoch->myLClock))
 	    wrAfterEpoch->myInstRaces.addLama(iVAddr,versions->getAgeInThread(wrAfterIt->block));
	}
      }
    }
//#if (defined DEBUG)
//    {
//      EpochList::iterator listIt=allEpochs.begin();
//      EpochList::iterator nextListIt=listIt;
//      nextListIt++;
//      while(nextListIt!=allEpochs.end()){
//        I((*listIt)->myClock>=(*nextListIt)->myClock);
//        I(!VClock::isOrder((*listIt)->myVClock,(*nextListIt)->myVClock));
//        listIt=nextListIt;
//        nextListIt++;
//      }
//    }
//#endif
    // If EagerMerge, we want this block to be removed later
    I(!blockRemovalEnabled);
    if(myState==State::EagerMerge)
      requestBlockRemoval(blockBase);
    return ((Address)(bufferBlock->wkData))+blockOffs;
  }
  
  RAddr Epoch::write(VAddr iVAddr, short iFlags,
		       VAddr dVAddr, Address dAddrR){
    I(iFlags&E_WRITE);
    I(myState!=State::Completed);
    if(!dAddrR)
      return 0;
    if((myState==State::FullReplay)&&traceDataAddresses.count(dVAddr)){
      traceCodeAddresses.insert(iVAddr);
      if(myTrace.size()>16){
	// Add access to the trace
	TraceAccessEvent *myEvent=
	  new TraceAccessEvent(this,dVAddr,TraceAccessEvent::Write);
	I(iVAddr==(VAddr)(osSim->eventGetInstructionPointer(myPid)->addr));
	myTrace.push_back(myEvent);
      }
      // Update the forward and backward race info of this access
      RaceByAddrEp::iterator forwRaceByAddrEpIt=
	myForwRacesByAddrEp.find(dVAddr);
      if(forwRaceByAddrEpIt!=myForwRacesByAddrEp.end())
	forwRaceByAddrEpIt->second->addWriteAccess(this,dVAddr);
      RaceByAddrEp::iterator backRaceByAddrEpIt=
	myBackRacesByAddrEp.find(dVAddr);
      if(backRaceByAddrEpIt!=myBackRacesByAddrEp.end())
	backRaceByAddrEpIt->second->addWriteAccess(this,dVAddr);
      I((forwRaceByAddrEpIt!=myForwRacesByAddrEp.end())||
	(backRaceByAddrEpIt!=myBackRacesByAddrEp.end()));
    }
    // Find block base address and offset for this access 
    size_t blockOffs=dAddrR&blockAddrMask;
    Address blockBase=dAddrR-blockOffs;
    // Find the buffer block to access
    BufferBlock *bufferBlock=getBufferBlock(blockBase);
    // No block => no access
    if(!bufferBlock)
      return 0;
    I(bufferBlock->baseAddr==blockBase);
    // Index of the chunk within the block and the offset within the chunk
    size_t chunkIndx=blockOffs>>logChunkSize;
    size_t chunkOffs=dAddrR&chunkAddrMask;
    // Access mask initially contains the bytes written by this access
    ChunkBitMask accMask=accessBitMask[iFlags&(E_RIGHT|E_LEFT|E_SIZE)][chunkOffs];
    // This write needs to be ordered after reads and writes whose data it overwrites
    // If we have already written to the same location, the order already exists
    ChunkBitMask backMask=accMask&(~(bufferBlock->wrMask[chunkIndx]));
    // Forward mask contains bytes that may be obsolete or prematurely read
    // If the data is stale or a source for a flow dependence, everything must be checkd
    // Othrwise, only newly stored bytes need be checked
    ChunkBitMask forwMask=
      (bufferBlock->isFlowSource()||bufferBlock->isStale())?accMask:(accMask&(~(bufferBlock->wrMask[chunkIndx])));
    // If back or forward mask is nonempty, other versions must be looked at
    if(backMask||forwMask){
      BlockVersions *versions=bufferBlock->getVersions();
      // We'll attempt to advance past all successors
      // However, we can't always do that
      bool noSuccessorsLeft=false;
      bool easyAdvance=canEasilyAdvance();
      // We don't truncate if we can easily advance
      bool canTruncate=(!easyAdvance)&&canBeTruncated();
      // True iff clock adjustment of the reader can be done without squashes
      bool simpleAdjust=easyAdvance||canTruncate;
      // It is only possible to adjust the logical clock if this is the initial run
      // of this epoch, and not a replay (in which case we must enforce ordering)
      if(myState<State::FullReplay){
        // We'll try to advance past all successors, but we won't
        // do it if squashes are needed to advance this epoch
        noSuccessorsLeft=true;
	// Go through all versions, starting with the most recent one
	for(BufferBlockList::iterator acPos=versions->accessors.begin();
	    acPos!=versions->accessors.end();acPos++){
	  // Is this going to avoid a squash (successor's read and this write overlap)?
	  bool isFlow=((*acPos)->xpMask[chunkIndx])&forwMask;
	  // Skip versions with non-overlapping accesses
	  if(!(((*acPos)->wrMask[chunkIndx]|(*acPos)->xpMask[chunkIndx])&forwMask))
	    continue;
	  // Skip versions from the same thread
	  if(myThread==(*acPos)->epoch->myThread){
	    // Same-thread dependences should already be properly ordered
	    I(((*acPos)->epoch==this)||(myClock>(*acPos)->epoch->myClock));
	    continue;
	  }
	  // No need to look at accesses with clocks lower than ours
	  if((*acPos)->epoch->myClock<myClock)
	    break;
	  // If this is a successor, decide whether to advance ahead of it
	  if((*acPos)->epoch->myClock>myClock){
	    // No advance if we can't simply adjust our clock
	    // (This is done to avoid deadlocks)
	    if(!simpleAdjust){
              // We didn't advance past this successor, so we'll have to handle this later
 	      noSuccessorsLeft=false;
	      continue;
	    }
// For some reason this is creating a problem in FFT (and possibly others)
// 	    // No advance if the successor will be squashed by this write
// 	    // because it is waiting to retry a synchronization acquire
// 	    if((((*acPos)->xpMask[chunkIndx])&forwMask)&&
// 	       ((*acPos)->epoch->myState==State::WaitAcqRetry)){
// 	      noSuccessorsLeft=false;
// 	      continue;
// 	    }
	  }
	  // If we got here we have an actual conflict between this write and an access
          // from another thread. We will adjust the write's clock by advancing or truncating
	  ClockValue newClock=(*acPos)->epoch->myClock+1;
	  I(newClock>myClock);
          // If anomalies are found, they are between writers and this epoch, unless
          // we are about to truncate it, in which case the read is in the new epoch
	  Epoch *currEpoch=canTruncate?changeEpoch():this;
	  I(currEpoch);
	  // Go thorough all conflicting accesses not synchronized with this write, mark them as anomalies
	  for(BufferBlockList::iterator acPosRace=acPos;
	      acPosRace!=versions->accessors.end();acPosRace++){
	    // Only look through accesses that are known to be not synchronized
	    if((*acPosRace)->epoch->myClock<myClock-getSyncClockDelta())
	      break;
	    // Skip blocks with non-overlapping accesses
	    if(!(((*acPosRace)->wrMask[chunkIndx]|(*acPosRace)->xpMask[chunkIndx])&forwMask))
	      continue;
	    // Skip blocks from the same thread
	    if(myThread==(*acPosRace)->epoch->myThread)
	      continue;
            // Ignore conflicts between two atomic sections
	    if((myState==State::Atm)&&((*acPosRace)->epoch->myState==State::Atm))
	      continue;
	    I(newClock>(*acPosRace)->epoch->myClock);
	    currEpoch->myInstRaces.addAnom(iVAddr,versions->getAgeInThread(*acPosRace));
	  }
	  // If truncated, remove the allocated buffer block and advance the new epoch
	  if(currEpoch!=this){
	    I(canTruncate);
	    if(!bufferBlock->isAccessed())
	      eraseBlock(blockBase);
	    I((myState!=State::EagerMerge)||(bufferBlockList.empty()));	    
	    currEpoch->advanceClock((*acPos)->epoch,false);
	    return 0;
	  }
	  // We will advance the clock of the current epoch
	  I(!canTruncate);
	  I(!VClock::isOrder(myVClock,(*acPos)->epoch->myVClock));
	  advanceClock((*acPos)->epoch,false);
	  // First conflict determines the new clock, no need to look for more
	  break;
	}
	// If we skipped no successors, there are no forward conflicts left now
	if(noSuccessorsLeft){
	  // But for debugging we leave the old mask to check
	  ID(ChunkBitMask oldForwMask=forwMask);
// When we advance past a waitig successor, advanceClock tries to make it GlobalSafe
// which in turn advance the successor and it may advance past us again... so we still
// need to look for successors even if we think we skipped none when trying to advance
//	  forwMask=0;
          noSuccessorsLeft=false;
	  ID(forwMask=oldForwMask);
	}
      }
      I(bufferBlock->baseAddr==blockBase);
      I(bufferBlock->myVersions==versions);
      // Now find the position of the block
      BlockVersions::BlockPosition blockPos=versions->findBlockPosition(bufferBlock);
      // Perform the write access to the block
      if(backMask)
	versions->access(true,bufferBlock,blockPos);
      I(*(bufferBlock->accessorsPos)==bufferBlock);
      I(*(bufferBlock->writersPos)==bufferBlock);
      I(bufferBlock->isProducer);
      I(!versions->accessors.empty());
      I(!versions->writers.empty());
      I(pendInstrCount);
      // Check for squashes and successor writes
      if(forwMask){
	// Go thorugh the successor versions in the accessors list
	BufferBlockList::iterator baseIt=blockPos;
	while(baseIt!=versions->accessors.begin()){
	  // Move one block forward in the list (toward newer blocks)
	  BufferBlockList::iterator forwIt=baseIt;
	  forwIt--;
          I(pendInstrCount);
	  BufferBlock *forwBlock=*forwIt;
	  Epoch *forwEpoch=forwBlock->epoch;
	  I(forwEpoch->myClock>=myClock);
	  if(forwBlock->xpMask[chunkIndx]&forwMask){
            I(!noSuccessorsLeft);
#if (defined DEBUG)
	    {
	      BufferBlockList::const_iterator
		searchIt=versions->accessors.begin();
	      while(searchIt!=versions->accessors.end()){
		if(searchIt==bufferBlock->accessorsPos)
		  break;
		searchIt++;
	      }
	      I(searchIt!=versions->accessors.end());
	    }
#endif
	    // A conflicting successor reader is squashed
	    I(pendInstrCount);
	    forwEpoch->squash(false);
	    I(pendInstrCount);
#if (defined DEBUG)
	    {
	      BufferBlockList::const_iterator
		searchIt=versions->accessors.begin();
	      while(searchIt!=versions->accessors.end()){
		if(searchIt==bufferBlock->accessorsPos)
		  break;
		searchIt++;
	      }
	      I(searchIt!=versions->accessors.end());
	    }
#endif
	    // Repeat with the same base iterator because
	    // we just removed the block ahead of it,
	    // and now we want to see the "new" block ahead
	    continue;
	  }else if(forwBlock->wrMask[chunkIndx]&forwMask){
            I(!noSuccessorsLeft);
	    // This block is stale because there is a successor overwriter
	    bufferBlock->becomeStale();
	    // This epoch is already succeeded
	    I(myState!=State::Acq);
	    myState+=State::NameSuccessors;
	    // No check for races if other epoch in the same thread
	    if(myThread!=forwEpoch->myThread){
	      // In any clock scheme, forwEpoch should not be before "this" epoch
	      I(!VClock::isOrder(forwEpoch->myVClock,myVClock));
	      if(compareClock(forwEpoch)!=StrongAfter){
		// An anomaly should also be a data race (no VClock ordering)
		I(!VClock::isOrder(myVClock,forwEpoch->myVClock));
		forwEpoch->myInstRaces.addAnom(iVAddr,versions->getAgeInThread(forwBlock));
	      }
	      if(compareCheckClock(forwEpoch)!=StrongAfter){
		// A check-clock anomaly should also be a data race (no VClock ordering)
		I(!VClock::isOrder(myVClock,forwEpoch->myVClock));
		forwEpoch->myInstRaces.addChka(iVAddr,versions->getAgeInThread(forwBlock));
	      }
	      if(((myState!=State::Atm)||(forwEpoch->myState!=State::Atm))
		 &&(forwEpoch->myState!=State::Completed)){
		// A running anomaly should also be a data race (no VClock ordering
                I(!VClock::isOrder(myVClock,forwEpoch->myVClock));
		forwEpoch->myInstRaces.addRuna(iVAddr,versions->getAgeInThread(forwBlock));
	      }
	      if(!VClock::isOrder(myVClock,forwEpoch->myVClock))
		forwEpoch->myInstRaces.addRace(iVAddr,versions->getAgeInThread(forwBlock));
	      if(!LClock::isOrder(myLClock,forwEpoch->myLClock))
		forwEpoch->myInstRaces.addLama(iVAddr,versions->getAgeInThread(forwBlock));
	    }
	  }
	  // Move toward the beginning of the list
	  // (from older to newer blocks)
	  baseIt--;
	}
      }
      // Check for obsolete versions
      if(backMask){
	BufferBlockList::iterator backIt=blockPos;
	I(backIt==bufferBlock->accessorsPos);
	backIt++;
	while(backMask&&(backIt!=versions->accessors.end())){
	  BufferBlock *otherBlock=*backIt;
	  backIt++;
	  ChunkBitMask wrConf=otherBlock->wrMask[chunkIndx]&backMask;
	  backMask^=wrConf;
	  ChunkBitMask rdConf=otherBlock->xpMask[chunkIndx]&backMask;
	  if(!(wrConf|rdConf))
	    continue;
	  if(otherBlock->epoch->myState==State::WaitAcqRetry){
	    I(rdConf);
	    I((myState==State::Rel)||
	      (myAtomicSection==atmOmmitInstr));
	    I(otherBlock->epoch->myState==State::NoSuccessors);
	    otherBlock->epoch->squash(false);
	    continue;
	  }
	  Epoch *prevEpoch=otherBlock->epoch;
	  otherBlock->becomeStale();
	  prevEpoch->myState+=State::NameSuccessors;
	  // No check for races if other epoch in the same thread
	  if(myThread==prevEpoch->myThread)
	    continue;
	  // Not a race when a releasor section overwriting a value read by acquirer
	  if((myState==State::Rel)&&(prevEpoch->myState==State::Acq))
	    continue;
	  // In any clock scheme, prevEpoch should not be after "this" epoch
	  I(!VClock::isOrder(myVClock,prevEpoch->myVClock));	  
	  // Check for anomalies using different clocks
	  if(compareClock(otherBlock->epoch)!=StrongBefore){
	    // An anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(prevEpoch->myVClock,myVClock));
	    myInstRaces.addAnom(iVAddr,versions->getAgeInThread(otherBlock));
	  }
	  if(compareCheckClock(otherBlock->epoch)!=StrongBefore){
	    // A check-clock anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(prevEpoch->myVClock,myVClock));
	    myInstRaces.addChka(iVAddr,versions->getAgeInThread(otherBlock));
	  }
	  if((prevEpoch->myState!=State::Atm)&&(myState!=State::Atm)&&
	     (prevEpoch->myState!=State::Completed)){
	    // A running anomaly should also be a data race (no VClock ordering)
	    I(!VClock::isOrder(prevEpoch->myVClock,myVClock));
	    myInstRaces.addRuna(iVAddr,versions->getAgeInThread(otherBlock));
	  }
	  if(!VClock::isOrder(prevEpoch->myVClock,myVClock))
	    myInstRaces.addRace(iVAddr,versions->getAgeInThread(otherBlock));
	  if(!LClock::isOrder(prevEpoch->myLClock,myLClock))
	    myInstRaces.addLama(iVAddr,versions->getAgeInThread(otherBlock));
	}
      }
    }
    // Add bytes to write mask, unless we are in condition mode
    bufferBlock->wrMask[chunkIndx]|=accMask;
//#if (defined DEBUG)
//    EpochList::iterator listIt=allEpochs.begin();
//    EpochList::iterator nextListIt=listIt;
//    nextListIt++;
//    while(nextListIt!=allEpochs.end()){
//      I((*listIt)->myClock>=(*nextListIt)->myClock);
//      I(!VClock::isOrder((*listIt)->myVClock,(*nextListIt)->myVClock));
//      listIt=nextListIt;
//      nextListIt++;
//    }
//#endif
    // If EagerMerge, we want this block to be removed later
    I(!blockRemovalEnabled);
    if(myState==State::EagerMerge)
      requestBlockRemoval(blockBase);
    return ((Address)(bufferBlock->wkData))+blockOffs;
  }
}
